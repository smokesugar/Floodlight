#pragma once

/*
	This entity component system is used to manage entities within the Floodlight engine.
*/

#include "Floodlight/Utilities/Keywords.h"
#include "ComponentPool.h"

namespace Floodlight {

	/*
		Global ID generator to be used in the templated function below.
	*/
	confined uint32
	GenerateID()
	{
		persist uint32 Counter = 0;
		return Counter++;
	}

	/*
		Get a unique identifier for a specific type.
	*/
	template<typename Type>
	confined uint32
	GetTypeID()
	{
		persist uint32 ID = GenerateID();
		return ID;
	}

	/*
		Entity component system registry. Use this to manage entities.
	*/
	class ECSRegistry
	{
	public:
		ECSRegistry();
		~ECSRegistry();

		Entity Create();
		void Destroy(Entity Ent);

		template<typename Type>
		inline Type& Insert(Entity Ent, const Type& Data)
		{
			return GetPool<Type>()->Insert(Ent, Data);
		}

		template<typename Type>
		inline void Remove(Entity Ent)
		{
			GetPool<Type>()->Remove(Ent);
		}

		/*
			Retrieve a component of a specific type from an entity.
		*/
		template<typename Type>
		inline Type& Get(Entity Ent)
		{
			return GetPool<Type>()->At(Ent);
		}

		/*
			Returns true if an entity contains a specific type of component.
		*/
		template<typename Type>
		bool Has(Entity Ent)
		{
			return GetPool<Type>()->Has(Ent);
		}
	private:
		template<typename Type>
		inline ComponentPool<Type>* GetPool()
		{
			uint32 TypeID = GetTypeID<Type>();
			if (Pools.find(TypeID) == Pools.end())
			{
				Pools[TypeID] = new ComponentPool<Type>();
				FL_Info("ECS: Registering a new component! (ID: {0})", TypeID);
			}
			return (ComponentPool<Type>*)Pools[TypeID];
		}
	private:
		std::unordered_map<uint32, _ComponentPool*> Pools;
	};

}