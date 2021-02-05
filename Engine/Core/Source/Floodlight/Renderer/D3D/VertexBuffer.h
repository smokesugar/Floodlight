#pragma once

#include <d3d12.h>

#include "RenderResource.h"

namespace Floodlight {

	/*
		Vertex buffer object. Used to store vertices for rendering.
	*/
	class VertexBuffer : public RenderResource
	{
	public:
		VertexBuffer(const void* Vertices, uint32 SizeBytes, uint32 StrideBytes);
		~VertexBuffer();

		inline uint32 GetCount() const { return Count; }
		inline const D3D12_VERTEX_BUFFER_VIEW& GetView() const { FL_Assert(Buffer, "Cannot retrieve view from an unintialized vertex buffer."); return View; }
	private:
		virtual void InternalRelease();
	private:
		ID3D12Resource* Buffer = nullptr;
		D3D12_VERTEX_BUFFER_VIEW View = {};
		uint32 Count = 0;
	};

	void BindVertexBuffer(const VertexBuffer* Buffer);

}