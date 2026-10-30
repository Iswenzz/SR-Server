#include "Q3WeaponWorld.hpp"
#include "Player.hpp"
#include "PMove.hpp"

#include "Game/Demo/DemoPlayer.hpp"
#include "Movements/Q3Weapons.hpp"
#include "Utils/Utils.hpp"

// CS_EFFECT_TAGS, where playFXOnTag keeps its "<fx id><tag>" strings
#define CS_EFFECT_TAGS 0x6A2
#define CS_EFFECT_TAGS_MAX 0x100
// The engine frees a temp entity 300 ms after its event, so ours are forgotten a little later
#define Q3_IMPACT_LIFETIME 1000
#define Q3_TRACE_SOLID 0x1
// Impact effects sit this far off the surface hit, so their flat parts don't z-fight with it
#define Q3_IMPACT_LIFT 2.0f

IZ_C
{
	gentity_t *G_TempEntity(const float *origin, int event);
	int G_EffectIndex(const char *name);
	int G_SoundAliasIndex(const char *name);
	void G_DObjUpdate(gentity_t *ent);
}

namespace SR
{
	// Models, effects and sounds per Q3Weapons::Defs entry. The mod precaches all of them.
	struct Q3WeaponLook
	{
		const char *model;
		const char *trail;
		const char *impact;
		const char *impactSound;
	};

	static const std::array<Q3WeaponLook, 2> Looks = { {
		{ "quake_rocket_projectile", "q3/rocket_trail", "explosions/grenadeExp_default", "weap_quake_rocket_explode" },
		{ "tag_origin", "q3/plasma_fire", "q3/plasma_explode", "weap_quake_plasma_explode" },
	} };

	static void ToEngine(const vec3 &v, vec3_t out)
	{
		for (int i = 0; i < 3; i++)
			out[i] = v[i];
	}

	// Takes the missile events of one shooter: timeOffset maps their command time onto the server's time, which
	// the clients draw the missiles at.
	void Q3WeaponWorld::Events(int owner, const std::vector<Q3WeaponEvent> &events, int timeOffset, bool demo)
	{
		for (const Q3WeaponEvent &event : events)
		{
			if (event.type == Q3WeaponEventType::Fire)
				Fire(owner, event, timeOffset, demo);
			else
				Explode(owner, event, demo);
		}
	}

	// Ends the missiles the simulation no longer has, starts the trails of the ones spawned last frame, and forgets
	// the impacts the engine has freed.
	void Q3WeaponWorld::Frame()
	{
		for (Q3WeaponVisual &visual : Visuals)
		{
			const Q3WeaponState *state = State(visual.owner, visual.demo);
			bool alive = false;
			for (int i = 0; state && i < state->count; i++)
			{
				const Q3Projectile &projectile = state->projectiles[i];
				alive |= projectile.weapon == visual.weapon && projectile.trTime == visual.trTime;
			}
			if (!alive)
			{
				Free(visual);
				continue;
			}
			// The bolted trail needs the model on the client first, as GSC waits a frame before playFXOnTag
			if (!visual.trailed && visual.spawnTime < level.time)
			{
				const int fx = G_EffectIndex(Looks[visual.weapon].trail);
				if (fx > 0)
				{
					const char *tag = va("%02d%s", fx, "tag_origin");
					G_AddEvent(visual.ent, EV_PLAY_FX_ON_TAG,
						G_FindConfigstringIndex(tag, CS_EFFECT_TAGS, CS_EFFECT_TAGS_MAX, qtrue, nullptr));
				}
				visual.trailed = true;
			}
		}
		std::erase_if(Visuals, [](const Q3WeaponVisual &visual) { return !visual.ent; });
		std::erase_if(Impacts, [](const Q3WeaponImpact &impact) { return level.time >= impact.expireTime; });
	}

	// Drops the missiles of one shooter, or of their demo playback.
	void Q3WeaponWorld::Clear(int owner, bool demo)
	{
		for (Q3WeaponVisual &visual : Visuals)
		{
			if (visual.owner == owner && visual.demo == demo)
				Free(visual);
		}
		std::erase_if(Visuals, [](const Q3WeaponVisual &visual) { return !visual.ent; });
	}

	// The engine frees every entity on a map change.
	void Q3WeaponWorld::Reset()
	{
		Visuals.clear();
		Impacts.clear();
	}

	// Snapshot rule for the entities above, by the client receiving the snapshot and the player it views.
	bool Q3WeaponWorld::Hidden(int receiver, int viewed, gentity_t *ent)
	{
		for (const Q3WeaponVisual &visual : Visuals)
		{
			if (visual.ent == ent)
				return !Visible(receiver, viewed, visual.owner, visual.demo);
		}
		for (const Q3WeaponImpact &impact : Impacts)
		{
			if (impact.ent == ent)
				return !Visible(receiver, viewed, impact.owner, impact.demo);
		}
		return false;
	}

