#pragma once

#include <d3d12.h>
#include <vector>

#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	using DescriptorHeapType = int;
	enum _DescriptorHeapType
	{
		DescriptorHeapType_CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		DescriptorHeapType_RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		DescriptorHeapType_DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		DescriptorHeapType_Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	};

	/*
		Used to allocate D3D descriptors.
	*/
	class DescriptorHeap
	{
	public:
		DescriptorHeap();
		~DescriptorHeap();

		// We should not be able to copy this class.
		DescriptorHeap(const DescriptorHeap&) = delete;
		void operator=(const DescriptorHeap&) = delete;

		void Init(DescriptorHeapType Type, uint32 Count);
		void Release();

		uint32 GetNewIndex();
		void FreeIndex(uint32 Index);

		static void Bind(DescriptorHeap** Heaps, uint32 Count);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleAtIndex(uint32 Index);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleAtIndex(uint32 Index);
		inline ID3D12DescriptorHeap* Get() const { return Heap; }
	private:
		ID3D12DescriptorHeap* Heap = nullptr;
		std::vector<uint32> Holes;
		uint32 MaxCount = 0;
		uint32 Counter = 0;
		uint32 D3DDescriptorSize = 0;
	};

}