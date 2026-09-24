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
		SaveState = CreateScope<playerState_t>();
	}

	void Player::Disconnect()
	{
		DemoPlayer->Stop();
	}

	void Player::Spawn()
	{
		PMove->Initialize();
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
