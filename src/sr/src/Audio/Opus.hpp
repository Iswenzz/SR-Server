#pragma once
#include "Base.hpp"

struct OpusDecoder;
struct OpusEncoder;
struct SRC_STATE_tag;

#define VOICE_OPUS_RATE 48000
#define VOICE_OPUS_FRAME_SIZE 960
#define VOICE_OPUS_MAX_FRAME_SIZE 5760
#define VOICE_OPUS_RADIO_BITRATE 96000
#define VOICE_OPUS_RADIO_CHANNELS 2

namespace SR
{
	// Radio files only, in stereo. Talkers' Opus is relayed exactly as it arrived. Each Opus packet says
	// how many channels it holds, so it needs no codec of its own.
	class OpusStreamEncoder
	{
	public:
		OpusStreamEncoder(int bitrate);
		~OpusStreamEncoder();

		OpusStreamEncoder(const OpusStreamEncoder &) = delete;
		OpusStreamEncoder &operator=(const OpusStreamEncoder &) = delete;

		VoicePacket_t Encode(std::vector<short> &frame);

	private:
		OpusEncoder *State = nullptr;
	};

	// One IW3SR talker, decoded down to narrowband frames for the stock clients that cannot read Opus.
	// An Opus frame is a little longer than a Speex one, so each packet yields one frame or, now and
	// then, two.
	class OpusTranscoder
	{
	public:
		OpusTranscoder();
		~OpusTranscoder();

		OpusTranscoder(const OpusTranscoder &) = delete;
		OpusTranscoder &operator=(const OpusTranscoder &) = delete;

		std::vector<std::vector<short>> Decode(const char *data, int size);

	private:
		OpusDecoder *State = nullptr;
		SRC_STATE_tag *Resampler = nullptr;

		std::vector<float> Decoded;
		std::vector<float> Pending;
	};

	// The talkers' transcoders, main thread only.
	class Opus
	{
	public:
		static void Shutdown();

		static std::vector<std::vector<short>> Decode(int talker, const char *data, int size);

	private:
		static inline std::array<std::unique_ptr<OpusTranscoder>, MAX_CLIENTS> Transcoders;
	};
}
