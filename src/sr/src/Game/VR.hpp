#pragma once
#include "Base.hpp"

// IW3SR clients in VR share where their head, hands and body trackers are as out of band "vr" datagrams:
// the version, the parts present, the trackers present, the head's angles, then the poses. The server
// keeps the head's angles for whoever follows that player, and relays the rest untouched to the clients
// whose userinfo carries sr_vrView. MSG_WriteByte carries the size, so a state tops out at 255 bytes.
#define VR_VERSION 1
#define VR_MAX_STATE 255

namespace SR
{
	// The last state a client sent: when, and where its head looked.
	struct VRState
	{
		int Time = 0;
		vec3_t HeadAngles = {};
	};

	class VR
	{
	public:
		static inline cvar_t *Relay = nullptr;
		static inline std::array<VRState, MAX_CLIENTS> States = {};

		static void Initialize();
		static void Frame();
		static void ResetClient(int clientNum);
		static void Receive(client_t *cl, const uint8_t *data, int size);
		static bool IsViewer(int clientNum);
	};
}
