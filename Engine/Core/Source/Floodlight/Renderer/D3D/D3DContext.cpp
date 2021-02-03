#include "D3DContext.h"

#include <dxgi1_6.h>
#include <comdef.h>
#include <d3dcompiler.h>

#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Utilities/Math.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

namespace Floodlight {

	/*
		In debug mode, break on failure.
	*/
	confined void
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

	confined ID3D12Resource*&
	GetSwapChainBuffer(uint32 Index)
	{
		persist ID3D12Resource* Buffers[D3DContext::SwapChainBufferCount];
		FL_Assert(Index < D3DContext::SwapChainBufferCount, "Index out of bounds.");
		return Buffers[Index];
	}

	confined ID3D12DescriptorHeap*&
	GetRTVDescriptorHeap()
	{
		persist ID3D12DescriptorHeap* DescHeap = nullptr;
		return DescHeap;
	}

	/*
		Retrieve the current render target view.
	*/
	confined D3D12_CPU_DESCRIPTOR_HANDLE
	GetRTV(uint32 Index)
	{
		uint32 DescriptorSize = D3DContext::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		auto handle = GetRTVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
		IncrementDescriptorHandle(&handle, DescriptorSize*Index);
		return handle;
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
	CreateRTVsAndPopulateSwapChainBuffers()
	{
		{ // Create the descriptor heap for the render target views
			D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
			Desc.NumDescriptors = D3DContext::SwapChainBufferCount;
			Desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			Desc.NodeMask = 0;
			HResultCall(D3DContext::GetDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&GetRTVDescriptorHeap())));
		}

