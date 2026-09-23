#pragma once
#include "Common.hpp"

struct playerState_tt
{
	int commandTime;
	int pm_type;
	int bobCycle;
	int pm_flags;
	int weapFlags;
	int otherFlags;
	int pm_time;
	vec3 origin;
	vec3 velocity;
	vec2 oldVelocity;
	int weaponTime;
	int weaponDelay;
	int grenadeTimeLeft;
	int throwBackGrenadeOwner;
	int throwBackGrenadeTimeLeft;
	int weaponRestrictKickTime;
	int foliageSoundTime;
	int gravity;
	float leanf;
	int speed;
	vec3 delta_angles;
	int groundEntityNum;
	vec3 vLadderVec;
	int jumpTime;
	float jumpOriginZ;
	int legsTimer;
	int legsAnim;
	int torsoTimer;
	int torsoAnim;
	int legsAnimDuration;
	int torsoAnimDuration;
	int damageTimer;
	int damageDuration;
	int flinchYawAnim;
	int movementDir;
	int eFlags;
	int eventSequence;
	int events[4];
	unsigned int eventParms[4];
	int oldEventSequence;
	int clientNum;
	int offHandIndex;
	OffhandSecondaryClass_t offhandSecondary;
	unsigned int weapon;
	int weaponstate;
	unsigned int weaponShotCount;
	float fWeaponPosFrac;
	int adsDelayTime;
	int spreadOverride;
	int spreadOverrideState;
	int viewmodelIndex;
	vec3 viewangles;
	int viewHeightTarget;
	float viewHeightCurrent;
	int viewHeightLerpTime;
	int viewHeightLerpTarget;
	int viewHeightLerpDown;
	vec2 viewAngleClampBase;
	vec2 viewAngleClampRange;
	int damageEvent;
	int damageYaw;
	int damagePitch;
	int damageCount;
	int stats[5];
	int ammo[128];
	int ammoclip[128];
	unsigned int weapons[4];
	unsigned int weaponold[4];
	unsigned int weaponrechamber[4];
	float proneDirection;
	float proneDirectionPitch;
	float proneTorsoPitch;
	ViewLockTypes_t viewlocked;
	int viewlocked_entNum;
	int cursorHint;
	int cursorHintString;
	int cursorHintEntIndex;
	int iCompassPlayerInfo;
	int radarEnabled;
	int locationSelectionInfo;
	sprintState_t sprintState;
	float fTorsoPitch;
	float fWaistPitch;
	float holdBreathScale;
	int holdBreathTimer;
	float moveSpeedScaleMultiplier;
	mantleState_t mantleState;
	float meleeChargeYaw;
	int meleeChargeDist;
	int meleeChargeTime;
	int perks;
	ActionSlotType_t actionSlotType[4];
	ActionSlotParam_t actionSlotParam[4];
	int entityEventSequence;
	int weapAnim;
	float aimSpreadScale;
	int shellshockIndex;
	int shellshockTime;
	int shellshockDuration;
	float dofNearStart;
	float dofNearEnd;
	float dofFarStart;
	float dofFarEnd;
	float dofNearBlur;
	float dofFarBlur;
	float dofViewmodelStart;
	float dofViewmodelEnd;
	int hudElemLastAssignedSoundID;
	objective_t objective[16];
	char weaponmodels[128];
	int deltaTime;
	int killCamEntity;
	hudElemState_t hud;
};

struct trace_tt
{
	float fraction;
	vec3 normal;
	int surfaceFlags;
	int contents;
	const char* material;
	TraceHitType hitType;
	uint16_t hitId;
	uint16_t modelIndex;
	uint16_t partName;
	uint16_t partGroup;
	byte allsolid;
	byte startsolid;
	byte walkable;
	byte padding;
};

struct pml_tt
{
	vec3 forward;
	vec3 right;
	vec3 up;
	float frametime;
	int msec;
	int walking;
	int groundPlane;
	int almostGroundPlane;
	trace_tt groundTrace;
	float impactSpeed;
	vec3 previous_origin;
	vec3 previous_velocity;
};

struct pmove_tt
{
	playerState_tt* ps;
	usercmd_t cmd;
	usercmd_t oldcmd;
	int tracemask;
	int numtouch;
	int touchents[32];
	vec3 mins;
	vec3 maxs;
	float xyspeed;
	int proneChange;
	float maxSprintTimeMultiplier;
	byte mantleStarted;
	char pad1[3];
	vec3 mantleEndPos;
	int mantleDuration;
	int viewChangeTime;
	float viewChange;
	char handler;
	char pad2[3];
};

