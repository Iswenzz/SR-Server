#include "Demo.hpp"

#include "DemoContainer.hpp"

#include "Utils/Utils.hpp"

namespace SR
{
	Demo::Demo(const std::string &id, const std::string &path)
	{
		ID = id;
		Reader = CreateScope<CoD4::DM1::DemoReader>(path);

		auto task = Async::Create(this);
		Async::Submit([this, task] { Open(task); });
	}

	Demo::~Demo()
	{
		Reader->Close();
	}

	void Demo::Open(const Ref<AsyncTask> &task)
	{
		std::scoped_lock lock(DemoContainer::Mutex);
		IsLoaded = false;
		DemoFrame previousFrame = { 0 };

		try
		{
			while (!task->IsCancelled() && Reader->Next())
			{
				DemoFrame frame = { 0 };

				if (!CanParseSnapshot())
					continue;

				ReadDemoInformations();

				auto snapshot = Reader->GetCurrentSnapshot();
				auto archive = Reader->GetCurrentFrame();
				auto ps = snapshot.ps;

				frame.valid = snapshot.valid;
				frame.chat = ProcessChat();
				frame.time = Reader->GetTimeMilliseconds();
				frame.fps = Reader->GetFPS();
				frame.ps = *reinterpret_cast<playerState_t *>(&ps);

				if (archive.commandTime)
				{
					VectorCopy(archive.angles, frame.ps.viewangles);
					VectorCopy(archive.velocity, frame.ps.velocity);
				}
				frame.velocity = GetVelocity(frame);
				frame.playerName = Reader->GetPlayerName().netname;
				frame.entities = previousFrame.entities;
				frame.forwardmove = *reinterpret_cast<char *>(&ps.dofNearStart);
				frame.rightmove = *reinterpret_cast<char *>(&ps.dofNearEnd);
				frame.buttons = *reinterpret_cast<int *>(&ps.dofFarStart);

				// Entities
				for (auto &ent : Reader->GetLastUpdatedEntities())
				{
					if (static_cast<int>(ent.eType) == ET_SCRIPTMOVER)
						frame.entities[ent.number] = *reinterpret_cast<entityState_t *>(&ent);
				}
				// Interpolate invalid packets
				if (frame.valid)
				{
					if (!previousFrame.valid)
						Interpolate(frame);
					LastValidFrame = Frames.size();
				}
				previousFrame = frame;
				Frames.push_back(frame);
			}
		}
		catch (...)
		{
		}
		Reader->Close();
		IsLoaded = true;
		task->Status = AsyncStatus::Successful;
	}

	void Demo::ReadDemoInformations()
	{
		if (HasReadInformations)
			return;
		HasReadInformations = true;

		MapName = Reader->ParseConfigString("mapname");
		PlayerName = Reader->GetPlayerName().netname;
		ConfigStrings = Reader->DemoFile->ConfigStrings;
		Weapons = Utils::SplitString(Reader->GetConfigString("defaultweapon_mp"), ' ');

		GetVersion();
	}

	void Demo::GetVersion()
	{
		std::vector<std::string> serverInfos = Utils::SplitString(Reader->DemoFile->ConfigStrings[0], '\\');
		if (serverInfos.empty())
			return;

		Version = 1;
		auto it = std::find(serverInfos.begin(), serverInfos.end(), "sr_demo_version");
		if (it != serverInfos.end() && std::next(it) != serverInfos.end())
			std::from_chars(std::next(it)->data(), std::next(it)->data() + std::next(it)->size(), Version);
	}

	int Demo::GetVelocity(DemoFrame &frame)
	{
		int frameVelocity = 0;
		if (frame.ps.velocity[0] != 0 || frame.ps.velocity[1] != 0 || frame.ps.velocity[2] != 0)
			frameVelocity =
				sqrtl((frame.ps.velocity[0] * frame.ps.velocity[0]) + (frame.ps.velocity[1] * frame.ps.velocity[1]));

		hudelem_t velocityHud{};
		for (int i = 0; i < MAX_HUDELEMENTS; i++)
		{
			if ((std::abs(frame.ps.hud.current[i].fontScale - 1.6) <= 0.05
					|| std::abs(frame.ps.hud.current[i].fontScale - 1.8) <= 0.05)
				&& frame.ps.hud.current[i].value > 0)
			{
				velocityHud = frame.ps.hud.current[i];
				break;
			}
		}
		return frameVelocity >= velocityHud.value ? frameVelocity : velocityHud.value;
	}

