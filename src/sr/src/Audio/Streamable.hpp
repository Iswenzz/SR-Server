#pragma once
#include "Base.hpp"

namespace SR
{
	// Encoded twice when it loads: narrowband Speex for stock clients, stereo Opus for IW3SR clients.
	class Streamable
	{
	public:
		std::string FilePath;
		int FileSize = 0;
		std::ifstream Input;
		std::ofstream Output;
		std::vector<short> Buffer;
		int Samples = 0;
		int Rate = 0;
		std::vector<VoicePacket_t> StreamPackets;
		std::vector<VoicePacket_t> OpusPackets;
		int StreamPosition = 0;
		int OpusPosition = 0;
		std::atomic<bool> IsLoaded = false;

		Streamable() = default;
		virtual ~Streamable();

		virtual void Open(const Ref<AsyncTask>& task) = 0;
		virtual void Save(const std::string& path) = 0;

		void Load(const short* pcm, size_t samples, int channels, int rate);
		void Rewind();
		bool IsStreamStart();
		bool IsStreamEnd();
	};
}
