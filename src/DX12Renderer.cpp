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
// Portions of this file are from Microsoft reference code, licensed:
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "DX12Renderer.h"
#include "DXUtility.h"
#include "DXGIUtility.h"
#include "Strings.h"
#include <algorithm>

using Microsoft::WRL::ComPtr;

const std::string DX12Renderer::ConfigureError = "DirectX 12 is not supported. Either the installed version of TouchDesigner is too old, or the selected GPU does not have needed features.";

static inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
{
    if (path == nullptr)
    {
        throw std::exception();
    }

    DWORD size = GetModuleFileName(nullptr, path, pathSize);
    if (size == 0 || size == pathSize)
    {
        // Method failed or path was truncated.
        throw std::exception();
    }

    WCHAR* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = L'\0';
    }
}

DX12Renderer::DX12Renderer()
{
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    myAssetsPath = assetsPath;
}

DX12Renderer::~DX12Renderer()
{
    CloseHandle(myFenceEvent);
}

Graphics DX12Renderer::getMode() const
{
    return Graphics::DX12;
}

void DX12Renderer::setup(HWND window)
{
    Renderer::setup(window);
    UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
    ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
        debug->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    DXGIUtility utility;

    utility.setDX12();

    std::wstring description;
    ComPtr<IDXGIAdapter1> adapter = utility.getHardwareAdapter(factory.Get(), description, true);

    if (adapter.Get() == nullptr)
    {
        throw std::runtime_error("Couldn't find suitable graphics device.");
    }

    myAdapterDescription = ConvertToMultiByte(description);

    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice)));

    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ThrowIfFailed(myDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myCommandQueue)));
    }

    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.Width = 0;
        swapChainDesc.Height = 0;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(myCommandQueue.Get(), window, &swapChainDesc, nullptr, nullptr, &swapChain));
        
        factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

        swapChain.As(&mySwapChain);

        mySwapChain->GetDesc1(&swapChainDesc);

        myWidth = swapChainDesc.Width;
        myHeight = swapChainDesc.Height;

        myViewport = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>(myWidth), static_cast<float>(myHeight) );
        myScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(myWidth), static_cast<LONG>(myHeight));
    }

    myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();

    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(myDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&myRTVHeap)));
        
        myRTVDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(myRTVHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myRenderTargets[n])));
            
            myDevice->CreateRenderTargetView(myRenderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, myRTVDescriptorSize);
        }
    }

    ThrowIfFailed(myDevice->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&myFence)));

	myFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (myFenceEvent == nullptr)
	{
		throw std::runtime_error("CreateEvent failed.");
	}
    
    {
        HANDLE handle;
        ThrowIfFailed(myDevice->CreateSharedHandle(myFence.Get(), nullptr, GENERIC_ALL, nullptr, &handle));

        myTEFence.take(TED3DSharedFenceCreate(handle, nullptr, nullptr));
        
        // TouchEngine duplicates the handle, so close it now
        CloseHandle(handle);
    }
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(myDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1] = {
            CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC)
        };

        CD3DX12_ROOT_PARAMETER1 rootParameters[1] = { };
        rootParameters[0].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {
            .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            .AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            .AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
            .MipLODBias = 0,
            .MaxAnisotropy = 0,
            .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
            .BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
            .MinLOD = 0.0f,
            .MaxLOD = D3D12_FLOAT32_MAX,
            .ShaderRegister = 0,
            .RegisterSpace = 0,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        };

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(
            _countof(rootParameters),
            rootParameters,
            1,
            &sampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(myDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&myRootSignature)));
    }

    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(getAssetFullPath(L"dx12shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(getAssetFullPath(L"dx12shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Configure the blend state description
        CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = myRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = blendDesc;
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(myDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&myPipelineState)));
    }

    myCommandList.setup(myDevice.Get());

    myOutputImage = DX12Image(myDevice.Get());

    TEResult result = TED3D12ContextCreate(myDevice.Get(), myContext.take());
    if (result != TEResultSuccess)
    {
        throw std::runtime_error("Couldn't create TED3D12Context.");
    }
}

