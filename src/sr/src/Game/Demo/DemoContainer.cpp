#include "DemoContainer.hpp"

#include "System/Environment.hpp"
#include "Utils/Log.hpp"

#include <CoD4DM1/Crypt/Huffman.hpp>

namespace SR
{
	void DemoContainer::Initialize()
	{
		CoD4::DM1::Huffman::InitMain();

		Cvar_RegisterInt("sr_demo_version", 2, 0, 100, CVAR_ROM | CVAR_SERVERINFO, "SR demo version");

		RegisterDemoFolder(Environment::ModDirectory / "demos");
		RegisterDemoFolder(Environment::ModDirectory / "wrs");
	}

	void DemoContainer::RegisterDemoFolder(const std::filesystem::path &path)
	{
		Directories.push_back(path.string());
	}

	int DemoContainer::RegisterSpeedrunDemo(const std::string &map, const std::string &playerId, const std::string &run,
		const std::string &mode, const std::string &way)
	{
		// Called from script: a filesystem exception here would terminate the server.
		std::vector<std::string> demos{};
		for (const std::string &folder : Directories)
		{
			std::error_code error;
			const auto demoDirectory = std::filesystem::path(folder) / playerId / map;

			for (const auto &entry : std::filesystem::directory_iterator(demoDirectory, error))
			{
				const std::string entryName = entry.path().filename().string();
				if (entryName == run + ".dm_1" || entryName.starts_with(run + "_"))
					demos.push_back(entry.path().string());
			}
		}
		if (demos.size())
		{
			std::string id = std::format("times_{}_{}", mode, way);
			const auto found = Demos.find(id);

			if (found != std::end(Demos))
			{
				if (!found->second->IsLoaded)
					return 2;
				Demos.erase(id);
			}
			const auto demoPath = demos.at(0);
			std::error_code error;
			const auto relativePath = std::filesystem::relative(demoPath, Environment::ModDirectory, error).string();
			Demos.insert({ id, CreateRef<Demo>(id, demoPath) });
			Log::WriteLine("^5[DemoContainer] Register demo {} {}", id.c_str(), relativePath.c_str());
			return 1;
		}
		return 0;
	}
}
