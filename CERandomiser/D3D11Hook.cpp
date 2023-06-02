#include "pch.h"
#include "D3D11Hook.h"

#include "GlobalKill.h"
#include <dxgi.h>
#pragma comment(lib, "dxgi")

D3D11Hook* D3D11Hook::instance = nullptr;
ImVec2 D3D11Hook::mScreenSize;


struct rgba {
    float r, g, b, a;
    rgba(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}
};


enum class IDXGISwapChainVMT {
	QueryInterface,
	AddRef,
	Release,
	SetPrivateData,
	SetPrivateDataInterface,
	GetPrivateData,
	GetParent,
	GetDevice,
	Present,
	GetBuffer,
	SetFullscreenState,
	GetFullscreenState,
	GetDesc,
	ResizeBuffers,
	ResizeTarget,
	GetContainingOutput,
	GetFrameStatistics,
	GetLastPresentCount,
};




void D3D11Hook::CreateDummySwapchain(IDXGISwapChain*& pDummySwapchain, ID3D11Device*& pDummyDevice)
{

	std::vector<std::string> errorCodes;

#define logSwapChainFailure(x) PLOG_ERROR << x; errorCodes.push_back(x)

	D3D_FEATURE_LEVEL featLevel;
	DXGI_SWAP_CHAIN_DESC sd{ 0 };
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.Height = 800;
	sd.BufferDesc.Width = 600;
	sd.BufferDesc.RefreshRate = { 60, 1 };
	sd.OutputWindow = GetForegroundWindow();
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;


	
	// the following attempts at D3D11CreateDeviceAndSwapChain differ in the driver_type param
	// https://learn.microsoft.com/en-us/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_driver_type

	// use D3D_DRIVER_TYPE_WARP
	// Seems to be the most consistent
	{
		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pDummySwapchain, &pDummyDevice, &featLevel, nullptr);
		if (SUCCEEDED(hr))
		{
			PLOG_INFO << "2: Succesfully created dummy swapchain";
			return;
		}
		else
		{
			logSwapChainFailure(std::format("2: failed to create dummy d3d device and swapchain, error: {:x}", (ULONG)hr));
		}
	}

	// use D3D_DRIVER_TYPE_REFERENCE. The original code I used.
	// works for me but not gronchy
	{
		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pDummySwapchain, &pDummyDevice, &featLevel, nullptr);
		if (SUCCEEDED(hr))
		{
			PLOG_INFO << "1: Succesfully created dummy swapchain";
			return;
		}
		else
		{
			logSwapChainFailure(std::format("1: failed to create dummy d3d device and swapchain, error: {:x}", (ULONG)hr));
		}
	}

	// use D3D_DRIVER_TYPE_HARDWARE
	// works for me
	{
		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pDummySwapchain, &pDummyDevice, &featLevel, nullptr);
		if (SUCCEEDED(hr))
		{
			PLOG_INFO << "6: Succesfully created dummy swapchain";
			return;
		}
		else
		{
			logSwapChainFailure(std::format("6: failed to create dummy d3d device and swapchain, error: {:x}", (ULONG)hr));
		}
	}

	// use D3D_DRIVER_TYPE_SOFTWARE
	// does not work for me (E_INVALIDARG)
	{
		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_SOFTWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pDummySwapchain, &pDummyDevice, &featLevel, nullptr);
		if (SUCCEEDED(hr))
		{
			PLOG_INFO << "5: Succesfully created dummy swapchain";
			return;
		}
		else
		{
			logSwapChainFailure(std::format("5: failed to create dummy d3d device and swapchain, error: {:x}", (ULONG)hr));
		}
	}


	// the next call seems to need us to set the adapter parameter ourselves instead of using null ptr

#define UseDXGI1 TRUE

