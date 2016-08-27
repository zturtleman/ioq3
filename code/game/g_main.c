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

#include "g_local.h"

level_locals_t	level;
clientStats_t	stats[MAX_CLIENT_STATS_SLOT];

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
	qboolean	teamShader;			// track and if changed, update shader state
} cvarTable_t;

gentity_t		g_entities[MAX_GENTITIES];
gclient_t		g_clients[MAX_CLIENTS];

backpack_t		backpack[MAX_BACKPACK_CONTENTS]; // mmp

vmCvar_t	g_gametype;
vmCvar_t	g_gameMode; // will be removed
vmCvar_t	g_ruleset;
vmCvar_t	g_dmflags;
vmCvar_t	g_fraglimit;
vmCvar_t	g_timelimit;
vmCvar_t	g_mercylimit;
vmCvar_t	g_overtime;
vmCvar_t	g_capturelimit;
vmCvar_t	g_scorelimit;

vmCvar_t	g_friendlyFire;
vmCvar_t	g_teamLocOverlay;
vmCvar_t	g_hitSound;
vmCvar_t	g_scoreBalance;
vmCvar_t	g_teamSize;
vmCvar_t	g_teamSizeQuota;
vmCvar_t	g_playersLocOverlay;

vmCvar_t	g_password;
vmCvar_t	g_needpass;
vmCvar_t	g_maxclients;
vmCvar_t	g_maxGameClients;
vmCvar_t	g_dedicated;
vmCvar_t	g_speed;
vmCvar_t	g_gravity;
vmCvar_t	g_cheats;
vmCvar_t	g_knockback;
vmCvar_t	g_quadfactor;
vmCvar_t	g_forcerespawn;
vmCvar_t	g_inactivity;
vmCvar_t	g_debugMove;
vmCvar_t	g_debugDamage;
vmCvar_t	g_debugAlloc;
vmCvar_t	g_matchMode;
vmCvar_t	g_weaponRespawn;
vmCvar_t	g_weaponTeamRespawn;
vmCvar_t	g_motd;
vmCvar_t	g_synchronousClients;
vmCvar_t	g_warmup;
//vmCvar_t	g_doWarmup;
vmCvar_t	g_restarted;
vmCvar_t	g_logfile;
vmCvar_t	g_logfileSync;
vmCvar_t	g_verboseLog;
vmCvar_t	g_blood;
vmCvar_t	g_podiumDist;
vmCvar_t	g_podiumDrop;
vmCvar_t	g_allowVote;
vmCvar_t	g_allowedVoteNames; // mmp
vmCvar_t	g_voteWaitTime; // mmp
vmCvar_t	g_specChat; // mmp
vmCvar_t	g_teamAutoJoin;
vmCvar_t	g_teamForceBalance;
vmCvar_t	g_banIPs;
vmCvar_t	g_filterBan;
vmCvar_t	g_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;

// mmp
vmCvar_t	g_info;
vmCvar_t	g_status;
vmCvar_t	g_redTeamCount;
vmCvar_t	g_blueTeamCount;
vmCvar_t	g_clientCount;
vmCvar_t	g_playerCount;
vmCvar_t	g_randomByte;
vmCvar_t	g_proMode; // was g_physicsMode
vmCvar_t	g_teamLock;
vmCvar_t	g_bookkeepingLog;
vmCvar_t	g_demeritLimit;
vmCvar_t	g_quadMode;
vmCvar_t	g_selfDamage;
vmCvar_t	g_doubleAmmo;
vmCvar_t	g_keycardRespawn;
vmCvar_t	g_keycardDropable;
vmCvar_t	g_noArenaGrenades;
vmCvar_t	g_noArenaLightningGun;
vmCvar_t	g_spamLimitCount;
vmCvar_t	g_spamLimitTimeRange;
vmCvar_t	g_allowSpecVote;
vmCvar_t	g_allowSpecCallVote;
vmCvar_t	g_randomSpawn;
vmCvar_t	g_adminPassword;
vmCvar_t	g_mapRotation;
vmCvar_t	g_mapRotation_ffa;
vmCvar_t	g_mapRotation_duel;
vmCvar_t	g_mapRotation_tdm;
vmCvar_t	g_mapRotation_ctf;
vmCvar_t	g_allowedAdminCmds;
vmCvar_t	g_enemyAttackLevel;
vmCvar_t	g_powerUps;
vmCvar_t	g_armor;
vmCvar_t	g_allowGhost;
vmCvar_t	g_shortGame;
vmCvar_t	g_roundFormat;
vmCvar_t	g_dynamicItemSpawns;

vmCvar_t	g_serviceScheduleSun;
vmCvar_t	g_serviceScheduleMon;
vmCvar_t	g_serviceScheduleTues;
vmCvar_t	g_serviceScheduleWed;
vmCvar_t	g_serviceScheduleThurs;
vmCvar_t	g_serviceScheduleFri;
vmCvar_t	g_serviceScheduleSat;
vmCvar_t	g_serviceScheduleDaily;
vmCvar_t	g_currentDay;

vmCvar_t	g_serviceOnEmptyTime;
vmCvar_t	g_serviceOnEmptyExec;

//unlagged - server options
vmCvar_t	g_truePing;
vmCvar_t	sv_fps; // this is for convenience - using "sv_fps.integer" is nice :)
//unlagged - server options
//

//unlagged - lagNudge
vmCvar_t     g_delagprojectiles;
//unlagged - lagNudge

vmCvar_t	g_rankings;
vmCvar_t	g_listEntity;
#ifdef MISSIONPACK
vmCvar_t	g_obeliskHealth;
vmCvar_t	g_obeliskRegenPeriod;
vmCvar_t	g_obeliskRegenAmount;
vmCvar_t	g_obeliskRespawnDelay;
vmCvar_t	g_cubeTimeout;
vmCvar_t	g_redteam;
vmCvar_t	g_blueteam;
vmCvar_t	g_singlePlayer;
vmCvar_t	g_enableDust;
vmCvar_t	g_enableBreath;
vmCvar_t	g_proxMineTimeout;
#endif

static cvarTable_t		gameCvarTable[] = {
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, 0, qfalse },

	// noset vars
	{ NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
	{ NULL, "gamedate", __DATE__ , CVAR_ROM, 0, qfalse  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse  },
	/*{ NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },*/

	// latched vars
	{ &g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH /* | CVAR_RULESET*/, 0, qfalse  }, // mmp - to be fased out
	{ &g_gameMode, "", "", /*CVAR_SERVERINFO | CVAR_USERINFO*/ 0, 0, qfalse  }, // to be removed

	{ &g_maxclients, "sv_maxclients", "16", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },

	{ &g_ruleset, "g_ruleset", "1", CVAR_SERVERINFO | CVAR_RULESET, 0, qfalse  },

	{ &g_dmflags, "dmflags", "0", CVAR_ROM, 0, qfalse  }, // mmp - to be fased out
	{ &g_fraglimit, "", "", 0, 0, qtrue }, // mmp - to be fased out
	{ &g_timelimit, "timelimit", "10", /*CVAR_SERVERINFO |*/ CVAR_RULESET | CVAR_ARCHIVE /*| CVAR_NORESTART*/, 0, qfalse },
	{ &g_mercylimit, "mercylimit", "0", CVAR_RULESET | CVAR_ARCHIVE , 0, qfalse },
	{ &g_overtime, "overtime", "2", /*CVAR_SERVERINFO |*/ CVAR_RULESET | CVAR_ARCHIVE /*| CVAR_NORESTART*/, 0, qfalse },
	{ &g_capturelimit, "", "", 0, 0, qtrue }, // mmp - to be fased out
	{ &g_scorelimit, "scorelimit", "0", /*CVAR_SERVERINFO |*/ CVAR_RULESET | CVAR_ARCHIVE /*| CVAR_NORESTART*/, 0, qfalse },

	// mmp - ruleset cvar add
	{ &g_friendlyFire, "g_friendlyFire", "1", CVAR_RULESET | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_teamLocOverlay, "g_teamLocOverlay", "1", CVAR_RULESET | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_hitSound, "g_hitSound", "1", CVAR_RULESET | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_scoreBalance, "g_scoreBalance", "1", CVAR_RULESET | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_teamSize, "g_teamSize", "0", CVAR_RULESET | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_teamSizeQuota, "g_teamSizeQuota", "2", CVAR_RULESET | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_playersLocOverlay, "g_playersLocOverlay", "0", CVAR_RULESET | CVAR_CHEAT, 0, qfalse  }, // just a cheat, mainly for debug, will be removed

	{ &g_speed, "g_speed", "400", CVAR_RULESET | CVAR_CHEAT, 0, qtrue  }, // mmp - is 320 in vq3
	{ &g_gravity, "g_gravity", "1000", CVAR_RULESET | CVAR_CHEAT, 0, qtrue  }, // mmp - is 800 in vq3
	{ &g_knockback, "g_knockback", "1250", CVAR_RULESET | CVAR_CHEAT, 0, qtrue  }, // mmp - is 1000 in vq3
	{ &g_quadfactor, "g_quadfactor", "4", CVAR_RULESET | CVAR_CHEAT, 0, qtrue  }, // mmp - is 3 in vq3
	{ &g_matchMode, "g_matchMode", "0", CVAR_RULESET, 0, qtrue  },
	{ &g_weaponRespawn, "g_weaponRespawn", "0", CVAR_RULESET, 0, qtrue  },
	{ &g_weaponTeamRespawn, "", "", 0, 0, qtrue }, // mmp - to be fased out
	{ &g_forcerespawn, "g_forcerespawn", "10", CVAR_RULESET, 0, qtrue },

	// change anytime vars
	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse  },

	{ &g_teamAutoJoin, "", "0", CVAR_ROM }, // this shit is broken
	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE, 0, qfalse },

	{ &g_warmup, "g_warmup", "10", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse  },
//	{ &g_doWarmup, "", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue  }, // mmp - to be fased out
	{ &g_logfile, "g_log", "games.log", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_logfileSync, "g_logsync", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_verboseLog, "g_verboseLog", "1", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0, qfalse  },

	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse },

	{ &g_dedicated, "dedicated", "0", 0, 0, qfalse  },

	{ &g_inactivity, "g_inactivity", "0", 0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse },
	{ &g_motd, "g_motd", "", 0, 0, qfalse },
	{ &g_blood, "com_blood", "1", 0, 0, qfalse },

	{ &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse },
	{ &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse },

	{ &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_allowedVoteNames, "g_allowedVoteNames", "/restart/", CVAR_ARCHIVE, 0, qfalse },
	{ &g_voteWaitTime, "g_voteWaitTime", "10", CVAR_ARCHIVE, 0, qfalse },
	{ &g_specChat, "g_specChat", "1", CVAR_ARCHIVE, 0, qtrue },
	{ &g_listEntity, "g_listEntity", "0", 0, 0, qfalse },

#ifdef MISSIONPACK
	{ &g_obeliskHealth, "g_obeliskHealth", "2500", 0, 0, qfalse },
	{ &g_obeliskRegenPeriod, "g_obeliskRegenPeriod", "1", 0, 0, qfalse },
	{ &g_obeliskRegenAmount, "g_obeliskRegenAmount", "15", 0, 0, qfalse },
	{ &g_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO, 0, qfalse },

	{ &g_cubeTimeout, "g_cubeTimeout", "30", 0, 0, qfalse },
	{ &g_redteam, "g_redteam", "redTeam", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue },
	{ &g_blueteam, "g_blueteam", "blueTeam", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO , 0, qtrue, qtrue  },
	{ &g_singlePlayer, "ui_singlePlayerActive", "", 0, 0, qfalse, qfalse  },

	{ &g_enableDust, "g_enableDust", "0", CVAR_SERVERINFO, 0, qtrue, qfalse },
	{ &g_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO, 0, qtrue, qfalse },
	{ &g_proxMineTimeout, "g_proxMineTimeout", "20000", 0, 0, qfalse },
#endif
	{ &g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qtrue},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qtrue},

//unlagged - server options
	{ &g_truePing, "g_truePing", "0", CVAR_ARCHIVE, 0, qfalse },
	// it's CVAR_SYSTEMINFO so the client's sv_fps will be automagically set to its value
	{ &sv_fps, "sv_fps", "", CVAR_SYSTEMINFO | CVAR_ARCHIVE, 0, qfalse },
//unlagged - server options

//unlagged - lagNudge
	{ &g_delagprojectiles, "g_delagprojectiles", "100", CVAR_SYSTEMINFO, 0, qfalse },
//unlagged - lagNudge

