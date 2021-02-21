#include "IndexBuffer.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		Using the D3D12 api, create the index buffer object on the GPU.
	*/
	IndexBuffer::IndexBuffer(const uint32* Indices, uint32 SizeBytes)
	{
		GPUResource* StagingBuffer = nullptr;

		{	// Create a temporary staging buffer
			D3D12_HEAP_PROPERTIES StagingHeapProps = CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
			D3D12_RESOURCE_DESC StagingResourceDesc = CreateBufferResourceDesc(SizeBytes);

			StagingBuffer = new GPUResource(StagingHeapProps, StagingResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
			StagingBuffer->IncrementRef();

			// Upload the index data to the staging buffer
			void* BufferData = nullptr;
			D3D12_RANGE ReadRange = {};
			StagingBuffer->Raw()->Map(0, &ReadRange, &BufferData);
			memcpy(BufferData, Indices, SizeBytes);
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
		View.SizeInBytes = SizeBytes;
		View.Format = DXGI_FORMAT_R32_UINT;
		Count = SizeBytes / sizeof(uint32);
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