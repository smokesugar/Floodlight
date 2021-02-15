#include "DepthStencilView.h"

#include "D3DContext.h"

namespace Floodlight {

	DepthStencilView::DepthStencilView(const Texture2D* Parent)
	{
		ParentResource = Parent->Resource;
		ParentResource->IncrementRef();
		DescriptorIndex = D3DContext::GetDSVDescriptorHeap().GetNewIndex();
		D3DContext::GetDevice()->CreateDepthStencilView(ParentResource->Raw(), nullptr, GetCPUHandle());
	}

	DepthStencilView::~DepthStencilView()
	{
		D3DContext::GetDSVDescriptorHeap().FreeIndex(DescriptorIndex);
		ParentResource->DecrementRef();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView::GetCPUHandle() const
	{
		return D3DContext::GetDSVDescriptorHeap().GetCPUHandleAtIndex(DescriptorIndex);
	}

	/*
		Clear the contents of the depth texture.
	*/
	void
	DepthStencilView::Clear(float Depth)
	{
		D3DContext::GetCommandList().Get()->ClearDepthStencilView(GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, Depth, 0, 0, nullptr);
	}

}