// mmp - cvar add

	{ &g_info, "info", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse }, // used by clients
	{ &g_status, "status", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse }, // used by server browser
	{ &g_redTeamCount, "redTeamCount", "0", CVAR_ROM, 0, qfalse }, // used for correct status info
	{ &g_blueTeamCount, "blueTeamCount", "0", CVAR_ROM, 0, qfalse }, // used for correct status info
	{ &g_clientCount, "clientCount", "0", CVAR_ROM, 0, qfalse }, // used for correct status info
	{ &g_playerCount, "playerCount", "0", CVAR_ROM, 0, qfalse }, // used for correct status info
	{ &g_randomByte, "randomByte", "0", CVAR_ROM, 0, qfalse }, // not needed anymore, was used for initial spawn selection - mmp
	{ &g_proMode, "g_proMode", "0", CVAR_SYSTEMINFO | CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH, 0, qfalse}, // elitist mode
	{ &g_teamLock, "g_teamLock", "0", CVAR_USERINFO, 0, qtrue },
	{ &g_bookkeepingLog, "g_bookkeepingLog", "bookkeeping.log", CVAR_ARCHIVE, 0, qfalse },
	{ &g_demeritLimit, "g_demeritLimit", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_quadMode, "g_quadMode", "0", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse }, // quad damage for all
	{ &g_selfDamage, "g_selfDamage", "1", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse },
	{ &g_doubleAmmo, "g_doubleAmmo", "0", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse }, // more ammo
	{ &g_keycardRespawn, "g_keycardRespawn", "0", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse },
	{ &g_keycardDropable, "g_keycardDropable", "0", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse },
	{ &g_noArenaGrenades, "g_noArenaGrenades", "0", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse }, // no spamming
	{ &g_noArenaLightningGun, "g_noArenaLightningGun", "0", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse },
	{ &g_enemyAttackLevel, "g_enemyAttackLevel", "0.5", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse }, // for aa1 gametype
	{ &g_powerUps, "g_powerUps", "1", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse },
	{ &g_armor, "g_armor", "1", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse }, // enable/disable armor spawns
	{ &g_allowGhost, "g_allowGhost", "0", 0, 0, qtrue }, // incomplete, please don't abuse, or it'll become cheat protected
	{ &g_shortGame, "g_shortGame", "0", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse }, // halves timelimit by half
	{ &g_roundFormat, "g_roundFormat", "1", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse }, // enables 3 round matches, 1r = normal, 2r = losing player must have half of leader, 3r = overtime
	{ &g_dynamicItemSpawns, "g_dynamicItemSpawns", "1", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse }, // increases item respawn times as rounds advance

	{ &g_spamLimitCount, "g_spamLimitCount", "4", CVAR_ARCHIVE, 0, qfalse },
	{ &g_spamLimitTimeRange, "g_spamLimitTimeRange", "2000", CVAR_ARCHIVE, 0, qfalse },
	{ &g_allowSpecVote, "g_allowSpecVote", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_allowSpecCallVote, "g_allowSpecCallVote", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_randomSpawn, "g_randomSpawn", "0", CVAR_ARCHIVE | CVAR_RULESET, 0, qfalse },
	{ &g_adminPassword, "g_adminPassword", "", 0, 0, qfalse },
	{ &g_mapRotation, "g_mapRotation", "rotation.cfg", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
	{ &g_mapRotation_ffa, "g_mapRotation_ffa", "rotation_ffa.cfg", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
	{ &g_mapRotation_duel, "g_mapRotation_duel", "rotation_duel.cfg", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
	{ &g_mapRotation_tdm, "g_mapRotation_tdm", "rotation_tdm.cfg", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },
	{ &g_mapRotation_ctf, "g_mapRotation_ctf", "rotation_ctf.cfg", CVAR_ARCHIVE | CVAR_NORESTART, 0, qfalse },

	{ &g_allowedAdminCmds, "g_allowedAdminCmds", "/restart/map/msg/players/gametype/matchMode/proMode/kick/mute/unmute/rpickup/timelimit/scorelimit/ruleSet/teamSize/shortGame/vstr/", CVAR_ARCHIVE, 0, qfalse }, // mmp

// scheduled maintenance vstr server calls
// a call is exec at the end of the match
	{ &g_serviceScheduleSun, "g_serviceScheduleSun", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serviceScheduleMon, "g_serviceScheduleMon", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serviceScheduleTues, "g_serviceScheduleTues", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serviceScheduleWed, "g_serviceScheduleWed", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serviceScheduleThurs, "g_serviceScheduleThurs", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serviceScheduleFri, "g_serviceScheduleFri", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serviceScheduleSat, "g_serviceScheduleSat", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serviceScheduleDaily, "g_serviceScheduleDaily", "", CVAR_ARCHIVE, 0, qfalse },
	{ &g_currentDay, "currentDay", "-1", CVAR_ROM, 0, qfalse }, // keep track of day

// empty server maintenance
	{ &g_serviceOnEmptyTime, "g_serviceOnEmptyTime", "5", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serviceOnEmptyExec, "g_serviceOnEmptyExec", "map_restart", CVAR_ARCHIVE, 0, qfalse },

// mmp - cvar add end

	{ &g_rankings, "g_rankings", "0", 0, 0, qfalse}

};

static int gameCvarTableSize = ARRAY_LEN( gameCvarTable );


void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( int restart );
/*void UpdateSpecMode ( void );*/
void CheckExitRules( void );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
Q_EXPORT intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {
	switch ( command ) {
	case GAME_INIT:
		G_InitGame( arg0, arg1, arg2 );
		return 0;
	case GAME_SHUTDOWN:
		G_ShutdownGame( arg0 );
		return 0;
	case GAME_CLIENT_CONNECT:
		return (intptr_t)ClientConnect( arg0, arg1, arg2 );
	case GAME_CLIENT_THINK:
		ClientThink( arg0 );
		return 0;
	case GAME_CLIENT_USERINFO_CHANGED:
		ClientUserinfoChanged( arg0 );
		return 0;
	case GAME_CLIENT_DISCONNECT:
		ClientDisconnect( arg0 );
		return 0;
	case GAME_CLIENT_BEGIN:
		ClientBegin( arg0 );
		return 0;
	case GAME_CLIENT_COMMAND:
		ClientCommand( arg0 );
		return 0;
	case GAME_RUN_FRAME:
		G_RunFrame( arg0 );
		return 0;
	case GAME_CONSOLE_COMMAND:
		return ConsoleCommand();
	case BOTAI_START_FRAME:
		return BotAIStartFrame( arg0 );
	}

	return -1;
}


void QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	trap_Print( text );
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	trap_Error( text );
}

const char *ShortTeamName(int team)  {
	if (team==TEAM_RED)
		return S_COLOR_RED"[R]"S_COLOR_WHITE;
	else if (team==TEAM_BLUE)
		return S_COLOR_BLUE"[B]"S_COLOR_WHITE;
	else if (team==TEAM_SPECTATOR)
		return S_COLOR_CYAN"[S]"S_COLOR_WHITE;
	return S_COLOR_YELLOW"[F]"S_COLOR_WHITE;
}


/*
==================
G_AllowedVotes
==================
 */
#define MAX_VOTENAME_LENGTH 14 // currently the longest string is "/map_restart/\0" (14 chars)

int G_AllowedVotes ( gentity_t *ent, qboolean printNames ) {

	char	tempStr[MAX_VOTENAME_LENGTH]; // TODO: remove, it's unused
	char	voteNames[MAX_CVAR_VALUE_STRING];
	int		voteFlags = 0;
	char	string[128]; // TODO: remove, it's unused
	qboolean	commaUse = qfalse;

	trap_Cvar_VariableStringBuffer( "g_allowedVoteNames", voteNames, sizeof( voteNames ) );

	// TODO: allow '*' to enable all votes
	// if a star wildcard is present, everything is allowed
	/*if(!Q_stricmp(voteNames, "*" ))
		return 65535;*/

	if ( printNames == qtrue ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Valid commands are:\n-------------------\n\"" ) );
		/*Q_strncpyz( string, "Valid commands are: ", sizeof(string));*/
	}

	if(Q_stristr(voteNames, "/restart/" ) != NULL) {
		voteFlags |= VOTE_MAP_RESTART;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"restart\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/nextmap/" ) != NULL) {
		voteFlags |= VOTE_NEXTMAP;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"nextmap\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/map/" ) != NULL) {
		voteFlags |= VOTE_MAP;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"map <mapname>\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/gametype/" ) != NULL) {
		voteFlags |= VOTE_GAMETYPE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"gametype <n>\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/matchMode/" ) != NULL && level.rulesetEnforced == qfalse) {
		voteFlags |= VOTE_MATCHMODE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"matchMode <n>\n\"" ) );
		}
	}
	if(Q_stristr(voteNames, "/proMode/" ) != NULL) {
		voteFlags |= VOTE_PROMODE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"proMode <0/1>\n\"" ) );
		}
	}
	if(Q_stristr(voteNames, "/clientkick/" ) != NULL) {
		voteFlags |= VOTE_CLIENTKICK;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"clientkick <clientnum>\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/kick/" ) != NULL) {
		voteFlags |= VOTE_KICK;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"kick <player>\n\"" ) );

		}
	}
	/*if(Q_stristr(voteNames, "/clientmute/" ) != NULL) {
		voteFlags |= VOTE_CLIENTMUTE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"clientmute <clientnum>\n\"" ) );

		}
	}*/
	if(Q_stristr(voteNames, "/mute/" ) != NULL) {
		voteFlags |= VOTE_MUTE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"mute <player>\n\"" ) );

		}
	}
	/*if(Q_stristr(voteNames, "/g_doWarmup/" ) != NULL) {
		voteFlags |= VOTE_DO_WARMUP;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"g_doWarmup\n\"" ) );

		}
	}*/
	if(Q_stristr(voteNames, "/endWarmup/" ) != NULL) {
		voteFlags |= VOTE_ENDWARMUP;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"endWarmup\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/rpickup/" ) != NULL && g_gametype.integer >= GT_TEAM) {
		voteFlags |= VOTE_RPICKUP;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"rpickup\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/giveadmin/" ) != NULL) {
		voteFlags |= VOTE_GIVEADMIN;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"giveadmin <player>\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/timelimit/" ) != NULL && level.rulesetEnforced == qfalse) {
		voteFlags |= VOTE_TIMELIMIT;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"timelimit <minute>\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/scorelimit/" ) != NULL && level.rulesetEnforced == qfalse) {
		voteFlags |= VOTE_SCORELIMIT;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"scorelimit <score>\n\"" ) );

		}
	}
	if(Q_stristr(voteNames, "/ruleSet/" ) != NULL) {
		voteFlags |= VOTE_RULESET;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"ruleset <n>\n\"" ) );
		}
	}
	if(Q_stristr(voteNames, "/teamSize/" ) != NULL && g_gametype.integer >= GT_TEAM) {
		voteFlags |= VOTE_TEAMSIZE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"teamsize <n>\n\"" ) );
		}
	}
	if(Q_stristr(voteNames, "/shortGame/" ) != NULL) {
		voteFlags |= VOTE_SHORTGAME;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"shortGame <0/1>\n\"" ) );
		}
	}
	if(Q_stristr(voteNames, "/bots/" ) != NULL) {
		voteFlags |= VOTE_BOTS;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"bots <n>\n\"" ) );
		}
	}
	if(Q_stristr(voteNames, "/ext/" ) != NULL) {
		voteFlags |= VOTE_EXT;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"ext <option>\n\"" ) );
		}
	}

	if ( printNames == qtrue ) {
		if (voteFlags == 0)
			trap_SendServerCommand( ent-g_entities, va("print \"... Um, nothing, i guess.\n\"" ) );
			/*strcat( string, "... Um, nothing, i guess" );*/

		/*trap_SendServerCommand( ent-g_entities, va("notify %i\\\"%s.\n\"", NF_ERROR, string ) );*/
	}

	return voteFlags;

}


/*
==================
ClientForString

move from g_svcmds.c
==================
*/

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	G_Printf( "User %s is not on the server\n", s );

	return NULL;
}


/*
==================
ClientNumForString

move from g_svcmds.c
==================
*/

int		ClientNumForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return -1;
		}
		return idnum;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return i;
		}
	}

	G_Printf( "User %s is not on the server\n", s );

	return -1;
}


/*
==================
G_RandomSelectUpdate
==================
*/

void G_RandomSelectUpdate ( void ) {
	int		count;
	int		rnd;

	if ( !level.rnd_mode ) {
		return; // no cointoss in progress
	}

	if ( level.time < level.rnd_nextthink ) {
		return; // wait
	}

	if ( level.rnd_mode == 1 ) {

		switch ( level.rnd_type ) {
			case RT_COINTOSS:
				trap_SendServerCommand( -1, va("notify %i\\\"COINTOSS IN:\n\"",
					NF_GAMEINFO) );
				break;

			case RT_RNDSELECT:
				trap_SendServerCommand( -1, va("notify %i\\\"RANDOM SELECTION IN:\n\"",
					NF_GAMEINFO) );
				break;

			case RT_RNDNUM:
				trap_SendServerCommand( -1, va("notify %i\\\"RANDOM NUMBER IN:\n\"",
					NF_GAMEINFO) );
				break;
		}
		level.rnd_mode++;
		level.rnd_nextthink = level.time + 1000;
		return;

	} else {

		count = 5 - level.rnd_mode;
		if ( count == 0 ) {
			switch ( level.rnd_type ) {
				case RT_COINTOSS:
					rnd = (random()) * 65536;
					if ( rnd & 1 ) {
						trap_SendServerCommand( -1, va("notify %i\\\"RESULT: " S_COLOR_YELLOW "HEADS!\n\"",
							NF_GAMEINFO) );
					} else {
						trap_SendServerCommand( -1, va("notify %i\\\"RESULT: " S_COLOR_RED "TAILS!\n\"",
							NF_GAMEINFO) );
					}
					break;

				case RT_RNDSELECT:
					trap_SendServerCommand( -1, va("notify %i\\\"SELECTED: %s\n\"",
						NF_GAMEINFO, level.rnd_selected) );
					break;

				case RT_RNDNUM:
					rnd = (random()) * ((level.rnd_maxNum - level.rnd_minNum) + 1);
					trap_SendServerCommand( -1, va("notify %i\\\"SELECTED: %i\n\"",
						NF_GAMEINFO, level.rnd_minNum + rnd) );
					break;
			}
			level.rnd_mode = 0; // clear cointoss mode
			return;
		}
		trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_GRAY "[" S_COLOR_WHITE "%i" S_COLOR_GRAY "]\n\"",
			NF_GAMEINFO, count) );
		level.rnd_mode++;
		level.rnd_nextthink = level.time + 1000;

	}

}

/*
==================
G_UnreadyPlayers

Remove ready status from players in warm-up
==================
*/

void G_UnreadyPlayers ( void ) {

	gclient_t	*cl;
	int			i;

	// remove ready status from clients
	for ( i=0 ; i < level.numPlayingClients ; i++ ) {
		g_entities[level.sortedClients[i]].client->pers.ready = qfalse;
	}

}

/*
==================
G_SpamCheck
==================
*/

void G_SpamCheck ( gentity_t *ent ) {

	int		spamRate;
	int		timeSet;

	// anything under 4 is insane
	if ( g_spamLimitCount.integer < 4 ) {
		return;
	}

	// anything over 4000 is insane, and anything under 250 is useless
	if ( g_spamLimitTimeRange.integer < 250 || g_spamLimitTimeRange.integer > 4000 ) {
		return;
	}

	//G_Printf ("^8DEBUG: SPAM TEST START - %i > %i ?\n", ent->client->pers.lastSpamTime, level.time - 2000);

	if ( ent->client->pers.lastSpamTime > ( level.time - g_spamLimitTimeRange.integer ) ) {

		//G_Printf ("^8DEBUG: SPAM RESET TEST - %i < %i ?\n", ent->client->pers.lastSpamResetTime, level.time - 4000);
		if ( ent->client->pers.lastSpamResetTime < ( level.time - ( g_spamLimitTimeRange.integer * 2 ) ) ) {
			ent->client->pers.lastSpamResetTime = level.time;
			timeSet = 1;
			ent->client->pers.spamCount = 2;
		} else {
			timeSet = ( ( level.time - ent->client->pers.lastSpamResetTime ) / g_spamLimitTimeRange.integer ) + 1;
			ent->client->pers.spamCount++;
		}
		if ( timeSet < 1 ) {
			timeSet = 1;
		}
		spamRate = ent->client->pers.spamCount / timeSet;
		//G_Printf ("^9DEBUG: SPAM TEST - %i / %i = %i\n", ent->client->pers.spamCount, timeSet, spamRate);

		// mute the spammer
		if ( spamRate >= ( g_spamLimitCount.integer ) ) {
			ent->client->sess.mute = qtrue;
			trap_SendServerCommand( -1,
				va("notify %i\\\"%s" S_COLOR_RED " has been muted, due to spamming.\n\"",
				NF_GAMEINFO, ent->client->pers.netname) );
		}

	}

	ent->client->pers.lastSpamTime = level.time;

}

/*
==================
G_TimeCompulsionUpdate

makes matches going into overtime or sudden death go faster
==================
*/

void G_TimeCompulsionUpdate (void) {

	if ( (level.time - level.startTime) >= level.rs_timelimit * 60000 ) {

		// are we in warmup?
		if ( level.warmupTime ) {
			return;
		}

		if ( level.time > level.c_timeUpdate ) {

			// don't bother with first increase
			if ( level.c_timeUpdate ) {

				// increase damage by 1/8 the normal damage
				if ( level.c_extraDamage < 3.999f ) {
					level.c_extraDamage += 0.125f; // increase extra damage til damage becomes quad damage
				}

				// increase idle flag returns by 1 second
				if ( level.c_flagReturnDecrease < DEC_FLAG_RETURN_TIME ) {
					level.c_flagReturnDecrease += 1;
				}

				// don't delay respawns in tournament
				if ( g_gametype.integer != GT_TOURNAMENT ) {
					if ( level.c_spawnDelay < 3300 ) {
						level.c_spawnDelay += 150;
					}
				}

				/*trap_SendServerCommand( -1,
					va("notify %i\\\"" S_COLOR_PINK "DEBUG= %f, %i, %i\n\"",
					NF_GAMEINFO, level.c_extraDamage, level.c_flagReturnDecrease, level.c_spawnDelay) );*/

				// debug
				/*trap_SendServerCommand( -1,
					va("notify %i\\\"" S_COLOR_RED "DEBUG= %i\n\"",
					NF_GAMEINFO, level.c_timeUpdate) );*/

				level.c_timeUpdate += 15000;

				// shouldn't happen, but just incase
				if ( level.time > level.c_timeUpdate ) {
					level.c_timeUpdate = 15000 + level.time;
				}

			} else {
				level.c_timeUpdate = 15000 + level.time;
			}

		}

	}
}


/*
==================
G_InfoUpdate

for use with 'g_info'
also see enum table 'infoUsage_t' in 'q_shared.h'
==================
*/

