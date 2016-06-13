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
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"
#ifdef MISSIONPACK
#include "../../ui/menudef.h"

typedef struct {
	const char *order;
	int taskNum;
} orderTask_t;

static const orderTask_t validOrders[] = {
	{ VOICECHAT_GETFLAG,						TEAMTASK_OFFENSE },
	{ VOICECHAT_OFFENSE,						TEAMTASK_OFFENSE },
	{ VOICECHAT_DEFEND,							TEAMTASK_DEFENSE },
	{ VOICECHAT_DEFENDFLAG,					TEAMTASK_DEFENSE },
	{ VOICECHAT_PATROL,							TEAMTASK_PATROL },
	{ VOICECHAT_CAMP,								TEAMTASK_CAMP },
	{ VOICECHAT_FOLLOWME,						TEAMTASK_FOLLOW },
	{ VOICECHAT_RETURNFLAG,					TEAMTASK_RETRIEVE },
	{ VOICECHAT_FOLLOWFLAGCARRIER,	TEAMTASK_ESCORT }
};

static const int numValidOrders = ARRAY_LEN(validOrders);

static int CG_ValidOrder(const char *p) {
	int i;
	for (i = 0; i < numValidOrders; i++) {
		if (Q_stricmp(p, validOrders[i].order) == 0) {
			return validOrders[i].taskNum;
		}
	}
	return -1;
}
#endif

/*
=================
CG_ParseScores

=================
*/

#define		CG_PARSESCOREBUFFERSIZE		13
static void CG_ParseScores( void ) {
	int		i, powerups;

	cg.numScores = atoi( CG_Argv( 1 ) );
	if ( cg.numScores > MAX_CLIENTS ) {
		cg.numScores = MAX_CLIENTS;
	}

	cg.teamScores[0] = atoi( CG_Argv( 2 ) );
	cg.teamScores[1] = atoi( CG_Argv( 3 ) );

	memset( cg.scores, 0, sizeof( cg.scores ) );
	for ( i = 0 ; i < cg.numScores ; i++ ) {

		// mmp - ok, like, when you remove an award or something, update this ffs
		//       otherwise, get ready for some fucked up scoreboards
		cg.scores[i].client = atoi( CG_Argv( i * CG_PARSESCOREBUFFERSIZE + 4 ) );
		cg.scores[i].score = atoi( CG_Argv( i * CG_PARSESCOREBUFFERSIZE + 5 ) );
		cg.scores[i].ping = atoi( CG_Argv( i * CG_PARSESCOREBUFFERSIZE + 6 ) );
		cg.scores[i].time = atoi( CG_Argv( i * CG_PARSESCOREBUFFERSIZE + 7 ) );
		cg.scores[i].scoreFlags = atoi( CG_Argv( i * CG_PARSESCOREBUFFERSIZE + 8 ) );
		powerups = atoi( CG_Argv( i * CG_PARSESCOREBUFFERSIZE + 9 ) );
		cg.scores[i].accuracy = atoi(CG_Argv(i * CG_PARSESCOREBUFFERSIZE + 10));
		cg.scores[i].excellentCount = atoi(CG_Argv(i * CG_PARSESCOREBUFFERSIZE + 11));
		cg.scores[i].guantletCount = atoi(CG_Argv(i * CG_PARSESCOREBUFFERSIZE + 12));
		cg.scores[i].defendCount = atoi(CG_Argv(i * CG_PARSESCOREBUFFERSIZE + 13));
		cg.scores[i].assistCount = atoi(CG_Argv(i * CG_PARSESCOREBUFFERSIZE + 14));
		cg.scores[i].perfect = atoi(CG_Argv(i * CG_PARSESCOREBUFFERSIZE + 15));
		cg.scores[i].captures = atoi(CG_Argv(i * CG_PARSESCOREBUFFERSIZE + 16));
		// also, the old code is soo noobish

		/*cg.scores[i].client = atoi( CG_Argv( i * 14 + 4 ) );
		cg.scores[i].score = atoi( CG_Argv( i * 14 + 5 ) );
		cg.scores[i].ping = atoi( CG_Argv( i * 14 + 6 ) );
		cg.scores[i].time = atoi( CG_Argv( i * 14 + 7 ) );
		cg.scores[i].scoreFlags = atoi( CG_Argv( i * 14 + 8 ) );
		powerups = atoi( CG_Argv( i * 14 + 9 ) );
		cg.scores[i].accuracy = atoi(CG_Argv(i * 14 + 10));
		cg.scores[i].impressiveCount = atoi(CG_Argv(i * 14 + 11));
		cg.scores[i].excellentCount = atoi(CG_Argv(i * 14 + 12));
		cg.scores[i].guantletCount = atoi(CG_Argv(i * 14 + 13));
		cg.scores[i].defendCount = atoi(CG_Argv(i * 14 + 14));
		cg.scores[i].assistCount = atoi(CG_Argv(i * 14 + 15));
		cg.scores[i].perfect = atoi(CG_Argv(i * 14 + 16));
		cg.scores[i].captures = atoi(CG_Argv(i * 14 + 17));*/

		if ( cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS ) {
			cg.scores[i].client = 0;
		}
		cgs.clientinfo[ cg.scores[i].client ].score = cg.scores[i].score;
		cgs.clientinfo[ cg.scores[i].client ].powerups = powerups;

		cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;
	}
#ifdef MISSIONPACK
	CG_SetScoreSelection(NULL);
#endif

}

/*
=================
CG_ParseTeamInfo

=================
*/

#define		CG_PLAYERINFOSLOTSIZE	11 // was 6
static void CG_ParseTeamInfo( void ) {
	int		i;
	int		client;

	numSortedTeamPlayers = atoi( CG_Argv( 1 ) );
	if( numSortedTeamPlayers < 0 || numSortedTeamPlayers > TEAM_MAXOVERLAY )
	{
		CG_Error( "CG_ParseTeamInfo: numSortedTeamPlayers out of range (%d)",
				numSortedTeamPlayers );
		return;
	}

	for ( i = 0 ; i < numSortedTeamPlayers ; i++ ) {
		client = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 2 ) );
		if( client < 0 || client >= MAX_CLIENTS )
		{
		  CG_Error( "CG_ParseTeamInfo: bad client number: %d", client );
		  return;
		}

		sortedTeamPlayers[i] = client;

		// maybe at some point, i'll add the player's armor and damage levels
		cgs.clientinfo[ client ].location = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 3 ) );
		cgs.clientinfo[ client ].health = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 4 ) );
		cgs.clientinfo[ client ].armor = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 5 ) );

		cgs.clientinfo[ client ].damageLvl = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 6 ) );
		cgs.clientinfo[ client ].armorLvl = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 7 ) );
		cgs.clientinfo[ client ].keycards = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 8 ) );

		cgs.clientinfo[ client ].curWeapon = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 9 ) );
		cgs.clientinfo[ client ].powerups = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 10 ) );

		cgs.clientinfo[ client ].posXLoc = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 11 ) );
		cgs.clientinfo[ client ].posYLoc = atoi( CG_Argv( i * CG_PLAYERINFOSLOTSIZE + 12 ) );
	}
}


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars

TODO: this shit is messy, clean it
================
*/
int CG_BitOutput( int bitFlags, int flagSet ) {
	if ( bitFlags & flagSet ) {
		return 1;
	}
	return 0;
}


