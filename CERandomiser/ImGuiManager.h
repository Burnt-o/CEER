#pragma once
#include "D3D11Hook.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
class ImGuiManager
{
private:
	// constructor and destructor private
	explicit ImGuiManager() = default;
	~ImGuiManager() = default;
	// singleton accessor

public:
	static ImGuiManager& get() {
		static ImGuiManager instance;
		return instance;
	}
private:
	void initializeImGuiResources(D3D11Hook& d3d, IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	bool m_isImguiInitialized = false;
	static inline std::mutex mDestructionGuard; // Protects against ImGuiManager singleton destruction while callbacks are executing

	static void onPresentHookCallback(D3D11Hook&, IDXGISwapChain*, UINT, UINT);

	// Actual ImGui resources
	ImFont* mDefaultFont = nullptr;
	HWND m_windowHandle = nullptr;
	static LRESULT __stdcall mNewWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Doesn't need hook cos we got SetWindowLongPtr
	WNDPROC mOldWndProc = nullptr;
	static eventpp::CallbackList<void(D3D11Hook&, IDXGISwapChain*, UINT, UINT)>::Handle mCallbackHandle;

public:
	eventpp::CallbackList<void()> ImGuiRenderCallback; // things that want to render w/ imgui will listen to this
	static void initialize(D3D11Hook& d3d) // Listen to presentHookCallback
	{
		mCallbackHandle = d3d.presentHookCallback.append(onPresentHookCallback);
	}
	static void release(); // release imgui resources
	static void destroy() // destroys the singleton
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard); // onPresentHookCallback also locks this
		release();
		get().~ImGuiManager();
	}
};

