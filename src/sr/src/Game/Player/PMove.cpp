#include "PMove.hpp"
#include "Player.hpp"

#include "Movements/CS.hpp"
#include "Movements/Q3.hpp"
#include "Movements/Q3Weapons.hpp"
#include "Q3WeaponWorld.hpp"

namespace SR
{
	PMove::PMove(class Player *player)
	{
		Player = player;
		Weapons = CreateScope<Q3WeaponState>();
		WeaponEvents = CreateScope<Q3WeaponEvents>();
	}

	PMove::~PMove() = default;

	void PMove::Initialize()
	{
		Player->ps->speed = g_speed->integer;
		Player->ps->gravity = static_cast<int>(g_gravity->value);
		Player->cl->jumpHeight = jump_height->value;

		*Weapons = {};
		WeaponEvents->clear();
	}

	void PMove::Register()
	{
		// In the systeminfo, where IW3SR looks for it before predicting the Q3 weapons.
		Cvar_RegisterBool("sr_q3weapons", qtrue, CVAR_SYSTEMINFO | CVAR_ROM,
			"Simulates the Q3CPMW rocket and plasma in pmove");
	}

	int PMove::GetSpeed()
	{
		return Player->ps->speed;
	}

	float PMove::GetSpeedScale()
	{
		return Player->ps->moveSpeedScaleMultiplier;
	}

	int PMove::GetGravity()
	{
		return Player->ps->gravity;
	}

	float PMove::GetJumpHeight()
	{
		return Player->cl->jumpHeight;
	}

	MovementMode PMove::GetMovementMode()
	{
		switch (SV_GetClientStat(Player->ps->clientNum, 1700)) // sr_mode
		{
		case 1: // 190
		case 2: // 210
			return MovementMode::COD4;
		case 3: // Q3
			return MovementMode::Q3;
		case 4: // Q3CPM
		case 5: // Q3CPMW
			return MovementMode::Q3CPM;
		case 6: // CS
		case 7: // Portal
			return MovementMode::CS;
		}
		return MovementMode::COD4;
	}

	bool PMove::HasQ3Weapons()
	{
		const int mode = SV_GetClientStat(Player->ps->clientNum, 1700); // sr_mode
		return mode >= 3 && mode <= 5; // Q3, Q3CPM, Q3CPMW
	}

	bool PMove::IsQ3Cpm()
	{
		const int mode = SV_GetClientStat(Player->ps->clientNum, 1700); // sr_mode
		return mode == 4 || mode == 5;
	}

	bool PMove::IsQ3Weapon(int weapon)
	{
		const WeaponDef *def = BG_GetWeaponDef(weapon);
		return def && Q3Weapons::Find(def->szInternalName) >= 0;
	}

	void PMove::JumpUpdateSurface(pml_t *pml)
	{
		Player->SurfaceFlags = pml->groundTrace.surfaceFlags;
	}

	// Runs PM_Weapon, then launches a Q3 missile when it fired a Q3 weapon.
	void PMove::Weapon(pmove_t *pm, pml_t *pml)
	{
		auto *qpm = reinterpret_cast<pmove_tt *>(pm);
		const int weaponTime = qpm->ps->weaponTime;
		const int sequence = qpm->ps->eventSequence;

		StockWeapon(pm, pml);

		if (!HasQ3Weapons())
			return;

		Q3Weapons::Switch(qpm->ps, IsQ3Cpm());
		if (!Q3Weapons::Fired(qpm->ps, sequence))
			return;

		const WeaponDef *def = BG_GetWeaponDef(qpm->ps->weapon);
		const int weapon = def ? Q3Weapons::Find(def->szInternalName) : -1;
		if (weapon >= 0)
			Q3Weapons::Fire(qpm, reinterpret_cast<pml_tt *>(pml), *Weapons, weapon, weaponTime, IsQ3Cpm(),
				WeaponEvents.get());
	}

	void PMove::StockWeapon(pmove_t *pm, pml_t *pml)
	{
		PM_Weapon(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml));
	}

	// Hands the missile events to the world and GSC once per server frame, outside pmove, so script never runs
	// mid-move.
	void PMove::NotifyWeapons()
	{
		if (WeaponEvents->empty())
			return;

		Q3WeaponWorld::Events(Player->ps->clientNum, *WeaponEvents, level.time - Player->ps->commandTime, false);

		gentity_t *ent = SV_GentityNum(Player->ps->clientNum);
		const int explode = SL_FindString("q3_explode");

		for (const Q3WeaponEvent &event : *WeaponEvents)
		{
			const char *item = Q3Weapons::Defs[event.weapon].item;
			const vec3_t origin = { event.origin[0], event.origin[1], event.origin[2] };

			if (event.type == Q3WeaponEventType::Explode && explode)
			{
				if (event.hitEnt != ENTITYNUM_NONE)
					Scr_AddEntity(SV_GentityNum(event.hitEnt));
				else
					Scr_AddUndefined();
				Scr_AddString(item);
				Scr_AddVector(origin);
				Scr_Notify(ent, static_cast<unsigned short>(explode), 3);
			}
		}
		WeaponEvents->clear();
	}

	bool PMove::WalkMove(pmove_t *pm, pml_t *pml)
	{
		switch (GetMovementMode())
		{
		case MovementMode::Q3:
			Q3::WalkMove(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml), false);
			return true;
		case MovementMode::Q3CPM:
			Q3::WalkMove(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml), true);
			return true;
		case MovementMode::CS:
			CS::WalkMove(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml));
			return true;
		}
		return false;
	}

	bool PMove::AirMove(pmove_t *pm, pml_t *pml)
	{
		switch (GetMovementMode())
		{
		case MovementMode::Q3:
			Q3::AirMove(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml), false);
			return true;
		case MovementMode::Q3CPM:
			Q3::AirMove(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml), true);
			return true;
		case MovementMode::CS:
			CS::AirMove(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml));
			return true;
		}
		return false;
	}

	bool PMove::GroundTrace(pmove_t *pm, pml_t *pml)
	{
		if (HasQ3Weapons())
		{
			Weapons->hitPlayers = Player->Q3HitPlayers;
			Q3Weapons::Detonate(reinterpret_cast<pmove_tt *>(pm), *Weapons, IsQ3Cpm(), WeaponEvents.get());
		}
		else
			Weapons->count = 0;

		switch (GetMovementMode())
		{
		case MovementMode::Q3:
		case MovementMode::Q3CPM:
			Q3::GroundTrace(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml));
			return true;
		case MovementMode::CS:
			CS::GroundTrace(reinterpret_cast<pmove_tt *>(pm), reinterpret_cast<pml_tt *>(pml));
			return true;
		}
		return false;
	}

	bool PMove::CrashLand(playerState_s *ps, pml_t *pml)
	{
		return false;
	}
}
