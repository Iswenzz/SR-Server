#include "Entity.hpp"

namespace SR
{
	// A mover whose move has ended is stood still again by the game; this one's never does.
	static constexpr int FOLLOW_FOREVER = 1 << 30;

	void EntityCommands::Register()
	{
		Scr_AddMethod("followmovement", FollowMovement, qfalse);
	}

	// The entity stands where what a player rides does and turns as it does, moved between snapshots the
	// way a player is. It names its rider, so the rider's own client can draw it where it predicts itself.
	// A second argument of 1 stands it as what is inside the ride instead, its feet on the bottom of it.
	void EntityCommands::FollowMovement(scr_entref_t num)
	{
		if (Scr_GetNumParam() < 1 || Scr_GetNumParam() > 2)
		{
			Scr_Error("Usage: FollowMovement(<player>, [part])\n");
			return;
		}

		gentity_t *ent = &g_entities[num.entnum];
		gentity_t *rider = Scr_GetEntity(0);
		Ref<Player> player = rider && rider->client ? Player::Get(rider->s.number) : nullptr;

		vec3 origin, angles;
		const int part = Scr_GetNumParam() > 1 ? Scr_GetInt(1) : 0;
		if (num.classnum || !player || !player->PMove->Pose(origin, angles, part))
		{
			Scr_AddBool(qfalse);
			return;
		}
		G_SetOrigin(ent, &origin[0]);
		VectorCopy(angles, ent->s.lerp.apos.trBase);
		VectorCopy(angles, ent->r.currentAngles);

		for (trajectory_t *move : { &ent->s.lerp.pos, &ent->s.lerp.apos })
		{
			move->trType = TR_INTERPOLATE;
			move->trTime = level.time;
			move->trDuration = FOLLOW_FOREVER;
		}
		ent->s.otherEntityNum = rider->s.number;
		ent->s.attackerEntityNum = part;
		SV_LinkEntity(ent);
		Scr_AddBool(qtrue);
	}

}
