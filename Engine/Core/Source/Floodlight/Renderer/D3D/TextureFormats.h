#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight
{
	using TextureFormat = int;
	enum _TextureFormat {
		RGBA8_UNORM = DXGI_FORMAT_R8G8B8A8_UNORM,
		D32_FLOAT = DXGI_FORMAT_D32_FLOAT
	};

	uint32 TextureFormatBPP(TextureFormat Format);
}