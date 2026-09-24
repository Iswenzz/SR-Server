#include "CoD4.hpp"

#define player_spectateSpeedScale 1.0f

// The glm mirrors are reinterpret_cast from the engine structs.
static_assert(sizeof(playerState_tt) == sizeof(playerState_t));
static_assert(sizeof(trace_tt) == sizeof(trace_t));
static_assert(sizeof(pml_tt) == sizeof(pml_t));
static_assert(sizeof(pmove_tt) == sizeof(pmove_t));

namespace SR
{
	vec3 CoD4::Normalize(const vec3 &v)
	{
		const float length = glm::length(v);
		return length > 0.0f ? v / length : vec3{ 0.0f, 0.0f, 0.0f };
	}

	float CoD4::CmdScale(playerState_tt *ps, usercmd_s *cmd)
	{
		const float fmove = static_cast<float>(cmd->forwardmove);
		const float rmove = static_cast<float>(cmd->rightmove);

		float max = abs(fmove);
		if (abs(rmove) > max)
			max = abs(rmove);

		if (max == 0.0f)
			return 0.0f;

		const float total = sqrt(fmove * fmove + rmove * rmove);
		float scale = static_cast<float>(ps->speed) * max / (total * 127.0f);

		if ((ps->pm_flags & PMF_WALKING) || ps->leanf != 0.0f)
			scale *= 0.4f;
		if (ps->pm_type == PM_NOCLIP)
			scale *= 3.0f;
		if (ps->pm_type == PM_UFO)
			scale *= 6.0f;
		if (ps->pm_type == PM_SPECTATOR)
			return scale * player_spectateSpeedScale;
		return scale;
	}

	void CoD4::ProjectVelocity(const vec3 &velIn, const vec3 &normal, vec3 &out)
	{
		const float lengthSq2D = velIn[0] * velIn[0] + velIn[1] * velIn[1];

		if (std::abs(normal[2]) < 0.001f || lengthSq2D == 0.0f)
		{
			out = velIn;
			return;
		}
		const float newZ = -(normal[0] * velIn[0] + normal[1] * velIn[1]) / normal[2];
		const float originalLengthSq = lengthSq2D + velIn[2] * velIn[2];
		const float adjustedLengthSq = lengthSq2D + newZ * newZ;
		const float lengthScale = std::sqrt(originalLengthSq / adjustedLengthSq);

		if (lengthScale < 1.0f || newZ < 0.0f || velIn[2] > 0.0f)
		{
			out[0] = velIn[0] * lengthScale;
			out[1] = velIn[1] * lengthScale;
			out[2] = newZ * lengthScale;
		}
	}

	void CoD4::JumpClearState(playerState_tt *ps)
	{
		ps->pm_flags &= ~PMF_JUMPING;
		ps->jumpOriginZ = 0.0;
	}

	int CoD4::CorrectAllSolid(pmove_tt *pm, pml_tt *pml, trace_tt *trace)
	{
		playerState_tt *ps = pm->ps;
		vec3 &origin = ps->origin;

		float sx, sy, sz;
		unsigned int byteOffset = 0;

		while (true)
		{
			int idx = byteOffset / 12;
			sx = CorrectSolidDeltas[idx][0] + origin[0];
			sy = CorrectSolidDeltas[idx][1] + origin[1];
			sz = CorrectSolidDeltas[idx][2] + origin[2];

			vec3 start = { sx, sy, sz };
			PM_playerTrace(pm, trace, start, pm->mins, pm->maxs, start, ps->clientNum, pm->tracemask);

			if (!trace->startsolid)
				break;

			byteOffset += 12;
			if (byteOffset >= 312)
			{
				ps->pm_flags &= ~PMF_JUMPING;
				ps->jumpOriginZ = 0.0f;
				ps->groundEntityNum = ENTITYNUM_NONE;
				pml->groundPlane = 0;
				pml->almostGroundPlane = 0;
				pml->walking = 0;
				return 0;
			}
		}
		// Write corrected origin back
		origin[0] = sx;
		origin[1] = sy;
		origin[2] = sz;

		// Second trace: straight down from corrected origin
		vec3 end = { sx, sy, sz };
		end[2] = origin[2] - 1.0f - 0.25f;

		PM_playerTrace(pm, trace, origin, pm->mins, pm->maxs, end, ps->clientNum, pm->tracemask);

		// Capture deltas before writing
		float dx = sx - origin[0];
		float dy = sy - origin[1];
		float dz = end[2] - origin[2];
		float fraction = trace->fraction;

		memcpy(&pml->groundTrace, trace, sizeof(pml->groundTrace));

		origin[0] += dx * fraction;
		origin[1] += dy * fraction;
		origin[2] += dz * fraction;
		return 1;
	}