bool DX12Renderer::configure(TEInstance* instance, std::string & error)
{
    if (!Renderer::configure(instance, error))
    {
        return false;
    }

    myMinBufferAlignment = std::max<size_t>(myMinBufferAlignment, 16);

    int32_t count = 0;
    TEResult result = TEInstanceGetSupportedTextureTypes(instance, nullptr, &count);
    if (result == TEResultInsufficientMemory)
    {
        std::vector<TETextureType> textureTypes(count);
        result = TEInstanceGetSupportedTextureTypes(instance, textureTypes.data(), &count);
        if (result == TEResultSuccess)
        {
            textureTypes.resize(count);
            if (std::find(textureTypes.begin(), textureTypes.end(), TETextureTypeD3DShared) != textureTypes.end())
            {
                result = TEInstanceGetSupportedD3DHandleTypes(instance, nullptr, &count);
                if (result == TEResultInsufficientMemory)
                {
                    std::vector<TED3DHandleType> handleTypes(count);
                    result = TEInstanceGetSupportedD3DHandleTypes(instance, handleTypes.data(), &count);
                    if (result == TEResultSuccess)
                    {
                        handleTypes.resize(count);
                        if (std::find(handleTypes.begin(), handleTypes.end(), TED3DHandleTypeD3D12ResourceNT) == handleTypes.end())
                        {
                            error = getConfigureError();
                            return false;
                        }
                    }
                }
            }
        }
    }
    result = TEInstanceGetSupportedSemaphoreTypes(instance, nullptr, &count);
    if (result == TEResultInsufficientMemory)
    {
        std::vector<TESemaphoreType> semaphoreTypes(count);
        result = TEInstanceGetSupportedSemaphoreTypes(instance, semaphoreTypes.data(), &count);
        if (result == TEResultSuccess)
        {
            semaphoreTypes.resize(count);
            if (std::find(semaphoreTypes.begin(), semaphoreTypes.end(), TESemaphoreTypeD3DFence) == semaphoreTypes.end())
            {
                error = getConfigureError();
                return false;
            }
        }
    }
    return true;
}

bool DX12Renderer::doesInputResourceTransfer() const
{
    return true;
}

void DX12Renderer::resize(int width, int height)
{
    if (width != myWidth || height != myHeight)
    {
        signal();
        waitForGPU();

        for (UINT n = 0; n < FrameCount; n++)
        {
            myRenderTargets[n].Reset();
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        mySwapChain->GetDesc(&swapChainDesc);
        mySwapChain->ResizeBuffers(FrameCount, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);

        myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(myRTVHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT n = 0; n < FrameCount; n++)
        {
            mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myRenderTargets[n]));
            myDevice->CreateRenderTargetView(myRenderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, myRTVDescriptorSize);
        }
        myWidth = width;
        myHeight = height;

        myViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(myWidth), static_cast<float>(myHeight));
        myScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(myWidth), static_cast<LONG>(myHeight));
    }
}

void DX12Renderer::stop()
{
    Renderer::stop();    // Do this first because it will cause our texture release callback to be invoked
    signal();
    waitForGPU();
}

bool DX12Renderer::render()
{
    // Do any pending buffer copy
    copyBuffer();

    // Render
    populateRenderCommandList();

    ID3D12CommandList* ppCommandLists[] = { myCommandList };
    myCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Signal for our own use and for the resource transfers we've added for TouchEngine
    signal();

    mySwapChain->Present(1, 0);

    waitForGPU();

    myCommandList.completed();

    return true;
}

void DX12Renderer::copyBuffer()
{
    if (myInputBufferCopyOffset < myInputBufferUsedOffset)
    {
        if (!myCommandList.isRecording())
        {
            myCommandList.reset(myPipelineState.Get());
        }

        myCommandList.get()->CopyResource(myInputSharedBuffer, myInputUploadBuffer);
        myCommandList.get()->CopyBufferRegion(myInputSharedBuffer, myInputBufferCopyOffset, myInputUploadBuffer, myInputBufferCopyOffset, myInputBufferUsedOffset - myInputBufferCopyOffset);

        myCommandList.usingResource(myInputSharedBuffer);
        myCommandList.usingResource(myInputUploadBuffer);

        myCommandList.barrier(myInputSharedBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);

        myInputBufferCopyOffset = myInputBufferUsedOffset;
    }
}

TouchObject<TETexture>	DX12Renderer::getTexture(const unsigned char* rgba, size_t bytesPerRow, int width, int height)
{
    if (!myCommandList.isRecording())
    {
        myCommandList.reset(myPipelineState.Get());
    }

    DX12Texture inputTex = DX12Texture(myDevice.Get(), myCommandList, rgba, bytesPerRow, width, height);

    TouchObject<TETexture> texture;
    {
        HANDLE handle;
        ThrowIfFailed(myDevice->CreateSharedHandle(inputTex, nullptr, GENERIC_ALL, nullptr, &handle));

        TouchObject<TED3DAllocation> allocation;
        allocation.take(TED3DAllocationCreate(handle, TED3DHandleTypeD3D12ResourceNT, 0, nullptr, nullptr));
        texture.take(TED3DSharedTextureCreate(allocation, 0, DX12Texture::Format, width, height, TETextureOriginTopLeft, kTETextureComponentMapIdentity, nullptr, nullptr));

        // TouchEngine duplicates it for its own use
        CloseHandle(handle);
    }

    // Add a resource transfer so TouchEngine will wait for the upload to complete on the GPU before using the texture
    addResourceTransfer(texture, myTEFence, getNextFenceValue());

    return texture;
}

