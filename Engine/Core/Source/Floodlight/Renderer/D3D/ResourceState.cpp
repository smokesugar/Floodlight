#include "ResourceState.h"

#include <unordered_map>

#include "Floodlight/Utilities/Keywords.h"
#include "D3DContext.h"

namespace Floodlight {

	confined std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> ResourceStates;

	D3D12_RESOURCE_STATES& 
	GetResourceState(ID3D12Resource* Resource)
	{
		return ResourceStates[Resource];
	}

	/*
		Create a ResourceBarrier to transition a resource.
	*/
    void
	TransitionResourceState(ID3D12Resource* Resource, D3D12_RESOURCE_STATES After)
    {
		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = Resource;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Barrier.Transition.StateBefore = GetResourceState(Resource);
		Barrier.Transition.StateAfter = After;
		if (Barrier.Transition.StateBefore != Barrier.Transition.StateAfter) {
			D3DContext::GetCommandList().Get()->ResourceBarrier(1, &Barrier);
			GetResourceState(Resource) = After;
		}
    }

}