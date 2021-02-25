#pragma once

#include "Floodlight/Renderer/D3D/VertexBuffer.h"
#include "Floodlight/Renderer/D3D/IndexBuffer.h"

namespace Floodlight
{
	/*
		The GPUGeometry struct pairs a vertex buffer and index buffer.
		Note the struct does not own the pointers.
	*/
	struct GPUGeometry
	{
		VertexBuffer* VB;
		IndexBuffer* IB;
	};

	/*
		The mesh class contains a list of submeshes.
	*/
	struct Mesh
	{
		std::vector<GPUGeometry> Submeshes;

		Mesh();
		~Mesh();

		Mesh(const Mesh&) = delete;
		inline void operator=(const Mesh&) = delete;
	};

	/*
		Mesh creation functions.
	*/
	Mesh* LoadGLTF(const char* Filepath);

}