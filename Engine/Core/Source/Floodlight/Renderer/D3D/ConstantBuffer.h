#pragma once

#include <d3d12.h>
#include <vector>

#include "Floodlight/Utilities/IntTypes.h"
#include "Floodlight/Utilities/Keywords.h"
#include "GPUResource.h"

namespace Floodlight {

	/*
		Structure that contains data to update constant buffers with.
	*/
	struct UpdateObj
	{
		void* Data;
		uint32 SizeBytes;
		uint32 Counter;
	};

	/*
		Constant buffer object. Used to store data for shader use.
		Note that updating a constant buffer will force it to update over multiple frames.
		This is required in order to 
	*/
	class ConstantBuffer
	{
	public:
		ConstantBuffer(uint32 SizeBytes);
		~ConstantBuffer();

		ConstantBuffer(const ConstantBuffer&) = delete;
		inline void operator=(const ConstantBuffer&) = delete;

		static void Update(ConstantBuffer* Buffer, void* Data, uint32 SizeBytes);
		static void Bind(const ConstantBuffer* Buffer, uint32 Index);

		// These functions manage the update queue.
		static void DoUpdateQueue(uint32 FrameIndex);
		static void DestroyUpdateQueue();
	private:
		// This function updates a constant buffer based on an update object.
		static bool DoUpdateObj(ConstantBuffer* Buffer, UpdateObj* Obj, uint32 FrameIndex);
	private:
		uint32 OriginalSize; // Size of buffer supplied in constructor.
		uint32 IndividualSizeBytes; // Size of each padded instance in the buffer.
		uint32 TotalSizeBytes;  // Total size of the allocated buffer.
		std::vector<uint32> DescriptorHeapIndices; // The indices into the descriptor heap.
		GPUResource* Buffer = nullptr; // The D3D buffer.
	};

}