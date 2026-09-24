#include "Voice.hpp"

#include "Audio.hpp"
#include "Opus.hpp"

#include "Game/Player/Player.hpp"

#include <cmath>
#include <optional>

#define VOICE_AMPLIFY 2
#define PROXIMITY_DISTANCE 1500

// Just over the 366 ms the client buffers before it starts playing.
#define RADIO_LEAD_PACKETS 20
// A hitch is caught up over several frames rather than overflowing the client's 40-packet queue.
#define RADIO_MAX_BURST 20

namespace SR
{
	// Packets a stream owes at this point on its playback clock, beyond those already sent.
	static int DuePackets(int64_t elapsed, int rate, int frameSize, int position, size_t total)
	{
		const int64_t due = RADIO_LEAD_PACKETS + elapsed * rate / (frameSize * 1000000LL);
		const int64_t left = static_cast<int64_t>(total) - position;
		return static_cast<int>(std::clamp<int64_t>(std::min(due - position, left), 0, RADIO_MAX_BURST));
	}

	static int ProximityStep(float gain)
	{
		const long step = std::lround(gain / VOICE_AMPLIFY * SPEEX_PROXIMITY_STEPS);
		return static_cast<int>(std::clamp<long>(step, 0, SPEEX_PROXIMITY_STEPS));
	}

	void Voice::Initialize()
	{
		// In the systeminfo, where IW3SR clients look for it on every gamestate.
		Relay = Cvar_RegisterBool("sr_voiceRelay", qtrue, CVAR_SYSTEMINFO | CVAR_ROM,
			"Relays Opus voice between the clients that speak it");
		Speex::Initialize();
	}

	void Voice::Shutdown()
	{
		Speex::Shutdown();
		Opus::Shutdown();
	}

	void Voice::Frame()
	{
		Stream();
	}

	// Paced on the playback clock. Running ahead of it makes the client's jitter buffer speed playback up
	// by 1% and drop what no longer fits, both of which are plain to hear on music. The two encodings
	// have their own frame lengths, so each keeps its own count.
	void Voice::Stream()
	{
		if (!Radio || !Radio->IsLoaded || Radio->IsStreamEnd())
			return;

		int i;
		gentity_t *entity;
		client_t *cl;

		const auto now = std::chrono::steady_clock::now();
		if (Radio->IsStreamStart())
			RadioStart = now;

		const int64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - RadioStart).count();
		const int speexCount = DuePackets(elapsed, SPEEX_RATE, SPEEX_FRAME_SIZE, Radio->StreamPosition,
			Radio->StreamPackets.size());
		const int opusCount = DuePackets(elapsed, VOICE_OPUS_RATE, VOICE_OPUS_FRAME_SIZE, Radio->OpusPosition,
			Radio->OpusPackets.size());

		for (i = 0, cl = svs.clients; i < level.maxclients; i++, cl++)
		{
			entity = &level.gentities[i];

			if (cl && cl->state == CS_ACTIVE && entity->client
				&& entity->client->sess.sessionState != SESS_STATE_INTERMISSION)
			{
				const auto &player = Player::Get(i);
				if (!player || !player->RadioEnabled)
					continue;

				const bool relay = IsRelayClient(i);
				const auto &packets = relay ? Radio->OpusPackets : Radio->StreamPackets;
				const int position = relay ? Radio->OpusPosition : Radio->StreamPosition;
				const int count = relay ? opusCount : speexCount;

				for (int p = 0; p < count; p++)
				{
					VoicePacket_t packet = packets[position + p];
					SV_QueueVoicePacket(i, i, &packet);
				}
			}
		}
		Radio->StreamPosition += speexCount;
		Radio->OpusPosition += opusCount;

