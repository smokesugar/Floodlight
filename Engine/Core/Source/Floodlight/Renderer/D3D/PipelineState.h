#pragma once

#include <d3d12.h>
#include <unordered_map>

#include "Floodlight/Utilities/IntTypes.h"
#include "TextureFormats.h"

namespace Floodlight {

	struct PipelineStateDesc
	{
		const wchar_t* VertexShader;
		const wchar_t* PixelShader;
		TextureFormat RTVFormats[8];
		uint32 NumRenderTargets;
		TextureFormat DSVFormat;
	};

	/*
		D3D pipeline state.
	*/
	class PipelineState
	{
	public:
		PipelineState(const PipelineStateDesc& Desc);
		~PipelineState();

		PipelineState(const PipelineState&) = delete;
		inline void operator=(const PipelineState&) = delete;

		uint32 GetRootSignatureIndexOfCBVAtVSRegister(uint32 Register) const;
		uint32 GetRootSignatureIndexOfCBVAtPSRegister(uint32 Register) const;
		uint32 GetRootSignatureIndexOfSRVAtRegister(uint32 Register) const;
		uint32 GetRootSignatureIndexOfSamplerAtRegister(uint32 Register) const;

		static const PipelineState* GetCurrentlyBound();

		// Binds a pipelines state. Note: internally, a pointer to the instance being bound, is saved. It can be retrieved by calling GetCurrentlyBound.
		static void Bind(const PipelineState* PS);
	private:
		ID3D12PipelineState* PSO = nullptr;
		ID3D12RootSignature* RootSignature = nullptr;

		std::unordered_map<uint32, uint32> CBVIndicesVS;
		std::unordered_map<uint32, uint32> CBVIndicesPS;
		std::unordered_map<uint32, uint32> SRVIndices;
		std::unordered_map<uint32, uint32> SamplerIndices;
	};

}