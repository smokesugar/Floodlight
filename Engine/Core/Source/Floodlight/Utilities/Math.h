#pragma once

/*
	Maths library for internal Floodlight use.
*/

#include <DirectXMath.h>
using namespace DirectX;

#define FloatPtr(var) ((float*)&var)

namespace Floodlight {

	typedef XMFLOAT2 float2;
	typedef XMFLOAT3 float3;
	typedef XMFLOAT4 float4;
	typedef XMVECTOR xmvector;
	
	typedef XMFLOAT4X4 float4x4;
	typedef XMMATRIX matrix;

	static constexpr float PI = 3.14159265359f;

	inline float
	ToRadians(float Deg)
	{
		return Deg * PI / 180.0f;
	}

	inline float
	ToDegrees(float Rad)
	{
		return Rad * 180.0f / PI;
	}

	inline float
	Min(float a, float b)
	{
		return a < b ? a : b;
	}

	inline float
	Max(float a, float b)
	{
		return a > b ? a : b;
	}

	inline float3
	Min(float3 a, float3 b)
	{
		return float3(Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z));
	}

	inline float3
	Max(float3 a, float3 b)
	{
		return float3(Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z));
	}

}