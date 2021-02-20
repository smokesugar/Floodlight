#pragma once

#include <d3d12.h>
#include <vector>

#include "Floodlight/Utilities/IntTypes.h"
#include "Floodlight/Utilities/Assert.h"
#include "Floodlight/Renderer/D3D/Texture.h"

namespace Floodlight {

	/*
		Manages multiple D3D command lists.
	*/
	class CommandList
	{
	public:
		CommandList();
		~CommandList();

		void Init(uint32 FrameCount);
		void Release();

		void NewFrame(uint32 Index);
		void Execute();

		// Only use this if you want to defer the deletion of a texture.
		void DestroyTexture2D(Texture2D* Texture);

		inline ID3D12GraphicsCommandList* Get() { return Lists[Frame]; }
		inline ID3D12CommandQueue* GetCommandQueue() const { return CommandQueue; }

		// We should not be able to copy this class.
		CommandList(const CommandList&) = delete;
		void operator=(const CommandList&) = delete;
	private:
		void FlushTexture2DDeletionQueue(uint32 FrameIndex);
	private:
		uint32 Count;
		uint32 Frame = 0;
		ID3D12CommandQueue* CommandQueue = nullptr;
		std::vector<ID3D12CommandAllocator*> Allocators;
		std::vector<ID3D12GraphicsCommandList*> Lists;
		std::vector<ID3D12Fence*> Fences;
		std::vector<uint64> FenceValues;

		std::vector<std::vector<Texture2D*>> Texture2DDeletionQueue;
	};

}