#include "SamplerState.h"

#include "D3DContext.h"

namespace Floodlight
{

	SamplerState::SamplerState()
	{
		DescriptorIndex = D3DContext::GetSamplerDescriptorHeap().GetNewIndex();

		D3D12_SAMPLER_DESC Desc = {};
		Desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		Desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		Desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		Desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		Desc.MipLODBias = 0;
		Desc.MinLOD = 0;
		Desc.MaxLOD = FLT_MAX;

		auto Handle = D3DContext::GetSamplerDescriptorHeap().GetCPUHandleAtIndex(DescriptorIndex);
		D3DContext::GetDevice()->CreateSampler(&Desc, Handle);
	}

	SamplerState::~SamplerState()
	{
		D3DContext::GetSamplerDescriptorHeap().FreeIndex(DescriptorIndex);
	}

	/*
		Issue a command to bind a sampler state.
	*/
    void
	SamplerState::Bind(const SamplerState* Sampler, uint32 Index)
    {
		D3D12_GPU_DESCRIPTOR_HANDLE Handle = D3DContext::GetSamplerDescriptorHeap().GetGPUHandleAtIndex(Sampler->DescriptorIndex);
		D3DContext::GetCommandList().Get()->SetGraphicsRootDescriptorTable(Index, Handle);
    }
}