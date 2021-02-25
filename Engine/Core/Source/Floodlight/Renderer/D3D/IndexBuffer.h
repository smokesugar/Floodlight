#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/IntTypes.h"
#include "GPUResource.h"

namespace Floodlight {

	/*
		Index buffer object. Used to store indices into vertices for rendering.
	*/
	class IndexBuffer
	{
	public:
		IndexBuffer(const uint32* Indices, uint32 SizeBytes);
		~IndexBuffer();

		IndexBuffer(const IndexBuffer&) = delete;
		inline void operator=(const IndexBuffer&) = delete;

		inline uint32 GetNumIndices() const { return Count; }
		
		static void Bind(const IndexBuffer* Buffer);
	private:
		GPUResource* Buffer = nullptr;
		D3D12_INDEX_BUFFER_VIEW View = {};
		uint32 Count = 0;
	};

}