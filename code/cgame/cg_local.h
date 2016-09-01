/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"


// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#ifdef MISSIONPACK
#define CG_FONT_THRESHOLD 0.1
#endif

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
#define	ZOOM_TIME			150
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

// mmp
#define FALL_TIME			500

#define ITEM_GET_FLASH		250
#define	MAX_FOV				140 // was 160, but you were able to see through walls
#define OVERSCAN_ADJ		0.90 // assume 90 percent of screen is at least visible

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	TEAMCHAT_WIDTH		80 // nolonger used
#define TEAMCHAT_HEIGHT		8
#define CHAT_HEIGHT			8
#define NOTIFY_HEIGHT		8

#define HUD_TEXT_HEIGHT		8
#define HUD_BOARD_TEXT_HEIGHT	10
#define TOTAL_HUD_TEXT_BOXES	4
#define HUD_TEXT_BUFFER_SIZE	256
#define HUD_FRAG_STAT_NAME_BUFFER_SIZE		48

#define	HUD_POS_LOCK_TYPES	6

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define	NUM_CROSSHAIRS		10

#define	NUM_HUDBAR_TYPES	10
//#define	NUM_HUDBAR_PARTS	6
#define	NUM_BORDER_TYPES	10
#define	NUM_MINISCORE_TYPES	10
#define	NUM_PLAYER_PORTS	128
#define	NUM_FIXED_PORTS		10

#define	NUM_CHATSOUND_TYPES	9
#define	NUM_HITSOUND_TYPES	9
#define	NUM_KILLSOUND_TYPES	9

// spectator info
#define	SINFO_CHAR_BUFFER_SIZE			1024

// filter color flags
#define	FC_CROSSHAIR_NAMES	1
#define	FC_SCOREBOARD_NAMES	2
#define	FC_TEAM_OVERLAY		4
#define	FC_FRAGGED_NAMES	8
#define	FC_CHAT_TEXT		16

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#define	DEFAULT_MODEL			"sarge"
#ifdef MISSIONPACK
#define	DEFAULT_TEAM_MODEL		"james"
#define	DEFAULT_TEAM_HEAD		"*james"
#else
#define	DEFAULT_TEAM_MODEL		"sarge"
#define	DEFAULT_TEAM_HEAD		"sarge"
#endif

#define DEFAULT_REDTEAM_NAME		"Red"
#define DEFAULT_BLUETEAM_NAME		"Blue"

#define SHORT_RED_TEAM_NAME		S_COLOR_RED"[R]"S_COLOR_WHITE
#define SHORT_BLUE_TEAM_NAME		S_COLOR_BLUE"[B]"S_COLOR_WHITE
#define SHORT_SPECTATOR_TEAM_NAME	S_COLOR_CYAN"[S]"S_COLOR_WHITE

typedef enum {
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,

	FOOTSTEP_TOTAL
} footstep_t;

typedef enum {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact
} lerpFrame_t;


typedef struct {
	lerpFrame_t		legs, torso, flag;
	int				painTime;
	int				painDirection;	// flip from 0 to 1
	int				lightningFiring;

	int				railFireTime;

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;
} playerEntity_t;

//=================================================



// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;
	int				teleportFlag;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				dustTrailTime;
	int				miscTime;

	int				snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t	pe;

	int				errorTime;		// decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;

	qboolean		extrapolated;	// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SCOREPLUM,
#ifdef MISSIONPACK
	LE_KAMIKAZE,
	LE_INVULIMPACT,
	LE_INVULJUICED,
	LE_SHOWREFENTITY
#endif
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	LEF_TUMBLE			 = 0x0002,			// tumble over time, used for ejecting shells
	LEF_SOUND1			 = 0x0004,			// sound 1 for kamikaze
	LEF_SOUND2			 = 0x0008			// sound 2 for kamikaze
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	refEntity_t		refEntity;
} localEntity_t;

//======================================================================


typedef struct {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;
	int				powerUps;
	int				accuracy;
	int				impressiveCount;
	int				excellentCount;
	int				guantletCount;
	int				defendCount;
	int				assistCount;
	int				captures;
	qboolean	perfect;
	int				team;
} score_t;


// end-game stats
typedef struct {
	int				clientNo;

	int				sessionTeam;
	int				score;
	int				ping;
	int				playTime;
	int				kills;
	int				deaths;
	int				suicides;
	int				teamKills;
	int				captures;
	int				killStreak;
} endGameStats_t;


// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_SOUNDS	32

typedef struct {
	qboolean		infoValid;

	char			name[MAX_QPATH];
	char			clan[MAX_QPATH];
	char			port[196];
	team_t			team;

	int				botSkill;		// 0 = not bot, 1-5 = bot

	vec3_t			color1; // to be removed
	vec3_t			color2; // to be removed

	byte			c1RGBA[4];
	byte			c2RGBA[4];

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				armor;
	int				curWeapon;
	int				armorLvl;
	int				damageLvl;
	int				keycards;

	// position of player for use with mini map
	int				posXLoc;
	int				posYLoc;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				teamTask;		// task in teamplay (offence/defence)
	qboolean		teamLeader;		// true when this is a team leader

	int				powerups;		// so can display quad/flag status

	int				medkitUsageTime;
	int				invulnerabilityStartTime;
	int				invulnerabilityStopTime;

	int				breathPuffTime;

	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
	char			headModelName[MAX_QPATH];
	char			headSkinName[MAX_QPATH];
	char			redTeam[MAX_TEAMNAME]; // to be removed
	char			blueTeam[MAX_TEAMNAME]; // to be removed
	qboolean		deferred;

	qboolean		newAnims;		// true if using the new mission pack animations
	qboolean		fixedlegs;		// true if legs yaw is always the same as torso yaw
	qboolean		fixedtorso;		// true if torso never changes yaw

	vec3_t			headOffset;		// move head in icon views
	footstep_t		footsteps;
	gender_t		gender;			// from model

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	qhandle_t		headModel;
	qhandle_t		headSkin;

	qhandle_t		modelIcon;

	animation_t		animations[MAX_TOTALANIMATIONS];

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];
} clientInfo_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	vec3_t			flashDlightColor;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose

	qhandle_t		weaponIcon;
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;
	int				missileRenderfx;

	void			(*ejectBrassFunc)( centity_t * );

	float			trailRadius;
	float			wiTrailTime;

	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;
} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
} itemInfo_t;


typedef struct {
	int				itemNum;
} powerupInfo_t;


#define MAX_SKULLTRAIL		10

typedef struct {
	vec3_t positions[MAX_SKULLTRAIL];
	int numpositions;
} skulltrail_t;


#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16

//unlagged - optimized prediction
#define NUM_SAVED_STATES (CMD_BACKUP + 2)
//unlagged - optimized prediction

typedef struct {
	int			clientFrame;		// incremented each frame

	int			clientNum;

	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			timelimitTicks;		// less than 10 seconds left
	int			fraglimitWarnings;

	int			itemGetFlashTime;	// this controls the screen flash when getting an item

	qboolean	mapRestart;			// set on a map restart to set back the weapon

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	centity_t		predictedPlayerEntity;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;

	// physics select
	int			physicsSelect;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef;
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis

	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			selectedScore;
	int			teamScores[2];
	int			readyMask[4];		// warm up ready flags
	score_t		scores[MAX_CLIENTS];
	qboolean	showScores;
	qboolean	scoreBoardShowing;
	int			scoreFadeTime;
	char		killerName[MAX_NAME_LENGTH];
	char			spectatorList[MAX_STRING_CHARS];		// list of names
	int				spectatorLen;												// length of list
	float			spectatorWidth;											// width in device units
	int				spectatorTime;											// next time to offset
	int				spectatorPaintX;										// current paint x
	int				spectatorPaintX2;										// current paint x
	int				spectatorOffset;										// current offset from start
	int				spectatorPaintLen; 									// current offset from start

#ifdef MISSIONPACK
	// skull trails
	skulltrail_t	skulltrails[MAX_CLIENTS];
#endif

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;
	int			voiceTime;

	// reward medals
	int			rewardStack;
	int			rewardTime;
	int			rewardCount[MAX_REWARDSTACK];
	qhandle_t	rewardShader[MAX_REWARDSTACK];
	qhandle_t	rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

#ifdef MISSIONPACK
	// for voice chat buffer
	int			voiceChatTime;
	int			voiceChatBufferIn;
	int			voiceChatBufferOut;
#endif

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately
	int			itemPickupMultiCount;
	int			itemPickupPrev;

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;
	int     nextOrbitTime;

	//qboolean cameraMode;		// if rendering from a loaded camera


	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

	// mega hud temp layout pos
	int			ypos[HUD_POS_LOCK_TYPES];

//unlagged - optimized prediction
	int			lastPredictedCommand;
	int			lastServerTime;
	playerState_t savedPmoveStates[NUM_SAVED_STATES];
	int			stateHead, stateTail;
//unlagged - optimized prediction

} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
	qhandle_t	charsetShader;
	qhandle_t	charsetProp;
	qhandle_t	charsetPropHQ;
	qhandle_t	charsetPropGlow;
	qhandle_t	charsetPropB;
	qhandle_t	charsetDigit;
	qhandle_t	charsetDigitHQ;
	qhandle_t	charsetBigNum;
	qhandle_t	charsetBigNumGrad;
	qhandle_t	whiteShader;
	qhandle_t	numChar;

