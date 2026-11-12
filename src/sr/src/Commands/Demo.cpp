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
		Scr_AddMethod("getdemoruntime", GetDemoRunTime, qfalse);
		Scr_AddMethod("stopdemo", StopDemo, qfalse);
		Scr_AddMethod("startghost", StartGhost, qfalse);
		Scr_AddMethod("finishghost", FinishGhost, qfalse);
		Scr_AddMethod("stopghost", StopGhost, qfalse);

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

	// The name is the one IW3SR shows for the demo; without one, or when it is undefined, the demo's own.
	void DemoCommands::PlayDemo(scr_entref_t num)
	{
		const int params = Scr_GetNumParam();
		if (params < 1 || params > 2)
		{
			Scr_Error("Usage: PlayDemo(<id>, <name?>)");
			return;
		}

		Ref<Player> player = Player::Get(num.entnum);
		std::string id = Scr_GetString(0);
		std::string name = params > 1 && Scr_GetType(1) == VAR_STRING ? Scr_GetString(1) : "";

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}

		// Frames are still being written by the loader until IsLoaded; no camera means "corrupted" to the script.
		auto demo = DemoContainer::Demos.find(id);
		if (demo != std::end(DemoContainer::Demos) && demo->second->IsLoaded && !demo->second->Frames.empty())
		{
			player->DemoPlayer->Play(demo->second, name);
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

	void DemoCommands::GetDemoRunTime(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: GetDemoRunTime()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		Scr_AddInt(player->DemoPlayer->RunTime());
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

	// The time is where in the run the ghost starts, zero without one. False while the demo is not loaded.
	void DemoCommands::StartGhost(scr_entref_t num)
	{
		const int params = Scr_GetNumParam();
		if (params < 1 || params > 4)
		{
			Scr_Error("Usage: StartGhost(<id>, <name?>, <time?>, <record?>)");
			return;
		}

		Ref<Player> player = Player::Get(num.entnum);
		std::string id = Scr_GetString(0);
		std::string name = params > 1 && Scr_GetType(1) == VAR_STRING ? Scr_GetString(1) : "";
		const int time = params > 2 ? Scr_GetInt(2) : 0;
		const int record = params > 3 ? Scr_GetInt(3) : 0;

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}

		auto demo = DemoContainer::Demos.find(id);
		const bool loaded =
			demo != std::end(DemoContainer::Demos) && demo->second->IsLoaded && !demo->second->Frames.empty();
		if (loaded)
			player->Ghost->Play(demo->second, name, time, record);
		Scr_AddBool(loaded);
	}

	void DemoCommands::FinishGhost(scr_entref_t num)
	{
		CHECK_PARAMS(1, "Usage: FinishGhost(<time>)");

		Ref<Player> player = Player::Get(num.entnum);
		const int time = Scr_GetInt(0);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		player->Ghost->Finish(time);
	}

	void DemoCommands::StopGhost(scr_entref_t num)
	{
		CHECK_PARAMS(0, "Usage: StopGhost()");

		Ref<Player> player = Player::Get(num.entnum);

		if (!player)
		{
			Scr_ObjectError("Player not found.\n");
			return;
		}
		player->Ghost->Stop();
	}
}
