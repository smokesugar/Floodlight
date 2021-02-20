#pragma once

#include "Floodlight/Application/Application.h"

#include "Floodlight/Renderer/D3D/PipelineState.h"
#include "Floodlight/Renderer/D3D/VertexBuffer.h"
#include "Floodlight/Renderer/D3D/IndexBuffer.h"
#include "Floodlight/Renderer/D3D/ConstantBuffer.h"
#include "Floodlight/Renderer/D3D/RenderTargetView.h"
#include "Floodlight/Renderer/D3D/DepthStencilView.h"
#include "Floodlight/Renderer/D3D/ShaderResourceView.h"
#include "Floodlight/Renderer/D3D/SamplerState.h"

namespace Floodlight {

	class Runtime : public Application {
	public:
		Runtime();
		~Runtime();

		virtual void Tick() override;
	private:
		PipelineState* PSO = nullptr;
		VertexBuffer* VBO = nullptr;
		IndexBuffer* IBO = nullptr;
		ConstantBuffer* MVPCBO = nullptr;
		Texture2D* IndirectTexture = nullptr;
		RenderTargetView* RTV = nullptr;
		DepthStencilView* DSV = nullptr;
		ShaderResourceView* SRV = nullptr;
		SamplerState* Sampler = nullptr;
	};

}