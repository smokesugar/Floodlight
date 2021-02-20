#include "Time.h"

#include "Floodlight/Utilities/WinIncLean.h"
#include "Floodlight/Utilities/Keywords.h"

namespace Floodlight {

	confined double DeltaTime = 0.0f;
	confined double TimeAccum = 0.0f;

	double
	Win32GetTime()
	{
		LARGE_INTEGER time, freq;
		QueryPerformanceCounter(&time);
		QueryPerformanceFrequency(&freq);
		return (double)time.QuadPart / (double)freq.QuadPart;
	}

	void
	Time::NewFrame()
	{
		// Calculate delta time
		persist double LastTime = Win32GetTime();
		double Now = Win32GetTime();
		DeltaTime = Now - LastTime;
		LastTime = Now;

		TimeAccum += DeltaTime;
	}

	float
	Time::GetDeltaTime()
	{
		return (float)DeltaTime;
	}

	float
	Time::GetTime()
	{
		return (float)TimeAccum;
	}

}