void G_InfoUpdate ( void ) {
	char		string[1024] = {""};
	char		afterString[1024] = {""};
//	char		*result;
//	int			info[INFO_MAXPOS];
	int			value;
	int			ckSum = 0;

	level.updateInfo = qfalse;

	// version
	Q_strcat ( string, sizeof(string), Com_ByteToHex( SVINFO_VERSION_YR ) );
	Q_strcat ( string, sizeof(string), Com_ByteToHex( SVINFO_VERSION_SV ) );

	// checksum would go in between these two values later

	// gametype
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( g_gametype.integer ) );
	ckSum += g_gametype.integer;

	// client count
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( g_clientCount.integer ) );
	ckSum += g_clientCount.integer;

	// active player count
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( g_playerCount.integer ) );
	ckSum += g_playerCount.integer;


	// info bit flags
	value = 0;
	if ( level.rulesetEnforced == qtrue )
		value |= INFO_BIT_RULESET;
	if ( level.rs_friendlyFire )
		value |= INFO_BIT_FRIENDLYFIRE;
	if ( level.rs_teamLocOverlay )
		value |= INFO_BIT_TEAMLOCOVERLAY;
	if ( level.rs_hitSound )
		value |= INFO_BIT_HITSOUND;
	if ( level.rs_randomSpawn )
		value |= INFO_BIT_RANDOMSPAWN;
	if ( g_proMode.integer == 1 )
		value |= INFO_BIT_FREESELECT_PHYSICS;
	if ( level.rs_quadMode )
		value |= INFO_BIT_QUADMODE;
	if ( level.rs_selfDamage )
		value |= INFO_BIT_SELFDAMAGE;
	if ( level.rs_doubleAmmo )
		value |= INFO_BIT_DOUBLEAMMO;
	if ( level.rs_keycardDropable )
		value |= INFO_BIT_KEYCARD_DROPABLE;
	if ( level.rs_randomSpawn )
		value |= INFO_BIT_SCOREBALANCE;
	if ( level.rs_noArenaGrenades )
		value |= INFO_BIT_NO_ARENA_GRENADES;
	if ( level.rs_noArenaLightningGun )
		value |= INFO_BIT_NO_ARENA_LIGHTNINGGUN;
	if ( level.rs_powerUps )
		value |= INFO_BIT_POWERUPS;
	if ( level.rs_armor )
		value |= INFO_BIT_ARMOR;
	if ( level.rs_popCTF )
		value |= INFO_BIT_POPCTF;

	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( value ) ); // only the lowest 8 bits are read in Com_ByteToHex()
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( value >> 8 ) );
	ckSum += value + ( value >> 8 );


	// timelimit
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_timelimit ) );
	ckSum += level.rs_timelimit;

	// overtime
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_overtime ) );
	ckSum += level.rs_overtime;

	// scorelimit 2 bytes
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_scorelimit ) ); // only the lowest 8 bits are read in Com_ByteToHex()
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_scorelimit >> 8 ) );
	ckSum += level.rs_scorelimit + ( level.rs_scorelimit >> 8 );

	// mercylimit 2 bytes
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_mercylimit ) ); // only the lowest 8 bits are read in Com_ByteToHex()
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_mercylimit >> 8 ) );
	ckSum += level.rs_mercylimit + ( level.rs_mercylimit >> 8 );


	// match mode
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_matchMode ) );
	ckSum += level.rs_matchMode;

	// weapon respawn time
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_weaponRespawn ) );
	ckSum += level.rs_weaponRespawn;

	// forced player respawn time
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_forceRespawn ) );
	ckSum += level.rs_forceRespawn;

	// keycard respawn time
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( level.rs_keycardRespawn ) );
	ckSum += level.rs_keycardRespawn;

	// enemy attack level
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( (int)(level.rs_enemyAttackLevel * 10) ) );
	ckSum += (int)(level.rs_enemyAttackLevel * 10);

	// extended info bit flags
	value = 0;
	if ( g_shortGame.integer )
		value |= INFO_BIT_SHORTGAME;
	if ( level.rs_roundFormat )
		value |= INFO_BIT_ROUND_BASED_MATCHES;

	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( value ) ); // only the lowest 8 bits are read in Com_ByteToHex()
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( value >> 8 ) );
	ckSum += value + ( value >> 8 );

	// teamsize
	Q_strcat ( afterString, sizeof(afterString), Com_ByteToHex( g_teamSize.integer ) );
	ckSum += g_teamSize.integer;


	// check sum (8 bit)
	Q_strcat ( string, sizeof(string), Com_ByteToHex( ckSum ) );

	// merge strings together
	Q_strcat ( string, sizeof(string), afterString ); // won't use until the game goes into beta

	// update info cvar
	trap_Cvar_Set( "info", va("%s", string) );
//	G_Printf ("^3TEST INFOUPDATE = '%s'\n", string); // debug

}

/*
==================
G_UpdateStatus

for use with 'g_status'
==================
*/

void G_UpdateStatus ( void ) {

	int		min;

	if ( level.time < level.lastStatusTime  ) {
		return;
	}

	level.lastStatusTime = level.time + 5000; // update game status every 5 seconds

	if ( level.warmupTime ) {
		trap_Cvar_Set( "status", "Warmup" );
		return;
	}

	if ( level.matchEnded == qtrue ) {
		trap_Cvar_Set( "status", "Intermission" );
		return;
	}

	if ( level.rs_timelimit < 1 ) {
		trap_Cvar_Set( "status", "Active" );
		return;
	}

	min = ( ( level.rs_timelimit * 60000 ) - ( level.time - level.startTime ) ) / 60000;

	if ( min < 0 ) {
		if ( level.rs_overtime > 0 ) {
			trap_Cvar_Set( "status", "Overtime" );
		} else {
			trap_Cvar_Set( "status", "Sudden death" );
		}
		return;
	}

	if ( min < 1 ) {
		trap_Cvar_Set( "status", "Less than a minute left" );
		return;
	}

	if ( min < 2 ) {
		trap_Cvar_Set( "status", "1 minute left" );
		return;
	}

	trap_Cvar_Set( "status", va("%i minutes left", min) );
	return;

}

/*
==================
G_TeamCount

for use with 'g_info'
==================
*/

void G_TeamCount ( void ) {
	int		count;

	// TODO: merge this function with the function below
	count = TeamCount( -1, TEAM_RED );
	trap_Cvar_Set( "redTeamCount", va("%i", count) );
	//G_Printf ("^8DEBUG: RED=%i\n", count);
	count = TeamCount( -1, TEAM_BLUE );
	trap_Cvar_Set( "blueTeamCount", va("%i", count) );
	//G_Printf ("^8DEBUG: BLUE=%i\n", count);

}

/*
==================
G_ClientCount

for use with 'g_info'
==================
*/

void G_ClientCount ( void ) {

	int		i;
	int		playerCount = 0;
	int		clientCount = 0;

	// count players
	for ( i = 0 ; i < level.maxclients ; i++ ) {

		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue; // skip empty client slots
		}

		clientCount++; // this is a client

		if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) {
			playerCount++; // this client is active
		}

	}

	// update player count cvars
	trap_Cvar_Set( "clientCount", va("%i", clientCount) );
	trap_Cvar_Set( "playerCount", va("%i", playerCount) );

	level.updateInfo = qtrue;

}


/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i=1, e=g_entities+i ; i < level.num_entities ; i++,e++ ){
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	G_Printf ("%i teams with %i entities\n", c, c2);
}

void G_RemapTeamShaders( void ) {
#ifdef MISSIONPACK
	char string[1024];
	float f = level.time * 0.001;
	Com_sprintf( string, sizeof(string), "team_icon/%s_red", g_redteam.string );
	AddRemap("textures/ctf2/redteam01", string, f);
	AddRemap("textures/ctf2/redteam02", string, f);
	Com_sprintf( string, sizeof(string), "team_icon/%s_blue", g_blueteam.string );
	AddRemap("textures/ctf2/blueteam01", string, f);
	AddRemap("textures/ctf2/blueteam02", string, f);
	trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
#endif
}

/*
==================
G_RuleSetUpdate

This is called at 'G_InitGame', and when a ruleset cvar is updated
Besure to update 'g_client.c' at "send ruleset info to client" part in ClientBegin()
==================
*/

void G_TeamSizeRuleSet ( void ) {

	if ( g_teamSize.integer > 0 && g_teamSize.integer < 4 ) {
		// match is no bigger than a 3on3 match
		level.rs_timelimit = 10;

		// no powerups on 2on2 matches
		if ( g_teamSize.integer < 3 ) {
			level.rs_powerUps = 0;
		} else {
			level.rs_powerUps = 1;
		}
	} else if ( g_teamSize.integer <= 0 ) {
		// configure timelimit based on team count
		if (g_redTeamCount.integer > 3 && g_blueTeamCount.integer > 3) {
			level.rs_timelimit = 20;
		} else {
			level.rs_timelimit = 10;
		}

		// enable powerups based on team count
		if (g_redTeamCount.integer > 2 && g_blueTeamCount.integer > 2) {
			level.rs_powerUps = 1;
		} else {
			level.rs_powerUps = 0;
		}

	} else {
		// match is at least a 4on4 match
		level.rs_timelimit = 20;
		level.rs_powerUps = 1;
	}

}

void G_RuleSetUpdate ( void ) {

	int		counts[TEAM_NUM_TEAMS];

	//G_Printf( "^1DEBUG: g_ruleset=%i\n", g_ruleset.integer );
	level.updateRuleset = qfalse; // we are now updating the ruleset
	level.forcefullyUpdateRuleset = qfalse; // likewise

	if ( g_ruleset.integer > RSET_CUSTOM && g_ruleset.integer < RSET_MAX_RULESET ) {
		level.rulesetEnforced = qtrue;

		level.rs_armor = 1;
		level.rs_popCTF = 0;
		level.rs_dynamicItemSpawns = 1;

		switch ( g_gametype.integer ) {

			case GT_TOURNAMENT:
			case GT_TEAM:
			case GT_CTF:
				level.rs_roundFormat = 1;
				break;

			default:
				level.rs_roundFormat = 0;

		}
	}

	switch ( g_ruleset.integer ) {
		case 1:
			/*level.rs_speed = 400;
			level.rs_gravity = 1000;
			level.rs_knockback = 1250;
			level.rs_quadFactor = 4;*/

			level.rs_teamLocOverlay = 1;
			level.rs_hitSound = 0;
			level.rs_randomSpawn = 0;
			level.rs_scoreBalance = 1;

			level.rs_quadMode = 0;
			level.rs_selfDamage = 1;

			level.rs_doubleAmmo = 0;
			level.rs_keycardRespawn = 0;
			level.rs_keycardDropable = 0;

			level.rs_noArenaGrenades = 0;
			level.rs_noArenaLightningGun = 0;
			level.rs_enemyAttackLevel = 0.5;

			level.rs_matchMode = MM_PICKUP_ONCE;

			if ( g_gametype.integer == GT_TOURNAMENT ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0; //20;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 5;
				level.rs_powerUps = 0;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_AA1 ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_TEAM ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //50;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_CTF ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0; //1000;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;

				level.rs_warmup = 10;
			} else { /* GT_FFA... */
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 69;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 0;
			}

			break;

		case 2:

			level.rs_teamLocOverlay = 0;
			level.rs_hitSound = 0;
			level.rs_randomSpawn = 1;
			level.rs_scoreBalance = 1;

			level.rs_quadMode = 0;
			level.rs_selfDamage = 1;

			level.rs_doubleAmmo = 0;
			level.rs_keycardRespawn = 30;
			level.rs_keycardDropable = 1;

			level.rs_noArenaGrenades = 0;
			level.rs_noArenaLightningGun = 0;
			level.rs_enemyAttackLevel = 0.5;

			level.rs_matchMode = MM_PICKUP_ALWAYS;

			if ( g_gametype.integer == GT_TOURNAMENT ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0; //20;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 30;
				level.rs_forceRespawn = 5;
				level.rs_powerUps = 0;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_AA1 ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_TEAM ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //50;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 30;
				level.rs_forceRespawn = 5;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_CTF ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //1000;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 15;
				level.rs_forceRespawn = 10;

				level.rs_warmup = 10;
			} else { /* GT_FFA... */
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 666; // was 35
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 10;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 0;
			}

			break;

		case 3:

			level.rs_teamLocOverlay = 0;
			level.rs_hitSound = 0;
			level.rs_randomSpawn = 1;
			level.rs_scoreBalance = 1;

			level.rs_quadMode = 1;
			level.rs_selfDamage = 1;

			level.rs_doubleAmmo = 0;
			level.rs_keycardRespawn = 30;
			level.rs_keycardDropable = 1;

			level.rs_noArenaGrenades = 0;
			level.rs_noArenaLightningGun = 0;
			level.rs_enemyAttackLevel = 1.0;

			level.rs_matchMode = MM_PICKUP_ALWAYS;

			if ( g_gametype.integer == GT_TOURNAMENT ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0; //20;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 30;
				level.rs_forceRespawn = 5;
				level.rs_powerUps = 0;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_AA1 ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_TEAM ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //50;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 30;
				level.rs_forceRespawn = 5;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_CTF ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //1000;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 15;
				level.rs_forceRespawn = 10;

				level.rs_warmup = 10;
			} else { /* GT_FFA... */
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 666; // was 35
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 10;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 0;
			}

			break;

		case 4:

			level.rs_teamLocOverlay = 1;
			level.rs_hitSound = 0;
			level.rs_randomSpawn = 1;
			level.rs_scoreBalance = 1;

			level.rs_quadMode = 0;
			level.rs_selfDamage = 0;

			level.rs_doubleAmmo = 1;
			level.rs_keycardRespawn = 0;
			level.rs_keycardDropable = 0;

			level.rs_noArenaGrenades = 0;
			level.rs_noArenaLightningGun = 0;
			level.rs_enemyAttackLevel = 0.5;

			level.rs_matchMode = MM_ALLWEAPONS;

			if ( g_gametype.integer == GT_TOURNAMENT ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0; //20;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 0;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 5;
				level.rs_powerUps = 0;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_AA1 ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_TEAM ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //50;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 0;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_CTF ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //1000;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 0;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;

				level.rs_warmup = 10;
			} else { /* GT_FFA... */
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 0;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 0;
			}

			break;

		case 5:

			level.rs_teamLocOverlay = 0;
			level.rs_hitSound = 0;
			level.rs_randomSpawn = 1;
			level.rs_scoreBalance = 1;

			level.rs_quadMode = 0;
			level.rs_selfDamage = 1;

			level.rs_doubleAmmo = 0;
			level.rs_keycardRespawn = 30;
			level.rs_keycardDropable = 1;

			level.rs_noArenaGrenades = 0;
			level.rs_noArenaLightningGun = 0;
			level.rs_enemyAttackLevel = 0.5;

			level.rs_matchMode = MM_ROCKET_MANIAX;

			if ( g_gametype.integer == GT_TOURNAMENT ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0; //20;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 30;
				level.rs_forceRespawn = 5;
				level.rs_powerUps = 0;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_AA1 ) {
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 0;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_TEAM ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //50;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 30;
				level.rs_forceRespawn = 5;

				level.rs_warmup = 10;
			} else if ( g_gametype.integer == GT_CTF ) {
				G_TeamSizeRuleSet();

				level.rs_mercylimit = 0; //1000;
				level.rs_overtime = 2;
				level.rs_scorelimit = 0;
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 15;
				level.rs_forceRespawn = 10;

				level.rs_warmup = 10;
			} else { /* GT_FFA... */
				level.rs_timelimit = 10;
				level.rs_mercylimit = 0;
				level.rs_overtime = 0;
				level.rs_scorelimit = 666; // was 35
				level.rs_friendlyFire = 1;
				level.rs_weaponRespawn = 10;
				level.rs_forceRespawn = 10;
				level.rs_powerUps = 1;

				level.rs_warmup = 0;
			}

			break;

		default:
			level.rulesetEnforced = qfalse;

			level.rs_timelimit = g_timelimit.integer;
			level.rs_mercylimit = g_mercylimit.integer;
			level.rs_overtime = g_overtime.integer;
			level.rs_scorelimit = g_scorelimit.integer;
			level.rs_friendlyFire = g_friendlyFire.integer;
			level.rs_matchMode = g_matchMode.integer;
			if (level.rs_matchMode >= MM_NUM_MMODES ) {
				level.rs_matchMode = MM_PICKUP_ONCE;
			}
			level.rs_weaponRespawn = g_weaponRespawn.integer;
			level.rs_forceRespawn = g_forcerespawn.integer;
			level.rs_teamLocOverlay = g_teamLocOverlay.integer;
			level.rs_hitSound = g_hitSound.integer;
			level.rs_randomSpawn = g_randomSpawn.integer;
			level.rs_scoreBalance = g_scoreBalance.integer;

			level.rs_quadMode = g_quadMode.integer;
			level.rs_selfDamage = g_selfDamage.integer;
			level.rs_doubleAmmo = g_doubleAmmo.integer;
			level.rs_keycardRespawn = g_keycardRespawn.integer;
			level.rs_keycardDropable = g_keycardDropable.integer;

			level.rs_noArenaGrenades = g_noArenaGrenades.integer;
			level.rs_noArenaLightningGun = g_noArenaLightningGun.integer;
			level.rs_enemyAttackLevel = g_enemyAttackLevel.value;
			level.rs_powerUps = g_powerUps.integer;
			level.rs_armor = g_armor.integer;
			level.rs_popCTF = 0;
			level.rs_dynamicItemSpawns = g_dynamicItemSpawns.integer;

			if ( g_gametype.integer == GT_FFA || g_gametype.integer == GT_AA1 ) {
				level.rs_roundFormat = 0;
			} else {
				level.rs_roundFormat = g_roundFormat.integer;
			}

			level.rs_warmup = g_warmup.integer;
			if ( level.rs_warmup == 1 ) {
				// workaround for a warmup related bug
				// TODO: fix it
				level.rs_warmup = 2;
			}

	}

	// map specified ruleset modifications
	if ( g_ruleset.integer > 0 && g_ruleset.integer <= 4 ) {

		if ( g_gametype.integer == GT_FFA ) {
			if ( level.rsmod_timelimit_dm > 0 && level.rsmod_timelimit_dm <= 100 ) {
				level.rs_timelimit = level.rsmod_timelimit_dm - 1;
			}
			if ( level.rsmod_overtime_dm > 0 && level.rsmod_overtime_dm <= 100 ) {
				level.rs_overtime = level.rsmod_overtime_dm - 1;
			}
		}
		else
		if ( g_gametype.integer == GT_TOURNAMENT ) {
			if ( level.rsmod_timelimit_tourney > 0 && level.rsmod_timelimit_tourney <= 100 ) {
				level.rs_timelimit = level.rsmod_timelimit_tourney - 1;
			}
			if ( level.rsmod_overtime_tourney > 0 && level.rsmod_overtime_tourney <= 100 ) {
				level.rs_overtime = level.rsmod_overtime_tourney - 1;
			}
		}
		else
		if ( g_gametype.integer == GT_TEAM ) {
			if ( level.rsmod_timelimit_team > 0 && level.rsmod_timelimit_team <= 100 ) {
				level.rs_timelimit = level.rsmod_timelimit_team - 1;
			}
			if ( level.rsmod_overtime_team > 0 && level.rsmod_overtime_team <= 100 ) {
				level.rs_overtime = level.rsmod_overtime_team - 1;
			}
		}
		else
		if ( g_gametype.integer == GT_CTF ) {
			if ( level.rsmod_timelimit_ctf > 0 && level.rsmod_timelimit_ctf <= 100 ) {
				level.rs_timelimit = level.rsmod_timelimit_ctf - 1;
			}
			if ( level.rsmod_overtime_ctf > 0 && level.rsmod_overtime_ctf <= 100 ) {
				level.rs_overtime = level.rsmod_overtime_ctf - 1;
			}
		}
		else
		{
			if ( level.rsmod_timelimit_else > 0 && level.rsmod_timelimit_else <= 100 ) {
				level.rs_timelimit = level.rsmod_timelimit_else - 1;
			}
			if ( level.rsmod_overtime_else > 0 && level.rsmod_overtime_else <= 100 ) {
				level.rs_overtime = level.rsmod_overtime_else - 1;
			}
		}

		if ( level.rsmod_matchMode > 0 && level.rsmod_matchMode <= MM_NUM_MMODES ) {
			level.rs_matchMode = level.rsmod_matchMode - 1;
		}

		/*G_Printf ("^8DEBUG: %i\n", level.rsmod_timelimit_dm);
		G_Printf ("^9DEBUG: %i\n", level.rs_timelimit);*/
	}


	// check if warmup was init'd, and prevents from getting out of it via a ruleset change
	// this will reset when map is entered
	if ( level.warmupRunning == qtrue ) {
		level.rs_warmup = 10; // prevent from getting out of warmup once set
	} else {
		if ( level.rs_warmup > 1 ) {
			level.warmupRunning = qtrue; // it's true that we're in warmup
		}
	}

	// send ruleset info
	// TODO: make this into a function
	/*trap_SendServerCommand( -1, va("ruleSet %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
					g_ruleset.integer,
					level.rs_timelimit, level.rs_overtime, level.rs_scorelimit, g_proMode.integer, level.rs_friendlyFire,
					level.rs_weaponRespawn, level.rs_forceRespawn, level.rs_teamLocOverlay, level.rs_hitSound, level.rs_scoreBalance,
					level.rs_quadMode, level.rs_selfDamage, level.rs_doubleAmmo, level.rs_keycardRespawn, level.rs_keycardDropable,
					level.rs_randomSpawn
					) );*/
	// also, any updates here, you must update in 'ClientBegin()' of 'g_client.c' as well

	// update server info
	level.updateInfo = qtrue;

}

