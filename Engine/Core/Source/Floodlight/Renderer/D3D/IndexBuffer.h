#pragma once

#include <d3d12.h>

#include "RenderResource.h"

namespace Floodlight {

	/*
		Index buffer object. Used to store indices into vertices for rendering.
	*/
	class IndexBuffer : public RenderResource
	{
	public:
		IndexBuffer();
		IndexBuffer(const uint32* Indices, uint32 SizeBytes);
		~IndexBuffer();

		inline uint32 GetCount() const { return Count; }
		inline const D3D12_INDEX_BUFFER_VIEW& GetView() const { FL_Assert(Buffer, "Cannot retrieve view from an unintialized index buffer.");  return View; }
	private:
		virtual void InternalRelease() override;
	private:
		ID3D12Resource* Buffer = nullptr;
		D3D12_INDEX_BUFFER_VIEW View = {};
		uint32 Count = 0;
	};

	void BindIndexBuffer(const IndexBuffer* Buffer);

}