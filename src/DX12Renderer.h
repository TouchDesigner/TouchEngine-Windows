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
#include "DX12CommandList.h"
#include "DX12Image.h"
#include "DX12UploadBuffer.h"
#include "DX12SharedBuffer.h"
#include <TouchEngine/TED3D12.h>
#include <DirectXMath.h>
#include <map>
#include <string>
#include <mutex>

class DX12Renderer :
    public Renderer
{
public:
						DX12Renderer();
	virtual				~DX12Renderer();
	virtual Graphics	getMode() const override;
	virtual void		setup(HWND window) override;
	virtual bool		configure(TEInstance* instance, std::string & error) override;
	virtual bool		doesInputResourceTransfer() const override;
	virtual void		resize(int width, int height) override;
	virtual void		stop() override;
	virtual bool		render() override;

	void copyBuffer();

	virtual TouchObject<TETexture>	getTexture(const unsigned char* rgba, size_t bytesPerRow, int width, int height) override;
	virtual void		clearInputs() override;

	virtual bool		setOutputImage(const TouchObject<TETexture>& texture, const TouchObject<TESemaphore>& semaphore, uint64_t waitValue) override;
	virtual void		setOutputImage(const TouchObject<TETexture>& texture) override;
	virtual void		clearOutputs() override;
	virtual TouchObject<TEGraphicsContext> getTEContext() const override;

	virtual const std::string& getDeviceName() const override;

protected:
	virtual TouchObject<TEBuffer> getDeviceBuffer(const void* src, size_t size) override;
private:
	static const UINT FrameCount = 2;
	void				waitForGPU();
	void				populateRenderCommandList();
	std::wstring		getAssetFullPath(LPCWSTR assetName) const;
	static void			textureCallback(TED3DAllocation *allocation, size_t offset, TEObjectEvent event, void* TE_NULLABLE info);
	static void			fenceCallback(HANDLE handle, TEObjectEvent event, void* TE_NULLABLE info);
	std::string			getConfigureError() const;
	void				willAllocateBuffer(size_t size);
	uint64_t			getNextFenceValue() const;
	void				signal();

	static const std::string ConfigureError;

	std::wstring myAssetsPath;

	CD3DX12_VIEWPORT myViewport;
	CD3DX12_RECT myScissorRect;
	Microsoft::WRL::ComPtr<ID3D12Device4> myDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> myCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> mySwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> myRenderTargets[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> myPipelineState;

	DX12CommandList myCommandList = DX12CommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	std::string myAdapterDescription;

	UINT myRTVDescriptorSize = 0;

	HANDLE myFenceEvent = INVALID_HANDLE_VALUE;
	Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
	TouchObject<TED3DSharedFence> myTEFence;
	uint64_t mySignalledFenceValue{ 0 };
	uint64_t myCompletedFenceValue{ 0 };

	TouchObject<TED3D12Context> myContext;

	UINT myFrameIndex = 0;

	DX12Image	myOutputImage;
	std::mutex	myOutputsLock;
	std::map<HANDLE, DX12Texture> myOutputTextures;
	std::map<HANDLE, Microsoft::WRL::ComPtr<ID3D12Fence>> myOutputFences;

	DX12UploadBuffer myInputUploadBuffer;
	DX12SharedBuffer myInputSharedBuffer;
	TouchObject<TED3DAllocation> myInputSharedAllocation;
	size_t myInputBufferUsedOffset = 0;
	size_t myInputBufferCopyOffset = 0;
};

