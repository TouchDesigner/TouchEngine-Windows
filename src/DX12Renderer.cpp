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
#include "DX12Utility.h"
#include "DXGIUtility.h"

using Microsoft::WRL::ComPtr;

const std::wstring DX12Renderer::ConfigureError = L"DirectX 12 is not supported. Either the installed version of TouchDesigner is too old, or the selected GPU does not have needed features.";

inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
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
    // Do this now because it will cause our texture release callback to be invoked
    clearInputImages();
    clearOutputImages();
}

bool DX12Renderer::setup(HWND window)
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

    ComPtr<IDXGIAdapter1> adapter = utility.getHardwareAdapter(factory.Get(), myAdapterDescription, true);

    if (adapter.Get() == nullptr)
    {
        return false;
    }

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

    ThrowIfFailed(myDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&myCommandAllocator)));
    
    ThrowIfFailed(myDevice->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&myFence)));

	myFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (myFenceEvent == nullptr)
	{
		return false;
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

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = myRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(myDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&myPipelineState)));
    }

    ThrowIfFailed(myDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, myCommandAllocator.Get(), myPipelineState.Get(), IID_PPV_ARGS(&myCommandList)));

    ThrowIfFailed(myCommandList->Close());

    TEResult result = TED3D12ContextCreate(myDevice.Get(), myContext.take());
    if (result != TEResultSuccess)
    {
        return false;
    }

    return true;
}

