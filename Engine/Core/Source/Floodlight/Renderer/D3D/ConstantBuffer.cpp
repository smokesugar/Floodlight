#include "ConstantBuffer.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		Pad constant buffer sizes to 256 boundaries.
	*/
	confined uint32
	PadConstantBufferSize(uint32 Size)
	{
		return (Size + 255) & ~255;
	}

	/*
		Allocate a resource with a certain size. Get and fill a descriptor.
	*/
	ConstantBuffer::ConstantBuffer(uint32 SizeBytes_)
	{
		/*
			First we allocate the resource on the GPU.
		*/
		IndividualSizeBytes = SizeBytes_;
		TotalSizeBytes = PadConstantBufferSize(SizeBytes_);

		D3D12_HEAP_PROPERTIES HeapProps = {};
		HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC ResourceDesc = {};
		ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ResourceDesc.Alignment = 0;
		ResourceDesc.Width = (uint64)(TotalSizeBytes);
		ResourceDesc.Height = 1;
		ResourceDesc.DepthOrArraySize = 1;
		ResourceDesc.MipLevels = 1;
		ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		ResourceDesc.SampleDesc = { 1, 0 };
		ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3DContext::GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&Buffer));

		/*
			Then we ask the descriptor heap to allocate us a slot within it.
		*/
		DescriptorHeapIndex =  D3DContext::GetCBVSRVUAVDescriptorHeap().GetNewIndex();

		/*
			Then we create the view in that slot.
		*/
		D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc = {};
		CBVDesc.BufferLocation = Buffer->GetGPUVirtualAddress();
		CBVDesc.SizeInBytes = TotalSizeBytes;
		D3DContext::GetDevice()->CreateConstantBufferView(&CBVDesc, D3DContext::GetCBVSRVUAVDescriptorHeap().GetCPUHandleAtIndex(DescriptorHeapIndex));
	}

	ConstantBuffer::~ConstantBuffer()
	{
		// Free the descriptor
		Release();
	}

	void
	ConstantBuffer::Update(void* Data, uint32 SizeBytes)
	{
		FL_Assert(SizeBytes == IndividualSizeBytes, "Updating constant buffer with an invalid amount of data.");

		D3D12_RANGE ReadRange = {};
		void* BufferData;
		Buffer->Map(0, &ReadRange, &BufferData);
		memcpy(BufferData, Data, SizeBytes);
		Buffer->Unmap(0, nullptr);
	}

	void
	ConstantBuffer::InternalRelease()
	{
		// Free our slot in the descriptor heap.
		D3DContext::GetCBVSRVUAVDescriptorHeap().FreeIndex(DescriptorHeapIndex);
		Buffer->Release();
	}

	/*
		Issue a command to bind the constant buffer.
	*/
	void 
	BindConstantBuffer(const ConstantBuffer* Buffer, uint32 Index)
	{
		// Get the handle to our view in the descriptor heap and bind it.
		D3D12_GPU_DESCRIPTOR_HANDLE Handle = D3DContext::GetCBVSRVUAVDescriptorHeap().GetGPUHandleAtIndex(Buffer->GetDescriptorHeapIndex());
		D3DContext::GetCommandList().Get()->SetGraphicsRootDescriptorTable(Index, Handle);
	}

}