		if (Radio->IsStreamEnd())
			Radio = nullptr;
	}

	// IW3SR advertises sr_voice in its userinfo. Everything else is a stock client, reading bare Speex.
	bool Voice::IsRelayClient(int clientNum)
	{
		if (clientNum < 0 || clientNum >= MAX_CLIENTS)
			return false;

		return atoi(Info_ValueForKey(svs.clients[clientNum].userinfo, "sr_voice")) >= 1;
	}

	bool Voice::ReadFrame(int talker, VoicePacket_t *packet, VoiceFrame &frame)
	{
		frame.Codec = VOICE_CODEC_SPEEX;
		frame.Data = packet->data;
		frame.Size = packet->dataSize;

		if (!IsRelayClient(talker))
			return frame.Size > 0;

		const int codec = static_cast<unsigned char>(packet->data[0]);
		if (packet->dataSize <= VOICE_HEADER_SIZE || codec > VOICE_CODEC_OPUS)
			return false;

		frame.Codec = codec;
		frame.Data += VOICE_HEADER_SIZE;
		frame.Size -= VOICE_HEADER_SIZE;
		return true;
	}

	VoicePacket_t Voice::RelayPacket(const VoiceFrame &frame, float gain)
	{
		VoicePacket_t packet{};
		if (frame.Size > VOICE_MAX_PACKET - VOICE_HEADER_SIZE)
			return packet;

		packet.data[0] = static_cast<char>(frame.Codec);
		packet.data[1] = static_cast<char>(std::clamp<long>(std::lround(gain * VOICE_UNITY_GAIN), 0, 255));
		std::memcpy(packet.data + VOICE_HEADER_SIZE, frame.Data, frame.Size);
		packet.dataSize = frame.Size + VOICE_HEADER_SIZE;
		return packet;
	}

	VoicePacket_t Voice::RawPacket(const VoiceFrame &frame)
	{
		VoicePacket_t packet{};
		std::memcpy(packet.data, frame.Data, frame.Size);
		packet.dataSize = frame.Size;
		return packet;
	}

	// Narrowband frames for stock clients: one per Speex packet, one or two per Opus packet.
	std::vector<std::vector<short>> Voice::Decode(int talker, const VoiceFrame &frame)
	{
		if (frame.Codec == VOICE_CODEC_OPUS)
			return Opus::Decode(talker, frame.Data, frame.Size);

		return { Speex::Decode(talker, frame.Data, frame.Size) };
	}

	float Voice::ProximityGain(gentity_t *talker, gentity_t *entity)
	{
		float distance = fabs(VectorDistance(talker->client->ps.origin, entity->client->ps.origin));

		if (distance > PROXIMITY_DISTANCE)
			distance = PROXIMITY_DISTANCE;
		distance = 1 - (distance / PROXIMITY_DISTANCE);

		return distance * VOICE_AMPLIFY;
	}

	// IW3SR listeners get the talker's own packet with the gain in its header, so neither codec is ever
	// re-encoded for them. Stock listeners need Speex, encoded once per talker and gain step whenever
	// the talker sent Opus or the listener hears by proximity.
	void Voice::BroadcastVoice(gentity_t *talker, VoicePacket_t *packet)
	{
		int i;
		gentity_t *entity;
		client_t *cl;

		const int talkerNum = talker->s.number;
		talker->client->lastVoiceTime = level.time;

		VoiceFrame frame;
		if (!ReadFrame(talkerNum, packet, frame))
			return;

		// Speex is decoded whether or not anyone needs it, since a decoder that skips frames drifts from
		// the talker's encoder. Opus decodes a gap as loss and recovers, so it waits until it is needed.
		std::optional<std::vector<std::vector<short>>> decoded;
		if (frame.Codec == VOICE_CODEC_SPEEX)
			decoded = Decode(talkerNum, frame);

		std::array<std::optional<std::vector<VoicePacket_t>>, SPEEX_PROXIMITY_STEPS + 1> speexPackets;

		for (i = 0, cl = svs.clients; i < level.maxclients; i++, cl++)
		{
			entity = &level.gentities[i];

			if (cl && cl->state == CS_ACTIVE && entity->client
				&& entity->client->sess.sessionState != SESS_STATE_INTERMISSION)
			{
				if (!voice_localEcho->boolean && entity == talker)
					continue;
				if (!voice_global->boolean && !OnSameTeam(entity, talker))
					continue;

				if (SV_ClientHasClientMuted(i, talkerNum) || !SV_ClientWantsVoiceData(i))
					continue;

				const auto &player = Player::Get(i);
				const bool proximity =
					entity->client->sess.sessionState == SESS_STATE_PLAYING && player && player->ProximityEnabled;
				const float gain = proximity ? ProximityGain(talker, entity) : 1.0f;

				if (IsRelayClient(i))
				{
					VoicePacket_t relayed = RelayPacket(frame, gain);
					if (relayed.dataSize > 0)
						SV_QueueVoicePacket(talkerNum, i, &relayed);
					continue;
				}
				if (frame.Codec == VOICE_CODEC_SPEEX && !proximity)
				{
					VoicePacket_t raw = RawPacket(frame);
					SV_QueueVoicePacket(talkerNum, i, &raw);
					continue;
				}
				const int step = ProximityStep(gain);
				auto &packets = speexPackets[step];

				if (!packets)
				{
					if (!decoded)
						decoded = Decode(talkerNum, frame);

					const float stepGain = static_cast<float>(step) / SPEEX_PROXIMITY_STEPS * VOICE_AMPLIFY;
					packets.emplace();

					for (auto &pcm : *decoded)
					{
						std::vector<short> amplified = Audio::Amplify(pcm, stepGain);
						VoicePacket_t encoded = Speex::EncodeProximity(talkerNum, step, amplified);
						if (encoded.dataSize > 0)
							packets->push_back(encoded);
					}
				}
				for (auto &encoded : *packets)
					SV_QueueVoicePacket(talkerNum, i, &encoded);
			}
		}
	}
}