/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;

#ifdef MISSIONPACK
		if (cv->teamShader) {
			remapped = qtrue;
		}
#endif
	}

#ifdef MISSIONPACK
	if (remapped) {
		G_RemapTeamShaders();
	}
#endif

	// check some things
	if ( g_gametype.integer < 0 || g_gametype.integer >= GT_MAX_GAME_TYPE ) {
		G_Printf( "g_gametype %i is out of range, defaulting to 0\n", g_gametype.integer );
		trap_Cvar_Set( "g_gametype", "0" );
		trap_Cvar_Update( &g_gametype );
	}

	// update ruleset
	/*G_RuleSetUpdate();*/

	level.warmupModificationCount = g_warmup.modificationCount;
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"",
						cv->cvarName, cv->vmCvar->string ) );
				}

#ifdef MISSIONPACK
				if (cv->teamShader) {
					remapped = qtrue;
				}
#endif

				// check cvars for updates
				/*if ( cv->vmCvar == &g_gameMode ) {
					G_RuleSetUpdate ();
				}*/

				// check cvars for updates
				if ( cv->vmCvar == &g_demeritLimit ) {
					if ( g_demeritLimit.integer > 0 && g_demeritLimit.integer < 10 ) {
						G_Printf( "g_demeritLimit has an invalid range of %i, function will not be active\n",
											g_demeritLimit.integer );
					}
				}

				if ( cv->vmCvar == &g_spamLimitCount ) {
					if ( g_spamLimitCount.integer > 0 && g_spamLimitCount.integer < 4 ) {
						G_Printf( "g_spamLimitCount has an invalid range of %i, function will not be active\n",
											g_spamLimitCount.integer );
					}
				}

				if ( cv->vmCvar == &g_spamLimitTimeRange ) {
					if ( g_spamLimitTimeRange.integer < 250 || g_spamLimitTimeRange.integer > 4000 ) {
						G_Printf( "g_spamLimitTimeRange has an invalid range of %i, function will not be active\n",
											g_spamLimitTimeRange.integer );
					}
				}

				// check for ruleset updates
				if ( cv->cvarFlags & CVAR_RULESET ) {
					if ( !level.warmupTime ) {
						level.rulesetViolated = qtrue; // match is disqualified
					} else {
						G_RuleSetUpdate ();
					}
				}

			}
		}
	}

#ifdef MISSIONPACK
	if (remapped) {
		G_RemapTeamShaders();
	}
#endif
}

/*
============
G_DisableLameServer
============
*/

void G_DisableLameServer( void ) {

	char		gameMod[MAX_STRING_CHARS];
	int	clientAllowCount;

	// forces a stardard to weed out lame administrators,
	// by locking out their dedicated server from the master server list

	trap_Cvar_VariableStringBuffer("fs_game", gameMod, sizeof(gameMod));

	//G_Printf ("DEBUG: '%s'\n", gameMod);

	// this is a prototype demo, mods should not be made yet
	// will be removed when game becomes beta
	if ( gameMod[0] != '\0' ) {
		trap_Cvar_Set( "dedicated", "1" );
		G_Printf ("WARNING: This is a prototype demo, mods should not be made yet, dedicated has been set to 1\n");
		return;
	}

	// if you're not running a mod, leave 'fs_game' blank ffs
	if ( !strcmp( gameMod, "basemf") ) {
		trap_Cvar_Set( "dedicated", "1" );
		G_Printf ("WARNING: Cvar 'fs_game' should not be set to 'basemf', dedicated has been set to 1\n");
		return;
	}

	clientAllowCount = g_maxclients.integer - (int)trap_Cvar_VariableValue("sv_privateClients");
	if ( ( ( (g_gametype.integer == GT_TOURNAMENT && clientAllowCount < 8)
				|| (g_gametype.integer != GT_TOURNAMENT && clientAllowCount < 16) )
				|| !trap_Cvar_VariableIntegerValue( "sv_pure" ) ) && !g_needpass.integer ) {
		trap_Cvar_Set( "dedicated", "1" );
		G_Printf ("WARNING: Due to settings being below MFArena standards, dedicated has been set to 1\n");
	}

}

/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed, int restart ) {
	int		i;
	qtime_t		timeStamp;

	G_Printf ("------- Game Initialization -------\n");
	G_Printf ("gamename: %s\n", GAMEVERSION);
	G_Printf ("gamedate: %s\n", __DATE__);
	G_Printf ("product version: %s\n", PRODUCT_VERSION);

	srand( randomSeed );
	trap_Cvar_Set( "randomByte", va("%i", (randomSeed >> 2) & 255) );
	//G_Printf ("^8DEBUG: random=%i\n", randomSeed);

	G_RegisterCvars();

	G_ProcessIPBans();

	G_InitMemory();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;
	level.voteCoolDownStart = levelTime;

	// clear player stats for the next map - mmp
	memset( &stats, 0, sizeof( stats ) );

	if (g_dedicated.integer > 1) {
		// mmp - this is a prototype, so disable broadcasting to some master server
		/*trap_Cvar_Set( "dedicated", "1" );
		G_Printf ("WARNING: This game is a demo, and not meant to be ran as a server, sorry.  'dedicated' has been set to 1\n");*/

		G_DisableLameServer(); // check if server admin is a n00b
	}
	// mmp - this is a prototype, we will disable downloading, so basemf is not filled with unwanted material
	/*if (g_dedicated.integer > 0) {
		trap_Cvar_Set( "sv_dlRate", "0" );
		trap_Cvar_Set( "sv_dlURL", "" );
		trap_Cvar_Set( "sv_allowDownload", "0" );
	}*/

	level.snd_fry = G_SoundIndex("sound/player/fry.wav");	// FIXME standing in lava / slime

	if ( g_gametype.integer != GT_SINGLE_PLAYER && g_logfile.string[0] ) {
		if ( g_logfileSync.integer ) {
			trap_FS_FOpenFile( g_logfile.string, &level.logFile, FS_APPEND_SYNC );
		} else {
			trap_FS_FOpenFile( g_logfile.string, &level.logFile, FS_APPEND );
		}
		if ( !level.logFile ) {
			G_Printf( "WARNING: Couldn't open logfile: %s\n", g_logfile.string );
		} else {
			char	serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf("------------------------------------------------------------\n" );
			G_LogPrintf("InitGame: %s\n", serverinfo );

			trap_RealTime(&timeStamp);
			G_LogPrintf("current init time: %04i-%02i-%02i %02i:%02i:%02i\n",
						1900+timeStamp.tm_year, 1+timeStamp.tm_mon, timeStamp.tm_mday,
						timeStamp.tm_hour, timeStamp.tm_min, timeStamp.tm_sec );
		}
	} else {
		G_Printf( "Not logging to disk.\n" );
	}

	G_InitWorldSession();

	// initialize all entities for this game
	memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
	level.clients = g_clients;

	// set client fields on player ents
	for ( i=0 ; i<level.maxclients ; i++ ) {
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	for ( i=0 ; i<MAX_CLIENTS ; i++ ) {
		g_entities[i].classname = "clientslot";
	}

	// let the server system know where the entites are
	trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();

	// update ruleset
	G_RuleSetUpdate();
	level.rulesetViolated = qfalse;
	level.c_extraDamage = 1.0f;

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	if( g_gametype.integer >= GT_TEAM ) {
		G_CheckTeamItems();
	}

	SaveRegisteredItems();

	G_Printf ("-----------------------------------\n");

	if( g_gametype.integer == GT_SINGLE_PLAYER || trap_Cvar_VariableIntegerValue( "com_buildScript" ) ) {
		G_ModelIndex( SP_PODIUM_MODEL );
	}

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAISetup( restart );
		BotAILoadMap( restart );
		G_InitBots( restart );
	}

	G_RemapTeamShaders();

	trap_SetConfigstring( CS_INTERMISSION, "" );

}



/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) {
	G_Printf ("==== ShutdownGame ====\n");

	if ( level.logFile ) {
		G_LogPrintf("ShutdownGame:\n" );
		G_LogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logFile );
		level.logFile = 0;
	}

	// write all the client session data so we can get it back
	G_WriteSessionData();

	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAIShutdown( restart );
	}
}



//===================================================================

void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer( void ) {
	int			i;
	gclient_t	*client;
	gclient_t	*nextInLine;

	if ( level.numPlayingClients >= 2 ) {
		return;
	}

	// never change during intermission
	if ( level.intermissiontime ) {
		return;
	}

	nextInLine = NULL;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		// never select the dedicated follow or scoreboard clients
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ||
			client->sess.spectatorClient < 0  ) {
			continue;
		}

		if(!nextInLine || client->sess.spectatorNum > nextInLine->sess.spectatorNum)
			nextInLine = client;
	}

	if ( !nextInLine ) {
		return;
	}

	level.warmupTime = -1;

	// set them to free-for-all team
	SetTeam( &g_entities[ nextInLine - level.clients ], "f" );
}

/*
=======================
AddTournamentQueue

Add client to end of tournament queue
=======================
*/

void AddTournamentQueue(gclient_t *client)
{
	int index;
	gclient_t *curclient;

	for(index = 0; index < level.maxclients; index++)
	{
		curclient = &level.clients[index];

		if(curclient->pers.connected != CON_DISCONNECTED)
		{
			if(curclient == client)
				curclient->sess.spectatorNum = 0;
			else if(curclient->sess.sessionTeam == TEAM_SPECTATOR)
				curclient->sess.spectatorNum++;
		}
	}
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[1];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s" );
}

/*
=======================
RemoveTournamentWinner
=======================
*/
void RemoveTournamentWinner( void ) {
	int			clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[0];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s" );
}

