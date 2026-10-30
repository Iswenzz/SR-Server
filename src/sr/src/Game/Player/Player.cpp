#include "Player.hpp"

namespace SR
{
	Player::Player(client_t *cl)
	{
		this->cl = cl;
		this->cl->clFrames = 0;
		this->ps = &cl->gentity->client->ps;

		std::fill(FrameTimes.begin(), FrameTimes.end(), 0);
	}

	void Player::Initialize()
	{
		DemoPlayer = CreateScope<class DemoPlayer>(this);
		PMove = CreateScope<class PMove>(this);
	}

	void Player::Disconnect()
	{
		DemoPlayer->Stop();
	}

	// A rewind never reaches back across a respawn.
	void Player::Spawn()
	{
		PMove->Initialize();

		RewindIndex = 0;
		RewindCount = 0;
	}

	void Player::CalculateFrame(int time)
	{
		FrameTimes[FrameStackIndex++ % PLAYER_FPS_STACK] = time;
		cl->clFrames++;
	}

	void Player::CalculateFPS()
	{
		int i = 0;
		int maxOccurence = -1;
		int value = -1;

		int possibleValues[1000] = { 0 };
		for (i = 0; i < PLAYER_FPS_STACK; i++)
		{
			if (FrameTimes[i] <= 0 || FrameTimes[i] >= 1000)
				continue;
			possibleValues[FrameTimes[i]]++;
		}
		for (i = 0; i < 1000; i++)
		{
			if (possibleValues[i] == 0 || possibleValues[i] < maxOccurence)
				continue;
			maxOccurence = possibleValues[i];
			value = i;
		}
		if (value > 0)
			cl->clFPS = static_cast<int>(1000 / value);
		cl->clFrames = 0;
		FrameStackIndex = 0;
		std::fill(this->FrameTimes.begin(), this->FrameTimes.end(), 0);
	}

	void Player::Packet(msg_t *msg)
	{
		DemoPlayer->Packet();
	}

	void Player::Frame()
	{
		if (!DEFINED_CLIENT(cl))
			return;

		CalculateFPS();
		VoiceChat();

		DemoPlayer->Frame();
		PMove->NotifyWeapons();
		RecordRewind();
	}

	void Player::VoiceChat()
	{
		LastVoiceTime = VoiceTime;
		VoiceTime = cl->gentity->client->lastVoiceTime;

		if (VoiceTime != LastVoiceTime)
		{
			Voice = true;
			VoiceCount = 0;
		}
		else if (Voice)
		{
			VoiceCount++;
			if (VoiceCount > 5)
			{
				Voice = false;
				VoiceCount = 0;
			}
		}
	}

	PlayerMoveState Player::GetMoveState() const
	{
		PlayerMoveState state{};
		state.pm_type = ps->pm_type;
		state.bobCycle = ps->bobCycle;
		state.pm_flags = ps->pm_flags;
		state.weapFlags = ps->weapFlags;
		state.otherFlags = ps->otherFlags;
		state.pm_time = ps->pm_time;
		VectorCopy(ps->origin, state.origin);
		VectorCopy(ps->velocity, state.velocity);
		Vector2Copy(ps->oldVelocity, state.oldVelocity);
		state.leanf = ps->leanf;
		state.groundEntityNum = ps->groundEntityNum;
		Vector2Copy(ps->vLadderVec, state.vLadderVec);
		state.jumpTime = ps->jumpTime;
		state.jumpOriginZ = ps->jumpOriginZ;
		state.damageTimer = ps->damageTimer;
		state.damageDuration = ps->damageDuration;
		state.movementDir = ps->movementDir;
		state.eFlags = ps->eFlags;
		state.eventSequence = ps->eventSequence;
		state.weapon = ps->weapon;
		state.adsDelayTime = ps->adsDelayTime;
		state.damageEvent = ps->damageEvent;
		state.damageYaw = ps->damageYaw;
		state.damagePitch = ps->damagePitch;
		state.damageCount = ps->damageCount;
		state.sprintState = ps->sprintState;
		state.holdBreathScale = ps->holdBreathScale;
		state.holdBreathTimer = ps->holdBreathTimer;
		state.moveSpeedScaleMultiplier = ps->moveSpeedScaleMultiplier;
		state.mantleState = ps->mantleState;
		state.entityEventSequence = ps->entityEventSequence;
		state.aimSpreadScale = ps->aimSpreadScale;
		return state;
	}

