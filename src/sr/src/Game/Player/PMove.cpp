#include "PMove.hpp"
#include "Player.hpp"

#include "Movements/CS.hpp"
#include "Movements/Q3.hpp"

namespace SR
{
	PMove::PMove(class Player *player)
	{
		Player = player;
	}

	void PMove::Initialize()
	{
		Player->ps->speed = g_speed->integer;
		Player->ps->gravity = static_cast<int>(g_gravity->value);
		Player->cl->jumpHeight = jump_height->value;
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

	void PMove::JumpUpdateSurface(pml_t *pml)
	{
		Player->SurfaceFlags = pml->groundTrace.surfaceFlags;
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
