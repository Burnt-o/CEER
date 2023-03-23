#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "ModuleHook.h"
#include "ModuleHookManager.h"


 extern "C" typedef HRESULT __stdcall DX11Present(IDXGISwapChain * pSwapChain, UINT SyncInterval, UINT Flags);
 extern "C" typedef HRESULT __stdcall DX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

class D3D11Hook
{
private:
	// constructor private
	explicit D3D11Hook();

	// singleton accessor
	static D3D11Hook& get() {
		static D3D11Hook instance;
		return instance;
	}


	// hook objects
	safetyhook::InlineHook mHookPresent;
	safetyhook::InlineHook mHookResizeBuffers;

	// hook functions
	static DX11Present newDX11Present;
	static DX11ResizeBuffers newDX11ResizeBuffers;
	static LRESULT __stdcall mNewWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Doesn't need hook cos we got SetWindowLongPtr
	WNDPROC mOldWndProc = nullptr;
	
	// D3D stuff
	void initializeD3Ddevice(IDXGISwapChain* pSwapChain);
	bool isD3DdeviceInitialized = false;
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	HWND m_windowHandle = nullptr;
	ID3D11RenderTargetView* m_pMainRenderTargetView = nullptr;


	// Imgui stuff
	ImFont* mDefaultFont = nullptr;
	


	~D3D11Hook() = default; // TODO: release d3d stuff

	// banned operations for singleton
	D3D11Hook(const D3D11Hook& arg) = delete; // Copy constructor
	D3D11Hook(const D3D11Hook&& arg) = delete;  // Move constructor
	D3D11Hook& operator=(const D3D11Hook& arg) = delete; // Assignment operator
	D3D11Hook& operator=(const D3D11Hook&& arg) = delete; // Move operator

public:
	static void initialize()
	{
		get();
	}

	static void destroy()
	{
		get().~D3D11Hook();
	}
};
