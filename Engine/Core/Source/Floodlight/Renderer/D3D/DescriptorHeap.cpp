#include "DescriptorHeap.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		Empty constructor and destructor.
	*/
	DescriptorHeap::DescriptorHeap()
	{
	}
	DescriptorHeap::~DescriptorHeap()
	{
	}

	/*
		Create the D3D descriptor heap based on a maximum count;
	*/
    void
	DescriptorHeap::Init(DescriptorHeapType Type, uint32 Count)
    {
		MaxCount = Count;

		bool ShaderVisible = Type == DescriptorHeapType_CBV_SRV_UAV || Type == DescriptorHeapType_Sampler;

		D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
		Desc.NumDescriptors = MaxCount;
		Desc.Type = (D3D12_DESCRIPTOR_HEAP_TYPE)Type;
		Desc.Flags = ShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		Desc.NodeMask = 0;

		D3DDescriptorSize = D3DContext::GetDevice()->GetDescriptorHandleIncrementSize(Desc.Type);

		D3DContext::GetDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Heap));
    }

	/*
		Release the D3D object.
	*/
    void
	DescriptorHeap::Release()
    {
		Heap->Release();
    }

	/*
		Select an index from either an array of holes or an incrementing size.
	*/
	uint32 DescriptorHeap::GetNewIndex()
	{
		uint32 Index;
		if (Holes.empty()) 
		{
			Index = Counter++;
		}
		else
		{
			Index = Holes[0];
			Holes[0] = Holes[Holes.size() - 1];
			Holes.pop_back();
		}
		FL_Assert(Index < MaxCount, "Number of descriptors has breached the maximum number allocated by this instance.");
		return Index;
	}

	/*
		Free an index to be allocated later
	*/
	void DescriptorHeap::FreeIndex(uint32 Index)
	{
		FL_Assert(Index < Counter && std::find(Holes.begin(), Holes.end(), Index) == Holes.end(), "Invalid index to free.");
		Holes.push_back(Index);
	}

	/*
		The following functions allow for other resources to access individual descriptors within the heap, given an index.
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandleAtIndex(uint32 Index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE Handle = Heap->GetCPUDescriptorHandleForHeapStart();
		IncrementDescriptorHandle(&Handle, Index * D3DDescriptorSize);
		return Handle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandleAtIndex(uint32 Index)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE Handle = Heap->GetGPUDescriptorHandleForHeapStart();
		IncrementDescriptorHandle(&Handle, Index * D3DDescriptorSize);
		return Handle;
	}

	/*
		Issue a command to bind the descriptor heap.
	*/
	void
	DescriptorHeap::Bind(DescriptorHeap** Heaps, uint32 Count)
	{
		std::vector<ID3D12DescriptorHeap*> Arr;
		for (uint32 i = 0; i < Count; i++)
		{
			Arr.push_back(Heaps[i]->Heap);
		}
		D3DContext::GetCommandList().Get()->SetDescriptorHeaps(Count, Arr.data());
	}

}