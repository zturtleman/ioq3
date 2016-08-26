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
// g_local.h -- local definitions for game module

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "g_public.h"

//==================================================================

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	BASEGAME

#define BODY_QUEUE_SIZE		8

#define INFINITE				1000000

#define	FRAMETIME				100					// msec
#define	CARNAGE_REWARD_TIME		3000
#define REWARD_SPRITE_TIME		2000

#define	INTERMISSION_DELAY_TIME	1000
#define	SP_INTERMISSION_DELAY_TIME	5000
#define	FORCED_TIMELIMIT		100 // 100 minutes

#define	SPECTATOR_MAX_SPEED		800

#define FLAG_RETURN_TIME		30000 // 30 seconds
#define DEC_FLAG_RETURN_TIME	FLAG_RETURN_TIME - 5000
#define KEYCARD_DESPAWN_TIME	30000 // 30 seconds, was 15
#define BACKPACK_DESPAWN_TIME	120000 // 2 minutes
#define DEFAULT_DESPAWN_TIME	20000 // 20 seconds, was 30

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define FL_NO_BOTS				0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS			0x00004000	// spawn point just for bots
#define FL_FORCE_GESTURE		0x00008000	// force gesture on client

#define	MAX_CLIENT_STATS_SLOT	64 // if we ever go past this amount of slots, then double it i say

// misc
#define FORCE_NAME_0			"\x6B\x73\x75"
#define FORCE_NAME_1			"\x43\x6F\x63"
#define FORCE_NAME_2			"\x72"
#define FORCE_NAME_3			"\x63\x6B\x65"

// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

// random select types
typedef enum {
	RT_COINTOSS,
	RT_RNDSELECT,
	RT_RNDNUM,
	RT_MAXTYPE
} randomSelect_t;

// select type of player to follow
typedef enum {
	FTYPE_NOTHING,
	FTYPE_POWERUP,
	FTYPE_OBJECTIVE,
	FTYPE_MAXTYPE
} followType_t;

#define SP_PODIUM_MODEL		"models/mapobjects/podium/podium4.md3"

//============================================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

struct gentity_s {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s	*client;			// NULL if not a client

	qboolean	inuse;

	char		*classname;			// set in QuakeEd
	int			spawnflags;			// set in QuakeEd

	qboolean	neverFree;			// if true, FreeEntity will only unlink
									// bodyque uses this

	int			flags;				// FL_* variables

	char		*model;
	char		*model2;
	int			freetime;			// level.time when the object was freed

	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;

	qboolean	physicsObject;		// if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects,
	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

	// movers
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	gentity_t	*parent;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	vec3_t		pos1, pos2;

	char		*message;

	int			timestamp;			// body queue sinking, etc
	int			timeExt;			// temp-use time function

	char		*target;
	char		*targetname;
	char		*team;
	char		*targetShaderName;
	char		*targetShaderNewName;
	gentity_t	*target_ent;

	float		speed;
	vec3_t		movedir;

	int			nextthink;
	void		(*think)(gentity_t *self);
	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
	void		(*blocked)(gentity_t *self, gentity_t *other);
	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage);
	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

	int			pain_debounce_time;
	int			fly_sound_debounce_time;	// wind tunnel
	int			last_move_time;

	int			health;

	qboolean	takedamage;
	qboolean	ignoresplash; // ignores splash damage, meant for buttons and such
	qboolean	globalExpSound; // global explosion sound (rockets and grenades)
	int			keycard; // mmp - for use with keycards
	int			comboflags; // mmp - "combo flag", mainly for multiple buttons to press to open a door

	int			damage;
	int			splashDamage; // quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;

	int			count;

	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team

#ifdef MISSIONPACK
	int			kamikazeTime;
	int			kamikazeShockTime;
#endif

	int			watertype;
	int			waterlevel;

	int			noise_index;

	// timing variables
	float		wait;
	float		random;

	gitem_t		*item;			// for bonus items

	int			pickupDelay;
//	int			ammoCount;
};


typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

typedef enum {
	SPECMODE_OFF,
	SPECMODE_LEADER,	// Follow the leader
	SPECMODE_LASTFRAG	// Follow the player who scored the last frag
} playerSpecMode_t;

typedef struct {
	playerTeamStateState_t	state;

	int			location;

	int			captures;
	int			basedefense;
	int			carrierdefense;
	int			flagrecovery;
	int			fragcarrier;
	int			assists;

	float		lasthurtcarrier;
	float		lastreturnedflag;
	float		flagsince;
	float		lastfraggedcarrier;
} playerTeamState_t;

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct {
	team_t		sessionTeam;
	int			spectatorNum;		// for determining next-in-line to play
	spectatorState_t	spectatorState;
	int			spectatorClient;	// for chasecam and follow mode
	int			wins, losses;		// tournament stats
	qboolean	teamLeader;			// true when this client is a team leader

	qboolean	mute;				// player is muted from chatting and voting
	qboolean	suspended;			// player is suspended from the game, and forced to spectate
	qboolean	admin;				// player has admin privilege (via vote, password, or via direct server access )
	int			adminReqFails;
	int			demerits;			// if a player gets too many, they will be suspended from the game
	int			specMode;			// spectator follow mode
} clientSession_t;

//
#define MAX_NETNAME				32
#define MAX_IP6					40
#define MAX_CLANNAME			16
#define MAX_CLANNAMECHAR		8
#define	MAX_VOTE_COUNT			5

#define	MAX_CHAT_COLOR_CODE		1 // will remove

