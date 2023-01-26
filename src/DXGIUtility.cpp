/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include "DXGIUtility.h"

using Microsoft::WRL::ComPtr;

Microsoft::WRL::ComPtr<IDXGIAdapter1> DXGIUtility::getHardwareAdapter(IDXGIFactory1* pFactory, std::wstring& description, bool requestHighPerformanceAdapter)
{
    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D, but don't create the
            // actual device yet.
            if (SUCCEEDED(test(adapter.Get())))
            {
                description = desc.Description;
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D, but don't create the
            // actual device yet.
            if (SUCCEEDED(test(adapter.Get())))
            {
                description = desc.Description;
                break;
            }
        }
    }

    return adapter;
}

void DXGIUtility::setDX11()
{
    myIs12 = false;
}

void DXGIUtility::setDX12()
{
    myIs12 = true;
}

HRESULT DXGIUtility::test(IDXGIAdapter1* adapter)
{
    if (myIs12)
    {
        return D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
    }
    else
    {
		D3D_FEATURE_LEVEL levels[] = {
	        D3D_FEATURE_LEVEL_11_1,
	        D3D_FEATURE_LEVEL_11_0,
	        D3D_FEATURE_LEVEL_10_1,
	        D3D_FEATURE_LEVEL_10_0
		};

		HRESULT hr = D3D11CreateDevice(adapter,
            D3D_DRIVER_TYPE_UNKNOWN,
			0,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			levels,
			ARRAYSIZE(levels),
			D3D11_SDK_VERSION,
			nullptr,
			nullptr,
			nullptr);
        return hr;
    }
}
