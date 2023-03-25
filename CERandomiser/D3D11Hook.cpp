#include "pch.h"
#include "D3D11Hook.h"

#include "global_kill.h"

struct rgba {
    float r, g, b, a;
    rgba(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}
};


bool D3D11Hook::killPresentHook = false;


void D3D11Hook::initialize()
{
	PLOG_INFO << "creating D3D11 hooks";
    // Hook dx11 present and resizebuffers
		auto builder = safetyhook::Factory::acquire();

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

		get().mHookPresent = builder.create_inline(p_oldPresent, &newDX11Present);
		get().mHookResizeBuffers = builder.create_inline(p_oldResizeBuffers, &newDX11ResizeBuffers);

}

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall D3D11Hook::mNewWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (!res) // I think this means the ImGui window didn't have anything to handle
	{
		// so call the original and let it handle it
		return CallWindowProc(D3D11Hook::get().mOldWndProc, hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return res;
	}
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

	// Use swap chain description to get MCC window handle
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	pSwapChain->GetDesc(&swapChainDesc);
	if (!&swapChainDesc) throw expected_exception("Failed to get swap chain description");
	m_windowHandle = swapChainDesc.OutputWindow;

	// Use backBuffer to get MainRenderTargetView
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (!pBackBuffer) throw expected_exception("Failed to get BackBuffer");

	m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRenderTargetView);
	pBackBuffer->Release();
	if (!m_pMainRenderTargetView) throw expected_exception("Failed to get MainRenderTargetView");

	// Setup the imgui WndProc callback
	mOldWndProc = (WNDPROC)SetWindowLongPtrW(m_windowHandle, GWLP_WNDPROC, (LONG_PTR)mNewWndProc);



	// Setup ImGui stuff
	PLOG_DEBUG << "Initializing ImGui";
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

	if (!ImGui_ImplWin32_Init(m_windowHandle)) 
	{
		throw expected_exception(std::format("ImGui_ImplWin32_Init failed w/ {} ", (uint64_t)m_windowHandle).c_str());
	};
	if (!ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext)) 
	{
		throw expected_exception(std::format("ImGui_ImplDX11_Init failed w/ {}, {} ", (uint64_t)m_pDevice, (uint64_t)m_pDeviceContext).c_str());
	};

	ImGui_ImplDX11_NewFrame(); // need to get a new frame to be able to load default font
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::EndFrame();
	mDefaultFont = ImGui::GetIO().Fonts->Fonts[0]; // this is crashing. Fonts[0] is invalid?

}
const std::string testString = "testttting";
HRESULT D3D11Hook::newDX11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	D3D11Hook& instance = get();
	// This flag will only be set to true on D3D11Hook destruction
	// We will destroy the hook ourselves then return original present
	if (D3D11Hook::killPresentHook)
	{
		PLOG_INFO << "PresentHook destroying itself from newPresent";
		// call original present
		HRESULT res = instance.mHookPresent.call<HRESULT, IDXGISwapChain*, UINT, UINT>(pSwapChain, SyncInterval, Flags);

		// destroy the hook 
		instance.mHookPresent.reset();

		// set the kill flag to false to communicate this back to the constructor
		D3D11Hook::killPresentHook = false;

		// return 
		return res;
	}




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
   
	// Setup ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// I don't 100% understand what this does, but it must be done before we try to render
	instance.m_pDeviceContext->OMSetRenderTargets(1, &instance.m_pMainRenderTargetView, NULL);

	/* insert drawing */
	// for testing
	ImGui::GetOverlayDrawList()->AddText(ImVec2(20, 20), ImU32(0xFFFFFFFF), testString.c_str());

	// Finish ImGui frame
	ImGui::EndFrame();
	ImGui::Render();

	// Render it !
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	// Call original present
	return instance.mHookPresent.call<HRESULT, IDXGISwapChain*, UINT, UINT>(pSwapChain, SyncInterval, Flags);

}

HRESULT D3D11Hook::newDX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
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
	// Hook destruction
	// Check if present hook exists
	if (instance.mHookPresent.target() != 0)
	{
		// set the kill flag
		D3D11Hook::killPresentHook = true;

		// Wait for newPresent to kill the hook itself - it will communicate that by setting the flag back to false
		// with timeout for failure
		int wait = 0;
		while (D3D11Hook::killPresentHook)
		{
			Sleep(10);
			wait++;

			if (wait > 100)
			{
				// Kill the hook ourselves since newPresent doesn't want to do it, apparently
				PLOG_ERROR << "Manually killing present hook";
				instance.mHookPresent.reset();
				break;
			}
		}
	}

	// We can safely destroy mHookResizeBuffers since the chances that it's running while this release() is called is neglible
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

	// restore the original wndProc
	if (instance.mOldWndProc)
	{
		SetWindowLongPtrW(instance.m_windowHandle, GWLP_WNDPROC, (LONG_PTR)instance.mOldWndProc);
		instance.mOldWndProc = nullptr;
	}
}