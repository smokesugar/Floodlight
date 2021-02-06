#include "ConstantBuffer.h"

#include <unordered_map>

#include "D3DContext.h"

namespace Floodlight {

	confined std::unordered_map<ReferenceCounter*, UpdateObj> UpdateQueue;

	/*
		Individual update call (Called from frame to frame).
	*/
	bool
	ConstantBuffer::DoUpdateObj(UpdateObj* Obj, uint32 FrameIndex)
	{
		Obj->Counter--;

		// Upload the data
		D3D12_RANGE ReadRange = {};
		uint8* BufferData;
		Obj->Buffer->Buffer->Map(0, &ReadRange, (void**)&BufferData);
		uint32 ByteOffset = Obj->Buffer->IndividualSizeBytes * FrameIndex;
		memcpy(BufferData + ByteOffset, Obj->Data, Obj->SizeBytes);
		Obj->Buffer->Buffer->Unmap(0, nullptr);

		// Say if we should remove it from the queue
		return Obj->Counter == 0;
	}

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
	ConstantBuffer::ConstantBuffer(uint32 SizeBytes)
	{
		/*
			First we allocate the resource on the GPU.
		*/
		OriginalSize = SizeBytes;
		IndividualSizeBytes = PadConstantBufferSize(OriginalSize);
		TotalSizeBytes = PadConstantBufferSize(IndividualSizeBytes*D3DContext::SwapChainBufferCount);

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

		// Create the constant buffer views
		for (uint32 i = 0; i < D3DContext::SwapChainBufferCount; i++) {
			// We ask the descriptor heap to allocate us slots within it.
			DescriptorHeapIndices.push_back(D3DContext::GetCBVSRVUAVDescriptorHeap().GetNewIndex());

			//Then we create the views in those slots.
			D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc = {};
			D3D12_GPU_DESCRIPTOR_HANDLE Handle;
			Handle.ptr = Buffer->GetGPUVirtualAddress();
			IncrementDescriptorHandle(&Handle, i * IndividualSizeBytes);
			CBVDesc.BufferLocation = Handle.ptr;
			CBVDesc.SizeInBytes = IndividualSizeBytes;
			D3DContext::GetDevice()->CreateConstantBufferView(&CBVDesc, D3DContext::GetCBVSRVUAVDescriptorHeap().GetCPUHandleAtIndex(DescriptorHeapIndices[i]));
		}
	}

	ConstantBuffer::~ConstantBuffer()
	{
		// Free the descriptor
		Release();
	}

	/*
		This function is called every frame to update each constant buffer in the queue.
	*/
	void
	ConstantBuffer::DoUpdateQueue(uint32 FrameIndex)
	{
		auto It = UpdateQueue.begin();
		while (It != UpdateQueue.end())
		{
			bool Finished = DoUpdateObj(&It->second, FrameIndex);
			if (Finished)
			{
				free(It->second.Data);
				It = UpdateQueue.erase(It);
			}
			else {
				It++;
			}
		}
	}

	/*
		Free all the allocated memory.
	*/
	void ConstantBuffer::DestroyUpdateQueue()
	{
		for (auto& [RefCounter, Obj] : UpdateQueue)
		{
			free(Obj.Data);
		}
	}

	/*
		Submit constant buffer for submission over multiple frames.
	*/
	void
	ConstantBuffer::Update(ConstantBuffer* Buffer, void* Data, uint32 SizeBytes)
	{
		FL_Assert(SizeBytes == Buffer->OriginalSize, "Updating constant buffer with an invalid amount of data.");

		// Create the object if it does not exist
		if (UpdateQueue.find(Buffer->RefCounter) == UpdateQueue.end())
		{
			UpdateObj Obj;
			Obj.Buffer = Buffer;
			Obj.Data = malloc(SizeBytes);
			Obj.SizeBytes = SizeBytes;
			UpdateQueue[Buffer->RefCounter] = Obj;
		}

		UpdateObj& Obj = UpdateQueue[Buffer->RefCounter];
		Obj.Counter = D3DContext::SwapChainBufferCount;
		memcpy(Obj.Data, Data, SizeBytes);

		// Update now
		DoUpdateObj(&Obj, D3DContext::GetSwapChainBufferIndex());
	}

	/*
		Issue a command to bind the constant buffer.
	*/
	void ConstantBuffer::Bind(const ConstantBuffer* Buffer, uint32 Index)
    {
		uint32 FrameIndex = D3DContext::GetSwapChainBufferIndex();
		uint32 DescriptorHeapIndex = Buffer->DescriptorHeapIndices[FrameIndex];
		D3D12_GPU_DESCRIPTOR_HANDLE Handle = D3DContext::GetCBVSRVUAVDescriptorHeap().GetGPUHandleAtIndex(DescriptorHeapIndex);
		D3DContext::GetCommandList().Get()->SetGraphicsRootDescriptorTable(Index, Handle);
    }

	/*
		Release D3D objects.
	*/
	void
	ConstantBuffer::InternalRelease()
	{
		// Free our slot in the descriptor heap.
		for(auto& Index : DescriptorHeapIndices)
			D3DContext::GetCBVSRVUAVDescriptorHeap().FreeIndex(Index);
		Buffer->Release();
	}

}