void DX12Renderer::clearInputs()
{
    waitForGPU();
    myInputSharedAllocation.reset();
    Renderer::clearInputs();
}

bool DX12Renderer::setOutputImage(const TouchObject<TETexture>& texture, const TouchObject<TESemaphore>& semaphore, uint64_t waitValue)
{
    setOutputImage(texture);

	if (semaphore)
	{
        if (TESemaphoreGetType(semaphore) != TESemaphoreTypeD3DFence)
        {
            throw std::runtime_error("Unexpected semaphore type");
        }
		HANDLE handle = TED3DSharedFenceGetHandle(static_cast<TED3DSharedFence*>(semaphore.get()));
        // lock because myOutputFences may be accessed from fenceCallback on another thread
        std::lock_guard<std::mutex> guard(myOutputsLock);
		auto it = myOutputFences.find(handle);
		if (it == myOutputFences.end())
		{
			// We cache output fences -
			// TouchEngine's callbacks allow us to delete our cached fence when the original is deleted

			ComPtr<ID3D12Fence> fence;

			ThrowIfFailed(myDevice->OpenSharedHandle(handle, IID_PPV_ARGS(&fence)));

			it = myOutputFences.insert(std::make_pair(handle, fence)).first;

			TED3DSharedFenceSetCallback(static_cast<TED3DSharedFence*>(semaphore.get()), fenceCallback, this);
		}

        // We must wait before using the texture on the GPU in the next frame
		myCommandQueue->Wait(it->second.Get(), waitValue);
	}
	
    return true;
}

void DX12Renderer::setOutputImage(const TouchObject<TETexture>& texture)
{
    const auto& previous = getOutputImage();
    if (previous)
    {
        // We can use the value from the previous frame here.
        // Strictly speaking there is no need for this for our simple renderer,
        // but it would be required if we didn't wait for the GPU after presenting
        // the current frame
        addResourceTransfer(previous, myTEFence, mySignalledFenceValue);
    }

    Renderer::setOutputImage(texture);

    if (texture && TETextureGetType(texture) == TETextureTypeD3DShared)
    {
        TED3DSharedTexture* shared = static_cast<TED3DSharedTexture*>(texture.get());
        TouchObject<TED3DAllocation> allocation;
        allocation.take(TED3DSharedTextureGetAllocation(shared));
        HANDLE h = TED3DAllocationGetHandle(allocation);
        TED3DHandleType type = TED3DAllocationGetHandleType(allocation);
        if (type != TED3DHandleTypeD3D12ResourceNT)
        {
            throw std::runtime_error("Unexpected D3D handle type");
        }

        // lock because myOutputTextures may be accessed from textureCallback on another thread
        std::lock_guard<std::mutex> guard(myOutputsLock);
        auto it = myOutputTextures.find(h);
        if (it == myOutputTextures.end())
        {
            // We cache output textures because TouchEngine will recycle them -
            // TouchEngine's callbacks allow us to delete our cached texture when the original is deleted
            it = myOutputTextures.insert(std::make_pair(h, DX12Texture(myDevice.Get(), h))).first;

            TED3DSharedTextureSetCallback(shared, textureCallback, this);
        }
        myOutputImage.update(myDevice.Get(), it->second);
    }
    else
    {
        myOutputImage.update(myDevice.Get(), DX12Texture());
    }
}

void DX12Renderer::clearOutputs()
{
    waitForGPU();
    setOutputImage(nullptr);
    std::lock_guard<std::mutex> guard(myOutputsLock);
    myOutputTextures.clear();
    myOutputFences.clear();
    Renderer::clearOutputs();
}

TouchObject<TEGraphicsContext> DX12Renderer::getTEContext() const
{
    return myContext;
}

const std::string& DX12Renderer::getDeviceName() const
{
    return myAdapterDescription;
}

