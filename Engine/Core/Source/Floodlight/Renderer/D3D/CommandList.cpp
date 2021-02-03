#include "CommandList.h"

#include "Floodlight/Utilities/Keywords.h"
#include "D3DContext.h"

namespace Floodlight {

	/*
		Prevent writing to a command list if it is still begin executed.
	*/
	confined void
	WaitForFenceToBeHit(ID3D12Fence* Fence, uint64 Value)
	{
		if (Fence->GetCompletedValue() < Value)
		{
			HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			Fence->SetEventOnCompletion(Value, EventHandle);
			WaitForSingleObject(EventHandle, INFINITE);
			CloseHandle(EventHandle);
		}
	}

	CommandList::CommandList()
	{	
	}

	CommandList::~CommandList()
	{
	}

	void CommandList::Init(uint32 FrameCount)
	{
		Count = FrameCount;
		
		{ // Create the command queue
			D3D12_COMMAND_QUEUE_DESC Desc = {};
			Desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			D3DContext::GetDevice()->CreateCommandQueue(&Desc, IID_PPV_ARGS(&CommandQueue));
		}

		Allocators.resize(Count);
		Lists.resize(Count);
		Fences.resize(Count);
		FenceValues.resize(Count);

		for (uint32 i = 0; i < Count; i++)
		{
			D3DContext::GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocators[i]));
			D3DContext::GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocators[i], nullptr, IID_PPV_ARGS(&Lists[i]));
			D3DContext::GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fences[i]));
			Lists[i]->Close();
			FenceValues[i] = 0;
		}
	}

	/*
		Ensure all D3D objects are released.
	*/
	void
	CommandList::Release()
	{
		CommandQueue->Release();
		for (uint32 i = 0; i < Count; i++)
		{
			Allocators[i]->Release();
			Lists[i]->Release();
			Fences[i]->Release();
		}
	}

	/*
		Move to the next command list.
	*/
	void
	CommandList::NewFrame(uint32 Index)
	{
		FL_Assert(Index < Count, "Invalid index passed to command list.");

		Frame = Index;

		// Before we start recording using this command list, ensure it has flushed from previous use.
		WaitForFenceToBeHit(Fences[Frame], FenceValues[Frame]);

		Allocators[Frame]->Reset();
		Lists[Frame]->Reset(Allocators[Frame], nullptr);
	}

	/*
		Flush the current command list.
	*/
	void
	CommandList::Execute()
	{
		Lists[Frame]->Close();
		ID3D12CommandList* TempArray[] = { Lists[Frame] };
		CommandQueue->ExecuteCommandLists(1, TempArray);
		CommandQueue->Signal(Fences[Frame], ++FenceValues[Frame]);
	}

}