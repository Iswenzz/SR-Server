#include "GhostPlayer.hpp"

#include "Game/Player/Player.hpp"
#include "Utils/Utils.hpp"

namespace SR
{
	namespace
	{
		constexpr int MaxLead = 300;
		constexpr int UnknownPing = 999;
		constexpr int TurretFlags = 0x300;

		constexpr int StateVersion = 1;
		constexpr int StateDelta = 1;
		constexpr int NameMax = 63;

		// The frames of the path searched behind and ahead of where the player last was on it, then how far
		// from it, in units, they have to be for the whole path to be searched, and to count as off it.
		constexpr int TrackBehind = 40;
		constexpr int TrackAhead = 200;
		constexpr float TrackNear = 200.0f;
		constexpr float TrackFar = 1000.0f;
	}

	GhostPlayer::GhostPlayer(class Player *player)
	{
		Player = player;
	}

	void GhostPlayer::Initialize()
	{
		// A command reaches the server's snapshots about a snapshot interval plus 10ms after the view showed it.
		LeadOffset = Cvar_RegisterInt("sr_ghost_lead", 60, -500, 500, 0,
			"Milliseconds a ghost runs ahead of its player, on top of their ping");
	}

	// The run time is where in its run the ghost starts, for a timer that resumes instead of starting at zero.
	// The record time is the demo's finishing time, zero when unknown.
	void GhostPlayer::Play(const Ref<class Demo> &demo, const std::string &name, int runTime, int recordTime)
	{
		Demo = demo;
		Name = name.empty() ? demo->PlayerName : name;
		StartTime = svs.time - runTime;
		FrameIndex = 0;
		RecordTime = recordTime;
		Progress = 0;
		TrackTime = 0;
		HasDelta = false;
		Finished = false;

		// Latched for the run: a lead that follows the ping would shake the ghost.
		const int ping = Player->cl->ping;
		Lead = (ping < UnknownPing ? std::clamp(ping, 0, MaxLead) : 0) + LeadOffset->integer;

		// The demo's weapon indexes are those of the server that recorded it.
		Weapons.assign(demo->Weapons.size() + 1, 0);
		const int count = BG_GetNumWeapons();
		for (size_t i = 0; i < demo->Weapons.size(); i++)
		{
			for (int weapon = 1; weapon < count; weapon++)
			{
				if (demo->Weapons[i] == BG_GetWeaponDef(weapon)->szInternalName)
				{
					Weapons[i + 1] = weapon;
					break;
				}
			}
		}
	}

	// The run ended at this time: the delta stays at how it compares to the record.
	void GhostPlayer::Finish(int time)
	{
		if (!Demo)
			return;

		Finished = true;
		if (RecordTime > 0)
		{
			Delta = time - RecordTime;
			HasDelta = true;
		}
	}

	void GhostPlayer::Stop()
	{
		Demo.reset();
		Weapons.clear();
	}

	// Entities 0 to 63 are player slots on every client whatever sv_maxclients is, so the last one is free on
	// any smaller server. A 64 slot server lends a free one, and has no ghost when full.
	int GhostPlayer::Slot()
	{
		for (int i = MAX_CLIENTS - 1; i >= 0; i--)
		{
			if (i >= sv_maxclients->integer || svs.clients[i].state == CS_FREE)
				return i;
		}
		return -1;
	}

	// The recorded state at the receiver's point of the run, held at either end of the demo. The run's own
	// player sees themselves predicted, ahead of every snapshot by their latency, so their ghost is led by
	// it; a follower gets both from the same snapshot.
	bool GhostPlayer::Frame(const client_t *receiver, playerState_t &ps)
	{
		if (!Demo || Demo->Frames.empty())
			return false;

		const std::vector<DemoFrame> &frames = Demo->Frames;
		const int last = static_cast<int>(frames.size()) - 1;
		const int runTime = svs.time - StartTime + (receiver == Player->cl ? Lead : 0);
		const int clock = std::clamp(runTime - Demo->RunOffset(), frames.front().time, frames.back().time);

		FrameIndex = std::clamp(FrameIndex, 0, last);
		while (FrameIndex < last && frames[FrameIndex + 1].time <= clock)
			FrameIndex++;
		while (FrameIndex > 0 && frames[FrameIndex].time > clock)
			FrameIndex--;

		const DemoFrame &frame = frames[FrameIndex];
		ps = frame.ps;
		if (FrameIndex == last)
			return true;

		// A teleport is a cut, not a move to blend across.
		const DemoFrame &next = frames[FrameIndex + 1];
		const int duration = next.time - frame.time;
		if (duration <= 0 || ((frame.ps.eFlags ^ next.ps.eFlags) & PLAYER_TELEPORT_BIT))
			return true;

		const float fraction = static_cast<float>(clock - frame.time) / duration;
		for (int i = 0; i < 3; i++)
		{
			const float turn = AngleNormalize180(next.ps.viewangles[i] - frame.ps.viewangles[i]);
			ps.origin[i] = std::lerp(frame.ps.origin[i], next.ps.origin[i], fraction);
			ps.viewangles[i] = frame.ps.viewangles[i] + turn * fraction;
		}
		return true;
	}

