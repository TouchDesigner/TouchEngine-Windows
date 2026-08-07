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
#include <d3d12.h>
#include <set>

class DX12CommandList
{
public:
	DX12CommandList(D3D12_COMMAND_LIST_TYPE type);
	D3D12_COMMAND_LIST_TYPE getType() const;
	void setup(ID3D12Device* device);
	void reset(ID3D12PipelineState *state);
	void barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void close();
	void completed();
	operator ID3D12GraphicsCommandList* () const
	{
		return get();
	}
	ID3D12GraphicsCommandList* get() const
	{
		return myCommandList.Get();
	}
	void usingResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource);
	void usingResource(ID3D12Resource* resource);
	bool isRecording() const;
private:
	D3D12_COMMAND_LIST_TYPE								myType;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		myCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	myCommandList;
	std::set<Microsoft::WRL::ComPtr<ID3D12Resource>>	myActiveResources;
	bool												myRecording = false;
};

