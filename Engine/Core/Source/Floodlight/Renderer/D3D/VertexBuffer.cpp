#include "VertexBuffer.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		Helper function used to create a D3D12_VERTEX_BUFFER_VIEW
	*/
	confined D3D12_VERTEX_BUFFER_VIEW
	CreateView(uint64 GPUVirtualAddress, uint32 SizeBytes, uint32 StrideBytes)
	{
		D3D12_VERTEX_BUFFER_VIEW View;
		View.BufferLocation = GPUVirtualAddress;
		View.SizeInBytes = SizeBytes;
		View.StrideInBytes = StrideBytes;
		return View;
	}

	/*
		Using the D3D12 api, create the vertex buffer object on the GPU.
	*/
	VertexBuffer::VertexBuffer(const void* Vertices, uint32 NumVertices, const uint32* AttributeSizes, uint32 NumAttributes)
	{
		// Calculate the total size of the buffer
		uint32 TotalSizeBytes = 0;
		for (uint32 i = 0; i < NumAttributes; i++)
		{
			TotalSizeBytes += AttributeSizes[i];
		}
		TotalSizeBytes *= NumVertices;
		Count = NumVertices;

		// Create a temporary staging buffer
		GPUResource* StagingBuffer = nullptr;
		{	
			D3D12_HEAP_PROPERTIES StagingHeapProps = CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
			D3D12_RESOURCE_DESC StagingResourceDesc = CreateBufferResourceDesc(TotalSizeBytes);

			StagingBuffer = new GPUResource(StagingHeapProps, StagingResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
			StagingBuffer->IncrementRef();

			// Upload the vertex data to the staging buffer
			void* BufferData = nullptr;
			D3D12_RANGE ReadRange = {};
			StagingBuffer->Raw()->Map(0, &ReadRange, &BufferData);
			memcpy(BufferData, Vertices, TotalSizeBytes);
			StagingBuffer->Raw()->Unmap(0, nullptr);
		}

		// Resource properties
		D3D12_HEAP_PROPERTIES HeapProps = CreateHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC ResourceDesc = CreateBufferResourceDesc(TotalSizeBytes);

		// Create the object
		Buffer = new GPUResource(HeapProps, ResourceDesc, D3D12_RESOURCE_STATE_COPY_DEST);
		Buffer->IncrementRef();

		// Copy from the staging buffer
		D3DContext::GetCommandList().Get()->CopyResource(Buffer->Raw(), StagingBuffer->Raw());
		Buffer->TransitionState(D3D12_RESOURCE_STATE_GENERIC_READ); // Then we can switch to generic read for rendering purposes.

		// Submit the release the staging buffer (we need to defer it until the copy command is executed)
		D3DContext::GetCommandList().QueueGPUResourceDecrement(StagingBuffer);

		// Create the views
		uint64 Offset = 0;
		for (uint32 i = 0; i < NumAttributes; i++)
		{
			uint32 AttributeSize = AttributeSizes[i];
			Views.push_back(CreateView(Buffer->Raw()->GetGPUVirtualAddress() + Offset, AttributeSize*NumVertices, AttributeSize));
			Offset += (uint64)AttributeSize * NumVertices;
		}
	}

	VertexBuffer::~VertexBuffer()
	{
		Buffer->DecrementRef();
	}

	/*
		Issue a command to set a vertex buffer.
	*/
	void
	VertexBuffer::Bind(const D3D12_VERTEX_BUFFER_VIEW* Views, uint32 Count)
	{
		D3DContext::GetCommandList().Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		D3DContext::GetCommandList().Get()->IASetVertexBuffers(0, Count, Views);
	}

}