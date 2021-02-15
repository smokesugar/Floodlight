#include "PipelineState.h"

#include <d3dcompiler.h>
#include <comdef.h>

#include "D3DContext.h"

namespace Floodlight {

	/*
		Helper function that compiles a shader from a file path.
	*/
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
		Helper function to extract the input layout from the vertex shader.
	*/
	confined std::vector<D3D12_INPUT_ELEMENT_DESC>
	ExtractInputLayout(ID3D12ShaderReflection* VSReflection)
	{
		D3D12_SHADER_DESC ShaderDesc = {};
		VSReflection->GetDesc(&ShaderDesc);

		std::vector<D3D12_INPUT_ELEMENT_DESC> IED = {};
		for (uint32 i = 0; i < ShaderDesc.InputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
			VSReflection->GetInputParameterDesc(i, &paramDesc);

			// fill out input element desc
			D3D12_INPUT_ELEMENT_DESC ElementDesc;
			ElementDesc.SemanticName = paramDesc.SemanticName;
			ElementDesc.SemanticIndex = paramDesc.SemanticIndex;
			ElementDesc.InputSlot = 0;
			ElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			ElementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			ElementDesc.InstanceDataStepRate = 0;

			// determine DXGI format
			if (paramDesc.Mask == 1)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) ElementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) ElementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) ElementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (paramDesc.Mask <= 3)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) ElementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) ElementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) ElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (paramDesc.Mask <= 7)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (paramDesc.Mask <= 15)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) ElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			//save element desc
			IED.push_back(ElementDesc);
		}

		return IED;
	}

	//confined D3D12_ROOT_SIGNATURE_DESC
	//ExtractRootSignature(ID3D12ShaderReflection* VSReflection)
	//{
	//	
	//}

	/*
		Pipeline state constructor. Create a root signature and pipeline state object by shader reflection.
	*/
	PipelineState::PipelineState(const PipelineStateDesc& Desc)
	{
		// Compile the shaders
		ID3DBlob* VertexShader = CompileShader(Desc.VertexShader, "main", "vs_5_0");
		ID3DBlob* PixelShader = CompileShader(Desc.PixelShader, "main", "ps_5_0");

		// Create reflections
		ID3D12ShaderReflection* VSReflection = nullptr;
		D3DReflect(VertexShader->GetBufferPointer(), VertexShader->GetBufferSize(), IID_PPV_ARGS(&VSReflection));

		{ // Create the root signature
			D3D12_DESCRIPTOR_RANGE Ranges[1];

			Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			Ranges[0].NumDescriptors = 1;
			Ranges[0].BaseShaderRegister = 0;
			Ranges[0].RegisterSpace = 0;
			Ranges[0].OffsetInDescriptorsFromTableStart = 0;

			D3D12_ROOT_PARAMETER Params[1];

			Params[0] = {};
			Params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			Params[0].DescriptorTable.NumDescriptorRanges = 1;
			Params[0].DescriptorTable.pDescriptorRanges = &Ranges[0];
			Params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			D3D12_ROOT_SIGNATURE_DESC Desc = {};
			Desc.NumParameters = (uint32)std::size(Params);
			Desc.pParameters = Params;
			Desc.NumStaticSamplers = 0;
			Desc.pStaticSamplers = nullptr;
			Desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			ID3DBlob* Blob = nullptr;
			ID3DBlob* Error = nullptr;
			D3D12SerializeRootSignature(&Desc, D3D_ROOT_SIGNATURE_VERSION_1, &Blob, &Error);
			D3DContext::GetDevice()->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(&RootSignature));

			if (Error) Error->Release();
			Blob->Release();
		}

		{ // Create the PSO
			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};

			// Shaders
			PSODesc.VS.BytecodeLength = VertexShader->GetBufferSize();
			PSODesc.VS.pShaderBytecode = VertexShader->GetBufferPointer();
			PSODesc.PS.BytecodeLength = PixelShader->GetBufferSize();
			PSODesc.PS.pShaderBytecode = PixelShader->GetBufferPointer();

			// Input layout
			auto IED = ExtractInputLayout(VSReflection);
			PSODesc.InputLayout.NumElements = (uint32)IED.size();
			PSODesc.InputLayout.pInputElementDescs = IED.data();

			// Root signature
			PSODesc.pRootSignature = RootSignature;

			// Rasterizer desc
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
			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			{
				PSODesc.BlendState.RenderTarget[i] = 
				{
					FALSE,FALSE,
					D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
					D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
					D3D12_LOGIC_OP_NOOP,
					D3D12_COLOR_WRITE_ENABLE_ALL,
				};;
			}
			PSODesc.BlendState.AlphaToCoverageEnable = FALSE;
			PSODesc.BlendState.IndependentBlendEnable = FALSE;

			// Depth Stencil State
			PSODesc.DepthStencilState.DepthEnable = TRUE;
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			PSODesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 1;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.SampleDesc = { 1, 0 };

			// Create the PSO
			D3DContext::GetDevice()->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSO));
		}

		// Release outstanding references
		VSReflection->Release();
		VertexShader->Release();
		PixelShader->Release();
	}

	PipelineState::~PipelineState()
	{
		PSO->Release();
		RootSignature->Release();
	}

	// Bind a pipeline state
	void
	PipelineState::Bind(const PipelineState* PS)
	{
		D3DContext::GetCommandList().Get()->SetPipelineState(PS->PSO);
		D3DContext::GetCommandList().Get()->SetGraphicsRootSignature(PS->RootSignature);
	}

}