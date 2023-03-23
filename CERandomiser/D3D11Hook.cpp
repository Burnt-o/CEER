#include "pch.h"
#include "D3D11Hook.h"

#include "global_kill.h"

struct rgba {
    float r, g, b, a;
    rgba(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}
};





D3D11Hook::D3D11Hook()
{
    // Hook dx11 present and resizebuffers
    auto mHookPresent = std::make_shared<ModuleInlineHook>(L"hookDX11Present", MultilevelPointer::make({0, 0, 0}), &newDX11Present, true);
    auto mHookResizeBuffers = std::make_shared<ModuleInlineHook>(L"hookDX11ResizeBuffers", MultilevelPointer::make({ 0, 0, 0 }), &newDX11ResizeBuffers, true);

    // HookManager will share ownership and manage/attach the hooks
	// We do keep a copy though so we can call the original functions
    ModuleHookManager::addHook(mHookPresent);
    ModuleHookManager::addHook(mHookResizeBuffers);



	// So I think we split off the below code into something that gets called by .. hkpresent... i dunno

    // Create a window called "My First Tool", with a menu bar.
    ImGui::Begin("My First Tool", NULL, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("testmenu"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
            //if (ImGui::MenuItem("Close", "Ctrl+W")) { my_tool_active = false; }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Edit a color stored as 4 floats
    rgba my_color = rgba(0.f, 0.f, 0.f, 0.f);
    ImGui::ColorEdit4("Color", &my_color.r);

    // Generate samples and plot them
    float samples[100];
    for (int n = 0; n < 100; n++)
        samples[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);
    ImGui::PlotLines("Samples", samples, 100);

    // Display contents in a scrolling region
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
    ImGui::BeginChild("Scrolling");
    for (int n = 0; n < 50; n++)
        ImGui::Text("%04d: Some text", n);
    ImGui::EndChild();
    ImGui::End();
}

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall D3D11Hook::mNewWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (!res)
	{
		PLOG_DEBUG << "ImGui_ImplWin32_WndProcHandler failed, calling original";
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

	if (!SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)m_pDevice)))
	{
		throw expected_exception("Failed to get D3D11Device");

	}

	// TODO: investigate if we're allowed to use shared pointers for all the D3D references we're grabbing so we don't have to manually clean up later
	// plus that would prevent leaking if we throw one of the expected exceptions

	// Get Device Context
	m_pDevice->GetImmediateContext(&m_pDeviceContext);
	if (!m_pDevice) throw expected_exception("Failed to get DeviceContext");

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
		throw expected_exception(std::format("ImGui_ImplWin32_Init failed w/ {} ", m_windowHandle).c_str());
	};
	if (!ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext)) 
	{
		throw expected_exception(std::format("ImGui_ImplDX11_Init failed w/ {}, {} ", m_pDevice, m_pDeviceContext).c_str());
	};

	ImGui_ImplDX11_NewFrame(); // need to get a new frame to be able to load default font
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::EndFrame();
	mDefaultFont = ImGui::GetIO().Fonts->Fonts[0]; // this is crashing. Fonts[0] is invalid?

}

HRESULT __stdcall D3D11Hook::newDX11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	if (!isD3DdeviceInitialized)
	{
		try
		{
			initializeD3Ddevice(pSwapChain);
			isD3DdeviceInitialized = true;
		}
		catch (expected_exception ex)
		{
			PLOG_FATAL << "Failed to initialize d3d device, info: " << std::endl
				<< ex.what()
				<< "CEER will now automatically close down";
			global_kill::kill_me();

			// Call original present
			return mHookPresent.get()->getInlineHook().call<HRESULT, IDXGISwapChain*, UINT, UINT>(pSwapChain, SyncInterval, Flags);
		}
		
	}
   
	// Setup ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// I don't 100% understand what this does, but it must be done before we try to render
	m_pDeviceContext->OMSetRenderTargets(1, &m_pMainRenderTargetView, NULL);

	// Finish ImGui frame
	ImGui::EndFrame();
	ImGui::Render();

	// Render it !
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	// Call original present
	return mHookPresent.get()->getInlineHook().call<HRESULT, IDXGISwapChain*, UINT, UINT>(pSwapChain, SyncInterval, Flags);

}

HRESULT __stdcall D3D11Hook::newDX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	// Need to release mainRenderTargetView before calling ResizeBuffers
	if (m_pMainRenderTargetView != nullptr)
	{
		m_pDeviceContext->OMSetRenderTargets(0, 0, 0);
		m_pMainRenderTargetView->Release();
	}

	// Call original ResizeBuffers
	HRESULT hr = mHookResizeBuffers.get()->getInlineHook().call<HRESULT, IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT>(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags); // Will return this at the end

	// Resetup the mainRenderTargetView
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (!pBackBuffer) throw expected_exception("Failed to get BackBuffer");

	m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRenderTargetView);
	pBackBuffer->Release();
	if (!m_pMainRenderTargetView) throw expected_exception("Failed to get MainRenderTargetView");

	m_pDeviceContext->OMSetRenderTargets(1, &m_pMainRenderTargetView, NULL); //TODO: is this necessary ? we don't do this in init

	// setup the viewport. TODO: is this necessary? we don't do this in init
	D3D11_VIEWPORT viewport;
	viewport.Width = Width;
	viewport.Height = Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	// We could grab the new window Height and Width too here if we cared about that, but I don't.

	return hr;
}