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
#include "DXUtility.h"
#include "DX12CommandList.h"

DX12CommandList::DX12CommandList(D3D12_COMMAND_LIST_TYPE type)
	: myType(type)
{

}

D3D12_COMMAND_LIST_TYPE DX12CommandList::getType() const
{
	return myType;
}

void DX12CommandList::setup(ID3D12Device* device)
{
	ThrowIfFailed(device->CreateCommandAllocator(myType, IID_PPV_ARGS(&myCommandAllocator)));
	ThrowIfFailed(device->CreateCommandList(0, myType, myCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&myCommandList)));
	ThrowIfFailed(myCommandList->Close());
}

void DX12CommandList::reset(ID3D12PipelineState *state)
{
	myRecording = true;
	ThrowIfFailed(myCommandAllocator->Reset());
	ThrowIfFailed(myCommandList->Reset(myCommandAllocator.Get(), state));
}

void DX12CommandList::barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	CD3DX12_RESOURCE_BARRIER transition(CD3DX12_RESOURCE_BARRIER::Transition(resource,
		before,
		after));
	myCommandList->ResourceBarrier(1, &transition);
}

void DX12CommandList::close()
{
	myRecording = false;
	ThrowIfFailed(myCommandList->Close());
}

void DX12CommandList::completed()
{
	myActiveResources.clear();
}

void DX12CommandList::usingResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource)
{
	myActiveResources.insert(resource);
}

void DX12CommandList::usingResource(ID3D12Resource* resource)
{
	myActiveResources.insert(resource);
}

bool DX12CommandList::isRecording() const
{
	return myRecording;
}