	void Demo::Interpolate(DemoFrame &interpolateFrame)
	{
		if (LastValidFrame >= Frames.size())
			return;

		DemoFrame &validFrame = Frames[LastValidFrame];

		for (int i = LastValidFrame + 1, c = 1; i < Frames.size(); i++, c++)
		{
			DemoFrame &frame = Frames[i];
			float interpolate = static_cast<float>(c) / (Frames.size() - LastValidFrame);

			frame.ps = interpolateFrame.ps;
			frame.buttons = interpolateFrame.buttons;
			frame.entities = interpolateFrame.entities;
			frame.forwardmove = std::lerp(validFrame.forwardmove, interpolateFrame.forwardmove, interpolate);
			frame.rightmove = std::lerp(validFrame.rightmove, interpolateFrame.rightmove, interpolate);
			frame.velocity = std::lerp(validFrame.velocity, interpolateFrame.velocity, interpolate);

			frame.ps.commandTime = std::lerp(validFrame.ps.commandTime, interpolateFrame.ps.commandTime, interpolate);
			frame.ps.legsTimer = std::lerp(validFrame.ps.legsTimer, interpolateFrame.ps.legsTimer, interpolate);
			frame.ps.weaponTime = std::lerp(validFrame.ps.weaponTime, interpolateFrame.ps.weaponTime, interpolate);
			frame.ps.weaponDelay = std::lerp(validFrame.ps.weaponDelay, interpolateFrame.ps.weaponDelay, interpolate);
			frame.ps.torsoTimer = std::lerp(validFrame.ps.torsoTimer, interpolateFrame.ps.torsoTimer, interpolate);
			frame.ps.jumpTime = std::lerp(validFrame.ps.jumpTime, interpolateFrame.ps.jumpTime, interpolate);
			frame.ps.pm_time = std::lerp(validFrame.ps.pm_time, interpolateFrame.ps.pm_time, interpolate);
			frame.ps.fWeaponPosFrac =
				std::lerp(validFrame.ps.fWeaponPosFrac, interpolateFrame.ps.fWeaponPosFrac, interpolate);
			frame.ps.adsDelayTime =
				std::lerp(validFrame.ps.adsDelayTime, interpolateFrame.ps.adsDelayTime, interpolate);
			frame.ps.deltaTime = std::lerp(validFrame.ps.deltaTime, interpolateFrame.ps.deltaTime, interpolate);
			frame.ps.legsAnimDuration =
				std::lerp(validFrame.ps.legsAnimDuration, interpolateFrame.ps.legsAnimDuration, interpolate);
			frame.ps.torsoAnimDuration =
				std::lerp(validFrame.ps.torsoAnimDuration, interpolateFrame.ps.torsoAnimDuration, interpolate);

			frame.ps.sprintState.sprintDelay = std::lerp(validFrame.ps.sprintState.sprintDelay,
				interpolateFrame.ps.sprintState.sprintDelay, interpolate);
			frame.ps.sprintState.lastSprintStart = std::lerp(validFrame.ps.sprintState.lastSprintStart,
				interpolateFrame.ps.sprintState.lastSprintStart, interpolate);
			frame.ps.sprintState.lastSprintEnd = std::lerp(validFrame.ps.sprintState.lastSprintEnd,
				interpolateFrame.ps.sprintState.lastSprintEnd, interpolate);
			frame.ps.sprintState.sprintStartMaxLength = std::lerp(validFrame.ps.sprintState.sprintStartMaxLength,
				interpolateFrame.ps.sprintState.sprintStartMaxLength, interpolate);
			frame.ps.mantleState.yaw =
				std::lerp(validFrame.ps.mantleState.yaw, interpolateFrame.ps.mantleState.yaw, interpolate);
			frame.ps.mantleState.timer =
				std::lerp(validFrame.ps.mantleState.timer, interpolateFrame.ps.mantleState.timer, interpolate);

			frame.ps.origin[0] = std::lerp(validFrame.ps.origin[0], interpolateFrame.ps.origin[0], interpolate);
			frame.ps.origin[1] = std::lerp(validFrame.ps.origin[1], interpolateFrame.ps.origin[1], interpolate);
			frame.ps.origin[2] = std::lerp(validFrame.ps.origin[2], interpolateFrame.ps.origin[2], interpolate);

			// Prevent angle clamp interpolation
			if (std::abs(validFrame.ps.viewangles[0] - interpolateFrame.ps.viewangles[0]) < 170)
				frame.ps.viewangles[0] =
					std::lerp(validFrame.ps.viewangles[0], interpolateFrame.ps.viewangles[0], interpolate);
			if (std::abs(validFrame.ps.viewangles[1] - interpolateFrame.ps.viewangles[1]) < 170)
				frame.ps.viewangles[1] =
					std::lerp(validFrame.ps.viewangles[1], interpolateFrame.ps.viewangles[1], interpolate);
			if (std::abs(validFrame.ps.viewangles[2] - interpolateFrame.ps.viewangles[2]) < 170)
				frame.ps.viewangles[2] =
					std::lerp(validFrame.ps.viewangles[2], interpolateFrame.ps.viewangles[2], interpolate);

			for (auto &[number, interpolateEntity] : interpolateFrame.entities)
			{
				if (validFrame.entities.find(number) == validFrame.entities.end())
					continue;

				entityState_t &frameEntity = frame.entities[number];
				entityState_t &validEntity = validFrame.entities[number];
				frameEntity = validEntity;

				frameEntity.lerp.pos.trTime =
					std::lerp(validEntity.lerp.pos.trTime, interpolateEntity.lerp.pos.trTime, interpolate);
				frameEntity.lerp.pos.trDuration =
					std::lerp(validEntity.lerp.pos.trDuration, interpolateEntity.lerp.pos.trDuration, interpolate);
				frameEntity.lerp.pos.trBase[0] =
					std::lerp(validEntity.lerp.pos.trBase[0], interpolateEntity.lerp.pos.trBase[0], interpolate);
				frameEntity.lerp.pos.trBase[1] =
					std::lerp(validEntity.lerp.pos.trBase[1], interpolateEntity.lerp.pos.trBase[1], interpolate);
				frameEntity.lerp.pos.trBase[2] =
					std::lerp(validEntity.lerp.pos.trBase[2], interpolateEntity.lerp.pos.trBase[2], interpolate);
				frameEntity.lerp.pos.trDelta[0] =
					std::lerp(validEntity.lerp.pos.trDelta[0], interpolateEntity.lerp.pos.trDelta[0], interpolate);
				frameEntity.lerp.pos.trDelta[1] =
					std::lerp(validEntity.lerp.pos.trDelta[1], interpolateEntity.lerp.pos.trDelta[1], interpolate);
				frameEntity.lerp.pos.trDelta[2] =
					std::lerp(validEntity.lerp.pos.trDelta[2], interpolateEntity.lerp.pos.trDelta[2], interpolate);

				frameEntity.lerp.apos.trTime =
					std::lerp(validEntity.lerp.apos.trTime, interpolateEntity.lerp.apos.trTime, interpolate);
				frameEntity.lerp.apos.trDuration =
					std::lerp(validEntity.lerp.apos.trDuration, interpolateEntity.lerp.apos.trDuration, interpolate);
				frameEntity.lerp.apos.trBase[0] =
					std::lerp(validEntity.lerp.apos.trBase[0], interpolateEntity.lerp.apos.trBase[0], interpolate);
				frameEntity.lerp.apos.trBase[1] =
					std::lerp(validEntity.lerp.apos.trBase[1], interpolateEntity.lerp.apos.trBase[1], interpolate);
				frameEntity.lerp.apos.trBase[2] =
					std::lerp(validEntity.lerp.apos.trBase[2], interpolateEntity.lerp.apos.trBase[2], interpolate);
				frameEntity.lerp.apos.trDelta[0] =
					std::lerp(validEntity.lerp.apos.trDelta[0], interpolateEntity.lerp.apos.trDelta[0], interpolate);
				frameEntity.lerp.apos.trDelta[1] =
					std::lerp(validEntity.lerp.apos.trDelta[1], interpolateEntity.lerp.apos.trDelta[1], interpolate);
				frameEntity.lerp.apos.trDelta[2] =
					std::lerp(validEntity.lerp.apos.trDelta[2], interpolateEntity.lerp.apos.trDelta[2], interpolate);
			}
		}
	}

	std::vector<std::string> Demo::ProcessChat()
	{
		std::vector<std::string> chat;
		std::vector<std::string> commands = Reader->GetLastCommandStrings();

		for (const std::string &command : commands)
		{
			// h "<message>"
			if (command.size() >= 4 && command[0] == 'h')
				chat.push_back(command.substr(3, command.size() - 4));
		}
		return chat;
	}

	bool Demo::CanParseSnapshot()
	{
		if (Reader->DemoFile->CurrentMessageType != CoD4::DM1::MSGType::MSG_SNAPSHOT)
			return false;
		if (!LastValidFrame && !Reader->GetCurrentSnapshot().valid)
			return false;
		return true;
	}
}
