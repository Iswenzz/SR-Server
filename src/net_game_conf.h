// server.h

// #include "../game/q_shared.h"
// #include "../qcommon/qcommon.h"
// #include "../game/g_public.h"
// #include "../game/bg_public.h"

#ifndef __NET_GAMECONF_H__
#define __NET_GAMECONF_H__

#ifndef AUTHORIZE_SERVER_NAME
	#define AUTHORIZE_SERVER_NAME "cod4master.activision.com"
#endif

#define AUTHORIZE_TIMEOUT 10000

#ifndef PORT_AUTHORIZE
	#define PORT_AUTHORIZE 20800
#endif

#define PORT_SERVER 28960

#define PROTOCOL_VERSION (unsigned int)(Sys_GetCommonVersion() + 0.00001)

#define LEGACY_PROTOCOL_VERSION 6

/* Protocols at or below this speak the stock 1.7/1.7a wire format: no reliable
   transport, no configclient/configdata, and no configDataAcknowledge in the
   packet header. Matches the <= 7 test the update proxy has always used. */
#define MAX_LEGACY_PROTOCOL_VERSION 7
#define IS_LEGACY_PROTOCOL(v) ((v) <= MAX_LEGACY_PROTOCOL_VERSION)

/* Stock clients size their configstring table for 1.7; CoD4X doubled it. */
#define MAX_LEGACY_CONFIGSTRINGS 2442

#define MAX_MSGLEN 0x20000

// the svc_strings[] array in cl_parse.c should mirror this
//
// server to client
//
enum svc_ops_e
{
	svc_nop,
	svc_gamestate,
	svc_configstring,  // [short] [string] only in gamestate messages
	svc_baseline,	   // only in gamestate messages
	svc_serverCommand, // [string] to be executed by client game module
	svc_download,	   // [short] size [size bytes]
	svc_snapshot,
	svc_EOF,
	svc_steamcommands,
	svc_statscommands,
	svc_configdata,
	svc_configclient,
	svc_acdata
};

//
// client to server
//

enum clc_ops_e
{
	clc_move,		   // [[usercmd_t]
	clc_moveNoDelta,   // [[usercmd_t]
	clc_clientCommand, // [string] message
	clc_EOF,
	clc_nop,
	clc_download,
	clc_empty1,
	clc_empty2,
	clc_steamcommands,
	clc_statscommands,
	clc_empty3,
	clc_empty4,
	clc_acdata
};

#endif
