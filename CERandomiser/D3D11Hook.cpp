#include "pch.h"
#include "D3D11Hook.h"

#include "global_kill.h"

DX11Present* D3D11Hook::m_pOriginalPresent = nullptr;
DX11ResizeBuffers* D3D11Hook::m_pOriginalResizeBuffers = nullptr;
DX11Present** D3D11Hook::m_ppPresent = nullptr;
DX11ResizeBuffers** D3D11Hook::m_ppResizeBuffers = nullptr;


struct rgba {
    float r, g, b, a;
    rgba(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}
};




void D3D11Hook::initialize()
{


#ifdef USE_VMT_HOOK
	enum class IDXGISwapChainVMT {
		QueryInterface,
		AddRef,
		Release,
		SetPrivateData,
		SetPrivateDataInterface,
		GetPrivateData,
		GetParent,
		GetDevice,
		Present,
		GetBuffer,
		SetFullscreenState,
		GetFullscreenState,
		GetDesc,
		ResizeBuffers,
		ResizeTarget,
		GetContainingOutput,
		GetFrameStatistics,
		GetLastPresentCount,
	};


	ID3D11Device* pDummyDevice = nullptr;
	IDXGISwapChain* pDummySwapchain = nullptr;

	// Create a dummy device, get swapchain vmt, hook present.
	D3D_FEATURE_LEVEL featLevel;
	DXGI_SWAP_CHAIN_DESC sd{ 0 };
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.Height = 800;
	sd.BufferDesc.Width = 600;
	sd.BufferDesc.RefreshRate = { 60, 1 };
	sd.OutputWindow = GetForegroundWindow();
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pDummySwapchain, &pDummyDevice, &featLevel, nullptr);
	if (FAILED(hr))
	{
		PLOG_FATAL << "failed to create dummy d3d device and swapchain";
		return;
	}

	// Get swapchain vmt
	void** pVMT = *(void***)pDummySwapchain;

	// Get Present's address out of vmt
	m_pOriginalPresent = (DX11Present*)pVMT[(UINT)IDXGISwapChainVMT::Present];
	m_ppPresent = (DX11Present**)&pVMT[(UINT)IDXGISwapChainVMT::Present];
	PLOG_INFO << "PRESENT: " << m_pOriginalPresent;
	PLOG_INFO << "PRESENT pointer: " << m_ppPresent;

	// Get resizeBuffers too
	m_pOriginalResizeBuffers = (DX11ResizeBuffers*)pVMT[(UINT)IDXGISwapChainVMT::ResizeBuffers];
	m_ppResizeBuffers = (DX11ResizeBuffers**)&pVMT[(UINT)IDXGISwapChainVMT::ResizeBuffers];
	PLOG_INFO << "RESIZEBUFFERS: " << m_pOriginalResizeBuffers;
	PLOG_INFO << "RESIZEBUFFERS pointer: " << m_ppResizeBuffers;

	// Don't need the dummy device anymore
	safe_release(pDummySwapchain);
	safe_release(pDummyDevice);

	PLOG_DEBUG << "rewriting present pointer";
	// Rewrite the present pointer to instead point to our newPresent
	// Need access tho!
	patch_pointer(m_ppPresent, (uintptr_t)& newDX11Present);
	// resizeBuffers too
	patch_pointer(m_ppResizeBuffers, (uintptr_t)&newDX11ResizeBuffers);

#else
	PLOG_INFO << "creating D3D11 hooks";
	// Hook dx11 present and resizebuffers
	auto mlp_oldPresent = MultilevelPointer::make(L"d3d11.dll", { 0x9E5D0 });
	auto mlp_oldResizeBuffers = MultilevelPointer::make(L"d3d11.dll", { 0x9EBC0 });
	void* p_oldPresent;
	void* p_oldResizeBuffers;
	if (!mlp_oldPresent->resolve(&p_oldPresent))
	{
		throw expected_exception("D3D11 hook unable to resolve Present");
	}

	if (!mlp_oldResizeBuffers->resolve(&p_oldResizeBuffers))
	{
		throw expected_exception("D3D11 hook unable to resolve ResizeBuffers");
	}

	PLOG_DEBUG << "oldPresent: " << p_oldPresent;
	PLOG_DEBUG << "oldResizeBuffers: " << p_oldResizeBuffers;

	get().mHookPresent = safetyhook::create_inline(p_oldPresent, &newDX11Present);
	get().mHookResizeBuffers = safetyhook::create_inline(p_oldResizeBuffers, &newDX11ResizeBuffers);