void CG_ParseServerinfo( void ) {
	const char	*info;
	char		*mapname;
	char		svrInfo[1024];
	int			bitFlags;
	int			i;

	info = CG_ConfigString( CS_SERVERINFO );
	cgs.gametype = atoi( Info_ValueForKey( info, "g_gametype" ) );
	trap_Cvar_Set("g_gametype", va("%i", cgs.gametype));

	cgs.dmflags = 0;
	cgs.teamflags = 0; // not used?

	Q_strncpyz( svrInfo, Info_ValueForKey( info, "info"), sizeof(svrInfo) );
	//trap_Cvar_Set("info", svrInfo); // do we need this?
	/*Com_Printf( S_COLOR_ORANGE "DEBUG: %s\n", svrInfo);
	CG_AddToHUDInfo ( hud_notifyBoxRoute.integer, va(S_COLOR_ORANGE "DEBUG: %s\n", svrInfo), 0, 0 );*/

	bitFlags = ( Com_HexToByte( svrInfo, INFO_POS_BITFLAGS_HI ) << 8 ) | Com_HexToByte( svrInfo, INFO_POS_BITFLAGS_LO ) ;

	cgs.timelimit = (float)Com_HexToByte( svrInfo, INFO_POS_TIMELIMIT );
	cgs.overtime = Com_HexToByte( svrInfo, INFO_POS_OVERTIME );
	cgs.scorelimit = ( Com_HexToByte( svrInfo, INFO_POS_SCORELIMIT_HI ) << 8 ) | Com_HexToByte( svrInfo, INFO_POS_SCORELIMIT_LO ) ;
	cgs.mercylimit = ( Com_HexToByte( svrInfo, INFO_POS_MERCYLIMIT_HI ) << 8 ) | Com_HexToByte( svrInfo, INFO_POS_MERCYLIMIT_LO ) ;

	cgs.matchMode = Com_HexToByte( svrInfo, INFO_POS_MATCHMODE );
	cgs.weaponRespawn = Com_HexToByte( svrInfo, INFO_POS_WEAPONRESPAWN );
	cgs.forceRespawn = Com_HexToByte( svrInfo, INFO_POS_FORCERESPAWN );

	// bit flags
	cgs.friendlyFire = CG_BitOutput( bitFlags, INFO_BIT_FRIENDLYFIRE );
	cgs.teamLocOverlay = CG_BitOutput( bitFlags, INFO_BIT_TEAMLOCOVERLAY );
	cgs.hitSound = CG_BitOutput( bitFlags, INFO_BIT_HITSOUND );
	cgs.randomSpawn = CG_BitOutput( bitFlags, INFO_BIT_RANDOMSPAWN );
	cgs.physicsMode = CG_BitOutput( bitFlags, INFO_BIT_FREESELECT_PHYSICS );
	cgs.quadMode = CG_BitOutput( bitFlags, INFO_BIT_QUADMODE );
	cgs.selfDamage = CG_BitOutput( bitFlags, INFO_BIT_SELFDAMAGE );
	cgs.doubleAmmo = CG_BitOutput( bitFlags, INFO_BIT_DOUBLEAMMO );
	cgs.keycardRespawn = Com_HexToByte( svrInfo, INFO_POS_KEYCARDRESPAWN );
	cgs.keycardDropable = CG_BitOutput( bitFlags, INFO_BIT_KEYCARD_DROPABLE );
	cgs.noArenaGrenades = CG_BitOutput( bitFlags, INFO_BIT_KEYCARD_DROPABLE );
	cgs.noArenaLightningGun = CG_BitOutput( bitFlags, INFO_BIT_KEYCARD_DROPABLE );
	cgs.powerUps = CG_BitOutput( bitFlags, INFO_BIT_POWERUPS );
	cgs.armor = CG_BitOutput( bitFlags, INFO_BIT_ARMOR );
	cgs.popCTF = CG_BitOutput( bitFlags, INFO_BIT_POPCTF );

	// ext'd bit flags
	bitFlags = ( Com_HexToByte( svrInfo, INFO_POS_BITFLAGSEXT_HI ) << 8 ) | Com_HexToByte( svrInfo, INFO_POS_BITFLAGSEXT_LO ) ;
	cgs.shortGame = CG_BitOutput( bitFlags, INFO_BIT_SHORTGAME );
	cgs.roundBasedMatches = CG_BitOutput( bitFlags, INFO_BIT_ROUND_BASED_MATCHES );

	cgs.teamSize = Com_HexToByte( svrInfo, INFO_POS_TEAMSIZE );

	// shorten the timelimit by half
	if (cgs.shortGame)
		cgs.timelimit = cgs.timelimit / 2;

	cgs.fullTimelimit = cgs.timelimit; // copy timelimit, so when a match is divided in rounds, the total time for a match can be displayed

	// split the timelimit for 2 rounds
	if (cgs.roundBasedMatches && cgs.gametype != GT_FFA && cgs.gametype != GT_AA1)
		cgs.timelimit = cgs.timelimit / 2;

	//Com_Printf( S_COLOR_ORANGE "DEBUG: %i\n", cgs.roundBasedMatches);

/*	cgs.scorelimit = ( Com_HexToByte( svrInfo, INFO_POS_SCORELIMIT_HI ) << 8 ) | Com_HexToByte( svrInfo, INFO_POS_SCORELIMIT_LO ) ;*/

	cgs.ruleSet = atoi( Info_ValueForKey( info, "g_ruleset" ) );

	/*cgs.scorelimit = atoi( Info_ValueForKey( info, "scorelimit" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.overtime = atoi( Info_ValueForKey( info, "overtime" ) );*/

	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
	Q_strncpyz( cgs.mapdispname, Info_ValueForKey( info, "mapname" ), sizeof(cgs.mapdispname) ); // for use with end-game stats

	Q_strncpyz( cgs.redTeam, Info_ValueForKey( info, "g_redTeam" ), sizeof(cgs.redTeam) );
	trap_Cvar_Set("g_redTeam", cgs.redTeam);
	Q_strncpyz( cgs.blueTeam, Info_ValueForKey( info, "g_blueTeam" ), sizeof(cgs.blueTeam) );
	trap_Cvar_Set("g_blueTeam", cgs.blueTeam);

	// if weapon mode is set to 2 or 3, then register all weapons
	if ( cgs.matchMode > 1 ) {
		for (i = WP_NUM_WEAPONS - 1 ; i > 0 ; i--) {
			CG_RegisterWeapon( i );
		}
	}

//unlagged - server options
	// we'll need this for deciding whether or not to predict weapon effects
//	cgs.delagHitscan = atoi( Info_ValueForKey( info, "g_delagHitscan" ) );
//	trap_Cvar_Set("g_delagHitscan", va("%i", cgs.delagHitscan));
	cgs.delagHitscan = 1;
//unlagged - server options

	// reset 'con_notifytime' to -1
	// future versions of mfarena will remove the notify drawing function from the engine
	//trap_Cvar_Set ( "con_notifytime","-1");// disables Con_DrawNotify() when state is received

}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup( void ) {
	const char	*info;
	int			warmup;

	info = CG_ConfigString( CS_WARMUP );

	warmup = atoi( info );
	cg.warmupCount = -1;

	if ( warmup == 0 && cg.warmup ) {

	} else if ( warmup > 0 && cg.warmup <= 0 ) {
#ifdef MISSIONPACK
		if (cgs.gametype >= GT_CTF && cgs.gametype <= GT_HARVESTER) {
			trap_S_StartLocalSound( cgs.media.countPrepareTeamSound, CHAN_ANNOUNCER );
		} else
#endif
		{
			trap_S_StartLocalSound( cgs.media.countPrepareSound, CHAN_ANNOUNCER );
		}
	}

	cg.warmup = warmup;
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void ) {
	const char *s;

	cgs.scores1 = atoi( CG_ConfigString( CS_SCORES1 ) );
	cgs.scores2 = atoi( CG_ConfigString( CS_SCORES2 ) );
	cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );
	if( cgs.gametype == GT_CTF ) {
		s = CG_ConfigString( CS_FLAGSTATUS );
		cgs.redflag = s[0] - '0';
		cgs.blueflag = s[1] - '0';
	}
#ifdef MISSIONPACK
	else if( cgs.gametype == GT_1FCTF ) {
		s = CG_ConfigString( CS_FLAGSTATUS );
		cgs.flagStatus = s[0] - '0';
	}
#endif
	cg.warmup = atoi( CG_ConfigString( CS_WARMUP ) );
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void) {
	char originalShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	char timeOffset[16];
	const char *o;
	char *n,*t;

	o = CG_ConfigString( CS_SHADERSTATE );
	while (o && *o) {
		n = strstr(o, "=");
		if (n && *n) {
			strncpy(originalShader, o, n-o);
			originalShader[n-o] = 0;
			n++;
			t = strstr(n, ":");
			if (t && *t) {
				strncpy(newShader, n, t-n);
				newShader[t-n] = 0;
			} else {
				break;
			}
			t++;
			o = strstr(t, "@");
			if (o) {
				strncpy(timeOffset, t, o-t);
				timeOffset[o-t] = 0;
				o++;
				trap_R_RemapShader( originalShader, newShader, timeOffset );
			}
		} else {
			break;
		}
	}
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void ) {
	const char	*str;
	int		num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	trap_GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC ) {
		CG_StartMusic();
	} else if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if ( num == CS_WARMUP ) {
		CG_ParseWarmup();
	} else if ( num == CS_SCORES1 ) {
		cgs.scores1 = atoi( str );
	} else if ( num == CS_SCORES2 ) {
		cgs.scores2 = atoi( str );
	} else if ( num == CS_LEVEL_START_TIME ) {
		cgs.levelStartTime = atoi( str );
	} else if ( num == CS_VOTE_TIME ) {
		cgs.voteTime = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_YES ) {
		cgs.voteYes = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_NO ) {
		cgs.voteNo = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_STRING ) {
		Q_strncpyz( cgs.voteString, str, sizeof( cgs.voteString ) );
		//cgs.voteFlash = cg.time + 512; // flash twice
		//trap_S_StartLocalSound( cgs.media.voteNow, CHAN_ANNOUNCER );
	} else if ( num >= CS_TEAMVOTE_TIME && num <= CS_TEAMVOTE_TIME + 1) {
		cgs.teamVoteTime[num-CS_TEAMVOTE_TIME] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_TIME] = qtrue;
	} else if ( num >= CS_TEAMVOTE_YES && num <= CS_TEAMVOTE_YES + 1) {
		cgs.teamVoteYes[num-CS_TEAMVOTE_YES] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_YES] = qtrue;
	} else if ( num >= CS_TEAMVOTE_NO && num <= CS_TEAMVOTE_NO + 1) {
		cgs.teamVoteNo[num-CS_TEAMVOTE_NO] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_NO] = qtrue;
	} else if ( num >= CS_TEAMVOTE_STRING && num <= CS_TEAMVOTE_STRING + 1) {
		Q_strncpyz( cgs.teamVoteString[num-CS_TEAMVOTE_STRING], str, sizeof( cgs.teamVoteString[0] ) );
#ifdef MISSIONPACK
		trap_S_StartLocalSound( cgs.media.voteNow, CHAN_ANNOUNCER );
#endif
	} else if ( num == CS_INTERMISSION ) {
		cg.intermissionStarted = atoi( str );
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.gameModels[ num-CS_MODELS ] = trap_R_RegisterModel( str );
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS ) {
		if ( str[0] != '*' ) {	// player specific sounds don't register here
			cgs.gameSounds[ num-CS_SOUNDS] = trap_S_RegisterSound( str, qfalse );
		}
	} else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS ) {
		CG_NewClientInfo( num - CS_PLAYERS );
		CG_BuildSpectatorString();
	} else if ( num == CS_FLAGSTATUS ) {
		if( cgs.gametype == GT_CTF ) {
			// format is rb where its red/blue, 0 is at base, 1 is taken, 2 is dropped
			cgs.redflag = str[0] - '0';
			cgs.blueflag = str[1] - '0';
		}
#ifdef MISSIONPACK
		else if( cgs.gametype == GT_1FCTF ) {
			cgs.flagStatus = str[0] - '0';
		}
#endif
	}
	else if ( num == CS_SHADERSTATE ) {
		CG_ShaderStateChanged();
	}
	else if ( num == CS_OVERTIME ) {
		// update overtime
		cgs.overtimeSets = atoi( str );
	}
	else if ( num == CS_ROUND ) {
		// update overtime
		cgs.currentRound = atoi( str );
	}

}