/*
=======================
AdjustTournamentScores
=======================
*/
void AdjustTournamentScores( void ) {
	int			clientNum0;
	int			clientNum1;

	clientNum0 = level.sortedClients[0];
	clientNum1 = level.sortedClients[1];

	// check to make sure both clients are still connected
	if ( level.clients[ clientNum0 ].pers.connected != CON_CONNECTED ) {
		if ( level.clients[ clientNum1 ].pers.connected == CON_CONNECTED ) {
			// player wins due to other player forfeiting
			level.clients[ clientNum1 ].sess.wins++;
			ClientUserinfoChanged( clientNum1 );
		}
		return;
	} else
	if ( level.clients[ clientNum1 ].pers.connected != CON_CONNECTED ) {
		if ( level.clients[ clientNum0 ].pers.connected == CON_CONNECTED ) {
			// player wins due to other player forfeiting
			level.clients[ clientNum0 ].sess.wins++;
			ClientUserinfoChanged( clientNum0 );
		}
		return;
	}

	// so now, we assume all clients are still connected

	// did match end up in a stalemate?
	if ( level.clients[ clientNum0 ].ps.persistant[PERS_SCORE] > level.clients[ clientNum1 ].ps.persistant[PERS_SCORE] ) {
		level.clients[ clientNum0 ].sess.wins++;
	} else {
		// indeed it has
		level.clients[ clientNum0 ].sess.losses++;
	}
	ClientUserinfoChanged( clientNum0 );

	// this player lost
	level.clients[ clientNum1 ].sess.losses++;
	ClientUserinfoChanged( clientNum1 );

/*
	int			clientNum;

	clientNum = level.sortedClients[0];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.wins++;
		ClientUserinfoChanged( clientNum );
	}

	clientNum = level.sortedClients[1];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.losses++;
		ClientUserinfoChanged( clientNum );
	}
*/

}

/*
=============
SortRanks

=============
*/
int QDECL SortRanks( const void *a, const void *b ) {
	gclient_t	*ca, *cb;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// sort special clients last
	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0 ) {
		return 1;
	}
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0  ) {
		return -1;
	}

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return 1;
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return -1;
	}


	// then spectators
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR && cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( ca->sess.spectatorNum > cb->sess.spectatorNum ) {
			return -1;
		}
		if ( ca->sess.spectatorNum < cb->sess.spectatorNum ) {
			return 1;
		}
		return 0;
	}
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
		return 1;
	}
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		return -1;
	}

	// then sort by score
	if ( ca->ps.persistant[PERS_SCORE]
		> cb->ps.persistant[PERS_SCORE] ) {
		return -1;
	}
	if ( ca->ps.persistant[PERS_SCORE]
		< cb->ps.persistant[PERS_SCORE] ) {
		return 1;
	}
	return 0;
}

/*
============
UpdateDemerits

Updates demerits on a player, to see if he needs to be suspended from
the game.
============
*/
void UpdateDemerits( gentity_t *ent , int adjustment ) {

	gentity_t *rent = NULL; // mmp - what is this for?

	// don't worry about demerits when the function is disabled
	if (g_demeritLimit.integer < 10) {
		return;
	}

	// don't give demerits during warm-up
	if ( level.warmupTime ) {
		return;
	}

	ent->client->sess.demerits += adjustment;
	if (ent->client->sess.demerits < 0) {
		ent->client->sess.demerits = 0; // keep demerits from going below 0
	} else if (ent->client->sess.demerits >= g_demeritLimit.integer) {
		// player has hit the demerit limit, let's suspend him from the match
		ent->client->sess.suspended = qtrue;

		// toss anything away, such as flags
		TossClientItems( ent ); // TODO: make flag return to base instantly
		/*if ( ent->client->sess.sessionTeam == TEAM_RED ) {
			if ( ent->client->ps.powerups[enemy_flag_pw] ) {

			}
		} else
		if ( ent->client->sess.sessionTeam == TEAM_BLUE ) {
			if ( ent->client->ps.powerups[enemy_flag_pw] ) {

			}
		} else*/

		SetTeam (ent, "s");
		trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Notice: " S_COLOR_WHITE "%s" S_COLOR_YELLOW " hit the demerit limit, and is suspended from the game\n\"",
						NF_GAMEINFO, ent->client->pers.netname) );
	}

}

/*
============
UpdateFollowLeader
============
*/
/*void UpdateFollowLeader( void ) {

	gclient_t	*client;
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
			continue;
		}
		if ( client->sess.specMode == SPECMODE_LASTFRAG ) {
			// don't bother if client number is invalid
			if ( level.sm_lastFrag < 0 && level.sm_lastFrag >= level.maxclients ) {
				return
			}
			trap_SendServerCommand( -1, va("notify %i\\\"DEBUG FOLLOW LASTFRAG = %i\n\"",
								NF_GAMEINFO, level.sm_lastFrag ) );
		}
	}

}*/

