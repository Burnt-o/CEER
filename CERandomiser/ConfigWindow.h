#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "Hook.h"
#include "HookManager.h"
class ConfigWindow
{
private:

	static ConfigWindow& get() {
		static ConfigWindow instance;
		return instance;
	}

	std::shared_ptr<InlineHook> mHookPresent;
	std::shared_ptr<InlineHook> mHookResizeBuffers;


	// hook functions
	HRESULT __stdcall newDX11Present(IDXGISwapChain * pSwapChain, UINT SyncInterval, UINT Flags);
	HRESULT __stdcall newDX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	
	
	// D3D stuff
	void initializeD3Ddevice(IDXGISwapChain* pSwapChain);
	bool isD3DdeviceInitialized = false;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	HWND m_windowHandle;
	ID3D11RenderTargetView* m_pMainRenderTargetView;
	static LRESULT __stdcall mNewWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Doesn't need hook cos we got SetWindowLongPtr
	WNDPROC mOldWndProc = nullptr;

	// Imgui stuff
	ImFont* mDefaultFont;
	

	ConfigWindow() = default;
	~ConfigWindow() = default;

public:
	static void initialize();
	//LONG_PTR getNewWndProcPointer()
	//{
	//	return (LONG_PTR)get()->mNewWndProc;
	//}
};