#ifdef MISSIONPACK
	qhandle_t	redCubeModel;
	qhandle_t	blueCubeModel;
	qhandle_t	redCubeIcon;
	qhandle_t	blueCubeIcon;
#endif
	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	neutralFlagModel;
	qhandle_t	redFlagShader[3];
	qhandle_t	blueFlagShader[3];
	qhandle_t	flagShader[4];

	qhandle_t	flagPoleModel;
	qhandle_t	flagFlapModel;

	qhandle_t	redFlagFlapSkin;
	qhandle_t	blueFlagFlapSkin;
	qhandle_t	neutralFlagFlapSkin;

	qhandle_t	redFlagBaseModel;
	qhandle_t	blueFlagBaseModel;
	qhandle_t	neutralFlagBaseModel;

#ifdef MISSIONPACK
	qhandle_t	overloadBaseModel;
	qhandle_t	overloadTargetModel;
	qhandle_t	overloadLightsModel;
	qhandle_t	overloadEnergyModel;

	qhandle_t	harvesterModel;
	qhandle_t	harvesterRedSkin;
	qhandle_t	harvesterBlueSkin;
	qhandle_t	harvesterNeutralModel;
#endif

	qhandle_t	armorModel;
	qhandle_t	armorIcon;

	qhandle_t	teamStatusBar;

	qhandle_t	deferShader;

	// gib explosions
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

	qhandle_t	smoke2;

	qhandle_t	machinegunBrassModel;
	qhandle_t	shotgunBrassModel;

	qhandle_t	laserRingsShader; // TODO: needs to be renamed, the laser gun was removed a long time ago, plasma gun and spread shot only uses this shader
	qhandle_t	laserCoreShader;

	qhandle_t	lightningShader;

	qhandle_t	friendShader;

	qhandle_t	balloonShader;
	qhandle_t	connectionShader;

	qhandle_t	selectShader;
	qhandle_t	viewBloodShader;
	qhandle_t	tracerShader;
//	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	crosshairTypeA[NUM_CROSSHAIRS - 1];
	qhandle_t	crosshairTypeB[NUM_CROSSHAIRS - 1];
	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;
	qhandle_t	noammoShader;

	qhandle_t	smokePuffShader;
	qhandle_t	smokePuffRageProShader;
	qhandle_t	blasterBallShader; // mmp
	qhandle_t	shotgunSmokePuffShader;
	qhandle_t	plasmaBallShader;
	qhandle_t	spreadBallShader;
	qhandle_t	waterBubbleShader;
	qhandle_t	bloodTrailShader;
#ifdef MISSIONPACK
	qhandle_t	nailPuffShader;
	qhandle_t	blueProxMine;
#endif

	qhandle_t	flameBallShader; // mmp

	qhandle_t	numberShaders[11];
	qhandle_t	hudBarShaders[NUM_HUDBAR_TYPES];
//	qhandle_t	hudBarShaders[NUM_HUDBAR_TYPES][NUM_HUDBAR_PARTS];
	qhandle_t	hudBorderShaders[NUM_BORDER_TYPES];
	qhandle_t	hudGrad;
	qhandle_t	hudGradRCurve;
	qhandle_t	hudPing;
	qhandle_t	hudMiniScoreShaders[NUM_MINISCORE_TYPES];
	qhandle_t	hudFlagStatus[NUM_MINISCORE_TYPES];

	qhandle_t	hudKeycards;

	qhandle_t	oldFilm1;
	qhandle_t	oldFilm2;

	qhandle_t	teamBG;

	// scoreboard portrait
	qhandle_t	hudDefaultPort;
	qhandle_t	hudFixedPort[NUM_FIXED_PORTS];
	qhandle_t	hudPlayerPort[NUM_PLAYER_PORTS];

	qhandle_t	armorIconWhite;
	qhandle_t	healthIconWhite;
	qhandle_t	flagIconWhite;
	qhandle_t	skullIconWhite; // not used?

	// in-game mini map
	qhandle_t	miniMap;
	qhandle_t	miniMapPlyr;
	qhandle_t	miniMapTeam;

	// gametype icon (note that sp will use the dm icon)
	qhandle_t	gametypeIcon_dm;
	qhandle_t	gametypeIcon_duel;
	qhandle_t	gametypeIcon_tdm;
	qhandle_t	gametypeIcon_ctf;

	// gametype icons
	qhandle_t	gametypeIcons;

	qhandle_t	scoreIconOK; // used for ready icon
	qhandle_t	scoreIconNo; // used for break icon
//	qhandle_t	scoreIconOut;
	qhandle_t	scoreIconLock; // used for something... might have to do with pizza
	qhandle_t	iconCam;
	qhandle_t	iconSkull;
	qhandle_t	iconSkullFlame;
	qhandle_t	iconSkullTele;
	qhandle_t	iconDrown;

	qhandle_t	rulesBG256; // ruleset background
	qhandle_t	rulesBG064; // objective background

	// button press
	qhandle_t	hudButtonPress;

	qhandle_t	hudScoreBoard[TEAM_BLUE+1]; // for FREE, RED, and BLUE
	qhandle_t	hudScoreBoardSpec; // spectator background
	qhandle_t	hudScoreBoardBG; // score background
	qhandle_t	hudHighlight; // highlight

	qhandle_t	hudTopScore; // top score display
	qhandle_t	hudTopScoreEfx;

	qhandle_t	shadowMarkShader;

	qhandle_t	botSkillShaders[5];

	// wall mark shaders
	qhandle_t	wakeMarkShader;
	qhandle_t	bloodMarkShader;
	qhandle_t	bulletMarkShader;
	qhandle_t	burnMarkShader;
	qhandle_t	holeMarkShader;
	qhandle_t	blasterMarkShader;
	qhandle_t	energyMarkShader;
	qhandle_t	spreadMarkShader;

	// powerup shaders
	qhandle_t	quadShader;
	qhandle_t	quadWareOffShader;
	qhandle_t	redQuadShader;
	qhandle_t	quadWeaponShader;
	qhandle_t	invisShader;
	qhandle_t	regenShader;
	qhandle_t	battleSuitShader;
	qhandle_t	battleWeaponShader;
	qhandle_t	pentShader;
	qhandle_t	pentWeaponShader;
	qhandle_t	hastePuffShader;
#ifdef MISSIONPACK
	qhandle_t	redKamikazeShader;
	qhandle_t	blueKamikazeShader;
#endif

	// player overlays
	qhandle_t       neutralOverlay;
	qhandle_t       redOverlay;
	qhandle_t       blueOverlay;
	qhandle_t       customOverlay;
	qhandle_t       customOverlayAlpha;

	// weapon effect models
	qhandle_t	bulletFlashModel;
	qhandle_t	ringFlashModel;
	qhandle_t	dishFlashModel;
	qhandle_t	ballFlashModel; // mmp
	qhandle_t	lightningExplosionModel;

	// weapon effect shaders
	qhandle_t	blasterExplosionShader; // mmp
	qhandle_t	laserExplosionShader;
	qhandle_t	plasmaExplosionShader;
	qhandle_t	spreadExplosionShader;
	qhandle_t	bulletExplosionShader;
	qhandle_t	rocketExplosionShader;
	qhandle_t	grenadeExplosionShader;
	qhandle_t	bfgExplosionShader;
	qhandle_t	bloodExplosionShader;

	qhandle_t	flameExplosionShader; // mmp

	// special effects models
	qhandle_t	teleportEffectModel;
	qhandle_t	teleportEffectShader;
