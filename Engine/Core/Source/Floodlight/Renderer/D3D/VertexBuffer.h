#pragma once

#include <d3d12.h>
#include <vector>

#include "Floodlight/Utilities/IntTypes.h"
#include "GPUResource.h"

namespace Floodlight {

	/*
		Vertex buffer object. Used to store vertices for rendering.
	*/
	class VertexBuffer
	{
	public:
		VertexBuffer(const void* Vertices, uint32 NumVertices, const uint32* AttributeSizes, uint32 NumAttributes);
		~VertexBuffer();

		VertexBuffer(const VertexBuffer&) = delete;
		inline void operator=(const VertexBuffer&) = delete;

		inline uint32 GetNumVertices() const { return Count; }
		inline uint32 GetNumViews() const { return Views.size(); }
		inline const D3D12_VERTEX_BUFFER_VIEW* GetViewsPointer() const { return Views.data(); }
		inline D3D12_VERTEX_BUFFER_VIEW GetView(uint32 Attribute) const { return Views[Attribute]; }

		static void Bind(const D3D12_VERTEX_BUFFER_VIEW* Views, uint32 Count);
	private:
		GPUResource* Buffer = nullptr;
		std::vector<D3D12_VERTEX_BUFFER_VIEW> Views;
		uint32 Count = 0;
	};

}