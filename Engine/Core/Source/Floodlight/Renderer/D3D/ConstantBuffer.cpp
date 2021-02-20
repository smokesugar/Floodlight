#include "ConstantBuffer.h"

#include <unordered_map>

#include "D3DContext.h"

namespace Floodlight {

	confined std::unordered_map<ConstantBuffer*, UpdateObj> UpdateQueue;

	/*
		Individual update call (Called from frame to frame).
	*/
	bool
	ConstantBuffer::DoUpdateObj(ConstantBuffer* Buffer, UpdateObj* Obj, uint32 FrameIndex)
	{
		Obj->Counter--;

		// Upload the data
		D3D12_RANGE ReadRange = {};
		uint8* BufferData;
		Buffer->Buffer->Raw()->Map(0, &ReadRange, (void**)&BufferData);
		uint32 ByteOffset = Buffer->IndividualSizeBytes * FrameIndex;
		memcpy(BufferData + ByteOffset, Obj->Data, Obj->SizeBytes);
		Buffer->Buffer->Raw()->Unmap(0, nullptr);

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

		D3D12_HEAP_PROPERTIES HeapProps = CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC ResourceDesc = CreateBufferResourceDesc(TotalSizeBytes);

		Buffer = new GPUResource(HeapProps, ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ);
		Buffer->IncrementRef();

		// Create the constant buffer views
		for (uint32 i = 0; i < D3DContext::SwapChainBufferCount; i++) {
			// We ask the descriptor heap to allocate us slots within it.
			DescriptorHeapIndices.push_back(D3DContext::GetCBVSRVUAVDescriptorHeap().GetNewIndex());

			//Then we create the views in those slots.
			D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc = {};
			D3D12_GPU_DESCRIPTOR_HANDLE Handle;
			Handle.ptr = Buffer->Raw()->GetGPUVirtualAddress();
			IncrementDescriptorHandle(&Handle, i * IndividualSizeBytes);
			CBVDesc.BufferLocation = Handle.ptr;
			CBVDesc.SizeInBytes = IndividualSizeBytes;
			D3DContext::GetDevice()->CreateConstantBufferView(&CBVDesc, D3DContext::GetCBVSRVUAVDescriptorHeap().GetCPUHandleAtIndex(DescriptorHeapIndices[i]));
		}
	}

	ConstantBuffer::~ConstantBuffer()
	{
		// Free our slot in the descriptor heap.
		for (auto& Index : DescriptorHeapIndices)
			D3DContext::GetCBVSRVUAVDescriptorHeap().FreeIndex(Index);
		Buffer->DecrementRef();
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
			bool Finished = DoUpdateObj(It->first, &It->second, FrameIndex);
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
		if (UpdateQueue.find(Buffer) == UpdateQueue.end())
		{
			UpdateObj Obj;
			Obj.Data = malloc(SizeBytes);
			Obj.SizeBytes = SizeBytes;
			UpdateQueue[Buffer] = Obj;
		}

		UpdateObj& Obj = UpdateQueue[Buffer];
		Obj.Counter = D3DContext::SwapChainBufferCount;
		memcpy(Obj.Data, Data, SizeBytes);
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

}