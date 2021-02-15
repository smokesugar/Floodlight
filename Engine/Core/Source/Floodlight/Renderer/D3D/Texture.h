#pragma once

#include <d3d12.h>

#include "GPUResource.h"
#include "Floodlight/Utilities/IntTypes.h"
#include "Floodlight/Utilities/Bit.h"

namespace Floodlight {

	using TextureFormat = int;
	enum _TextureFormat {
		RGBA8_UNORM = DXGI_FORMAT_R8G8B8A8_UNORM,
		D32_FLOAT = DXGI_FORMAT_D32_FLOAT
	};

	using TextureFlags = int;
	enum _TextureFlags {
		TextureFlag_RenderTarget = Bit(0),
		TextureFlag_DepthStencil = Bit(1),
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

	/*
		The underlying resource for render target views, depth stencil views and shader resource views.
	*/
	class Texture2D
	{
		friend class RenderTargetView;
		friend class DepthStencilView;
	public:
		Texture2D(Texture2DDesc Desc);
		// This class owns the ID3D12Resource
		Texture2D(GPUResource* Res, TextureFlags Flags);
		~Texture2D();

		Texture2D(const Texture2D&) = delete;
		inline void operator=(const Texture2D&) = delete;

		static void Copy(const Texture2D* Dest, const Texture2D* Src);

		inline GPUResource* GetResource() const { return Resource; }
		inline const Texture2DDesc& GetDesc() const { return TexDesc; }
	private:
		Texture2DDesc TexDesc;
		GPUResource* Resource = nullptr;
	};

}