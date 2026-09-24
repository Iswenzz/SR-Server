#include "API.hpp"
#include "Application.hpp"

#include "Audio/Voice.hpp"
#include "Game/Demo/DemoRecord.hpp"
#include "Game/Entity/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/Player/Player.hpp"
#include "Game/Server.hpp"
#include "System/Debug.hpp"
#include "System/Netchan.hpp"

IZ_C_START

void SR_Initialize()
{
	Application::Start();
}

void SR_Shutdown()
{
	Application::Shutdown();
}

void SR_InitializePlayer(client_t *cl)
{
	if (!DEFINED_CLIENT(cl))
		return;

	Player::Add(cl);
}

void SR_FreePlayer(client_t *cl)
{
	const int num = cl - svs.clients;
	if (!Player::Get(num))
		return;

	Player::List[num]->Disconnect();
	Player::List[num].reset();
}

void SR_ClientSpawn(gclient_t *client)
{
	if (!DEFINED_GCLIENT(client))
		return;

	// ClientBegin can spawn from script before SR_InitializePlayer runs.
	const auto &player = Player::Get(client->ps.clientNum);
	if (player)
		player->Spawn();
}

void SR_CalculateFrame(client_t *cl, usercmd_t *cmd)
{
	if (!DEFINED_CLIENT(cl))
		return;

	const auto &player = Player::Get(cl->gentity->s.number);
	if (player)
		player->CalculateFrame(cmd->serverTime - cl->lastUsercmd.serverTime);
}

void SR_InitializeEntity(gentity_t *ent)
{
	if (!DEFINED_ENTITY(ent))
		return;

	Entity::Add(ent);
}

void SR_SetMapAmbient(const char *alias, int volume)
{
	Map::SetAmbient(alias, volume);
}

void SR_Frame()
{
	Server::Frame();
}

void SR_SpawnServer(const char *levelname)
{
	Server::Spawn(levelname);
}

void SR_Restart()
{
	Server::Restart();
}

void SR_BroadcastVoice(gentity_t *talker, VoicePacket_t *packet)
{
	Voice::BroadcastVoice(talker, packet);
}

qboolean SR_DemoIsPlaying(client_t *cl)
{
	if (!DEFINED_CLIENT(cl))
		return qfalse;

	const auto &player = Player::Get(cl->gentity->client->ps.clientNum);
	return static_cast<qboolean>(player && !!player->DemoPlayer->Demo);
}

void SR_DemoUpdateEntity(client_t *cl, snapshotInfo_t *snapInfo, msg_t *msg, const int time, entityState_t *from,
	entityState_t *to, qboolean force)
{
	const auto &player = Player::Get(cl->gentity->client->ps.clientNum);
	if (player)
		player->DemoPlayer->UpdateEntity(snapInfo, msg, time, from, to, force);
	else
		MSG_WriteDeltaEntity(snapInfo, msg, time, from, to, force);
}

void SR_DemoButton(client_t *cl, usercmd_t *cmd)
{
	if (!DEFINED_CLIENT(cl))
		return;

	cl->gentity->client->ps.dofNearStart = *reinterpret_cast<float *>(&cmd->forwardmove);
	cl->gentity->client->ps.dofNearEnd = *reinterpret_cast<float *>(&cmd->rightmove);
	cl->gentity->client->ps.dofFarStart = *reinterpret_cast<float *>(&cmd->buttons);
}

void SR_DemoFrame(client_t *cl)
{
	DemoRecord::Frame(cl);
}

void SR_Packet(netadr_t *from, client_t *cl, msg_t *msg)
{
	Netchan::Packet(from, cl, msg);
}

void SR_Print(conChannel_t channel, char *msg)
{
	Log::Write(msg);
}

int SR_PmoveGetSpeed(playerState_t *ps)
{
	const auto &player = Player::Get(ps->clientNum);
	return player ? player->PMove->GetSpeed() : ps->speed;
}

float SR_PmoveGetSpeedScale(playerState_t *ps)
{
	const auto &player = Player::Get(ps->clientNum);
	return player ? player->PMove->GetSpeedScale() : ps->moveSpeedScaleMultiplier;
}

int SR_PmoveGetGravity(playerState_t *ps)
{
	const auto &player = Player::Get(ps->clientNum);
	return player ? player->PMove->GetGravity() : ps->gravity;
}

float SR_PmoveGetJumpHeight(unsigned int num)
{
	const auto &player = Player::Get(num);
	return player ? player->PMove->GetJumpHeight() : 0;
}

void SR_JumpUpdateSurface(playerState_s *ps, pml_t *pml)
{
	const auto &player = Player::Get(ps->clientNum);
	if (player)
		player->PMove->JumpUpdateSurface(pml);
}

int SR_PmoveWalkMove(pmove_t *pm, pml_t *pml)
{
	const auto &player = Player::Get(pm->ps->clientNum);
	return player && player->PMove->WalkMove(pm, pml);
}

int SR_PmoveAirMove(pmove_t *pm, pml_t *pml)
{
	const auto &player = Player::Get(pm->ps->clientNum);
	return player && player->PMove->AirMove(pm, pml);
}

int SR_PmoveGroundTrace(pmove_t *pm, pml_t *pml)
{
	const auto &player = Player::Get(pm->ps->clientNum);
	return player && player->PMove->GroundTrace(pm, pml);
}

int SR_PmoveCrashLand(playerState_s *ps, pml_t *pml)
{
	const auto &player = Player::Get(ps->clientNum);
	return player && player->PMove->CrashLand(ps, pml);
}

void SR_NetchanDebugSize(int size)
{
	Debug::NetchanPacketSize(size);
}

IZ_C_END