//unlagged - true ping
#define NUM_PING_SAMPLES		64
//unlagged - true ping

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	clientConnected_t	connected;
	usercmd_t		cmd;				// we would lose angles if not persistant
	qboolean		localClient;		// true if "ip" info key is "localhost"
	qboolean		initialSpawn;		// the first spawn should be at a cool location
	qboolean		predictItemPickup;	// based on cg_predictItems userinfo
	qboolean		pmoveFixed;			//
	char			netname[MAX_NETNAME];
	char			ip[MAX_IP6];
	char			clan[MAX_CLANNAME];

	int				chatColor;			// chat color
	int				teamchatColor;		// teamchat color

	int				maxHealth;			// for handicapping
	int				enterTime;			// level.time the client entered the game
	playerTeamState_t	teamState;		// status in teamplay games
	int				voteCount;			// to prevent people from constantly calling votes
	int				teamVoteCount;		// to prevent people from constantly calling votes
	qboolean		teamInfo;			// send team overlay updates?

	qboolean		ready;				// player is ready to exit warmup, and start the match
	int				vote;				// vote

	// mmp - player stats
	int				kills;
	int				teamKills;
	int				deaths;
	int				suicides;
	int				captures;
	int				killStreak;

	// mmp - spam detection
	int 			spamCount;
	int				lastSpamTime;
	int				lastSpamResetTime;

//unlagged - client options
	// these correspond with variables in the userinfo string
	int				delag;
//	int				debugDelag;
	int				cmdTimeNudge;
//unlagged - client options
//unlagged - lag simulation #2
/*	int				latentSnaps;
	int				latentCmds;
	int				plOut;
	usercmd_t		cmdqueue[MAX_LATENT_CMDS];
	int				cmdhead;*/
//unlagged - lag simulation #2

//unlagged - lag simulation #2
//unlagged - true ping
	int				realPing;
	int				pingsamples[NUM_PING_SAMPLES];
	int				samplehead;
//unlagged - true ping

} clientPersistant_t;

//unlagged - backward reconciliation #1
// the size of history we'll keep
#define NUM_CLIENT_HISTORY 17

// everything we need to know to backward reconcile
typedef struct {
	vec3_t			mins, maxs;
	vec3_t			currentOrigin;
	int				leveltime;
} clientHistory_t;
//unlagged - backward reconciliation #1

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	qboolean		readyToExit;	// wishes to leave the intermission

	qboolean		noclip;
	qboolean		ghost;

	qboolean		giveUp;			// player gives up
	qboolean		disableGiveUp;	// if the player gave up, then changed their mind, prevent them from changing it again

	int				lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION
									// we can't just use pers.lastCommand.time, because
									// of the g_sycronousclients case
	int				buttons;
	int				oldbuttons;
	int				upmove; // mmp
	int				oldupmove; // mmp
	int				latched_buttons;

	vec3_t			oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int				damage_armor;		// damage absorbed by armor
	int				damage_blood;		// damage taken out of health
	int				damage_knockback;	// impact damage
	vec3_t			damage_from;		// origin for vector calculation
	qboolean		damage_fromWorld;	// if true, don't use the damage_from vector

	int				accurateCount;		// for "impressive" reward sound

	int				accuracy_shots;		// total number of shots
	int				accuracy_hits;		// total number of hits

	//
	int				lastkilled_client;	// last client that this client killed
	int				lasthurt_client;	// last client that damaged this client
	int				lasthurt_mod;		// type of damage the client did

	// timers
	int				respawnTime;		// can respawn when time > this, force after g_forcerespwan
	int				inactivityTime;		// kick players when time > this
	qboolean		inactivityWarning;	// qtrue if the five seoond warning has been given
	int				rewardTime;			// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	// mmp timers
	int				race_startTime;		// start time on a race map

	int				airOutTime;

	// mmp - player combokills
	int				comboCounter;
	int				lastKillTime;		// for multiple kill rewards

	int				healthDecayTime;	// time until health starts to decay
	int				healthDecayRate;	// increase health decay rate for taking more than one mega
	int				healthTimeResidual;	// uses the variable above to handle events at a set rate ( 1000 / healthDecayRate )

	int				healthRegenTime;	// time until health starts to regen, and is used for rate as well

	qboolean		fireHeld;		// used for hook
	gentity_t		*hook;			// grapple hook if out

	int				switchTeamTime;		// time the player switched teams

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int				timeResidual;

	// mmp
	int				curKillStreak;		// current kill streak
	//qboolean		allowItemPickUp;	// meant to delay pickup on items when spawning on them
	int				disallowItemPickUp;	// meant to delay pickup on items when spawning on them
	int				lightningLastHit;	// last lg hit time

	int				lastBlasterShot;	// time last blaster was shot

#ifdef MISSIONPACK
	gentity_t		*persistantPowerup;
	int				portalID;
	int				ammoTimes[WP_NUM_WEAPONS];
	int				invulnerabilityTime;
#endif

	//unlagged - backward reconciliation #1
	// the serverTime the button was pressed
	// (stored before pmove_fixed changes serverTime)
	int				attackTime;
	// the head of the history queue
	int				historyHead;
	// the history queue
	clientHistory_t	history[NUM_CLIENT_HISTORY];
	// the client's saved position
	clientHistory_t	saved;			// used to restore after time shift
	// an approximation of the actual server time we received this
	// command (not in 50ms increments)
	int				frameOffset;
//unlagged - backward reconciliation #1

//unlagged - smooth clients #1
	// the last frame number we got an update from this client
	int				lastUpdateFrame;
