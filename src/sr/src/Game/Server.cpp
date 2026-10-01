#include "Server.hpp"
#include "Map.hpp"

#include "Audio/Voice.hpp"
#include "Game/Demo/DemoContainer.hpp"
#include "Game/Entity/Entity.hpp"
#include "Game/Player/Player.hpp"
#include "Game/VR.hpp"
#include "System/Debug.hpp"
#include "System/Vegas.hpp"

namespace SR
{
	void Server::Spawn(const std::string& levelName)
	{
		Log::WriteLine("^5[Server] Spawn server");

		Async::Shutdown();
		Async::Initialize();

		Player::List = {};
		Entity::List = {};
		DemoContainer::Demos.clear();
	}

	void Server::Restart()
	{
		Map::Restart();
	}

	void Server::Frame()
	{
		for (const auto& player : Player::List)
		{
			if (player)
			{
				player->Frame();
				Vegas::Frame(player);
			}
		}
		Map::Frame();
		Voice::Frame();
		VR::Frame();
		Debug::FPS();
	}
}
