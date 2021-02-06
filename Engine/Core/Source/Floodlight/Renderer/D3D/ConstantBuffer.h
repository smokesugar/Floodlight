#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/IntTypes.h"
#include "Floodlight/Utilities/Keywords.h"

#include "RenderResource.h"

namespace Floodlight {

	/*
		Structure that contains data to update constant buffers with.
	*/
	struct UpdateObj
	{
		class ConstantBuffer* Buffer;
		void* Data;
		uint32 SizeBytes;
		uint32 Counter;
	};

	/*
		Constant buffer object. Used to store data for shader use.
	*/
	class ConstantBuffer : public RenderResource
	{
	public:
		ConstantBuffer(uint32 SizeBytes);
		~ConstantBuffer();

		static void DoUpdateQueue(uint32 FrameIndex);
		static void DestroyUpdateQueue();

		static void Update(ConstantBuffer* Buffer, void* Data, uint32 SizeBytes);
		static void Bind(const ConstantBuffer* Buffer, uint32 Index);
	private:
		virtual void InternalRelease() override;
		static bool DoUpdateObj(UpdateObj* Obj, uint32 FrameIndex);
	private:
		uint32 OriginalSize;
		uint32 IndividualSizeBytes;
		uint32 TotalSizeBytes;
		std::vector<uint32> DescriptorHeapIndices;
		ID3D12Resource* Buffer = nullptr;
	};

}