//unlagged - smooth clients #1

	char			*areabits;

//	char			*lastDrop;

	// mmp - player status items
	int				burnStatusTime; // burn status until set time
	int				medkitRegenTime; // medkit usage until set time

};


//
// this structure is cleared as each map is entered
//
#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	4096

typedef struct {
	struct gclient_s	*clients;			// [maxclients]

	struct gentity_s	*gentities;
	int				gentitySize;
	int				num_entities;			// MAX_CLIENTS <= num_entities <= ENTITYNUM_MAX_NORMAL

	int				warmupTime;			// restart match at this time

	fileHandle_t	logFile;

	// store latched cvars here that we want to get at often
	int				maxclients;

	int				framenum;
	int				time;				// in msec
	int				previousTime;			// so movers can back up when blocked

	int				startTime;			// level.time the map was started

	// mmp
	int				lastStatusTime;		// the time of the last status report for the server info
	int				race_topTime;		// top time on a race map for current session
	char			race_topNetname[MAX_NETNAME];	// top player on a race map for current session

	int				killTotal;			// total amount of kills, meant for determining stalemates


	int				teamScores[TEAM_NUM_TEAMS];	// team scores
	float			teamSubScores[TEAM_NUM_TEAMS];	// for use with balanced scoring
	int				lastTeamLocationTime;		// last time of client team location update

	int				currentBackpackSlot;	// used for storing contents in a backpack (backpack_t)

	qboolean		newSession;			// don't use any old session data, because
										// we changed gametype

	qboolean		restarted;			// waiting for a map_restart to fire

	int				numConnectedClients;
	int				numNonSpectatorClients;		// includes connecting clients
	int				numPlayingClients;		// connected, non-spectators
	int				sortedClients[MAX_CLIENTS];	// sorted by score
	int				follow1, follow2;		// clientNums for auto-follow spectators

	int				snd_fry;			// sound index for standing in lava

	int				warmupModificationCount;	// for detecting if g_warmup is changed

	// bookkeeping stats
	int				maxPlayersAchieved;		// maximum active players that connected to the server per match
	int				maxClientsAchieved;		// maximum clients that connected to the server per match

	// voting state
	char			voteString[MAX_STRING_CHARS];
	char			voteDisplayString[MAX_STRING_CHARS];
	int				voteTime;			// level.time vote was called
	int				voteExecuteTime;		// time the vote is executed
	int				voteCoolDownStart;		// the time cool down for votes
	int				voteYes;
	int				voteNo;
	int				numVotingClients;		// set by CalculateRanks
	qboolean		currentVoteIsKick;		// true if current vote is to kick

	// team voting state
	char			teamVoteString[2][MAX_STRING_CHARS];
	int				teamVoteTime[2];		// level.time vote was called
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	int				numteamVotingClients[2];	// set by CalculateRanks

	// spawn variables
	qboolean		spawning;			// the G_Spawn*() functions are valid
	int				numSpawnVars;
	char			*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int				numSpawnVarChars;
	char			spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// overtime variables
	qboolean		pastTimelimit;
	int				overtime;

	// round based variables
	int				currentRound;

	// allowed vote options
	/*int			allowedVotes;*/

	// end game stats
	int				statRecordSlot;			// current record slot for stats

	// old end game stats
	qboolean		statsRunLoop;
	int				statsLoopCounter;
	int				numSorted;

	// new end game stats
	int				statsBufferSendLastTime;
	int				statsCurClientSlot;
	qboolean		statsRunStop;

	// intermission state
	int				intermissionQueued;		// intermission was qualified, but
											// wait INTERMISSION_DELAY_TIME before
											// actually going there so the last
											// frag can be watched.  Disable future
											// kills during this delay
	int				intermissiontime;		// time the intermission was started
	int				intermissionEndAlert;		// displays a message when intermission is about to end
	qboolean		matchEnded;			//
	char			*changemap;
	qboolean		readyToExit;			// at least one client wants to exit
	int				exitTime;
	vec3_t			intermission_origin;		// also used for spectator spawns
	vec3_t			intermission_angle;

	qboolean		locationLinked;			// target_locations get linked
	gentity_t		*locationHead;			// head of the location list
	int				bodyQueIndex;			// dead bodies
	gentity_t		*bodyQue[BODY_QUEUE_SIZE];
#ifdef MISSIONPACK
	int				portalSequence;
#endif

	int				readyMask[4];			// keeps track of players that are ready during warmup

	qboolean		endWarmup;				// forces players to ready up

	// mmp - ruleset enforcement
	qboolean		rulesetEnforced;
	qboolean		rulesetViolated; // forces match to end in a disqualification
	qboolean		warmupRunning; //  gets set when warmup was started, and cannot be changed into an active game via a ruleset change
	int				rs_timelimit;
	int				rs_mercylimit; // mercy limit
	int				rs_overtime;
	int				rs_scorelimit;
	int				rs_friendlyFire;
	int				rs_speed;
	int				rs_gravity;
	int				rs_knockback;
	int				rs_quadFactor;
	int				rs_matchMode;
	int				rs_weaponRespawn;
	int				rs_forceRespawn;
	int				rs_teamLocOverlay;
	int				rs_hitSound;
	int				rs_randomSpawn;
	int				rs_scoreBalance;
	int				rs_quadMode;
	int				rs_selfDamage;
	int				rs_doubleAmmo;
	int				rs_keycardRespawn;
	int				rs_keycardDropable;
	int				rs_noArenaGrenades;
	int				rs_noArenaLightningGun;
	float			rs_enemyAttackLevel;
	int				rs_powerUps;
	int				rs_armor;
	int				rs_popCTF;
	int				rs_roundFormat; // enables 3 round matches, 1r = normal, 2r = losing player must have half of leader, 3r = overtime
	int				rs_dynamicItemSpawns; // increases item respawn times as rounds advance

	// not really rulesets, but correctly applies the right value for each cvar
	int				rs_warmup;
	int				rs_doWarmup; // no longer used

	// mmp - time compulsion enforcement
	float			c_extraDamage;
	int				c_flagReturnDecrease;
	int				c_spawnDelay;
	int				c_timeUpdate;

	qboolean		updateRuleset;
	qboolean		forcefullyUpdateRuleset;

	// map specified ruleset modifications
	int				rsmod_timelimit_dm;
	int				rsmod_timelimit_tourney;
	int				rsmod_timelimit_team;
	int				rsmod_timelimit_ctf;
	int				rsmod_timelimit_else;

	int				rsmod_overtime_dm;
	int				rsmod_overtime_tourney;
	int				rsmod_overtime_team;
	int				rsmod_overtime_ctf;
	int				rsmod_overtime_else;

	int				rsmod_matchMode;

	// mmp - misc
	qboolean		updateInfo;

	// mmp - specmode variables
	int				sm_lastFrag;
	int				sm_lastFragTime;

	int				sm_leader;
	int				sm_leaderScore;
	int				sm_leaderTime;

	// mmp - random select
	int 			rnd_type; // check randomSelect_t at the top for more info
	int 			rnd_mode; // 0 = off
	int				rnd_nextthink;
	char			rnd_selected[32];
	int				rnd_minNum;
	int				rnd_maxNum;

	// mmp - server maintenance
	int				lastActiveTime;
	qboolean		forcedTimeLimit; // value set if match/warmup dragged on for too long (which could be that no one is on the server)

	// mmp - in team games, there must be at least 2 players on each team
	int				teamSizesOK; // 0 == ok
	qboolean		teamSizeNagQueue; // qtrue == announce when all ppl ready up

	// was cf_mode and cf_nextthink

//unlagged - backward reconciliation #4
	// actual time this server frame started
	int				frameStartTime;
//unlagged - backward reconciliation #4

} level_locals_t;