	// The teleport bit stays the live one, so the teleport that follows always toggles it against what
	// the client last saw and the move snaps instead of lerping across the map.
	void Player::SetMoveState(const PlayerMoveState &state)
	{
		ps->pm_type = state.pm_type;
		ps->bobCycle = state.bobCycle;
		ps->pm_flags = state.pm_flags;
		ps->weapFlags = state.weapFlags;
		ps->otherFlags = state.otherFlags;
		ps->pm_time = state.pm_time;
		VectorCopy(state.origin, ps->origin);
		VectorCopy(state.velocity, ps->velocity);
		Vector2Copy(state.oldVelocity, ps->oldVelocity);
		ps->leanf = state.leanf;
		ps->groundEntityNum = state.groundEntityNum;
		Vector2Copy(state.vLadderVec, ps->vLadderVec);
		ps->jumpTime = state.jumpTime;
		ps->jumpOriginZ = state.jumpOriginZ;
		ps->damageTimer = state.damageTimer;
		ps->damageDuration = state.damageDuration;
		ps->movementDir = state.movementDir;
		ps->eFlags = (state.eFlags & ~PLAYER_TELEPORT_BIT) | (ps->eFlags & PLAYER_TELEPORT_BIT);
		ps->eventSequence = state.eventSequence;
		ps->weapon = state.weapon;
		ps->adsDelayTime = state.adsDelayTime;
		ps->damageEvent = state.damageEvent;
		ps->damageYaw = state.damageYaw;
		ps->damagePitch = state.damagePitch;
		ps->damageCount = state.damageCount;
		ps->sprintState = state.sprintState;
		ps->holdBreathScale = state.holdBreathScale;
		ps->holdBreathTimer = state.holdBreathTimer;
		ps->moveSpeedScaleMultiplier = state.moveSpeedScaleMultiplier;
		ps->mantleState = state.mantleState;
		ps->entityEventSequence = state.entityEventSequence;
		ps->aimSpreadScale = state.aimSpreadScale;
	}

	void Player::RecordRewind()
	{
		if (cl->gentity->client->sess.sessionState != SESS_STATE_PLAYING)
			return;

		PlayerRewindFrame &frame = RewindFrames[RewindIndex];
		frame.time = level.time;
		VectorCopy(ps->viewangles, frame.viewangles);
		frame.state = GetMoveState();

		RewindIndex = (RewindIndex + 1) % PLAYER_REWIND_FRAMES;
		RewindCount = std::min(RewindCount + 1, PLAYER_REWIND_FRAMES);
	}

	// The newest frame recorded at or before time, or the oldest one when the history does not reach that far.
	const PlayerRewindFrame *Player::FindRewind(int time) const
	{
		const PlayerRewindFrame *oldest = nullptr;
		for (int i = 1; i <= RewindCount; i++)
		{
			const PlayerRewindFrame &frame = RewindFrames[(RewindIndex - i + PLAYER_REWIND_FRAMES) % PLAYER_REWIND_FRAMES];
			if (frame.time <= time)
				return &frame;
			oldest = &frame;
		}
		return oldest;
	}

	// TeleportPlayer leaves the velocity alone, so the restored one carries on.
	void Player::Rewind(const PlayerRewindFrame &frame)
	{
		SetMoveState(frame.state);

		vec3_t origin;
		vec3_t angles;
		VectorCopy(frame.state.origin, origin);
		VectorCopy(frame.viewangles, angles);
		TeleportPlayer(cl->gentity, origin, angles);
	}

	clientSnapshot_t *Player::GetFrame()
	{
		return &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
	}

	// Script methods can pass any entity number, not only a client one.
	const Ref<Player> &Player::Get(int num)
	{
		static const Ref<Player> none;
		if (num < 0 || num >= MAX_CLIENTS)
			return none;
		return List[num];
	}

	void Player::Add(client_t *cl)
	{
		Ref<Player> player = CreateRef<Player>(cl);
		player->Initialize();
		List[cl->gentity->s.number] = player;
	}
}