/*
============
UpdateSpecMode
============
*/
void UpdateSpecMode( void ) {

	gclient_t	*client;
	int		i;

	if ( level.sm_lastFragTime != 0 ) {
		if ( level.time > level.sm_lastFragTime ) {

			for ( i = 0 ; i < level.maxclients ; i++ ) {
				client = &level.clients[i];
				if ( client->pers.connected != CON_CONNECTED ) {
					continue;
				}
				if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
					continue;
				}
				if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
					continue;
				}
				if ( client->sess.specMode == SPECMODE_LASTFRAG ) {
					// don't bother if client number is invalid
					if ( level.sm_lastFrag < 0 && level.sm_lastFrag >= level.maxclients ) {
						break;
					}
					client->sess.spectatorClient = level.sm_lastFrag;
					/*trap_SendServerCommand( -1, va("notify %i\\\"DEBUG FOLLOW LASTFRAG = %i\n\"",
										NF_GAMEINFO, level.sm_lastFrag ) );*/
				}
			}

			level.sm_lastFragTime = 0;
		}
	}

}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void ) {
	int		i;
	int		rank;
	int		score;
	int		newScore;
	gclient_t	*cl;

	int		highScore = 0;
	int		highScoreClient = -1;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots

	for (i = 0; i < ARRAY_LEN(level.numteamVotingClients); i++)
		level.numteamVotingClients[i] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) {
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) {
				level.numNonSpectatorClients++;

				// decide if this should be auto-followed
				if ( level.clients[i].pers.connected == CON_CONNECTED ) {
					level.numPlayingClients++;
					// bots and muted players cannot vote
					if ( !(g_entities[i].r.svFlags & SVF_BOT || level.clients[i].sess.mute == qtrue) ) { // mmp - uncomment
						level.numVotingClients++;
						if ( level.clients[i].sess.sessionTeam == TEAM_RED )
							level.numteamVotingClients[0]++;
						else if ( level.clients[i].sess.sessionTeam == TEAM_BLUE )
							level.numteamVotingClients[1]++;
					}
					if ( level.follow1 == -1 ) {
						level.follow1 = i;
					} else if ( level.follow2 == -1 ) {
						level.follow2 = i;
					}
				}

			} else {
				if ( g_allowSpecVote.integer ) {
					level.numVotingClients++;
				}
			}

		}
	}

	qsort( level.sortedClients, level.numConnectedClients,
		sizeof(level.sortedClients[0]), SortRanks );

	// set the rank value for all clients that are connected and not spectators
	if ( g_gametype.integer >= GT_TEAM ) {
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( i = 0;  i < level.numConnectedClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 2;
			} else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 0;
			} else {
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	} else {
		rank = -1;
		score = 0;
		for ( i = 0;  i < level.numPlayingClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			newScore = cl->ps.persistant[PERS_SCORE];
			if ( i == 0 || newScore != score ) {
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
			} else {
				// we are tied with the previous client
				level.clients[ level.sortedClients[i-1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if ( g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1 ) {
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( g_gametype.integer >= GT_TEAM ) {
		trap_SetConfigstring( CS_SCORES1, va("%i", level.teamScores[TEAM_RED] ) );
		trap_SetConfigstring( CS_SCORES2, va("%i", level.teamScores[TEAM_BLUE] ) );
	} else {
		if ( level.numConnectedClients == 0 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", SCORE_NOT_PRESENT) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else if ( level.numConnectedClients == 1 ) {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} else {
			trap_SetConfigstring( CS_SCORES1, va("%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va("%i", level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE] ) );
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if ( level.intermissiontime ) {
		SendScoreboardMessageToAllClients();
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
			DeathmatchScoreboardMessage( g_entities + i );
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent ) {
	// take out of follow mode if needed
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		StopFollowing( ent );
	}

	FindIntermissionPoint();
	// move to the spot
	VectorCopy( level.intermission_origin, ent->s.origin );
	VectorCopy( level.intermission_origin, ent->client->ps.origin );
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	memset( ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups) );

	ent->client->ps.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void ) {
	gentity_t	*ent, *target;
	vec3_t		dir;

	// find the intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if ( !ent ) {	// the map creator forgot to put in an intermission point...
		G_Printf( "*the map creator forgot to put in an intermission point...  what an asshole!\n" );
		SelectSpawnPoint ( vec3_origin, level.intermission_origin, level.intermission_angle, qfalse );
	} else {
		VectorCopy (ent->s.origin, level.intermission_origin);
		VectorCopy (ent->s.angles, level.intermission_angle);
		// if it has a target, look towards it
		if ( ent->target ) {
			target = G_PickTarget( ent->target );
			if ( target ) {
				VectorSubtract( target->s.origin, level.intermission_origin, dir );
				vectoangles( dir, level.intermission_angle );
			}
		}
	}

}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void ) {
	int			i;
	gentity_t	*client;

	if ( level.intermissiontime ) {
		return;		// already active
	}

	//G_Printf( "^1DEBUG: BeginIntermission" );

	trap_SendServerCommand( -1, "bgm_endMatch" );

	// if in tournament mode, change the wins / losses
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		AdjustTournamentScores();
	}

	level.intermissiontime = level.time;
	// move all clients to the intermission point
	for (i=0 ; i< level.maxclients ; i++) {
		client = g_entities + i;
		if (!client->inuse)
			continue;
		// respawn if dead
		if (client->health <= 0) {
			ClientRespawn(client);
		}
		MoveClientToIntermission( client );
	}
#ifdef MISSIONPACK
	if (g_singlePlayer.integer) {
		trap_Cvar_Set("ui_singlePlayerActive", "0");
		UpdateTournamentInfo();
	}
#else
	// if single player game
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		UpdateTournamentInfo();
		SpawnModelsOnVictoryPads();
	}
#endif
	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

}


/*
=============
MapRotation

Grabs a random map from the file specified in "g_mapRotation",
and copy it with a command to the cvar "nextmap".

Returns true if file read was fine, or if "g_mapRotation" is not set.

TODO: check if this code causes a memory leak

=============
*/
qboolean MapRotation( const char *qpath ) {

	//Here the game finds the nextmap if g_autonextmap is set
	if ( strlen(qpath) ) {
		// mmp - the auto map code was grabbed from openarena
		// however, will modify to run a better rotation system
		// such as preventing the last X amount of maps from rotation from being picked again

		fileHandle_t	file, mapfile;
		char			serverinfo[MAX_INFO_STRING];
		char			nextmap[MAX_STRING_CHARS];

		trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

		//Read the file:
		trap_FS_FOpenFile(qpath, &file, FS_READ);
		if (file) {
			char  buffer[4*1024]; // buffer to read file into
			char mapnames[1024][20]; // Array of mapnames in the map pool
			char *pointer;
			int choice, count=0; //The random choice from mapnames and count of mapnames
			int i;
			memset(&buffer,0,sizeof(buffer));
			trap_FS_Read(&buffer,sizeof(buffer),file);
			pointer = buffer;
			while ( qtrue ) {
				Q_strncpyz(mapnames[count],COM_Parse( &pointer ),20);
				if ( !mapnames[count][0] ) {
					break;
				}
				/*G_Printf("Mapname in mappool: %s\n",mapnames[count]);*/
				count++;
			}
			trap_FS_FCloseFile(file);
			//It is possible that the maps in the file read are flawed, so we try up to ten times:
			for(i=0;i<10;i++) {
				choice = (count > 0)? rand()%count : 0;
				if( Q_stricmp (mapnames[choice], Info_ValueForKey(serverinfo,"mapname") ) == 0 ) {
					continue;
				}
				//Now check that the map exists:
				trap_FS_FOpenFile(va("maps/%s.bsp",mapnames[choice]),&mapfile,FS_READ);
				if(mapfile) {
					/*G_Printf("Picked map number %i - %s\n",choice,mapnames[choice]);*/
					Q_strncpyz(nextmap,va("map %s",mapnames[choice]),sizeof(nextmap));
					trap_Cvar_Set("nextmap",nextmap);
					trap_FS_FCloseFile(mapfile);
					break;
				}
			}
		} else {
			G_Printf( "Could not read file '%s' for map rotation\n", qpath );
			//trap_SendServerCommand( ent-g_entities, "print \"Map rotation not set.\n\"" );
			return qfalse;
		}
	} /*else {
		G_Printf( "Auto map rotation not enabled\n" );
	}*/

	return qtrue;

}


/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar

=============
*/
void ExitLevel (void) {
	int			i;
	gclient_t	*cl;
	char		nextmap[MAX_STRING_CHARS];
	char		d1[MAX_STRING_CHARS];
	char		scheduleExec[MAX_STRING_CHARS];

	qtime_t		timeStamp;
	int			dayOfWeek;
	qboolean	serviceRun;
	qboolean	result = qfalse;

	//bot interbreeding
	BotInterbreedEndMatch();

	// daily server schedule maintenance (not fully tested, use with caution)
	trap_RealTime(&timeStamp);
	dayOfWeek = timeStamp.tm_wday; // 0 = sunday, 6 = saturday

	serviceRun = qfalse;
	if ( g_currentDay.integer < 0 ) {
		// two full matches need to run before the schedule can work
		trap_Cvar_Set( "currentDay", va("%i", dayOfWeek) );
	} else if ( g_currentDay.integer != dayOfWeek ) {
		// TODO: code this better
		switch ( dayOfWeek ) {
			default: // just a failsafe
				trap_Cvar_VariableStringBuffer( "g_serviceScheduleSun", scheduleExec, sizeof(scheduleExec) );
				break;
			case 1:
				trap_Cvar_VariableStringBuffer( "g_serviceScheduleMon", scheduleExec, sizeof(scheduleExec) );
				break;
			case 2:
				trap_Cvar_VariableStringBuffer( "g_serviceScheduleTues", scheduleExec, sizeof(scheduleExec) );
				break;
			case 3:
				trap_Cvar_VariableStringBuffer( "g_serviceScheduleWed", scheduleExec, sizeof(scheduleExec) );
				break;
			case 4:
				trap_Cvar_VariableStringBuffer( "g_serviceScheduleThurs", scheduleExec, sizeof(scheduleExec) );
				break;
			case 5:
				trap_Cvar_VariableStringBuffer( "g_serviceScheduleFri", scheduleExec, sizeof(scheduleExec) );
				break;
			case 6:
				trap_Cvar_VariableStringBuffer( "g_serviceScheduleSat", scheduleExec, sizeof(scheduleExec) );
				break;
		}

		if ( Q_stricmp( scheduleExec, "" ) ) {
			// TODO: code this better
			switch ( dayOfWeek ) {
				default: // just a failsafe
					trap_SendConsoleCommand( EXEC_APPEND, "vstr g_serviceScheduleSun\n" );
					break;
				case 1:
					trap_SendConsoleCommand( EXEC_APPEND, "vstr g_serviceScheduleMon\n" );
					break;
				case 2:
					trap_SendConsoleCommand( EXEC_APPEND, "vstr g_serviceScheduleTues\n" );
					break;
				case 3:
					trap_SendConsoleCommand( EXEC_APPEND, "vstr g_serviceScheduleWed\n" );
					break;
				case 4:
					trap_SendConsoleCommand( EXEC_APPEND, "vstr g_serviceScheduleThurs\n" );
					break;
				case 5:
					trap_SendConsoleCommand( EXEC_APPEND, "vstr g_serviceScheduleFri\n" );
					break;
				case 6:
					trap_SendConsoleCommand( EXEC_APPEND, "vstr g_serviceScheduleSat\n" );
					break;
			}
			serviceRun = qtrue;
		} else {
			serviceRun = qfalse;
		}

		// update current day
		trap_Cvar_Set( "currentDay", va("%i", dayOfWeek) );
	}

	if ( serviceRun == qfalse ) {

		// the tournament gametype just restarts the map
		if ( g_gametype.integer == GT_TOURNAMENT && level.forcedTimeLimit == qfalse ) {
			if ( !level.restarted ) {
				//RemoveTournamentLoser();
				trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
				level.restarted = qtrue;
				level.changemap = NULL;
				level.intermissiontime = 0;
			}
			return;
		}

		// pick map from rotation based on game type
		if ( g_gametype.integer == GT_FFA )
			result = MapRotation( g_mapRotation_ffa.string );
		else if ( g_gametype.integer == GT_TOURNAMENT )
			result = MapRotation( g_mapRotation_duel.string );
		else if ( g_gametype.integer == GT_TEAM )
			result = MapRotation( g_mapRotation_tdm.string );
		else if ( g_gametype.integer == GT_CTF )
			result = MapRotation( g_mapRotation_ctf.string );

		// pick map from regular rotation
		if ( result == qfalse )
			MapRotation( g_mapRotation.string );

		trap_Cvar_VariableStringBuffer( "nextmap", nextmap, sizeof(nextmap) );
		trap_Cvar_VariableStringBuffer( "d1", d1, sizeof(d1) );

		if( !Q_stricmp( nextmap, "map_restart 0" ) && Q_stricmp( d1, "" ) ) {
			trap_Cvar_Set( "nextmap", "vstr d2" );
			trap_SendConsoleCommand( EXEC_APPEND, "vstr d1\n" );
		} else {
			trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
		}

	}

	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
	}

	// we need to do this here before changing to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for (i=0 ; i< g_maxclients.integer ; i++) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;

	sec = ( level.time - level.startTime ) / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	Q_vsnprintf(string + 7, sizeof(string) - 7, fmt, argptr);
	va_end( argptr );

	if ( g_dedicated.integer ) {
		G_Printf( "%s", string + 7 );
	}

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
=================
G_VerboseLogPrintf

Print extra unneeded information to the logfile with a time stamp if it is open
=================
*/
void QDECL G_VerboseLogPrintf( int vlevel, const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;

	// don't display of log any unneeded information
	if ( g_verboseLog.integer < vlevel ) {
		return;
	}

	sec = ( level.time - level.startTime ) / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	Q_vsnprintf(string + 7, sizeof(string) - 7, fmt, argptr);
	va_end( argptr );

	if ( g_dedicated.integer ) {
		G_Printf( "%s", string + 7 );
	}

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
}


/*
=================
G_RecordStats

ent = entity
recordType = type
=================
*/

void G_RecordStats ( gentity_t *ent, int recordType ) {

	// if we have maxed out the slots, just fuck it
	if ( level.statRecordSlot >= MAX_CLIENT_STATS_SLOT ) {
		return;
	}

	stats[level.statRecordSlot].kills = ent->client->pers.kills;

	G_Printf( S_COLOR_PINK "DEBUG: slot=%i, %i -> %i\n", level.statRecordSlot, ent->client->pers.kills, stats[level.statRecordSlot].kills );

	/*if ( recordType == RS_ENDSCORE ) {

	}*/


	level.statRecordSlot++;

}


/*
=================
G_BroadcastEndGameStats

=================
*/

void G_BroadcastEndGameStats( void ) {

	gclient_t	*cl;

	int			ping;
	int			playTime;

	if ( level.time < level.statsBufferSendLastTime  ) {
		return;
	}
	level.statsBufferSendLastTime = level.time + 250; // broadcast stats buffer four times a seconds

	if ( level.statsRunStop == qtrue ) {
		return;
	}

	if ( level.statsCurClientSlot >= level.numConnectedClients || level.statsCurClientSlot >= MAX_GAME_STATS ) {
		level.statsRunStop = qtrue;
		/*G_Printf( S_COLOR_PINK "DEBUG: G_BroadcastEndGameStats END [%i]\n", level.time );*/
		trap_SendServerCommand( -1, "endStats" );
		return;
	}

	//G_Printf( S_COLOR_PINK "DEBUG: G_BroadcastEndGameStats [%i]\n", level.time );
	cl = &level.clients[level.sortedClients[level.statsCurClientSlot]];

	// skip players not connected
	if ( cl->pers.connected == CON_CONNECTING ) {
		return;
	}

	// skip spectators
	if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
	playTime = ((level.time - cl->pers.enterTime) +1000) / 60000;
	if ( playTime > 99 ) {
		playTime = 99;
	}

	/*G_Printf( S_COLOR_PINK "DEBUG: [%i] [%i] %i %i %i %i %i %i %i %i %i %i\n", level.time, level.sortedClients[level.statsCurClientSlot],
			cl->sess.sessionTeam, cl->ps.persistant[PERS_SCORE], ping, playTime, cl->pers.kills, cl->pers.deaths, cl->pers.suicides, cl->pers.teamKills, cl->pers.captures, cl->pers.killStreak );*/
	/*trap_SendServerCommand( -1, va("sendStats %i\\%i\\%i\\%i\\%i\\%i\\%i\\%i\\%i\\%i\\%i\n", level.sortedClients[level.statsCurClientSlot],
			cl->sess.sessionTeam, cl->ps.persistant[PERS_SCORE], ping, playTime, cl->pers.kills, cl->pers.deaths, cl->pers.suicides, cl->pers.teamKills, cl->pers.captures, cl->pers.killStreak) );*/
	trap_SendServerCommand( -1, va("sendStats %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[level.statsCurClientSlot],
			cl->sess.sessionTeam, cl->ps.persistant[PERS_SCORE], ping, playTime, cl->pers.kills, cl->pers.deaths, cl->pers.suicides, cl->pers.teamKills, cl->pers.captures, cl->pers.killStreak) );

	level.statsCurClientSlot++;
}



/*
=================
G_EndGameStats

This is no longer used
=================
*/

void G_PrintBooleanStats( char *string, int result ) {
	if ( result > 0 ) {
		trap_SendServerCommand( -1, va("print \"  %s: Yes\n\"", string ));
	} else {
		trap_SendServerCommand( -1, va("print \"  %s: No\n\"", string ));
	}
}

void G_PrintValueStats( char *string, int result, const char *afterString ) {
	if ( result > 1 ) {
		trap_SendServerCommand( -1, va("print \"  %s: %i%ss\n\"", string, result, afterString ));
	} else if ( result == 1 ) {
		trap_SendServerCommand( -1, va("print \"  %s: 1%s\n\"", string, afterString ));
	} else {
		trap_SendServerCommand( -1, va("print \"  %s: No\n\"", string ));
	}
}

void G_EndGameStats( void ) {

	char		string[1024] = {""};
	int			i;
	gclient_t	*cl;
	int			ping;
	int			playTime;
	//int		numSorted;
	int			value;
	float		kdr; // total kills, divide by total deaths
	qboolean	specialRule = qfalse;

	// still in warmup?
	if ( level.warmupTime ) {
		return;
	}

	// was there a rule set violation?
	if ( level.rulesetViolated == qtrue ) {
		level.statsRunLoop = qfalse;
		return;
	}

	if ( level.statsRunLoop == qfalse ) {
		return;
	}

	if ( level.statsLoopCounter == -1 ) {
		trap_SendServerCommand( -1, "print \"\n======================================\nPLAYER STATS:\n======================================\n\"" );
		if (g_gametype.integer >= GT_TEAM) {
			trap_SendServerCommand( -1, va("print \"RED: %4i        vs.        BLUE: %4i\n\"",
							level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
			trap_SendServerCommand( -1, "print \"--------------------------------------\n\"" );
		}

		level.numSorted = level.numConnectedClients;
		if ( level.numSorted > 32 ) {
			level.numSorted = 32;
		}
	} else

	// was: for (i=0 ; i < numSorted ; i++)
	if ( level.statsLoopCounter >= 0 ) {

		cl = &level.clients[level.sortedClients[level.statsLoopCounter]];

		if ( cl->sess.sessionTeam != TEAM_SPECTATOR ) {
			if ( cl->pers.connected != CON_CONNECTING ) {

				ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
				playTime = ((level.time - cl->pers.enterTime) +1000) / 60000;
				if ( playTime > 99 ) {
					playTime = 99;
				}

				if (cl->pers.deaths > 0) {
					kdr = ((float)cl->pers.kills) / ((float)cl->pers.deaths);
				} else {
					kdr = (float)cl->pers.kills;
				}

				trap_SendServerCommand( -1, va( "print \"NAME: %s\n\"", cl->pers.netname ) );
				if (g_gametype.integer >= GT_TEAM) {
					if (cl->sess.sessionTeam == TEAM_RED) {
						trap_SendServerCommand( -1, "print \"TEAM: RED\n\"" );
					} else {
						trap_SendServerCommand( -1, "print \"TEAM: BLUE\n\"" );
					}
				}
				trap_SendServerCommand( -1, va( "print \"SCORE: %4i  PING: %3i  TIME: %2im\n\"",
						cl->ps.persistant[PERS_SCORE], ping, playTime ) );
				trap_SendServerCommand( -1, va( "print \"KILLS: %4i  DEATHS: %4i  KDR: %6.3f\n\"",
						cl->pers.kills, cl->pers.deaths, kdr ) );
				if (g_gametype.integer >= GT_TEAM) {
					if ( level.rs_friendlyFire ) {
						trap_SendServerCommand( -1, va( "print \"SUICIDES: %4i  TEAMKILLS: %4i\n\"",
								cl->pers.suicides, cl->pers.teamKills ) );
					} else {
						trap_SendServerCommand( -1, va( "print \"SUICIDES: %4i\n\"",
								cl->pers.suicides ) );
					}
					if (g_gametype.integer >= GT_CTF) {
						trap_SendServerCommand( -1, va( "print \"CAPTURES: %4i  MAX KILL STREAK: %4i\n\"",
								cl->pers.captures, cl->pers.killStreak ) );
					} else {
						trap_SendServerCommand( -1, va( "print \"MAX KILL STREAK: %4i\n\"",
								cl->pers.killStreak ) );
					}
				} else {
					trap_SendServerCommand( -1, va( "print \"SUICIDES: %4i  MAX KILL STREAK: %4i\n\"",
							cl->pers.suicides, cl->pers.killStreak ) );
				}

				trap_SendServerCommand( -1, "print \"--------------------------------------\n\"" );
			}
		}

	}

	level.statsLoopCounter++;
	if ( level.statsLoopCounter >= level.numSorted ) {

		/*trap_SendServerCommand( -1, "print \"RULE SET:\n\n\"" );*/

		// TODO: add map's name
		/*G_PrintBooleanStats("GAME RULES ENFORCED", g_ruleset.integer);*/
		switch ( g_ruleset.integer ) {
			case 1:
				trap_SendServerCommand( -1, "print \"RULE SET: Standard\n\n\"" );
				break;
			case 2:
				trap_SendServerCommand( -1, "print \"RULE SET: Hardcore\n\n\"" );
				break;
			case 3:
				trap_SendServerCommand( -1, "print \"RULE SET: Nightmare!\n\n\"" );
				break;
			case 4:
				trap_SendServerCommand( -1, "print \"RULE SET: Arena\n\n\"" );
				break;
			default:
				trap_SendServerCommand( -1, "print \"RULE SET: Custom\n\n\"" );
		}
		G_PrintValueStats("TIME LIMIT", level.rs_timelimit, " minute");
		if ( level.rs_timelimit > 0 ) {
			if ( level.rs_overtime > 1 ) {
				trap_SendServerCommand( -1, va( "print \"  OVERTIME: Extension of %i minutes\n\"", level.rs_overtime ) );
			} else if ( level.rs_overtime == 1 ) {
				trap_SendServerCommand( -1, va( "print \"  OVERTIME: Extension of %i minute\n\"", level.rs_overtime ) );
			} else {
				trap_SendServerCommand( -1, va( "print \"  OVERTIME: Sudden death\n\"", level.rs_timelimit ) );
			}
		}
		G_PrintValueStats("SCORE LIMIT", level.rs_scorelimit, " point");
		if ( g_proMode.integer == 1 ) {
			trap_SendServerCommand( -1, "print \"  PRO-MODE: Free-select physics\n\n\"" );
		} else {
			trap_SendServerCommand( -1, "print \"  PRO-MODE: Restricted physics\n\n\"" );
		}


		if ( g_teamSize.integer > 0 ) {
			G_PrintValueStats("SET TEAM SIZE", g_teamSize.integer, " player");
		}
		G_PrintValueStats("WEAPON RESPAWN", level.rs_weaponRespawn, " second");
		G_PrintValueStats("KEYCARD RESPAWN", level.rs_keycardRespawn, " second");
		G_PrintBooleanStats("KEYCARD DROPABLE", level.rs_keycardDropable);

		G_PrintValueStats("FORCED PLAYER RESPAWN", level.rs_forceRespawn, " second");
		G_PrintBooleanStats("SELF DAMAGE", level.rs_selfDamage);
		if (g_gametype.integer >= GT_TEAM) {
			G_PrintBooleanStats("FRIENDLY FIRE", level.rs_friendlyFire);
			G_PrintBooleanStats("TEAM'S LOC. ON MINIMAP", level.rs_teamLocOverlay);
			if (g_gametype.integer == GT_TEAM) {
				G_PrintBooleanStats("TEAM SCORE BALANCE", level.rs_scoreBalance);
			}

		}
		G_PrintBooleanStats("HIT SOUND", level.rs_hitSound);
		G_PrintBooleanStats("TOTALLY RANDOM SPAWNS", level.rs_randomSpawn);

		if ( g_ruleset.integer < 1 || g_ruleset.integer > 2 ) {
		// note that '> 2' is used instead of 3, because Nightmare and Arena mode uses quad mode and/or double ammo

			trap_SendServerCommand( -1, "print \"\nSPECIAL RULES:\n\n\"" );
			if ( level.rs_quadMode ) {
				trap_SendServerCommand( -1, "print \"  QUAD MODE\n\"" );
				specialRule = qtrue;
			}
			if ( level.rs_doubleAmmo ) {
				trap_SendServerCommand( -1, "print \"  DOUBLE AMMO\n\"" );
				specialRule = qtrue;
			}

			if ( specialRule == qfalse ) {
				trap_SendServerCommand( -1, "print \"  No special rules enabled\n\"" );
			}

		}

		trap_SendServerCommand( -1, "print \"======================================\n\"" );

		level.statsRunLoop = qfalse;

	}

}


/*
=================
G_BookkeepingLog
=================
*/

void G_BookkeepingLog( void ) {

	char		string[1024] = {""};
	fileHandle_t	f;
	int			len;
	int			timeCode;

	qtime_t		timeStamp;

	if ( !strlen(g_bookkeepingLog.string) ) {
		G_Printf( "Bookkeeping not logged\n" );
		return;
	}

	// Timestamp, return 0 if successful and !0 for an error.
	// ??? - mmp
	/*if ( trap_RealTime(&timeStamp) == 0 ) {*/

	trap_RealTime(&timeStamp);

	trap_FS_FOpenFile( g_bookkeepingLog.string, &f, FS_APPEND );
	if( !f ) {
		G_Printf( "WARNING: Couldn't open bookkeeping logfile: \"%s\"\n", g_bookkeepingLog.string );
		return;
	}

	/*
	reference
	---------

	qtime->tm_sec = tms->tm_sec;
	qtime->tm_min = tms->tm_min;
	qtime->tm_hour = tms->tm_hour;
	qtime->tm_mday = tms->tm_mday;
	qtime->tm_mon = tms->tm_mon;
	qtime->tm_year = tms->tm_year;
	qtime->tm_wday = tms->tm_wday;
	qtime->tm_yday = tms->tm_yday;
	qtime->tm_isdst = tms->tm_isdst;
	*/

	// timecode format for stat trackers
	timeCode = ((timeStamp.tm_yday * 1440) + timeStamp.tm_hour * 60) + timeStamp.tm_min ;

	Com_sprintf( string,
					sizeof(string), "\\timeStamp\\%04i-%02i-%02i %02i:%02i:%02i\\timeCode\\%04i.%06i\\maxClients\\%i\\maxPlayers\\%i\\\n",
					1900+timeStamp.tm_year, 1+timeStamp.tm_mon, timeStamp.tm_mday,
					timeStamp.tm_hour, timeStamp.tm_min, timeStamp.tm_sec,
					timeStamp.tm_year, timeCode,
					level.maxClientsAchieved, level.maxPlayersAchieved );
	len = strlen( string );
	trap_FS_Write (string, len, f);

	trap_FS_FCloseFile( f );

	/*} else {
		G_Printf( "WARNING: Couldn't retrieve timestamp.\n" );
	}*/

}


/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string ) {
	int				i, numSorted;
	gclient_t		*cl;
#ifdef MISSIONPACK
	qboolean won = qtrue;
#endif
	G_LogPrintf( "Exit: %s\n", string );

	level.intermissionQueued = level.time;
	level.matchEnded = qtrue;

	//G_Printf( "^1DEBUG: LogExit: %s", string );

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if ( numSorted > 32 ) {
		numSorted = 32;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		G_LogPrintf( "red:%i  blue:%i\n",
			level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( cl->pers.connected == CON_CONNECTING ) {
			continue;
		}

		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i],	cl->pers.netname );
#ifdef MISSIONPACK
		if (g_singlePlayer.integer && g_gametype.integer == GT_TOURNAMENT) {
			if (g_entities[cl - level.clients].r.svFlags & SVF_BOT && cl->ps.persistant[PERS_RANK] == 0) {
				won = qfalse;
			}
		}
#endif

	}

#ifdef MISSIONPACK
	if (g_singlePlayer.integer) {
		if (g_gametype.integer >= GT_CTF) {
			won = level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE];
		}
		trap_SendConsoleCommand( EXEC_APPEND, (won) ? "spWin\n" : "spLose\n" );
	}
#endif

	// init display stats in console
	/*level.statsLoopCounter = -3; // delay sending stats to the console
	level.statsRunLoop = qtrue;*/

	level.statsRunStop = qfalse; // allow stats to be broadcasted to clients

	trap_SendServerCommand( -1, va("totalPlayTime %i", level.time - level.startTime) ); // send total play time to clients

}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 15 seconds before going on.
=================
*/
void CheckIntermissionExit( void ) {
	int			ready, notReady, playerCount;
	int			i;
	gclient_t	*cl;
	int			readyMask;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		return;
	}

	// broadcast end-game stats to clients
	G_BroadcastEndGameStats();

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	playerCount = 0;
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		// don't count empty slots
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		// don't count spectators
		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		// don't count bots, bots are always ready
		if ( g_entities[i].r.svFlags & SVF_BOT ) {
			continue;
		}

		playerCount++;
		if ( cl->readyToExit ) {
			ready++;
			// mmp - TODO: display more than 16 ready players
			if ( i < 16 ) {
				readyMask |= 1 << i;
			}
		} else {
			notReady++;
		}
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for (i=0 ; i< g_maxclients.integer ; i++) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		// skip spectators
		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds
	if ( level.time < level.intermissiontime + 5000 ) {
		return;
	}

	// only test ready status when there are real players present
	if ( playerCount > 0 ) {
		// if nobody wants to go, clear timer
		if ( !ready ) {
			level.readyToExit = qfalse;
			return;
		}

		// if everyone wants to go, go now
		if ( !notReady ) {

			if ( !level.intermissionEndAlert ) {
				level.intermissionEndAlert = 96;
				trap_SendServerCommand( -1, va("notify %i\\\""S_COLOR_YELLOW"Next match will begin in 5 seconds.  Get ready!\n\"",
								NF_GAMEINFO) );
				level.readyToExit = qtrue;
				level.exitTime = level.time;
			} else if ( level.time > level.exitTime + 5000 ) {
					ExitLevel();
			}
			return;
		}
	}

	// the first person to ready starts the ten second timeout
	if ( !level.readyToExit ) {
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited fifthteen seconds since at least one player
	// wanted to exit, go ahead
	if ( level.time < level.exitTime + 15000 ) {

		if ( level.time > level.exitTime + 10000 && !level.intermissionEndAlert ) {
			level.intermissionEndAlert = 69;
			trap_SendServerCommand( -1, va("notify %i\\\""S_COLOR_YELLOW"Next match will begin in 5 seconds.  Get ready!\n\"",
							NF_GAMEINFO) );
		}

		return;
	}

	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void ) {
	int		a, b;

	if ( level.numPlayingClients < 2 ) {
		return qfalse;
	}

	if ( g_gametype.integer == GT_AA1 ) {
		if ( level.teamScores[TEAM_BLUE] < 0 ) {
			return qtrue;
		}
		if ( level.teamScores[TEAM_RED] > 0 ) {
			return qtrue;
		}
		return qfalse;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return a == b;
}

/*
=============
ScoreIsTooFarApart
=============
*/
qboolean ScoreIsTooFarApart( void ) {
	int		a, b, c;

	if ( level.numPlayingClients < 2 ) {
		return qtrue;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		a = level.teamScores[TEAM_RED];
		b = level.teamScores[TEAM_BLUE];
	} else {
		a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
		b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];
	}

	// advance the round if player is only 4 points behind, or 19 in team matches
	c = a - b;

	if ( g_gametype.integer >= GT_TEAM ) {
		if ( g_gametype.integer == GT_CTF ) {
			if ( level.rs_popCTF ) {
				if ( c < 2 && c > -2 )
					return qfalse;
			} else {
				if ( c < 200 && c > -200 )
					return qfalse;
			}
		} else {
			if ( c < 20 && c > -20 )
				return qfalse;
		}
	} else {
		if ( c < 5 && c > -5 )
			return qfalse;
	}

	// if trailing player/team does not have at least half of the leader's score, end the round
	if ( a > ( b * 2 ) )
		return qtrue;
	if ( b > ( a * 2 ) )
		return qtrue;

	return qfalse;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.

LogExit() is what triggers the match to end

TODO: remove all commented-out code when we do some spring cleaning
=================
*/
void CheckExitRules( void ) {
	int			i;
	gclient_t	*cl;

	float		timelimit;
	int			overtime;
	int			scorelimit;
	int			mercylimit;

	int			a, b;

	qboolean	giveUp;

	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime ) {
		CheckIntermissionExit ();
		return;
	}

	if ( level.intermissionQueued ) {
#ifdef MISSIONPACK
		int time = (g_singlePlayer.integer) ? SP_INTERMISSION_DELAY_TIME : INTERMISSION_DELAY_TIME;
		if ( level.time - level.intermissionQueued >= time ) {
			level.intermissionQueued = 0;
			//G_Printf( "^1DEBUG: SP call BeginIntermission()" );
			BeginIntermission();
		}
#else
		if ( level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME ) {
			level.intermissionQueued = 0;
			//G_Printf( "^1DEBUG: call BeginIntermission()" );
			BeginIntermission();

			// record stats for book keeping
			if ( g_dedicated.integer ) {
				G_BookkeepingLog();
			}
		}
#endif
		return;
	}

	if ( level.matchEnded == qtrue ) {
		return;
	}

	// use custom defined settings, or ruleset defined settings
	timelimit = level.rs_timelimit;
	if ( g_shortGame.integer )
		timelimit = timelimit / 2;
	overtime = level.rs_overtime;
	scorelimit = level.rs_scorelimit;
	mercylimit = level.rs_mercylimit;

	// prevent matches from dragging on
	if ( level.time - level.startTime >= FORCED_TIMELIMIT*60000 ) {
		trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_RED "Forced timelimit reached.  Match went on for too long!\n\"", NF_GAMEINFO) );
		trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
		LogExit( "Forced timelimit." );
		level.forcedTimeLimit = qtrue;
		return;
	}

	// still in warmup?
	if ( level.warmupTime ) {
		return;
	}

	if ( level.rulesetViolated == qtrue ) {
		trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Match disqualified, due to ruleset modification.\n\"", NF_GAMEINFO) );
		trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
		LogExit( "Match disqualified." );
	}

	// round based matches, 1r = must get at least half of leader to advance, 2r = get more points than other(s), 3r = overtime
	if ( timelimit ) {

		if ( level.rs_roundFormat ) {

			if ( level.currentRound == 0 ) {

				if ( level.time - level.startTime >= timelimit*30000 ) {

					if ( ScoreIsTooFarApart() ) {
						trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
						/*trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Timelimit hit, runner-up was not close enough to the leader.\n\"", NF_GAMEINFO) );*/
						trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Timelimit hit, too much of a gap in scores to advance.\n\"", NF_GAMEINFO) );
						LogExit( "Timelimit hit." );
						return;
					} else {
						trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Advancing to the second round.\n\"", NF_GAMEINFO) );
						level.currentRound = 1; // second round
						trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_OVERTIME ) );
						trap_SetConfigstring( CS_ROUND, va("%i", level.currentRound ) );
						G_LogPrintf( "Round advance.\n" );
						return;
					}

				}

			}

		}
	}

	if ( overtime > 0 ) {

		if ( timelimit ) {

			if ( (level.time - level.startTime) >= ((timelimit + level.overtime*overtime)*60000) ) {

				// don't let boring matches drag on
				if ( level.killTotal == 0 ) {
					trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
					trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Match over, due to stalemate.\n\"", NF_GAMEINFO) );
					LogExit( "Match stalemate." );
					return;
				}

				if ( ScoreIsTied() ) {
					trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Overtime, %i minutes added to timelimit.\n\"",
									NF_GAMEINFO, overtime) );
					level.overtime++;
					trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_OVERTIME ) );
					trap_SetConfigstring( CS_OVERTIME, va("%i", level.overtime ) );
					G_LogPrintf( "Overtime. (%i)\n", level.overtime );
					return;
				} else {
					trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
					if ( level.overtime ) {
						trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Overtime over.\n\"", NF_GAMEINFO) );
						LogExit( "Overtime over." );
					} else {
						trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Timelimit hit.\n\"", NF_GAMEINFO) );
						LogExit( "Timelimit hit." );
					}
					return;
				}
			}

		}

	} else {

		if ( level.pastTimelimit == qtrue ) {

			// check for sudden death / overtime
			if ( ScoreIsTied() ) {
				return;
			}

			if ( timelimit ) {
				if ( level.time - level.startTime >= timelimit*60000 ) {
					trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Sudden death over.\n\"", NF_GAMEINFO) );
					trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
					LogExit( "Sudden death over." );
					return;
				}
			}

		} else {

			if ( timelimit ) {
				if ( level.time - level.startTime >= timelimit*60000 ) {

					// don't let boring matches drag on
					if ( level.killTotal == 0 ) {
						trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
						trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Match over, due to stalemate.\n\"", NF_GAMEINFO) );
						LogExit( "Match stalemate." );
						return;
					}

					// check for sudden death / overtime
					if ( ScoreIsTied() ) {
						trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Sudden death.\n\"", NF_GAMEINFO) );
						trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_OVERTIME ) );
						G_LogPrintf( "Sudden death.\n" );
						level.pastTimelimit = qtrue;
					} else {
						trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Timelimit hit.\n\"", NF_GAMEINFO) );
						trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
						LogExit( "Timelimit hit." );
					}
					return;
				}
			}

		}

	}

	// tdm scorelimit
	if ( g_gametype.integer == GT_TEAM ) {

		if ( scorelimit ) {
			if ( level.teamScores[TEAM_RED] >= scorelimit ) {
				//trap_SendServerCommand( -1, "print \"Red team hit the scorelimit.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Red team hit the scorelimit.\n\"", NF_GAMEINFO) );
				trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
				LogExit( "Scorelimit hit." );
				return;
			}

			if ( level.teamScores[TEAM_BLUE] >= scorelimit ) {
				//trap_SendServerCommand( -1, "print \"Blue team hit the scorelimit.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Blue team hit the scorelimit.\n\"", NF_GAMEINFO) );
				trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
				LogExit( "Scorelimit hit." );
				return;
			}
		}

		if ( mercylimit ) {
			if ( (level.teamScores[TEAM_RED]-level.teamScores[TEAM_BLUE]) >= mercylimit || (level.teamScores[TEAM_BLUE]-level.teamScores[TEAM_RED]) >= mercylimit ) {
				//trap_SendServerCommand( -1, "print \"Blue team hit the scorelimit.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Mercylimit hit.\n\"", NF_GAMEINFO) );
				trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
				LogExit( "Mercylimit hit." );
				return;
			}
		}

	} else

	// aa1 scorelimit
	if ( g_gametype.integer == GT_AA1 ) {
		if ( level.teamScores[TEAM_BLUE] >= scorelimit && scorelimit ) {
			trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "The target hit the scorelimit.\n\"", NF_GAMEINFO) );
			trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
			LogExit( "Scorelimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_RED] > 0 ) {
			trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Target has been eliminated.\n\"", NF_GAMEINFO) );
			trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
			LogExit( "Target eliminated." );
			return;
		}

	} else
	// dm scorelimit
	if ( g_gametype.integer < GT_TEAM ) {

		//giveUp = qfalse; // clear it, just incase

		if (scorelimit) {
			for ( i=0 ; i< g_maxclients.integer ; i++ ) {
				cl = level.clients + i;
				if ( cl->pers.connected != CON_CONNECTED ) {
					continue;
				}
				if ( cl->sess.sessionTeam != TEAM_FREE ) {
					continue;
				}

				if ( cl->ps.persistant[PERS_SCORE] >= scorelimit ) {
					LogExit( "Scorelimit hit." );
					/*trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " hit the scorelimit.\n\"",
						cl->pers.netname ) );*/
					trap_SendServerCommand( -1, va("notify %i\\\"%s" S_COLOR_YELLOW " hit the scorelimit.\n\"",
										NF_GAMEINFO, cl->pers.netname ) );
					trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
					return;
				}
			}
		}

		if ( mercylimit ) {
			a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
			b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];
			if ( (a-b) >= mercylimit || (b-a) >= mercylimit ) {
				//trap_SendServerCommand( -1, "print \"Blue team hit the scorelimit.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Mercylimit hit.\n\"", NF_GAMEINFO) );
				trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
				LogExit( "Mercylimit hit." );
				return;
			}
		}

		// also check if player forfeits
		if ( g_gametype.integer == GT_TOURNAMENT ) {

			/*if ( level.numPlayingClients < 2 ) {
				// TODO: add cvar to enable or disable this function.
				trap_SendServerCommand( -1, va("notify %i\\\"Player left the match.  Match has been forfeited.\n\"", NF_GAMEINFO) );
				trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
				LogExit( "Player left the match." );
			}*/

			// TODO: just check the two active clients, instead of all of them
			for ( i=0 ; i< g_maxclients.integer ; i++ ) {
				cl = level.clients + i;
				if ( cl->pers.connected != CON_CONNECTED ) {
					continue;
				}
				if ( cl->sess.sessionTeam != TEAM_FREE ) {
					continue;
				}

				// if a player gives up in a duel, end the match
				if ( cl->giveUp == qtrue ) {
					LogExit( "Match forfeited." );
					trap_SendServerCommand( -1, va("notify %i\\\"%s" S_COLOR_YELLOW " forfeited the match.\n\"",
										NF_GAMEINFO, cl->pers.netname ) );
					trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
					return;
				}

			}

		}

	} else
	// misc team scorelimit
	if ( g_gametype.integer >= GT_CTF ) {

		if ( scorelimit ) {

			if ( level.teamScores[TEAM_RED] >= scorelimit ) {
				//trap_SendServerCommand( -1, "print \"Red team hit the scorelimit.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Red team hit the scorelimit.\n\"", NF_GAMEINFO) );
				LogExit( "Scorelimit hit." );
				trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
				return;
			}

			if ( level.teamScores[TEAM_BLUE] >= scorelimit ) {
				//trap_SendServerCommand( -1, "print \"Blue team hit the scorelimit.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Blue team hit the scorelimit.\n\"", NF_GAMEINFO) );
				LogExit( "Scorelimit hit." );
				trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
				return;
			}

		}

		if ( mercylimit ) {
			if ( (level.teamScores[TEAM_RED]-level.teamScores[TEAM_BLUE]) >= mercylimit || (level.teamScores[TEAM_BLUE]-level.teamScores[TEAM_RED]) >= mercylimit ) {
				//trap_SendServerCommand( -1, "print \"Blue team hit the scorelimit.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Mercylimit hit.\n\"", NF_GAMEINFO) );
				trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_TIMELIMIT ) );
				LogExit( "Mercylimit hit." );
				return;
			}
		}
	}
}


