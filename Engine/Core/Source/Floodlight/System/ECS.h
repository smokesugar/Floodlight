#pragma once

/*
	This entity component system is used to manage entities within the Floodlight engine.
*/

#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	/*
		Floodlight entity type.
	*/
	enum class Entity : uint32 {};

	/*
		Entity component system registry. Use this to manage entities.
	*/
	class ECSRegistry
	{
	public:
		ECSRegistry();
		~ECSRegistry();
	private:
	};

}