#include "Streamable.hpp"

#include "Audio.hpp"
#include "Opus.hpp"
#include "Speex.hpp"

// About -3 dB.
#define RADIO_HEADROOM 0.7f

namespace SR
{
	template <typename Encoder>
	static std::vector<VoicePacket_t> EncodeFrames(const std::vector<short>& buffer, size_t frameSize, Encoder& encoder)
	{
		std::vector<VoicePacket_t> packets;

		for (size_t position = 0; position < buffer.size(); position += frameSize)
		{
			const size_t end = std::min(buffer.size(), position + frameSize);
			std::vector<short> frame(buffer.begin() + position, buffer.begin() + end);

			// An empty packet reads as malformed on the client, which drops the rest of the message.
			VoicePacket_t packet = encoder.Encode(frame);
			if (packet.dataSize > 0)
				packets.push_back(packet);
		}
		return packets;
	}

	Streamable::~Streamable()
	{
		if (Input.is_open())
			Input.close();
		if (Output.is_open())
			Output.close();
	}

	// Mono narrowband for stock clients, stereo at 48 kHz for IW3SR clients, whose voice buffers are stereo.
	// Mastered music sits at full scale and both codecs overshoot it on decode, so it goes in with some
	// headroom.
	void Streamable::Load(const short* pcm, size_t samples, int channels, int rate)
	{
		std::vector<short> interleaved(pcm, pcm + samples);
		interleaved = Audio::Amplify(interleaved, RADIO_HEADROOM);

		std::vector<short> mono = channels == 2 ? Audio::StereoToMono(interleaved.data(), interleaved.size()) : interleaved;
		std::vector<short> stereo = channels == 2 ? std::move(interleaved) : Audio::MonoToStereo(mono);

		Buffer = Audio::Resample(mono.data(), mono.size(), SPEEX_CHANNELS, rate, SPEEX_RATE);
		std::vector<short> wide =
			Audio::Resample(stereo.data(), stereo.size(), VOICE_OPUS_RADIO_CHANNELS, rate, VOICE_OPUS_RATE);

		SpeexEncoder speex(SPEEX_RADIO_COMPLEXITY);
		OpusStreamEncoder opus(VOICE_OPUS_RADIO_BITRATE);

		StreamPackets = EncodeFrames(Buffer, SPEEX_FRAME_SIZE, speex);
		OpusPackets = EncodeFrames(wide, VOICE_OPUS_FRAME_SIZE * VOICE_OPUS_RADIO_CHANNELS, opus);
	}

	void Streamable::Rewind()
	{
		StreamPosition = 0;
		OpusPosition = 0;
	}

	bool Streamable::IsStreamStart()
	{
		return !StreamPosition && !OpusPosition;
	}

	bool Streamable::IsStreamEnd()
	{
		return StreamPosition >= StreamPackets.size() && OpusPosition >= OpusPackets.size();
	}
}
