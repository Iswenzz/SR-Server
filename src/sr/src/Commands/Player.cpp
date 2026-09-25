#include "Player.hpp"

#include "Audio/Voice.hpp"

namespace SR
{
	void PlayerCommands::Register()
	{
		Scr_AddMethod("getdamagetimer", GetDamageTimer, qfalse);
		Scr_AddMethod("getviewheightlerpdown", GetViewHeightLerpDown, qfalse);
		Scr_AddMethod("getviewheightlerptarget", GetViewHeightLerpTarget, qfalse);
		Scr_AddMethod("getviewheightlerptime", GetViewHeightLerpTime, qfalse);
		Scr_AddMethod("loadstate", LoadState, qfalse);
		Scr_AddMethod("proximityenable", ProximityEnable, qfalse);
		Scr_AddMethod("radioenable", RadioEnable, qfalse);
		Scr_AddMethod("savestate", SaveState, qfalse);
		Scr_AddMethod("setjumporigin", SetJumpOrigin, qfalse);
		Scr_AddMethod("surfaceflags", SurfaceFlags, qfalse);
		Scr_AddMethod("voicechatbuttonpressed", VoiceChatButtonPressed, qfalse);
		Scr_AddMethod("weaponstate", GetWeaponState, qfalse);
	}

	void PlayerCommands::VoiceChatButtonPressed(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddBool(player->Voice);
	}

	void PlayerCommands::SurfaceFlags(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->SurfaceFlags);
	}

	void PlayerCommands::SetJumpOrigin(scr_entref_t num)
	{
		CHECK_PARAMS(1, "Usage: SetJumpOrigin(<origin>)\n");

		Ref<Player> player = Player::Get(num.entnum);
		float origin = Scr_GetFloat(0);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		player->ps->jumpOriginZ = origin;
	}

	void PlayerCommands::GetViewHeightLerpDown(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->ps->viewHeightLerpDown);
	}

	void PlayerCommands::GetViewHeightLerpTarget(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->ps->viewHeightLerpTarget);
	}

	void PlayerCommands::GetViewHeightLerpTime(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->ps->viewHeightLerpTime);
	}

	void PlayerCommands::GetDamageTimer(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->ps->damageTimer);
	}

	void PlayerCommands::GetWeaponState(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->ps->weaponstate);
	}

	void PlayerCommands::LoadState(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		player->ps->pm_type = player->SaveState->pm_type;
		player->ps->bobCycle = player->SaveState->bobCycle;
		player->ps->pm_flags = player->SaveState->pm_flags;
		player->ps->weapFlags = player->SaveState->weapFlags;
		player->ps->otherFlags = player->SaveState->otherFlags;
		player->ps->pm_time = player->SaveState->pm_time;
		VectorCopy(player->SaveState->origin, player->ps->origin);
		VectorCopy(player->SaveState->velocity, player->ps->velocity);
		Vector2Copy(player->SaveState->oldVelocity, player->ps->oldVelocity);
		player->ps->leanf = player->SaveState->leanf;
		player->ps->groundEntityNum = player->SaveState->groundEntityNum;
		Vector2Copy(player->SaveState->vLadderVec, player->ps->vLadderVec);
		player->ps->jumpTime = player->SaveState->jumpTime;
		player->ps->jumpOriginZ = player->SaveState->jumpOriginZ;
		player->ps->damageTimer = player->SaveState->damageTimer;
		player->ps->damageDuration = player->SaveState->damageDuration;
		player->ps->movementDir = player->SaveState->movementDir;
		player->ps->eFlags = player->SaveState->eFlags;
		player->ps->eventSequence = player->SaveState->eventSequence;
		player->ps->weapon = player->SaveState->weapon;
		player->ps->adsDelayTime = player->SaveState->adsDelayTime;
		player->ps->damageEvent = player->SaveState->damageEvent;
		player->ps->damageYaw = player->SaveState->damageYaw;
		player->ps->damagePitch = player->SaveState->damagePitch;
		player->ps->damageCount = player->SaveState->damageCount;
		player->ps->sprintState = player->SaveState->sprintState;
		player->ps->holdBreathScale = player->SaveState->holdBreathScale;
		player->ps->holdBreathTimer = player->SaveState->holdBreathTimer;
		player->ps->moveSpeedScaleMultiplier = player->SaveState->moveSpeedScaleMultiplier;
		player->ps->mantleState = player->SaveState->mantleState;
		player->ps->entityEventSequence = player->SaveState->entityEventSequence;
		player->ps->aimSpreadScale = player->SaveState->aimSpreadScale;
	}

	void PlayerCommands::RadioEnable(scr_entref_t num)
	{
		CHECK_PARAMS(1, "Usage: RadioEnable(<state>)");

		if (num.entnum >= MAX_CLIENTS)
		{
			Scr_ObjectError("Not a player.\n");
			return;
		}
		Voice::RadioEnabled[num.entnum] = Scr_GetInt(0);
	}

	void PlayerCommands::SaveState(scr_entref_t num)
	{
		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		*player->SaveState = *player->ps;
	}

	// Reached from the mod's connect callback, before SR_InitializePlayer has made the Player, so it
	// writes Voice's per-slot flag rather than needing one.
	void PlayerCommands::ProximityEnable(scr_entref_t num)
	{
		CHECK_PARAMS(1, "Usage: ProximityEnable(<state>)");

		if (num.entnum >= MAX_CLIENTS)
		{
			Scr_ObjectError("Not a player.\n");
			return;
		}
		Voice::ProximityEnabled[num.entnum] = Scr_GetInt(0);
	}
}