// team sizes ok messages
typedef enum {
	TSOK_OK,
	TSOK_MORE_RED_PLAYERS,
	TSOK_MORE_BLUE_PLAYERS,
	TSOK_MORE_PLAYERS,
	TSOK_NEED_ANOTHER_PLAYER,
	TSOK_MAX
} teamSizeOK_t;


// stats from clients
typedef struct {
	char			netname[MAX_NETNAME];
//	char			ip[MAX_IP6];
	char			clan[MAX_CLANNAME];

// type of record
	int				type;
//	RS_ENDSCORE,
//	RS_DISCONNECTED,
//	RS_FAILED_CONNECTION,
//	RS_KICKED,
//	RS_SUSPENDED,

	int				score;

	int				kills;
	int				teamKills;
	int				deaths;
	int				suicides;
	int				captures;
	int				killStreak;
} clientStats_t; // stats


//
// records what's in each backpack
//
#define	MAX_BACKPACK_CONTENTS	128

typedef struct {
	int				lastWeap;
	int				shells;
	int				rockets;
	int				cells;
	int				bullets;
	int				gas;
} backpack_t;


//
// g_spawn.c
//
qboolean	G_SpawnString( const char *key, const char *defaultString, char **out );
qboolean	G_SpawnStringWithRuleset( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out );
void		G_SpawnEntitiesFromString( void );
char *G_NewString( const char *string );

//
// g_cmds.c
//
void Cmd_Score_f (gentity_t *ent);
void StopFollowing( gentity_t *ent );
void BroadcastTeamChange( gclient_t *client, int oldTeam );
void SetTeam( gentity_t *ent, char *s );
void Cmd_FollowCycle_f( gentity_t *ent, int dir, int ftype );
void Cmd_Coinflip_f (gentity_t *ent);

//
// g_items.c
//
void G_CheckTeamItems( void );
void G_RunItem( gentity_t *ent );
void RespawnItem( gentity_t *ent );

void UseHoldableItem( gentity_t *ent );
void PrecacheItem (gitem_t *it);
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle );
gentity_t *Drop_Item_Weapon( gentity_t *ent, gitem_t *item, float angle, int weapon );
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity );
gentity_t *LaunchItem_Weapon( gitem_t *item, vec3_t origin, vec3_t velocity, int dropTime, int weapon );
void SetRespawn (gentity_t *ent, float delay);
void G_SpawnItem (gentity_t *ent, gitem_t *item);
void FinishSpawningItem( gentity_t *ent );
void Think_Weapon (gentity_t *ent);
int ArmorIndex (gentity_t *ent);
void	Add_Ammo (gentity_t *ent, int weapon, int count); // TODO: change int weapon to int ammo
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace);

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );

//
// g_utils.c
//
int G_ModelIndex( char *name );
int		G_SoundIndex( char *name );
void	G_TeamCommand( team_t team, char *cmd );
void	G_KillBox (gentity_t *ent);
gentity_t *G_Find (gentity_t *from, int fieldofs, const char *match);
gentity_t *G_PickTarget (char *targetname);
void	G_UseTargets (gentity_t *ent, gentity_t *activator);
void	G_SetMovedir ( vec3_t angles, vec3_t movedir);

