#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/WinIncLean.h"
#include "Floodlight/Utilities/IntTypes.h"
#include "Floodlight/Utilities/Assert.h"

#include "CommandList.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "PipelineState.h"
#include "Texture.h"

#define cbuffer struct

namespace Floodlight {

	void HResultCall(HRESULT HR);
	void IncrementDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE* Handle, int32 Amount);
	void IncrementDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE* Handle, int32 Amount);
	uint32 PadConstantBufferSize(uint32 Size);

	namespace D3DContext {

		/*
			Constants
		*/
		static constexpr uint32 SwapChainBufferCount = 3;

		/*
			Lifetime Controls
		*/
		void Init(HWND Window);
		void Shutdown();
		void ResizeSwapChain(uint32 Width, uint32 Height);
		bool IsInitialized();
		void BeginFrame();
		void EndFrame();

		/*
			Behavior
		*/
		void SetViewport(float X, float Y, float Width, float Height);
		void SetScissor(uint32 X, uint32 Y, uint32 Width, uint32 Height);

		/*
			Accessors
		*/
		ID3D12Device*& GetDevice();
		CommandList& GetCommandList();
		DescriptorHeap& GetCBVSRVUAVDescriptorHeap();
		DescriptorHeap& GetRTVDescriptorHeap();
		DescriptorHeap& GetDSVDescriptorHeap();
		DescriptorHeap& GetSamplerDescriptorHeap();
		uint32 GetSwapChainBufferIndex();
		Texture2D*& GetBackBuffer();
	}

}