/*
=============
announceInvalidTeam

Announce to clients why team match has not started
=============
*/
void announceInvalidTeam( int reason ) {

	if ( g_gametype.integer < GT_TEAM ) {
		return;
	}

	// TODO: code this better
	switch (reason) {
		case TSOK_MORE_RED_PLAYERS:
			trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_RED "RED" S_COLOR_YELLOW " team needs more players.\n\"", NF_ERROR) );
			break;
		case TSOK_MORE_BLUE_PLAYERS:
			trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_BLUE "BLUE" S_COLOR_YELLOW " team needs more players.\n\"", NF_ERROR) );
			break;
		case TSOK_MORE_PLAYERS:
			trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Teams need more players.\n\"", NF_ERROR) );
			break;
		case TSOK_NEED_ANOTHER_PLAYER:
			trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Match needs another player.\n\"", NF_ERROR) );
			break;
	}

	level.teamSizeNagQueue = qfalse;

}


/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
=============
CheckTournament

Once a frame, check for changes in tournament player state
=============
*/
void CheckTournament( void ) {
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( level.numPlayingClients == 0 ) {
		return;
	}

	if ( g_gametype.integer == GT_TOURNAMENT ) {

		// pull in a spectator if needed
		/*if ( level.numPlayingClients < 2 ) {
			AddTournamentPlayer();
		}*/

		// if we don't have two players, or a vote is in progress, go back to warm up
		if ( level.numPlayingClients != 2 || level.voteTime || level.voteExecuteTime ) {
			if ( level.warmupTime != -1 ) {
				level.warmupTime = -1;
				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
				G_LogPrintf( "Warmup:\n" );
			}
			return;
		}

		if ( level.warmupTime == 0 ) {
			return;
		}

		// if the warmup is changed at the console, restart it
		if ( g_warmup.modificationCount != level.warmupModificationCount ) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if ( level.warmupTime < 0 ) {
			if ( level.numPlayingClients == 2 ) {
				// fudge by -1 to account for extra delays
				if ( level.rs_warmup > 1 ) {
					// check to see if both players are ready to exit warmup
					if ( level.endWarmup == qtrue || (
							( g_entities[level.sortedClients[0]].client->pers.ready ||
							( g_entities[level.sortedClients[0]].r.svFlags & SVF_BOT ) ) &&
							( g_entities[level.sortedClients[1]].client->pers.ready ||
							( g_entities[level.sortedClients[1]].r.svFlags & SVF_BOT ) ) ) ) {
						level.warmupTime = level.time + ( level.rs_warmup /*- 1*/ ) * 1000;
					}
				} else {
					level.warmupTime = 0;
				}

				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			}
			return;
		}

		// if the warmup time has counted down, restart
		if ( level.time > level.warmupTime ) {
			level.warmupTime += 10000;
			trap_Cvar_Set( "g_restarted", "1" );
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			return;
		}

	} else if ( g_gametype.integer != GT_SINGLE_PLAYER && level.warmupTime != 0 ) {
		int			counts[TEAM_NUM_TEAMS];
		qboolean	notEnough = qfalse;
		int 		i;
		int			clientsReady = 0;
		int			teamQuota;
		qboolean	teamsAllReady = qfalse; // display a message if team size quota is not meet

		if ( g_gametype.integer >= GT_TEAM ) {
			counts[TEAM_BLUE] = TeamCount( -1, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( -1, TEAM_RED );
			level.teamSizesOK = TSOK_OK;

			// set the minimum player required to start a match
			teamQuota = g_teamSizeQuota.integer;
			if ( teamQuota < 1 ) {
				if ( g_teamSize.integer > 0 ) {
					teamQuota = g_teamSize.integer;
				} else {
					teamQuota = 2;
				}
			}

			if ( g_gametype.integer >= GT_AA1 ) {

				if ( counts[TEAM_RED] < teamQuota ) {
					notEnough = qtrue;
					level.teamSizesOK |= TSOK_MORE_RED_PLAYERS; // |= 1
				}
				if ( counts[TEAM_BLUE] < 1 ) {
					notEnough = qtrue;
					level.teamSizesOK |= TSOK_MORE_BLUE_PLAYERS; // |= 2
				} else if ( counts[TEAM_BLUE] > 1 ) {
					notEnough = qtrue;
				}

			} else {

				if ( counts[TEAM_RED] < teamQuota ) {
					level.teamSizesOK |= TSOK_MORE_RED_PLAYERS; // |= 1
					notEnough = qtrue;
				}
				if ( counts[TEAM_BLUE] < teamQuota ) {
					level.teamSizesOK |= TSOK_MORE_BLUE_PLAYERS; // |= 2
					notEnough = qtrue;
				}

			}

		} else if ( level.numPlayingClients < 2 ) {
			notEnough = qtrue;
		}

		// check to see if all players are ready to exit warmup
		if( level.rs_warmup ){

			// are we not forcing warmup to end?
			if ( level.endWarmup == qfalse ) {

				for( i = 0; i < level.numPlayingClients; i++ ){
					if( ( g_entities[level.sortedClients[i]].client->pers.ready || g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT ) && g_entities[level.sortedClients[i]].inuse )
						clientsReady++;
				}

				if (clientsReady < level.numPlayingClients) {
					notEnough = qtrue;
					level.teamSizeNagQueue = qtrue;
				} else {
					teamsAllReady = qtrue;
				}

			}

		}

		//mmp - if we don't have enough players, or a vote is in progress, go back to warm up
		if ( notEnough || level.voteTime || level.voteExecuteTime ) {
			if ( level.warmupTime != -1 ) {
				level.warmupTime = -1;
				trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
				G_LogPrintf( "Warmup:\n" );

				level.teamSizeNagQueue = qtrue;
			}

			if ( level.rs_warmup && teamsAllReady == qtrue && level.teamSizeNagQueue == qtrue ) {
				announceInvalidTeam( level.teamSizesOK );
			}

			return; // still waiting for team members
		}

		if ( level.warmupTime == 0 ) {
			return;
		}

		// if the warmup is changed at the console, restart it
		if ( g_warmup.modificationCount != level.warmupModificationCount ) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		/*if ( g_gametype.integer == GT_AA1 ) {
			if (counts[TEAM_BLUE] > 1) {
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_YELLOW "Warm up restarted, blue team can only have one player.\n\"", NF_ERROR) );
				G_UnreadyPlayers(); // unready players
				return;
			}
		}*/

		// if all players have arrived and readied up, start the countdown
		if ( level.warmupTime < 0 ) {
			// fudge by -1 to account for extra delays (mmp - why?)
			if ( level.rs_warmup > 1 ) {
				level.warmupTime = level.time + ( level.rs_warmup /*- 1*/ ) * 1000;
			} else {
				level.warmupTime = 0;
			}

			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			return;
		}

		// if the warmup time has counted down, restart
		if ( level.time > level.warmupTime ) {
			level.warmupTime += 10000;
			trap_Cvar_Set( "g_restarted", "1" );
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			return;
		}
	}
}


/*
==================
CheckVote
==================
*/
void CheckVote( void ) {
	if ( level.voteExecuteTime && level.voteExecuteTime < level.time ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}
	if ( !level.voteTime ) {
		return;
	}
	if ( level.time - level.voteTime >= VOTE_TIME ) {
		//trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
		trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_RED "Vote failed.\n\"", NF_GAMEINFO) );
	} else {
		if ( level.currentVoteIsKick == qtrue ) {
			// require more votes to kick a player
			if ( level.voteYes >= level.numVotingClients*0.75 ) {
				// execute the command, then remove the vote
				//trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_WHITE "Vote passed.\n\"", NF_GAMEINFO) );
				level.voteExecuteTime = level.time + 3000;
			} else if ( level.voteNo > level.numVotingClients*0.25 ) {
				// same behavior as a timeout
				//trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_RED "Vote failed.\n\"", NF_GAMEINFO) );
			} else {
				// still waiting for a 3/4 majority
				return;
			}
		} else {
			// ATVI Q3 1.32 Patch #9, WNF
			if ( level.voteYes > level.numVotingClients/2 ) {
				// execute the command, then remove the vote
				//trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_WHITE "Vote passed.\n\"", NF_GAMEINFO) );
				level.voteExecuteTime = level.time + 3000;
			} else if ( level.voteNo >= level.numVotingClients/2 ) {
				// same behavior as a timeout
				//trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
				trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_RED "Vote failed.\n\"", NF_GAMEINFO) );
			} else {
				// still waiting for a majority
				return;
			}
		}
	}
	level.voteCoolDownStart = level.time; // set cool down time for voting
	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );

}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char *message) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		trap_SendServerCommand( i, message );
	}
}

