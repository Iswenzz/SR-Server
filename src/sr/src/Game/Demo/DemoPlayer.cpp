#include "DemoPlayer.hpp"

#include "Utils/Utils.hpp"

#define DEMO_STATE_VERSION 1
#define DEMO_NAME_MAX 63

namespace SR
{
	DemoPlayer::DemoPlayer(class Player *player)
	{
		Player = player;
	}

	// An empty name is the one recorded in the demo.
	void DemoPlayer::Play(const Ref<class Demo> &demo, const std::string &name)
	{
		Demo = demo;
		Name = name.empty() ? demo->PlayerName : name;
		FrameIndex = 0;
		PreviousFrameIndex = 0;
		Clock = 0;
		Rate = 1;
		LastServerTime = 0;
		HasFrame = false;

		Entity = G_Spawn();
		Scr_SetString(&Entity->classname, scr_const.script_origin);
		G_CallSpawnEntity(Entity);
	}

	// The camera entity belongs to the script, which deletes it.
	void DemoPlayer::Stop()
	{
		Player->cl->deltaMessage = 0;
		Demo.reset();
		Entity = nullptr;
		HasFrame = false;
	}

	void DemoPlayer::UpdateEntity(snapshotInfo_t *snapInfo, msg_t *msg, const int time, entityState_t *from,
		entityState_t *to, qboolean force)
	{
		if (!from || !to)
		{
			MSG_WriteDeltaEntity(snapInfo, msg, time, from, to, force);
			return;
		}
		entityState_t entity = *to;
		gentity_t *gEntity = &g_entities[to->number];
		bool shouldUpdate = gEntity && gEntity->classname == scr_const.script_brushmodel;

		auto it = CurrentFrame.entities.find(to->number);
		if (it != CurrentFrame.entities.end() && shouldUpdate)
		{
			entityState_t demoEntity = it->second;
			vec3_t pos;

			entity.lerp.pos.trType = TR_STATIONARY;
			entity.lerp.pos.trTime = 0;
			entity.lerp.pos.trDuration = 0;
			BG_EvaluateTrajectory(&demoEntity.lerp.pos, CurrentFrame.ps.commandTime, pos);
			VectorCopy(demoEntity.lerp.pos.trDelta, entity.lerp.pos.trDelta);
			VectorCopy(pos, entity.lerp.pos.trBase);

			entity.lerp.apos.trType = TR_STATIONARY;
			entity.lerp.apos.trTime = 0;
			entity.lerp.apos.trDuration = 0;
			BG_EvaluateTrajectory(&demoEntity.lerp.apos, CurrentFrame.ps.commandTime, pos);
			VectorCopy(demoEntity.lerp.apos.trDelta, entity.lerp.apos.trDelta);
			VectorCopy(pos, entity.lerp.apos.trBase);
		}
		MSG_WriteDeltaEntity(snapInfo, msg, time, from, &entity, force);
	}

	bool DemoPlayer::ComputeFrame()
	{
		if (!Demo || Demo->Frames.empty())
			return false;

		// Controls
		int direction = Player->cl->lastUsercmd.forwardmove < 0 ? -1 : 1;
		bool fastForward = Player->cl->lastUsercmd.forwardmove > 0;
		bool slowmo = Player->cl->lastUsercmd.buttons & KEY_MASK_JUMP;
		float speed = slowmo ? 0.1f : fastForward ? 2.0f : 1.0f;

		// Pace playback with the demo timestamps rather than the server framerate,
		// so demos recorded at any snapshot rate play back at real speed
		if (!HasFrame)
			Clock = Demo->Frames.front().time;

		int frameTime = LastServerTime ? svs.time - LastServerTime : 0;
		LastServerTime = svs.time;
		Rate = speed * direction;
		Clock += frameTime * Rate;

		// EOF forward, or rewound past the beginning
		if (Clock < Demo->Frames.front().time || Clock > Demo->Frames.back().time)
		{
			Stop();
			return false;
		}

		PreviousFrameIndex = FrameIndex;
		while (FrameIndex + 1 < Demo->Frames.size() && Demo->Frames.at(FrameIndex + 1).time <= Clock)
			FrameIndex++;
		while (FrameIndex > 0 && Demo->Frames.at(FrameIndex).time > Clock)
			FrameIndex--;

		DemoFrame frame = Demo->Frames.at(FrameIndex);
		if (FrameIndex + 1 < Demo->Frames.size())
		{
			const DemoFrame &next = Demo->Frames.at(FrameIndex + 1);
			int duration = next.time - frame.time;
			if (duration > 0)
				InterpolateFrame(frame, next, (Clock - frame.time) / duration);
		}
		CurrentFrame = frame;
		HasFrame = true;

		return true;
	}

