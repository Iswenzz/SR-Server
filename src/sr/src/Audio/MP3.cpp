#include "MP3.hpp"

#include "Audio.hpp"

#define MINIMP3_ONLY_MP3
#define MINIMP3_NO_STDIO
#define MINIMP3_IMPLEMENTATION

#include <minimp3/minimp3_ex.h>

namespace SR
{
	MP3::MP3(const std::string& filepath)
	{
		FilePath = filepath;

		auto task = Async::Create(this);
		Async::Submit([this, task] { Open(task); });
	}

	void MP3::Open(const Ref<AsyncTask>& task)
	{
		if (!std::filesystem::exists(FilePath))
		{
			task->Status = AsyncStatus::Failure;
			return;
		}
		IsLoaded = false;
		mp3dec_t mp3d;
		mp3dec_init(&mp3d);

		Input.open(FilePath, std::ios_base::binary);
		Log::WriteLine("^5[MP3] Opening {}", FilePath.c_str());

		Input.seekg(0, Input.end);
		FileSize = Input.tellg();
		Input.seekg(0, Input.beg);

		std::vector<unsigned char> buffer(FileSize);
		Input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

		mp3dec_file_info_t fileInfo;
		if (mp3dec_load_buf(&mp3d, buffer.data(), buffer.size(), &fileInfo, nullptr, nullptr))
		{
			Log::WriteLine("^1[MP3] Error opening {}", FilePath.c_str());
			task->Status = AsyncStatus::Failure;
			return;
		}
		Rate = fileInfo.hz;
		Samples = fileInfo.samples;

		Load(fileInfo.buffer, fileInfo.samples, fileInfo.channels, Rate);
		free(fileInfo.buffer);
		IsLoaded = true;
		task->Status = AsyncStatus::Successful;
	}

	void MP3::Save(const std::string& path) { }
}
