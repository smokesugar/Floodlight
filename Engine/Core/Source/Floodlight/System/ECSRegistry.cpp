#include "ECSRegistry.h"

namespace Floodlight {

	/*
		Constructor of the ECS registry.
	*/
	ECSRegistry::ECSRegistry()
	{
	}

	/*
		Destructor of the ECS registry.
	*/
	ECSRegistry::~ECSRegistry()
	{
		for (auto& [ID, Pool] : Pools)
		{
			delete Pool;
		}
	}

	/*
		Create a new entity.
	*/
	Entity
	ECSRegistry::Create()
	{
		persist uint32 Counter = 1;
		return (Entity)Counter++;
	}

	/*
		Destroy all the components an entity has.
	*/
	void
	ECSRegistry::Destroy(Entity Ent)
	{
		for (auto& [ID, Pool] : Pools)
		{
			if (Pool->Has(Ent))
				Pool->Remove(Ent);
		}
	}

}