	void DemoPlayer::InterpolateFrame(DemoFrame &frame, const DemoFrame &next, float interpolate)
	{
		// Only interpolate continuous motion: velocity and the movement keys must
		// stay the exact recorded values, not a blend of two frames
		frame.ps.commandTime = std::lerp(frame.ps.commandTime, next.ps.commandTime, interpolate);

		frame.ps.origin[0] = std::lerp(frame.ps.origin[0], next.ps.origin[0], interpolate);
		frame.ps.origin[1] = std::lerp(frame.ps.origin[1], next.ps.origin[1], interpolate);
		frame.ps.origin[2] = std::lerp(frame.ps.origin[2], next.ps.origin[2], interpolate);

		// Prevent angle clamp interpolation
		if (std::abs(frame.ps.viewangles[0] - next.ps.viewangles[0]) < 170)
			frame.ps.viewangles[0] = std::lerp(frame.ps.viewangles[0], next.ps.viewangles[0], interpolate);
		if (std::abs(frame.ps.viewangles[1] - next.ps.viewangles[1]) < 170)
			frame.ps.viewangles[1] = std::lerp(frame.ps.viewangles[1], next.ps.viewangles[1], interpolate);
		if (std::abs(frame.ps.viewangles[2] - next.ps.viewangles[2]) < 170)
			frame.ps.viewangles[2] = std::lerp(frame.ps.viewangles[2], next.ps.viewangles[2], interpolate);
	}

	// The run timer at the playback clock: version 1 demos start 900ms into the run, later ones 100ms before it.
	int DemoPlayer::RunTime() const
	{
		if (!Demo || !HasFrame)
			return 0;
		return std::max(0, static_cast<int>(Clock) + (Demo->Version == 1 ? 900 : -100));
	}

	// IW3SR shows the demo's state since 1.9.1; any other client would only print the datagrams as unknown.
	// The server time is the snapshot's, so the client can carry the run time on between two of them.
	void DemoPlayer::Send(client_t *cl) const
	{
		if (!Demo || !HasFrame || !Utils::ClientVersion(cl - svs.clients, 1, 9, 1))
			return;

		byte buffer[128];
		msg_t msg;
		MSG_Init(&msg, buffer, sizeof(buffer));
		MSG_WriteString(&msg, "srdemo");
		MSG_WriteByte(&msg, DEMO_STATE_VERSION);
		MSG_WriteLong(&msg, svs.time);
		MSG_WriteLong(&msg, RunTime());
		MSG_WriteShort(&msg, static_cast<int>(std::round(Rate * 100)));
		MSG_WriteShort(&msg, std::clamp(Velocity, 0, 0x7FFF));
		MSG_WriteShort(&msg, std::clamp(CurrentFrame.fps, 0, 0x7FFF));
		MSG_WriteByte(&msg, CurrentFrame.forwardmove);
		MSG_WriteByte(&msg, CurrentFrame.rightmove);
		MSG_WriteLong(&msg, CurrentFrame.buttons);
		MSG_WriteString(&msg, Name.substr(0, DEMO_NAME_MAX).c_str());
		if (!msg.overflowed)
			NET_OutOfBandData(NS_SERVER, &cl->netchan.remoteAddress, msg.data, msg.cursize);
	}

