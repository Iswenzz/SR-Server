#pragma once
#include "PMove.hpp"

#include "Game/Demo/DemoPlayer.hpp"

#include <optional>

#define PLAYER_FPS_STACK 20
#define PLAYER_SAVE_STATES 32
#define PLAYER_REWIND_FRAMES 256 // 12.8 s at sv_fps 20
#define PLAYER_TELEPORT_BIT 0x2	 // CoD4's; EF_TELEPORT_BIT in g_shared.h is Quake 3's 0x4
#define PMF_PRONING 0x1
#define PMF_CROUCHING 0x2

// cl->gentity->s.number is the client number.
// cl->gentity->client->ps.clientNumber is the spectator client or client if not spectating.
namespace SR
{
	// The playerState fields a save, a load or a rewind moves a player with.
	struct PlayerMoveState
	{
		int pm_type;
		int bobCycle;
		int pm_flags;
		int weapFlags;
		int otherFlags;
		int pm_time;
		vec3_t origin;
		vec3_t velocity;
		vec2_t oldVelocity;
		float leanf;
		int groundEntityNum;
		vec2_t vLadderVec;
		int jumpTime;
		float jumpOriginZ;
		int damageTimer;
		int damageDuration;
		int movementDir;
		int eFlags;
		int eventSequence;
		unsigned int weapon;
		int adsDelayTime;
		int damageEvent;
		int damageYaw;
		int damagePitch;
		int damageCount;
		sprintState_t sprintState;
		float holdBreathScale;
		int holdBreathTimer;
		float moveSpeedScaleMultiplier;
		mantleState_t mantleState;
		int entityEventSequence;
		float aimSpreadScale;
	};

	struct PlayerRewindFrame
	{
		int time;
		vec3_t viewangles;
		PlayerMoveState state;
	};

	class Player
	{
	public:
		static inline std::array<Ref<Player>, MAX_CLIENTS> List{};
		client_t *cl = nullptr;
		playerState_t *ps = nullptr;
		std::array<std::optional<PlayerMoveState>, PLAYER_SAVE_STATES> SaveStates{};
		std::array<PlayerRewindFrame, PLAYER_REWIND_FRAMES> RewindFrames{};
		int RewindIndex = 0;
		int RewindCount = 0;
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

		PlayerMoveState GetMoveState() const;
		void SetMoveState(const PlayerMoveState &state);
		void RecordRewind();
		const PlayerRewindFrame *FindRewind(int time) const;
		void Rewind(const PlayerRewindFrame &frame);

		clientSnapshot_t *GetFrame();
		static const Ref<Player> &Get(int num);
		static void Add(client_t *cl);
	};
}
