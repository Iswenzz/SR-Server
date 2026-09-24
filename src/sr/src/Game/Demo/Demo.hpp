#pragma once
#include "Base.hpp"

// clang-format off
#include <CoD4DM1/Demo/Demo.hpp>
#include <CoD4DM1/API/DemoReader.hpp>
// clang-format on

#define DEMO_MAX_CONFIGSTRINGS 2 * 2442

namespace SR
{
	struct DemoFrame
	{
		bool valid;
		playerState_t ps;
		int time;
		int fps;
		std::string playerName;
		std::vector<std::string> chat;
		std::map<int, entityState_t> entities;
		int velocity;
		char forwardmove;
		char rightmove;
		int buttons;
	};

	class Demo
	{
	public:
		std::string ID;
		Scope<CoD4::DM1::DemoReader> Reader;
		std::vector<DemoFrame> Frames{};
		std::array<std::string, DEMO_MAX_CONFIGSTRINGS> ConfigStrings{};
		std::vector<std::string> Weapons{};

		std::string MapName;
		std::string PlayerName;

		int Version = 0;
		int LastValidFrame = 0;
		bool HasReadInformations = false;
		std::atomic<bool> IsLoaded = false;

		Demo(const std::string& id, const std::string& path);
		~Demo();

		void Open(const Ref<AsyncTask>& task);
		void ReadDemoInformations();
		void Interpolate(DemoFrame& interpolateFrame);
		void GetVersion();
		int GetVelocity(DemoFrame& frame);
		std::vector<std::string> ProcessChat();
		bool CanParseSnapshot();
	};
}
