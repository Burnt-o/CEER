#pragma once
#include "ModuleHook.h"
#include "ModuleHookManager.h"

// imgui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_stdlib.h"

// Define the DX functions we're going to hook
 extern "C" typedef HRESULT __stdcall DX11Present(IDXGISwapChain * pSwapChain, UINT SyncInterval, UINT Flags);
 extern "C" typedef HRESULT __stdcall DX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

 //SCALAR TO ImVec2 OPERATIONS
 inline ImVec2 operator + (const ImVec2& v1, float s) { return ImVec2(v1.x + s, v1.y + s); }
 inline ImVec2 operator - (const ImVec2& v1, float s) { return ImVec2(v1.x - s, v1.y - s); }
 inline ImVec2 operator * (const ImVec2& v1, float s) { return ImVec2(v1.x * s, v1.y * s); }
 inline ImVec2 operator / (const ImVec2& v1, float s) { return ImVec2(v1.x / s, v1.y / s); }


 //ImVec2 TO ImVec2 OPERATIONS
 inline ImVec2 operator + (const ImVec2& v1, const ImVec2& v2) { return ImVec2(v1.x + v2.x, v1.y + v2.y); }
 inline ImVec2 operator - (const ImVec2& v1, const ImVec2& v2) { return ImVec2(v1.x - v2.x, v1.y - v2.y); }
 inline ImVec2 operator * (const ImVec2& v1, const ImVec2& v2) { return ImVec2(v1.x * v2.x, v1.y * v2.y); }
 inline ImVec2 operator / (const ImVec2& v1, const ImVec2& v2) { return ImVec2(v1.x / v2.x, v1.y / v2.y); }

 //SCALAR TO ImVec4 OPERATIONS
 inline ImVec4 operator + (const ImVec4& v1, float s) { return ImVec4(v1.x + s, v1.y + s, v1.z + s, v1.w + s); }
 inline ImVec4 operator - (const ImVec4& v1, float s) { return ImVec4(v1.x - s, v1.y - s, v1.z - s, v1.w - s); }
 inline ImVec4 operator * (const ImVec4& v1, float s) { return ImVec4(v1.x * s, v1.y * s, v1.z * s, v1.w * s); }
 inline ImVec4 operator / (const ImVec4& v1, float s) { return ImVec4(v1.x / s, v1.y / s, v1.z / s, v1.w / s); }


 //ImVec2 TO ImVec4 OPERATIONS
 inline ImVec4 operator + (const ImVec4& v1, const ImVec4& v2) { return ImVec4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w); }
 inline ImVec4 operator - (const ImVec4& v1, const ImVec4& v2) { return ImVec4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w); }
 inline ImVec4 operator * (const ImVec4& v1, const ImVec4& v2) { return ImVec4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w); }
 inline ImVec4 operator / (const ImVec4& v1, const ImVec4& v2) { return ImVec4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w); }


 // Singleton: on construction, attaches hooks (VMT hook so our stuff shows up in OBS etc) to the games Present and ResizeBuffers function.
 // The hooks will invoke presentHookEvent for other classes to listen to.
 // On destruction, hooks will be unattached.

 
 class D3D11Hook
{
private:
	 static D3D11Hook* instance; 	// Private Singleton instance so static hooks/callbacks can access
	 std::mutex mDestructionGuard; // Protects against Singleton destruction while hooks are executing

	 // Our hook functions
	 static DX11Present newDX11Present;
	 static DX11ResizeBuffers newDX11ResizeBuffers;

	 // For a VMT hook we need to keep track of the original functions and the VMT entry containing the function
	 // Pointers to original functions
	 DX11Present* m_pOriginalPresent = nullptr;
	 DX11ResizeBuffers* m_pOriginalResizeBuffers = nullptr;
	 // Pointers to the games pointers to original function (we redirect these to create hook)
	 DX11Present** m_ppPresent = nullptr;
	 DX11ResizeBuffers** m_ppResizeBuffers = nullptr;
	

	// D3D data
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	ID3D11RenderTargetView* m_pMainRenderTargetView = nullptr;

	void CreateDummySwapchain(IDXGISwapChain*& pDummySwapchain, ID3D11Device*& pDummyDevice);
	void initializeD3Ddevice(IDXGISwapChain*);
	bool isD3DdeviceInitialized = false;

	static ImVec2 mScreenSize;

public:

	explicit D3D11Hook();
	~D3D11Hook();


	// Callback for present rendering
	eventpp::CallbackList<void(ID3D11Device*, ID3D11DeviceContext*, IDXGISwapChain*, ID3D11RenderTargetView*)> presentHookEvent;

	// Callback for window resize
	eventpp::CallbackList<void(ImVec2 newScreenSize)> resizeBuffersHookEvent;

	// Banned operations for singleton
	D3D11Hook(const D3D11Hook& arg) = delete; // Copy constructor
	D3D11Hook(const D3D11Hook&& arg) = delete;  // Move constructor
	D3D11Hook& operator=(const D3D11Hook& arg) = delete; // Assignment operator
	D3D11Hook& operator=(const D3D11Hook&& arg) = delete; // Move operator

	static ImVec2 getScreenSize() { return mScreenSize; }


};