#ifdef MISSIONPACK
	qhandle_t	kamikazeEffectModel;
	qhandle_t	kamikazeShockWave;
	qhandle_t	kamikazeHeadModel;
	qhandle_t	kamikazeHeadTrail;
	qhandle_t	guardPowerupModel;
	qhandle_t	scoutPowerupModel;
	qhandle_t	doublerPowerupModel;
	qhandle_t	ammoRegenPowerupModel;
	qhandle_t	invulnerabilityImpactModel;
	qhandle_t	invulnerabilityJuicedModel;
	qhandle_t	medkitUsageModel;
	qhandle_t	dustPuffShader;
	qhandle_t	heartShader;
	qhandle_t	invulnerabilityPowerupModel;
#endif

	// scoreboard headers
	qhandle_t	scoreboardName;
	qhandle_t	scoreboardPing;
	qhandle_t	scoreboardScore;
	qhandle_t	scoreboardTime;

	// medals shown during gameplay
	qhandle_t	medalImpressive;
	qhandle_t	medalExcellent;
	qhandle_t	medalGauntlet;
	qhandle_t	medalDefend;
	qhandle_t	medalAssist;
	qhandle_t	medalCapture;

	// sounds
	sfxHandle_t	quadSound;
	sfxHandle_t	tracerSound;
	sfxHandle_t	selectSound;
	sfxHandle_t	useNothingSound;
	sfxHandle_t	wearOffSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];
	sfxHandle_t	sfx_lghit1;
	sfxHandle_t	sfx_lghit2;
	sfxHandle_t	sfx_lghit3;
	sfxHandle_t	sfx_ric1;
	sfxHandle_t	sfx_ric2;
	sfxHandle_t	sfx_ric3;
	//sfxHandle_t	sfx_railg;
	sfxHandle_t	sfx_rockexp;
	sfxHandle_t	sfx_blasterexp; // mmp
	sfxHandle_t	sfx_spreadexp; // mmp
	sfxHandle_t	sfx_plasmaexp;

	// mmp
	sfxHandle_t	sfx_exp_hi;
	sfxHandle_t	sfx_exp_lo;
	sfxHandle_t	sfx_exp_global;
	//

	sfxHandle_t	sfx_chghit;
	sfxHandle_t	sfx_chghitflesh;
	sfxHandle_t	sfx_chghitmetal;
#ifdef MISSIONPACK
	sfxHandle_t	sfx_proxexp;
	sfxHandle_t	sfx_nghit;
	sfxHandle_t	sfx_nghitflesh;
	sfxHandle_t	sfx_nghitmetal;
	sfxHandle_t kamikazeExplodeSound;
	sfxHandle_t kamikazeImplodeSound;
	sfxHandle_t kamikazeFarSound;
	sfxHandle_t useInvulnerabilitySound;
	sfxHandle_t invulnerabilityImpactSound1;
	sfxHandle_t invulnerabilityImpactSound2;
	sfxHandle_t invulnerabilityImpactSound3;
	sfxHandle_t invulnerabilityJuicedSound;
	sfxHandle_t obeliskHitSound1;
	sfxHandle_t obeliskHitSound2;
	sfxHandle_t obeliskHitSound3;
	sfxHandle_t	obeliskRespawnSound;
	sfxHandle_t	winnerSound;
	sfxHandle_t	loserSound;
#endif
	sfxHandle_t	gibSound;
	sfxHandle_t	gibBounce1Sound;
	sfxHandle_t	gibBounce2Sound;
	sfxHandle_t	gibBounce3Sound;
	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
	sfxHandle_t	noAmmoSound;
	sfxHandle_t	respawnSound;
	sfxHandle_t	superRespawnSound;
	sfxHandle_t	chatSound[NUM_CHATSOUND_TYPES]; // mmp - there are 9 chat sounds
	sfxHandle_t	clockTickSound;
//	sfxHandle_t	talkSound;
	sfxHandle_t	landSound;
	sfxHandle_t	fallSound;
	sfxHandle_t	jumpPadSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;

#ifdef MISSIONPACK
	sfxHandle_t hitSound;
#endif
	sfxHandle_t hitSound0[NUM_HITSOUND_TYPES];
	sfxHandle_t hitSound1[NUM_HITSOUND_TYPES];
	sfxHandle_t hitSound2[NUM_HITSOUND_TYPES];
	sfxHandle_t hitSound3[NUM_HITSOUND_TYPES];
	sfxHandle_t hitSound4[NUM_HITSOUND_TYPES];
	sfxHandle_t killSound[NUM_HITSOUND_TYPES];
	sfxHandle_t hitSoundHighArmor;
	sfxHandle_t hitSoundLowArmor;
	sfxHandle_t hitTeamSound;
	sfxHandle_t impressiveSound;
	sfxHandle_t excellentSound;
	sfxHandle_t deniedSound;
	sfxHandle_t humiliationSound;
	sfxHandle_t assistSound;
	sfxHandle_t defendSound;
	sfxHandle_t firstImpressiveSound;
	sfxHandle_t firstExcellentSound;
	sfxHandle_t firstHumiliationSound;

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t voteNow;
	sfxHandle_t votePassed;
	sfxHandle_t voteFailed;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t flightSound;
	sfxHandle_t medkitSound;

#ifdef MISSIONPACK
	sfxHandle_t weaponHoverSound;
#endif

	// teamplay sounds
	sfxHandle_t captureAwardSound;
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	sfxHandle_t	captureYourTeamSound;
	sfxHandle_t	captureOpponentSound;
	sfxHandle_t	returnYourTeamSound;
	sfxHandle_t	returnOpponentSound;
	sfxHandle_t	takenYourTeamSound;
	sfxHandle_t	takenOpponentSound;

	sfxHandle_t	redFlagReturnedSound;
	sfxHandle_t	blueFlagReturnedSound;
#ifdef MISSIONPACK
	sfxHandle_t	neutralFlagReturnedSound;
#endif
	sfxHandle_t	enemyTookYourFlagSound;
	sfxHandle_t	yourTeamTookEnemyFlagSound;
	sfxHandle_t	youHaveFlagSound;
#ifdef MISSIONPACK
	sfxHandle_t	enemyTookTheFlagSound;
	sfxHandle_t	yourTeamTookTheFlagSound;
	sfxHandle_t	yourBaseIsUnderAttackSound;
#endif
	sfxHandle_t	holyShitSound;

	// misc sounds
	sfxHandle_t	oneMinuteTone;
	sfxHandle_t	voteUpdate;

	sfxHandle_t	toneConnect1;
	sfxHandle_t	toneDisconnect1;
	sfxHandle_t	timelimit;
	sfxHandle_t	overtime;

	// tournament sounds
	sfxHandle_t	count10Sound;
	sfxHandle_t	count9Sound;
	sfxHandle_t	count8Sound;
	sfxHandle_t	count7Sound;
	sfxHandle_t	count6Sound;
	sfxHandle_t	count5Sound;
	sfxHandle_t	count4Sound;
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;
	sfxHandle_t	countEngageSound;

	// announcer sounds
	// TODO: combind with above
	sfxHandle_t	an_shit;
	sfxHandle_t	an_yes;
	sfxHandle_t	an_great;
	sfxHandle_t	an_ok;
	sfxHandle_t	an_excellent;
	sfxHandle_t	an_superb;
	sfxHandle_t	an_keepItUp;
	sfxHandle_t	an_wonderful;
	sfxHandle_t	an_fuckem;
	sfxHandle_t	an_youSuck;
	sfxHandle_t	an_cBreaker;
	sfxHandle_t	an_perfect;

	sfxHandle_t	countPrepareSound;

