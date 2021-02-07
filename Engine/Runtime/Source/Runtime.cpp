#include "Runtime.h"

#include "Floodlight/Utilities/DebugLog.h"
#include "Floodlight/Utilities/Math.h"
#include "Floodlight/System/ECSRegistry.h"

namespace Floodlight {

	struct PositionComponent
	{	
		float3 Pos;
		PositionComponent(float3 Position)
		{
			Pos = Position;
		}
	};

	struct VelocityComponent
	{
		float3 Vel;
		VelocityComponent(float3 Velocity)
		{
			Vel = Velocity;
		}
	};
	
	Runtime::Runtime()
	{
		ECSRegistry Reg;
		Entity Ent0 = Reg.Create();
		Entity Ent1 = Reg.Create();
		Entity Ent2 = Reg.Create();
		Entity Ent3 = Reg.Create();

		Reg.Emplace<PositionComponent>(Ent0, float3(0.0f, 1.0f, 0.0f));
		Reg.Insert(Ent0, VelocityComponent{ float3(0.0f, 6.0f, 0.3f) });
		FL_Assert(Reg.Has<PositionComponent>(Ent0), "Failure to add/remove component.");
		FL_Assert(Reg.Has<VelocityComponent>(Ent0), "Failure to add/remove component.");

		Reg.Insert(Ent1, PositionComponent{ float3(0.0f, 1.0f, 0.0f) });
		Reg.Remove<PositionComponent>(Ent1);
		Reg.Insert(Ent1, VelocityComponent{ float3(1.0f, 6.0f, 0.3f) });
		Reg.Remove<VelocityComponent>(Ent1);
		FL_Assert(!Reg.Has<PositionComponent>(Ent1), "Failure to add/remove component.");
		FL_Assert(!Reg.Has<VelocityComponent>(Ent1), "Failure to add/remove component.");

		Reg.Insert(Ent2, PositionComponent{ float3(0.0f, 1.0f, 0.0f) });
		Reg.Remove<PositionComponent>(Ent2);
		Reg.Insert(Ent2, VelocityComponent{ float3(5.0f, 6.0f, 4.3f) });
		Reg.Destroy(Ent2);
		FL_Assert(!Reg.Has<PositionComponent>(Ent2), "Failure to add/remove component.");
		FL_Assert(!Reg.Has<VelocityComponent>(Ent2), "Failure to add/remove component.");

		Reg.Insert(Ent3, PositionComponent{ float3(1.0f, 1.0f, 7.0f) });
		FL_Assert(Reg.Has<PositionComponent>(Ent3), "Failure to add/remove component.");
		
		{
			PositionComponent& Pos = Reg.Get<PositionComponent>(Ent0);
			FL_Trace("Ent0 Position: {0} {1} {2}", Pos.Pos.x, Pos.Pos.y, Pos.Pos.z);
		}

		{
			PositionComponent& Pos = Reg.Get<PositionComponent>(Ent3);
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