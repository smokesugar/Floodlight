#include "VertexBuffer.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		Using the D3D12 api, create the vertex buffer object on the GPU.
	*/
	VertexBuffer::VertexBuffer(const void* Vertices, uint32 SizeBytes, uint32 StrideBytes)
	{
		GPUResource* StagingBuffer = nullptr;

		{	// Create a temporary staging buffer
			D3D12_HEAP_PROPERTIES StagingHeapProps = CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
			D3D12_RESOURCE_DESC StagingResourceDesc = CreateBufferResourceDesc(SizeBytes);

			StagingBuffer = new GPUResource(StagingHeapProps, StagingResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
			StagingBuffer->IncrementRef();

			// Upload the vertex data to the staging buffer
			void* BufferData = nullptr;
			D3D12_RANGE ReadRange = {};
			StagingBuffer->Raw()->Map(0, &ReadRange, &BufferData);
			memcpy(BufferData, Vertices, SizeBytes);
			StagingBuffer->Raw()->Unmap(0, nullptr);
		}

		// Resource properties
		D3D12_HEAP_PROPERTIES HeapProps = CreateHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC ResourceDesc = CreateBufferResourceDesc(SizeBytes);

		// Create the object
		Buffer = new GPUResource(HeapProps, ResourceDesc, D3D12_RESOURCE_STATE_COPY_DEST);
		Buffer->IncrementRef();

		// Copy from the staging buffer
		D3DContext::GetCommandList().Get()->CopyResource(Buffer->Raw(), StagingBuffer->Raw());
		Buffer->TransitionState(D3D12_RESOURCE_STATE_GENERIC_READ); // Then we can switch to generic read for rendering purposes.

		// Submit the release the staging buffer (we need to defer it until the copy command is executed)
		D3DContext::GetCommandList().QueueGPUResourceDecrement(StagingBuffer);

		// Create the view
		View.BufferLocation = Buffer->Raw()->GetGPUVirtualAddress();
		View.StrideInBytes = StrideBytes;
		View.SizeInBytes = SizeBytes;
		Count = SizeBytes / StrideBytes;
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