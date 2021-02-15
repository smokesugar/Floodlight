#pragma once

#include <d3d12.h>

namespace Floodlight {

	struct PipelineStateDesc
	{
		const wchar_t* VertexShader;
		const wchar_t* PixelShader;
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

		static void Bind(const PipelineState* PS);
	private:
		ID3D12PipelineState* PSO = nullptr;
		ID3D12RootSignature* RootSignature = nullptr;
	};

}