/*
=======================
CG_ParseSndCall

=======================
*/

void CG_ParseSndCall( int sndCode ) {

	switch ( sndCode ) {
		case SC_CONNECT:
			trap_S_StartLocalSound( cgs.media.toneConnect1, CHAN_ANNOUNCER );
			break;
		case SC_DISCONNECT:
			trap_S_StartLocalSound( cgs.media.toneDisconnect1, CHAN_ANNOUNCER );
			break;
		case SC_TIMELIMIT:
			trap_S_StartLocalSound( cgs.media.timelimit, CHAN_ANNOUNCER );
			break;
		case SC_OVERTIME:
			trap_S_StartLocalSound( cgs.media.overtime, CHAN_ANNOUNCER );
			// reset timelimit based alerts
			cg.timelimitWarnings = 0;
			cg.timelimitTicks = 0;
			break;
		case SC_EXPLOSION:
			trap_S_StartLocalSound( cgs.media.sfx_rockexp, CHAN_ANNOUNCER );
			break;
		case SC_EXPLOSION_GLOBAL:
			trap_S_StartLocalSound( cgs.media.sfx_exp_global, CHAN_AUTO ); // testing as a global sound event, most likely will be removed
			break;

		case SC_AN_SHIT:
			trap_S_StartLocalSound( cgs.media.an_shit, CHAN_ANNOUNCER );
			break;
		case SC_AN_YES:
			trap_S_StartLocalSound( cgs.media.an_yes, CHAN_ANNOUNCER );
			break;
		case SC_AN_GREAT:
			trap_S_StartLocalSound( cgs.media.an_great, CHAN_ANNOUNCER );
			break;
		case SC_AN_OK:
			trap_S_StartLocalSound( cgs.media.an_ok, CHAN_ANNOUNCER );
			break;
		case SC_AN_EXCELLENT:
			trap_S_StartLocalSound( cgs.media.an_excellent, CHAN_ANNOUNCER );
			break;
		case SC_AN_SUPERB:
			trap_S_StartLocalSound( cgs.media.an_superb, CHAN_ANNOUNCER );
			break;
		case SC_AN_KEEPITUP:
			trap_S_StartLocalSound( cgs.media.an_keepItUp, CHAN_ANNOUNCER );
			break;
		case SC_AN_WONDERFUL:
			trap_S_StartLocalSound( cgs.media.an_wonderful, CHAN_ANNOUNCER );
			break;
		case SC_AN_FUCKEM:
			trap_S_StartLocalSound( cgs.media.an_fuckem, CHAN_ANNOUNCER );
			break;
		case SC_AN_YOUSUCK:
			trap_S_StartLocalSound( cgs.media.an_youSuck, CHAN_ANNOUNCER );
			break;

		case SC_AN_COMBO_BREAKER:
			trap_S_StartLocalSound( cgs.media.an_cBreaker, CHAN_ANNOUNCER );
			break;
		case SC_AN_PERFECT:
			trap_S_StartLocalSound( cgs.media.an_perfect, CHAN_ANNOUNCER );
			break;

	}

}

/*
=======================
CG_ParseRuleSet

TODO: revert to server info, as this function will not work in demos
=======================
*/

void CG_ParseRuleSet() {

	/*cgs.ruleSet = atoi ( CG_Argv ( 1 ) );

	cgs.timelimit = atoi ( CG_Argv ( 2 ) );
	cgs.overtime = atoi ( CG_Argv ( 3 ) );
	cgs.scorelimit = atoi ( CG_Argv ( 4 ) );
	cgs.physicsMode = atoi ( CG_Argv ( 5 ) );

	cgs.friendlyFire = atoi ( CG_Argv ( 6 ) );
	cgs.weaponRespawn = atoi ( CG_Argv ( 7 ) );
	cgs.forceRespawn = atoi ( CG_Argv ( 8 ) );
	cgs.teamLocOverlay = atoi ( CG_Argv ( 9 ) );
	cgs.hitSound = atoi ( CG_Argv ( 10 ) );
	cgs.scoreBalance = atoi ( CG_Argv ( 11 ) );

	cgs.quadMode = atoi ( CG_Argv ( 12 ) );
	cgs.selfDamage = atoi ( CG_Argv ( 13 ) );
	cgs.doubleAmmo = atoi ( CG_Argv ( 14 ) );
	cgs.keycardRespawn = atoi ( CG_Argv ( 15 ) );
	cgs.keycardDropable = atoi ( CG_Argv ( 16 ) );

	cgs.randomSpawn = atoi ( CG_Argv ( 17 ) );*/

	cgs.serverInfoLoad = -1; // if serverInfoLoad & 63 == 5, it needs to be parsed

}


