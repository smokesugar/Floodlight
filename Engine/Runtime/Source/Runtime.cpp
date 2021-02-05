#include "Runtime.h"

#include "Floodlight/Renderer/D3D/RenderResource.h"
#include "Floodlight/Utilities/DebugLog.h"
#include "Floodlight/Utilities/Math.h"
#include "Floodlight/System/ECSRegistry.h"

namespace Floodlight {

	struct Position
	{	
		float3 Pos;
	};

	struct Velocity
	{
		float3 Vel;
	};
	
	Runtime::Runtime()
	{
		ECSRegistry Reg;
		Entity Ent0 = Reg.Create();
		Entity Ent1 = Reg.Create();
		Entity Ent2 = Reg.Create();
		Entity Ent3 = Reg.Create();

		Reg.Insert(Ent0, Position{ float3(0.0f, 1.0f, 0.0f) });
		Reg.Insert(Ent0, Velocity{ float3(0.0f, 6.0f, 0.3f) });
		FL_Assert(Reg.Has<Position>(Ent0), "Failure to add/remove component.");
		FL_Assert(Reg.Has<Velocity>(Ent0), "Failure to add/remove component.");

		Reg.Insert(Ent1, Position{ float3(0.0f, 1.0f, 0.0f) });
		Reg.Remove<Position>(Ent1);
		Reg.Insert(Ent1, Velocity{ float3(1.0f, 6.0f, 0.3f) });
		Reg.Remove<Velocity>(Ent1);
		FL_Assert(!Reg.Has<Position>(Ent1), "Failure to add/remove component.");
		FL_Assert(!Reg.Has<Velocity>(Ent1), "Failure to add/remove component.");

		Reg.Insert(Ent2, Position{ float3(0.0f, 1.0f, 0.0f) });
		Reg.Remove<Position>(Ent2);
		Reg.Insert(Ent2, Velocity{ float3(5.0f, 6.0f, 4.3f) });
		Reg.Destroy(Ent2);
		FL_Assert(!Reg.Has<Position>(Ent2), "Failure to add/remove component.");
		FL_Assert(!Reg.Has<Velocity>(Ent2), "Failure to add/remove component.");

		Reg.Insert(Ent3, Position{ float3(1.0f, 1.0f, 7.0f) });
		FL_Assert(Reg.Has<Position>(Ent3), "Failure to add/remove component.");
		
		{
			Position& Pos = Reg.Get<Position>(Ent0);
			FL_Trace("Ent0 Position: {0} {1} {2}", Pos.Pos.x, Pos.Pos.y, Pos.Pos.z);
		}

		{
			Position& Pos = Reg.Get<Position>(Ent3);
			FL_Trace("Ent3 Position: {0} {1} {2}", Pos.Pos.x, Pos.Pos.y, Pos.Pos.z);
		}
	}

	Runtime::~Runtime()
	{
		
	}

	void
	Runtime::Tick()
	{
		
	}

}