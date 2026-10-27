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
		Scr_AddMethod("rewindstate", RewindState, qfalse);
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

	// The slot is optional so scripts written for the single save keep working. A slot that was never
	// saved returns false instead of loading a zeroed state.
	void PlayerCommands::LoadState(scr_entref_t num)
	{
		const int params = Scr_GetNumParam();
		if (params > 1)
		{
			Scr_Error("Usage: LoadState(<slot?>)");
			return;
		}

		Ref<Player> player = Player::Get(num.entnum);
		const int slot = params ? Scr_GetInt(0) : 0;

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		if (slot < 0 || slot >= PLAYER_SAVE_STATES)
		{
			Scr_Error("LoadState: slot out of range");
			return;
		}
		if (!player->SaveStates[slot])
		{
			Scr_AddBool(false);
			return;
		}
		player->SetMoveState(*player->SaveStates[slot]);
		Scr_AddBool(true);
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

	// Moves the player back to where they were ms ago, view angles included, and returns the server time
	// of the frame it landed on. Undefined when nothing was recorded since the spawn.
	void PlayerCommands::RewindState(scr_entref_t num)
	{
		CHECK_PARAMS(1, "Usage: RewindState(<ms>)");

		Ref<Player> player = Player::Get(num.entnum);
		const int ms = Scr_GetInt(0);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		const PlayerRewindFrame *frame = player->FindRewind(level.time - ms);
		if (!frame)
			return;

		player->Rewind(*frame);
		Scr_AddInt(frame->time);
	}

	void PlayerCommands::SaveState(scr_entref_t num)
	{
		const int params = Scr_GetNumParam();
		if (params > 1)
		{
			Scr_Error("Usage: SaveState(<slot?>)");
			return;
		}

		Ref<Player> player = Player::Get(num.entnum);
		const int slot = params ? Scr_GetInt(0) : 0;

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		if (slot < 0 || slot >= PLAYER_SAVE_STATES)
		{
			Scr_Error("SaveState: slot out of range");
			return;
		}
		player->SaveStates[slot] = player->GetMoveState();
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