#ifdef MISSIONPACK
	// new stuff
	qhandle_t patrolShader;
	qhandle_t assaultShader;
	qhandle_t campShader;
	qhandle_t followShader;
	qhandle_t defendShader;
	qhandle_t teamLeaderShader;
	qhandle_t retrieveShader;
	qhandle_t escortShader;
	qhandle_t flagShaders[3];
	sfxHandle_t	countPrepareTeamSound;

	sfxHandle_t ammoregenSound;
	sfxHandle_t doublerSound;
	sfxHandle_t guardSound;
	sfxHandle_t scoutSound;

	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;
#endif

	sfxHandle_t	regenSound;
	sfxHandle_t	protectSound;
	sfxHandle_t	n_healthSound;
	sfxHandle_t	hgrenb1aSound;
	sfxHandle_t	hgrenb2aSound;
	sfxHandle_t	wstbimplSound;
	sfxHandle_t	wstbimpmSound;
	sfxHandle_t	wstbimpdSound;
	sfxHandle_t	wstbactvSound;

} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournament restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXOffset;		// adjustment for overscan
	float			screenYOffset;
	float			screenXBias;
	float			overscanAdj;		// if overscan adjustments are enabled

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags; // not used anymore
	int				teamflags; // not used?
	int				maxclients;

	 // game stats
	endGameStats_t	endGameStats[MAX_GAME_STATS];
	int				endGameStats_CurSlot;
	qboolean		endGameStats_Active;

	int				totalPlayTime; // total length of time match went for

	// parsed from ruleset cmd
	int				ruleSet;

	float			timelimit;
	float			fullTimelimit;
	int				overtime;
	int				scorelimit;
	int				mercylimit;
	int				physicsMode;
	int				friendlyFire;
	int				matchMode;
	int				weaponRespawn;
	int				forceRespawn;
	int				teamLocOverlay;
	int				hitSound;
	int				randomSpawn;
	int				scoreBalance;

	int				quadMode;
	int				selfDamage;
	int				doubleAmmo;
	int				keycardRespawn;
	int				keycardDropable;

	int				noArenaGrenades;
	int				noArenaLightningGun;
	int				enemyAttackLevel;
	int				powerUps;
	int				armor;
	int				popCTF;

	int				shortGame;
	int				roundBasedMatches;

	int				teamSize;

	int				serverInfoLoad;

	//
	int				overtimeSets;
	int				currentRound; // 0 = 1st round, 1 = 2nd round

	char			mapname[MAX_QPATH];
	char			mapdispname[MAX_QPATH];

	char			redTeam[MAX_QPATH];
	char			blueTeam[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];
	int				voteFlash;

	int				teamVoteTime[2];
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	qboolean		teamVoteModified[2];	// beep whenever changed
	char			teamVoteString[2][MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				redflag, blueflag;		// flag status from configstrings
	int				flagStatus;

	qboolean  newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// hud text
	// buffer width is x3 because of embedded color codes
	char			hudInfoMsgs[TOTAL_HUD_TEXT_BOXES][HUD_TEXT_HEIGHT][HUD_TEXT_BUFFER_SIZE];
	int				hudInfoMsgTimes[TOTAL_HUD_TEXT_BOXES][HUD_TEXT_HEIGHT];
	int				hudInfoPos[TOTAL_HUD_TEXT_BOXES];
	int				hudInfoLastPos[TOTAL_HUD_TEXT_BOXES];

	char			hudFragInfoKiller[HUD_TEXT_HEIGHT][HUD_FRAG_STAT_NAME_BUFFER_SIZE];
	qhandle_t		hudFragInfoDeathIcon[HUD_TEXT_HEIGHT];
	char			hudFragInfoVictom[HUD_TEXT_HEIGHT][HUD_FRAG_STAT_NAME_BUFFER_SIZE];
	int				hudFragInfoTimes[HUD_TEXT_HEIGHT];
	int				hudFragInfoPos;
	int				hudFragInfoLastPos;

	// intermission scoreboard info/chat
	char			hudBoardInfoMsgs[HUD_BOARD_TEXT_HEIGHT][HUD_TEXT_BUFFER_SIZE];
//	int				hudBoardInfoTimes[HUD_BOARD_TEXT_HEIGHT];
	int				hudBoardInfoPos;
//	int				hudBoardInfoLastPos;

	// TODO - remove teamchat, chat and notify
	// teamchat width is *3 because of embedded color codes
//	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
	char			teamChatMsgs[TEAMCHAT_HEIGHT][HUD_TEXT_BUFFER_SIZE];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;

	// chat width is *3 because of embedded color codes
	char			chatMsgs[CHAT_HEIGHT][HUD_TEXT_BUFFER_SIZE];
	int				chatMsgTimes[CHAT_HEIGHT];
	int				chatPos;
	int				lastChatPos;

	// notify width is *3 because of embedded color codes
	char			notifyMsgs[NOTIFY_HEIGHT][HUD_TEXT_BUFFER_SIZE];
	int				notifyMsgTimes[NOTIFY_HEIGHT];
	int				notifyPos;
	int				lastNotifyPos;

	// mini map information
	float			miniMapXScale;
	float			miniMapYScale;

	char			spectatorInfo[SINFO_CHAR_BUFFER_SIZE];
	char			spectatorInfoScroll[SINFO_CHAR_BUFFER_SIZE*4];

	int cursorX;
	int cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void *capturedItem;
	qhandle_t activeCursor;

	// orders
	int currentOrder;
	qboolean orderPending;
	int orderTime;
	int currentVoiceClient;
	int acceptOrderTime;
	int acceptTask;
	int acceptLeader;
	char acceptVoice[MAX_NAME_LENGTH];

	// media
	cgMedia_t		media;

//unlagged - client options
	// this will be set to the server's g_delagHitscan
	int				delagHitscan;
//unlagged - client options

	fileHandle_t	chatLogFile; // mmp

} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t		cg_weapons[MAX_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_gibs;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
extern	vmCvar_t		cg_drawAmmoWarning;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_drawTeamOverlay;
extern	vmCvar_t		cg_teamOverlayUserinfo;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_animSpeed;
extern	vmCvar_t		cg_debugAnim;
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_railTrailTime;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_footsteps;
extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_brassTime;
extern	vmCvar_t		cg_gun_frame;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_tracerChance;
extern	vmCvar_t		cg_tracerWidth;
extern	vmCvar_t		cg_tracerLength;
extern	vmCvar_t		cg_autoswitch;
extern	vmCvar_t		cg_switchOnEmpty;
extern	vmCvar_t		cg_switchToEmpty;
extern	vmCvar_t		cg_powerupOverlay;
extern	vmCvar_t		cg_itemPickUpOverlay;
extern	vmCvar_t		cg_intermissionEffect;
extern	vmCvar_t		cg_ignore;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_zoomFov;
extern	vmCvar_t		cg_fovMode;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_drawAttacker;
extern	vmCvar_t		cg_synchronousClients;
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
extern	vmCvar_t		cg_stats;
extern	vmCvar_t 		cg_forceModel;
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_blood;
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_deferPlayers;
extern	vmCvar_t		cg_drawFriend;
extern	vmCvar_t		cg_teamChatsOnly;
#ifdef MISSIONPACK
extern	vmCvar_t		cg_noVoiceChats;
extern	vmCvar_t		cg_noVoiceText;
#endif
extern  vmCvar_t		cg_scorePlum;
extern  vmCvar_t		cg_damagePlum;
//unlagged - smooth clients #2
// this is done server-side now
//extern	vmCvar_t		cg_smoothClients;
//unlagged - smooth clients #2
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
//extern	vmCvar_t		cg_pmove_fixed;
extern	vmCvar_t		cg_cameraOrbit;
extern	vmCvar_t		cg_cameraOrbitDelay;
extern	vmCvar_t		cg_timescaleFadeEnd;
extern	vmCvar_t		cg_timescaleFadeSpeed;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_cameraMode;
extern  vmCvar_t		cg_smallFont;
extern  vmCvar_t		cg_bigFont;
extern	vmCvar_t		cg_noTaunt;
extern	vmCvar_t		cg_noProjectileTrail;
extern	vmCvar_t		cg_oldRail;
extern	vmCvar_t		cg_oldRocket;
extern	vmCvar_t		cg_oldPlasma;
extern	vmCvar_t		cg_damageKick;
extern	vmCvar_t		cg_chatSound;
extern	vmCvar_t		cg_teamChatSound;
extern	vmCvar_t		cg_playerTaunt;

extern	vmCvar_t		cg_trueLightning;

extern	vmCvar_t		cg_hitSound;
extern	vmCvar_t		cg_killSound;
extern	vmCvar_t		cg_teamKillSound;
extern	vmCvar_t		cg_lumOverlay;
extern	vmCvar_t		cg_connectionTone;

extern	vmCvar_t		cg_weaponCycleSkipsBlaster;
extern	vmCvar_t		cg_placebo;
extern	vmCvar_t		cg_weaponBobbing;


// hud
extern	vmCvar_t		hud_scoreboard_pingType;
extern	vmCvar_t		hud_scoreboard_barType;

extern	vmCvar_t		hud_keys_show; // was hud_showKeys
extern	vmCvar_t		hud_keys_align;
extern	vmCvar_t		hud_keys_xPos;
extern	vmCvar_t		hud_keys_xAlign;
extern	vmCvar_t		hud_keys_yPos;
extern	vmCvar_t		hud_keys_scale;
extern	vmCvar_t		hud_keys_color;
extern	vmCvar_t		hud_keys_pressColor;

extern	vmCvar_t		hud_testStatus_show;

extern	vmCvar_t		hud_crosshair_innerColor;
extern	vmCvar_t		hud_crosshair_innerType;
extern	vmCvar_t		hud_crosshair_outsideColor;
extern	vmCvar_t		hud_crosshair_outsideType;

extern	vmCvar_t		hud_fps_show;
extern	vmCvar_t		hud_fps_align;
extern	vmCvar_t		hud_fps_style;
extern	vmCvar_t		hud_fps_posLock;
extern	vmCvar_t		hud_fps_xPos;
extern	vmCvar_t		hud_fps_xAlign;
extern	vmCvar_t		hud_fps_yPos;
extern	vmCvar_t		hud_fps_xScale;
extern	vmCvar_t		hud_fps_yScale;
extern	vmCvar_t		hud_fps_color;

extern	vmCvar_t		hud_gameClock_show;

extern	vmCvar_t		hud_digitalClock_show;
extern	vmCvar_t		hud_digitalClock_align;
extern	vmCvar_t		hud_digitalClock_style;
extern	vmCvar_t		hud_digitalClock_posLock;
extern	vmCvar_t		hud_digitalClock_xPos;
extern	vmCvar_t		hud_digitalClock_xAlign;
extern	vmCvar_t		hud_digitalClock_yPos;
extern	vmCvar_t		hud_digitalClock_xScale;
extern	vmCvar_t		hud_digitalClock_yScale;
extern	vmCvar_t		hud_digitalClock_color;

extern	vmCvar_t		hud_ups_show;
extern	vmCvar_t		hud_ups_align;
extern	vmCvar_t		hud_ups_style;
extern	vmCvar_t		hud_ups_posLock;
extern	vmCvar_t		hud_ups_xPos;
extern	vmCvar_t		hud_ups_xAlign;
extern	vmCvar_t		hud_ups_yPos;
extern	vmCvar_t		hud_ups_xScale;
extern	vmCvar_t		hud_ups_yScale;
extern	vmCvar_t		hud_ups_color;

extern	vmCvar_t		hud_briefScore_show;
extern	vmCvar_t		hud_briefScore_align;
extern	vmCvar_t		hud_briefScore_style;
extern	vmCvar_t		hud_briefScore_posLock;
extern	vmCvar_t		hud_briefScore_xPos;
extern	vmCvar_t		hud_briefScore_xAlign;
extern	vmCvar_t		hud_briefScore_yPos;
extern	vmCvar_t		hud_briefScore_xScale;
extern	vmCvar_t		hud_briefScore_yScale;
/*extern	vmCvar_t		hud_briefScore_colorTeam;
extern	vmCvar_t		hud_briefScore_colorEnemy;
extern	vmCvar_t		hud_briefScore_colorRed;
extern	vmCvar_t		hud_briefScore_colorBlue;
extern	vmCvar_t		hud_briefScore_colorNum;*/

extern	vmCvar_t		hud_matchInfo_show;
extern	vmCvar_t		hud_matchInfo_align;
extern	vmCvar_t		hud_matchInfo_style;
extern	vmCvar_t		hud_matchInfo_posLock;
extern	vmCvar_t		hud_matchInfo_xPos;
extern	vmCvar_t		hud_matchInfo_xAlign;
extern	vmCvar_t		hud_matchInfo_yPos;
extern	vmCvar_t		hud_matchInfo_xScale;
extern	vmCvar_t		hud_matchInfo_yScale;
extern	vmCvar_t		hud_matchInfo_color;

extern	vmCvar_t		hud_gameMap_show;
extern	vmCvar_t		hud_gameMap_align;
extern	vmCvar_t		hud_gameMap_posLock;
extern	vmCvar_t		hud_gameMap_xPos;
extern	vmCvar_t		hud_gameMap_xAlign;
extern	vmCvar_t		hud_gameMap_yPos;
extern	vmCvar_t		hud_gameMap_xScale;
extern	vmCvar_t		hud_gameMap_yScale;
extern	vmCvar_t		hud_gameMap_colorBG;
extern	vmCvar_t		hud_gameMap_colorPlayer;
extern	vmCvar_t		hud_gameMap_colorTeam;

extern	vmCvar_t		hud_statusHealth_show;
extern	vmCvar_t		hud_statusHealth_showZero;
extern	vmCvar_t		hud_statusHealth_colorType;
extern	vmCvar_t		hud_statusHealth_align;
extern	vmCvar_t		hud_statusHealth_style;
extern	vmCvar_t		hud_statusHealth_posLock;
extern	vmCvar_t		hud_statusHealth_xPos;
extern	vmCvar_t		hud_statusHealth_xAlign;
extern	vmCvar_t		hud_statusHealth_yPos;
extern	vmCvar_t		hud_statusHealth_xScale;
extern	vmCvar_t		hud_statusHealth_yScale;
extern	vmCvar_t		hud_statusHealth_colorNormal;
extern	vmCvar_t		hud_statusHealth_colorHigh;
extern	vmCvar_t		hud_statusHealth_colorLow;
extern	vmCvar_t		hud_statusHealth_colorLowFlash;
extern	vmCvar_t		hud_statusHealth_colorNormalRed;
extern	vmCvar_t		hud_statusHealth_colorHighRed;
extern	vmCvar_t		hud_statusHealth_colorLowRed;
extern	vmCvar_t		hud_statusHealth_colorLowFlashRed;
extern	vmCvar_t		hud_statusHealth_colorNormalBlue;
extern	vmCvar_t		hud_statusHealth_colorHighBlue;
extern	vmCvar_t		hud_statusHealth_colorLowBlue;
extern	vmCvar_t		hud_statusHealth_colorLowFlashBlue;

extern	vmCvar_t		hud_statusHealthIcon_show;
extern	vmCvar_t		hud_statusHealthIcon_showZero;
extern	vmCvar_t		hud_statusHealthIcon_colorType;
extern	vmCvar_t		hud_statusHealthIcon_align;
extern	vmCvar_t		hud_statusHealthIcon_style;
extern	vmCvar_t		hud_statusHealthIcon_posLock;
extern	vmCvar_t		hud_statusHealthIcon_xPos;
extern	vmCvar_t		hud_statusHealthIcon_xAlign;
extern	vmCvar_t		hud_statusHealthIcon_yPos;
extern	vmCvar_t		hud_statusHealthIcon_xScale;
extern	vmCvar_t		hud_statusHealthIcon_yScale;
extern	vmCvar_t		hud_statusHealthIcon_colorNormal;
extern	vmCvar_t		hud_statusHealthIcon_colorHigh;
extern	vmCvar_t		hud_statusHealthIcon_colorLow;
extern	vmCvar_t		hud_statusHealthIcon_colorLowFlash;
extern	vmCvar_t		hud_statusHealthIcon_colorNormalRed;
extern	vmCvar_t		hud_statusHealthIcon_colorHighRed;
extern	vmCvar_t		hud_statusHealthIcon_colorLowRed;
extern	vmCvar_t		hud_statusHealthIcon_colorLowFlashRed;
extern	vmCvar_t		hud_statusHealthIcon_colorNormalBlue;
extern	vmCvar_t		hud_statusHealthIcon_colorHighBlue;
extern	vmCvar_t		hud_statusHealthIcon_colorLowBlue;
extern	vmCvar_t		hud_statusHealthIcon_colorLowFlashBlue;

extern	vmCvar_t		hud_statusArmor_show;
extern	vmCvar_t		hud_statusArmor_showZero;
extern	vmCvar_t		hud_statusArmor_colorBreakPoint;
extern	vmCvar_t		hud_statusArmor_colorType;
extern	vmCvar_t		hud_statusArmor_align;
extern	vmCvar_t		hud_statusArmor_style;
extern	vmCvar_t		hud_statusArmor_posLock;
extern	vmCvar_t		hud_statusArmor_xPos;
extern	vmCvar_t		hud_statusArmor_xAlign;
extern	vmCvar_t		hud_statusArmor_yPos;
extern	vmCvar_t		hud_statusArmor_xScale;
extern	vmCvar_t		hud_statusArmor_yScale;
extern	vmCvar_t		hud_statusArmor_color;
extern	vmCvar_t		hud_statusArmor_colorBreak;
extern	vmCvar_t		hud_statusArmor_colorRed;
extern	vmCvar_t		hud_statusArmor_colorRedBreak;
extern	vmCvar_t		hud_statusArmor_colorBlue;
extern	vmCvar_t		hud_statusArmor_colorBlueBreak;
extern	vmCvar_t		hud_statusArmor_colorTier1;
extern	vmCvar_t		hud_statusArmor_colorTier1Break;
extern	vmCvar_t		hud_statusArmor_colorTier2;
extern	vmCvar_t		hud_statusArmor_colorTier2Break;
extern	vmCvar_t		hud_statusArmor_colorTier3;
extern	vmCvar_t		hud_statusArmor_colorTier3Break;


extern	vmCvar_t		hud_statusArmorIcon_show;
extern	vmCvar_t		hud_statusArmorIcon_showZero;
extern	vmCvar_t		hud_statusArmorIcon_colorType;
extern	vmCvar_t		hud_statusArmorIcon_align;
extern	vmCvar_t		hud_statusArmorIcon_style;
extern	vmCvar_t		hud_statusArmorIcon_posLock;
extern	vmCvar_t		hud_statusArmorIcon_xPos;
extern	vmCvar_t		hud_statusArmorIcon_xAlign;
extern	vmCvar_t		hud_statusArmorIcon_yPos;
extern	vmCvar_t		hud_statusArmorIcon_xScale;
extern	vmCvar_t		hud_statusArmorIcon_yScale;
extern	vmCvar_t		hud_statusArmorIcon_color;
extern	vmCvar_t		hud_statusArmorIcon_colorRed;
extern	vmCvar_t		hud_statusArmorIcon_colorBlue;
extern	vmCvar_t		hud_statusArmorIcon_colorTier1;
extern	vmCvar_t		hud_statusArmorIcon_colorTier2;
extern	vmCvar_t		hud_statusArmorIcon_colorTier3;

extern	vmCvar_t		hud_statusArmorTier_show;
extern	vmCvar_t		hud_statusArmorTier_showZero;
extern	vmCvar_t		hud_statusArmorTier_colorType;
extern	vmCvar_t		hud_statusArmorTier_type;
extern	vmCvar_t		hud_statusArmorTier_align;
extern	vmCvar_t		hud_statusArmorTier_style;
extern	vmCvar_t		hud_statusArmorTier_posLock;
extern	vmCvar_t		hud_statusArmorTier_xPos;
extern	vmCvar_t		hud_statusArmorTier_xAlign;
extern	vmCvar_t		hud_statusArmorTier_yPos;
extern	vmCvar_t		hud_statusArmorTier_xScale;
extern	vmCvar_t		hud_statusArmorTier_yScale;
extern	vmCvar_t		hud_statusArmorTier_color;
extern	vmCvar_t		hud_statusArmorTier_colorRed;
extern	vmCvar_t		hud_statusArmorTier_colorBlue;
extern	vmCvar_t		hud_statusArmorTier_colorTier1;
extern	vmCvar_t		hud_statusArmorTier_colorTier2;
extern	vmCvar_t		hud_statusArmorTier_colorTier3;

extern	vmCvar_t		hud_statusLevel_show;
extern	vmCvar_t		hud_statusLevel_colorType;
extern	vmCvar_t		hud_statusLevel_type;
extern	vmCvar_t		hud_statusLevel_align;
extern	vmCvar_t		hud_statusLevel_style;
extern	vmCvar_t		hud_statusLevel_posLock;
extern	vmCvar_t		hud_statusLevel_xPos;
extern	vmCvar_t		hud_statusLevel_xAlign;
extern	vmCvar_t		hud_statusLevel_yPos;
extern	vmCvar_t		hud_statusLevel_xScale;
extern	vmCvar_t		hud_statusLevel_yScale;
extern	vmCvar_t		hud_statusLevel_color;
extern	vmCvar_t		hud_statusLevel_colorRed;
extern	vmCvar_t		hud_statusLevel_colorBlue;

extern	vmCvar_t		hud_statusPhysics_show;
extern	vmCvar_t		hud_statusPhysics_colorType;
extern	vmCvar_t		hud_statusPhysics_type;
extern	vmCvar_t		hud_statusPhysics_align;
extern	vmCvar_t		hud_statusPhysics_style;
extern	vmCvar_t		hud_statusPhysics_posLock;
extern	vmCvar_t		hud_statusPhysics_xPos;
extern	vmCvar_t		hud_statusPhysics_xAlign;
extern	vmCvar_t		hud_statusPhysics_yPos;
extern	vmCvar_t		hud_statusPhysics_xScale;
extern	vmCvar_t		hud_statusPhysics_yScale;
extern	vmCvar_t		hud_statusPhysics_color;
extern	vmCvar_t		hud_statusPhysics_colorRed;
extern	vmCvar_t		hud_statusPhysics_colorBlue;

extern	vmCvar_t		hud_statusKeycards_show;
extern	vmCvar_t		hud_statusKeycards_align;
extern	vmCvar_t		hud_statusKeycards_style;
extern	vmCvar_t		hud_statusKeycards_posLock;
extern	vmCvar_t		hud_statusKeycards_xPos;
extern	vmCvar_t		hud_statusKeycards_xAlign;
extern	vmCvar_t		hud_statusKeycards_yPos;
extern	vmCvar_t		hud_statusKeycards_xScale;
extern	vmCvar_t		hud_statusKeycards_yScale;

extern	vmCvar_t		hud_statusHoldable_show;
extern	vmCvar_t		hud_statusHoldable_align;
extern	vmCvar_t		hud_statusHoldable_style;
extern	vmCvar_t		hud_statusHoldable_posLock;
extern	vmCvar_t		hud_statusHoldable_xPos;
extern	vmCvar_t		hud_statusHoldable_xAlign;
extern	vmCvar_t		hud_statusHoldable_yPos;
extern	vmCvar_t		hud_statusHoldable_xScale;
extern	vmCvar_t		hud_statusHoldable_yScale;

extern	vmCvar_t		hud_statusItem_show;
extern	vmCvar_t		hud_statusItem_align;
extern	vmCvar_t		hud_statusItem_style;
extern	vmCvar_t		hud_statusItem_posLock;
extern	vmCvar_t		hud_statusItem_xPos;
extern	vmCvar_t		hud_statusItem_xAlign;
extern	vmCvar_t		hud_statusItem_yPos;
extern	vmCvar_t		hud_statusItem_xScale;
extern	vmCvar_t		hud_statusItem_yScale;

extern	vmCvar_t		hud_weaponBar_show;
extern	vmCvar_t		hud_weaponBar_align;
extern	vmCvar_t		hud_weaponBar_style;
extern	vmCvar_t		hud_weaponBar_angle;
//extern	vmCvar_t		hud_weaponBar_posLock;
extern	vmCvar_t		hud_weaponBar_xPos;
extern	vmCvar_t		hud_weaponBar_xAlign;
extern	vmCvar_t		hud_weaponBar_yPos;
//extern	vmCvar_t		hud_weaponBar_xScale;
//extern	vmCvar_t		hud_weaponBar_yScale;
extern	vmCvar_t		hud_weaponBar_colorNormal;
extern	vmCvar_t		hud_weaponBar_colorLow;
extern	vmCvar_t		hud_weaponBar_colorDead;
//extern	vmCvar_t		hud_weaponBar_colorText;
//extern	vmCvar_t		hud_weaponBar_colorTextDead;
extern	vmCvar_t		hud_weaponBar_colorFlash1;
extern	vmCvar_t		hud_weaponBar_colorFlash2;

extern	vmCvar_t		hud_ammoBar_show;
extern	vmCvar_t		hud_ammoBar_align;
extern	vmCvar_t		hud_ammoBar_style;
extern	vmCvar_t		hud_ammoBar_angle;
extern	vmCvar_t		hud_ammoBar_xPos;
extern	vmCvar_t		hud_ammoBar_xAlign;
extern	vmCvar_t		hud_ammoBar_yPos;
extern	vmCvar_t		hud_ammoBar_colorNormal;
extern	vmCvar_t		hud_ammoBar_colorLow;
extern	vmCvar_t		hud_ammoBar_colorDead;
extern	vmCvar_t		hud_ammoBar_colorText;
extern	vmCvar_t		hud_ammoBar_colorTextDead;
extern	vmCvar_t		hud_ammoBar_colorFlash1;
extern	vmCvar_t		hud_ammoBar_colorFlash2;

extern	vmCvar_t		hud_voteStatus_show;
extern	vmCvar_t		hud_voteStatus_align;
extern	vmCvar_t		hud_voteStatus_style;
extern	vmCvar_t		hud_voteStatus_posLock;
extern	vmCvar_t		hud_voteStatus_xPos;
extern	vmCvar_t		hud_voteStatus_xAlign;
extern	vmCvar_t		hud_voteStatus_yPos;
extern	vmCvar_t		hud_voteStatus_xScale;
extern	vmCvar_t		hud_voteStatus_yScale;

extern	vmCvar_t		hud_teamOverlay_show;
extern	vmCvar_t		hud_teamOverlay_align;
extern	vmCvar_t		hud_teamOverlay_xPos;
extern	vmCvar_t		hud_teamOverlay_xAlign;
extern	vmCvar_t		hud_teamOverlay_yPos;
extern	vmCvar_t		hud_teamOverlay_scale;

extern	vmCvar_t		hud_chatBoxRoute;
extern	vmCvar_t		hud_teamChatBoxRoute;
extern	vmCvar_t		hud_notifyBoxRoute;
extern	vmCvar_t		hud_notifyBoxFilter;

extern	vmCvar_t		hud_infoBox0_show;
extern	vmCvar_t		hud_infoBox0_xPos;
extern	vmCvar_t		hud_infoBox0_xAlign;
extern	vmCvar_t		hud_infoBox0_yPos;
extern	vmCvar_t		hud_infoBox0_lines;
extern	vmCvar_t		hud_infoBox0_dir;
extern	vmCvar_t		hud_infoBox0_scale;

extern	vmCvar_t		hud_infoBox1_show;
extern	vmCvar_t		hud_infoBox1_xPos;
extern	vmCvar_t		hud_infoBox1_xAlign;
extern	vmCvar_t		hud_infoBox1_yPos;
extern	vmCvar_t		hud_infoBox1_lines;
extern	vmCvar_t		hud_infoBox1_dir;
extern	vmCvar_t		hud_infoBox1_scale;

extern	vmCvar_t		hud_infoBox2_show;
extern	vmCvar_t		hud_infoBox2_xPos;
extern	vmCvar_t		hud_infoBox2_xAlign;
extern	vmCvar_t		hud_infoBox2_yPos;
extern	vmCvar_t		hud_infoBox2_lines;
extern	vmCvar_t		hud_infoBox2_dir;
extern	vmCvar_t		hud_infoBox2_scale;

extern	vmCvar_t		hud_infoBox3_show;
extern	vmCvar_t		hud_infoBox3_xPos;
extern	vmCvar_t		hud_infoBox3_xAlign;
extern	vmCvar_t		hud_infoBox3_yPos;
extern	vmCvar_t		hud_infoBox3_lines;
extern	vmCvar_t		hud_infoBox3_dir;
extern	vmCvar_t		hud_infoBox3_scale;

extern	vmCvar_t		hud_fragInfo_show;
extern	vmCvar_t		hud_fragInfo_xPos;
extern	vmCvar_t		hud_fragInfo_xAlign;
extern	vmCvar_t		hud_fragInfo_yPos;
extern	vmCvar_t		hud_fragInfo_lines;
extern	vmCvar_t		hud_fragInfo_dir;
extern	vmCvar_t		hud_fragInfo_align;
extern	vmCvar_t		hud_fragInfo_scale;

extern	vmCvar_t		hud_pickUpInfo_show;
extern	vmCvar_t		hud_pickUpInfo_xPos;
extern	vmCvar_t		hud_pickUpInfo_xAlign;
extern	vmCvar_t		hud_pickUpInfo_yPos;
extern	vmCvar_t		hud_pickUpInfo_align;
extern	vmCvar_t		hud_pickUpInfo_style;
extern	vmCvar_t		hud_pickUpInfo_time;
extern	vmCvar_t		hud_pickUpInfo_scale;

extern	vmCvar_t		hud_filterColors;
extern	vmCvar_t		hud_aspectRatioScale;
extern	vmCvar_t		hud_autoRatioScale;
extern	vmCvar_t		hud_overscanAdjust;

extern	vmCvar_t		hud_iUnderstand;
extern	vmCvar_t		hud_showRulesetInFreeFloat;

extern	vmCvar_t		hud_useEmoticons;
extern	vmCvar_t		hud_hqFontThreshold;

extern	vmCvar_t		hud_customHUD;


#ifdef MISSIONPACK
extern	vmCvar_t		cg_redTeamName;
extern	vmCvar_t		cg_blueTeamName;
extern	vmCvar_t		cg_currentSelectedPlayer;
extern	vmCvar_t		cg_currentSelectedPlayerName;
extern	vmCvar_t		cg_singlePlayer;
extern	vmCvar_t		cg_enableDust;
extern	vmCvar_t		cg_enableBreath;
extern	vmCvar_t		cg_singlePlayerActive;
extern  vmCvar_t		cg_recordSPDemo;
extern  vmCvar_t		cg_recordSPDemoName;
extern	vmCvar_t		cg_obeliskRespawnDelay;
#endif

//unlagged - client options
extern	vmCvar_t		cg_delag;
//extern	vmCvar_t		cg_debugDelag;
//extern	vmCvar_t		cg_drawBBox;
extern	vmCvar_t		cg_cmdTimeNudge;
extern	vmCvar_t		sv_fps;
extern	vmCvar_t		cg_projectileNudge;
extern	vmCvar_t		cg_optimizePrediction;
extern	vmCvar_t		cl_timeNudge;
//extern	vmCvar_t		cg_latentSnaps;
//extern	vmCvar_t		cg_latentCmds;
//extern	vmCvar_t		cg_plOut;
//unlagged - client options

//unlagged - cg_unlagged.c
void CG_PredictWeaponEffects( centity_t *cent );
//void CG_AddBoundingBox( centity_t *cent );
qboolean CG_Cvar_ClampInt( const char *name, vmCvar_t *vmCvar, int min, int max );
//unlagged - cg_unlagged.c

//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL CG_Error( const char *msg, ... ) __attribute__ ((noreturn, format (printf, 1, 2)));

void CG_StartMusic( void );
void CG_EndMatchMusic( void );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );
void CG_LoadMenus(const char *menuFile);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
void CG_RankRunFrame( void );
void CG_SetScoreSelection(void *menu);
score_t *CG_GetSelectedScore( void );
void CG_BuildSpectatorString( void );
char CG_AddEmoticons( char *str );

