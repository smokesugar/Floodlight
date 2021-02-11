#pragma once

#include <d3d12.h>
#include <vector>

#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	using DescriptorHeapType = int;
	enum _DescriptorHeapType
	{
		DescriptorHeapType_CBV_SRV_UAV = 0,
		DescriptorHeapType_RTV = 2,
	};

	/*
		Used to allocate D3D descriptors.
	*/
	class DescriptorHeap
	{
	public:
		DescriptorHeap();
		~DescriptorHeap();

		void Init(DescriptorHeapType Type, uint32 Count);
		void Release();

		uint32 GetNewIndex();
		void FreeIndex(uint32 Index);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleAtIndex(uint32 Index);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleAtIndex(uint32 Index);
		inline ID3D12DescriptorHeap* Get() const { return Heap; }

		// We should not be able to copy this class.
		DescriptorHeap(const DescriptorHeap&) = delete;
		void operator=(const DescriptorHeap&) = delete;
	private:
		ID3D12DescriptorHeap* Heap = nullptr;
		std::vector<uint32> Holes;
		uint32 MaxCount = 0;
		uint32 Counter = 0;
		uint32 D3DDescriptorSize = 0;
	};

	void BindDescriptorHeap(const DescriptorHeap* Heap);

}