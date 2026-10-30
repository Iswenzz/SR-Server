#pragma once
#include "Base.hpp"

namespace SR
{
	struct Q3WeaponEvent;
	struct Q3WeaponState;

	// A missile drawn by the server, followed by its owner and fire time.
	struct Q3WeaponVisual
	{
		int owner;
		int weapon;
		int trTime;
		bool demo;
		bool trailed;
		int spawnTime;
		gentity_t *ent;
	};

	// An impact effect or sound the server sent, kept until the engine frees it.
	struct Q3WeaponImpact
	{
		int owner;
		bool demo;
		int expireTime;
		gentity_t *ent;
	};

	// What the Q3 weapons do outside the shooter's own pmove: the missiles and impacts every client sees, and the
	// push on other players.
	class Q3WeaponWorld
	{
	public:
		static void Events(int owner, const std::vector<Q3WeaponEvent> &events, int timeOffset, bool demo);
		static void Frame();
		static void Clear(int owner, bool demo);
		static void Reset();
		static bool Hidden(int receiver, int viewed, gentity_t *ent);

	private:
		static inline std::vector<Q3WeaponVisual> Visuals;
		static inline std::vector<Q3WeaponImpact> Impacts;

		static void Fire(int owner, const Q3WeaponEvent &event, int timeOffset, bool demo);
		static void Explode(int owner, const Q3WeaponEvent &event, bool demo);
		static void Push(int owner, const Q3WeaponEvent &event);
		static const Q3WeaponState *State(int owner, bool demo);
		static void Free(Q3WeaponVisual &visual);
		static bool Visible(int receiver, int viewed, int owner, bool demo);
	};
}
