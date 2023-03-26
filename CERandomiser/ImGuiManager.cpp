#include "pch.h"
#include "ImGuiManager.h"
#include "global_kill.h"



IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall ImGuiManager::mNewWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (!res) // I think this means the ImGui window didn't have anything to handle
	{
		// so call the original and let it handle it
		return CallWindowProc(ImGuiManager::get().mOldWndProc, hWnd, uMsg, wParam, lParam);
	}
	else
	{
		ImGui::GetIO().WantCaptureMouse = true;
		return res;
	}
}

void ImGuiManager::initializeImGuiResources(D3D11Hook& d3d, IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	// Use swap chain description to get MCC window handle
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	pSwapChain->GetDesc(&swapChainDesc);
	if (!&swapChainDesc) throw expected_exception("Failed to get swap chain description");
	m_windowHandle = swapChainDesc.OutputWindow;

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
	if (!ImGui_ImplDX11_Init(d3d.m_pDevice, d3d.m_pDeviceContext))
	{
		throw expected_exception(std::format("ImGui_ImplDX11_Init failed w/ {}, {} ", (uint64_t)d3d.m_pDevice, (uint64_t)d3d.m_pDeviceContext).c_str());
	};

	ImGui_ImplDX11_NewFrame(); // need to get a new frame to be able to load default font
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::EndFrame();
	mDefaultFont = ImGui::GetIO().Fonts->Fonts[0]; 
}

void ImGuiManager::release()
{
	ImGuiManager& instance = get();
		// restore the original wndProc
	if (instance.mOldWndProc)
	{
		SetWindowLongPtrW(instance.m_windowHandle, GWLP_WNDPROC, (LONG_PTR)instance.mOldWndProc);
		instance.mOldWndProc = nullptr;
	}

}

void ImGuiManager::onPresentHookCallback(D3D11Hook& d3d, IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	std::scoped_lock<std::mutex> lock(mDestructionGuard); // ImGuiManager::destroy also locks this

	if (!get().m_isImguiInitialized)
	{
		PLOG_INFO << "Initializing ImGuiManager";
		try
		{
			get().initializeImGuiResources(d3d, pSwapChain, SyncInterval, Flags);
			get().m_isImguiInitialized = true;
		}
		catch (expected_exception& ex)
		{
			PLOG_FATAL << "Failed to initialize ImGui, info: " << std::endl
				<< ex.what() << std::endl
				<< "CEER will now automatically close down";
			global_kill::kill_me();
			return;
		}
	}

	// Start ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// I don't 100% understand what this does, but it must be done before we try to render
	d3d.m_pDeviceContext->OMSetRenderTargets(1, &d3d.m_pMainRenderTargetView, NULL);

	// invoke callback of anything that wants to render with ImGui
	get().ImGuiRenderCallback();

	// Finish ImGui frame
	ImGui::EndFrame();
	ImGui::Render();

	// Render it !
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

