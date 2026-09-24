#pragma once
#include "Base.hpp"

#include <speex/speex.h>

// Narrowband on retail's 8192 Hz clock: everything encoded here is for stock clients, which read no
// more than that. IW3SR clients get Opus, or decode a talker's ultra-wideband Speex themselves.
#define SPEEX_RATE 8192
#define SPEEX_FRAME_SIZE 160
#define SPEEX_QUALITY 10
#define SPEEX_CHANNELS 1
#define SPEEX_PCM 1
#define SPEEX_PCM_CHUNK 16
#define SPEEX_BITS_PER_SAMPLE 16

// Proximity is encoded once per talker and gain step rather than once per listener.
#define SPEEX_PROXIMITY_STEPS 32
#define SPEEX_PROXIMITY_COMPLEXITY 5
#define SPEEX_RADIO_COMPLEXITY 10

namespace SR
{
	// Speex carries state from one frame to the next, so each instance must only ever see one stream.
	class SpeexEncoder
	{
	public:
		SpeexEncoder(int complexity);
		~SpeexEncoder();

		SpeexEncoder(const SpeexEncoder &) = delete;
		SpeexEncoder &operator=(const SpeexEncoder &) = delete;

		VoicePacket_t Encode(std::vector<short> &frame);

	private:
		void *State = nullptr;
		SpeexBits Bits = {};
	};

	class SpeexDecoder
	{
	public:
		SpeexDecoder();
		~SpeexDecoder();

		SpeexDecoder(const SpeexDecoder &) = delete;
		SpeexDecoder &operator=(const SpeexDecoder &) = delete;

		std::vector<short> Decode(const char *data, int size);

	private:
		void *State = nullptr;
		SpeexBits Bits = {};
	};

	// The talkers' streams, main thread only. Radio files encode with their own encoder on a worker.
	class Speex
	{
	public:
		static void Initialize();
		static void Shutdown();

		static std::vector<short> Decode(int talker, const char *data, int size);
		static VoicePacket_t EncodeProximity(int talker, int step, std::vector<short> &frame);

	private:
		static inline std::array<std::unique_ptr<SpeexDecoder>, MAX_CLIENTS> Decoders;
		static inline std::array<std::array<std::unique_ptr<SpeexEncoder>, SPEEX_PROXIMITY_STEPS + 1>, MAX_CLIENTS>
			ProximityEncoders;
	};
}
