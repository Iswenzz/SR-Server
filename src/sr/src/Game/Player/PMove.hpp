#pragma once
#include "Player.hpp"

namespace SR
{
	enum class MovementMode
	{
		COD4,
		Q3,
		Q3CPM,
		CS,
	};

	class PMove
	{
	public:
		class Player *Player = nullptr;

		PMove(class Player *player);
		virtual ~PMove() = default;

		void Initialize();

		int GetSpeed();
		float GetSpeedScale();
		int GetGravity();
		float GetJumpHeight();
		MovementMode GetMovementMode();

		void JumpUpdateSurface(pml_t *pml);
		bool WalkMove(pmove_t *pm, pml_t *pml);
		bool AirMove(pmove_t *pm, pml_t *pml);
		bool GroundTrace(pmove_t *pm, pml_t *pml);
		bool CrashLand(playerState_s *ps, pml_t *pml);
	};
}