	// Shooters at IW3SR 1.10.0 draw their own missiles. Everyone viewing the shooter sees the server's, and others
	// only when the mod shows them. Demo missiles belong to their viewer alone.
	bool Q3WeaponWorld::Visible(int receiver, int viewed, int owner, bool demo)
	{
		if (demo)
			return receiver == owner || viewed == owner;
		if (receiver == owner)
			return !Utils::ClientVersion(owner, 1, 10, 0);
		if (viewed == owner)
			return true;

		const auto &player = Player::Get(owner);
		return player && player->Q3ShowOthers;
	}

	void Q3WeaponWorld::Fire(int owner, const Q3WeaponEvent &event, int timeOffset, bool demo)
	{
		const Q3WeaponLook &look = Looks[event.weapon];
		const Q3WeaponDef &def = Q3Weapons::Defs[event.weapon];
		const auto &player = Player::Get(owner);
		const bool cpm = player && player->PMove->IsQ3Cpm();

		gentity_t *ent = G_Spawn();
		ent->s.eType = ET_GENERAL;
		ent->r.contents = 0;
		ent->r.svFlags |= 0x8; // broadcast: the server never moves it, so its PVS origin would go stale
		G_SetModel(ent, look.model);
		G_DObjUpdate(ent);

		const vec3 delta = event.dir * (cpm ? def.cpmSpeed : def.speed);
		ent->s.lerp.pos.trType = TR_LINEAR;
		ent->s.lerp.pos.trTime = event.trTime + timeOffset;
		ent->s.lerp.pos.trDuration = 0;
		for (int i = 0; i < 3; i++)
		{
			ent->s.lerp.pos.trBase[i] = event.origin[i];
			ent->s.lerp.pos.trDelta[i] = static_cast<float>(static_cast<int>(delta[i]));
			ent->r.currentOrigin[i] = event.origin[i];
		}
		vec3_t dir, angles;
		ToEngine(event.dir, dir);
		vectoangles(dir, angles);
		VectorCopy(angles, ent->s.lerp.apos.trBase);
		VectorCopy(angles, ent->r.currentAngles);
		SV_LinkEntity(ent);

		Visuals.push_back({ owner, event.weapon, event.trTime, demo, false, level.time, ent });
	}

	void Q3WeaponWorld::Explode(int owner, const Q3WeaponEvent &event, bool demo)
	{
		for (Q3WeaponVisual &visual : Visuals)
		{
			if (visual.owner == owner && visual.demo == demo && visual.weapon == event.weapon
				&& visual.trTime == event.trTime)
				Free(visual);
		}
		std::erase_if(Visuals, [](const Q3WeaponVisual &visual) { return !visual.ent; });

		const Q3WeaponLook &look = Looks[event.weapon];
		vec3_t origin;
		ToEngine(event.origin + event.dir * Q3_IMPACT_LIFT, origin);

		if (const int fx = G_EffectIndex(look.impact); fx > 0)
		{
			vec3_t normal;
			ToEngine(event.dir, normal);

			gentity_t *ent = G_TempEntity(origin, EV_PLAY_FX);
			ent->s.eventParm = fx;
			vectoangles(normal, ent->s.lerp.apos.trBase); // faces out of the surface hit
			Impacts.push_back({ owner, demo, level.time + Q3_IMPACT_LIFETIME, ent });
		}
		gentity_t *sound = G_TempEntity(origin, EV_SOUND_ALIAS);
		sound->s.eventParm = G_SoundAliasIndex(look.impactSound);
		Impacts.push_back({ owner, demo, level.time + Q3_IMPACT_LIFETIME, sound });

		if (!demo)
			Push(owner, event);
	}

	// G_RadiusDamage's knockback on everyone but the shooter, when the mod lets missiles hit players.
	void Q3WeaponWorld::Push(int owner, const Q3WeaponEvent &event)
	{
		const auto &shooter = Player::Get(owner);
		if (!shooter || !shooter->Q3HitPlayers)
			return;

		const auto visible = [](const vec3 &from, const vec3 &to)
		{
			vec3_t start, end;
			const vec3_t zero = { 0, 0, 0 };
			ToEngine(from, start);
			ToEngine(to, end);
			trace_t trace = {};
			trace.fraction = 1.0f;
			G_TraceCapsule(&trace, start, zero, zero, end, ENTITYNUM_NONE, Q3_TRACE_SOLID);
			return trace.fraction == 1.0f;
		};
		for (const auto &player : Player::List)
		{
			if (!player || player == shooter || player->DemoPlayer->Demo)
				continue;

			gentity_t *ent = SV_GentityNum(player->ps->clientNum);
			if (!ent->client || ent->client->sess.sessionState != SESS_STATE_PLAYING || player->ps->pm_type != PM_NORMAL)
				continue;

			auto *ps = reinterpret_cast<playerState_tt *>(player->ps);
			Q3Weapons::Splash(ps, event.weapon, event.damageScale, event.origin, false, false, visible);
		}
	}

	const Q3WeaponState *Q3WeaponWorld::State(int owner, bool demo)
	{
		const auto &player = Player::Get(owner);
		if (!player)
			return nullptr;
		return demo ? player->DemoPlayer->Weapons.get() : player->PMove->Weapons.get();
	}

	void Q3WeaponWorld::Free(Q3WeaponVisual &visual)
	{
		if (visual.ent)
			G_FreeEntity(visual.ent);
		visual.ent = nullptr;
	}
}