	// The ghost as its slot's player entity and client in a snapshot, or -1 without one. It wears the
	// player's own model. The recorded events stay out: they would run as the slot's server events and
	// play the run's sounds.
	int GhostPlayer::Snapshot(const client_t *receiver, entityState_t *entity, clientState_t *client)
	{
		const int slot = Slot();
		playerState_t ps;
		if (slot < 0 || !Frame(receiver, ps))
			return -1;

		ps.clientNum = slot;
		ps.eFlags &= ~TurretFlags;
		ps.oldEventSequence = ps.eventSequence;
		ps.entityEventSequence = ps.eventSequence;

		*entity = {};
		entity->number = slot;
		BG_PlayerStateToEntityState(&ps, entity, qfalse, 1);
		entity->eType = ET_PLAYER;
		entity->weapon = ps.weapon < Weapons.size() ? Weapons[ps.weapon] : 0;
		entity->weaponModel = 0;

		// A mover the run stood on is elsewhere now, and the client would carry the ghost along with it.
		if (entity->groundEntityNum < ENTITYNUM_WORLD)
			entity->groundEntityNum = ENTITYNUM_WORLD;

		*client = *G_GetClientState(Player->cl - svs.clients);
		client->clientIndex = slot;
		Q_strncpyz(client->netname, Name.c_str(), sizeof(client->netname));
		return slot;
	}

	// The squared distance from a point to the recorded path over a range of its frames, with the frame the
	// nearest stretch starts at and how far along it the nearest point is. A teleport joins nothing.
	float GhostPlayer::Nearest(const float *origin, int from, int to, int &index, float &fraction) const
	{
		const std::vector<DemoFrame> &frames = Demo->Frames;
		float best = std::numeric_limits<float>::max();

		for (int i = from; i < to; i++)
		{
			const playerState_t &start = frames[i].ps;
			const playerState_t &end = frames[i + 1].ps;
			const bool cut = (start.eFlags ^ end.eFlags) & PLAYER_TELEPORT_BIT;

			vec3_t stretch, offset, apart;
			VectorSubtract(end.origin, start.origin, stretch);
			VectorSubtract(origin, start.origin, offset);

			const float length = DotProduct(stretch, stretch);
			const float along = cut || length <= 0 ? 0 : std::clamp(DotProduct(offset, stretch) / length, 0.0f, 1.0f);
			VectorMA(offset, -along, stretch, apart);

			const float distance = DotProduct(apart, apart);
			if (distance < best)
			{
				best = distance;
				index = i;
				fraction = along;
			}
		}
		return best;
	}

	// How long after the ghost the player passes where they are, in milliseconds, negative when ahead of
	// it. The path is searched around where they last were on it, so a route that crosses itself does not
	// jump to its other pass, and as a whole once they are far from that, for a shortcut or a load.
	void GhostPlayer::Track()
	{
		if (Finished || TrackTime == svs.time)
			return;
		TrackTime = svs.time;
		HasDelta = false;

		const std::vector<DemoFrame> &frames = Demo->Frames;
		const int last = static_cast<int>(frames.size()) - 1;
		if (last < 1)
			return;

		const float *origin = Player->ps->origin;
		int index = 0;
		float fraction = 0;
		Progress = std::clamp(Progress, 0, last - 1);
		const int from = std::max(Progress - TrackBehind, 0);
		const int to = std::min(Progress + TrackAhead, last);
		float distance = Nearest(origin, from, to, index, fraction);

		if (distance > TrackNear * TrackNear)
		{
			int otherIndex = 0;
			float otherFraction = 0;
			const float other = Nearest(origin, 0, last, otherIndex, otherFraction);

			// Half as far, the distances being squared.
			if (other < distance * 0.25f)
			{
				distance = other;
				index = otherIndex;
				fraction = otherFraction;
			}
		}
		if (distance > TrackFar * TrackFar)
			return;

		const float time = std::lerp(static_cast<float>(frames[index].time), static_cast<float>(frames[index + 1].time),
			fraction);
		Progress = index;
		Delta = svs.time - StartTime - (static_cast<int>(time) + Demo->RunOffset());
		HasDelta = true;
	}

	// IW3SR labels the ghost and shows the delta since 1.10.0; any other client would only print the
	// datagrams as unknown.
	void GhostPlayer::Send(client_t *cl)
	{
		const int slot = Slot();
		if (!Demo || Demo->Frames.empty() || slot < 0 || !Utils::ClientVersion(cl - svs.clients, 1, 10, 0))
			return;
		Track();

		byte buffer[128];
		msg_t msg;
		MSG_Init(&msg, buffer, sizeof(buffer));
		MSG_WriteString(&msg, "srghost");
		MSG_WriteByte(&msg, StateVersion);
		MSG_WriteByte(&msg, slot);
		MSG_WriteByte(&msg, HasDelta ? StateDelta : 0);
		MSG_WriteLong(&msg, RecordTime);
		MSG_WriteLong(&msg, Delta);
		MSG_WriteString(&msg, Name.substr(0, NameMax).c_str());
		if (!msg.overflowed)
			NET_OutOfBandData(NS_SERVER, &cl->netchan.remoteAddress, msg.data, msg.cursize);
	}

	// With each snapshot, to whoever sees a ghost: its player, and anyone following them. The snapshot is
	// built by now and not yet sent.
	void GhostPlayer::State(client_t *cl)
	{
		if (cl->state != CS_ACTIVE)
			return;

		const clientSnapshot_t *frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
		const auto &player = Player::Get(frame->ps.clientNum);
		if (player)
			player->Ghost->Send(cl);
	}
}