TouchObject<TEBuffer> DX12Renderer::getDeviceBuffer(const void* src, size_t size)
{    
    // round so our sub-allocations are always aligned
    size = alignedBufferSize(size);

    willAllocateBuffer(size);

    auto buffer = TouchObject<TED3DSharedBuffer>::make_take(TED3DSharedBufferCreate(myInputSharedAllocation, myInputBufferUsedOffset, size, nullptr, nullptr));

    memcpy(static_cast<uint8_t*>(myInputUploadBuffer.data()) + myInputBufferUsedOffset, src, size);
    myInputBufferUsedOffset += size;

    // Add a resource transfer so TouchEngine will wait for the upload to complete on the GPU before using the buffer
    addResourceTransfer(buffer, myTEFence, getNextFenceValue());

    return buffer;
}

void DX12Renderer::waitForGPU()
{
    if (myCompletedFenceValue < mySignalledFenceValue)
    {
        myFence->SetEventOnCompletion(mySignalledFenceValue, myFenceEvent);

        WaitForSingleObjectEx(myFenceEvent, INFINITE, FALSE);

        myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();

        myCompletedFenceValue = mySignalledFenceValue;
    }
}

void DX12Renderer::populateRenderCommandList()
{
    if (!myCommandList.isRecording())
    {
        myCommandList.reset(myPipelineState.Get());
    }

    myCommandList.get()->RSSetViewports(1, &myViewport);
    myCommandList.get()->RSSetScissorRects(1, &myScissorRect);

    myCommandList.usingResource(myRenderTargets[myFrameIndex]);

    myCommandList.barrier(myRenderTargets[myFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(myRTVHeap->GetCPUDescriptorHandleForHeapStart(), myFrameIndex, myRTVDescriptorSize);
    myCommandList.get()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { myBackgroundColor.red, myBackgroundColor.green, myBackgroundColor.blue, myBackgroundColor.alpha};
    myCommandList.get()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    myCommandList.get()->SetGraphicsRootSignature(myRootSignature.Get());
    float blendFactors[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    myCommandList.get()->OMSetBlendFactor(blendFactors);

    if (myOutputImage.getTexture().isValid())
    {
        myOutputImage.fit(static_cast<float>(myWidth), static_cast<float>(myHeight));
        myOutputImage.position(0.0f, 0.0f);
        myOutputImage.draw(myCommandList);
    }

    myCommandList.barrier(myRenderTargets[myFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	myCommandList.close();
}

std::wstring DX12Renderer::getAssetFullPath(LPCWSTR assetName) const
{
    return myAssetsPath + assetName;
}

void DX12Renderer::textureCallback(TED3DAllocation *allocation, size_t offset, TEObjectEvent event, void* TE_NULLABLE info)
{
    if (event == TEObjectEventRelease)
    {
        HANDLE handle = TED3DAllocationGetHandle(allocation);
        
        DX12Renderer* renderer = static_cast<DX12Renderer*>(info);
        std::lock_guard<std::mutex> guard(renderer->myOutputsLock);
        renderer->myOutputTextures.erase(handle);
    }
}

void DX12Renderer::fenceCallback(HANDLE handle, TEObjectEvent event, void* TE_NULLABLE info)
{
    if (event == TEObjectEventRelease)
    {
        DX12Renderer* renderer = static_cast<DX12Renderer*>(info);
        std::lock_guard<std::mutex> guard(renderer->myOutputsLock);
        renderer->myOutputFences.erase(handle);
    }
}

std::string DX12Renderer::getConfigureError() const
{
    std::string composed = ConfigureError;
    if (!myAdapterDescription.empty())
    {
        composed += "\nThe selected GPU is: ";
        composed += myAdapterDescription;
    }
    return composed;
}

void DX12Renderer::willAllocateBuffer(size_t size)
{
    if (myInputSharedBuffer.getSize() - myInputBufferUsedOffset < size)
    {
        size_t allocSize = std::max<size_t>(1024 * 200, size);
        myInputSharedBuffer = DX12SharedBuffer(myDevice.Get(), allocSize);
        myInputSharedAllocation.take(TED3DAllocationCreate(myInputSharedBuffer.getSharedHandle(), TED3DHandleType12CommittedNT, myInputSharedBuffer.getSize(), nullptr, nullptr));
        myInputUploadBuffer = DX12UploadBuffer(myDevice.Get(), myInputSharedBuffer.getRequiredUploadSize());
        myInputBufferUsedOffset = 0;
        myInputBufferCopyOffset = 0;
    }
}

uint64_t DX12Renderer::getNextFenceValue() const
{
    return mySignalledFenceValue + 1;
}

void DX12Renderer::signal()
{
    uint64_t next = getNextFenceValue();
    myCommandQueue->Signal(myFence.Get(), next);
    mySignalledFenceValue = next;
}