	void DemoPlayer::Packet()
	{
		if (!Demo || !HasFrame)
			return;

		playerState_t originalPS = *Player->ps;

		// Copy the demo frame to the player without huds.
		memcpy(Player->ps, &CurrentFrame.ps, sizeof(playerState_t) - sizeof(hudElemState_t));
		Player->cl->clFPS = CurrentFrame.fps;

		// State
		Player->ps->clientNum = originalPS.clientNum;
		Player->ps->commandTime = originalPS.commandTime;
		Player->ps->viewlocked = originalPS.viewlocked;
		Player->ps->viewlocked_entNum = originalPS.viewlocked_entNum;
		Player->ps->viewHeightCurrent = originalPS.viewHeightCurrent;
		Player->ps->viewHeightLerpDown = originalPS.viewHeightLerpDown;
		Player->ps->viewHeightLerpTarget = originalPS.viewHeightLerpTarget;
		Player->ps->viewHeightLerpTime = originalPS.viewHeightLerpTime;
		Player->ps->viewHeightTarget = originalPS.viewHeightTarget;
		Player->ps->pm_type = originalPS.pm_type;
		Player->ps->eFlags = originalPS.eFlags;
		Player->ps->otherFlags = originalPS.otherFlags;
		Player->ps->pm_flags &= ~PMF_PRONING;
		Player->ps->pm_flags &= ~PMF_CROUCHING;
		Vector2Copy(originalPS.viewAngleClampBase, Player->ps->viewAngleClampBase);
		Vector2Copy(originalPS.viewAngleClampRange, Player->ps->viewAngleClampRange);
		Player->ps->killCamEntity = originalPS.killCamEntity;
		memcpy(&Player->ps->stats, &originalPS.stats, sizeof(Player->ps->stats));

		// Weapon
		Player->ps->viewmodelIndex = originalPS.viewmodelIndex;
		Player->ps->weapon = originalPS.weapon;
		Player->ps->weaponstate = originalPS.weaponstate;
		memcpy(&Player->ps->ammo, &originalPS.ammo, sizeof(Player->ps->ammo));
		memcpy(&Player->ps->ammoclip, &originalPS.ammoclip, sizeof(Player->ps->ammoclip));
		memcpy(&Player->ps->weapons, &originalPS.weapons, sizeof(Player->ps->weapons));
		memcpy(&Player->ps->weaponold, &originalPS.weaponold, sizeof(Player->ps->weaponold));
		memcpy(&Player->ps->weaponrechamber, &originalPS.weaponrechamber, sizeof(Player->ps->weaponrechamber));
		memcpy(&Player->ps->weaponmodels, &originalPS.weaponmodels, sizeof(Player->ps->weaponmodels));
		int weaponId = CurrentFrame.ps.weapon - 1;
		Weapon = weaponId < Demo->Weapons.size() && weaponId >= 0 ? Demo->Weapons[weaponId] : "";

		// Movement
		VectorCopy(originalPS.delta_angles, Player->ps->delta_angles);
		VectorCopy(CurrentFrame.ps.viewangles, Entity->r.currentAngles);
		VectorCopy(CurrentFrame.ps.origin, Player->ps->origin);
		VectorCopy(CurrentFrame.ps.velocity, Player->ps->velocity);
	}

	void DemoPlayer::Frame()
	{
		if (!ComputeFrame())
			return;

		clientSnapshot_t *frame = Player->GetFrame();
		Player->cl->clFPS = CurrentFrame.fps;

		// Movement
		VectorCopy(CurrentFrame.ps.origin, frame->ps.origin);
		SetClientViewAngle(Player->cl->gentity, CurrentFrame.ps.viewangles);
		VectorCopy(Player->ps->viewangles, frame->ps.viewangles);
		VectorCopy(Player->ps->delta_angles, frame->ps.delta_angles);
		Velocity = CurrentFrame.velocity;

		// Commands
		for (int i = PreviousFrameIndex + 1; i <= FrameIndex; i++)
			for (const std::string &message : Demo->Frames.at(i).chat)
				SV_SendServerCommand(Player->cl, "h \"^5[Demo] ^7%s\"", message.c_str());
	}

	// With each snapshot, to whoever sees a demo: the player watching it, and anyone following them. The
	// snapshot is built by now and not yet sent.
	void DemoPlayer::Snapshot(client_t *cl)
	{
		if (cl->state != CS_ACTIVE)
			return;

		const clientSnapshot_t *frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
		const auto &player = Player::Get(frame->ps.clientNum);
		if (player)
			player->DemoPlayer->Send(cl);
	}
}
