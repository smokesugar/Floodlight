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

	confined PipelineState*&
	GetPipelineState()
	{
		persist PipelineState* PSO = nullptr;
		return PSO;
	}

	confined VertexBuffer*&
	GetVertexBuffer()
	{
		persist VertexBuffer* VB = nullptr;
		return VB;
	}

	confined IndexBuffer*&
	GetIndexBuffer()
	{
		persist IndexBuffer* IB = nullptr;
		return IB;
	}

	confined ConstantBuffer*&
	GetMVPConstantBuffer()
	{
		persist ConstantBuffer* CB = nullptr;
		return CB;
	}

	confined Texture2D*&
	GetIndirectTexture()
	{
		persist Texture2D* Texture = nullptr;
		return Texture;
	}

	confined RenderTargetView*&
	GetIndirectRTV()
	{
		persist RenderTargetView* RTV = nullptr;
		return RTV;
	}

	confined DepthStencilView*&
	GetDepthStencilView()
	{
		persist DepthStencilView* DSV = nullptr;
		return DSV;
	}

	confined void
	CreateDepthBuffer(uint32 Width, uint32 Height)
	{
		Texture2DDesc Desc = {};
		Desc.Width = Width;
		Desc.Height = Height;
		Desc.Format = D32_FLOAT;
		Desc.Flags = TextureFlag_DepthStencil;

		Texture2D* DepthBuffer = new Texture2D(Desc);
		GetDepthStencilView() = new DepthStencilView(DepthBuffer);
		delete DepthBuffer;
	}

	/*
		Create all objects used in drawing
	*/
	confined void
	InitializeAssets()
	{
		{ // Create pipeline state
			PipelineStateDesc Desc = {};
			Desc.VertexShader = L"Resources/Shaders/TestShader_v.hlsl";
			Desc.PixelShader = L"Resources/Shaders/TestShader_p.hlsl";
			GetPipelineState() = new PipelineState(Desc);
		}

		{ // Create the vertex buffer
			float Vertices[] =
			{
				-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
				 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
				 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
				-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
				 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
			};
			
			GetVertexBuffer() = new VertexBuffer(Vertices, sizeof(Vertices), 7*sizeof(float));
		}

		{ // Create the index buffer
			uint32 Indices[] =
			{
				0, 1, 2,    // side 1
				2, 1, 3,
				4, 0, 6,    // side 2
				6, 0, 2,
				7, 5, 6,    // side 3
				6, 5, 4,
				3, 1, 7,    // side 4
				7, 1, 5,
				4, 5, 0,    // side 5
				0, 5, 1,
				3, 7, 2,    // side 6
				2, 7, 6,
			};

			GetIndexBuffer() = new IndexBuffer(Indices, sizeof(Indices));
		}

		{ // Create MVP constant buffer
			GetMVPConstantBuffer() = new ConstantBuffer(sizeof(matrix));
		}

		{ // Create indirect render targets
			Texture2DDesc Desc = {};
			Desc.Width = 800;
			Desc.Height = 600;
			Desc.Format = RGBA8_UNORM;
			Desc.Flags = TextureFlag_RenderTarget;
			
			GetIndirectTexture() = new Texture2D(Desc);
			GetIndirectRTV() = new RenderTargetView(GetIndirectTexture());

			CreateDepthBuffer(Desc.Width, Desc.Height);
		}
	}

	/*
		Release all the objects used in drawing
	*/
	confined void
	ReleaseAssets()
	{
		delete GetIndirectTexture();
		delete GetIndirectRTV();
		delete GetDepthStencilView();

		delete GetMVPConstantBuffer();
		delete GetIndexBuffer();
		delete GetVertexBuffer();

		delete GetPipelineState();
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

		InitializeAssets();

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

		ConstantBuffer::DestroyUpdateQueue();

		ReleaseAssets();

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
		Flush command queue and do other shit.
	*/
	void
	D3DContext::Render(uint32 Width, uint32 Height, float DeltaTime, float Time)
	{
		/*
			New frame, new index.
		*/
		uint32 Frame = GetSwapChain()->GetCurrentBackBufferIndex();
		GetCommandList().NewFrame(Frame);

		/*
			Bind the pipeline state and graphics root signature.
		*/
		PipelineState::Bind(GetPipelineState());

		/*
			Set viewports and clear and bind the render target.
		*/
		SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
		SetScissor(0, 0, Width, Height);

		
		{ // Resize the texture if required
			Texture2D* SwapChainBuffer = GetSwapChainBuffer(Frame);
			Texture2D* IndirectTexture = GetIndirectTexture();
			if (!AreTextureDescDimensionsAndFormatsTheSame(SwapChainBuffer->GetDesc(), IndirectTexture->GetDesc()))
			{
				FL_Info("Resizing render target.");

				{
					Texture2DDesc WantedDesc = GetIndirectTexture()->GetDesc();
					WantedDesc.Width = SwapChainBuffer->GetDesc().Width;
					WantedDesc.Height = SwapChainBuffer->GetDesc().Height;
					WantedDesc.Format = SwapChainBuffer->GetDesc().Format;

					delete GetIndirectTexture();
					delete GetIndirectRTV();
					GetIndirectTexture() = new Texture2D(WantedDesc);
					GetIndirectRTV() = new RenderTargetView(GetIndirectTexture());
				}

				{
					Texture2DDesc WantedDesc = {};
					WantedDesc.Width = SwapChainBuffer->GetDesc().Width;
					WantedDesc.Height = SwapChainBuffer->GetDesc().Height;
					WantedDesc.Format = D32_FLOAT;
					WantedDesc.Flags = TextureFlag_DepthStencil;

					delete GetDepthStencilView();
					Texture2D* DepthBuffer = new Texture2D(WantedDesc);
					GetDepthStencilView() = new DepthStencilView(DepthBuffer);
					delete DepthBuffer;
				}
			}
		}

		Texture2D* SwapChainBuffer = GetSwapChainBuffer(Frame);
		Texture2D* IndirectTexture = GetIndirectTexture();
		RenderTargetView* RTV = GetIndirectRTV();
		DepthStencilView* DSV = GetDepthStencilView();

		BindRenderTargets(&RTV, 1, DSV);
		RTV->Clear(float4(0.6f, 0.2f, 0.6f, 1.0f));
		DSV->Clear(1.0f);

		/*
			Bind the descriptor heap
		*/
		BindDescriptorHeap(&GetCBVSRVUAVDescriptorHeap());

		/*
			Update the MVP constants and bind the buffer.
		*/
		persist float Accum = 1.0f;
		Accum += DeltaTime;
		if (Accum >= 1.0f) {
			Accum = 0.0f;
			float AspectRatio = (float)Width / (float)Height;
			matrix MVP = XMMatrixRotationRollPitchYaw(ToRadians(Time * 121.0f), ToRadians(Time * 365.0f), 0.0f) * XMMatrixTranslation(0.0f, 0.0f, 2.0f) * XMMatrixPerspectiveFovLH(ToRadians(80.0f), AspectRatio, 0.1f, 100.0f);
			ConstantBuffer::Update(GetMVPConstantBuffer(), &MVP, sizeof(MVP));
			ConstantBuffer::Bind(GetMVPConstantBuffer(), 0);
		}

		/*
			Bind vertices and draw.
		*/
		VertexBuffer::Bind(GetVertexBuffer());
		IndexBuffer::Bind(GetIndexBuffer());
		GetCommandList().Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GetCommandList().Get()->DrawIndexedInstanced(GetIndexBuffer()->GetCount(), 1, 0, 0, 0);

		// Blit to the current swap chain buffer.
		Texture2D::Copy(SwapChainBuffer, IndirectTexture);

		/*
			Indicate that we are presenting the render target.
		*/
		SwapChainBuffer->GetResource()->TransitionState(D3D12_RESOURCE_STATE_PRESENT);

		/*
			Do cbuffer update queue for this frame.
		*/
		ConstantBuffer::DoUpdateQueue(GetSwapChainBufferIndex());

		/*
			Indicate we are finished with recording commands
		*/
		GetCommandList().Execute();

		/*
			Present image.
		*/
		HResultCall(GetSwapChain()->Present(0, 0));
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

}
