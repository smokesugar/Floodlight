#include "Mesh.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "Floodlight/Renderer/D3D/D3DContext.h"
#include "Floodlight/Utilities/Math.h"

namespace Floodlight
{
	Submesh::Submesh(VertexBuffer* VertexBuf, IndexBuffer* IndexBuf)
	{
		VB = VertexBuf;
		IB = IndexBuf;
	}

	Submesh::~Submesh()
	{
		delete VB;
		if (IB) delete IB;
	}

	/*
		Bind the resources of a submesh to the render pipeline.
	*/
	void
	Submesh::Draw()
	{
		VertexBuffer::Bind(VB->GetViewsPointer(), VB->GetNumViews());

		if (IB)
		{
			IndexBuffer::Bind(IB);
			D3DContext::GetCommandList().Get()->DrawIndexedInstanced(IB->GetNumIndices(), 1, 0, 0, 0);
		}
		else
		{
			D3DContext::GetCommandList().Get()->DrawInstanced(VB->GetNumVertices(), 1, 0, 0);
		}
	}

	/*
		Mesh function definitions.
	*/

	Mesh::Mesh()
	{
	}

	Mesh::~Mesh()
	{
		for (auto Ptr : Submeshes)
		{
			delete Ptr;
		}
	}

	/*
		Get the transform from a cgltf_node.
	*/
	confined matrix
	GetNodeTransform(cgltf_node* Node)
	{
		FL_Assert(Node->has_matrix, "This node does not have a transform.");
		return *(matrix*)(&Node->matrix);
	}
	
	/*
		Flatten the node hierarchy.
	*/
	confined void
	ProcessNode(cgltf_node* Node, Mesh* Asset, const matrix& ParentTransform)
	{
		matrix Transform = GetNodeTransform(Node) * ParentTransform;
		
		for (uint32 i = 0; i < Node->children_count; i++)
		{
			ProcessNode(Node->children[i], Asset, Transform);
		}
	}

	/*
		Load a mesh from a gltf file on disk.
	*/

	Mesh* LoadGLTF(const char* Filepath)
	{
		cgltf_options Options = {};
		cgltf_data* Data = nullptr;

		cgltf_result Result = cgltf_parse_file(&Options, Filepath, &Data);
		FL_Assert(Result == cgltf_result_success, "Failed to load GLTF 3D model at: '{0}'", Filepath);

		Result = cgltf_load_buffers(&Options, Data, Filepath);
		FL_Assert(Result == cgltf_result_success, "Failed to load GLTF 3D model at: '{0}'", Filepath);

		Mesh* M = new Mesh();
		
		cgltf_scene* Scene = Data->scene;
		for (uint32 i = 0; i < Scene->nodes_count; i++)
		{
			auto* Node = Scene->nodes[i];
			ProcessNode(Node, M, XMMatrixIdentity());
		}

		cgltf_free(Data);

		return M;
	}

}