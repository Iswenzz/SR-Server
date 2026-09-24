#pragma once
#include "Base.hpp"

#include "Speex.hpp"

namespace SR
{
	class Audio
	{
	public:
		static std::vector<short> Amplify(std::vector<short> &data, float multiplier);
		static std::vector<short> StereoToMono(short *data, int samples);
		static std::vector<short> MonoToStereo(const std::vector<short> &mono);
		static std::vector<short> Resample(short *buffer, int samples, int channels, int rate, int newRate);
	};
}
