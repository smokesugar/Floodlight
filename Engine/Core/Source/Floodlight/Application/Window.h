#pragma once

#include "Floodlight/Utilities/WinIncLean.h"
#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	struct WindowDesc {
		uint32 Width;
		uint32 Height;
		const char* Title;
	};

	HWND CreateWin32Window(WindowDesc Desc);
	void PollWin32Events();

}