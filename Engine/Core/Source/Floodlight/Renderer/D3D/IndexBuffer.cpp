#include "IndexBuffer.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		Using the D3D12 api, create the index buffer object on the GPU.
	*/
	IndexBuffer::IndexBuffer(const uint32* Indices, uint32 SizeBytes)
	{
		Count = SizeBytes / sizeof(uint32);

		// Memory layout properties
		D3D12_HEAP_PROPERTIES HeapProps = CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD);

		// Resource properties
		D3D12_RESOURCE_DESC ResourceDesc = CreateBufferResourceDesc(SizeBytes);

		// Create the object
		Buffer = new GPUResource(HeapProps, ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
		Buffer->IncrementRef();

		// Upload the index data
		void* BufferData = nullptr;
		D3D12_RANGE ReadRange = {};
		Buffer->Raw()->Map(0, &ReadRange, &BufferData);
		memcpy(BufferData, Indices, SizeBytes);
		Buffer->Raw()->Unmap(0, nullptr);

		// Create the view
		View.BufferLocation = Buffer->Raw()->GetGPUVirtualAddress();
		View.SizeInBytes = SizeBytes;
		View.Format = DXGI_FORMAT_R32_UINT;
	}

	IndexBuffer::~IndexBuffer()
	{
		Buffer->DecrementRef();
	}

	/*
		Issue a command to set the index buffer.
	*/
	void
	IndexBuffer::Bind(const IndexBuffer* Buffer)
	{
		D3DContext::GetCommandList().Get()->IASetIndexBuffer(&Buffer->View);
	}

}