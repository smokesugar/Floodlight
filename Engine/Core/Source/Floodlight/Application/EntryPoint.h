#pragma once

#include <iostream>

#include "Floodlight/Utilities/WinIncLean.h"
#include "EngineStart.h"

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Allocate a console window for debug builds
	#ifndef NDEBUG
		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
	#endif

	Floodlight::EngineStart();
	return 0;
}