#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/IntTypes.h"
#include "GPUResource.h"

namespace Floodlight {

	/*
		Vertex buffer object. Used to store vertices for rendering.
	*/
	class VertexBuffer
	{
	public:
		VertexBuffer(const void* Vertices, uint32 SizeBytes, uint32 StrideBytes);
		~VertexBuffer();

		VertexBuffer(const VertexBuffer&) = delete;
		inline void operator=(const VertexBuffer&) = delete;

		inline uint32 GetCount() const { return Count; }
		static void Bind(const VertexBuffer* Buffer);
	private:
		GPUResource* Buffer = nullptr;
		D3D12_VERTEX_BUFFER_VIEW View = {};
		uint32 Count = 0;
	};

}