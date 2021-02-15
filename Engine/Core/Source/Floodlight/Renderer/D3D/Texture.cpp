#include "Texture.h"

#include "D3DContext.h"

namespace Floodlight {

	Texture2D::Texture2D(Texture2DDesc Desc)
	{
		TexDesc = Desc;

		bool RenderTarget = Desc.Flags & TextureFlag_RenderTarget;
		bool DepthStencil = Desc.Flags & TextureFlag_DepthStencil;

		D3D12_HEAP_PROPERTIES HeapProps = {};
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC ResDesc = {};
		ResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		ResDesc.Alignment = 0;
		ResDesc.Width = Desc.Width;
		ResDesc.Height = Desc.Height;
		ResDesc.DepthOrArraySize = 1;
		ResDesc.MipLevels = 1;
		ResDesc.Format = (DXGI_FORMAT)Desc.Format;
		ResDesc.SampleDesc = { 1, 0 };
		ResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		ResDesc.Flags = (RenderTarget ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE) | (DepthStencil ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE);

		D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_COMMON;
		Resource = new GPUResource(HeapProps, ResDesc, ResourceState);
		Resource->IncrementRef();
	}

	Texture2D::Texture2D(GPUResource* Res, TextureFlags Flags)
	{
		Resource = Res;
		Resource->IncrementRef();

		// Create a descriptor from the resource.
		D3D12_RESOURCE_DESC DxDesc = Res->Raw()->GetDesc();
		TexDesc.Width = (uint32)DxDesc.Width;
		TexDesc.Height = (uint32)DxDesc.Height;
		TexDesc.Format = DxDesc.Format;
		TexDesc.Flags = Flags;
	}

	Texture2D::~Texture2D()
	{
		Resource->DecrementRef();
	}

    void Texture2D::Copy(const Texture2D* Dest, const Texture2D* Src)
    {
		FL_Assert(AreTextureDescDimensionsAndFormatsTheSame(Dest->GetDesc(), Src->GetDesc()), "Trying to copy a texture when the destination and source have mismatching dimensions/formats.");

		Dest->Resource->TransitionState(D3D12_RESOURCE_STATE_COPY_DEST);
		Src->Resource->TransitionState(D3D12_RESOURCE_STATE_COPY_SOURCE);
		D3DContext::GetCommandList().Get()->CopyResource(Dest->Resource->Raw(), Src->Resource->Raw());
    }

}