enum pmType_t
{
	PM_NORMAL = 0x0,
	PM_NORMAL_LINKED = 0x1,
	PM_NOCLIP = 0x2,
	PM_UFO = 0x3,
	PM_SPECTATOR = 0x4,
	PM_INTERMISSION = 0x5,
	PM_LASTSTAND = 0x6,
	PM_DEAD = 0x7,
	PM_DEAD_LINKED = 0x8,
};

enum PMoveFlags
{
	PMF_NONE = 0,
	PMF_DUCKED = BIT(1),
	PMF_MANTLE = BIT(2),
	PMF_LADDER = BIT(3),
	PMF_SIGHT_AIMING = BIT(4),
	PMF_BACKWARDS_RUN = BIT(5),
	PMF_LADDER_DOWN = BIT(5) | BIT(3),
	PMF_WALKING = BIT(6),
	PMF_TIME_HARDLANDING = BIT(7),
	PMF_TIME_KNOCKBACK = BIT(8),
	PMF_PRONEMOVE_OVERRIDDEN = BIT(9),
	PMF_JUMP_HELD = BIT(10),
	PMF_FROZEN = BIT(11),
	PMF_NO_PRONE = BIT(12),
	PMF_LADDER_FALL = BIT(13),
	PMF_JUMPING = BIT(14),
	PMF_PRONE = BIT(0),
	PMF_SPRINTING = BIT(15),
	PMF_SHELLSHOCKED = BIT(16),
	PMF_MELEE_CHARGE = BIT(17),
	PMF_NO_SPRINT = BIT(18),
	PMF_NO_JUMP = BIT(19),
	PMF_VEHICLE_ATTACHED = BIT(20)
};

enum entity_event_t
{
	EV_NONE = 0x0,
	EV_FOLIAGE_SOUND = 0x1,
	EV_STOP_WEAPON_SOUND = 0x2,
	EV_SOUND_ALIAS = 0x3,
	EV_SOUND_ALIAS_AS_MASTER = 0x4,
	EV_STOPSOUNDS = 0x5,
	EV_STANCE_FORCE_STAND = 0x6,
	EV_STANCE_FORCE_CROUCH = 0x7,
	EV_STANCE_FORCE_PRONE = 0x8,
	EV_ITEM_PICKUP = 0x9,
	EV_AMMO_PICKUP = 0xA,
	EV_NOAMMO = 0xB,
	EV_EMPTYCLIP = 0xC,
	EV_EMPTY_OFFHAND = 0xD,
	EV_RESET_ADS = 0xE,
	EV_RELOAD = 0xF,
	EV_RELOAD_FROM_EMPTY = 0x10,
	EV_RELOAD_START = 0x11,
	EV_RELOAD_END = 0x12,
	EV_RELOAD_START_NOTIFY = 0x13,
	EV_RELOAD_ADDAMMO = 0x14,
	EV_RAISE_WEAPON = 0x15,
	EV_FIRST_RAISE_WEAPON = 0x16,
	EV_PUTAWAY_WEAPON = 0x17,
	EV_WEAPON_ALT = 0x18,
	EV_PULLBACK_WEAPON = 0x19,
	EV_FIRE_WEAPON = 0x1A,
	EV_FIRE_WEAPON_LASTSHOT = 0x1B,
	EV_RECHAMBER_WEAPON = 0x1C,
	EV_EJECT_BRASS = 0x1D,
	EV_MELEE_SWIPE = 0x1E,
	EV_FIRE_MELEE = 0x1F,
	EV_PREP_OFFHAND = 0x20,
	EV_USE_OFFHAND = 0x21,
	EV_SWITCH_OFFHAND = 0x22,
	EV_MELEE_HIT = 0x23,
	EV_MELEE_MISS = 0x24,
	EV_MELEE_BLOOD = 0x25,
	EV_FIRE_WEAPON_MG42 = 0x26,
	EV_FIRE_QUADBARREL_1 = 0x27,
	EV_FIRE_QUADBARREL_2 = 0x28,
	EV_BULLET_HIT = 0x29,
	EV_BULLET_HIT_CLIENT_SMALL = 0x2A,
	EV_BULLET_HIT_CLIENT_LARGE = 0x2B,
	EV_GRENADE_BOUNCE = 0x2C,
	EV_GRENADE_EXPLODE = 0x2D,
	EV_ROCKET_EXPLODE = 0x2E,
	EV_ROCKET_EXPLODE_NOMARKS = 0x2F,
	EV_FLASHBANG_EXPLODE = 0x30,
	EV_CUSTOM_EXPLODE = 0x31,
	EV_CUSTOM_EXPLODE_NOMARKS = 0x32,
	EV_CHANGE_TO_DUD = 0x33,
	EV_DUD_EXPLODE = 0x34,
	EV_DUD_IMPACT = 0x35,
	EV_BULLET = 0x36,
	EV_PLAY_FX = 0x37,
	EV_PLAY_FX_ON_TAG = 0x38,
	EV_PHYS_EXPLOSION_SPHERE = 0x39,
	EV_PHYS_EXPLOSION_CYLINDER = 0x3A,
	EV_PHYS_EXPLOSION_JOLT = 0x3B,
	EV_PHYS_JITTER = 0x3C,
	EV_EARTHQUAKE = 0x3D,
	EV_GRENADE_SUICIDE = 0x3E,
	EV_DETONATE = 0x3F,
	EV_NIGHTVISION_WEAR = 0x40,
	EV_NIGHTVISION_REMOVE = 0x41,
	EV_OBITUARY = 0x42,
	EV_NO_FRAG_GRENADE_HINT = 0x43,
	EV_NO_SPECIAL_GRENADE_HINT = 0x44,
	EV_TARGET_TOO_CLOSE_HINT = 0x45,
	EV_TARGET_NOT_ENOUGH_CLEARANCE = 0x46,
	EV_LOCKON_REQUIRED_HINT = 0x47,
	EV_FOOTSTEP_SPRINT = 0x48,
	EV_FOOTSTEP_RUN = 0x49,
	EV_FOOTSTEP_WALK = 0x4A,
	EV_FOOTSTEP_PRONE = 0x4B,
	EV_JUMP = 0x4C,
	EV_LANDING_FIRST = 0x4D,
	EV_LANDING_LAST = 0x69,
	EV_LANDING_PAIN_FIRST = 0x6A,
	EV_LANDING_PAIN_LAST = 0x86,
	EV_MAX_EVENTS = 0x87,
};

