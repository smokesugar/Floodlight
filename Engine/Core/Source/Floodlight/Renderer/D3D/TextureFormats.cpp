#include "TextureFormats.h"

#include "Floodlight/Utilities/Assert.h"

namespace Floodlight
{

	/*
		Utility function that returns the per pixel size of a texture format.
	*/
	uint32
	TextureFormatBPP(TextureFormat Format)
	{
		switch (Format)
		{
		case RGBA8_UNORM:
			return 4;
		case D32_FLOAT:
			return 4;
		}

		FL_Assert(false, "Invalid format.");
		return 0;
	}

}