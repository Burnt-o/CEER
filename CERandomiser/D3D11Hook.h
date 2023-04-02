#pragma once
#include "ModuleHook.h"
#include "ModuleHookManager.h"


// Define the DX functions we're going to hook
 extern "C" typedef HRESULT __stdcall DX11Present(IDXGISwapChain * pSwapChain, UINT SyncInterval, UINT Flags);
 extern "C" typedef HRESULT __stdcall DX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);


 // Singleton: on construction, attaches hooks (VMT hook so our stuff shows up in OBS etc) to the games Present and ResizeBuffers function.
 // The hooks will invoke presentHookEvent for other classes to listen to.
 // On destruction, hooks will be unattached.
class D3D11Hook
{
private:
	 static D3D11Hook* instance; 	// private Singleton instance so static hooks/callbacks can access
	 std::mutex mDestructionGuard; // Protects against Singleton destruction while hooks are executing

	 // our hook functions
	 static DX11Present newDX11Present;
	 static DX11ResizeBuffers newDX11ResizeBuffers;

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

	void initializeD3Ddevice(IDXGISwapChain*);
	bool isD3DdeviceInitialized = false;

public:

	explicit D3D11Hook();
	~D3D11Hook();


	// Callback for present rendering
	eventpp::CallbackList<void(ID3D11Device*, ID3D11DeviceContext*, IDXGISwapChain*, ID3D11RenderTargetView*)> presentHookEvent;


	// Banned operations for singleton
	D3D11Hook(const D3D11Hook& arg) = delete; // Copy constructor
	D3D11Hook(const D3D11Hook&& arg) = delete;  // Move constructor
	D3D11Hook& operator=(const D3D11Hook& arg) = delete; // Assignment operator
	D3D11Hook& operator=(const D3D11Hook&& arg) = delete; // Move operator



};
