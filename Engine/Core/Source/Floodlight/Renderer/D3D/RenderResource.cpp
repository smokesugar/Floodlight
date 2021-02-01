#include "RenderResource.h"

#include "Floodlight/Utilities/DebugLog.h"

namespace Floodlight {

	RenderResource::RenderResource()
	{
		FL_Trace("RenderResource: Creating a new underlying resource.");
		RefCounter = new ReferenceCounter();
		RefCounter->Increment();
	}

	RenderResource::RenderResource(const RenderResource& Other)
	{
		FL_Trace("RenderResource: Sharing an underlying resource.");
		RefCounter = Other.RefCounter;
		RefCounter->Increment();
	}

	void
	RenderResource::operator=(const RenderResource& Other)
	{
		Release();
		FL_Trace("RenderResource: Sharing an underlying resource.");
		RefCounter = Other.RefCounter;
		RefCounter->Increment();
	}

	void
	RenderResource::Release()
	{
		RefCounter->Decrement();
		if (RefCounter->Count() == 0)
		{
			FL_Trace("RenderResource: Destroying and deleting the underlying resource (RefCount: {0}).", RefCounter->Count());
			InternalRelease();
			delete RefCounter;
		}
		else {
			FL_Trace("RenderResource: Destroying but not deleting underlying resource (RefCount: {0}).", RefCounter->Count());
		}
	}

}