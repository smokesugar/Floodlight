#include "VertexBuffer.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		Using the D3D12 api, create the vertex buffer object on the GPU.
	*/
	VertexBuffer::VertexBuffer(const void* Vertices, uint32 SizeBytes, uint32 StrideBytes)
	{
		Count = SizeBytes / StrideBytes;

		// Memory layout properties
		D3D12_HEAP_PROPERTIES HeapProps = CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD);

		// Resource properties
		D3D12_RESOURCE_DESC ResourceDesc = CreateBufferResourceDesc(SizeBytes);

		// Create the object
		Buffer = new GPUResource(HeapProps, ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
		Buffer->IncrementRef();

		// Upload the vertex data
		void* BufferData = nullptr;
		D3D12_RANGE ReadRange = {};
		Buffer->Raw()->Map(0, &ReadRange, &BufferData);
		memcpy(BufferData, Vertices, SizeBytes);
		Buffer->Raw()->Unmap(0, nullptr);

		// Create the view
		View.BufferLocation = Buffer->Raw()->GetGPUVirtualAddress();
		View.StrideInBytes = StrideBytes;
		View.SizeInBytes = SizeBytes;
	}

	VertexBuffer::~VertexBuffer()
	{
		Buffer->DecrementRef();
	}

	/*
		Issue a command to set a vertex buffer.
	*/
	void
	VertexBuffer::Bind(const VertexBuffer* Buffer)
	{
		D3DContext::GetCommandList().Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		D3DContext::GetCommandList().Get()->IASetVertexBuffers(0, 1, &Buffer->View);
	}

}