#include "GPUResource.h"

#include "D3DContext.h"
#include "Floodlight/Utilities/Assert.h"

namespace Floodlight {

    GPUResource::GPUResource(ID3D12Resource* Res, D3D12_RESOURCE_STATES InitialState)
    {
		Resource = Res;
		State = InitialState;
    }

    GPUResource::GPUResource(const D3D12_HEAP_PROPERTIES& HeapProps, const D3D12_RESOURCE_DESC& Desc, D3D12_RESOURCE_STATES InitialState)
	{
		D3DContext::GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &Desc, InitialState, nullptr, IID_PPV_ARGS(&Resource));
		State = InitialState;
	}

	GPUResource::~GPUResource()
	{
		Resource->Release();
	}

	void GPUResource::TransitionState(D3D12_RESOURCE_STATES TargetState)
	{
		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = Resource;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Barrier.Transition.StateBefore = State;
		Barrier.Transition.StateAfter = TargetState;
		if (Barrier.Transition.StateBefore != Barrier.Transition.StateAfter) {
			D3DContext::GetCommandList().Get()->ResourceBarrier(1, &Barrier);
			State = TargetState;
		}
	}

	void GPUResource::IncrementRef()
	{
		References++;
	}

	void GPUResource::DecrementRef()
	{
		FL_Assert(References > 0, "Cannot decrement this reference count; there are no more references.");
		References--;
		if (References == 0)
		{
			delete this;
		}
	}

}