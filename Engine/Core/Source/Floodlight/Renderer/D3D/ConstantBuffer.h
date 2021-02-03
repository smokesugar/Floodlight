#pragma once

#include <d3d12.h>

#include "Floodlight/Utilities/IntTypes.h"
#include "Floodlight/Utilities/Keywords.h"

#include "RenderResource.h"

namespace Floodlight{

	/*
		Constant buffer object. Used to store data for shader use.
	*/
	class ConstantBuffer : public RenderResource
	{
	public:
		ConstantBuffer(uint32 SizeBytes);
		~ConstantBuffer();

		void Update(void* Data, uint32 SizeBytes);

		inline uint32 GetDescriptorHeapIndex() const { return DescriptorHeapIndex; }
	private:
		virtual void InternalRelease() override;
	private:
		uint32 IndividualSizeBytes;
		uint32 TotalSizeBytes;
		uint32 DescriptorHeapIndex;
		ID3D12Resource* Buffer = nullptr;
		
	};

	void BindConstantBuffer(const ConstantBuffer* Buffer, uint32 Index);

}