	void CoD4::GroundTraceMissed(pmove_tt *pm, pml_tt *pml)
	{
		playerState_tt *ps = pm->ps;

		vec3 end;
		end[0] = ps->origin[0];
		end[1] = ps->origin[1];
		end[2] = ps->origin[2];

		trace_tt trace;
		if (ps->groundEntityNum == ENTITYNUM_NONE)
		{
			end[2] = ps->origin[2] - 1.0f;

			G_TraceCapsule((trace_t *)&trace, glm::value_ptr(ps->origin), glm::value_ptr(pm->mins),
				glm::value_ptr(pm->maxs), glm::value_ptr(end), ps->clientNum, pm->tracemask);

			pml->almostGroundPlane = (trace.fraction != 1.0f) ? 1 : 0;
		}
		else
		{
			end[2] = ps->origin[2] - 64.0f;
			G_TraceCapsule((trace_t *)&trace, glm::value_ptr(ps->origin), glm::value_ptr(pm->mins),
				glm::value_ptr(pm->maxs), glm::value_ptr(end), ps->clientNum, pm->tracemask);

			if (trace.fraction == 1.0f)
			{
				pml->almostGroundPlane = 0;
			}
			else
			{
				pml->almostGroundPlane = (trace.fraction < 0.015625f) ? 1 : 0;
			}
		}
		ps->groundEntityNum = ENTITYNUM_NONE;
		pml->groundPlane = 0;
		pml->walking = 0;
	}