#if UseDXGI1 == TRUE
#define IDXGIFactoryA IDXGIFactory1
#define CreateDXGIFactoryA CreateDXGIFactory1
#define IDXGIAdapterA IDXGIAdapter1
#define EnumAdaptersA EnumAdapters1
#else
#define IDXGIFactoryA IDXGIFactory
#define CreateDXGIFactoryA CreateDXGIFactory
#define IDXGIAdapterA IDXGIAdapter
#define EnumAdaptersA EnumAdapters
#endif

	IDXGIFactoryA* pFactory;
	HRESULT fhr = CreateDXGIFactoryA(__uuidof(IDXGIFactoryA), (void**)(&pFactory));

	std::vector <IDXGIAdapterA*> vAdapters;
	if (fhr != S_OK)
	{
		PLOG_ERROR << "Failed to create DXGIFactory: " << std::hex << (ULONG)fhr;
	}
	else
	{
		UINT i = 0;
		IDXGIAdapterA* pAdapter;
		while (pFactory->EnumAdaptersA(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			vAdapters.push_back(pAdapter);
			++i;
		}
		PLOG_DEBUG << "default adapter: " << std::hex << vAdapters.front();
		PLOG_DEBUG << "adapter count: " << vAdapters.size();

	}

	// use D3D_DRIVER_TYPE_UNKNOWN
	// works if I pass it the adapter manually
	// tries each adapter
	for (auto adapter : vAdapters)
	{
		PLOG_INFO << "Attemtping D3D11CreateDeviceAndSwapChain with adapter: " << std::hex << adapter;

			HRESULT hr = D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pDummySwapchain, &pDummyDevice, &featLevel, nullptr);
			if (SUCCEEDED(hr))
			{
				PLOG_INFO << "4: Succesfully created dummy swapchain";
				return;
			}
			else
			{
				logSwapChainFailure(std::format("4: failed to create dummy d3d device and swapchain, error: {:x}", (ULONG)hr));
			}

	}



	// If execution got here, none of the createdevice calls succeeded. Throw an exception and log the error codes

	std::string resultsString;
	for (auto error : errorCodes)
	{
		resultsString += error + "\n";
	}
	throw InitException(resultsString);


}


D3D11Hook::D3D11Hook()
{

	if (instance != nullptr)
	{
		throw InitException("Cannot have more than one D3D11Hook");
	}
	instance = this;

	ID3D11Device* pDummyDevice = nullptr;
	IDXGISwapChain* pDummySwapchain = nullptr;

	// Create a dummy device
	CreateDummySwapchain(pDummySwapchain, pDummyDevice);

	// Get swapchain vmt
	void** pVMT = *(void***)pDummySwapchain;

	// Get Present's address out of vmt
	m_pOriginalPresent = (DX11Present*)pVMT[(UINT)IDXGISwapChainVMT::Present];
	m_ppPresent = (DX11Present**)&pVMT[(UINT)IDXGISwapChainVMT::Present];
	PLOG_INFO << "PRESENT: " << m_pOriginalPresent;
	PLOG_INFO << "PRESENT pointer: " << m_ppPresent;

	// Get resizeBuffers too
	m_pOriginalResizeBuffers = (DX11ResizeBuffers*)pVMT[(UINT)IDXGISwapChainVMT::ResizeBuffers];
	m_ppResizeBuffers = (DX11ResizeBuffers**)&pVMT[(UINT)IDXGISwapChainVMT::ResizeBuffers];
	PLOG_INFO << "RESIZEBUFFERS: " << m_pOriginalResizeBuffers;
	PLOG_INFO << "RESIZEBUFFERS pointer: " << m_ppResizeBuffers;

	// Don't need the dummy device anymore
	safe_release(pDummySwapchain);
	safe_release(pDummyDevice);

	PLOG_DEBUG << "rewriting present pointer";
	// Rewrite the present pointer to instead point to our newPresent
	// Need access tho!
	patch_pointer(m_ppPresent, (uintptr_t)& newDX11Present);
	// resizeBuffers too
	patch_pointer(m_ppResizeBuffers, (uintptr_t)&newDX11ResizeBuffers);





}





