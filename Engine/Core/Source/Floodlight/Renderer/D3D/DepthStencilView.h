#pragma once

#include <d3d12.h>

#include "Texture.h"

namespace Floodlight {

	class DepthStencilView
	{
	public:
		DepthStencilView(const Texture2D* Parent);
		~DepthStencilView();

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
		inline GPUResource* GetParentResource() const { return ParentResource; }

		void Clear(float Depth);

		DepthStencilView(const DepthStencilView&) = delete;
		inline void operator=(const DepthStencilView&) = delete;
	private:
		uint32 DescriptorIndex = 0;
		GPUResource* ParentResource = nullptr;
	};

}