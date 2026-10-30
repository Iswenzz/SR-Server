#pragma once
#include "Player.hpp"

namespace SR
{
	struct Q3WeaponState;
	struct Q3WeaponEvent;

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
		// Kept behind pointers: the movement headers' CoD4 class would hide the CoD4 demo namespace
		Scope<Q3WeaponState> Weapons;
		Scope<std::vector<Q3WeaponEvent>> WeaponEvents;

		PMove(class Player *player);
		virtual ~PMove();

		void Initialize();
		static void Register();

		int GetSpeed();
		float GetSpeedScale();
		int GetGravity();
		float GetJumpHeight();
		MovementMode GetMovementMode();
		bool HasQ3Weapons();
		bool IsQ3Cpm();
		bool IsQ3Weapon(int weapon);

		void JumpUpdateSurface(pml_t *pml);
		void Weapon(pmove_t *pm, pml_t *pml);
		static void StockWeapon(pmove_t *pm, pml_t *pml);
		void NotifyWeapons();
		bool WalkMove(pmove_t *pm, pml_t *pml);
		bool AirMove(pmove_t *pm, pml_t *pml);
		bool GroundTrace(pmove_t *pm, pml_t *pml);
		bool CrashLand(playerState_s *ps, pml_t *pml);
	};
}