//unlagged, sagos modfication
void SnapVectorTowards( vec3_t v, vec3_t to );

//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );
void CG_AddBufferedSound( sfxHandle_t sfx);

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawPicExt( float x, float y, float width, float height, float tx1, float ty1, float tx2, float ty2, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string,
				   float charWidth, float charHeight, const float *modulate );


void CG_DrawStringExt( int x, int y, const char *string, const float *setColor,
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars );
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );

int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);

//
// cg_megadraw.c
//
/*void CG_DrawTest( int xpos, int ypos, int style, float scale, vec4_t color );
static CG_MegaDrawFPS( int xpos, int ypos, int style, float scale, vec4_t color );
static CG_MegaDrawTimer( int xpos, int ypos, int style, float scale, vec4_t color );
static CG_MegaDrawDigitTimer( int xpos, int ypos, int style, float scale, vec4_t color );
static CG_MegaDrawUPS( int xpos, int ypos, int style, float scale, vec4_t color );
static CG_MegaDrawBriefScore( int xpos, int ypos, int layout, int style, float scale, vec4_t color, vec4_t color1, vec4_t color2 );
static void CG_MegaDrawHUDInfo( int xpos, int ypos, int xsize, int ysize, int ydir, int hudBox, int style, float scale, vec4_t hcolor );
static void CG_MegaDrawChat( int xpos, int ypos, int xsize, int ysize, int ydir, int style, float scale, vec4_t hcolor );
static void CG_MegaDrawCrosshair( float xpos, float ypos, float xsize, float ysize, int typeA, int typeB, vec4_t colorA, vec4_t colorB );
static void CG_MegaDrawHudPart( float xpos, float ypos, float xsize, float ysize, qhandle_t hShader, int fcol, int frow, int fcol2, int frow2, vec4_t color);
static void CG_MegaDrawBorder( float xpos, float ypos, float xsize, float ysize, int ptype, vec4_t color);*/


