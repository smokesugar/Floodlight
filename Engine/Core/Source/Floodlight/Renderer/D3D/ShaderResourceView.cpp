#include "ShaderResourceView.h"

#include "D3DContext.h"
#include "PipelineState.h"

namespace Floodlight {

	ShaderResourceView::ShaderResourceView(const Texture2D* Parent)
	{
		ParentResource = Parent->Resource;
		ParentResource->IncrementRef();
		DescriptorIndex = D3DContext::GetCBVSRVUAVDescriptorHeap().GetNewIndex();
		auto Handle = D3DContext::GetCBVSRVUAVDescriptorHeap().GetCPUHandleAtIndex(DescriptorIndex);
		D3DContext::GetDevice()->CreateShaderResourceView(ParentResource->Raw(), nullptr, Handle);
	}

	ShaderResourceView::~ShaderResourceView()
	{
		D3DContext::GetCBVSRVUAVDescriptorHeap().FreeIndex(DescriptorIndex);
		ParentResource->DecrementRef();
	}

	/*
		Issue a command to bind a shader resource view.
	*/
	void
	ShaderResourceView::Bind(const ShaderResourceView* SRV, uint32 Register)
	{
		auto Handle = D3DContext::GetCBVSRVUAVDescriptorHeap().GetGPUHandleAtIndex(SRV->DescriptorIndex);
		uint32 RootSigIndex = PipelineState::GetCurrentlyBound()->GetRootSignatureIndexOfSRVAtRegister(Register);
		D3DContext::GetCommandList().Get()->SetGraphicsRootDescriptorTable(RootSigIndex, Handle);
	}

}