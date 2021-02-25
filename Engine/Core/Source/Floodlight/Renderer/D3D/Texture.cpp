#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "D3DContext.h"

namespace Floodlight {

	/*
		Texture2D constructor.
	*/
	Texture2D::Texture2D(Texture2DDesc Desc)
	{
		// Save the desc
		TexDesc = Desc;

		// Booleans that store the state of the flags.
		bool RenderTarget = Desc.Flags & TextureFlag_RenderTarget;
		bool DepthStencil = Desc.Flags & TextureFlag_DepthStencil;
		bool Staging = Desc.Flags & TextureFlag_Staging;

		// Heap properties
		D3D12_HEAP_PROPERTIES HeapProps = CreateHeapProperties(Staging ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
	
		// Resource desc
		D3D12_RESOURCE_FLAGS Flags = (RenderTarget ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE) | (DepthStencil ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE);
		D3D12_RESOURCE_DESC ResDesc;
		if (!Staging)
		{
			ResDesc = CreateTexture2DResourceDesc(Desc.Width, Desc.Height, (DXGI_FORMAT)Desc.Format, Flags);
		}
		else
		{
			ResDesc = CreateBufferResourceDesc(Desc.Width * Desc.Height * TextureFormatBPP(Desc.Format));
		}

		// Resource creation
		D3D12_RESOURCE_STATES ResourceState = Staging ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;
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

    void
	Texture2D::UploadData(void* Data, uint32 SizeBytes)
    {	
		// Create a staging texture that we will use to upload the data
		Texture2DDesc StagingDesc = {};
		StagingDesc.Width = TexDesc.Width;
		StagingDesc.Height = TexDesc.Height;
		StagingDesc.Format = TexDesc.Format;
		StagingDesc.Flags = TextureFlag_Staging;
		Texture2D* StagingTexture = new Texture2D(StagingDesc);

		// Upload the data to the staging texture
		void* MappedData = nullptr;
		D3D12_RANGE Range = {};
		StagingTexture->Resource->Raw()->Map(0, &Range, &MappedData);
		memcpy(MappedData, Data, SizeBytes);
		StagingTexture->Resource->Raw()->Unmap(0, nullptr);

		// Get the footprint of the staging texture to copy
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
		Footprint.Offset = 0;
		Footprint.Footprint.Format = (DXGI_FORMAT)TexDesc.Format;
		Footprint.Footprint.Width = TexDesc.Width;
		Footprint.Footprint.Height = TexDesc.Height;
		Footprint.Footprint.Depth = 1;
		Footprint.Footprint.RowPitch = TexDesc.Width * TextureFormatBPP(TexDesc.Format);

		// Fill copy location information
		D3D12_TEXTURE_COPY_LOCATION SrcLoc = CreateTextureCopyLocation(StagingTexture->Resource->Raw(), Footprint);
		D3D12_TEXTURE_COPY_LOCATION DestLoc = CreateTextureCopyLocation(Resource->Raw(), 0);

		// Copy the texture
		D3DContext::GetCommandList().Get()->CopyTextureRegion(&DestLoc, 0, 0, 0, &SrcLoc, nullptr);

		// Queue the destruction of the staging texture (we need to defer it until after the copy step is completed).
		D3DContext::GetCommandList().QueueTexture2DDestruction(StagingTexture);
    }

	/*
		Issue a command to copy the contents of one texture to another.
	*/
    void
	Texture2D::Copy(const Texture2D* Dest, const Texture2D* Src)
    {
		FL_Assert(AreTextureDescDimensionsAndFormatsTheSame(Dest->GetDesc(), Src->GetDesc()), "Trying to copy a texture when the destination and source have mismatching dimensions/formats.");

		Dest->Resource->TransitionState(D3D12_RESOURCE_STATE_COPY_DEST);
		Src->Resource->TransitionState(D3D12_RESOURCE_STATE_COPY_SOURCE);
		D3DContext::GetCommandList().Get()->CopyResource(Dest->Resource->Raw(), Src->Resource->Raw());
    }

	/*
		Publicly accessible function to load image data from non-native formats (png, jpg, bmp, etc.).
	*/
    uint8*
	LoadNonNativeTexture(const char* FilePath, uint32* Width, uint32* Height)
    {
		int W, H;
		uint8* Data = stbi_load(FilePath, &W, &H, nullptr, 4);
		FL_Assert(Data, "Failed to load non-native texture at: {0}", FilePath);
		*Width = W;
		*Height = H;
		return Data;
    }

	/*
		Free the memory allocated in the above function.
	*/
    void FreeNonNativeTexture(uint8* Data)
    {
		stbi_image_free(Data);
    }

}