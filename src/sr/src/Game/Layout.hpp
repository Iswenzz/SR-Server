#pragma once
#include "Base.hpp"

namespace SR
{
	// Defrag maps ship two physics layouts: brushwork a map tags notcpm is gone
	// for CPM players, notvq3 for everyone else. Each player sees and collides
	// with their own.
	class Layout
	{
	public:
		static void Restart();
		static bool Hidden(int clientNum, gentity_t *ent);

	private:
		static inline std::unordered_map<int, int> ByModel;
		static inline bool Loaded = false;

		static void Load();
	};
}