void D3D11Hook::initializeD3Ddevice(IDXGISwapChain* pSwapChain)
{

	PLOG_DEBUG << "Initializing D3Ddevice" << std::endl;

	if (!SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&m_pDevice)))
	{
		throw InitException(std::format("Failed to get D3D11Device, pSwapChain: {:x}", (uint64_t)pSwapChain).c_str());

	}

	// Get Device Context
	m_pDevice->GetImmediateContext(&m_pDeviceContext);
	if (!m_pDeviceContext)
	{
		throw InitException("Failed to get DeviceContext");
	}



	// Use backBuffer to get MainRenderTargetView
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (!pBackBuffer) throw InitException("Failed to get BackBuffer");

	m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRenderTargetView);
	pBackBuffer->Release();
	if (!m_pMainRenderTargetView) throw InitException("Failed to get MainRenderTargetView");

	// Store the windows size
	D3D11_VIEWPORT pViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{ 0 };
	UINT numViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	m_pDeviceContext->RSGetViewports(&numViewports, pViewports);
	PLOG_VERBOSE << "number of viewports: " << numViewports;

	mScreenSize = { (pViewports->Width), (pViewports->Height) };
	

}

// static 
HRESULT D3D11Hook::newDX11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	auto d3d = instance;
	
	std::scoped_lock<std::mutex> lock(d3d->mDestructionGuard); // Protects against D3D11Hook singleton destruction while hooks are executing


	if (!d3d->isD3DdeviceInitialized)
	{
		try
		{
			d3d->initializeD3Ddevice(pSwapChain);
			d3d->isD3DdeviceInitialized = true;
			PLOG_DEBUG << "D3D device initialized";
		}
		catch (InitException& ex)
		{
			PLOG_FATAL << "Failed to initialize d3d device, info: " << std::endl
				<< ex.what() << std::endl
				<< "CEER will now automatically close down";
			GlobalKill::killMe();

			// Call original present
			return d3d->m_pOriginalPresent(pSwapChain, SyncInterval, Flags);


		}
		
	}
   
	// Invoke the callback
	d3d->presentHookEvent(d3d->m_pDevice, d3d->m_pDeviceContext, pSwapChain, d3d->m_pMainRenderTargetView);

	// Call original present
	return d3d->m_pOriginalPresent(pSwapChain, SyncInterval, Flags);

}



// static
HRESULT D3D11Hook::newDX11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	auto d3d = instance;

	std::scoped_lock<std::mutex> lock(d3d->mDestructionGuard); // Protects against D3D11Hook singleton destruction while hooks are executing

	// Need to release mainRenderTargetView before calling ResizeBuffers
	if (d3d->m_pMainRenderTargetView != nullptr)
	{
		d3d->m_pDeviceContext->OMSetRenderTargets(0, 0, 0);
		d3d->m_pMainRenderTargetView->Release();
	}

	// Call original ResizeBuffers
	HRESULT hr = d3d->m_pOriginalResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);


	// Resetup the mainRenderTargetView
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (!pBackBuffer) throw InitException("Failed to get BackBuffer");

	d3d->m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &d3d->m_pMainRenderTargetView);
	pBackBuffer->Release();
	if (!d3d->m_pMainRenderTargetView) throw InitException("Failed to get MainRenderTargetView");

	// Grab the new windows size
	mScreenSize = { (float)Width, (float)Height };

	return hr;
}

// Safely destroys hooks
// Releases D3D resources, if we acquired them
D3D11Hook::~D3D11Hook()
{
// It's important that hooks are destroyed BEFORE the rest of the class is
// as the hook functions will try to access class members
// and also the d3d resources need to be manually released

	std::scoped_lock<std::mutex> lock(mDestructionGuard); // Hook functions lock this

	// Destroy the hooks
	// rewrite the pointers to go back to the original value
	patch_pointer(m_ppPresent, (uintptr_t)m_pOriginalPresent);
	// resizeBuffers too
	patch_pointer(m_ppResizeBuffers, (uintptr_t)m_pOriginalResizeBuffers);

	// D3D resource releasing:
	// need to call release on the device https://learn.microsoft.com/en-us/windows/win32/api/d3d9helper/nf-d3d9helper-idirect3dswapchain9-getdevice
	
	safe_release(m_pDevice);
	safe_release(m_pDeviceContext);
	safe_release(m_pMainRenderTargetView);
	instance = nullptr;
	
}