#include "RenderTargetView.h"

#include "D3DContext.h"
#include "ResourceState.h"

namespace Floodlight {

	RenderTargetView::RenderTargetView(const Texture2D* Parent)
	{
		Init(Parent->Resource);
	}

	RenderTargetView::RenderTargetView(ID3D12Resource* Parent)
	{
		Init(Parent);
	}

	RenderTargetView::~RenderTargetView()
	{
		D3DContext::GetRTVDescriptorHeap().FreeIndex(DescriptorIndex);
	}

	/*
		Clear the render target.
	*/
	void
	RenderTargetView::Clear(float4 Color)
	{
		D3DContext::GetCommandList().Get()->ClearRenderTargetView(GetCPUHandle(), FloatPtr(Color), 0, nullptr);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE
	RenderTargetView::GetCPUHandle() const
	{
		return D3DContext::GetRTVDescriptorHeap().GetCPUHandleAtIndex(DescriptorIndex);
	}

	void
	RenderTargetView::Init(ID3D12Resource* Parent)
	{
		ParentResource = Parent;
		DescriptorIndex = D3DContext::GetRTVDescriptorHeap().GetNewIndex();
		D3DContext::GetDevice()->CreateRenderTargetView(Parent, nullptr, D3DContext::GetRTVDescriptorHeap().GetCPUHandleAtIndex(DescriptorIndex));
	}

	/*
		Take an array of render targets and bind them.
	*/
	void
	BindRenderTargets(RenderTargetView** RenderTargets, uint32 NumRenderTargets)
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RTVHandles;
		for (uint32 i = 0; i < NumRenderTargets; i++)
		{
			RTVHandles.push_back(RenderTargets[i]->GetCPUHandle());
			TransitionResourceState(RenderTargets[i]->GetParentResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		
		D3DContext::GetCommandList().Get()->OMSetRenderTargets(NumRenderTargets, RTVHandles.data(), false, nullptr);
	}

}