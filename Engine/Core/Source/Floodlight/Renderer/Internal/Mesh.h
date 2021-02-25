#pragma once

#include "Floodlight/Renderer/D3D/VertexBuffer.h"
#include "Floodlight/Renderer/D3D/IndexBuffer.h"

namespace Floodlight
{
	/*
		The submesh class universally wraps vertex and index buffers.
	*/
	class Submesh
	{
	public:
		// This class owns these resources. The index buffer is optional.
		Submesh(VertexBuffer* VertexBuf, IndexBuffer* IndexBuf);
		~Submesh();

		void Draw();
	private:
		VertexBuffer* VB;
		IndexBuffer* IB;
	};

	/*
		The mesh class contains a list of submeshes.
	*/
	struct Mesh
	{
		std::vector<Submesh*> Submeshes;

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