//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;
extern  char systemChat[256];
extern  char teamChat1[256];
extern  char teamChat2[256];

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style);
int CG_Text_Width(const char *text, float scale, int limit);
int CG_Text_Height(const char *text, float scale, int limit);
void CG_SelectPrevPlayer( void );
void CG_SelectNextPlayer( void );
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_ShowResponseHead( void );
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat( void );
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText( void );
const char *CG_GetKillerText( void );
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles);
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
void CG_CheckOrderPending( void );
const char *CG_GameTypeString( void );
qboolean CG_YourTeamHasFlag( void );
qboolean CG_OtherTeamHasFlag( void );
qhandle_t CG_StatusHandle(int task);



//
// cg_player.c
//
void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team, qboolean isMissile );
void CG_NewClientInfo( int clientNum );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
void CG_PredictPlayerState( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
//static void CG_AddToFragInfo( int hudBox, const char *str );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );


//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out, vec3_t angles_in, vec3_t angles_out);

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );



//
// cg_weapons.c
//
void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );

void CG_RegisterWeapon( int weaponNum );
void CG_RegisterItemVisuals( int itemNum );

void CG_FireWeapon( centity_t *cent );
void CG_RemoveWeapon( void );
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType );
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum );
void CG_ShotgunFire( entityState_t *es );
void CG_SuperShotgunFire( entityState_t *es );
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );

