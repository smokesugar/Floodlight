#include "Runtime.h"

#include "Floodlight/Utilities/DebugLog.h"
#include "Floodlight/Utilities/Math.h"
#include "Floodlight/System/ECSRegistry.h"
#include "Floodlight/Application/Window.h"
#include "Floodlight/Renderer/D3D/D3DContext.h"
#include "Floodlight/Application/Time.h"

namespace Floodlight {

	struct MVPConstants
	{
		matrix MVP;
		matrix Model;
	};
	
	Runtime::Runtime()
	{
		{
			PipelineStateDesc Desc = {};
			Desc.VertexShader = L"Resources/Shaders/TestShader_v.hlsl";
			Desc.PixelShader = L"Resources/Shaders/TestShader_p.hlsl";
			Desc.DSVFormat = D32_FLOAT;
			Desc.NumRenderTargets = 1;
			Desc.RTVFormats[0] = RGBA8_UNORM;
			PSO = new PipelineState(Desc);
		}

		{ // Create the vertex buffer
			TestMesh = LoadGLTF("Resources/Models/Sponza.gltf");

		}

		{ // Create MVP constant buffer
			MVPCBO = new ConstantBuffer(sizeof(MVPConstants));
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

			uint8* ImageData = LoadNonNativeTexture("Resources/Textures/checkerboard.png", &Desc.Width, &Desc.Height);

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
		delete TestMesh;
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
		MVPConstants MVP;
		MVP.Model = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixTranslation(0.0f, -2.0f, 0.0f);
		matrix View = XMMatrixInverse(nullptr, XMMatrixRotationRollPitchYaw(ToRadians(Time::GetTime() * 100.0f), ToRadians(Time::GetTime() * 60), 0.0f));
		MVP.MVP = MVP.Model * View * XMMatrixPerspectiveFovLH(ToRadians(80.0f), AspectRatio, 0.1f, 100.0f);
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
		for (uint32 i = 0; i < TestMesh->Submeshes.size(); i++)
		{
			auto VB = TestMesh->Submeshes[i].VB;
			auto IB = TestMesh->Submeshes[i].IB;
			VertexBuffer::Bind(VB->GetViewsPointer(), VB->GetNumViews());
			IndexBuffer::Bind(IB);
			D3DContext::GetCommandList().Get()->DrawIndexedInstanced(IB->GetNumIndices(), 1, 0, 0, 0);
		}

		// Blit to the current swap chain buffer.
		Texture2D::Copy(BackBuffer, IndirectTexture);
	}

}