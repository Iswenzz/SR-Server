#include "Demo.hpp"

#include "Game/Demo/DemoContainer.hpp"

namespace SR
{
	void DemoCommands::Register()
	{
		Scr_AddMethod("playdemo", PlayDemo, qfalse);
		Scr_AddMethod("isdemoplaying", IsDemoPlaying, qfalse);
		Scr_AddMethod("getspeedrunvelocity", GetSpeedrunVelocity, qfalse);
		Scr_AddMethod("getdemoversion", GetDemoVersion, qfalse);
		Scr_AddMethod("getdemoweapon", GetDemoWeapon, qfalse);
		Scr_AddMethod("getdemorightmove", GetDemoRightMove, qfalse);
		Scr_AddMethod("getdemoforwardmove", GetDemoForwardMove, qfalse);
		Scr_AddMethod("getdemobuttons", GetDemoButtons, qfalse);
		Scr_AddMethod("getdemotimer", GetDemoTimer, qfalse);
		Scr_AddMethod("stopdemo", StopDemo, qfalse);

		Scr_AddFunction("registerspeedrundemo", RegisterSpeedrunDemo, qfalse);
		Scr_AddFunction("isdemoloaded", IsDemoLoaded, qfalse);
	}

	void DemoCommands::RegisterSpeedrunDemo()
	{
		CHECK_PARAMS(5, "Usage: RegisterSpeedrunDemo(<map>, <playerId>, <runId>, <mode>, <way>)");

		std::string map = Scr_GetString(0);
		std::string playerId = Scr_GetString(1);
		std::string runId = Scr_GetString(2);
		std::string mode = Scr_GetString(3);
		std::string way = Scr_GetString(4);

		Scr_AddInt(DemoContainer::RegisterSpeedrunDemo(map, playerId, runId, mode, way));
	}

	void DemoCommands::PlayDemo(scr_entref_t num)
	{
		CHECK_PARAMS(1, "Usage: PlayDemo(<id>)");

		Ref<Player> player = Player::Get(num.entnum);
		std::string id = Scr_GetString(0);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}

		// Frames are still being written by the loader until IsLoaded; no camera means "corrupted" to the script.
		auto demo = DemoContainer::Demos.find(id);
		if (demo != std::end(DemoContainer::Demos) && demo->second->IsLoaded && !demo->second->Frames.empty())
		{
			player->DemoPlayer->Play(demo->second);
			Scr_AddEntity(player->DemoPlayer->Entity);
		}
	}

	void DemoCommands::IsDemoLoaded()
	{
		CHECK_PARAMS(1, "Usage: IsDemoLoaded(<id>)");

		std::string id = Scr_GetString(0);
		auto demo = DemoContainer::Demos.find(id);

		Scr_AddBool(demo != std::end(DemoContainer::Demos) ? demo->second->IsLoaded.load() : false);
	}

	void DemoCommands::IsDemoPlaying(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: IsDemoPlaying()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddBool(!!player->DemoPlayer->Demo);
	}

	void DemoCommands::GetSpeedrunVelocity(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: GetSpeedrunVelocity()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->DemoPlayer->Velocity);
	}

	void DemoCommands::GetDemoVersion(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: GetDemoVersion()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->DemoPlayer->Demo ? player->DemoPlayer->Demo->Version : 1);
	}

	void DemoCommands::GetDemoWeapon(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: GetDemoWeapon()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddString(player->DemoPlayer->Weapon.c_str());
	}

	void DemoCommands::GetDemoRightMove(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: GetDemoRightMove()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->DemoPlayer->Demo ? player->DemoPlayer->CurrentFrame.rightmove : 0);
	}

	void DemoCommands::GetDemoForwardMove(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: GetDemoForwardMove()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->DemoPlayer->Demo ? player->DemoPlayer->CurrentFrame.forwardmove : 0);
	}

	void DemoCommands::GetDemoTimer(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: GetDemoTimer()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->DemoPlayer->Demo ? player->DemoPlayer->CurrentFrame.time : 0);
	}

	void DemoCommands::GetDemoButtons(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: GetDemoButtons()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->DemoPlayer->Demo ? player->DemoPlayer->CurrentFrame.buttons : 0);
	}

	void DemoCommands::StopDemo(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: StopDemo()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		player->DemoPlayer->Stop();
	}
}
