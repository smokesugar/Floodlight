#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight
{

	/*
		Sampler state object bound to the graphics pipeline to let shaders sample textures.
	*/
	class SamplerState
	{
	public:
		SamplerState();
		~SamplerState();

		SamplerState(const SamplerState&) = delete;
		inline void operator=(const SamplerState&) = delete;

		static void Bind(const SamplerState* Sampler, uint32 Index);
	private:
		uint32 DescriptorIndex = 0;
	};

}