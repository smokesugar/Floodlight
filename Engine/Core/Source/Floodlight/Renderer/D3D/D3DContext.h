#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/WinIncLean.h"
#include "Floodlight/Utilities/IntTypes.h"
#include "Floodlight/Utilities/Assert.h"

#include "CommandList.h"

#define cbuffer struct

namespace Floodlight {

	void HResultCall(HRESULT HR);
	void IncrementDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE* Handle, int32 Amount);
	uint32 PadConstantBufferSize(uint32 Size);

	namespace D3DContext {

		/*
			Constants
		*/
		constexpr uint32 SwapChainBufferCount = 2;

		/*
			Lifetime Controls
		*/
		void Init(HWND Window);
		void Shutdown();
		void ResizeSwapChain(uint32 Width, uint32 Height);
		bool IsInitialized();

		/*
			Behavior
		*/
		void SetViewport(float X, float Y, float Width, float Height);
		void SetScissor(uint32 X, uint32 Y, uint32 Width, uint32 Height);
		void Render(uint32 Width, uint32 Height, float Time);

		/*
			Accessors
		*/
		ID3D12Device*& GetDevice();
		CommandList& GetCommandList();
		
	}

}