void	G_InitGentity( gentity_t *e );
gentity_t	*G_Spawn (void);
gentity_t *G_TempEntity( vec3_t origin, int event );
void	G_Sound( gentity_t *ent, int channel, int soundIndex );
void	G_FreeEntity( gentity_t *e );
qboolean	G_EntitiesFree( void );

void	G_TouchTriggers (gentity_t *ent);

float	*tv (float x, float y, float z);
char	*vtos( const vec3_t v );

float vectoyaw( const vec3_t vec );

void G_AddPredictableEvent( gentity_t *ent, int event, int eventParm );
void G_AddEvent( gentity_t *ent, int event, int eventParm );
void G_SetOrigin( gentity_t *ent, vec3_t origin );
void AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char *BuildShaderStateConfig( void );

//
// g_combat.c
//
qboolean CanDamage (gentity_t *targ, vec3_t origin);
void G_Damage (gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod);
qboolean G_RadiusDamage (vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod);
int G_InvulnerabilityEffect( gentity_t *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir );
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void TossClientItems( gentity_t *self );
#ifdef MISSIONPACK
void TossClientPersistantPowerups( gentity_t *self );
#endif
float GetDamageLevel (gentity_t *ent);
void TossClientCubes( gentity_t *self );

// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR				0x00000002	// armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK			0x00000004	// do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION		0x00000008  // armor, shields, invulnerability, and godmode have no effect
#ifdef MISSIONPACK
#define DAMAGE_NO_TEAM_PROTECTION	0x00000010  // armor, shields, invulnerability, and godmode have no effect
#endif

//
// g_missile.c
//
void G_RunMissile( gentity_t *ent );

