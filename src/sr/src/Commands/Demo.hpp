#pragma once
#include "Command.hpp"

namespace SR
{
	class DemoCommands
	{
	public:
		static void Register();
		static void RegisterSpeedrunDemo();
		static void PlayDemo(scr_entref_t num);
		static void IsDemoLoaded();
		static void IsDemoPlaying(scr_entref_t num);
		static void GetSpeedrunVelocity(scr_entref_t num);
		static void GetDemoVersion(scr_entref_t num);
		static void GetDemoTimer(scr_entref_t num);
		static void GetDemoRunTime(scr_entref_t num);
		static void GetDemoWeapon(scr_entref_t num);
		static void GetDemoRightMove(scr_entref_t num);
		static void GetDemoForwardMove(scr_entref_t num);
		static void GetDemoButtons(scr_entref_t num);
		static void StopDemo(scr_entref_t num);
	};
}