	void CoD4::CrashLand(playerState_tt *ps, pml_tt *pml)
	{
		float vel = pml->previous_velocity[2];
		float gravity = (float)ps->gravity;
		float acc = -gravity;
		float a = acc * 0.5f;
		float dist = pml->previous_origin[2] - ps->origin[2];
		float den = vel * vel - dist * (a * 4.0f);

		if (den < 0.0f)
			return;

		float sqrtDen = sqrtf(den);
		float t = (-vel - sqrtDen) / (a * 2.0f);
		float landVel = (t * acc + vel) * -1.0f;
		float fallHeight = (landVel * landVel) / (2.0f * gravity);

		int damage;
		if (bg_fallDamageMinHeight->value < bg_fallDamageMaxHeight->value)
		{
			if (bg_fallDamageMinHeight->value < fallHeight && (pml->groundTrace.surfaceFlags & 1) == 0
				&& ps->pm_type < PM_DEAD)
			{
				if (bg_fallDamageMaxHeight->value <= fallHeight)
				{
					damage = 100;
				}
				else
				{
					float ratio = (fallHeight - bg_fallDamageMinHeight->value)
						/ (bg_fallDamageMaxHeight->value - bg_fallDamageMinHeight->value) * 100.0f;
					int d = (int)ratio;
					damage = (d >= 100) ? 100 : (d > 0 ? d : 0);
				}
			}
			else
			{
				damage = 0;
			}
		}
		else
		{
			damage = 0;
		}
		int viewDip;
		if (fallHeight > 12.0f)
		{
			viewDip = (int)((fallHeight - 12.0f) / 26.0f * 4.0f + 4.0f);
			if (viewDip > 24)
				viewDip = 24;
		}
		else
		{
			viewDip = 0;
		}
		int surfaceFlags = pml->groundTrace.surfaceFlags;

		int v12;
		if (surfaceFlags & SURF_NOSTEPS)
		{
			v12 = 0;
		}
		else
		{
			v12 = (int)((surfaceFlags >> 20) & (SURF_NOIMPACT | SURF_LADDER | SURF_SKY | SURF_SLICK | SURF_NODAMAGE));
		}
		if (damage)
		{
			float scale;
			if (damage >= 100 || (surfaceFlags & SURF_SLICK) != 0)
			{
				scale = 0.6700000166893005f;
				ps->velocity[0] = ps->velocity[0] * scale;
				ps->velocity[1] = ps->velocity[1] * scale;
				ps->velocity[2] = scale * ps->velocity[2];
			}
			else
			{
				int time = 35 * damage + 500;
				float velocity_decrease_scale;

				if (time > 2000)
				{
					velocity_decrease_scale = 0.2f;
					time = 2000;
				}
				else if (time > 500)
				{
					if (time < 1500)
						velocity_decrease_scale =
							0.5f - ((float)(35 * damage + 500) - 500.0f) / 1000.0f * 0.300000011920929f;
					else
						velocity_decrease_scale = 0.2f;
				}
				else
				{
					velocity_decrease_scale = 0.5f;
				}
				ps->pm_flags |= PMF_TIME_HARDLANDING;
				ps->pm_time = time;
				ps->velocity[0] = ps->velocity[0] * velocity_decrease_scale;
				ps->velocity[1] = ps->velocity[1] * velocity_decrease_scale;
				ps->velocity[2] = velocity_decrease_scale * ps->velocity[2];
			}
			// Damage landing surface event
			if ((surfaceFlags & SURF_NOSTEPS) == 0)
			{
				int v17 = (surfaceFlags >> 20) & 0x1F;
				if (v17)
				{
					int ev = v17 + EV_LANDING_PAIN_FIRST;
					if (ev)
						BG_AddPredictableEventToPlayerstate(ev, damage, ps);
				}
			}
		}
		else if (fallHeight > 4.0f)
		{
			if (fallHeight >= 8.0f)
			{
				if (fallHeight >= 12.0f)
				{
					// Hard landing
					ps->velocity[0] = ps->velocity[0] * 0.6700000166893005;
					ps->velocity[1] = ps->velocity[1] * 0.6700000166893005;
					ps->velocity[2] = 0.6700000166893005 * ps->velocity[2];
					if ((surfaceFlags & SURF_NOSTEPS) == 0)
					{
						int v21 = (surfaceFlags >> 20) & 0x1F;
						if (v21)
						{
							int v22 = v21 + EV_LANDING_FIRST;
							if (v22)
							{
								BG_AddPredictableEventToPlayerstate(v22, viewDip, ps);
							}
						}
					}
				}
				else
				{
					// Medium landing (run footstep)
					if ((surfaceFlags & SURF_NOSTEPS) == 0)
					{
						int v20 = (surfaceFlags >> 20) & 0x1F;
						if (v20)
						{
							v20 = (v20 & ~0xFF) | (EV_FOOTSTEP_RUN & 0xFF);
							BG_AddPredictableEventToPlayerstate(v20, v12, ps);
						}
					}
				}
			}
			else
			{
				// Light landing (walk footstep)
				if ((surfaceFlags & SURF_NOSTEPS) == 0)
				{
					int v19 = (surfaceFlags >> 20) & 0x1F;
					if (v19)
					{
						v19 = (v19 & ~0xFF) | (EV_FOOTSTEP_WALK & 0xFF);
						BG_AddPredictableEventToPlayerstate(v19, v12, ps);
					}
				}
			}
		}
	}

	bool CoD4::InKnockback(playerState_tt *ps)
	{
		return (ps->pm_flags & PMF_TIME_KNOCKBACK) && ps->pm_time;
	}
}