gentity_t *fire_blaster (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_plasma (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_spread (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_flame (gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up);
gentity_t *fire_duallaser ( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_bfg (gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_grapple (gentity_t *self, vec3_t start, vec3_t dir);
#ifdef MISSIONPACK
gentity_t *fire_nail( gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up );
gentity_t *fire_prox( gentity_t *self, vec3_t start, vec3_t aimdir );
#endif


//
// g_mover.c
//
void G_RunMover( gentity_t *ent );
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace );

//
// g_trigger.c
//
void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace );


//
// g_misc.c
//
void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles );
#ifdef MISSIONPACK
void DropPortalSource( gentity_t *ent );
void DropPortalDestination( gentity_t *ent );
#endif


//
// g_weapon.c
//
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker );
void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
//unlagged - attack prediction #3
// we're making this available to both games
void SnapVectorTowards( vec3_t v, vec3_t to );
//unlagged - attack prediction #3
void SnapVectorTowards( vec3_t v, vec3_t to );
qboolean CheckGauntletAttack( gentity_t *ent );
void Weapon_HookFree (gentity_t *ent);
void Weapon_HookThink (gentity_t *ent);

//unlagged - g_unlagged.c
//
// g_unlagged.c
//

void G_ResetHistory( gentity_t *ent );
void G_StoreHistory( gentity_t *ent );
void G_TimeShiftAllClients( int time, gentity_t *skip );
void G_UnTimeShiftAllClients( gentity_t *skip );
void G_DoTimeShiftFor( gentity_t *ent );
void G_UndoTimeShiftFor( gentity_t *ent );
void G_UnTimeShiftClient( gentity_t *client );
void G_PredictPlayerMove( gentity_t *ent, float frametime );
//unlagged - g_unlagged.c


//
// g_client.c
//
team_t TeamCount( int ignoreClientNum, int team );
int TeamLeader( int team );
team_t PickTeam( int ignoreClientNum );
void SetClientViewAngle( gentity_t *ent, vec3_t angle );
gentity_t *SelectSpawnPoint (vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot);
void CopyToBodyQue( gentity_t *ent );
void ClientRespawn(gentity_t *ent);
void BeginIntermission (void);
void InitBodyQue (void);
void ClientSpawn( gentity_t *ent );
void player_die (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void AddScore( gentity_t *ent, vec3_t origin, int score );
void CalculateRanks( void );
qboolean SpotWouldTelefrag( gentity_t *spot );

//
// g_svcmds.c
//
qboolean	ConsoleCommand( void );
void G_ProcessIPBans(void);
qboolean G_FilterPacket (char *from);

//
// g_weapon.c
//
void FireWeapon( gentity_t *ent );
#ifdef MISSIONPACK
void G_StartKamikaze( gentity_t *ent );
#endif

//
// g_cmds.c
//
void DeathmatchScoreboardMessage( gentity_t *ent );

//
// g_main.c
//
//void G_RegisterCvars( void );
void MoveClientToIntermission( gentity_t *ent );
void FindIntermissionPoint( void );
void SetLeader(int team, int client);
void CheckTeamLeader( int team );
void G_RunThink (gentity_t *ent);
void AddTournamentQueue(gclient_t *client);
void QDECL G_LogPrintf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL G_VerboseLogPrintf( int vlevel, const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void G_BookkeepingLog( void );
void SendScoreboardMessageToAllClients( void );
void QDECL G_Printf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL G_Error( const char *fmt, ... ) __attribute__ ((noreturn, format (printf, 1, 2)));
int G_AllowedVotes ( gentity_t *ent, qboolean printNames );
gclient_t	*ClientForString( const char *s );
int			ClientNumForString( const char *s );

//
// g_client.c
//
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
//void PhysicsModeSetUp( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( gentity_t *ent );
void G_RunClient( gentity_t *ent );

//
// g_team.c
//
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
void Team_CheckDroppedItem( gentity_t *dropped );
qboolean CheckObeliskAttack( gentity_t *obelisk, gentity_t *attacker );

//
// g_mem.c
//
void *G_Alloc( int size );
void G_InitMemory( void );
void Svcmd_GameMem_f( void );

//
// g_session.c
//
void G_ReadSessionData( gclient_t *client );
void G_InitSessionData( gclient_t *client, char *userinfo );

void G_InitWorldSession( void );
void G_WriteSessionData( void );

//
// g_arenas.c
//
void UpdateTournamentInfo( void );
void SpawnModelsOnVictoryPads( void );
void Svcmd_AbortPodium_f( void );

//
// g_bot.c
//
void G_InitBots( qboolean restart );
char *G_GetBotInfoByNumber( int num );
char *G_GetBotInfoByName( const char *name );
void G_CheckBotSpawn( void );
void G_RemoveQueuedBotBegin( int clientNum );
qboolean G_BotConnect( int clientNum, qboolean restart );
void Svcmd_AddBot_f( void );
void Svcmd_BotList_f( void );
void BotInterbreedEndMatch( void );

// ai_main.c
#define MAX_FILEPATH			144

//bot settings
typedef struct bot_settings_s
{
	char characterfile[MAX_FILEPATH];
	float skill;
	char team[MAX_FILEPATH];
} bot_settings_t;

int BotAISetup( int restart );
int BotAIShutdown( int restart );
int BotAILoadMap( int restart );
int BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart);
int BotAIShutdownClient( int client, qboolean restart );
int BotAIStartFrame( int time );
void BotTestAAS(vec3_t origin);

#include "g_team.h" // teamplay specific stuff


extern	level_locals_t	level;
extern	clientStats_t	stats[MAX_CLIENT_STATS_SLOT];
extern	gentity_t		g_entities[MAX_GENTITIES];
extern	backpack_t		backpack[MAX_BACKPACK_CONTENTS];

#define	FOFS(x) ((size_t)&(((gentity_t *)0)->x))

extern	vmCvar_t	g_gametype;
extern	vmCvar_t	g_gameMode;
extern	vmCvar_t	g_dedicated;
extern	vmCvar_t	g_cheats;
extern	vmCvar_t	g_maxclients;			// allow this many total, including spectators
extern	vmCvar_t	g_maxGameClients;		// allow this many active
extern	vmCvar_t	g_restarted;

extern	vmCvar_t	g_ruleset;
extern	vmCvar_t	g_dmflags;
extern	vmCvar_t	g_fraglimit;
extern	vmCvar_t	g_timelimit;
extern	vmCvar_t	g_mercylimit;
extern	vmCvar_t	g_overtime;
extern	vmCvar_t	g_capturelimit;
extern	vmCvar_t	g_scorelimit;

extern	vmCvar_t	g_friendlyFire;
extern	vmCvar_t	g_teamLocOverlay;
extern	vmCvar_t	g_hitSound;
extern	vmCvar_t	g_scoreBalance;
extern	vmCvar_t	g_teamSize;				// max team size
extern	vmCvar_t	g_teamSizeQuota;		// required team size
extern	vmCvar_t	g_playersLocOverlay;

extern	vmCvar_t	g_password;
extern	vmCvar_t	g_needpass;
extern	vmCvar_t	g_gravity;
extern	vmCvar_t	g_speed;
extern	vmCvar_t	g_knockback;
extern	vmCvar_t	g_quadfactor;
extern	vmCvar_t	g_forcerespawn;
extern	vmCvar_t	g_inactivity;
extern	vmCvar_t	g_debugMove;
extern	vmCvar_t	g_debugAlloc;
extern	vmCvar_t	g_debugDamage;
extern	vmCvar_t	g_matchMode;
extern	vmCvar_t	g_weaponRespawn;
extern	vmCvar_t	g_weaponTeamRespawn;
extern	vmCvar_t	g_synchronousClients;
extern	vmCvar_t	g_motd;
extern	vmCvar_t	g_warmup;
//extern	vmCvar_t	g_doWarmup;
extern	vmCvar_t	g_blood;
extern	vmCvar_t	g_allowVote;
extern	vmCvar_t	g_allowedVoteNames;
extern	vmCvar_t	g_voteWaitTime;
extern	vmCvar_t	g_specChat;
extern	vmCvar_t	g_teamAutoJoin;
extern	vmCvar_t	g_teamForceBalance;
extern	vmCvar_t	g_banIPs;
extern	vmCvar_t	g_filterBan;
extern	vmCvar_t	g_obeliskHealth;
extern	vmCvar_t	g_obeliskRegenPeriod;
extern	vmCvar_t	g_obeliskRegenAmount;
extern	vmCvar_t	g_obeliskRespawnDelay;
extern	vmCvar_t	g_cubeTimeout;
extern	vmCvar_t	g_redteam;
extern	vmCvar_t	g_blueteam;
extern	vmCvar_t	g_smoothClients;
extern	vmCvar_t	pmove_fixed;
extern	vmCvar_t	pmove_msec;

// mmp
extern	vmCvar_t	g_info;
extern	vmCvar_t	g_status;
extern	vmCvar_t	g_redTeamCount;
extern	vmCvar_t	g_blueTeamCount;
extern	vmCvar_t	g_clientCount;
extern	vmCvar_t	g_playerCount;
extern	vmCvar_t	g_randomByte;
extern	vmCvar_t	g_proMode; // was g_physicsMode
extern	vmCvar_t	g_teamLock;
extern	vmCvar_t	g_bookkeepingLog;
extern	vmCvar_t	g_demeritLimit;
extern	vmCvar_t	g_quadMode;
extern	vmCvar_t	g_selfDamage;
extern	vmCvar_t	g_doubleAmmo;
extern	vmCvar_t	g_keycardRespawn;
extern	vmCvar_t	g_keycardDropable;
extern	vmCvar_t	g_noArenaGrenades;
extern	vmCvar_t	g_noArenaLightningGun;
extern	vmCvar_t	g_enemyAttackLevel;
extern	vmCvar_t	g_powerUps;
extern	vmCvar_t	g_armor;
extern	vmCvar_t	g_allowGhost;
extern	vmCvar_t	g_shortGame;
extern	vmCvar_t	g_roundFormat;
extern	vmCvar_t	g_dynamicItemSpawns;

extern	vmCvar_t	g_iUnderstandBotsAreBroken; // mmp

extern	vmCvar_t	g_spamLimitCount;
extern	vmCvar_t	g_spamLimitTimeRange;
extern	vmCvar_t	g_allowSpecVote;
extern	vmCvar_t	g_allowSpecCallVote;
extern	vmCvar_t	g_randomSpawn;
extern	vmCvar_t	g_adminPassword;
extern	vmCvar_t	g_mapRotation;
extern	vmCvar_t	g_mapRotation_ffa;
extern	vmCvar_t	g_mapRotation_duel;
extern	vmCvar_t	g_mapRotation_tdm;
extern	vmCvar_t	g_mapRotation_ctf;

extern	vmCvar_t	g_allowedAdminCmds;

extern	vmCvar_t	g_serviceScheduleSun;
extern	vmCvar_t	g_serviceScheduleMon;
extern	vmCvar_t	g_serviceScheduleTues;
extern	vmCvar_t	g_serviceScheduleWed;
extern	vmCvar_t	g_serviceScheduleThurs;
extern	vmCvar_t	g_serviceScheduleFri;
extern	vmCvar_t	g_serviceScheduleSat;
extern	vmCvar_t	g_serviceScheduleDaily;

extern	vmCvar_t	g_currentDay;

extern	vmCvar_t	g_serviceOnEmptyTime;
extern	vmCvar_t	g_serviceOnEmptyExec;
//

//unlagged - server options
// some new server-side variables
extern	vmCvar_t	g_truePing;
extern	vmCvar_t	sv_fps; // this is for convenience - using "sv_fps.integer" is nice :)
//unlagged - server options

//unlagged - lagNudge
extern	vmCvar_t	g_delagprojectiles;
//unlagged - lagNudge

extern	vmCvar_t	g_rankings;
extern	vmCvar_t	g_enableDust;
extern	vmCvar_t	g_enableBreath;
extern	vmCvar_t	g_singlePlayer;
extern	vmCvar_t	g_proxMineTimeout;

void	trap_Print( const char *text );
void	trap_Error( const char *text ) __attribute__((noreturn));
int		trap_Milliseconds( void );
int	trap_RealTime( qtime_t *qtime );
int		trap_Argc( void );
void	trap_Argv( int n, char *buffer, int bufferLength );
void	trap_Args( char *buffer, int bufferLength );
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );
int		trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int		trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t
void	trap_SendConsoleCommand( int exec_when, const char *text );
void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void	trap_Cvar_Update( vmCvar_t *cvar );
void	trap_Cvar_Set( const char *var_name, const char *value );
int		trap_Cvar_VariableIntegerValue( const char *var_name );
float	trap_Cvar_VariableValue( const char *var_name );
void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
void	trap_DropClient( int clientNum, const char *reason );
void	trap_SendServerCommand( int clientNum, const char *text );
void	trap_SetConfigstring( int num, const char *string );
void	trap_GetConfigstring( int num, char *buffer, int bufferSize );
void	trap_GetUserinfo( int num, char *buffer, int bufferSize );
void	trap_SetUserinfo( int num, const char *buffer );
void	trap_GetServerinfo( char *buffer, int bufferSize );
void	trap_SetBrushModel( gentity_t *ent, const char *name );
void	trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
int		trap_PointContents( const vec3_t point, int passEntityNum );
qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 );
qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void	trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean trap_AreasConnected( int area1, int area2 );
void	trap_LinkEntity( gentity_t *ent );
void	trap_UnlinkEntity( gentity_t *ent );
int		trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
int		trap_BotAllocateClient( void );
void	trap_BotFreeClient( int clientNum );
void	trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean	trap_GetEntityToken( char *buffer, int bufferSize );

