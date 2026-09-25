#pragma once
#include "PMove.hpp"

#include "Game/Demo/DemoPlayer.hpp"

#define PLAYER_FPS_STACK 20
#define PMF_PRONING 0x1
#define PMF_CROUCHING 0x2

// cl->gentity->s.number is the client number.
// cl->gentity->client->ps.clientNumber is the spectator client or client if not spectating.
namespace SR
{
	class Player
	{
	public:
		static inline std::array<Ref<Player>, MAX_CLIENTS> List{};
		client_t *cl = nullptr;
		playerState_t *ps = nullptr;
		Scope<playerState_t> SaveState;
		Scope<class PMove> PMove;
		Scope<class DemoPlayer> DemoPlayer;
		int SurfaceFlags = 0;

		int FPS = 0;
		int FrameStackIndex = 0;
		std::array<int, PLAYER_FPS_STACK> FrameTimes{};

		bool Voice = false;
		int VoiceCount = 0;
		int VoiceTime = 0;
		int LastVoiceTime = 0;

		Player(client_t *cl);
		~Player() = default;

		void Initialize();
		void Disconnect();
		void Spawn();

		void CalculateFrame(int time);
		void CalculateFPS();

		void Packet(msg_t *msg);
		void Frame();
		void VoiceChat();

		clientSnapshot_t *GetFrame();
		static const Ref<Player> &Get(int num);
		static void Add(client_t *cl);
	};
}