void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end );
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team );
void CG_DrawWeaponSelect( void );

void CG_OutOfAmmoChange( void );	// should this be in pmove?

//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader,
				    const vec3_t origin, const vec3_t dir,
					float orientation,
				    float r, float g, float b, float a,
					qboolean alphaFade,
					float radius, qboolean temporary );

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p,
				   const vec3_t vel,
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader );
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
void CG_SpawnEffect( vec3_t org );
#ifdef MISSIONPACK
void CG_KamikazeEffect( vec3_t org );
void CG_ObeliskExplode( vec3_t org, int entityNum );
void CG_ObeliskPain( vec3_t org );
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles );
void CG_InvulnerabilityJuiced( vec3_t org );
void CG_LightningBoltBeam( vec3_t start, vec3_t end );
#endif
void CG_ScorePlum( int client, vec3_t org, int score );

void CG_GibPlayer( vec3_t playerOrigin );
void CG_BigExplode( vec3_t playerOrigin );

void CG_Bleed( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader, int msec,
								qboolean isSprite );

void CG_Lightning_Discharge (vec3_t origin, int msec);  // The SARACEN's Lightning Discharge

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );
//unlagged - early transitioning
//void CG_TransitionEntity( centity_t *cent );
//unlagged - early transitioning

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingItem( int itemNum );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );
void CG_DrawOldTourneyScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );
void CG_ShaderStateChanged(void);
void CG_AddToHUDInfo( int hudBox, const char *str, int emoticon, int hideInfo );
#ifdef MISSIONPACK
void CG_LoadVoiceChats( void );
void CG_VoiceChatLocal( int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd );
void CG_PlayBufferedVoiceChats( void );
#endif

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );


