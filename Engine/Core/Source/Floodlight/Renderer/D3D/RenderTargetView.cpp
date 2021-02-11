#include "RenderTargetView.h"

#include "D3DContext.h"
#include "ResourceState.h"

namespace Floodlight {

	RenderTargetView::RenderTargetView(const Texture2D* Parent)
	{
		FL_Assert(Parent->GetDesc().Flags & TextureFlag_RenderTarget, "Cannot create a render target with this texture; it was not specified with the render target flag.");

		ParentResource = Parent->Resource;
		DescriptorIndex = D3DContext::GetRTVDescriptorHeap().GetNewIndex();
		D3DContext::GetDevice()->CreateRenderTargetView(ParentResource, nullptr, D3DContext::GetRTVDescriptorHeap().GetCPUHandleAtIndex(DescriptorIndex));
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

	/*
		Take an array of render targets and bind them.
	*/
	void
	BindRenderTargets(RenderTargetView** RenderTargets, uint32 NumRenderTargets, DepthStencilView* DSV)
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RTVHandles;
		for (uint32 i = 0; i < NumRenderTargets; i++)
		{
			RTVHandles.push_back(RenderTargets[i]->GetCPUHandle());
			TransitionResourceState(RenderTargets[i]->GetParentResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		
		if (DSV)
		{
			TransitionResourceState(DSV->GetParentResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_WRITE);
			D3DContext::GetCommandList().Get()->OMSetRenderTargets(NumRenderTargets, RTVHandles.data(), false, &DSV->GetCPUHandle());
		}
		else
		{
			D3DContext::GetCommandList().Get()->OMSetRenderTargets(NumRenderTargets, RTVHandles.data(), false, nullptr);
		}
	}

}