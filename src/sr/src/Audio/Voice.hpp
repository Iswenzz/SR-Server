#pragma once
#include "Streamable.hpp"

extern cvar_t *voice_localEcho;
extern cvar_t *voice_global;
extern cvar_t *voice_deadChat;

// Clients whose userinfo carries sr_voice put these two bytes ahead of every voice packet, and get them
// back on everything sent to them: the codec, then the gain to play it at, where 64 is unity. MSG_WriteByte
// carries the size, so a packet tops out at 255 bytes.
#define VOICE_CODEC_SPEEX 0
#define VOICE_CODEC_OPUS 1
#define VOICE_HEADER_SIZE 2
#define VOICE_UNITY_GAIN 64
#define VOICE_MAX_PACKET 255

namespace SR
{
	// A talker's packet without its header.
	struct VoiceFrame
	{
		int Codec = VOICE_CODEC_SPEEX;
		const char *Data = nullptr;
		int Size = 0;
	};

	class Voice
	{
	public:
		static inline std::map<std::string, Ref<Streamable>> Audios;
		static inline Ref<Streamable> Radio = nullptr;
		static inline std::chrono::steady_clock::time_point RadioStart;
		static inline cvar_t *Relay = nullptr;

		static void Initialize();
		static void Shutdown();
		static void Frame();

		static void Stream();
		static void BroadcastVoice(gentity_t *talker, VoicePacket_t *packet);
		static bool IsRelayClient(int clientNum);

	private:
		static bool ReadFrame(int talker, VoicePacket_t *packet, VoiceFrame &frame);
		static VoicePacket_t RelayPacket(const VoiceFrame &frame, float gain);
		static VoicePacket_t RawPacket(const VoiceFrame &frame);
		static std::vector<std::vector<short>> Decode(int talker, const VoiceFrame &frame);
		static float ProximityGain(gentity_t *talker, gentity_t *entity);
	};
}
