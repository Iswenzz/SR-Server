#pragma once
#include "Command.hpp"

namespace SR
{
	class PlayerCommands
	{
	public:
		static void Register();
		static void GetDamageTimer(scr_entref_t num);
		static void GetViewHeightLerpDown(scr_entref_t num);
		static void GetViewHeightLerpTarget(scr_entref_t num);
		static void GetViewHeightLerpTime(scr_entref_t num);
		static void GetWeaponState(scr_entref_t num);
		static void LoadState(scr_entref_t num);
		static void ProximityEnable(scr_entref_t num);
		static void RadioEnable(scr_entref_t num);
		static void RewindState(scr_entref_t num);
		static void SaveState(scr_entref_t num);
		static void SetJumpOrigin(scr_entref_t num);
		static void SurfaceFlags(scr_entref_t num);
		static void VoiceChatButtonPressed(scr_entref_t num);
	};
}
