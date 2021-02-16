#include "D3DContext.h"

#include <dxgi1_6.h>
#include <comdef.h>
#include <d3dcompiler.h>

#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Utilities/Math.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderTargetView.h"

namespace Floodlight {

	/*
		In debug mode, break on failure.
	*/
	void
	HResultCall(HRESULT HR)
	{
		_com_error Err(HR);
		FL_Assert(SUCCEEDED(HR), "Failed to compile shader: {0}", Err.ErrorMessage());
	}

	/*
		Increment a D3D descriptor handle.
	*/
	void
	IncrementDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE* Handle, int32 Amount)
	{
		Handle->ptr = SIZE_T(INT64(Handle->ptr) + INT64(Amount)); // Increment pointer
	}
    void
	IncrementDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE* Handle, int32 Amount)
    {
		Handle->ptr = UINT64(INT64(Handle->ptr) + INT64(Amount)); // Increment pointer
    }

	/*
		Private accessors.
	*/

	confined ID3D12Fence*&
	GetFence()
	{
		persist ID3D12Fence* Fence = nullptr;
		return Fence;
	}

	confined uint32&
	GetFenceValue()
	{
		persist uint32 Val = 0;
		return Val;
	}

	confined IDXGISwapChain3*&
	GetSwapChain()
	{
		persist IDXGISwapChain3* SwapChain = nullptr;
		return SwapChain;
	}

	confined Texture2D*&
	GetSwapChainBuffer(uint32 Index)
	{
		persist Texture2D* Buffers[D3DContext::SwapChainBufferCount];
		FL_Assert(Index < D3DContext::SwapChainBufferCount, "Index out of bounds.");
		return Buffers[Index];
	}

	/*
		Stall until a command queue is empty.
	*/
	void
	WaitForCommandQueueToFlush(ID3D12CommandQueue* Queue)
	{
		Queue->Signal(GetFence(), ++GetFenceValue());
		if (GetFence()->GetCompletedValue() < GetFenceValue())
		{
			HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			GetFence()->SetEventOnCompletion(GetFenceValue(), EventHandle);
			WaitForSingleObject(EventHandle, INFINITE);
			CloseHandle(EventHandle);
		}
	}

	/*
		Create the D3D swap-chain
	*/
	confined void
	CreateSwapChain(HWND Window, uint32 Width, uint32 Height)
	{
		IDXGIFactory* Factory = nullptr;
		CreateDXGIFactory(IID_PPV_ARGS(&Factory));

		DXGI_SWAP_CHAIN_DESC Desc = {};
		Desc.BufferDesc.Width = Width;
		Desc.BufferDesc.Height = Height;
		Desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		Desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		Desc.SampleDesc = { 1, 0 };
		Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		Desc.BufferCount = D3DContext::SwapChainBufferCount;
		Desc.OutputWindow = Window;
		Desc.Windowed = true;
		Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* Temp = nullptr;
		HResultCall(Factory->CreateSwapChain(D3DContext::GetCommandList().GetCommandQueue(), &Desc, &Temp));
		Temp->QueryInterface(IID_PPV_ARGS(&GetSwapChain()));
		Temp->Release();

		Factory->Release();
	}

	/*
		Create render target views and populate buffer storage
	*/
	confined void
	PopulateSwapChainBuffers()
	{
		for (uint32 i = 0; i < D3DContext::SwapChainBufferCount; i++)
		{
			ID3D12Resource* Texture = nullptr;
			HResultCall(GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&Texture)));
			GPUResource* Res = new GPUResource(Texture, D3D12_RESOURCE_STATE_COMMON);
			GetSwapChainBuffer(i) = new Texture2D(Res, TextureFlag_RenderTarget);
		}
	}

	/*
		Tell D3D to break when we encounter errors
	*/
	confined void
	EnableInfoQueue()
	{
#ifndef NDEBUG
		ID3D12InfoQueue* InfoQueue = nullptr;
		HResultCall(D3DContext::GetDevice()->QueryInterface(IID_PPV_ARGS(&InfoQueue)));

		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		HResultCall(InfoQueue->PushStorageFilter(&NewFilter));
		InfoQueue->Release();
#endif
	}

	confined bool&
	GetInitializedBoolean()
	{
		persist bool Initialized = false;
		return Initialized;
	}

	/*
		Create core D3D resources.
	*/
	void
	D3DContext::Init(HWND Window)
	{
		{ // Enable the debug layer
			ID3D12Debug* Debug = nullptr;
			HResultCall(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
			Debug->EnableDebugLayer();
			Debug->Release();
		}

		{ // Create the D3D device
			HResultCall(D3D12CreateDevice(
				nullptr,
				D3D_FEATURE_LEVEL_12_0,
				IID_PPV_ARGS(&GetDevice())
			));
		}

		// Enable the info queue
		EnableInfoQueue();

		// Create the fence
		GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&GetFence()));

		// Create the command list
		GetCommandList().Init(SwapChainBufferCount);

		// Create the descriptor heaps
		GetCBVSRVUAVDescriptorHeap().Init(DescriptorHeapType_CBV_SRV_UAV, 200);
		GetRTVDescriptorHeap().Init(DescriptorHeapType_RTV, 20);
		GetDSVDescriptorHeap().Init(DescriptorHeapType_DSV, 20);

		{ // Create the D3D swap-chain
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			uint32 Width = ClientRect.right - ClientRect.left;
			uint32 Height = ClientRect.bottom - ClientRect.top;
			CreateSwapChain(Window, Width, Height);
			PopulateSwapChainBuffers();
		}

		WaitForCommandQueueToFlush(D3DContext::GetCommandList().GetCommandQueue());

		GetInitializedBoolean() = true;
	}

	/*
		Release core D3D resources.
	*/
	void
	D3DContext::Shutdown()
	{
		WaitForCommandQueueToFlush(GetCommandList().GetCommandQueue());

		ConstantBuffer::DestroyUpdateQueue();;

		for (uint32 i = 0; i < SwapChainBufferCount; i++)
		{
			delete GetSwapChainBuffer(i);
		}

		GetSwapChain()->Release();
		GetCommandList().Release();
		GetCBVSRVUAVDescriptorHeap().Release();
		GetRTVDescriptorHeap().Release();
		GetDSVDescriptorHeap().Release();
		GetFence()->Release();
		GetDevice()->Release();
	}
	
	/*
		Resize the swap chain and repopulate the buffers and render targets.
	*/
	void
	D3DContext::ResizeSwapChain(uint32 Width, uint32 Height)
	{
		WaitForCommandQueueToFlush(GetCommandList().GetCommandQueue());

		for (uint32 i = 0; i < SwapChainBufferCount; i++)
		{
			delete GetSwapChainBuffer(i);
		}

		GetSwapChain()->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
		PopulateSwapChainBuffers();
	}

	bool
	D3DContext::IsInitialized()
	{
		return GetInitializedBoolean();
	}

	void
	D3DContext::BeginFrame()
	{
		uint32 Frame = GetSwapChain()->GetCurrentBackBufferIndex();
		GetCommandList().NewFrame(Frame);

		BindDescriptorHeap(&GetCBVSRVUAVDescriptorHeap());
	}

	void
	D3DContext::EndFrame()
	{
		uint32 Frame = GetSwapChain()->GetCurrentBackBufferIndex();
		GetSwapChainBuffer(Frame)->GetResource()->TransitionState(D3D12_RESOURCE_STATE_PRESENT);
		ConstantBuffer::DoUpdateQueue(GetSwapChainBufferIndex());
		GetCommandList().Execute();
		HResultCall(GetSwapChain()->Present(0, 0));
	}

	/*
		Set the D3D viewport.
	*/
	void
	D3DContext::SetViewport(float X, float Y, float Width, float Height)
	{
		D3D12_VIEWPORT Viewport;
		Viewport.TopLeftX = X;
		Viewport.TopLeftY = Y;
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;
		Viewport.Width = Width;
		Viewport.Height = Height;
		GetCommandList().Get()->RSSetViewports(1, &Viewport);
	}

	/*
		Set the D3D scissor rect.
	*/
	void
	D3DContext::SetScissor(uint32 X, uint32 Y, uint32 Width, uint32 Height)
	{
		D3D12_RECT ScissorRect;
		ScissorRect.top = X;
		ScissorRect.left = Y;
		ScissorRect.right = Width + X;
		ScissorRect.bottom = Height + Y;
		GetCommandList().Get()->RSSetScissorRects(1, &ScissorRect);
	}

	/*
		Public Accessors
	*/

	ID3D12Device*&
	D3DContext::GetDevice()
	{
		persist ID3D12Device* Device = nullptr;
		return Device;
	}

	CommandList&
	D3DContext::GetCommandList()
	{
		persist CommandList List;
		return List;
	}

    DescriptorHeap&
	D3DContext::GetCBVSRVUAVDescriptorHeap()
    {
		persist DescriptorHeap Heap;
		return Heap;
    }

	DescriptorHeap&
	D3DContext::GetRTVDescriptorHeap()
	{
		persist DescriptorHeap Heap;
		return Heap;
	}

    DescriptorHeap& D3DContext::GetDSVDescriptorHeap()
    {
		persist DescriptorHeap Heap;
		return Heap;
    }

	/*
		Return the current swap chain buffer index.
	*/
    uint32
	D3DContext::GetSwapChainBufferIndex()
    {
		return GetSwapChain()->GetCurrentBackBufferIndex();
    }

	/*
		Return the current back buffer.
	*/
    Texture2D*&
	D3DContext::GetBackBuffer()
    {
		return GetSwapChainBuffer(GetSwapChainBufferIndex());
    }

}
