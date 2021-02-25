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
				-0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,
				-0.5f,  0.5f, -0.5f,

				-0.5f, -0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,
				 0.5f,  0.5f,  0.5f,
				 0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f,  0.5f,
				-0.5f, -0.5f,  0.5f,


				-0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f,  0.5f,
				-0.5f,  0.5f,  0.5f,

				 0.5f,  0.5f,  0.5f,
				 0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,


				-0.5f, -0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,
				 0.5f, -0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,
				-0.5f, -0.5f,  0.5f,
				-0.5f, -0.5f, -0.5f,

				 0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f, -0.5f,
				-0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f, -0.5f,
				 0.5f,  0.5f,  0.5f,
				 0.5f,  0.5f, -0.5f,

				 0.0f, 0.0f,
				 1.0f, 1.0f,
				 1.0f, 0.0f,
				 1.0f, 1.0f,
				 0.0f, 0.0f,
				 0.0f, 1.0f,

				 0.0f, 0.0f,
				 1.0f, 0.0f,
				 1.0f, 1.0f,
				 1.0f, 1.0f,
				 0.0f, 1.0f,
				 0.0f, 0.0f,

				 1.0f, 0.0f,
				 1.0f, 1.0f,
				 0.0f, 1.0f,
				 0.0f, 1.0f,
				 0.0f, 0.0f,
				 1.0f, 0.0f,

				 1.0f, 0.0f,
				 0.0f, 1.0f,
				 1.0f, 1.0f,
				 0.0f, 1.0f,
				 1.0f, 0.0f,
				 0.0f, 0.0f,


				 0.0f, 1.0f,
				 1.0f, 1.0f,
				 1.0f, 0.0f,
				 1.0f, 0.0f,
				 0.0f, 0.0f,
				 0.0f, 1.0f,

				 1.0f, 0.0f,
				 0.0f, 1.0f,
				 0.0f, 0.0f,
				 0.0f, 1.0f,
				 1.0f, 0.0f,
				 1.0f, 1.0f,
			};

			uint32 Attributes[2] = {
				3*sizeof(float),
				2*sizeof(float)
			};
			CubeMesh = new Submesh(new VertexBuffer(Vertices, std::size(Vertices)/5, Attributes, std::size(Attributes)), nullptr);
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
		delete CubeMesh;
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
		ConstantBuffer::BindVS(MVPCBO, 0);

		/*
			Bind shader resource views
		*/
		ShaderResourceView::Bind(SRV, 0);

		/*
			Bind samplers
		*/
		SamplerState::Bind(Sampler, 0);

		/*
			Submit draw call.
		*/
		CubeMesh->Draw();

		// Blit to the current swap chain buffer.
		Texture2D::Copy(BackBuffer, IndirectTexture);
	}

}