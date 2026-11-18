#pragma once
#include "CoD4.hpp"

// Laid out alike in every file that includes this, whatever packing was left on before it (server.h leaves 1).
#pragma pack(push, 8)

namespace SR
{
	// Mario Kart Wii's speed on foot: Quake 3 CPM steering under the game's own limit and acceleration.
	// The limit is the player's speed, so a boost is a higher one for a while. Big is a mega mushroom's size.
	class Racer
	{
	public:
		static void WalkMove(pmove_tt* pm, pml_tt* pml, bool big);
		static void AirMove(pmove_tt* pm, pml_tt* pml, bool big);
		static void View(playerState_tt* ps, bool big);
		static bool Boosted(float limit);
		static float Ground(float limit, int surfaceFlags);
		static int Under(pmove_tt* pm);
		static bool With(pmove_tt* pm, pml_tt* pml);
		static float Gain(float speed, float limit);
		static float Coast(float speed);
		static float Allowed(float speed, float limit, float frametime, bool driven);
	};
}

#pragma pack(pop)
