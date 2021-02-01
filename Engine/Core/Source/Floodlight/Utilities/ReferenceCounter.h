#pragma once

#include "IntTypes.h"
#include "Assert.h"

namespace Floodlight {
	
	/*
		Used for reference counting in Floodlight resources.
	*/
	class ReferenceCounter 
	{
	public:
		ReferenceCounter() = default;
		inline void Increment() { ++NumReferences; }
		inline void Decrement() { FL_Assert(NumReferences > 0, "Cannot decrement a reference counter with no references."); --NumReferences; }
		inline uint32 Count() const { return NumReferences; }
	private:
		uint32 NumReferences = 0;
	};

}