		{ // Retrieve all the swap-chain buffers and create the render target views
			uint32 DescriptorSize = D3DContext::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE RTVHeapPtr = GetRTVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

			for (uint32 i = 0; i < D3DContext::SwapChainBufferCount; i++)
			{
				HResultCall(GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&GetSwapChainBuffer(i))));
				D3DContext::GetDevice()->CreateRenderTargetView(GetSwapChainBuffer(i), nullptr, RTVHeapPtr);
				IncrementDescriptorHandle(&RTVHeapPtr, DescriptorSize);
			}
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

	confined ID3D12RootSignature*&
	GetRootSignature()
	{
		persist ID3D12RootSignature* Signature = nullptr;
		return Signature;
	}

	confined ID3D12PipelineState*&
	GetPipelineState()
	{
		persist ID3D12PipelineState* PSO = nullptr;
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

	confined ConstantBuffer*&
	GetTestConstantBuffer()
	{
		persist ConstantBuffer* CB = nullptr;
		return CB;
	}

	confined ID3DBlob*
	CompileShader(const wchar_t* File, const char* EntryPoint, const char* Profile)
	{
		#ifdef NDEBUG
			UINT CompileFlags = 0;
		#else
			UINT CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;;
		#endif

		ID3DBlob* ShaderBlob = nullptr;
		ID3DBlob* ErrorBlob = nullptr;
		HRESULT HR = D3DCompileFromFile(File, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint, Profile, 0, 0, &ShaderBlob, &ErrorBlob);

		if (FAILED(HR)) {
			if (ErrorBlob) {
				FL_Assert(false, "{0}", (const char*)ErrorBlob->GetBufferPointer());
			}
			else {
				_com_error Err(HR);
				FL_Assert(false, "Failed to compile shader: {0}", Err.ErrorMessage());
			}
		}

		if (ErrorBlob) ErrorBlob->Release();
		return ShaderBlob;
	}

	/*
		Create all objects used in drawing
	*/
	confined void
	InitializeAssets()
	{
		{ // Create an empty root signature
			D3D12_DESCRIPTOR_RANGE Ranges[2];

			Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			Ranges[0].NumDescriptors = 1;
			Ranges[0].BaseShaderRegister = 0;
			Ranges[0].RegisterSpace = 0;
			Ranges[0].OffsetInDescriptorsFromTableStart = 0;

			Ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			Ranges[1].NumDescriptors = 1;
			Ranges[1].BaseShaderRegister = 1;
			Ranges[1].RegisterSpace = 0;
			Ranges[1].OffsetInDescriptorsFromTableStart = 0;

			D3D12_ROOT_PARAMETER Params[2];

			Params[0] = {};
			Params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			Params[0].DescriptorTable.NumDescriptorRanges = 1;
			Params[0].DescriptorTable.pDescriptorRanges = &Ranges[0];

			Params[1] = {};
			Params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			Params[1].DescriptorTable.NumDescriptorRanges = 1;
			Params[1].DescriptorTable.pDescriptorRanges = &Ranges[1];

			D3D12_ROOT_SIGNATURE_DESC Desc = {};
			Desc.NumParameters = std::size(Params);
			Desc.pParameters = Params;
			Desc.NumStaticSamplers = 0;
			Desc.pStaticSamplers = nullptr;
			Desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			ID3DBlob* Signature;
			ID3DBlob* Error;

			HResultCall(D3D12SerializeRootSignature(&Desc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &Error));
			HResultCall(D3DContext::GetDevice()->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&GetRootSignature())));

			if (Error)
				Error->Release();
			Signature->Release();
		}

		{ // Create shaders and pipeline state
			ID3DBlob* VertexShader = CompileShader(L"Resources/Shaders/TestShader.hlsl", "VSMain", "vs_5_0");
			ID3DBlob* PixelShader = CompileShader(L"Resources/Shaders/TestShader.hlsl", "PSMain", "ps_5_0");

			D3D12_INPUT_ELEMENT_DESC InputLayout[] =
			{
				{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};

			// Input layout
			PSODesc.InputLayout.NumElements = (uint32)std::size(InputLayout);
			PSODesc.InputLayout.pInputElementDescs = InputLayout;
			PSODesc.pRootSignature = GetRootSignature();

			// Shaders
			PSODesc.VS.pShaderBytecode = VertexShader->GetBufferPointer();
			PSODesc.VS.BytecodeLength = VertexShader->GetBufferSize();
			PSODesc.PS.pShaderBytecode = PixelShader->GetBufferPointer();
			PSODesc.PS.BytecodeLength = PixelShader->GetBufferSize();

			// Rasterizer State
			PSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
			PSODesc.RasterizerState.FrontCounterClockwise = FALSE;
			PSODesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			PSODesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			PSODesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			PSODesc.RasterizerState.DepthClipEnable = TRUE;
			PSODesc.RasterizerState.MultisampleEnable = FALSE;
			PSODesc.RasterizerState.AntialiasedLineEnable = FALSE;
			PSODesc.RasterizerState.ForcedSampleCount = 0;
			PSODesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			// Blend State
			PSODesc.BlendState.AlphaToCoverageEnable = FALSE;
			PSODesc.BlendState.IndependentBlendEnable = FALSE;
			const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
			{
				FALSE,FALSE,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};
			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				PSODesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

			// Depth Stencil State
			PSODesc.DepthStencilState.DepthEnable = FALSE;
			PSODesc.DepthStencilState.StencilEnable = FALSE;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 1;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.SampleDesc = { 1, 0 };

			HResultCall(D3DContext::GetDevice()->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GetPipelineState())));

			VertexShader->Release();
			PixelShader->Release();
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
			GetTestConstantBuffer() = new ConstantBuffer(sizeof(matrix));
		}
	}

	/*
		Release all the objects used in drawing
	*/
	confined void
	ReleaseAssets()
	{
		delete GetMVPConstantBuffer();
		delete GetTestConstantBuffer();
		delete GetIndexBuffer();
		delete GetVertexBuffer();
		GetPipelineState()->Release();
		GetRootSignature()->Release();
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
		GetCBVSRVUAVDescriptorHeap().Init(200);

		{ // Create the D3D swap-chain
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			uint32 Width = ClientRect.right - ClientRect.left;
			uint32 Height = ClientRect.bottom - ClientRect.top;
			CreateSwapChain(Window, Width, Height);
			CreateRTVsAndPopulateSwapChainBuffers();
		}

		InitializeAssets();

		WaitForCommandQueueToFlush(D3DContext::GetCommandList().GetCommandQueue());
	}

	/*
		Release core D3D resources.
	*/
	void
	D3DContext::Shutdown()
	{
		WaitForCommandQueueToFlush(GetCommandList().GetCommandQueue());

		ReleaseAssets();

		for (uint32 i = 0; i < SwapChainBufferCount; i++)
			GetSwapChainBuffer(i)->Release();

		GetRTVDescriptorHeap()->Release();
		GetSwapChain()->Release();
		GetCommandList().Release();
		GetCBVSRVUAVDescriptorHeap().Release();
		GetFence()->Release();
		GetDevice()->Release();
	}
	
	/*
		Resize the swap chain and repopulate the buffers and render targets.
	*/
	void
	D3DContext::ResizeSwapChain(uint32 Width, uint32 Height)
	{
		GetRTVDescriptorHeap()->Release();
		for (uint32 i = 0; i < SwapChainBufferCount; i++)
			GetSwapChainBuffer(i)->Release();

		GetSwapChain()->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
		CreateRTVsAndPopulateSwapChainBuffers();
	}

	bool
	D3DContext::IsInitialized()
	{
		return GetDevice() != nullptr;
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
		Create a transition resource barrier.
	*/
	confined D3D12_RESOURCE_BARRIER
	CreateTransitionBarrier(ID3D12Resource* Resource, D3D12_RESOURCE_STATES Start, D3D12_RESOURCE_STATES End)
	{
		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = Resource;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Barrier.Transition.StateBefore = Start;
		Barrier.Transition.StateAfter = End;
		return Barrier;
	}

	/*
		Flush command queue and do other shit.
	*/
	void
	D3DContext::Render(uint32 Width, uint32 Height, float Time)
	{
		/*
			New frame, new index.
		*/
		uint32 Frame = GetSwapChain()->GetCurrentBackBufferIndex();
		GetCommandList().NewFrame(Frame);
		
		/*
			Indicate that we are rendering to the render target.
		*/
		D3D12_RESOURCE_BARRIER BarrierToRenderTarget = CreateTransitionBarrier(GetSwapChainBuffer(Frame), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		GetCommandList().Get()->ResourceBarrier(1, &BarrierToRenderTarget);

		/*
			Bind the pipeline state and graphics root signature.
		*/
		GetCommandList().Get()->SetGraphicsRootSignature(GetRootSignature());
		GetCommandList().Get()->SetPipelineState(GetPipelineState());

		/*
			Set viewports and clear and bind the render target.
		*/
		auto RTV = GetRTV(Frame);
		float4 ClearColor(0.6f, 0.2f, 0.6f, 1.0f);
		SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
		SetScissor(0, 0, Width, Height);
		GetCommandList().Get()->ClearRenderTargetView(RTV, FloatPtr(ClearColor), 0, nullptr);
		GetCommandList().Get()->OMSetRenderTargets(1, &RTV, false, nullptr);

		/*
			Bind the descriptor heap
		*/
		BindDescriptorHeap(&GetCBVSRVUAVDescriptorHeap());

		/*
			Update the MVP constants and bind the buffer.
		*/
		float AspectRatio = (float)Width / (float)Height;
		matrix MVP = XMMatrixRotationRollPitchYaw(0.0f, ToRadians(Time * 90.0f), ToRadians(Time * 180.0f)) * XMMatrixTranslation(0.0f, 0.0f, 2.0f) * XMMatrixPerspectiveFovLH(ToRadians(80.0f), AspectRatio, 0.1f, 100.0f);
		GetMVPConstantBuffer()->Update(&MVP, sizeof(MVP));
		BindConstantBuffer(GetMVPConstantBuffer(), 0);
		matrix TestOffset = XMMatrixTranslation(sinf(Time), 0.0f, 0.0f);
		GetTestConstantBuffer()->Update(&TestOffset, sizeof(TestOffset));
		BindConstantBuffer(GetTestConstantBuffer(), 1);

		/*
			Bind vertices and draw.
		*/
		BindVertexBuffer(GetVertexBuffer());
		BindIndexBuffer(GetIndexBuffer());
		GetCommandList().Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GetCommandList().Get()->DrawIndexedInstanced(GetIndexBuffer()->GetCount(), 1, 0, 0, 0);

		/*
			Indicate that we are presenting the render target.
		*/
		D3D12_RESOURCE_BARRIER BarrierToPresent = CreateTransitionBarrier(GetSwapChainBuffer(Frame), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		GetCommandList().Get()->ResourceBarrier(1, &BarrierToPresent);

		// Indicate we are finished with recording commands
		GetCommandList().Execute();

		// Present buffer
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

}
