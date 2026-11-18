#pragma once
#include "Command.hpp"

namespace SR
{
	class EntityCommands
	{
	public:
		static void Register();

	private:
		static void FollowMovement(scr_entref_t num);
	};
}
