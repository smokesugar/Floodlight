#include "Mesh.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "Floodlight/Renderer/D3D/D3DContext.h"
#include "Floodlight/Utilities/Math.h"

namespace Floodlight
{
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
			if (Ptr.VB) delete Ptr.VB;
			if (Ptr.IB) delete Ptr.IB;
		}
	}

	/*
		Get the transform from a cgltf_node.
	*/
	confined matrix
	GetNodeTransform(cgltf_node* Node)
	{
		matrix Scaling = XMMatrixScaling(Node->scale[0], Node->scale[1], Node->scale[2]);
		matrix Rotation = XMMatrixRotationRollPitchYaw(Node->rotation[0], Node->rotation[1], Node->rotation[2]);
		matrix Translation = XMMatrixTranslation(Node->translation[0], Node->translation[1], Node->translation[2]);
		return Scaling * Rotation * Translation;
	}

	/*
		Utility function to get the count in components of a cgltf_type.
	*/
	confined uint32
	CGLTFTypeComponentCount(cgltf_type Type)
	{
		switch (Type) {
		case cgltf_type_scalar:
			return 1;
		case cgltf_type_vec2:
			return 2;
		case cgltf_type_vec3:
			return 3;
		case cgltf_type_vec4:
		case cgltf_type_mat2:
			return 4;
		case cgltf_type_mat3:
			return 9;
		case cgltf_type_mat4:
			return 16;
		}

		FL_Assert(false, "Invalid cgltf_type!");
		return 0;
	}

	/*
		Utility function to get the size of a cgltf_component_type in bytes.
	*/
	confined uint32
	CGLTFComponentTypeSize(cgltf_component_type component_type)
	{
		switch (component_type) {
		case cgltf_component_type_r_8:
		case cgltf_component_type_r_8u:
			return 1;
		case cgltf_component_type_r_16:
		case cgltf_component_type_r_16u:
			return 2;
		case cgltf_component_type_r_32u:
		case cgltf_component_type_r_32f:
			return 4;
		}

		FL_Assert(false, "Invalid cgltf_component_type!");
		return 0;
	}
	
	/*
		Flatten the node hierarchy.
	*/
	confined void
	ProcessNode(cgltf_node* Node, Mesh* Asset, const matrix& ParentTransform)
	{
		matrix Transform = GetNodeTransform(Node) * ParentTransform;

		// Add a mesh if the node is contains one.
		if (Node->mesh)
		{
			auto M = Node->mesh;
			for (uint32 i = 0; i < M->primitives_count; i++) {
				auto& Prim = M->primitives[i];

				// Variables for attributes
				cgltf_attribute* PositionAttrib = nullptr;
				cgltf_attribute* NormalAttrib = nullptr;
				cgltf_attribute* TexCoordAttrib = nullptr;

				// Loop through cgltf primitive and gather wanted attributes
				for (uint32 i = 0; i < Prim.attributes_count; i++)
				{
					auto& Attrib = Prim.attributes[i];
					if (strcmp("POSITION", Attrib.name) == 0) PositionAttrib = &Attrib;
					if (strcmp("NORMAL", Attrib.name) == 0) NormalAttrib = &Attrib;
					if (strcmp("TEXCOORD_0", Attrib.name) == 0) TexCoordAttrib = &Attrib;
				}

				// Ensure we have all the attributes we need.
				FL_Assert(PositionAttrib && TexCoordAttrib && NormalAttrib, "Invalid GLTF file.");

				// Gather all the vertex data
				const cgltf_accessor* PositionAccessor = PositionAttrib->data;
				const cgltf_buffer_view* PositionView = PositionAccessor->buffer_view;
				const float* PositionData = (float*)((uint8*)PositionView->buffer->data + PositionView->offset);

				const cgltf_accessor* NormalAccessor = NormalAttrib->data;
				const cgltf_buffer_view* NormalView = NormalAccessor->buffer_view;
				const float* NormalData = (float*)((uint8*)NormalView->buffer->data + NormalView->offset);

				const cgltf_accessor* TexCoordAccessor = TexCoordAttrib->data;
				const cgltf_buffer_view* TexCoordView = TexCoordAccessor->buffer_view;
				const float* TexCoordData = (float*)((uint8*)TexCoordView->buffer->data + TexCoordView->offset);

				FL_Assert(PositionAccessor->count == TexCoordAccessor->count && PositionAccessor->count == NormalAccessor->count, "Invalid GLTF file.");

				uint32 NumVertices = (uint32)PositionAccessor->count;

				std::vector<float> Vertices;
				Vertices.reserve(NumVertices * 8);

				// Push each vertex into the array.
				uint32 NumPositionComponents = CGLTFTypeComponentCount(PositionAccessor->type);
				for (uint32 i = 0; i < NumVertices; i++)
				{
					Vertices.push_back(PositionData[i * NumPositionComponents + 0]);
					Vertices.push_back(PositionData[i * NumPositionComponents + 1]);
					Vertices.push_back(PositionData[i * NumPositionComponents + 2]);
				}

				// Push each normal into the array.
				uint32 NumNormalComponents = CGLTFTypeComponentCount(NormalAccessor->type);
				for (uint32 i = 0; i < NumVertices; i++)
				{
					Vertices.push_back(NormalData[i * NumNormalComponents + 0]);
					Vertices.push_back(NormalData[i * NumNormalComponents + 1]);
					Vertices.push_back(NormalData[i * NumNormalComponents + 2]);
				}

				// Push each texcoord into the array.
				uint32 NumTexCoordComponents = CGLTFTypeComponentCount(TexCoordAccessor->type);
				for (uint32 i = 0; i < NumVertices; i++)
				{
					Vertices.push_back(TexCoordData[i * NumTexCoordComponents + 0]);
					Vertices.push_back(TexCoordData[i * NumTexCoordComponents + 1]);
				}

				// Gather index data
				const cgltf_accessor* IndexAccessor = Prim.indices;
				const cgltf_buffer_view* IndexView = IndexAccessor->buffer_view;

				uint32 IndexSizeBytes = CGLTFComponentTypeSize(IndexAccessor->component_type);
				FL_Assert(IndexSizeBytes == 4 || IndexSizeBytes == 2, "Invalid GLTF file.");

				uint32 NumIndices = (uint32)IndexAccessor->count;
				std::vector<uint32> Indices;
				Indices.reserve(NumIndices);

				if (IndexSizeBytes == 4)
				{
					uint32* IndexData = (uint32*)((uint8*)IndexView->buffer->data + IndexView->offset);
					for (uint32 i = 0; i < NumIndices; i++)
					{
						Indices.push_back(IndexData[i]);
					}
				}
				else if (IndexSizeBytes == 2)
				{
					uint16* IndexData = (uint16*)((uint8*)IndexView->buffer->data + IndexView->offset);
					for (uint32 i = 0; i < NumIndices; i++)
					{
						Indices.push_back((uint32)IndexData[i]);
					}
				}

				// Create the resources on the gpu.
				GPUGeometry Geom = {};

				uint32 AttributeSizes[3] =
				{
					3 * sizeof(float),
					3 * sizeof(float),
					2 * sizeof(float),
				};
				Geom.VB = new VertexBuffer(Vertices.data(), NumVertices, AttributeSizes, (uint32)std::size(AttributeSizes));

				Geom.IB = new IndexBuffer(Indices.data(), NumIndices * 4);

				Asset->Submeshes.push_back(Geom);
			}
		}
		
		// Recurse through all the children.
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