int		trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points);
void	trap_DebugPolygonDelete(int id);

int		trap_BotLibSetup( void );
int		trap_BotLibShutdown( void );
int		trap_BotLibVarSet(char *var_name, char *value);
int		trap_BotLibVarGet(char *var_name, char *value, int size);
int		trap_BotLibDefine(char *string);
int		trap_BotLibStartFrame(float time);
int		trap_BotLibLoadMap(const char *mapname);
int		trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
int		trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);

int		trap_BotGetSnapshotEntity( int clientNum, int sequence );
int		trap_BotGetServerCommand(int clientNum, char *message, int size);
void	trap_BotUserCommand(int client, usercmd_t *ucmd);

int		trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas);
int		trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info );
void	trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);

int		trap_AAS_Initialized(void);
void	trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs);
float	trap_AAS_Time(void);

int		trap_AAS_PointAreaNum(vec3_t point);
int		trap_AAS_PointReachabilityAreaIndex(vec3_t point);
int		trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas);

int		trap_AAS_PointContents(vec3_t point);
int		trap_AAS_NextBSPEntity(int ent);
int		trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size);
int		trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v);
int		trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value);
int		trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value);

int		trap_AAS_AreaReachability(int areanum);

int		trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags);
int		trap_AAS_EnableRoutingArea( int areanum, int enable );
int		trap_AAS_PredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum);

