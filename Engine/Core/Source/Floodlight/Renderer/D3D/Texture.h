#pragma once

#include <d3d12.h>

#include "GPUResource.h"
#include "Floodlight/Utilities/Bit.h"
#include "TextureFormats.h"

namespace Floodlight {

	using TextureFlags = int;
	enum _TextureFlags {
		TextureFlag_RenderTarget = Bit(0),
		TextureFlag_DepthStencil = Bit(1),
		TextureFlag_Staging = Bit(2),
	};

	struct Texture2DDesc {
		uint32 Width;
		uint32 Height;
		TextureFormat Format;
		TextureFlags Flags;
	};

	inline bool AreTextureDescDimensionsAndFormatsTheSame(const Texture2DDesc& A, const Texture2DDesc& B)
	{
		return A.Width == B.Width && A.Height == B.Height && A.Format == B.Format;
	}

	uint8* LoadNonNativeTexture(const char* FilePath, uint32* Width, uint32* Height);
	void FreeNonNativeTexture(uint8* Data);

	/*
		The underlying resource for render target views, depth stencil views and shader resource views.
	*/
	class Texture2D
	{
		friend class RenderTargetView;
		friend class DepthStencilView;
		friend class ShaderResourceView;
	public:
		Texture2D(Texture2DDesc Desc);
		// This class owns the ID3D12Resource
		Texture2D(GPUResource* Res, TextureFlags Flags);
		~Texture2D();

		Texture2D(const Texture2D&) = delete;
		inline void operator=(const Texture2D&) = delete;

		void UploadData(void* Data, uint32 SizeBytes);

		static void Copy(const Texture2D* Dest, const Texture2D* Src);

		inline GPUResource* GetResource() const { return Resource; }
		inline const Texture2DDesc& GetDesc() const { return TexDesc; }
	private:
		Texture2DDesc TexDesc;
		GPUResource* Resource = nullptr;
	};

}