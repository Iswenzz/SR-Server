#pragma once
#include "Macros.hpp"

IZ_C_START

struct pml_t;
struct pmove_t;

void SR_Initialize();
void SR_Shutdown();
void SR_SpawnServer(const char *levelname);
void SR_Restart();
void SR_Frame();
void SR_Packet(netadr_t *from, client_t *cl, msg_t *msg);

void SR_InitializePlayer(client_t *cl);
void SR_FreePlayer(client_t *cl);
void SR_InitializeEntity(gentity_t *ent);
void SR_ClientSpawn(gclient_t *client);
void SR_CalculateFrame(client_t *cl, usercmd_t *cmd);
void SR_BroadcastVoice(gentity_t *talker, struct VoicePacket_t *packet);
void SR_VRPacket(client_t *cl, msg_t *msg);
qboolean SR_IsVoiceRelayClient(client_t *cl);

void SR_NetchanDebugSize(int size);
void SR_SetMapAmbient(const char *alias, int volume);
void SR_Print(conChannel_t channel, char *msg);

void SR_DemoFrame(client_t *cl);
qboolean SR_DemoIsPlaying(client_t *cl);
void SR_DemoButton(client_t *cl, usercmd_t *cmd);
void SR_DemoUpdateEntity(client_t *cl, snapshotInfo_t *snapInfo, msg_t *msg, const int time, entityState_t *from,
	entityState_t *to, qboolean force);

int SR_PmoveGetSpeed(playerState_t *ps);
float SR_PmoveGetSpeedScale(playerState_t *ps);
int SR_PmoveGetGravity(playerState_t *ps);
float SR_PmoveGetJumpHeight(unsigned int num);
void SR_JumpUpdateSurface(playerState_t *ps, struct pml_t *pml);
int SR_PmoveWalkMove(struct pmove_t *pm, struct pml_t *pml);
int SR_PmoveAirMove(struct pmove_t *pm, struct pml_t *pml);
int SR_PmoveGroundTrace(struct pmove_t *pm, struct pml_t *pml);
int SR_PmoveCrashLand(playerState_t *ps, struct pml_t *pml);

IZ_C_END
