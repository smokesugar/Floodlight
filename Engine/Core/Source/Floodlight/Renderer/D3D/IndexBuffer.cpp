#include "IndexBuffer.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		If using empty constructor, initialize nothing.
	*/
	IndexBuffer::IndexBuffer()
	{

	}

	/*
		Using the D3D12 api, create the index buffer object on the GPU.
	*/
	IndexBuffer::IndexBuffer(const uint32* Indices, uint32 SizeBytes)
	{
		Count = SizeBytes / sizeof(uint32);

		// Memory layout properties
		D3D12_HEAP_PROPERTIES HeapProps = {};
		HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		// Resource properties
		D3D12_RESOURCE_DESC ResourceDesc = {};
		ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ResourceDesc.Alignment = 0;
		ResourceDesc.Width = SizeBytes;
		ResourceDesc.Height = 1;
		ResourceDesc.DepthOrArraySize = 1;
		ResourceDesc.MipLevels = 1;
		ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		ResourceDesc.SampleDesc = { 1, 0 };
		ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// Create the object
		D3DContext::GetDevice()->CreateCommittedResource(
			&HeapProps, D3D12_HEAP_FLAG_NONE,
			&ResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&Buffer)
		);

		// Upload the index data
		void* BufferData = nullptr;
		D3D12_RANGE ReadRange = {};
		Buffer->Map(0, &ReadRange, &BufferData);
		memcpy(BufferData, Indices, SizeBytes);
		Buffer->Unmap(0, nullptr);

		// Create the view
		View.BufferLocation = Buffer->GetGPUVirtualAddress();
		View.SizeInBytes = SizeBytes;
		View.Format = DXGI_FORMAT_R32_UINT;
	}

	IndexBuffer::~IndexBuffer()
	{
		Release();
	}

	void
	IndexBuffer::InternalRelease()
	{
		if (Buffer) Buffer->Release();
	}

	/*
		Issue a command to set the index buffer.
	*/
	void
	BindIndexBuffer(const IndexBuffer* Buffer)
	{
		D3DContext::GetCommandList().Get()->IASetIndexBuffer(&Buffer->GetView());
	}

}