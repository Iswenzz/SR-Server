#include "Speex.hpp"

namespace SR
{
	SpeexEncoder::SpeexEncoder(int complexity)
	{
		spx_int32_t rate = SPEEX_RATE;
		spx_int32_t quality = SPEEX_QUALITY;

		speex_bits_init(&Bits);
		State = speex_encoder_init(&speex_nb_mode);
		speex_encoder_ctl(State, SPEEX_SET_SAMPLING_RATE, &rate);
		speex_encoder_ctl(State, SPEEX_SET_QUALITY, &quality);
		speex_encoder_ctl(State, SPEEX_SET_COMPLEXITY, &complexity);
	}

	SpeexEncoder::~SpeexEncoder()
	{
		speex_bits_destroy(&Bits);
		if (State)
			speex_encoder_destroy(State);
	}

	VoicePacket_t SpeexEncoder::Encode(std::vector<short> &frame)
	{
		VoicePacket_t encodedData{};
		frame.resize(SPEEX_FRAME_SIZE);

		speex_bits_reset(&Bits);
		speex_encode_int(State, frame.data(), &Bits);
		encodedData.dataSize = speex_bits_write(&Bits, encodedData.data, sizeof(encodedData.data));
		return encodedData;
	}

	// Narrowband reads the leading layer of wideband and ultra-wideband packets too.
	SpeexDecoder::SpeexDecoder()
	{
		spx_int32_t perceptualEnhancement = 0;
		spx_int32_t rate = SPEEX_RATE;

		speex_bits_init(&Bits);
		State = speex_decoder_init(&speex_nb_mode);
		speex_decoder_ctl(State, SPEEX_SET_ENH, &perceptualEnhancement);
		speex_decoder_ctl(State, SPEEX_SET_SAMPLING_RATE, &rate);
	}

	SpeexDecoder::~SpeexDecoder()
	{
		speex_bits_destroy(&Bits);
		if (State)
			speex_decoder_destroy(State);
	}

	std::vector<short> SpeexDecoder::Decode(const char *data, int size)
	{
		std::vector<short> decodedData(SPEEX_FRAME_SIZE);

		speex_bits_read_from(&Bits, data, size);
		speex_decode_int(State, &Bits, decodedData.data());
		return decodedData;
	}

	void Speex::Initialize()
	{
		for (auto &decoder : Decoders)
			decoder = std::make_unique<SpeexDecoder>();
	}

	void Speex::Shutdown()
	{
		for (auto &decoder : Decoders)
			decoder.reset();
		for (auto &steps : ProximityEncoders)
			for (auto &encoder : steps)
				encoder.reset();
	}

	std::vector<short> Speex::Decode(int talker, const char *data, int size)
	{
		if (talker < 0 || talker >= MAX_CLIENTS || !Decoders[talker])
			return std::vector<short>(SPEEX_FRAME_SIZE);

		return Decoders[talker]->Decode(data, size);
	}

	VoicePacket_t Speex::EncodeProximity(int talker, int step, std::vector<short> &frame)
	{
		if (talker < 0 || talker >= MAX_CLIENTS || step < 0 || step > SPEEX_PROXIMITY_STEPS)
			return {};

		auto &encoder = ProximityEncoders[talker][step];
		if (!encoder)
			encoder = std::make_unique<SpeexEncoder>(SPEEX_PROXIMITY_COMPLEXITY);
		return encoder->Encode(frame);
	}
}
