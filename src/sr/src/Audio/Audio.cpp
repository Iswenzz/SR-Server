#include "Audio.hpp"

#include <samplerate.h>

namespace SR
{
	std::vector<short> Audio::Amplify(std::vector<short> &data, float multiplier)
	{
		std::vector<short> dataAmplified(data.size());
		for (int i = 0; i < data.size(); i++)
		{
			int amplified = data[i] * multiplier;

			if (amplified > SHRT_MAX)
				amplified = SHRT_MAX;
			else if (amplified < SHRT_MIN)
				amplified = SHRT_MIN;

			dataAmplified[i] = static_cast<short>(amplified);
		}
		return dataAmplified;
	}

	std::vector<short> Audio::StereoToMono(short *data, int samples)
	{
		std::vector<short> mono(samples / 2);

		for (int i = 0; i < mono.size(); i++)
		{
			short left = data[i * 2];
			short right = data[i * 2 + 1];
			mono[i] = (left + right) / 2;
		}
		return mono;
	}

	std::vector<short> Audio::MonoToStereo(const std::vector<short> &mono)
	{
		std::vector<short> stereo(mono.size() * 2);

		for (size_t i = 0; i < mono.size(); i++)
		{
			stereo[i * 2] = mono[i];
			stereo[i * 2 + 1] = mono[i];
		}
		return stereo;
	}

	std::vector<short> Audio::Resample(short *buffer, int samples, int channels, int rate, int newRate)
	{
		double ratio = static_cast<double>(newRate) / static_cast<double>(rate);
		int newSamples = ceil(samples * ratio);
		// libsndfile goes crazy with odd size in case of saving
		if (newSamples % 2 != 0)
			newSamples++;

		std::vector<float> decodedData(samples);
		std::vector<float> sampledData(newSamples);

		src_short_to_float_array(buffer, decodedData.data(), decodedData.size());

		SRC_DATA data = { 0 };
		data.end_of_input = 1;
		data.data_in = decodedData.data();
		data.data_out = sampledData.data();
		data.input_frames = samples / channels;
		data.output_frames = newSamples / channels;
		data.src_ratio = ratio;

		// Only radio files come through here, on a worker, so the slow sinc filter costs nothing in game.
		// Linear interpolation aliases the treble into audible grit on music.
		if (int error = src_simple(&data, SRC_SINC_BEST_QUALITY, channels); error)
		{
			Log::WriteLine("^1[Voice] Downsample error {}", src_strerror(error));
			return {};
		}
		std::vector<short> resampledData(data.output_frames_gen * channels);
		src_float_to_short_array(sampledData.data(), resampledData.data(), resampledData.size());
		return resampledData;
	}
}
