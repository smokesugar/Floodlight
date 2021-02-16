#pragma once

#include "Floodlight/Utilities/IntTypes.h"
#include "Texture.h"

namespace Floodlight {

	/*
		Used to give shaders access to sample a texture.
	*/
	class ShaderResourceView
	{
	public:
		ShaderResourceView(const Texture2D* Parent);
		~ShaderResourceView();

		ShaderResourceView(const ShaderResourceView&) = delete;
		inline void operator=(const ShaderResourceView&) = delete;

		static void Bind(const ShaderResourceView* SRV, uint32 Index);
	private:
		uint32 DescriptorIndex = 0;
		GPUResource* ParentResource = nullptr;
	};

}