/*
=======================
CG_AddToHUDInfo

setting a value for emoticon, enables the function
however, it is meant only for chat text
=======================
*/
void CG_AddToHUDInfo( int hudBox, const char *str, int emoticon, int hideInfo ) {
	int			len;
	char		*p, *ls;
	const char	*advstr;
	int			lastcolor;
	int			hudInfoHeight;
	int			emoticonSkipPos = 0;
	int			shaft;
	qboolean	shaftEnded;

	//Com_Printf( S_COLOR_ORANGE "DEBUG: %i\n", emoticon);

	if ( hudBox >= TOTAL_HUD_TEXT_BOXES ) {
		return;
	}

	switch ( hudBox ) {
		case 0:
			hudInfoHeight = hud_infoBox0_lines.integer;
			break;
		case 1:
			hudInfoHeight = hud_infoBox1_lines.integer;
			break;
		case 2:
			hudInfoHeight = hud_infoBox2_lines.integer;
			break;
		case 3:
			hudInfoHeight = hud_infoBox3_lines.integer;
			break;
	}

	//hudInfoHeight = CHAT_HEIGHT;

	//CG_AddEmoticons ( *str );

	len = 0;

//	p = cgs.hudInfoMsgs[hudBox][cgs.hudInfoPos[hudBox] % hudInfoHeight];
	p = cgs.hudBoardInfoMsgs[cgs.hudBoardInfoPos % HUD_BOARD_TEXT_HEIGHT]; // MMP - might not need the '% HUD_BOARD_TEXT_HEIGHT' part
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while (*str) {
		if (len > HUD_TEXT_BUFFER_SIZE - 1) {
			/*
			if (ls) {
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			cgs.hudInfoMsgTimes[hudBox][cgs.hudInfoPos[hudBox] % hudInfoHeight] = cg.time;

			cgs.hudInfoPos[hudBox]++;
			p = cgs.hudInfoMsgs[hudBox][cgs.hudInfoPos[hudBox] % hudInfoHeight];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
			*/

			break;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}

		// emoticons
		if ( emoticon && hud_useEmoticons.integer ) {
			if ( (emoticonSkipPos + 1) == emoticon ) {
				//Com_Printf( S_COLOR_SKY "DEBUG: %i=='%i'\n", emoticonSkipPos, emoticon);
				if ( *str == '<' ) {
					advstr = str + 1;
					if ( *advstr == '3' ) {
						*p++ = '\x14';
						str+=2;
						continue;
					}
				} else
				if ( *str == ';' ) {
					advstr = str + 1;
					if ( *advstr == ')' ) {
						*p++ = '\x08';
						str+=2;
						continue;
					}
				} else
				if ( *str == ':' || *str == '=' ) {
					advstr = str + 1;
					if ( *advstr == 'Z' ) {
						*p++ = '\x04';
						str+=2;
						continue;
					} else
					if ( *advstr == '3' ) {
						*p++ = '\x05';
						str+=2;
						continue;
					} else
					if ( *advstr == 'S' ) {
						*p++ = '\x06';
						str+=2;
						continue;
					} else
					if ( *advstr == 'b' ) {
						*p++ = '\x07';
						str+=2;
						continue;
					} else
					if ( *advstr == 'O' ) {
						// can't use 0x0A, it's used by the "newline" constants
						*p++ = '\x0F';
						str+=2;
						continue;
					} else

					if ( *advstr == ')' ) {
						*p++ = '\x15';
						str+=2;
						continue;
					} else
					if ( *advstr == '(' ) {
						*p++ = '\x16';
						str+=2;
						continue;
					} else
					if ( *advstr == 'P' ) {
						*p++ = '\x17';
						str+=2;
						continue;
					} else
					if ( *advstr == 'D' ) {
						*p++ = '\x18';
						str+=2;
						continue;
					} else
					if ( *advstr == 'I' ) {
						*p++ = '\x09';
						str+=2;
						continue;
					}
				} else
				if ( *str == 'D' ) {
					advstr = str + 1;
					if ( *advstr == ':' || *advstr == '=' ) {
						*p++ = '\x19';
						str+=2;
						continue;
					}
				} else
				if ( *str == 'X' ) {
					advstr = str + 1;
					if ( *advstr == 'D' ) {
						*p++ = '\x1A';
						str+=2;
						continue;
					}
				} else
				if ( *str == '8' ) {
					advstr = str + 1;
					if ( *advstr == '=' ) {
						/*Com_Printf( S_COLOR_SKY "DEBUG: we got dick\n");*/
						shaft = 1;
						shaftEnded = qfalse;
						while (*advstr == '=' && shaftEnded == qfalse) {
							advstr++;
							// how big are you?
							if ( *advstr == 'D' ) {
								/*Com_Printf( S_COLOR_SKY "DEBUG: shaft is good\n");*/
								*p++ = '\x1B';
								while ( shaft ) {
									*p++ = '\x1C';
									shaft--;
									str++;
								}
								*p++ = '\x1D';
								str+=2;
								continue;
							}
							shaft++;
							/*Com_Printf( S_COLOR_SKY "DEBUG: shaft is growing\n");*/
						}
						if ( shaftEnded == qtrue ) {
							continue;
						}
					}
				}
			} else {
				/*Com_Printf( S_COLOR_SKY "DEBUG: %i!='%i'\n", emoticonSkipPos, emoticon);*/
				emoticonSkipPos++;
			}
		}

		/*if ( *str == '\n' ) {
			*p++ = *str++;
			continue;
		}*/
		if ( *str == ' ' ) {
			ls = p;
		}
		*p++ = *str++;
		len++;
	}
	*p = 0; // write end of string

	// add to HUD info, unless it's filtered out
	if ( !hideInfo ) {
		Q_strncpyz(cgs.hudInfoMsgs[hudBox][cgs.hudInfoPos[hudBox] % hudInfoHeight],
						cgs.hudBoardInfoMsgs[cgs.hudBoardInfoPos % HUD_BOARD_TEXT_HEIGHT],
						sizeof(cgs.hudInfoMsgs[hudBox][cgs.hudInfoPos[hudBox] % hudInfoHeight]) );

		// set display time for message, and scroll the text correctly
		cgs.hudInfoMsgTimes[hudBox][cgs.hudInfoPos[hudBox] % hudInfoHeight] = cg.time;
		cgs.hudInfoPos[hudBox]++;
		if (cgs.hudInfoPos[hudBox] - cgs.hudInfoLastPos[hudBox] > hudInfoHeight)
			cgs.hudInfoLastPos[hudBox] = cgs.hudInfoPos[hudBox] - hudInfoHeight;
	}

/*	Q_strncpyz(cgs.hudBoardInfoMsgs[cgs.hudBoardInfoPos], cgs.hudInfoMsgs[hudBox][cgs.hudInfoPos[hudBox] % hudInfoHeight],
					sizeof(cgs.hudBoardInfoMsgs[cgs.hudBoardInfoPos]) );*/

	cgs.hudBoardInfoPos++;
	if (cgs.hudBoardInfoPos >= HUD_BOARD_TEXT_HEIGHT)
		cgs.hudBoardInfoPos = 0;

}

