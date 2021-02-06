#pragma once

#include "Floodlight/Utilities/ReferenceCounter.h"

namespace Floodlight {

	/*
		Internal base class used by resources. They are reference counted.
		Ensure that Release() is called in the destructor of inherited resources.
	*/
	class RenderResource
	{
	public:
		RenderResource();
		RenderResource(const RenderResource& Other);
		virtual ~RenderResource() {}

		void operator=(const RenderResource& Other);
	protected:
		void Release();
		virtual void InternalRelease() = 0;
	protected:
		ReferenceCounter* RefCounter;
	};

}