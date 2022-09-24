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

#pragma once
#include "Renderer.h"
#include "DX12Image.h"
#include <TouchEngine/TED3D12.h>
#include <DirectXMath.h>
#include <map>

class DX12Renderer :
    public Renderer
{
public:
						DX12Renderer();
	virtual				~DX12Renderer();
	virtual bool		setup(HWND window) override;
	virtual bool		doesTextureTransfer() const override;
	virtual void		resize(int width, int height) override;
	virtual void		stop() override;
	virtual bool		render() override;

	void executeCommandList();

	virtual size_t		getInputImageCount() const override;

	virtual void		beginImageLayout() override;
	virtual void		addInputImage(const unsigned char* rgba, size_t bytesPerRow, int width, int height) override;
	virtual bool		getInputImage(size_t index, TouchObject<TETexture>& texture, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue) override;
	virtual void		clearInputImages() override;
	virtual void		addOutputImage() override;
	virtual void		endImageLayout() override;

	virtual bool		releaseOutputImage(size_t index, TouchObject<TETexture>& texture, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue) override;
	virtual void		setOutputImage(size_t index, const TouchObject<TETexture>& texture) override;
	virtual void		acquireOutputImage(size_t index, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue) override;
	
	virtual void		clearOutputImages() override;
	virtual TEGraphicsContext* getTEContext() const override;
private:
	static const UINT FrameCount = 2;
	void				getHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
	void				waitForGPU();
	void				beginCommandList(ID3D12PipelineState* state);
	void				populateRenderCommandList();
	std::wstring		getAssetFullPath(LPCWSTR assetName) const;
	void				drawImages(std::vector<DX12Image>& images, float scale, float xOffset);
	static void			textureCallback(HANDLE handle, TEObjectEvent event, void* TE_NULLABLE info);
	static void			fenceCallback(HANDLE handle, TEObjectEvent event, void* TE_NULLABLE info);

	std::wstring myAssetsPath;

	CD3DX12_VIEWPORT myViewport;
	CD3DX12_RECT myScissorRect;
	Microsoft::WRL::ComPtr<ID3D12Device> myDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> myCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> myCommandAllocator;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> mySwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> myRenderTargets[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> myCommandList;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> myPipelineState;

	UINT myRTVDescriptorSize;

	HANDLE myFenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
	TouchObject<TED3DSharedFence> myTEFence;
	UINT64 myNextFenceValue{ 1 };
	UINT64 myCompletedFenceValue{ 0 };
	UINT64 myInputUpdateFenceValue{ 0 };

	TouchObject<TED3D12Context> myContext;

	UINT myFrameIndex = 0;
	int myWidth;
	int myHeight;

	std::vector<DX12Image> myInputImages;
	std::vector<DX12Image> myOutputImages;
	std::map<HANDLE, DX12Texture> myOutputTextures;
	std::map<HANDLE, Microsoft::WRL::ComPtr<ID3D12Fence>> myOutputFences;
};

