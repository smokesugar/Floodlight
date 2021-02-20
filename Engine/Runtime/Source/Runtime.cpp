#include "Runtime.h"

#include "Floodlight/Utilities/DebugLog.h"
#include "Floodlight/Utilities/Math.h"
#include "Floodlight/System/ECSRegistry.h"
#include "Floodlight/Application/Window.h"
#include "Floodlight/Renderer/D3D/D3DContext.h"
#include "Floodlight/Application/Time.h"

namespace Floodlight {

	
	Runtime::Runtime()
	{
		{
			PipelineStateDesc Desc = {};
			Desc.VertexShader = L"Resources/Shaders/TestShader_v.hlsl";
			Desc.PixelShader = L"Resources/Shaders/TestShader_p.hlsl";
			PSO = new PipelineState(Desc);
		}

		{ // Create the vertex buffer
			float Vertices[] = {
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				

				-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				

				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			};

			VBO = new VertexBuffer(Vertices, sizeof(Vertices), 5 * sizeof(float));
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

			IBO = new IndexBuffer(Indices, sizeof(Indices));
		}

		{ // Create MVP constant buffer
			MVPCBO = new ConstantBuffer(sizeof(matrix));
		}

		{ // Create indirect render targets
			Texture2DDesc Desc = {};
			Desc.Width = 800;
			Desc.Height = 600;
			Desc.Format = RGBA8_UNORM;
			Desc.Flags = TextureFlag_RenderTarget;

			IndirectTexture = new Texture2D(Desc);
			RTV = new RenderTargetView(IndirectTexture);
		}

		{ // Create the depth buffer
			Texture2DDesc Desc = {};
			Desc.Width = 800;
			Desc.Height = 600;
			Desc.Format = D32_FLOAT;
			Desc.Flags = TextureFlag_DepthStencil;

			Texture2D* DepthBuffer = new Texture2D(Desc);
			DSV = new DepthStencilView(DepthBuffer);
			delete DepthBuffer;
		}

		{ // Create the shader resource view
			Texture2DDesc Desc = {};
			Desc.Format = RGBA8_UNORM;
			Desc.Flags = 0;

			uint8* ImageData = LoadNonNativeTexture("Resources/Textures/tyler.jpg", &Desc.Width, &Desc.Height);

			Texture2D* TempTexture = new Texture2D(Desc);
			TempTexture->UploadData(ImageData, Desc.Width*Desc.Height*TextureFormatBPP(Desc.Format));
			SRV = new ShaderResourceView(TempTexture);
			delete TempTexture;

			FreeNonNativeTexture(ImageData);
		}

		{ // Create the sampler state
			Sampler = new SamplerState();
		}
	}

	Runtime::~Runtime()
	{
		delete PSO;
		delete VBO;
		delete IBO;
		delete MVPCBO;
		delete IndirectTexture;
		delete RTV;
		delete DSV;
		delete SRV;
		delete Sampler;
	}

	void
	Runtime::Tick()
	{
		PipelineState::Bind(PSO);

		uint32 Width, Height;
		GetMainWindowSize(&Width, &Height);
		D3DContext::SetViewport(0, 0, (float)Width, (float)Height);
		D3DContext::SetScissor(0, 0, Width, Height);

		Texture2D* BackBuffer = D3DContext::GetBackBuffer();

		if (!AreTextureDescDimensionsAndFormatsTheSame(BackBuffer->GetDesc(), IndirectTexture->GetDesc()))
		{
			FL_Info("Resizing render target.");

			{
				Texture2DDesc WantedDesc = IndirectTexture->GetDesc();
				WantedDesc.Width = BackBuffer->GetDesc().Width;
				WantedDesc.Height = BackBuffer->GetDesc().Height;
				WantedDesc.Format = BackBuffer->GetDesc().Format;

				delete IndirectTexture;
				delete RTV;
				IndirectTexture = new Texture2D(WantedDesc);
				RTV = new RenderTargetView(IndirectTexture);
			}

			{
				Texture2DDesc WantedDesc = {};
				WantedDesc.Width = BackBuffer->GetDesc().Width;
				WantedDesc.Height = BackBuffer->GetDesc().Height;
				WantedDesc.Format = D32_FLOAT;
				WantedDesc.Flags = TextureFlag_DepthStencil;

				delete DSV;
				Texture2D* DepthBuffer = new Texture2D(WantedDesc);
				DSV = new DepthStencilView(DepthBuffer);
				delete DepthBuffer;
			}
		}

		BindRenderTargets(&RTV, 1, DSV);
		RTV->Clear(float4(0.6f, 0.2f, 0.6f, 1.0f));
		DSV->Clear(1.0f);

		/*
			Update the MVP constants and bind the buffer.
		*/
		float AspectRatio = (float)Width / (float)Height;
		matrix MVP = XMMatrixRotationRollPitchYaw(ToRadians(Time::GetTime() * 180.0f), ToRadians(Time::GetTime() * 90.0f), 0.0f) * XMMatrixTranslation(0.0f, 0.0f, 2.0f) * XMMatrixPerspectiveFovLH(ToRadians(80.0f), AspectRatio, 0.1f, 100.0f);
		ConstantBuffer::Update(MVPCBO, &MVP, sizeof(MVP));
		ConstantBuffer::Bind(MVPCBO, 0);

		/*
			Bind shader resource views
		*/
		ShaderResourceView::Bind(SRV, 2);

		/*
			Bind samplers
		*/
		SamplerState::Bind(Sampler, 1);

		/*
			Bind vertices and draw.
		*/
		VertexBuffer::Bind(VBO);
		IndexBuffer::Bind(IBO);
		D3DContext::GetCommandList().Get()->DrawInstanced(VBO->GetCount(), 1, 0, 0);

		// Blit to the current swap chain buffer.
		Texture2D::Copy(BackBuffer, IndirectTexture);
	}

}