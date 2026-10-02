#include "VR.hpp"
#include "Utils/Utils.hpp"

// A followed player's head is shown for this long after its last state, then its aim again.
#define VR_STALE_TIME 500
// The first bytes of a state: version, parts, trackers, then the head's pitch, yaw and roll and the
// body's yaw as shorts.
#define VR_HEAD_ANGLES 3
#define VR_HEADER_SIZE 11

namespace SR
{
	void VR::Initialize()
	{
		// In the systeminfo, where IW3SR clients look for it before sending anything.
		Relay = Cvar_RegisterBool("sr_vrRelay", qtrue, CVAR_SYSTEMINFO | CVAR_ROM,
			"Relays VR state between the IW3SR clients that draw it");

		for (int i = 0; i < MAX_CLIENTS; i++)
			ResetClient(i);
	}

	void VR::ResetClient(int clientNum)
	{
		if (clientNum < 0 || clientNum >= MAX_CLIENTS)
			return;

		States[clientNum] = {};
	}

	// IW3SR draws VR players since 1.8.3; any other client would only print the datagrams as unknown.
	bool VR::IsViewer(int clientNum)
	{
		return clientNum < sv_maxclients->integer && Utils::ClientVersion(clientNum, 1, 8, 3);
	}

	// Checked only as far as relaying it safely needs: the clients that draw it parse the rest.
	void VR::Receive(client_t *cl, const uint8_t *data, int size)
	{
		const int num = cl - svs.clients;
		if (num < 0 || num >= MAX_CLIENTS || size < VR_HEADER_SIZE || size > VR_MAX_STATE || data[0] != VR_VERSION)
			return;

		VRState &state = States[num];
		state.Time = svs.time;
		for (int i = 0; i < 3; i++)
		{
			const int value = data[VR_HEAD_ANGLES + i * 2] | (data[VR_HEAD_ANGLES + i * 2 + 1] << 8);
			state.HeadAngles[i] = SHORT2ANGLE(value);
		}

		byte buffer[VR_MAX_STATE + 16];
		msg_t msg;
		MSG_Init(&msg, buffer, sizeof(buffer));
		MSG_WriteString(&msg, "vr");
		MSG_WriteByte(&msg, 1);
		MSG_WriteByte(&msg, num);
		MSG_WriteByte(&msg, size);
		MSG_WriteData(&msg, data, size);
		if (msg.overflowed)
			return;

		for (int i = 0; i < sv_maxclients->integer; i++)
		{
			client_t *other = &svs.clients[i];
			if (i != num && other->state >= CS_ACTIVE && IsViewer(i))
				NET_OutOfBandData(NS_SERVER, &other->netchan.remoteAddress, msg.data, msg.cursize);
		}
	}

	// Whoever follows a VR player sees where that player's head looks, rather than where it aims. The
	// snapshots are built by now and not yet sent.
	void VR::Frame()
	{
		for (int i = 0; i < sv_maxclients->integer; i++)
		{
			client_t *cl = &svs.clients[i];
			if (cl->state < CS_ACTIVE)
				continue;

			clientSnapshot_t *frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
			const int followed = frame->ps.clientNum;
			if (!(frame->ps.otherFlags & 2) || followed == i || followed < 0 || followed >= MAX_CLIENTS)
				continue;

			const VRState &state = States[followed];
			if (state.Time && svs.time - state.Time <= VR_STALE_TIME)
				VectorCopy(state.HeadAngles, frame->ps.viewangles);
		}
	}
}
