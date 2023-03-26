#include "pch.h"
#include "D3D11Hook.h"

#include "global_kill.h"

struct rgba {
    float r, g, b, a;
    rgba(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}
};




void D3D11Hook::initialize()
{
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
		}
		catch (expected_exception& ex)
		{
			PLOG_FATAL << "Failed to initialize d3d device, info: " << std::endl
				<< ex.what() << std::endl
				<< "CEER will now automatically close down";
			global_kill::kill_me();

			// Call original present
			return instance.mHookPresent.call<HRESULT, IDXGISwapChain*, UINT, UINT>(pSwapChain, SyncInterval, Flags);
		}
		
	}
   
	// Invoke the callback
	instance.presentHookCallback(instance, pSwapChain, SyncInterval, Flags);

	


	// Call original present
	return instance.mHookPresent.call<HRESULT, IDXGISwapChain*, UINT, UINT>(pSwapChain, SyncInterval, Flags);

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
	HRESULT hr = instance.mHookResizeBuffers.call<HRESULT, IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT>(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags); // Will return this at the end

	// Resetup the mainRenderTargetView
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (!pBackBuffer) throw expected_exception("Failed to get BackBuffer");

	instance.m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &instance.m_pMainRenderTargetView);
	pBackBuffer->Release();
	if (!instance.m_pMainRenderTargetView) throw expected_exception("Failed to get MainRenderTargetView");

	instance.m_pDeviceContext->OMSetRenderTargets(1, &instance.m_pMainRenderTargetView, NULL); //TODO: is this necessary ? we don't do this in init

	// setup the viewport. TODO: is this necessary? we don't do this in init
	D3D11_VIEWPORT viewport;
	viewport.Width = Width;
	viewport.Height = Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	instance.m_pDeviceContext->RSSetViewports(1, &viewport);

	// We could grab the new window Height and Width too here if we cared about that, but I don't.

	return hr;
}

// Safely destroys hooks
// Releases D3D resources, if we acquired them
void D3D11Hook::release()
{
	D3D11Hook& instance = get();
	// Destroy the hooks
	instance.mHookPresent.reset();
	instance.mHookResizeBuffers.reset();

	// D3D resource releasing:
	// need to call release on the device https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nf-d3d9helper-idirect3dswapchain9-getdevice
	if (instance.m_pDevice)
	{
		instance.m_pDevice->Release();
		instance.m_pDevice = nullptr;
	}

	// and the device context
	if (instance.m_pDeviceContext)
	{
		instance.m_pDeviceContext->Release();
		instance.m_pDeviceContext = nullptr;
	}

	// and the mainRenderTargetView
	if (instance.m_pMainRenderTargetView)
	{
		instance.m_pMainRenderTargetView->Release();
		instance.m_pMainRenderTargetView = nullptr;
	}


}