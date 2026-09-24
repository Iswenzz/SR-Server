#pragma once
// https://github.com/xoxor4d/iw3xo-dev/blob/master/src/components/modules/movement.cpp
#include "CoD4.hpp"

namespace SR
{
	class Q3
	{
	public:
		static void WalkMove(pmove_tt* pm, pml_tt* pml, bool cpm = false);
		static void AirMove(pmove_tt* pm, pml_tt* pml, bool cpm = false);
		static void GroundTrace(pmove_tt* pm, pml_tt* pml);
		static void AirControl(pmove_tt* pm, pml_tt* pml, const vec3& wishdir, float wishspeed);
		static void Accelerate(playerState_tt* ps, pml_tt* pml, const vec3& wishdir, float wishspeed, float accel);
		static void AccelerateWalk(const vec3& wishdir, pml_tt* pml, playerState_tt* ps, float wishspeed, float accel);
		static bool JumpCheck(pmove_tt* pm, pml_tt* pml, bool cpm = false);
		static void Friction(pmove_tt* pm, pml_tt* pml, float friction = 6.0f);
		static void ClipVelocity(const vec3& in, const vec3& normal, vec3& out, float overbounce);
		static float PermuteRestrictiveClipPlanes(const vec3& velocity, int planeCount, vec3* planes, int* permutation);
		static bool SlideMove(pmove_tt* pm, pml_tt* pml, bool gravity);
		static void StepSlideMove(pmove_tt* pm, pml_tt* pml, bool gravity, bool cpm = false);
		static void SetMovementDir(pmove_tt* pm);
	};
}
