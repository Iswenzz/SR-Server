#include "Utils.hpp"

namespace SR
{
	// Whether the client runs IW3SR at this version or later, as the sr_version in its userinfo says. What
	// an IW3SR client can do is told by its version: stock clients have none.
	bool Utils::ClientVersion(int clientNum, int major, int minor, int patch)
	{
		if (clientNum < 0 || clientNum >= MAX_CLIENTS)
			return false;

		int version[3] = {};
		const char *value = Info_ValueForKey(svs.clients[clientNum].userinfo, "sr_version");
		if (!value || sscanf(value, "%d.%d.%d", &version[0], &version[1], &version[2]) < 1)
			return false;

		const int wanted[3] = { major, minor, patch };
		for (int i = 0; i < 3; i++)
		{
			if (version[i] != wanted[i])
				return version[i] > wanted[i];
		}
		return true;
	}

	std::vector<std::string> Utils::SplitString(const std::string& source, char delimiter)
	{
		std::vector<std::string> results;

		size_t prev = 0;
		size_t next = 0;

		while ((next = source.find_first_of(delimiter, prev)) != std::string::npos)
		{
			if (next - prev != 0)
				results.push_back(source.substr(prev, next - prev));
			prev = next + 1;
		}
		if (prev < source.size())
			results.push_back(source.substr(prev));

		return results;
	}
}