//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void		trap_Print( const char *fmt );

// abort the game
void		trap_Error(const char *fmt) __attribute__((noreturn));

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds( void );

// console variable interaction
void		trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void		trap_Cvar_Update( vmCvar_t *vmCvar );
void		trap_Cvar_Set( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// ServerCommand and ConsoleCommand parameter access
int			trap_Argc( void );
void		trap_Argv( int n, char *buffer, int bufferLength );
void		trap_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile( fileHandle_t f );
int			trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );
void		trap_RemoveCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen( void );

// model collision
void		trap_CM_LoadMap( const char *mapname );
int			trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
int			trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int			trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );
void		trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int			trap_CM_MarkFragments( int numPoints, const vec3_t *points,
			const vec3_t projection,
			int maxPoints, vec3_t pointBuffer,
			int maxFragments, markFragment_t *fragmentBuffer );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void		trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
void		trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void		trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
void		trap_S_ClearLoopingSounds( qboolean killall );
void		trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// respatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
void		trap_S_StartBackgroundTrack( const char *intro, const char *loop );	// empty name stops music
void	trap_S_StopBackgroundTrack( void );


void		trap_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	trap_R_RegisterModel( const char *name );			// returns rgb axis if not found
qhandle_t	trap_R_RegisterSkin( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShader( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShaderNoMip( const char *name );			// returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene( void );
void		trap_R_AddRefEntityToScene( const refEntity_t *re );

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
void		trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void		trap_R_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
int			trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
void		trap_R_RenderScene( const refdef_t *fd );
void		trap_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic( float x, float y, float w, float h,
			float s1, float t1, float s2, float t2, qhandle_t hShader );
void		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
int			trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame,
					   float frac, const char *tagName );
void		trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );
qboolean	trap_R_inPVS( const vec3_t p1, const vec3_t p2 );

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
void		trap_SetUserCmdValue( int stateValue, float sensitivityScale, int flags );

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

int			trap_MemoryRemaining( void );
void		trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
qboolean	trap_Key_IsDown( int keynum );
int			trap_Key_GetCatcher( void );
void		trap_Key_SetCatcher( int catcher );
int			trap_Key_GetKey( const char *binding );


typedef enum {
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
} q3print_t;


int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic (int handle);
void trap_CIN_DrawCinematic (int handle);
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h);

int			trap_RealTime(qtime_t *qtime);
void		trap_SnapVector( float *v );

qboolean	trap_loadCamera(const char *name);
void		trap_startCamera(int time);
qboolean	trap_getCameraInfo(int time, vec3_t *origin, vec3_t *angles);

qboolean	trap_GetEntityToken( char *buffer, int bufferSize );

void	CG_ClearParticles (void);
void	CG_AddParticles (void);
void	CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void	CG_ParticleSmoke (qhandle_t pshader, centity_t *cent);
void	CG_AddParticleShrapnel (localEntity_t *le);
void	CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent);
void	CG_ParticleBulletDebris (vec3_t	org, vec3_t vel, int duration);
void	CG_ParticleSparks (vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void	CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir);
void	CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void	CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
extern qboolean		initparticles;
int CG_NewParticleArea ( int num );