enum scriptAnimEventTypes_t
{
	ANIM_ET_PAIN = 0x0,
	ANIM_ET_DEATH = 0x1,
	ANIM_ET_FIREWEAPON = 0x2,
	ANIM_ET_JUMP = 0x3,
	ANIM_ET_JUMPBK = 0x4,
	ANIM_ET_LAND = 0x5,
	ANIM_ET_DROPWEAPON = 0x6,
	ANIM_ET_RAISEWEAPON = 0x7,
	ANIM_ET_CLIMB_MOUNT = 0x8,
	ANIM_ET_CLIMB_DISMOUNT = 0x9,
	ANIM_ET_RELOAD = 0xA,
	ANIM_ET_CROUCH_TO_PRONE = 0xB,
	ANIM_ET_PRONE_TO_CROUCH = 0xC,
	ANIM_ET_STAND_TO_CROUCH = 0xD,
	ANIM_ET_CROUCH_TO_STAND = 0xE,
	ANIM_ET_STAND_TO_PRONE = 0xF,
	ANIM_ET_PRONE_TO_STAND = 0x10,
	ANIM_ET_MELEEATTACK = 0x11,
	ANIM_ET_KNIFE_MELEE = 0x12,
	ANIM_ET_KNIFE_MELEE_CHARGE = 0x13,
	ANIM_ET_SHELLSHOCK = 0x14,
	NUM_ANIM_EVENTTYPES = 0x15,
};

IZ_C
{
	extern const vec3 CorrectSolidDeltas[26];
	extern cvar_t* bg_fallDamageMinHeight;
	extern cvar_t* bg_fallDamageMaxHeight;

	void PM_playerTrace(pmove_tt * pm, trace_tt * results, const vec3& start, const vec3& mins, const vec3& maxs,
		const vec3& end, int pass_entity_num, int content_mask);
	void PM_StepSlideMove(pmove_tt * pm, pml_tt * pml, bool gravity);
	void PM_AddTouchEnt(pmove_tt * pm, int entity_num);
	void REGPARM2 PM_Friction(playerState_tt * ps, pml_tt * pml);
	void REGPARM2 PM_GroundTrace(pmove_tt * pm, pml_tt * pml);
	void PM_ProjectVelocity(const vec3& normal, const vec3& velIn, vec3& velOut);

	void Jump_ClearState(playerState_tt * ps);
	int BG_AnimScriptEvent(playerState_tt * ps, scriptAnimEventTypes_t event, int isContinue, int force);
	void BG_AddPredictableEventToPlayerstate(int newEvent, unsigned int eventParm, playerState_tt* ps);
}

namespace SR
{
	class CoD4
	{
	public:
		static float CmdScale(playerState_tt* ps, usercmd_s* cmd);
		static void ProjectVelocity(const vec3& in, const vec3& normal, vec3& out);
		static vec3 Normalize(const vec3& v);
		static void JumpClearState(playerState_tt* ps);
		static int CorrectAllSolid(pmove_tt* pm, pml_tt* pml, trace_tt* trace);
		static void GroundTraceMissed(pmove_tt* pm, pml_tt* pml);
		static void CrashLand(playerState_tt* ps, pml_tt* pml);
		static bool InKnockback(playerState_tt* ps);
	};
}