#endif // USE_VMT_HOOK




}





void D3D11Hook::initializeD3Ddevice(IDXGISwapChain* pSwapChain)
{

	PLOG_DEBUG << "Initializing D3Ddevice" << std::endl;

	if (!SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&m_pDevice)))
	{
		throw expected_exception(std::format("Failed to get D3D11Device, pSwapChain: {:x}", (uint64_t)pSwapChain).c_str());

	}

	// Get Device Context
	m_pDevice->GetImmediateContext(&m_pDeviceContext);
	if (!m_pDeviceContext)
	{
		throw expected_exception("Failed to get DeviceContext");
	}



	// Use backBuffer to get MainRenderTargetView
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (!pBackBuffer) throw expected_exception("Failed to get BackBuffer");

	m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRenderTargetView);
	pBackBuffer->Release();
	if (!m_pMainRenderTargetView) throw expected_exception("Failed to get MainRenderTargetView");

	

}

HRESULT D3D11Hook::newDX11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	std::scoped_lock<std::mutex> lock(D3D11Hook::mDestructionGuard); // Protects against D3D11Hook singleton destruction while hooks are executing
	D3D11Hook& instance = get();

	if (!instance.isD3DdeviceInitialized)
	{
		try
		{
			instance.initializeD3Ddevice(pSwapChain);
			instance.isD3DdeviceInitialized = true;
			PLOG_DEBUG << "D3D device initialized";
		}
		catch (expected_exception& ex)
		{
			PLOG_FATAL << "Failed to initialize d3d device, info: " << std::endl
				<< ex.what() << std::endl
				<< "CEER will now automatically close down";
			global_kill::kill_me();

			// Call original present
#ifdef USE_VMT_HOOK
			return m_pOriginalPresent(pSwapChain, SyncInterval, Flags);
#else
			return instance.mHookPresent.call<HRESULT, IDXGISwapChain*, UINT, UINT>(pSwapChain, SyncInterval, Flags);
#endif

		}
		
	}
   
	// Invoke the callback
	instance.presentHookCallback(instance, pSwapChain, SyncInterval, Flags);

	


	// Call original present
#ifdef USE_VMT_HOOK
	return m_pOriginalPresent(pSwapChain, SyncInterval, Flags);
#else
	return instance.mHookPresent.call<HRESULT, IDXGISwapChain*, UINT, UINT>(pSwapChain, SyncInterval, Flags);
#endif

}




HRESULT D3D11Hook::newDX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	std::scoped_lock<std::mutex> lock(D3D11Hook::mDestructionGuard); // Protects against D3D11Hook singleton destruction while hooks are executing
	D3D11Hook& instance = get();

	// Need to release mainRenderTargetView before calling ResizeBuffers
	if (instance.m_pMainRenderTargetView != nullptr)
	{
		instance.m_pDeviceContext->OMSetRenderTargets(0, 0, 0);
		instance.m_pMainRenderTargetView->Release();
	}

	// Call original ResizeBuffers
#ifdef USE_VMT_HOOK
	HRESULT hr =  m_pOriginalResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
#else
	HRESULT hr = instance.mHookResizeBuffers.call<HRESULT, IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT>(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags); // Will return this at the end
#endif


	// Resetup the mainRenderTargetView
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (!pBackBuffer) throw expected_exception("Failed to get BackBuffer");

	instance.m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &instance.m_pMainRenderTargetView);
	pBackBuffer->Release();
	if (!instance.m_pMainRenderTargetView) throw expected_exception("Failed to get MainRenderTargetView");

	// We could grab the new window Height and Width too here if we cared about that, but I don't.

	return hr;
}

// Safely destroys hooks
// Releases D3D resources, if we acquired them
void D3D11Hook::release()
{
	D3D11Hook& instance = get();
	// Destroy the hooks
#ifdef USE_VMT_HOOK
	// rewrite the pointers to go back to the original value
	patch_pointer(m_ppPresent, (uintptr_t)m_pOriginalPresent);
	// resizeBuffers too
	patch_pointer(m_ppResizeBuffers, (uintptr_t)m_pOriginalResizeBuffers);
#else
	instance.mHookPresent.reset();
	instance.mHookResizeBuffers.reset();
#endif

	// D3D resource releasing:
	// need to call release on the device https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nf-d3d9helper-idirect3dswapchain9-getdevice
	safe_release(instance.m_pDevice);
	safe_release(instance.m_pDeviceContext);
	safe_release(instance.m_pMainRenderTargetView);
	
}