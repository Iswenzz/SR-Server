#include "WAV.hpp"

#include "Audio.hpp"

namespace SR
{
	WAV::WAV(const std::string &filepath)
	{
		FilePath = filepath;

		auto task = Async::Create(this);
		Async::Submit([this, task] { Open(task); });
	}

	void WAV::Open(const Ref<AsyncTask> &task)
	{
		if (!std::filesystem::exists(FilePath))
		{
			task->Status = AsyncStatus::Failure;
			return;
		}
		IsLoaded = false;
		Input.open(FilePath, std::ios_base::binary);

		Log::WriteLine("^5[WAV] Opening {}", FilePath.c_str());

		// Only canonical 16-bit PCM files, with the data chunk right after "fmt ".
		WavHeader header{};
		Input.read(reinterpret_cast<char *>(&header), sizeof(WavHeader));
		if (!Input || std::memcmp(header.riff, "RIFF", 4) || std::memcmp(header.wave, "WAVE", 4)
			|| std::memcmp(header.subchunk2ID, "data", 4) || header.audioFormat != SPEEX_PCM
			|| header.bitsPerSample != SPEEX_BITS_PER_SAMPLE || header.numChannels < 1 || header.numChannels > 2)
		{
			Log::WriteLine("^1[WAV] Unsupported format {}", FilePath.c_str());
			task->Status = AsyncStatus::Failure;
			return;
		}
		FileSize = header.chunkSize;
		Samples = header.subchunk2Size;
		Rate = header.sampleRate;

		std::vector<short> pcm(Samples / sizeof(short));
		Input.read(reinterpret_cast<char *>(pcm.data()), pcm.size() * sizeof(short));
		pcm.resize(Input.gcount() / sizeof(short));

		Load(pcm.data(), pcm.size(), header.numChannels, Rate);
		IsLoaded = true;
		task->Status = AsyncStatus::Successful;
	}

	void WAV::Save(const std::string &path)
	{
		if (!Buffer.size())
			return;

		Output.open(path, std::ios_base::binary);
		WriteHeader(Output, 1, SPEEX_RATE, Buffer.size() * sizeof(short));
		Output.write(reinterpret_cast<char *>(Buffer.data()), Buffer.size() * sizeof(short));
	}

	void WAV::WriteHeader(std::ofstream &file, int channels, int rate, int samples)
	{
		file.seekp(0, file.end);
		int fileSize = file.tellp();
		file.seekp(0, file.beg);

		WavHeader wav;
		std::memcpy(&wav.riff, "RIFF", 4);
		wav.chunkSize = fileSize + sizeof(WavHeader) - 8;
		std::memcpy(&wav.wave, "WAVE", 4);
		std::memcpy(&wav.fmt, "fmt ", 4);
		wav.subchunk1Size = SPEEX_PCM_CHUNK;
		wav.audioFormat = SPEEX_PCM;
		wav.numChannels = channels;
		wav.sampleRate = rate;
		wav.bitsPerSample = SPEEX_BITS_PER_SAMPLE;
		wav.byteRate = wav.sampleRate * wav.numChannels * wav.bitsPerSample / 8;
		wav.blockAlign = wav.numChannels * wav.bitsPerSample / 8;
		std::memcpy(&wav.subchunk2ID, "data", 4);
		wav.subchunk2Size = samples;

		file.write(reinterpret_cast<char *>(&wav), sizeof(wav));
	}
}
