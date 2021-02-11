#pragma once

#include "Floodlight/Utilities/IntTypes.h"
#include "Floodlight/Utilities/Math.h"
#include "Texture.h"
#include "DepthStencilView.h"

namespace Floodlight {

	/*
		Used to bind a texture as a render target.
	*/
	class RenderTargetView
	{
	public:
		RenderTargetView(const Texture2D* Parent);
		~RenderTargetView();

		RenderTargetView(const RenderTargetView&) = delete;
		inline void operator=(const RenderTargetView&) = delete;

		void Clear(float4 Color);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
		inline ID3D12Resource* GetParentResource() const { return ParentResource; }
	private:
		uint32 DescriptorIndex = 0;
		ID3D12Resource* ParentResource;
	};

	void BindRenderTargets(RenderTargetView** RenderTargets, uint32 NumRenderTargets, DepthStencilView* DSV);

}