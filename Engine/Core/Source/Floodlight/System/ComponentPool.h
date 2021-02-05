#pragma once

#include <unordered_map>
#include <vector>

#include "Floodlight/Utilities/Assert.h"
#include "Entity.h"

namespace Floodlight {

	/*
		Class declaration of the component pool base class.
	*/
	class _ComponentPool
	{
	public:
		_ComponentPool() {}
		virtual ~_ComponentPool() {}
		virtual bool Has(Entity Ent) const = 0;
		virtual void Remove(Entity Ent) = 0;
	};

	/*
		Templated class definition of the component pool class.
	*/
	template<typename Type>
	class ComponentPool : public _ComponentPool
	{
	public:
		ComponentPool()
		{
		}
		virtual ~ComponentPool()
		{
		}

		/*
			Add an entity to this pool along with it's data.
		*/
		Type&
		Insert(Entity Ent, const Type& Data)
		{
			FL_Assert(Indices.find(Ent) == Indices.end(), "Cannot add this component to this entity; the entity already owns this type of component.");
			uint32 NewIndex =(uint32) Components.size();
			Indices[Ent] = NewIndex;
			Entities[NewIndex] = Ent;
			Components.push_back(Data);
			return Components[NewIndex];
		}

		/*
			Remove an entity from this pool and destroy its data.
		*/
		virtual void
		Remove(Entity Ent) override
		{
			FL_Assert(Indices.find(Ent) != Indices.end(), "Cannot remove this component from this entity; the entity does not own this type of component.");

			// Retrieve information
			uint32 RemovedIndex = Indices[Ent];
			uint32 LastIndex = (uint32)Components.size() - 1;
			Entity RemovedEntity = Ent;
			Entity LastEntity = Entities[LastIndex];

			// Copy the data from the last index to the removed index
			Components[RemovedIndex] = Components[LastIndex];
			Components.pop_back();

			// Update maps
			Indices[LastEntity] = RemovedIndex;
			Entities[RemovedIndex] = LastEntity;
			Indices.erase(Ent);
			Entities.erase(LastIndex);
		}

		/*
			At accessor.
		*/
		inline Type&
		At(Entity Ent)
		{
			FL_Assert(Indices.find(Ent) != Indices.end(), "Cannot retrieve this component from this entity; the entity does not own this type of component.");
			return Components[Indices[Ent]];
		}

		/*
			Index operator.
		*/
		inline Type&
		operator[](Entity Ent)
		{
			return At(Ent);
		}

		/*
			Does this pool contain that entity.
		*/
		virtual inline bool
		Has(Entity Ent) const override
		{
			return Indices.find(Ent) != Indices.end();
		}
	private:
		std::unordered_map<Entity, uint32> Indices;
		std::unordered_map<uint32, Entity> Entities;
		std::vector<Type> Components;
	};

}