/*
==================
SetLeader
==================
*/
void SetLeader(int team, int client) {
	int i;

	if ( level.clients[client].pers.connected == CON_DISCONNECTED ) {
		PrintTeam(team, va("print \"%s is not connected\n\"", level.clients[client].pers.netname) );
		return;
	}
	if (level.clients[client].sess.sessionTeam != team) {
		PrintTeam(team, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname) );
		return;
	}
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader) {
			level.clients[i].sess.teamLeader = qfalse;
			ClientUserinfoChanged(i);
		}
	}
	level.clients[client].sess.teamLeader = qtrue;
	ClientUserinfoChanged( client );
	PrintTeam(team, va("print \"%s is the new team leader\n\"", level.clients[client].pers.netname) );
}

/*
==================
CheckTeamLeader
==================
*/
void CheckTeamLeader( int team ) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		if (level.clients[i].sess.teamLeader)
			break;
	}
	if (i >= level.maxclients) {
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if (level.clients[i].sess.sessionTeam != team)
				continue;
			if (!(g_entities[i].r.svFlags & SVF_BOT)) {
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}

		if (i >= level.maxclients) {
			for ( i = 0 ; i < level.maxclients ; i++ ) {
				if (level.clients[i].sess.sessionTeam != team)
					continue;
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}
	}
}

/*
==================
CheckTeamVote
==================
*/
void CheckTeamVote( int team ) {
	int cs_offset;

	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		return;
	}
	if ( level.time - level.teamVoteTime[cs_offset] >= VOTE_TIME ) {
		trap_SendServerCommand( -1, "print \"Team vote failed.\n\"" );
	} else {
		if ( level.teamVoteYes[cs_offset] > level.numteamVotingClients[cs_offset]/2 ) {
			// execute the command, then remove the vote
			trap_SendServerCommand( -1, "print \"Team vote passed.\n\"" );
			//
			if ( !Q_strncmp( "leader", level.teamVoteString[cs_offset], 6) ) {
				//set the team leader
				SetLeader(team, atoi(level.teamVoteString[cs_offset] + 7));
			}
			else {
				trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.teamVoteString[cs_offset] ) );
			}
		} else if ( level.teamVoteNo[cs_offset] >= level.numteamVotingClients[cs_offset]/2 ) {
			// same behavior as a timeout
			trap_SendServerCommand( -1, "print \"Team vote failed.\n\"" );
		} else {
			// still waiting for a majority
			return;
		}
	}
	level.teamVoteTime[cs_offset] = 0;
	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, "" );

}

/*
==================
CheckServerEmpty

Checks to see if there are no players left on the server
If empty, server will execute a command via cvar 'g_serviceOnEmptyExec'
==================
*/
static void CheckServerEmpty( void ) {

	// value less than zero disables this function
	if ( g_serviceOnEmptyTime.integer < 0 ) {
		return;
	}

	if ( g_clientCount.integer > 0 ) {
		level.lastActiveTime = level.time;
	} else if (level.lastActiveTime > 0) {
		if ( level.lastActiveTime + g_serviceOnEmptyTime.integer * 1000 < level.time ) {
			level.lastActiveTime = 0;
			G_LogPrintf("CheckServerEmpty: Last client left, executing command '%s'\n", g_serviceOnEmptyExec.string );
			trap_SendConsoleCommand( EXEC_APPEND, va("%s\n",g_serviceOnEmptyExec.string) );
		}
	}

}


/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) {
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) {
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) {
			trap_Cvar_Set( "g_needpass", "1" );
		} else {
			trap_Cvar_Set( "g_needpass", "0" );
		}
	}
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent) {
//	float	thinktime;
	int		thinktime;

	thinktime = ent->nextthink;
	if (thinktime <= 0) {
		return;
	}
	if (thinktime > level.time) {
		return;
	}

	ent->nextthink = 0;
	if (!ent->think) {
		G_Error ( "NULL ent->think");
	}
	ent->think (ent);
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) {
	int			i;
	gentity_t	*ent;

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted ) {
		return;
	}

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;

	// get any cvar changes
	G_UpdateCvars();

	if ( level.updateInfo == qtrue ) {
		G_InfoUpdate();
	}

	// run cointoss
	G_RandomSelectUpdate();

	//
	// go through all allocated objects
	//
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse ) {
			continue;
		}

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) {
			if ( ent->s.event ) {
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				if ( ent->client ) {
					ent->client->ps.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			if ( ent->freeAfterEvent ) {
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity( ent );
				continue;
			} else if ( ent->unlinkAfterEvent ) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity( ent );
			}
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		if ( !ent->r.linked && ent->neverFree ) {
			continue;
		}

		if ( ent->s.eType == ET_MISSILE ) {
			G_RunMissile( ent );
			continue;
		}

		if ( ent->s.eType == ET_ITEM || ent->physicsObject ) {
			G_RunItem( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER ) {
			G_RunMover( ent );
			continue;
		}

		if ( i < MAX_CLIENTS ) {
			G_RunClient( ent );
			continue;
		}

		G_RunThink( ent );
	}

	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++ ) {
		if ( ent->inuse ) {
			ClientEndFrame( ent );
		}
	}

	// see if it is time to do a tournament restart
	CheckTournament();

	// see if it is time to end the level
	CheckExitRules();

	// update effects of overtime/sudden death
	G_TimeCompulsionUpdate();

	// check if we need to update the specmode function
	UpdateSpecMode();

	// check if ruleset needs to be updated
	if ( level.updateRuleset == qtrue ) {
		if ( level.warmupTime ) {
			G_RuleSetUpdate();
			level.rulesetViolated = qfalse;
		} else {
			// the ruleset cannot be updated during a game
			level.updateRuleset = qfalse;
		}
	} else
	if ( level.forcefullyUpdateRuleset == qtrue ) {
		// the game itself wants to update the ruleset, just do it, shuts the violation system the fuck up
		G_RuleSetUpdate();
		level.rulesetViolated = qfalse;
	}

	// "I'm the man" - Joe Jackson

	// update to team status?
	CheckTeamStatus();

	// update server info status
	G_UpdateStatus();

	// cancel vote if timed out
	CheckVote();

	// check team votes
	/*CheckTeamVote( TEAM_RED );
	CheckTeamVote( TEAM_BLUE );*/

	// server maintenance when players leave
	CheckServerEmpty();

	// for tracking changes
	CheckCvars();

	if (g_listEntity.integer) {
		for (i = 0; i < MAX_GENTITIES; i++) {
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		trap_Cvar_Set("g_listEntity", "0");
	}
}