bool DX12Renderer::configure(TEInstance* instance, std::wstring & error)
{
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

bool DX12Renderer::doesInputTextureTransfer() const
{
    return true;
}

void DX12Renderer::resize(int width, int height)
{
    if (width != myWidth || height != myHeight)
    {
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
    CloseHandle(myFenceEvent);
}

bool DX12Renderer::render()
{
    populateRenderCommandList();

    executeCommandList();

    mySwapChain->Present(1, 0);

    waitForGPU();

    return true;
}

void DX12Renderer::executeCommandList()
{
    ID3D12CommandList* ppCommandLists[] = { myCommandList.Get() };
    myCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    myNextFenceValue++;
}

size_t DX12Renderer::getInputImageCount() const
{
    return myInputImages.size();
}

void DX12Renderer::beginImageLayout()
{
    beginCommandList(nullptr);
}

void DX12Renderer::addInputImage(const unsigned char* rgba, size_t bytesPerRow, int width, int height)
{
	myInputImages.emplace_back(DX12Texture(myDevice.Get(), myCommandList.Get(), rgba, bytesPerRow, width, height));
    Renderer::addInputImage(rgba, bytesPerRow, width, height);
}

bool DX12Renderer::getInputImage(size_t index, TouchObject<TETexture> & texture, TouchObject<TESemaphore> & semaphore, uint64_t & waitValue)
{
    if (inputDidChange(index))
    {
        texture.set(myInputImages[index].getTexture().getTETexture());
        semaphore = myTEFence;
        waitValue = myInputUpdateFenceValue;

        markInputUnchanged(index);
        return true;
    }
    return false;
}

void DX12Renderer::clearInputImages()
{
    waitForGPU();
    myInputImages.clear();
    Renderer::clearInputImages();
}

void DX12Renderer::addOutputImage()
{
    myOutputImages.emplace_back(myDevice.Get());
    Renderer::addOutputImage();
}

void DX12Renderer::endImageLayout()
{
    myCommandList->Close();

    executeCommandList();

    myInputUpdateFenceValue = myNextFenceValue;

    // For simplicity we wait here so we can dispose of associated resources at this point
    waitForGPU();

    for (auto& next : myInputImages)
    {
        next.getTexture().uploadDidComplete();
    }
}

bool DX12Renderer::updateOutputImage(const TouchObject<TEInstance>& instance, size_t index, const std::string& identifier)
{
    bool success = false;
    const auto& previous = getOutputImage(index);
    TEResult result = TEResultSuccess;
    if (previous)
    {
        result = TEInstanceAddTextureTransfer(instance, previous, myTEFence, myCompletedFenceValue);
    }
    TouchObject<TETexture> texture;

    if (result == TEResultSuccess)
    {
        result = TEInstanceLinkGetTextureValue(instance, identifier.c_str(), TELinkValueCurrent, texture.take());
    }
    if (result == TEResultSuccess)
    {
        setOutputImage(index, texture);

        if (texture && TETextureGetType(texture) == TETextureTypeD3DShared)
        {
            TED3DSharedTexture* shared = static_cast<TED3DSharedTexture*>(texture.get());
            HANDLE h = TED3DSharedTextureGetHandle(shared);
            auto it = myOutputTextures.find(h);
            if (it == myOutputTextures.end())
            {
                // We cache output textures because TouchEngine will recycle them -
                // TouchEngine's callbacks allow us to delete our cached texture when the original is deleted
                it = myOutputTextures.insert(std::make_pair(h, DX12Texture(myDevice.Get(), shared))).first;

                TED3DSharedTextureSetCallback(shared, textureCallback, this);
            }
            myOutputImages[index].update(it->second);
            success = true;

            if (texture && TEInstanceHasTextureTransfer(instance, texture))
            {
                TouchObject<TESemaphore> semaphore;
                uint64_t waitValue = 0;
                result = TEInstanceGetTextureTransfer(instance, texture, semaphore.take(), &waitValue);

                if (result == TEResultSuccess)
                {
                    if (TESemaphoreGetType(semaphore) == TESemaphoreTypeD3DFence)
                    {
                        HANDLE handle = TED3DSharedFenceGetHandle(static_cast<TED3DSharedFence*>(semaphore.get()));
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

                        myCommandQueue->Wait(it->second.Get(), waitValue);
                    }
                }
            }
        }
        else
        {
            success = false;
        }
    }
    if (!success)
    {
        myOutputImages.at(index).update(DX12Texture());
        setOutputImage(index, nullptr);
        if (!texture)
        {
            // Having no texture is OK
            success = true;
        }
    }
    return success;
}

void DX12Renderer::clearOutputImages()
{
    waitForGPU();
    myOutputImages.clear();
    Renderer::clearOutputImages();
}

TEGraphicsContext* DX12Renderer::getTEContext() const
{
    return myContext;
}

const std::wstring& DX12Renderer::getDeviceName() const
{
    return myAdapterDescription;
}

void DX12Renderer::waitForGPU()
{
    if (myCompletedFenceValue < myNextFenceValue)
    {
        myCommandQueue->Signal(myFence.Get(), myNextFenceValue);

        myFence->SetEventOnCompletion(myNextFenceValue, myFenceEvent);

        WaitForSingleObjectEx(myFenceEvent, INFINITE, FALSE);

        myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();

        myCompletedFenceValue = myNextFenceValue;
        myNextFenceValue++;
    }
}

void DX12Renderer::beginCommandList(ID3D12PipelineState* state)
{
    ThrowIfFailed(myCommandAllocator->Reset());

    ThrowIfFailed(myCommandList->Reset(myCommandAllocator.Get(), state));
}

void DX12Renderer::populateRenderCommandList()
{
    beginCommandList(myPipelineState.Get());

    myCommandList->RSSetViewports(1, &myViewport);
    myCommandList->RSSetScissorRects(1, &myScissorRect);

    CD3DX12_RESOURCE_BARRIER transition(CD3DX12_RESOURCE_BARRIER::Transition(myRenderTargets[myFrameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET));
    myCommandList->ResourceBarrier(1, &transition);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(myRTVHeap->GetCPUDescriptorHandleForHeapStart(), myFrameIndex, myRTVDescriptorSize);
    myCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { myBackgroundColor[0], myBackgroundColor[1], myBackgroundColor[2], 1.0f };
    myCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    auto longest = max(myInputImages.size(), myOutputImages.size());
    float scale = 1.0f / (longest + 1.0f);

    if (longest > 0)
    {
        myCommandList->SetGraphicsRootSignature(myRootSignature.Get());

        drawImages(myInputImages, scale, -0.5f);
        drawImages(myOutputImages, scale, 0.5f);
    }

    CD3DX12_RESOURCE_BARRIER transition2(CD3DX12_RESOURCE_BARRIER::Transition(myRenderTargets[myFrameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT));
	myCommandList->ResourceBarrier(1, &transition2);

	myCommandList->Close();
}

std::wstring DX12Renderer::getAssetFullPath(LPCWSTR assetName) const
{
    return myAssetsPath + assetName;
}

void DX12Renderer::drawImages(std::vector<DX12Image>& images, float scale, float xOffset)
{
    float numImages = (1.0f / scale) - 1.0f;
    float spacing = 1.0f / numImages;
    float yOffset = 1.0f - spacing;
    float ratio = static_cast<float>(myHeight) / myWidth;
    for (auto& image : images)
    {
        image.scale(scale * ratio, scale);
        image.position(xOffset, yOffset);
        image.draw(myCommandList.Get());
        yOffset -= spacing * 2;
    }
}

void DX12Renderer::textureCallback(HANDLE handle, TEObjectEvent event, void* TE_NULLABLE info)
{
    if (event == TEObjectEventRelease)
    {
        DX12Renderer* renderer = static_cast<DX12Renderer*>(info);
        renderer->myOutputTextures.erase(handle);
    }
}

void DX12Renderer::fenceCallback(HANDLE handle, TEObjectEvent event, void* TE_NULLABLE info)
{
    if (event == TEObjectEventRelease)
    {
        DX12Renderer* renderer = static_cast<DX12Renderer*>(info);
        renderer->myOutputFences.erase(handle);
    }
}

std::wstring DX12Renderer::getConfigureError() const
{
    std::wstring composed = ConfigureError;
    if (!myAdapterDescription.empty())
    {
        composed += L"\nThe selected GPU is: ";
        composed += myAdapterDescription;
    }
    return composed;
}
