#pragma once
#include "Base.hpp"

namespace SR
{
	class Utils
	{
	public:
		static std::vector<std::string> SplitString(const std::string& source, char delimiter);
		static bool ClientVersion(int clientNum, int major, int minor, int patch);
	};
}
