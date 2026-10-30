#include "Layout.hpp"

#define LAYOUT_NOT_CPM 1
#define LAYOUT_NOT_VQ3 2

namespace SR
{
	void Layout::Restart()
	{
		ByModel.clear();
		Loaded = false;
	}

	// The layout of every brush model the map's entity string tags, by the
	// "*n" of its entity's model key. Read once a map is loaded.
	void Layout::Load()
	{
		Loaded = true;
		ByModel.clear();

		const char *text = CM_EntityString();
		if (!text)
			return;

		const std::string_view entities(text);
		size_t at = 0;
		while ((at = entities.find('{', at)) != std::string_view::npos)
		{
			const size_t end = entities.find('}', at);
			if (end == std::string_view::npos)
				break;
			const std::string_view block = entities.substr(at, end - at);
			at = end;

			int model = -1, layout = 0;
			size_t cursor = 0;
			while (true)
			{
				const size_t keyStart = block.find('"', cursor);
				const size_t keyEnd = block.find('"', keyStart + 1);
				const size_t valueStart = block.find('"', keyEnd + 1);
				const size_t valueEnd = block.find('"', valueStart + 1);
				if (keyStart == std::string_view::npos || keyEnd == std::string_view::npos
					|| valueStart == std::string_view::npos || valueEnd == std::string_view::npos)
					break;
				cursor = valueEnd + 1;

				const std::string_view key = block.substr(keyStart + 1, keyEnd - keyStart - 1);
				const std::string value(block.substr(valueStart + 1, valueEnd - valueStart - 1));
				if (key == "model" && value.starts_with('*'))
					model = std::atoi(value.c_str() + 1);
				else if (key == "notcpm" && std::atoi(value.c_str()))
					layout |= LAYOUT_NOT_CPM;
				else if (key == "notvq3" && std::atoi(value.c_str()))
					layout |= LAYOUT_NOT_VQ3;
			}
			if (model > 0 && layout)
				ByModel[model] = layout;
		}
	}

	// Whether an entity is out of a client's layout. CPM players (sr_mode Q3CPM
	// or Q3CPMW) run the CPM one, everyone else the VQ3 one, as their movement
	// does (PMove::GetMovementMode).
	bool Layout::Hidden(int clientNum, gentity_t *ent)
	{
		if (clientNum < 0 || clientNum >= MAX_CLIENTS || !ent || !ent->r.bmodel)
			return false;
		if (!Loaded)
			Load();
		if (ByModel.empty())
			return false;

		const auto layout = ByModel.find(ent->s.index);
		if (layout == ByModel.end())
			return false;

		const int mode = SV_GetClientStat(clientNum, 1700);
		const bool cpm = mode == 4 || mode == 5;
		return layout->second & (cpm ? LAYOUT_NOT_CPM : LAYOUT_NOT_VQ3);
	}
}