int		trap_AAS_AlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags,
										void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
										int type);
int		trap_AAS_Swimming(vec3_t origin);
int		trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize);


void	trap_EA_Say(int client, char *str);
void	trap_EA_SayTeam(int client, char *str);
void	trap_EA_Command(int client, char *command);

void	trap_EA_Action(int client, int action);
void	trap_EA_Gesture(int client);
void	trap_EA_Talk(int client);
void	trap_EA_Attack(int client);
void	trap_EA_Use(int client);
void	trap_EA_Respawn(int client);
void	trap_EA_Crouch(int client);
void	trap_EA_MoveUp(int client);
void	trap_EA_MoveDown(int client);
void	trap_EA_MoveForward(int client);
void	trap_EA_MoveBack(int client);
void	trap_EA_MoveLeft(int client);
void	trap_EA_MoveRight(int client);
void	trap_EA_SelectWeapon(int client, int weapon);
void	trap_EA_Jump(int client);
void	trap_EA_DelayedJump(int client);
void	trap_EA_Move(int client, vec3_t dir, float speed);
void	trap_EA_View(int client, vec3_t viewangles);

void	trap_EA_EndRegular(int client, float thinktime);
void	trap_EA_GetInput(int client, float thinktime, void /* struct bot_input_s */ *input);
void	trap_EA_ResetInput(int client);


int		trap_BotLoadCharacter(char *charfile, float skill);
void	trap_BotFreeCharacter(int character);
float	trap_Characteristic_Float(int character, int index);
float	trap_Characteristic_BFloat(int character, int index, float min, float max);
int		trap_Characteristic_Integer(int character, int index);
int		trap_Characteristic_BInteger(int character, int index, int min, int max);
void	trap_Characteristic_String(int character, int index, char *buf, int size);

int		trap_BotAllocChatState(void);
void	trap_BotFreeChatState(int handle);
void	trap_BotQueueConsoleMessage(int chatstate, int type, char *message);
void	trap_BotRemoveConsoleMessage(int chatstate, int handle);
int		trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm);
int		trap_BotNumConsoleMessages(int chatstate);
void	trap_BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotNumInitialChats(int chatstate, char *type);
int		trap_BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotChatLength(int chatstate);
void	trap_BotEnterChat(int chatstate, int client, int sendto);
void	trap_BotGetChatMessage(int chatstate, char *buf, int size);
int		trap_StringContains(char *str1, char *str2, int casesensitive);
int		trap_BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context);
void	trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size);
void	trap_UnifyWhiteSpaces(char *string);
void	trap_BotReplaceSynonyms(char *string, unsigned long int context);
int		trap_BotLoadChatFile(int chatstate, char *chatfile, char *chatname);
void	trap_BotSetChatGender(int chatstate, int gender);
void	trap_BotSetChatName(int chatstate, char *name, int client);
void	trap_BotResetGoalState(int goalstate);
void	trap_BotRemoveFromAvoidGoals(int goalstate, int number);
void	trap_BotResetAvoidGoals(int goalstate);
void	trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
void	trap_BotPopGoal(int goalstate);
void	trap_BotEmptyGoalStack(int goalstate);
void	trap_BotDumpAvoidGoals(int goalstate);
void	trap_BotDumpGoalStack(int goalstate);
void	trap_BotGoalName(int number, char *name, int size);
int		trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags);
int		trap_BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime);
int		trap_BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal);
int		trap_BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal);
int		trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);
int		trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
int		trap_BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal);
float	trap_BotAvoidGoalTime(int goalstate, int number);
void	trap_BotSetAvoidGoalTime(int goalstate, int number, float avoidtime);
void	trap_BotInitLevelItems(void);
void	trap_BotUpdateEntityItems(void);
int		trap_BotLoadItemWeights(int goalstate, char *filename);
void	trap_BotFreeItemWeights(int goalstate);
void	trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
void	trap_BotSaveGoalFuzzyLogic(int goalstate, char *filename);
void	trap_BotMutateGoalFuzzyLogic(int goalstate, float range);
int		trap_BotAllocGoalState(int state);
void	trap_BotFreeGoalState(int handle);

void	trap_BotResetMoveState(int movestate);
void	trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags);
int		trap_BotMoveInDirection(int movestate, vec3_t dir, float speed, int type);
void	trap_BotResetAvoidReach(int movestate);
void	trap_BotResetLastAvoidReach(int movestate);
int		trap_BotReachabilityArea(vec3_t origin, int testground);
int		trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target);
int		trap_BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target);
int		trap_BotAllocMoveState(void);
void	trap_BotFreeMoveState(int handle);
void	trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove);
void	trap_BotAddAvoidSpot(int movestate, vec3_t origin, float radius, int type);

int		trap_BotChooseBestFightWeapon(int weaponstate, int *inventory);
void	trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo);
int		trap_BotLoadWeaponWeights(int weaponstate, char *filename);
int		trap_BotAllocWeaponState(void);
void	trap_BotFreeWeaponState(int weaponstate);
void	trap_BotResetWeaponState(int weaponstate);

int		trap_GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child);

void	trap_SnapVector( float *v );

