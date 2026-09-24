#include "Opus.hpp"

#include "Speex.hpp"
#include "Voice.hpp"

#include <opus/opus.h>
#include <samplerate.h>

namespace SR
{
	OpusStreamEncoder::OpusStreamEncoder(int bitrate)
	{
		int error = 0;

		State = opus_encoder_create(VOICE_OPUS_RATE, VOICE_OPUS_RADIO_CHANNELS, OPUS_APPLICATION_AUDIO, &error);
		if (!State)
			return;

		opus_encoder_ctl(State, OPUS_SET_BITRATE(bitrate));
		opus_encoder_ctl(State, OPUS_SET_COMPLEXITY(10));
	}

	OpusStreamEncoder::~OpusStreamEncoder()
	{
		if (State)
			opus_encoder_destroy(State);
	}

	VoicePacket_t OpusStreamEncoder::Encode(std::vector<short> &frame)
	{
		VoicePacket_t encodedData{};
		frame.resize(VOICE_OPUS_FRAME_SIZE * VOICE_OPUS_RADIO_CHANNELS);

		if (!State)
			return encodedData;

		const int bytes = opus_encode(State, frame.data(), VOICE_OPUS_FRAME_SIZE,
			reinterpret_cast<unsigned char *>(encodedData.data + VOICE_HEADER_SIZE),
			VOICE_MAX_PACKET - VOICE_HEADER_SIZE);
		if (bytes <= 0)
			return encodedData;

		encodedData.data[0] = VOICE_CODEC_OPUS;
		encodedData.data[1] = static_cast<char>(VOICE_UNITY_GAIN);
		encodedData.dataSize = bytes + VOICE_HEADER_SIZE;
		return encodedData;
	}

	OpusTranscoder::OpusTranscoder()
	{
		int error = 0;

		State = opus_decoder_create(VOICE_OPUS_RATE, 1, &error);
		Resampler = src_new(SRC_SINC_FASTEST, 1, &error);
	}

	OpusTranscoder::~OpusTranscoder()
	{
		if (Resampler)
			src_delete(Resampler);
		if (State)
			opus_decoder_destroy(State);
	}

	std::vector<std::vector<short>> OpusTranscoder::Decode(const char *data, int size)
	{
		std::vector<std::vector<short>> frames;
		if (!State || !Resampler)
			return frames;

		Decoded.resize(VOICE_OPUS_MAX_FRAME_SIZE);
		const int samples = opus_decode_float(State, reinterpret_cast<const unsigned char *>(data), size,
			Decoded.data(), VOICE_OPUS_MAX_FRAME_SIZE, 0);
		if (samples <= 0)
			return frames;

		const size_t start = Pending.size();
		Pending.resize(start + samples);

		SRC_DATA resample = {};
		resample.data_in = Decoded.data();
		resample.input_frames = samples;
		resample.data_out = Pending.data() + start;
		resample.output_frames = samples;
		resample.src_ratio = static_cast<double>(SPEEX_RATE) / VOICE_OPUS_RATE;

		const bool resampled = src_process(Resampler, &resample) == 0;
		Pending.resize(start + (resampled ? resample.output_frames_gen : 0));

		while (Pending.size() >= SPEEX_FRAME_SIZE)
		{
			std::vector<short> frame(SPEEX_FRAME_SIZE);
			src_float_to_short_array(Pending.data(), frame.data(), SPEEX_FRAME_SIZE);
			Pending.erase(Pending.begin(), Pending.begin() + SPEEX_FRAME_SIZE);
			frames.push_back(std::move(frame));
		}
		return frames;
	}

	void Opus::Shutdown()
	{
		for (auto &transcoder : Transcoders)
			transcoder.reset();
	}

	std::vector<std::vector<short>> Opus::Decode(int talker, const char *data, int size)
	{
		if (talker < 0 || talker >= MAX_CLIENTS)
			return {};

		auto &transcoder = Transcoders[talker];
		if (!transcoder)
			transcoder = std::make_unique<OpusTranscoder>();
		return transcoder->Decode(data, size);
	}
}
