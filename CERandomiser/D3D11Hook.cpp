#include "pch.h"
#include "D3D11Hook.h"

#include "global_kill.h"


D3D11Hook* D3D11Hook::instance = nullptr;

struct rgba {
    float r, g, b, a;
    rgba(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}
};


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




D3D11Hook::D3D11Hook()
{

	if (instance != nullptr)
	{
		throw expected_exception("Cannot have more than one D3D11Hook");
	}
	instance = this;

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

// static 
HRESULT D3D11Hook::newDX11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	auto d3d = instance;
	
	std::scoped_lock<std::mutex> lock(d3d->mDestructionGuard); // Protects against D3D11Hook singleton destruction while hooks are executing


	if (!d3d->isD3DdeviceInitialized)
	{
		try
		{
			d3d->initializeD3Ddevice(pSwapChain);
			d3d->isD3DdeviceInitialized = true;
			PLOG_DEBUG << "D3D device initialized";
		}
		catch (expected_exception& ex)
		{
			PLOG_FATAL << "Failed to initialize d3d device, info: " << std::endl
				<< ex.what() << std::endl
				<< "CEER will now automatically close down";
			global_kill::kill_me();

			// Call original present
			return d3d->m_pOriginalPresent(pSwapChain, SyncInterval, Flags);


		}
		
	}
   
	// Invoke the callback
	d3d->presentHookCallback(d3d->m_pDevice, d3d->m_pDeviceContext, pSwapChain, d3d->m_pMainRenderTargetView);

	// Call original present
	return d3d->m_pOriginalPresent(pSwapChain, SyncInterval, Flags);

}



// static
HRESULT D3D11Hook::newDX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	auto d3d = instance;

	std::scoped_lock<std::mutex> lock(d3d->mDestructionGuard); // Protects against D3D11Hook singleton destruction while hooks are executing

	// Need to release mainRenderTargetView before calling ResizeBuffers
	if (d3d->m_pMainRenderTargetView != nullptr)
	{
		d3d->m_pDeviceContext->OMSetRenderTargets(0, 0, 0);
		d3d->m_pMainRenderTargetView->Release();
	}

	// Call original ResizeBuffers
	HRESULT hr = d3d->m_pOriginalResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);


	// Resetup the mainRenderTargetView
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (!pBackBuffer) throw expected_exception("Failed to get BackBuffer");

	d3d->m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &d3d->m_pMainRenderTargetView);
	pBackBuffer->Release();
	if (!d3d->m_pMainRenderTargetView) throw expected_exception("Failed to get MainRenderTargetView");

	// We could grab the new window Height and Width too here if we cared about that, but I don't.

	return hr;
}

// Safely destroys hooks
// Releases D3D resources, if we acquired them
D3D11Hook::~D3D11Hook()
{
// It's important that hooks are destroyed BEFORE the rest of the class is
// as the hook functions will try to access class members
// and also the d3d resources need to be manually released

	std::scoped_lock<std::mutex> lock(mDestructionGuard); // Hook functions lock this

	// Destroy the hooks
	// rewrite the pointers to go back to the original value
	patch_pointer(m_ppPresent, (uintptr_t)m_pOriginalPresent);
	// resizeBuffers too
	patch_pointer(m_ppResizeBuffers, (uintptr_t)m_pOriginalResizeBuffers);

	// D3D resource releasing:
	// need to call release on the device https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nf-d3d9helper-idirect3dswapchain9-getdevice
	
	safe_release(m_pDevice);
	safe_release(m_pDeviceContext);
	safe_release(m_pMainRenderTargetView);
	instance = nullptr;
	
}