#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	class GPUResource
	{
	public:
		GPUResource(ID3D12Resource* Res, D3D12_RESOURCE_STATES InitialState);
		GPUResource(const D3D12_HEAP_PROPERTIES& HeapProps, const D3D12_RESOURCE_DESC& Desc, D3D12_RESOURCE_STATES InitialState);
		~GPUResource();

		GPUResource(const GPUResource&) = delete;
		inline void operator=(const GPUResource&) = delete;

		void TransitionState(D3D12_RESOURCE_STATES TargetState);

		inline ID3D12Resource* Raw() const { return Resource; }

		void IncrementRef();
		void DecrementRef();
	private:
		uint32 References = 0;
		ID3D12Resource* Resource = nullptr;
		D3D12_RESOURCE_STATES State;
	};

}