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
	// constructor and destructor private
	explicit D3D11Hook() = default;
	~D3D11Hook() = default;
	// singleton accessor
	static D3D11Hook& get() {
		static D3D11Hook instance;
		return instance;
	}


	// hook objects
	safetyhook::InlineHook mHookPresent;
	safetyhook::InlineHook mHookResizeBuffers;
	static inline std::mutex mDestructionGuard; // Protects against D3D11Hook singleton destruction while hooks are executing

	// hook functions
	static DX11Present newDX11Present;
	static DX11ResizeBuffers newDX11ResizeBuffers;
	static LRESULT __stdcall mNewWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Doesn't need hook cos we got SetWindowLongPtr
	WNDPROC mOldWndProc = nullptr;
	
	static bool killPresentHook; // used to avoid race condition when destroying PresentHook


public:
	// D3D stuff
	void initializeD3Ddevice(IDXGISwapChain* pSwapChain);
	bool isD3DdeviceInitialized = false;
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	HWND m_windowHandle = nullptr;
	ID3D11RenderTargetView* m_pMainRenderTargetView = nullptr;

	// callback for present rendering
	eventpp::CallbackList<void(D3D11Hook&, IDXGISwapChain*, UINT , UINT )> presentHookCallback;

	// Imgui stuff
	ImFont* mDefaultFont = nullptr;
	


	// banned operations for singleton
	D3D11Hook(const D3D11Hook& arg) = delete; // Copy constructor
	D3D11Hook(const D3D11Hook&& arg) = delete;  // Move constructor
	D3D11Hook& operator=(const D3D11Hook& arg) = delete; // Assignment operator
	D3D11Hook& operator=(const D3D11Hook&& arg) = delete; // Move operator

public:
	static void initialize(); // Set up hooks
	static void release(); // gets rid of hooks and releases d3d resources
	static void destroy() // destroys the singleton
	{
		// It's important that hooks are destroyed BEFORE the rest of the class is
		// as the hook functions will try to access class members
		// and also the d3d resources need to be manually released
		release();
		std::scoped_lock<std::mutex> lock(mDestructionGuard); // Hook functions lock this
		get().~D3D11Hook();
	}
};
