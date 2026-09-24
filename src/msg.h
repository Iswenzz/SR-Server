#ifndef __MSG_H__
#define __MSG_H__

#include "q_shared.h"
#include "entity.h"
#include "q_shared.h"
#include "player.h"

#include <stdint.h>

typedef struct snapshotInfo_s
{
	int clnum;
	struct client_s *client;
	int snapshotDeltaTime;
	byte fromBaseline;
	byte archived;
	byte pad[2];
} snapshotInfo_t;

//
// msg.c
//
typedef struct
{
	qboolean overflowed; // 0x00
	qboolean readonly;	 // 0x04
	byte *data;			 // 0x08
	byte *splitData;	 // 0x0c
	int maxsize;		 // 0x10
	int cursize;		 // 0x14
	int splitSize;		 // 0x18
	int readcount;		 // 0x1c
	int bit;			 // 0x20	// for bitwise reads and writes
	union
	{
		int lastRefEntity; // 0x24
		int lengthoffset;
	};
} msg_t; // Size: 0x28

typedef struct netField_s
{
	char *name;
	int offset;
	int bits; // 0 = float
	byte changeHints;
	byte pad[3];
} netField_t;

struct clientState_s;
struct playerState_s;
struct usercmd_s;

enum DeltaFlags
{
	DELTA_FLAGS_NONE = 0x1,
	DELTA_FLAGS_FORCE = 0x0,
};

#ifdef __cplusplus
extern "C"
{
#endif
	void MSG_Init(msg_t *buf, byte *data, int length);
	void MSG_InitReadOnly(msg_t *buf, byte *data, int length);
	void MSG_InitReadOnlySplit(msg_t *buf, byte *data, int length, byte *, int);
	void MSG_Clear(msg_t *buf);
	void MSG_BeginReading(msg_t *msg);
	void MSG_Copy(msg_t *buf, byte *data, int length, msg_t *src);
	void MSG_WriteByte(msg_t *msg, int c);
	void MSG_WriteShort(msg_t *msg, int c);
	void MSG_WriteLong(msg_t *msg, int c);
	void MSG_WriteFloat(msg_t *msg, float c);
	void MSG_WriteData(msg_t *buf, const void *data, int length);
	void MSG_WriteString(msg_t *sb, const char *s);
	void MSG_WriteBigString(msg_t *sb, const char *s);
	int MSG_ReadByte(msg_t *msg);
	int MSG_ReadShort(msg_t *msg);
	int MSG_ReadLong(msg_t *msg);
	char *MSG_ReadString(msg_t *msg, char *bigstring, int len);
	char *MSG_ReadStringLine(msg_t *msg, char *bigstring, int len);
	void MSG_ReadData(msg_t *msg, void *data, int len);
	float MSG_ReadFloat(msg_t *msg);
	void MSG_ClearLastReferencedEntity(msg_t *msg);
	void MSG_SetLegacyOrigin(qboolean enable);
	void MSG_WriteDeltaEntity(struct snapshotInfo_s *snapInfo, msg_t *msg, const int time, entityState_t *from,
		entityState_t *to, qboolean force);
	void MSG_WriteBit0(msg_t *msg);
	int MSG_WriteBitsNoCompress(int d, byte *src, byte *dst, int size);
	void MSG_WriteVector(msg_t *msg, vec3_t c);
	void MSG_WriteInt64(msg_t *msg, int64_t c);
	int64_t MSG_ReadInt64(msg_t *msg);

	void MSG_WriteBit1(msg_t *);
	void MSG_WriteBits(msg_t *, int bits, int bitcount);
	int MSG_ReadBits(msg_t *msg, int numBits);
	void MSG_WriteReliableCommandToBuffer(const char *source, char *destination, int length);

	int MSG_ReadDeltaClient(msg_t *msg, const int time, clientState_t *from, clientState_t *to, int number);
	void MSG_WriteDeltaClient(struct snapshotInfo_s *snapInfo, msg_t *msg, const int time, clientState_t *from,
		clientState_t *to, qboolean force);
	// void MSG_WriteDeltaField(struct snapshotInfo_s *snapInfo, msg_t *msg, const int time, const byte *from, const
	// byte *to, const struct netField_s* field, int fieldNum, byte forceSend);
	void __cdecl MSG_WriteDeltaPlayerstate(struct snapshotInfo_s *, msg_t *, int, struct playerState_s *,
		struct playerState_s *);
	void __cdecl MSG_ReadDeltaPlayerstate(const int localClientNum, msg_t *msg, const int time, playerState_t *from,
		playerState_t *to, bool predictedFieldsIgnoreXor);
	void __cdecl MSG_WriteEntityIndex(struct snapshotInfo_s *, msg_t *, int, int);
	void __cdecl MSG_ReadDeltaUsercmdKey(msg_t *msg, int key, struct usercmd_s *from, struct usercmd_s *to);
	void __cdecl MSG_SetDefaultUserCmd(struct playerState_s *ps, struct usercmd_s *ucmd);
	void MSG_WriteBase64(msg_t *msg, byte *inbuf, int len);
	void MSG_ReadBase64(msg_t *msg, byte *outbuf, int len);
	void MSG_BeginWriteMessageLength(msg_t *msg);
	void MSG_EndWriteMessageLength(msg_t *msg);

	int MSG_ReadBit(msg_t *msg);
	int MSG_ReadEntityIndex(msg_t *msg, int numBits);
	void MSG_WriteDeltaClient(struct snapshotInfo_s *snapInfo, msg_t *msg, const int time, clientState_t *from,
		clientState_t *to, qboolean force);
	void MSG_RegisterCvars();
	int __cdecl MSG_WriteDelta_LastChangedField(byte *from, byte *to, netField_t *fields, int numFields);

#ifdef __cplusplus
}
#endif

#endif