/*
=======================
CG_AddToTeamChat

=======================
*/
static void CG_AddToTeamChat( const char *str ) {
	int len;
	char *p, *ls;
	int lastcolor;
	int chatHeight;

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT) {
		chatHeight = cg_teamChatHeight.integer;
	} else {
		chatHeight = TEAMCHAT_HEIGHT;
	}

	if (chatHeight <= 0 || cg_teamChatTime.integer <= 0) {
		// team chat disabled, dump into normal chat
		cgs.teamChatPos = cgs.teamLastChatPos = 0;
		return;
	}

	len = 0;

	p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while (*str) {
		if (len > TEAMCHAT_WIDTH - 1) {
			if (ls) {
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;

			cgs.teamChatPos++;
			p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if (*str == ' ') {
			ls = p;
		}
		*p++ = *str++;
		len++;
	}
	*p = 0;

	cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
	cgs.teamChatPos++;

	if (cgs.teamChatPos - cgs.teamLastChatPos > chatHeight)
		cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
}

/*
=======================
CG_AddToChat

=======================
*/
static void CG_AddToChat( const char *str ) {
	int len;
	char *p, *ls;
	int lastcolor;
	int chatHeight;

	/*if (cg_chatHeight.integer < CUSTOM_HEIGHT) {
		chatHeight = cg_chatHeight.integer;
	} else {
		chatHeight = CUSTOM_HEIGHT;
	}*/
		chatHeight = CHAT_HEIGHT;

	/*if (chatHeight <= 0 || cg_chatTime.integer <= 0) {
		// custom chat disabled, dump into normal chat
		cgs.chatPos = cgs.lastChatPos = 0;
		return;
	}*/

	len = 0;

	p = cgs.chatMsgs[cgs.chatPos % chatHeight];
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while (*str) {
		if (len > HUD_TEXT_BUFFER_SIZE - 1) {
			if (ls) {
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			cgs.chatMsgTimes[cgs.chatPos % chatHeight] = cg.time;

			cgs.chatPos++;
			p = cgs.chatMsgs[cgs.chatPos % chatHeight];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		/*if ( *str == 'A' ) {
			*p++ = *str++;
			continue;
		}*/
		if ( *str == ' ' ) {
			ls = p;
		}
		*p++ = *str++;
		len++;
	}
	*p = 0;

	cgs.chatMsgTimes[cgs.chatPos % chatHeight] = cg.time;
	cgs.chatPos++;

	if (cgs.chatPos - cgs.lastChatPos > chatHeight)
		cgs.lastChatPos = cgs.chatPos - chatHeight;
}

/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournament restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart( void ) {
	if ( cg_showmiss.integer ) {
		CG_Printf( "CG_MapRestart\n" );
	}

	CG_InitLocalEntities();
	CG_InitMarkPolys();
	CG_ClearParticles ();

	// make sure the "3 frags left" warnings play again
	cg.fraglimitWarnings = 0;

	cg.timelimitWarnings = 0;
	cg.rewardTime = 0;
	cg.rewardStack = 0;
	cg.intermissionStarted = qfalse;
	cg.levelShot = qfalse;

	cgs.voteTime = 0;

	cg.mapRestart = qtrue;

	CG_StartMusic();

	trap_S_ClearLoopingSounds(qtrue);

	// we really should clear more parts of cg here and stop sounds

	// play the "fight" sound if this is a restart without warmup
	if ( cg.warmup == 0 /* && cgs.gametype == GT_TOURNAMENT */) {
		trap_S_StartLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
		CG_CenterPrint( "FIGHT!", 120, GIANTCHAR_WIDTH*2 );
	}
#ifdef MISSIONPACK
	if (cg_singlePlayerActive.integer) {
		trap_Cvar_Set("ui_matchStartTime", va("%i", cg.time));
		if (cg_recordSPDemo.integer && *cg_recordSPDemoName.string) {
			trap_SendConsoleCommand(va("set g_synchronousclients 1 ; record %s \n", cg_recordSPDemoName.string));
		}
	}
#endif
	trap_Cvar_Set("cg_thirdPerson", "0");
}

#ifdef MISSIONPACK

#define MAX_VOICEFILESIZE	16384
#define MAX_VOICEFILES		8
#define MAX_VOICECHATS		64
#define MAX_VOICESOUNDS		64
#define MAX_CHATSIZE		64
#define MAX_HEADMODELS		64

typedef struct voiceChat_s
{
	char id[64];
	int numSounds;
	sfxHandle_t sounds[MAX_VOICESOUNDS];
	char chats[MAX_VOICESOUNDS][MAX_CHATSIZE];
} voiceChat_t;

typedef struct voiceChatList_s
{
	char name[64];
	int gender;
	int numVoiceChats;
	voiceChat_t voiceChats[MAX_VOICECHATS];
} voiceChatList_t;

typedef struct headModelVoiceChat_s
{
	char headmodel[64];
	int voiceChatNum;
} headModelVoiceChat_t;

voiceChatList_t voiceChatLists[MAX_VOICEFILES];
headModelVoiceChat_t headModelVoiceChat[MAX_HEADMODELS];

/*
=================
CG_ParseVoiceChats
=================
*/
int CG_ParseVoiceChats( const char *filename, voiceChatList_t *voiceChatList, int maxVoiceChats ) {
	int	len, i;
	fileHandle_t f;
	char buf[MAX_VOICEFILESIZE];
	char **p, *ptr;
	char *token;
	voiceChat_t *voiceChats;
	qboolean compress;
	sfxHandle_t sound;

	compress = qtrue;
	if (cg_buildScript.integer) {
		compress = qfalse;
	}

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "voice chat file not found: %s\n", filename ) );
		return qfalse;
	}
	if ( len >= MAX_VOICEFILESIZE ) {
		trap_Print( va( S_COLOR_RED "voice chat file too large: %s is %i, max allowed is %i\n", filename, len, MAX_VOICEFILESIZE ) );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	ptr = buf;
	p = &ptr;

	Com_sprintf(voiceChatList->name, sizeof(voiceChatList->name), "%s", filename);
	voiceChats = voiceChatList->voiceChats;
	for ( i = 0; i < maxVoiceChats; i++ ) {
		voiceChats[i].id[0] = 0;
	}
	token = COM_ParseExt(p, qtrue);
	if (!token || token[0] == 0) {
		return qtrue;
	}
	if (!Q_stricmp(token, "female")) {
		voiceChatList->gender = GENDER_FEMALE;
	}
	else if (!Q_stricmp(token, "male")) {
		voiceChatList->gender = GENDER_MALE;
	}
	else if (!Q_stricmp(token, "neuter")) {
		voiceChatList->gender = GENDER_NEUTER;
	}
	else {
		trap_Print( va( S_COLOR_RED "expected gender not found in voice chat file: %s\n", filename ) );
		return qfalse;
	}

	voiceChatList->numVoiceChats = 0;
	while ( 1 ) {
		token = COM_ParseExt(p, qtrue);
		if (!token || token[0] == 0) {
			return qtrue;
		}
		Com_sprintf(voiceChats[voiceChatList->numVoiceChats].id, sizeof( voiceChats[voiceChatList->numVoiceChats].id ), "%s", token);
		token = COM_ParseExt(p, qtrue);
		if (Q_stricmp(token, "{")) {
			trap_Print( va( S_COLOR_RED "expected { found %s in voice chat file: %s\n", token, filename ) );
			return qfalse;
		}
		voiceChats[voiceChatList->numVoiceChats].numSounds = 0;
		while(1) {
			token = COM_ParseExt(p, qtrue);
			if (!token || token[0] == 0) {
				return qtrue;
			}
			if (!Q_stricmp(token, "}"))
				break;
			sound = trap_S_RegisterSound( token, compress );
			voiceChats[voiceChatList->numVoiceChats].sounds[voiceChats[voiceChatList->numVoiceChats].numSounds] = sound;
			token = COM_ParseExt(p, qtrue);
			if (!token || token[0] == 0) {
				return qtrue;
			}
			Com_sprintf(voiceChats[voiceChatList->numVoiceChats].chats[
							voiceChats[voiceChatList->numVoiceChats].numSounds], MAX_CHATSIZE, "%s", token);
			if (sound)
				voiceChats[voiceChatList->numVoiceChats].numSounds++;
			if (voiceChats[voiceChatList->numVoiceChats].numSounds >= MAX_VOICESOUNDS)
				break;
		}
		voiceChatList->numVoiceChats++;
		if (voiceChatList->numVoiceChats >= maxVoiceChats)
			return qtrue;
	}
	return qtrue;
}

/*
=================
CG_LoadVoiceChats
=================
*/
void CG_LoadVoiceChats( void ) {
	int size;

	size = trap_MemoryRemaining();
	CG_ParseVoiceChats( "scripts/female1.voice", &voiceChatLists[0], MAX_VOICECHATS );
	CG_ParseVoiceChats( "scripts/female2.voice", &voiceChatLists[1], MAX_VOICECHATS );
	CG_ParseVoiceChats( "scripts/female3.voice", &voiceChatLists[2], MAX_VOICECHATS );
	CG_ParseVoiceChats( "scripts/male1.voice", &voiceChatLists[3], MAX_VOICECHATS );
	CG_ParseVoiceChats( "scripts/male2.voice", &voiceChatLists[4], MAX_VOICECHATS );
	CG_ParseVoiceChats( "scripts/male3.voice", &voiceChatLists[5], MAX_VOICECHATS );
	CG_ParseVoiceChats( "scripts/male4.voice", &voiceChatLists[6], MAX_VOICECHATS );
	CG_ParseVoiceChats( "scripts/male5.voice", &voiceChatLists[7], MAX_VOICECHATS );
	CG_Printf("voice chat memory size = %d\n", size - trap_MemoryRemaining());
}

/*
=================
CG_HeadModelVoiceChats
=================
*/
int CG_HeadModelVoiceChats( char *filename ) {
	int	len, i;
	fileHandle_t f;
	char buf[MAX_VOICEFILESIZE];
	char **p, *ptr;
	char *token;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		//trap_Print( va( "voice chat file not found: %s\n", filename ) );
		return -1;
	}
	if ( len >= MAX_VOICEFILESIZE ) {
		trap_Print( va( S_COLOR_RED "voice chat file too large: %s is %i, max allowed is %i\n", filename, len, MAX_VOICEFILESIZE ) );
		trap_FS_FCloseFile( f );
		return -1;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	ptr = buf;
	p = &ptr;

	token = COM_ParseExt(p, qtrue);
	if (!token || token[0] == 0) {
		return -1;
	}

	for ( i = 0; i < MAX_VOICEFILES; i++ ) {
		if ( !Q_stricmp(token, voiceChatLists[i].name) ) {
			return i;
		}
	}

	//FIXME: maybe try to load the .voice file which name is stored in token?

	return -1;
}


/*
=================
CG_GetVoiceChat
=================
*/
int CG_GetVoiceChat( voiceChatList_t *voiceChatList, const char *id, sfxHandle_t *snd, char **chat) {
	int i, rnd;

	for ( i = 0; i < voiceChatList->numVoiceChats; i++ ) {
		if ( !Q_stricmp( id, voiceChatList->voiceChats[i].id ) ) {
			rnd = random() * voiceChatList->voiceChats[i].numSounds;
			*snd = voiceChatList->voiceChats[i].sounds[rnd];
			*chat = voiceChatList->voiceChats[i].chats[rnd];
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
CG_VoiceChatListForClient
=================
*/
voiceChatList_t *CG_VoiceChatListForClient( int clientNum ) {
	clientInfo_t *ci;
	int voiceChatNum, i, j, k, gender;
	char filename[MAX_QPATH], headModelName[MAX_QPATH];

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	for ( k = 0; k < 2; k++ ) {
		if ( k == 0 ) {
			if (ci->headModelName[0] == '*') {
				Com_sprintf( headModelName, sizeof(headModelName), "%s/%s", ci->headModelName+1, ci->headSkinName );
			}
			else {
				Com_sprintf( headModelName, sizeof(headModelName), "%s/%s", ci->headModelName, ci->headSkinName );
			}
		}
		else {
			if (ci->headModelName[0] == '*') {
				Com_sprintf( headModelName, sizeof(headModelName), "%s", ci->headModelName+1 );
			}
			else {
				Com_sprintf( headModelName, sizeof(headModelName), "%s", ci->headModelName );
			}
		}
		// find the voice file for the head model the client uses
		for ( i = 0; i < MAX_HEADMODELS; i++ ) {
			if (!Q_stricmp(headModelVoiceChat[i].headmodel, headModelName)) {
				break;
			}
		}
		if (i < MAX_HEADMODELS) {
			return &voiceChatLists[headModelVoiceChat[i].voiceChatNum];
		}
		// find a <headmodelname>.vc file
		for ( i = 0; i < MAX_HEADMODELS; i++ ) {
			if (!strlen(headModelVoiceChat[i].headmodel)) {
				Com_sprintf(filename, sizeof(filename), "scripts/%s.vc", headModelName);
				voiceChatNum = CG_HeadModelVoiceChats(filename);
				if (voiceChatNum == -1)
					break;
				Com_sprintf(headModelVoiceChat[i].headmodel, sizeof ( headModelVoiceChat[i].headmodel ),
							"%s", headModelName);
				headModelVoiceChat[i].voiceChatNum = voiceChatNum;
				return &voiceChatLists[headModelVoiceChat[i].voiceChatNum];
			}
		}
	}
	gender = ci->gender;
	for (k = 0; k < 2; k++) {
		// just pick the first with the right gender
		for ( i = 0; i < MAX_VOICEFILES; i++ ) {
			if (strlen(voiceChatLists[i].name)) {
				if (voiceChatLists[i].gender == gender) {
					// store this head model with voice chat for future reference
					for ( j = 0; j < MAX_HEADMODELS; j++ ) {
						if (!strlen(headModelVoiceChat[j].headmodel)) {
							Com_sprintf(headModelVoiceChat[j].headmodel, sizeof ( headModelVoiceChat[j].headmodel ),
									"%s", headModelName);
							headModelVoiceChat[j].voiceChatNum = i;
							break;
						}
					}
					return &voiceChatLists[i];
				}
			}
		}
		// fall back to male gender because we don't have neuter in the mission pack
		if (gender == GENDER_MALE)
			break;
		gender = GENDER_MALE;
	}
	// store this head model with voice chat for future reference
	for ( j = 0; j < MAX_HEADMODELS; j++ ) {
		if (!strlen(headModelVoiceChat[j].headmodel)) {
			Com_sprintf(headModelVoiceChat[j].headmodel, sizeof ( headModelVoiceChat[j].headmodel ),
					"%s", headModelName);
			headModelVoiceChat[j].voiceChatNum = 0;
			break;
		}
	}
	// just return the first voice chat list
	return &voiceChatLists[0];
}

#define MAX_VOICECHATBUFFER		32

typedef struct bufferedVoiceChat_s
{
	int clientNum;
	sfxHandle_t snd;
	int voiceOnly;
	char cmd[MAX_SAY_TEXT];
	char message[MAX_SAY_TEXT];
} bufferedVoiceChat_t;

bufferedVoiceChat_t voiceChatBuffer[MAX_VOICECHATBUFFER];

/*
=================
CG_PlayVoiceChat
=================
*/
void CG_PlayVoiceChat( bufferedVoiceChat_t *vchat ) {
#ifdef MISSIONPACK
	// if we are going into the intermission, don't start any voices
	if ( cg.intermissionStarted ) {
		return;
	}

	if ( !cg_noVoiceChats.integer ) {
		trap_S_StartLocalSound( vchat->snd, CHAN_VOICE);
		if (vchat->clientNum != cg.snap->ps.clientNum) {
			int orderTask = CG_ValidOrder(vchat->cmd);
			if (orderTask > 0) {
				cgs.acceptOrderTime = cg.time + 5000;
				Q_strncpyz(cgs.acceptVoice, vchat->cmd, sizeof(cgs.acceptVoice));
				cgs.acceptTask = orderTask;
				cgs.acceptLeader = vchat->clientNum;
			}
			// see if this was an order
			CG_ShowResponseHead();
		}
	}
	if (!vchat->voiceOnly && !cg_noVoiceText.integer) {
		CG_AddToTeamChat( vchat->message );
		CG_Printf( "%s\n", vchat->message );
	}
	voiceChatBuffer[cg.voiceChatBufferOut].snd = 0;
#endif
}

/*
=====================
CG_PlayBufferedVoieChats
=====================
*/
void CG_PlayBufferedVoiceChats( void ) {
#ifdef MISSIONPACK
	if ( cg.voiceChatTime < cg.time ) {
		if (cg.voiceChatBufferOut != cg.voiceChatBufferIn && voiceChatBuffer[cg.voiceChatBufferOut].snd) {
			//
			CG_PlayVoiceChat(&voiceChatBuffer[cg.voiceChatBufferOut]);
			//
			cg.voiceChatBufferOut = (cg.voiceChatBufferOut + 1) % MAX_VOICECHATBUFFER;
			cg.voiceChatTime = cg.time + 1000;
		}
	}
#endif
}

/*
=====================
CG_AddBufferedVoiceChat
=====================
*/
void CG_AddBufferedVoiceChat( bufferedVoiceChat_t *vchat ) {
#ifdef MISSIONPACK
	// if we are going into the intermission, don't start any voices
	if ( cg.intermissionStarted ) {
		return;
	}

	memcpy(&voiceChatBuffer[cg.voiceChatBufferIn], vchat, sizeof(bufferedVoiceChat_t));
	cg.voiceChatBufferIn = (cg.voiceChatBufferIn + 1) % MAX_VOICECHATBUFFER;
	if (cg.voiceChatBufferIn == cg.voiceChatBufferOut) {
		CG_PlayVoiceChat( &voiceChatBuffer[cg.voiceChatBufferOut] );
		cg.voiceChatBufferOut++;
	}
#endif
}

/*
=================
CG_VoiceChatLocal
=================
*/
void CG_VoiceChatLocal( int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd ) {
#ifdef MISSIONPACK
	char *chat;
	voiceChatList_t *voiceChatList;
	clientInfo_t *ci;
	sfxHandle_t snd;
	bufferedVoiceChat_t vchat;

	// if we are going into the intermission, don't start any voices
	if ( cg.intermissionStarted ) {
		return;
	}

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	cgs.currentVoiceClient = clientNum;

	voiceChatList = CG_VoiceChatListForClient( clientNum );

	if ( CG_GetVoiceChat( voiceChatList, cmd, &snd, &chat ) ) {
		//
		if ( mode == SAY_TEAM || !cg_teamChatsOnly.integer ) {
			vchat.clientNum = clientNum;
			vchat.snd = snd;
			vchat.voiceOnly = voiceOnly;
			Q_strncpyz(vchat.cmd, cmd, sizeof(vchat.cmd));
			if ( mode == SAY_TELL ) {
				Com_sprintf(vchat.message, sizeof(vchat.message), "[%s]: %c%c%s", ci->name, Q_COLOR_ESCAPE, color, chat);
			}
			else if ( mode == SAY_TEAM ) {
				Com_sprintf(vchat.message, sizeof(vchat.message), "(%s): %c%c%s", ci->name, Q_COLOR_ESCAPE, color, chat);
			}
			else {
				Com_sprintf(vchat.message, sizeof(vchat.message), "%s: %c%c%s", ci->name, Q_COLOR_ESCAPE, color, chat);
			}
			CG_AddBufferedVoiceChat(&vchat);
		}
	}
#endif
}

/*
=================
CG_VoiceChat
=================
*/
void CG_VoiceChat( int mode ) {
	const char *cmd;
	int clientNum, color;
	qboolean voiceOnly;

	voiceOnly = atoi(CG_Argv(1));
	clientNum = atoi(CG_Argv(2));
	color = atoi(CG_Argv(3));
	cmd = CG_Argv(4);

	if (cg_noTaunt.integer != 0) {
		if (!strcmp(cmd, VOICECHAT_KILLINSULT)  || !strcmp(cmd, VOICECHAT_TAUNT) || \
			!strcmp(cmd, VOICECHAT_DEATHINSULT) || !strcmp(cmd, VOICECHAT_KILLGAUNTLET) || \
			!strcmp(cmd, VOICECHAT_PRAISE)) {
			return;
		}
	}

	CG_VoiceChatLocal( mode, voiceOnly, clientNum, color, cmd );
}
#endif

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (text[i] == '\x19')
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

/*
=================
CG_CleanChatText
=================
*/
static void CG_CleanChatText( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (text[i] < ' ' || text[i] > '~' )
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

/*
=================
CG_FilterColorText
=================
*/
static void CG_FilterColorText( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if ( text[i] == '^' && text[i+1] &&
					( ( text[i+1] >= '0' && text[i+1] <= '9' ) ||
					( text[i+1] >= 'a' && text[i+1] <= 'z' ) ||
					( text[i+1] >= 'A' && text[i+1] <= 'Z') ) ) {
			i++;
			continue;
		}
		text[l++] = text[i];
	}
	text[l] = '\0';
}


/*
=================
CG_ParseReadyMask

=================
*/
static void CG_ParseReadyMask ( void ) {
	int	readyMask[4];
	int	i;
	int	quiet;
	readyMask[0] = atoi ( CG_Argv ( 1 ) );
	readyMask[1] = atoi ( CG_Argv ( 2 ) );
	readyMask[2] = atoi ( CG_Argv ( 3 ) );
	readyMask[3] = atoi ( CG_Argv ( 4 ) );
	quiet = atoi ( CG_Argv ( 5 ) );

	if ( cg.warmup >= 0 )
		return;

	if ( cg.time - cgs.levelStartTime >= 1000 && !quiet && !(hud_notifyBoxFilter.integer & NF_READY) ) {
		if ( readyMask[0] != cg.readyMask[0] || readyMask[1] != cg.readyMask[1] || readyMask[2] != cg.readyMask[2] || readyMask[3] != cg.readyMask[3] ) {
			for ( i = 0; i < 64 ; i++ ) {
				//if () {
					if ( ( cg.readyMask[i>>4] & ( 1 << (i & 15) ) ) != ( readyMask[i>>4] & ( 1 << (i & 15) ) ) ) {

						if ( readyMask [i>>4] & ( 1 << (i & 15) ) ) {
							//CG_Printf2 ("%s ^5is ready\n", cgs.clientinfo[ i ].name);
							CG_AddToHUDInfo ( hud_notifyBoxRoute.integer,
									va ("%s " S_COLOR_GREEN "is ready\n", cgs.clientinfo[ i ].name), 0, 0 );
						}
						else
						{
							//CG_Printf2 ("%s ^6is not ready\n", cgs.clientinfo[ i ].name);
							CG_AddToHUDInfo ( hud_notifyBoxRoute.integer,
									va ("%s " S_COLOR_RED "is not ready\n", cgs.clientinfo[ i ].name), 0, 0 );
						}
					}
				//}
			}
		}
	}

	cg.readyMask[0] = readyMask[0];
	cg.readyMask[1] = readyMask[1];
	cg.readyMask[2] = readyMask[2];
	cg.readyMask[3] = readyMask[3];

}

/*
=================
CG_Stats

Display end-game stats, and/or rule set
=================
*/

void CG_PrintBooleanStats( char *string, int result ) {
	if ( result > 0 ) {
		Com_Printf( "  %s: Yes\n", string );
	} else {
		Com_Printf( "  %s: No\n", string );
	}
}

void CG_PrintValueStats( char *string, int result, const char *afterString ) {
	if ( result > 1 ) {
		Com_Printf( "  %s: %i%ss\n", string, result, afterString );
	} else if ( result == 1 ) {
		Com_Printf( "  %s: 1%s\n", string, afterString );
	} else {
		Com_Printf( "  %s: No\n", string );
	}
}

static void CG_Stats ( int endGame ) {

	int			s1, s2, score;
	qboolean	specialRule = qfalse;
	int			i;
	centity_t	*cent;
	const char	*name;
	const char	*s;
	float		kdr; // total kills, divide by total deaths

	int			msec, sec, min;

	if ( endGame ) {

		// stats

		cgs.endGameStats_Active = qfalse;
		CG_AddToHUDInfo ( hud_notifyBoxRoute.integer,
				S_COLOR_GREEN "End-game player stats have been sent to all clients, please observe console if you wish.\n", 0, 0 );

		Com_Printf( "\n======================================\nPLAYER STATS:\n======================================\n" );

		if ( cgs.gametype >= GT_TEAM ) {
			s1 = cgs.scores1;
			s2 = cgs.scores2;

			Com_Printf( "RED: %4i        vs.        BLUE: %4i\n", s1, s2 );
			Com_Printf( "--------------------------------------\n" );
		}

		for ( i=0 ; i < cgs.endGameStats_CurSlot ; i++ ) {
			name = cgs.clientinfo[ cgs.endGameStats[i].clientNo ].name;
			Com_Printf( "NAME: %s\n", name );
			if ( cgs.gametype >= GT_TEAM ) {
				if ( cgs.endGameStats[i].sessionTeam == TEAM_RED ) {
					Com_Printf( "TEAM: RED\n" );
				} else {
					Com_Printf( "TEAM: BLUE\n" );
				}
			}
			Com_Printf( "SCORE: %4i  PING: %3i  TIME: %2im\n", cgs.endGameStats[i].score, cgs.endGameStats[i].ping, cgs.endGameStats[i].playTime );
			if (cgs.endGameStats[i].deaths > 0) {
				kdr = ((float)cgs.endGameStats[i].kills) / ((float)cgs.endGameStats[i].deaths);
			} else {
				kdr = (float)cgs.endGameStats[i].kills;
			}
			Com_Printf( "KILLS: %4i  DEATHS: %4i  KDR: %6.3f\n", cgs.endGameStats[i].kills, cgs.endGameStats[i].deaths, kdr );
			if ( cgs.gametype >= GT_TEAM ) {
				if ( cgs.friendlyFire ) {
					Com_Printf( "SUICIDES: %4i  TEAMKILLS: %4i\n", cgs.endGameStats[i].suicides, cgs.endGameStats[i].teamKills );
				} else {
					Com_Printf( "SUICIDES: %4i\n", cgs.endGameStats[i].suicides );
				}
			if ( cgs.gametype == GT_CTF ) {
					Com_Printf( "CAPTURES: %4i  MAX KILL STREAK: %4i\n", cgs.endGameStats[i].captures, cgs.endGameStats[i].killStreak );
				} else {
					Com_Printf( "MAX KILL STREAK: %4i\n", cgs.endGameStats[i].killStreak );
				}
			} else {
				Com_Printf( "SUICIDES: %4i  MAX KILL STREAK: %4i\n", cgs.endGameStats[i].suicides, cgs.endGameStats[i].killStreak );
			}

			Com_Printf( "--------------------------------------\n" );
		}

	} else {

		CG_AddToHUDInfo ( hud_notifyBoxRoute.integer, "RuleSet information was echoed into the console, per request.\n", 0, 0 );

		Com_Printf( "\n======================================\nCURRENT MATCH OPTIONS AND RULESET:\n======================================\n" );

	}

	// ruleset info

	switch ( cgs.gametype ) {
		case GT_FFA:
			Com_Printf( "GAMETYPE: Free For All (Deathmatch)\n" );
			break;
		case GT_TOURNAMENT:
			Com_Printf( "GAMETYPE: Tournament (Duel)\n" );
			break;
		case GT_SINGLE_PLAYER:
			Com_Printf( "GAMETYPE: Single Player\n" );
			break;
		case GT_TEAM:
			Com_Printf( "GAMETYPE: Team Deathmatch\n" );
			break;
		case GT_CTF:
			Com_Printf( "GAMETYPE: Capture The Flag\n" );
			break;
		case GT_AA1:
			Com_Printf( "GAMETYPE: All Against One\n" );
			break;
		default:
			Com_Printf( "GAMETYPE: Unknown\n" );
			break;
	}

	if ( endGame ) {

		if ( cgs.totalPlayTime < 6000000 ) {
			msec = cgs.totalPlayTime / 100;
			sec = msec / 10;
			min = sec / 60;
			msec -= sec * 10;
			sec -= min * 60;
		} else {
			msec = 9;
			sec = 59;
			min = 99;
		}

		Com_Printf( "PLAY TIME: %02i:%02i.%i\n", min, sec, msec );

	}

	s = CG_ConfigString( CS_MESSAGE );
	if ( s[0] ) {
		Com_Printf( "MAP: %s - %s\n", cgs.mapdispname, s );
	} else {
		Com_Printf( "MAP: %s\n", cgs.mapdispname );
	}
		Com_Printf( "--------------------------------------\n" );

	switch ( cgs.ruleSet ) {
		case 1:
			Com_Printf( "RULE SET: Standard\n\n" );
			break;
		case 2:
			Com_Printf( "RULE SET: Hardcore\n\n" );
			break;
		case 3:
			Com_Printf( "RULE SET: Nightmare!\n\n" );
			break;
		case 4:
			Com_Printf( "RULE SET: Arena\n\n" );
			break;
		case 5:
			Com_Printf( "RULE SET: Rockets\n\n" );
			break;
		default:
			Com_Printf( "RULE SET: Custom\n\n" );
	}

	/*
	CG_PrintValueStats("XXX", cgs.XXX, " XXX");
	CG_PrintBooleanStats("XXX", cgs.XXX);
	*/

	switch ( cgs.matchMode ) {
		case MM_PICKUP_ONCE:
			Com_Printf( "  MATCH MODE: Weapon-stay\n" );
			break;
		case MM_PICKUP_ALWAYS:
			Com_Printf( "  MATCH MODE: Weapon always pickup\n" );
			break;
		case MM_PICKUP_ALWAYS_NOAMMO:
			Com_Printf( "  MATCH MODE: No ammo\n" );
			break;
		case MM_ALLWEAPONS_MAXAMMO:
			Com_Printf( "  MATCH MODE: Most weapons, max ammo\n" );
			break;
		case MM_ALLWEAPONS:
			Com_Printf( "  MATCH MODE: All weapons\n" );
			break;
		case MM_ROCKET_MANIAX:
			Com_Printf( "  MATCH MODE: Rocket-maniax\n" );
			break;
		default:
			Com_Printf( "  MATCH MODE: WTF???\n" );
	}

	CG_PrintValueStats("TIME LIMIT", cgs.fullTimelimit, " minute");
	if ( cgs.timelimit > 0 ) {
		if ( cgs.overtime > 1 ) {
			Com_Printf( "  OVERTIME: Extension of %i minutes\n", cgs.overtime );
		} else if ( cgs.overtime > 0 ) {
			Com_Printf( "  OVERTIME: Extension of 1 minute\n" );
		} else {
			Com_Printf( "  OVERTIME: Sudden death\n" );
		}
	}

	if ( cgs.roundBasedMatches ) {
			Com_Printf( "  ROUND FORMAT: Double rounds\n" );
	} else {
			Com_Printf( "  ROUND FORMAT: Single round\n" );
	}

	CG_PrintValueStats("SCORE LIMIT", cgs.scorelimit, " point");
	if ( cgs.teamSize > 0) {
		CG_PrintValueStats("TEAM SIZE SET", cgs.teamSize, " player");
	}

	if ( cgs.weaponRespawn > 0 ) {
		CG_PrintValueStats("WEAPON RESPAWN", cgs.weaponRespawn, " second");
	} else {
		Com_Printf( "  WEAPON RESPAWN: No (stay)\n" );
	}
	if ( cgs.keycardRespawn > 0 ) {
		CG_PrintValueStats("KEYCARD RESPAWN", cgs.keycardRespawn, " second");
	} else {
		Com_Printf( "  KEYCARD RESPAWN: No (stay)\n" );
	}
	CG_PrintBooleanStats("KEYCARD DROPABLE", cgs.keycardDropable);

	CG_PrintValueStats("FORCED PLAYER RESPAWN", cgs.forceRespawn, " second");
	CG_PrintBooleanStats("SELF DAMAGE", cgs.selfDamage);

	if ( cgs.gametype >= GT_TEAM ) {
		CG_PrintBooleanStats("FRIENDLY FIRE", cgs.friendlyFire);
		CG_PrintBooleanStats("TEAM'S LOC. ON MINIMAP", cgs.teamLocOverlay);
		if (cgs.gametype == GT_TEAM) {
			CG_PrintBooleanStats("TEAM SCORE BALANCE", cgs.scoreBalance);
		}
	}

	CG_PrintBooleanStats("HIT SOUND", cgs.hitSound);
	CG_PrintBooleanStats("TOTALLY RANDOM SPAWNS", cgs.randomSpawn);

	// special rules
	Com_Printf( "\nSPECIAL RULES:\n\n" );
	if ( cgs.quadMode ) {
		Com_Printf( "  QUAD MODE\n" );
		specialRule = qtrue;
	}
	if ( cgs.doubleAmmo ) {
		Com_Printf( "  DOUBLE AMMO\n" );
		specialRule = qtrue;
	}
	if ( cgs.physicsMode ) {
		Com_Printf( "  FREE-SELECT PRO-MODE PHYSICS\n" );
		specialRule = qtrue;
	}
	if ( specialRule == qfalse ) {
		Com_Printf( "  No special rules\n" );
	}

	Com_Printf( "======================================\n\n" );

}

/*
=================
CG_SendStats

=================
*/
static void CG_SendStats ( void ) {

	if ( cgs.endGameStats_Active == qfalse ) {
		cgs.endGameStats_CurSlot = 0;
		cgs.endGameStats_Active = qtrue;
	}

	cgs.endGameStats[cgs.endGameStats_CurSlot].clientNo = atoi ( CG_Argv ( 1 ) );

	cgs.endGameStats[cgs.endGameStats_CurSlot].sessionTeam = atoi ( CG_Argv ( 2 ) );
	cgs.endGameStats[cgs.endGameStats_CurSlot].score = atoi ( CG_Argv ( 3 ) );
	cgs.endGameStats[cgs.endGameStats_CurSlot].ping = atoi ( CG_Argv ( 4 ) );
	cgs.endGameStats[cgs.endGameStats_CurSlot].playTime = atoi ( CG_Argv ( 5 ) );
	cgs.endGameStats[cgs.endGameStats_CurSlot].kills = atoi ( CG_Argv ( 6 ) );

	cgs.endGameStats[cgs.endGameStats_CurSlot].deaths = atoi ( CG_Argv ( 7 ) );
	cgs.endGameStats[cgs.endGameStats_CurSlot].suicides = atoi ( CG_Argv ( 8 ) );
	cgs.endGameStats[cgs.endGameStats_CurSlot].teamKills = atoi ( CG_Argv ( 9 ) );
	cgs.endGameStats[cgs.endGameStats_CurSlot].captures = atoi ( CG_Argv ( 10 ) );
	cgs.endGameStats[cgs.endGameStats_CurSlot].killStreak = atoi ( CG_Argv ( 11 ) );

	cgs.endGameStats_CurSlot++;

}


/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void ) {
	const char	*cmd;
	char		text[MAX_SAY_TEXT];
	int			filter;

	cmd = CG_Argv(0);

	if ( !cmd[0] ) {
		// server claimed the command
		return;
	}

	if ( !strcmp( cmd, "cp" ) ) {
		CG_CenterPrint( CG_Argv(1), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CG_ConfigStringModified();
		return;
	}

	if ( !strcmp( cmd, "print" ) ) {
		CG_Printf( "%s", CG_Argv(1) );
		return;
	}

	if ( !strcmp( cmd, "notify" ) ) {
		CG_Printf( "%s", CG_Argv(2) );

		//Com_Printf( S_COLOR_PINK "DEBUG: '%s'\n", CG_Argv(1));
		filter = atoi( CG_Argv(1) ) & hud_notifyBoxFilter.integer;
		// the notification channel can be filtered for the hud via hud_notifyBoxFilter
		if ( hud_chatBoxRoute.integer >= 0 && hud_chatBoxRoute.integer < TOTAL_HUD_TEXT_BOXES ) {
			CG_AddToHUDInfo ( hud_notifyBoxRoute.integer, CG_Argv(2), 0, filter );
		}
		return;
	}

	if ( !strcmp( cmd, "chat" ) ) {
		if (cg_chatSound.integer > 0 && cg_chatSound.integer <= 9 && !cg_teamChatsOnly.integer) {
			trap_S_StartLocalSound( cgs.media.chatSound[cg_chatSound.integer - 1], CHAN_LOCAL_SOUND );
		}
		Q_strncpyz( text, CG_Argv(2), MAX_SAY_TEXT );
//		CG_RemoveChatEscapeChar( text );
		CG_CleanChatText( text );
		if ( hud_filterColors.integer & FC_CHAT_TEXT ) {
			CG_FilterColorText ( text );
		}
		if ( hud_chatBoxRoute.integer >= 0 && hud_chatBoxRoute.integer < TOTAL_HUD_TEXT_BOXES ) {
			CG_AddToHUDInfo ( hud_chatBoxRoute.integer, text, atoi( CG_Argv(1) ), cg_teamChatsOnly.integer );
		}
		CG_Printf( "%s\n", text );
		return;
	}

	if ( !strcmp( cmd, "tchat" ) ) {
		if (cg_chatSound.integer > 0 && cg_chatSound.integer <= 9)
			trap_S_StartLocalSound( cgs.media.chatSound[cg_teamChatSound.integer - 1], CHAN_LOCAL_SOUND );
		Q_strncpyz( text, CG_Argv(2), MAX_SAY_TEXT );
//		CG_RemoveChatEscapeChar( text );
		CG_CleanChatText( text );
		if ( hud_filterColors.integer & FC_CHAT_TEXT ) {
			CG_FilterColorText ( text );
		}
		//CG_AddToTeamChat( text );
		if ( hud_teamChatBoxRoute.integer >= 0 && hud_teamChatBoxRoute.integer < TOTAL_HUD_TEXT_BOXES ) {
			CG_AddToHUDInfo ( hud_teamChatBoxRoute.integer, text, atoi( CG_Argv(1) ), 0 );
		}
		CG_Printf( "%s\n", text );
		return;
	}

#ifdef MISSIONPACK
	if ( !strcmp( cmd, "vchat" ) ) {
		CG_VoiceChat( SAY_ALL );
		return;
	}

	if ( !strcmp( cmd, "vtchat" ) ) {
		CG_VoiceChat( SAY_TEAM );
		return;
	}

	if ( !strcmp( cmd, "vtell" ) ) {
		CG_VoiceChat( SAY_TELL );
		return;
	}
#endif

	if ( !strcmp( cmd, "scores" ) ) {
		CG_ParseScores();
		return;
	}

	if ( !strcmp( cmd, "tinfo" ) ) {
		CG_ParseTeamInfo();
		return;
	}

	if ( !strcmp( cmd, "map_restart" ) ) {
		CG_MapRestart();
		return;
	}

	// global sound feedback
	if ( !strcmp ( cmd, "sndCall" ) ) {
		CG_ParseSndCall( atoi ( CG_Argv ( 1 ) ) );
		return;
	}

	// ruleset update
	if ( !strcmp ( cmd, "ruleSet" ) ) {
		CG_ParseRuleSet ();
		/*CG_ParseRuleSet( atoi ( CG_Argv ( 1 ) ), atoi ( CG_Argv ( 2 ) ), atoi ( CG_Argv ( 3 ) ),
								atoi ( CG_Argv ( 4 ) ), atoi ( CG_Argv ( 5 ) ) );*/
		return;
	}

	// play end match bgm
	if ( !strcmp ( cmd, "bgm_endMatch" ) ) {
		CG_EndMatchMusic();
		return;
	}

	// total length of time match went for
	if ( !strcmp ( cmd, "totalPlayTime" ) ) {
		cgs.totalPlayTime = atoi ( CG_Argv ( 1 ) );
		return;
	}

	// Send stats receiver buffer
	if ( !strcmp ( cmd, "sendStats" ) ) {
		/*Com_Printf( "DEBUG: %i\n", atoi ( CG_Argv ( 2 ) ) );*/
		CG_SendStats();
		return;
	}

	// End stats receiver buffer
	if ( !strcmp ( cmd, "endStats" ) ) {
		CG_Stats(1);
		return;
	}

	// print ruleset in console
	if ( !strcmp ( cmd, "printRuleSet" ) ) {
		CG_Stats(0);
		return;
	}

	// server update on who's ready during warmup
	if ( !strcmp ( cmd, "readyMask" ) ) {
		CG_ParseReadyMask();
		return;
	}

	if ( Q_stricmp (cmd, "remapShader") == 0 )
	{
		if (trap_Argc() == 4)
		{
			char shader1[MAX_QPATH];
			char shader2[MAX_QPATH];
			char shader3[MAX_QPATH];

			Q_strncpyz(shader1, CG_Argv(1), sizeof(shader1));
			Q_strncpyz(shader2, CG_Argv(2), sizeof(shader2));
			Q_strncpyz(shader3, CG_Argv(3), sizeof(shader3));

			trap_R_RemapShader(shader1, shader2, shader3);
		}

		return;
	}

	// loaddeferred can be both a servercmd and a consolecmd
	if ( !strcmp( cmd, "loaddefered" ) ) {	// FIXME: spelled wrong, but not changing for demo
		CG_LoadDeferredPlayers();
		return;
	}

	// clientLevelShot is sent before taking a special screenshot for
	// the menu system during development
	if ( !strcmp( cmd, "clientLevelShot" ) ) {
		cg.levelShot = qtrue;
		return;
	}

	CG_Printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( trap_GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
