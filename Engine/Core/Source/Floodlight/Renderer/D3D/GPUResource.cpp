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

	/*
		Helper function that creates a default heap properties struct and initializes it with a type.
	*/
	D3D12_HEAP_PROPERTIES
	CreateHeapProperties(D3D12_HEAP_TYPE Type)
	{
		D3D12_HEAP_PROPERTIES HeapProps = {};
		HeapProps.Type = Type;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;
		return HeapProps;
	}

	/*
		Helper function that creates a buffer resource desc with a specific size in bytes.
	*/
	D3D12_RESOURCE_DESC
	CreateBufferResourceDesc(uint32 SizeBytes)
	{
		D3D12_RESOURCE_DESC ResourceDesc = {};
		ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ResourceDesc.Alignment = 0;
		ResourceDesc.Width = SizeBytes;
		ResourceDesc.Height = 1;
		ResourceDesc.DepthOrArraySize = 1;
		ResourceDesc.MipLevels = 1;
		ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		ResourceDesc.SampleDesc = { 1, 0 };
		ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		return ResourceDesc;
	}
	
	/*
		Helper function that creates a texture2d resource desc with a specific information;
	*/
	D3D12_RESOURCE_DESC
	CreateTexture2DResourceDesc(uint32 Width, uint32 Height, DXGI_FORMAT Format, D3D12_RESOURCE_FLAGS Flags)
	{
		D3D12_RESOURCE_DESC ResDesc = {};
		ResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		ResDesc.Alignment = 0;
		ResDesc.Width = Width;
		ResDesc.Height = Height;
		ResDesc.DepthOrArraySize = 1;
		ResDesc.MipLevels = 1;
		ResDesc.Format = Format;
		ResDesc.SampleDesc = { 1, 0 };
		ResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		ResDesc.Flags = Flags;
		return ResDesc;
	}

	/*
		Function that creates a texture copy location based on a resource and a subresource index.
	*/
	D3D12_TEXTURE_COPY_LOCATION
	CreateTextureCopyLocation(ID3D12Resource* Resource, uint32 Subresource)
	{
		D3D12_TEXTURE_COPY_LOCATION Loc = {};
		Loc.pResource = Resource;
		Loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		Loc.SubresourceIndex = Subresource;
		return Loc;
	}

	/*
		Function that creates a texture copy location based on a resource and a placed subresource footprint.
	*/
	D3D12_TEXTURE_COPY_LOCATION
	CreateTextureCopyLocation(ID3D12Resource* Resource, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& Footprint)
	{
		D3D12_TEXTURE_COPY_LOCATION Loc = {};
		Loc.pResource = Resource;
		Loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		Loc.PlacedFootprint = Footprint;
		return Loc;
	}

}