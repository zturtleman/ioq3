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

#ifdef MISSIONPACK
#include "../../ui/menudef.h"			// for the voice chats
#endif

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		if( cl->accuracy_shots ) {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

#ifdef MISSIONPACK
		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy,
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
			cl->ps.persistant[PERS_DEFEND_COUNT],
			cl->ps.persistant[PERS_ASSIST_COUNT],
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
#else
		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy,
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
			cl->ps.persistant[PERS_DEFEND_COUNT],
			cl->ps.persistant[PERS_ASSIST_COUNT],
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
#endif
		j = strlen(entry);
		if (stringlength + j >= sizeof(string))
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i,
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}

/*
==================
SendReadymask

Updates information on who is ready during warmup
note that quiet is an integer and not qboolean, since it's broadcasted through the server channel
==================
*/

void SendReadymask( int clientnum, int quiet ) {
	int			ready, notReady, playerCount;
	int			i;
	gclient_t	*cl;
	gentity_t 	*ent;//mmp - not sure if this is needed anymore
	int		readyMask[4];
	char		entry[32];

	if ( !level.warmupTime ) {
		return;
	}

	// clear ready masks
	readyMask[0] = 0;
	readyMask[1] = 0;
	readyMask[2] = 0;
	readyMask[3] = 0;

	// see which players are ready
	for ( i = 0; i < g_maxclients.integer; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		if ( cl->pers.ready || ( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) ) {
			// if there are more than 64 players, then fuck it
			if ( i < 64 )
				readyMask [i>>4] |= 1 << (i & 15);
		}
	}

	// mmp - note, in assembly, short redundant code can be faster than using loops, idk about c++
	level.readyMask[0] = readyMask[0];
	level.readyMask[1] = readyMask[1];
	level.readyMask[2] = readyMask[2];
	level.readyMask[3] = readyMask[3];
	Com_sprintf (entry, sizeof(entry), " %i %i %i %i %i ", readyMask[0], readyMask[1], readyMask[2], readyMask[3], quiet);

	trap_SendServerCommand( clientnum, va("readyMask%s", entry ));
}

/*
=================
Cmd_Ready_f
=================
*/
void Cmd_Ready_f( gentity_t *ent ) {

	if ( level.warmupTime != -1 ) {
		//trap_SendServerCommand( ent-g_entities, "print \"Cannot use this command at this time.\n\"");
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command at this time.\n\"", NF_ERROR) );
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR){
		//trap_SendServerCommand( ent-g_entities, "print \"Cannot use this command as a spectator.\n\"");
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command as a spectator.\n\"", NF_ERROR) );
		return;
	}

	/*if ( g_readyStartRate.integer <= 0 ) {
		//trap_SendServerCommand( ent-g_entities, "print \"This server doesn't require players to be ready.\n\"");
		trap_SendServerCommand( -1, va("notify %i\\\"This server doesn't require players to be ready.\n\"", NF_ERROR) );
		return;
	}*/

	if ( ent->client->pers.ready )
		ent->client->pers.ready = qfalse;
	else
		ent->client->pers.ready = qtrue;

	SendReadymask( -1, 0 );
}


/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f( gentity_t *ent ) {

	int			i = 0;
	char		buffer[1024];
	gclient_t	*cl;

	buffer[0] = '\0';

	strcat(buffer, va("PLAYERS\n-------\n" ) );

	while ( i < MAX_CLIENTS ) {
		cl = (g_entities+i)->client;
		if ((g_entities+i)->client->pers.connected == CON_CONNECTED) {
			strcat(buffer, va("%3i. %s%s", (g_entities+i)->s.clientNum, ShortTeamName(cl->sess.sessionTeam), cl->pers.netname ) );

			if (cl->sess.admin == qtrue) {
				strcat(buffer, S_COLOR_AMBER " [ADMIN]");
			}
			if (cl->sess.mute == qtrue) {
				strcat(buffer, S_COLOR_GRAY " [MUTED]");
			}
			if (cl->sess.suspended == qtrue) {
				strcat(buffer, S_COLOR_RED " [SUSPENDED]" S_COLOR_WHITE "\n");
			} else {
				strcat(buffer, S_COLOR_WHITE "\n");
			}
		}

		i++;
	}
	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", buffer));

}

/*
=================
Cmd_Practice_f
=================
*/
void Cmd_Practice_f( gentity_t *ent ) {

	if ( level.warmupTime != -1 ) {
		//trap_SendServerCommand( ent-g_entities, "print \"Cannot use this command at this time.\n\"");
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command at this time.\n\"", NF_ERROR) );
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR){
		//trap_SendServerCommand( ent-g_entities, "print \"Cannot use this command as a spectator.\n\"");
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command as a spectator.\n\"", NF_ERROR) );
		return;
	}


	if ( ent->client->pers.practice ) {
		ent->client->pers.practice = qfalse;
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Practice mode off.\n\"", NF_GAMEINFO) );
	} else {
		ent->client->pers.practice = qtrue;
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Practice mode on.\n\"", NF_GAMEINFO) );
	}
}


/*
==================
Cmd_AllowedAdminCmds_f
==================
*/

int Cmd_AllowedAdminCmds_f ( gentity_t *ent, qboolean printNames ) {

	char	adminNames[MAX_CVAR_VALUE_STRING];
	int		adminFlags = 0;
	qboolean	commaUse = qfalse;

	trap_Cvar_VariableStringBuffer( "g_allowedAdminCmds", adminNames, sizeof( adminNames ) );

	// TODO: allow '*' to enable all admins
	// if a star wildcard is present, everything is allowed
	/*if(!Q_stricmp(adminNames, "*" ))
		return 65535;*/

	if ( printNames == qtrue ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Valid admin commands are:\n-------------------------\n\"" ) );
		/*Q_strncpyz( string, "Valid commands are: ", sizeof(string));*/
	}

	if(Q_stristr(adminNames, "/restart/" ) != NULL) {
		adminFlags |= ADMIN_MAP_RESTART;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"restart\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/map/" ) != NULL) {
		adminFlags |= ADMIN_MAP;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"map <mapname>\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/msg/" ) != NULL) {
		adminFlags |= ADMIN_MSG;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"msg\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/players/" ) != NULL) {
		adminFlags |= ADMIN_PLAYERS;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"players\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/gametype/" ) != NULL) {
		adminFlags |= ADMIN_GAMETYPE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"gametype <n>\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/matchMode/" ) != NULL && level.rulesetEnforced == qfalse) {
		adminFlags |= ADMIN_MATCHMODE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"matchMode <n>\n\"" ) );
		}
	}
	if(Q_stristr(adminNames, "/proMode/" ) != NULL) {
		adminFlags |= ADMIN_PROMODE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"proMode <0/1>\n\"" ) );
		}
	}
	if(Q_stristr(adminNames, "/kick/" ) != NULL) {
		adminFlags |= ADMIN_KICK;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"kick <player>\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/mute/" ) != NULL) {
		adminFlags |= ADMIN_MUTE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"mute <player>\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/unmute/" ) != NULL) {
		adminFlags |= ADMIN_UNMUTE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"unmute <player>\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/rpickup/" ) != NULL && g_gametype.integer >= GT_TEAM) {
		adminFlags |= ADMIN_RPICKUP;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"rpickup\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/timelimit/" ) != NULL && level.rulesetEnforced == qfalse) {
		adminFlags |= ADMIN_TIMELIMIT;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"timelimit <minute>\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/scorelimit/" ) != NULL && level.rulesetEnforced == qfalse) {
		adminFlags |= ADMIN_SCORELIMIT;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"scorelimit <score>\n\"" ) );

		}
	}
	if(Q_stristr(adminNames, "/ruleSet/" ) != NULL) {
		adminFlags |= ADMIN_RULESET;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"ruleset <n>\n\"" ) );
		}
	}
	if(Q_stristr(adminNames, "/teamSize/" ) != NULL && g_gametype.integer >= GT_TEAM) {
		adminFlags |= ADMIN_TEAMSIZE;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"teamsize <n>\n\"" ) );
		}
	}
	if(Q_stristr(adminNames, "/shortGame/" ) != NULL) {
		adminFlags |= ADMIN_SHORTGAME;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"shortGame <0/1>\n\"" ) );
		}
	}
	if(Q_stristr(adminNames, "/vstr/" ) != NULL) {
		adminFlags |= ADMIN_VSTR;
		if ( printNames == qtrue ) {
			trap_SendServerCommand( ent-g_entities, va("print \"vstr <cvar>\n\"" ) );
		}
	}

	if ( printNames == qtrue ) {
		if (adminFlags == 0)
			trap_SendServerCommand( ent-g_entities, va("print \"Oddly, no admin commands were made available.\n\"" ) );
	}

	return adminFlags;

}


/*
=================
Cmd_Admin_f

Commands only available to admins
TODO: code some of these better
=================
*/

void Cmd_Admin_Mute_f( gentity_t *ent ) {

	gclient_t	*cl;
	int			clnum;

	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: admin mute <player>\n\"" );
		return;
	}

	// find the player
	trap_Argv( 2, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}
	clnum = ClientNumForString( str ); // TODO: optimize this

	if ( cl->sess.mute != qtrue ) {

		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_RED " has been muted by admin " S_COLOR_WHITE "%s" S_COLOR_RED ".\n\"",
			NF_GAMEINFO, cl->pers.netname, ent->client->pers.netname) );

		// set mute
		cl->sess.mute = qtrue;

		G_LogPrintf( "Admin Mute: %s (No. %i) was muted by admin %s (No. %i)\n",
				cl->pers.netname, clnum, ent->client->pers.netname, ent-g_entities );
	}

	// update number of voting clients
	CalculateRanks();

}

/*
-----------------
*/

void Cmd_Admin_Unmute_f( gentity_t *ent ) {

	gclient_t	*cl;
	int			clnum;

	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: admin unmute <player>\n\"" );
		return;
	}

	// find the player
	trap_Argv( 2, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}
	clnum = ClientNumForString( str ); // TODO: optimize this

	if ( cl->sess.mute != qfalse ) {

		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_GREEN " is no longer muted, thanks to admin " S_COLOR_WHITE "%s" S_COLOR_GREEN ".\n\"",
			NF_GAMEINFO, cl->pers.netname, ent->client->pers.netname) );

		// set mute
		cl->sess.mute = qfalse;

		G_LogPrintf( "Admin Unmute: %s (No. %i) was unmuted by admin %s (No. %i)\n",
				cl->pers.netname, clnum, ent->client->pers.netname, ent-g_entities );
	}

	// update number of voting clients
	CalculateRanks();

}

/*
-----------------
*/

void Cmd_Admin_VStr_f( gentity_t *ent ) {

	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];
	char		keyVal[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: admin vstr <key>\n\"" );
		return;
	}

	// get cvar key
	trap_Argv( 2, str, sizeof( str ) );

	if ( !Info_Validate( str ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Illegal character(s) in string.\n\"" );
		return;
	}

	// check if cvar key is used
	trap_Cvar_VariableStringBuffer( va( "admin_%s", str ), keyVal, sizeof(keyVal) );
	if ( !strlen(keyVal) ) {
		trap_SendServerCommand( ent-g_entities, va( "print \"Key '%s' is not used.\n\"", str ) );
		return;
	}

	G_LogPrintf( "Admin VStr: Key \"admin_%s\" was executed by admin %s (No. %i)\n", str, ent->client->pers.netname, ent-g_entities );

	// exec it
	trap_SendConsoleCommand( EXEC_APPEND, va( "vstr admin_%s\n", str ) );

}

/*
-----------------
*/

void Cmd_Admin_Map_f( gentity_t *ent ) {

	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];
	char		s[MAX_STRING_CHARS];

	if ( trap_Argc() < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: admin map <mapname>\n\"" );
		return;
	}

	// get cvar key
	trap_Argv( 2, str, sizeof( str ) );

	if ( !Info_Validate( str ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Illegal character(s) in string.\n\"" );
		return;
	}

	G_LogPrintf( "Admin Map: Changed to map '%s' by admin %s (No. %i)\n",
			str ,ent->client->pers.netname, ent-g_entities );

	trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
	if (*s) {
		trap_SendConsoleCommand( EXEC_APPEND, va( "map %s; set nextmap \"%s\"\n", str, s ) );
	} else {
		trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", str ) );
	}

}

/*
-----------------
*/

void Cmd_Admin_Players_f( gentity_t *ent ) {


	int			i = 0;
	char		buffer[1024];
	gclient_t	*cl;

	buffer[0] = '\0';

	strcat(buffer, va("PLAYERS\n-------\n" ) );

	while ( i < MAX_CLIENTS ) {
		cl = (g_entities+i)->client;
		if ((g_entities+i)->client->pers.connected == CON_CONNECTED) {
			strcat(buffer, va("%3i. %s%s", (g_entities+i)->s.clientNum, ShortTeamName(cl->sess.sessionTeam), cl->pers.netname ) );

			if (cl->sess.admin == qtrue) {
				strcat(buffer, S_COLOR_AMBER " [A]");
			}
			if (cl->sess.mute == qtrue) {
				strcat(buffer, S_COLOR_GRAY " [M]");
			}
			if (cl->sess.suspended == qtrue) {
				strcat(buffer, S_COLOR_RED " [S]");
			}

			strcat(buffer, va( S_COLOR_WHITE " (%s)\n", cl->pers.ip ));
		}

		i++;
	}
	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", buffer));

}

/*
-----------------
*/

void Cmd_Admin_Msg_f( gentity_t *ent ) {


	int			i = 0;
	char		buffer[1024];
	fileHandle_t	fHandle;

	int fileLen;

	char * p;

	buffer[0] = '\0'; // this most likely has no use here

	fileLen = trap_FS_FOpenFile("adminmsg.cfg", &fHandle, FS_READ);
	if( !fHandle ) {
		trap_SendServerCommand( ent-g_entities, "print \"No admin message bulletin.\n\"" );
		return;
	}

	G_LogPrintf( "Admin Msg Read: Admin %s (No. %i) read your bulletin\n",
			ent->client->pers.netname, ent-g_entities );

	if ( (fileLen) > (sizeof(buffer) - 10) ) {
		fileLen = (sizeof(buffer) - 10);
	}
	buffer[fileLen] = '\0';

	trap_FS_Read(buffer, fileLen, fHandle);
	trap_FS_FCloseFile(fHandle);

	//strip carrier returns (0x0D)
	while( ( p = strchr( buffer, '\r' ) ) ) {
		memmove(p, p + 1, fileLen - (p - buffer));
	}

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", buffer));

}

/*
-----------------
*/

void Cmd_Admin_RPickup_f( gentity_t *ent ) {

	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent-g_entities, "print \"Cannot call random pickup in non-team gametypes.\n\"" );
		return;
	}

	if ( !level.warmupTime ) {
		trap_SendServerCommand( ent-g_entities, "print \"Cannot call random pickup during a match.\n\"" );
		return;
	}

	G_LogPrintf( "Admin RPickup: Admin %s (No. %i) enforced a random pickup\n",
			ent->client->pers.netname, ent-g_entities );

	trap_SendConsoleCommand( EXEC_APPEND, "rpickup\n");

}

/*
-----------------
*/

void Cmd_Admin_Kick_f( gentity_t *ent ) {

	gclient_t	*cl;
	int			clnum;

	char		str[MAX_TOKEN_CHARS];
	char		buffer[1024];

	if ( trap_Argc() < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: admin kick <player>\n\"" );
		return;
	}

	// find the player
	trap_Argv( 2, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}
	clnum = ClientNumForString( str ); // TODO: optimize this

	buffer[0] = '\0';
	strcat(buffer, va("was kicked by admin %s", ent->client->pers.netname) );
	trap_DropClient ( clnum, buffer );

	// make a server log about admin kick, incase abuse happens
	G_LogPrintf( "Admin Kick: %s (No. %i) was kicked by admin %s (No. %i)\n",
			cl->pers.netname, clnum, ent->client->pers.netname, ent-g_entities );

	// update number of voting clients
	CalculateRanks();

}

/*
-----------------
*/

void Cmd_Admin_Gametype_f( gentity_t *ent, int i ) {

	if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
		return;
	}

	G_LogPrintf( "Admin GameType: Admin %s (No. %i) changed gametype to %i\n",
			ent->client->pers.netname, ent-g_entities, i );

	trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype %i; map_restart\n", i) );

}

/*
-----------------
*/

void Cmd_Admin_MatchMode_f( gentity_t *ent, int i ) {

	if ( i < 0 || i >= MM_NUM_MMODES ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid matchmode.\n\"" );
		return;
	}

	G_LogPrintf( "Admin MatchMode: Admin %s (No. %i) changed matchmode to %i\n",
			ent->client->pers.netname, ent-g_entities, i );

	trap_SendConsoleCommand( EXEC_APPEND, va("g_matchmode %i; map_restart\n", i) );

}

/*
-----------------
*/

void Cmd_Admin_ProMode_f( gentity_t *ent, int i ) {

	if ( i < 0 )
		i = 0;
	else if ( i > 1 )
		i = 1;

	G_LogPrintf( "Admin ProMode: Admin %s (No. %i) changed proMode to %i\n",
			ent->client->pers.netname, ent-g_entities, i );

	trap_SendConsoleCommand( EXEC_APPEND, va("g_proMode %i; map_restart\n", i) );

}

/*
-----------------
*/

void Cmd_Admin_Timelimit_f( gentity_t *ent, int i ) {

	if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
		return;
	}

	G_LogPrintf( "Admin TimeLimit: Admin %s (No. %i) changed timelimit to %i\n",
			ent->client->pers.netname, ent-g_entities, i );

	if ( level.warmupTime )
		trap_SendConsoleCommand( EXEC_APPEND, va("timelimit %i\n", i) );
	else
		trap_SendConsoleCommand( EXEC_APPEND, va("timelimit %i; map_restart\n", i) );

}

/*
-----------------
*/

void Cmd_Admin_Scorelimit_f( gentity_t *ent, int i ) {

	if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
		return;
	}

	G_LogPrintf( "Admin ScoreLimit: Admin %s (No. %i) changed scorelimit to %i\n",
			ent->client->pers.netname, ent-g_entities, i );

	if ( level.warmupTime )
		trap_SendConsoleCommand( EXEC_APPEND, va("scorelimit %i\n", i) );
	else
		trap_SendConsoleCommand( EXEC_APPEND, va("scorelimit %i; map_restart\n", i) );

}

/*
-----------------
*/

void Cmd_Admin_RuleSet_f( gentity_t *ent, int i ) {

	if( i < 0 || i > 4 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid ruleSet.\n\"" );
		return;
	}

	G_LogPrintf( "Admin RuleSet: Admin %s (No. %i) changed ruleSet to %i\n",
			ent->client->pers.netname, ent-g_entities, i );

	trap_SendConsoleCommand( EXEC_APPEND, va("g_ruleSet %i; map_restart\n", i) );

}

/*
-----------------
*/

void Cmd_Admin_TeamSize_f( gentity_t *ent, int i ) {

	if( i < 0 || i > 16 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid teamsize.\n\"" );
		return;
	}

	G_LogPrintf( "Admin TeamSize: Admin %s (No. %i) changed teamSize to %i\n",
			ent->client->pers.netname, ent-g_entities, i );

	if ( level.warmupTime )
		trap_SendConsoleCommand( EXEC_APPEND, va("g_teamSize %i\n", i) );
	else
		trap_SendConsoleCommand( EXEC_APPEND, va("g_teamSize %i; map_restart\n", i) );

}

/*
-----------------
*/

void Cmd_Admin_ShortGame_f( gentity_t *ent, int i ) {

	if ( i < 0 )
		i = 0;
	else if ( i > 1 )
		i = 1;

	G_LogPrintf( "Admin ShortGame: Admin %s (No. %i) changed shortGame to %i\n",
			ent->client->pers.netname, ent-g_entities, i );

	if ( level.warmupTime )
		trap_SendConsoleCommand( EXEC_APPEND, va("g_shortGame %i\n", i) );
	else
		trap_SendConsoleCommand( EXEC_APPEND, va("g_shortGame %i; map_restart\n", i) );

}

/*
-----------------
*/

void Cmd_Admin_Restart_f( gentity_t *ent ) {

	G_LogPrintf( "Admin Restart: Admin %s (No. %i) restarted the map\n",
			ent->client->pers.netname, ent-g_entities );

	trap_SendConsoleCommand( EXEC_APPEND, "map_restart\n");

}

/*
-----------------
Cmd_Admin_f
-----------------
*/

void Cmd_Admin_f( gentity_t *ent ) {
	char		cmd[MAX_TOKEN_CHARS];

	char		arg[MAX_STRING_TOKENS];

	int			allowedCmds;

	if ( ent->client->sess.admin == qfalse ) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You do not have administrator rights.\n\"", NF_ERROR) );
		return;
	}

	// if no command is given, display a list of commands
	if ( trap_Argc() < 2 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: admin <cmd> (arg)\n\n\"" );
	} else {

		allowedCmds = Cmd_AllowedAdminCmds_f( ent, qfalse );

		trap_Argv( 1, cmd, sizeof( cmd ) );
		trap_Argv( 2, arg, sizeof( arg ) );
		//atoi( arg )

		if (Q_stricmp (cmd, "mute") == 0 && (allowedCmds & ADMIN_MUTE) ) {
			Cmd_Admin_Mute_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "unmute") == 0 && (allowedCmds & ADMIN_UNMUTE)) {
			Cmd_Admin_Unmute_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "kick") == 0 && (allowedCmds & ADMIN_KICK)) {
			Cmd_Admin_Kick_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "vstr") == 0 && (allowedCmds & ADMIN_VSTR)) {
			Cmd_Admin_VStr_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "players") == 0 && (allowedCmds & ADMIN_PLAYERS)) {
			Cmd_Admin_Players_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "map") == 0 && (allowedCmds & ADMIN_MAP)) {
			Cmd_Admin_Map_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "msg") == 0 && (allowedCmds & ADMIN_MSG)) {
			Cmd_Admin_Msg_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "rpickup") == 0 && (allowedCmds & ADMIN_RPICKUP)) {
			Cmd_Admin_RPickup_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "restart") == 0 && (allowedCmds & ADMIN_MAP_RESTART) ) {
			Cmd_Admin_Restart_f( ent );
			return;
		}
		if (Q_stricmp (cmd, "gametype") == 0 && (allowedCmds & ADMIN_GAMETYPE) ) {
			Cmd_Admin_Gametype_f( ent, atoi( arg ) );
			return;
		}
		if (Q_stricmp (cmd, "matchmode") == 0 && (allowedCmds & ADMIN_MATCHMODE) ) {
			Cmd_Admin_MatchMode_f( ent, atoi( arg ) );
			return;
		}
		if (Q_stricmp (cmd, "promode") == 0 && (allowedCmds & ADMIN_PROMODE) ) {
			Cmd_Admin_ProMode_f( ent, atoi( arg ) );
			return;
		}
		if (Q_stricmp (cmd, "timelimit") == 0 && (allowedCmds & ADMIN_TIMELIMIT) ) {
			Cmd_Admin_Timelimit_f( ent, atoi( arg ) );
			return;
		}
		if (Q_stricmp (cmd, "scorelimit") == 0 && (allowedCmds & ADMIN_SCORELIMIT) ) {
			Cmd_Admin_Scorelimit_f( ent, atoi( arg ) );
			return;
		}
		if (Q_stricmp (cmd, "ruleset") == 0 && (allowedCmds & ADMIN_RULESET) ) {
			Cmd_Admin_RuleSet_f( ent, atoi( arg ) );
			return;
		}
		if (Q_stricmp (cmd, "teamsize") == 0 && (allowedCmds & ADMIN_TEAMSIZE) ) {
			Cmd_Admin_TeamSize_f( ent, atoi( arg ) );
			return;
		}
		if (Q_stricmp (cmd, "shortgame") == 0 && (allowedCmds & ADMIN_SHORTGAME) ) {
			Cmd_Admin_ShortGame_f( ent, atoi( arg ) );
			return;
		}

		// invalid command
		trap_SendServerCommand( ent-g_entities, "print \"Invalid admin command.\n\n\"" );

	}

	Cmd_AllowedAdminCmds_f( ent, qtrue );

}

/*
=================
Cmd_AdminReq_f
=================
*/
void Cmd_AdminReq_f( gentity_t *ent ) {
	char		pw[MAX_TOKEN_CHARS];

	if ( !strlen(g_adminPassword.string) ) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Administrator rights requests are disabled.\n\"", NF_ERROR) );
		return;
	}

	if ( trap_Argc() < 2 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: adminreq <password w/ no spaces>\n\"" );
		return;
	}

	if ( ent->client->sess.admin == qtrue ) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You already have administrator rights.\n\"", NF_ERROR) );
		return;
	}

	trap_Argv( 1, pw, sizeof( pw ) );

	if ( strcmp( g_adminPassword.string, pw) != 0 || ent->client->sess.adminReqFails > 2 ) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Administrator rights request denied, invalid password.\n\"", NF_ERROR) );
		ent->client->sess.adminReqFails++;
		G_LogPrintf( "AdminReq DENIED: No. %i - %s\n", ent-g_entities, ent->client->pers.netname );
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You received administrator rights, handle with care.\n\"", NF_GAMEINFO) );
	ent->client->sess.admin = qtrue;
	G_LogPrintf( "AdminReq Accepted: No. %i - %s\n", ent-g_entities, ent->client->pers.netname );

}


/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, "print \"You must be alive to use this command.\n\"");
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
StringIsInteger
==================
*/
qboolean StringIsInteger( const char * s ) {
	int			i;
	int			len;
	qboolean	foundDigit;

	len = strlen( s );
	foundDigit = qfalse;

	for ( i=0 ; i < len ; i++ ) {
		if ( !isdigit( s[i] ) ) {
			return qfalse;
		}

		foundDigit = qtrue;
	}

	return foundDigit;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		cleanName[MAX_STRING_CHARS];

	// numeric values could be slot numbers
	if ( StringIsInteger( s ) ) {
		idnum = atoi( s );
		if ( idnum >= 0 && idnum < level.maxclients ) {
			cl = &level.clients[idnum];
			if ( cl->pers.connected == CON_CONNECTED ) {
				return idnum;
			}
		}
	}

	// check for a name match
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		Q_strncpyz(cleanName, cl->pers.netname, sizeof(cleanName));
		Q_CleanStr(cleanName);
		if ( !Q_stricmp( cleanName, s ) ) {
			return idnum;
		}
	}

	//trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/*
==================
Cmd_GiveUp_f

Player gives up the match, and ends up
==================
*/

void Cmd_GiveUp_f (gentity_t *ent)
{

	int		clientNum;
	int		clientNum0;
	int		clientNum1;

	// Prevent the player from giving up again
	if (ent->client->disableGiveUp == qtrue) {
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR){
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command as a spectator.\n\"", NF_ERROR) );
		return;
	}

	if ( level.warmupTime ) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"This command cannot be called during warm-up.\n\"", NF_ERROR) );
		return;
	}

	if ( g_gametype.integer < GT_TEAM ) {
		if ( g_gametype.integer == GT_FFA ) {
			trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command in Free For All.\n\"", NF_ERROR) );
			return;
		} else
		if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
			trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command in Single Player.\n\"", NF_ERROR) );
			return;
		}
		clientNum0 = level.sortedClients[0];
		clientNum1 = level.sortedClients[1];
		if ( clientNum1 != ent->client->ps.clientNum ) {
			// make sure the scores are not tied
			if ( level.clients[clientNum0].ps.persistant[PERS_SCORE] == level.clients[clientNum1].ps.persistant[PERS_SCORE] ) {
				trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command while not losing.\n\"", NF_ERROR) );
				return;
			}
		}
	} else {
		// TODO: make this command work in team based games
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot use this command in team based games.\n\"", NF_ERROR) );
		return;
	}

	if (ent->client->giveUp == qfalse) {
		ent->client->giveUp = qtrue;
		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_LTGRAY " gives up.\n\"",
			NF_GAMEINFO, ent->client->pers.netname) );
	} else {
		ent->client->giveUp = qfalse;

		// Since the player changed their mind, prevent them from giving up again.
		// This also prevents spamming.
		ent->client->disableGiveUp = qtrue;

		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_LTGRAY " no longer gives up.\n\"",
			NF_GAMEINFO, ent->client->pers.netname) );
	}

}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	name = ConcatArgs( 1 );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH]; // 200 health
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		// give all valid weapons other than the grappling hook
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 -
			( 1 << WP_GRAPPLING_HOOK ) - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for ( i = 2 ; i < AT_NUM_AMMO ; i++ ) {
//			ent->client->ps.ammo[i] = 999;
			ent->client->ps.ammo[i] = 666; // because is was meant to be
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		ent->client->ps.stats[STAT_ARMOR] = 200; // 200 armor points
		ent->client->ps.stats[STAT_ARMORTIER] = 3; // tier 3 armor

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "keycards") == 0)
	{
		ent->client->ps.stats[STAT_INVENTORY] |= 7; // all keycards

		if (!give_all)
			return;
	}

/*
	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
#ifdef MISSIONPACK
	if (Q_stricmp(name, "impressive") == 0) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
#endif
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}
*/

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF";
	else
		msg = "godmode ON";
		//msg = "godmode ON\n";

	//trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
	trap_SendServerCommand( ent-g_entities, va("notify %i\\\"%s\n\"", NF_GAMEINFO, msg ) );
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF";
	} else {
		msg = "noclip ON";
		//msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	//trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
	trap_SendServerCommand( ent-g_entities, va("notify %i\\\"%s\n\"", NF_GAMEINFO, msg ) );
}

/*
==================
Cmd_Ghost_f

argv(0) ghost
==================
*/
void Cmd_Ghost_f( gentity_t *ent ) {
	char	*msg;

	/*
	if ( !CheatsOk( ent ) ) {
		return;
	}
	*/

	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		return;
	}

	// ghost mode disabled
	if ( !g_allowGhost.integer ) {
		return;
	}

	if ( ent->client->ghost ) {
		msg = "ghost OFF";
	} else {
		msg = "ghost ON";
	}
	ent->client->ghost = !ent->client->ghost;

	trap_SendServerCommand( ent-g_entities, va("notify %i\\\"%s\n\"", NF_GAMEINFO, msg ) );
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f(gentity_t *ent)
{
	if(!ent->client->pers.localClient)
	{
		trap_SendServerCommand(ent-g_entities,
			"print \"The levelshot command must be executed by a local client\n\"");
		return;
	}

	if(!CheatsOk(ent))
		return;

	// doesn't work in single player
	if(g_gametype.integer == GT_SINGLE_PLAYER)
	{
		trap_SendServerCommand(ent-g_entities,
			"print \"Must not be in singleplayer mode for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap_SendServerCommand(ent-g_entities, "clientLevelShot");
}


/*
==================
Cmd_TeamTask_f
==================
*/
void Cmd_TeamTask_f( gentity_t *ent ) {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if (ent->health <= 0) {
		return;
	}
	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -666; // was -999
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	if ( client->sess.sessionTeam == TEAM_RED ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the red team.\n\"",
			client->pers.netname) );
	} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the blue team.\n\"",
		client->pers.netname));
	} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the spectators.\n\"",
		client->pers.netname));
	} else if ( client->sess.sessionTeam == TEAM_FREE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the battle.\n\"",
		client->pers.netname));
	}
}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}

		// force team balance
		if ( g_teamForceBalance.integer  ) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( clientNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( clientNum, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				/*trap_SendServerCommand( clientNum,
					"cp \"Red team has too many players.\n\"" );*/
				trap_SendServerCommand( ent-g_entities,
					va("notify %i\\\"Red team has too many players, sorry.\n\"", NF_ERROR) );
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				/*trap_SendServerCommand( clientNum,
					"cp \"Blue team has too many players.\n\"" );*/
				trap_SendServerCommand( ent-g_entities,
					va("notify %i\\\"Blue team has too many players, sorry.\n\"", NF_ERROR) );
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

		// can only be one player on blue in AA1
		if ( g_gametype.integer == GT_AA1 && team == TEAM_BLUE ) {
			if ( TeamCount( clientNum, TEAM_BLUE ) > 0 ) {
				trap_SendServerCommand( ent-g_entities,
					va("notify %i\\\"Cannot join blue.  Blue already has a player, sorry.\n\"", NF_ERROR) );
				return; // ignore the request
			}
		}

		// team size
		if ( g_teamSize.integer ) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( clientNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( clientNum, TEAM_RED );

			trap_SendServerCommand( -1, va ("print \"DEBUG: R=%i B=%i MAX=%i\n\"",
					counts[TEAM_RED], counts[TEAM_BLUE], g_teamSize.integer) );

			if ( team == TEAM_RED && counts[TEAM_RED] >= g_teamSize.integer ) {
				if ( g_teamSize.integer == 1 ) {
					trap_SendServerCommand( ent-g_entities,
						va("notify %i\\\"Red team cannot accept anymore players.  Only 1 player per team, sorry.\n\"", NF_ERROR) );
				} else {
					trap_SendServerCommand( ent-g_entities,
						va("notify %i\\\"Red team cannot accept anymore players.  Only %i players per team, sorry.\n\"", NF_ERROR, g_teamSize.integer) );
				}
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] >= g_teamSize.integer ) {
				if ( g_teamSize.integer == 1 ) {
					trap_SendServerCommand( ent-g_entities,
						va("notify %i\\\"Blue team cannot accept anymore players.  Only 1 player per team, sorry.\n\"", NF_ERROR) );
				} else {
					trap_SendServerCommand( ent-g_entities,
						va("notify %i\\\"Blue team cannot accept anymore players.  Only %i players per team, sorry.\n\"", NF_ERROR, g_teamSize.integer) );
				}
				return; // ignore the request
			}

			// there are available team slots
		}

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( (g_gametype.integer == GT_TOURNAMENT)
		&& level.numNonSpectatorClients >= 2 ) {
		team = TEAM_SPECTATOR;
	} else if ( g_maxGameClients.integer > 0 &&
		level.numNonSpectatorClients >= g_maxGameClients.integer ) {
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		CopyToBodyQue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die (ent, ent, ent, 100000, /*MOD_SUICIDE*/ MOD_NOTHING);

	}

	// they go to the end of the line for tournaments
//	if(team == TEAM_SPECTATOR && oldTeam != team)
//		AddTournamentQueue(client);

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	// reset aimbot detection meter
//	client->pers.ab_meter = 0;

	// update warmup ready status
	client->pers.ready = qfalse;
	if (team == TEAM_SPECTATOR)
		SendReadymask( -1, 1 );
	else
		SendReadymask( -1, 1 );

	//BroadcastTeamChange( client, oldTeam );

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum );

	ClientBegin( clientNum );
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;

	SetClientViewAngle( ent, ent->client->ps.viewangles );

	// prevent spectators from getting their view stuck at one angle
	// TODO: check if this fucks up anything
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		ent->client->ps.stats[STAT_HEALTH] = 1;
	}
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		oldTeam = ent->client->sess.sessionTeam;
		switch ( oldTeam ) {
			case TEAM_BLUE:
				trap_SendServerCommand( ent-g_entities, "print \"Blue team\n\"" );
				break;
			case TEAM_RED:
				trap_SendServerCommand( ent-g_entities, "print \"Red team\n\"" );
				break;
			case TEAM_FREE:
				trap_SendServerCommand( ent-g_entities, "print \"Free team\n\"" );
				break;
			case TEAM_SPECTATOR:
				trap_SendServerCommand( ent-g_entities, "print \"Spectator team\n\"" );
				break;
		}
		return;
	}

	// keep suspended players out of the game
	if (ent->client->sess.suspended == qtrue) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You are currently suspended from the game.\n\"", NF_ERROR ) );
		return;
	}

	// will uncomment when testing is completed
	/*if ( ent->client->switchTeamTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 5 seconds.\n\"" );
		return;
	}*/

	// if they are playing a tournament game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE
		&& level.warmupTime == 0 ) {
		ent->client->sess.losses++;
	}

	trap_Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s );

	ent->client->switchTeamTime = level.time + 5000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	// if they are playing a tournament game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE
		&& level.warmupTime == 0 ) {
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir, int ftype ) {
	int		clientnum;
	int		original;

	// if they are playing a tournament game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client->sess.sessionTeam == TEAM_FREE
		&& level.warmupTime == 0 ) {
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	if ( dir > 1 && dir < -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	// if dedicated follow client, just switch between the two auto clients
	if (ent->client->sess.spectatorClient < 0) {
		if (ent->client->sess.spectatorClient == -1) {
			ent->client->sess.spectatorClient = -2;
		} else if (ent->client->sess.spectatorClient == -2) {
			ent->client->sess.spectatorClient = -1;
		}
		return;
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;

	// dir set to 0 is meant for toggling follow mode
	if ( dir == 0 ) {
		if ( clientnum < level.maxclients ) {
			if (level.clients[ clientnum ].pers.connected == CON_CONNECTED &&
							level.clients[ clientnum ].sess.sessionTeam != TEAM_SPECTATOR) {
				ent->client->sess.spectatorClient = clientnum;
				ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
				return;
			}
		}

		// ok, currently, level.clients[clientnum] is not an active player, let's check the rest of the clients
		dir = 1; // most set this
	}

	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// switch based on a filter
		if ( ftype ) {
			switch ( ftype ) {

				// filter based on powerup
				case FTYPE_POWERUP:
					if ( !level.clients[ clientnum ].ps.powerups[PW_PENT] && !level.clients[ clientnum ].ps.powerups[PW_QUAD] ) {
						continue;
					}

				// filter based on objective
				case FTYPE_OBJECTIVE:
					if ( !level.clients[ clientnum ].ps.powerups[PW_REDFLAG] && !level.clients[ clientnum ].ps.powerups[PW_BLUEFLAG] ) {
						continue;
					}
					break;

				// filter based on score leader
				case FTYPE_LEADER:
					if ( /*clientnum != level.sortedClients[0] && */ level.clients[clientnum].ps.persistant[PERS_SCORE] != level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE] ) {
						continue;
					}
					break;
			}
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}


/*
=================
Cmd_RndInProgress_f
=================
*/

void Cmd_RndInProgress_f( gentity_t *ent ) {

	switch ( level.rnd_type ) {
		case RT_COINTOSS:
			trap_SendServerCommand( ent-g_entities, va("notify %i\\\"A cointoss is in progress.\n\"", NF_ERROR) );
			return;

		case RT_RNDSELECT:
			trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Random select is in progress.\n\"", NF_ERROR) );
			return;

		case RT_RNDNUM:
			trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Random number select is in progress.\n\"", NF_ERROR) );
			return;
	}

}

/*
=================
Cmd_Cointoss_f

cointoss
=================
*/
void Cmd_Cointoss_f( gentity_t *ent ) {

	if ( !level.warmupTime ) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"This command can only be called during warm-up, sorry.\n\"", NF_ERROR) );
		return;
	}

	if ( level.rnd_mode ) {
		Cmd_RndInProgress_f( ent );
		return;
	}

	if (ent->client->sess.mute == qtrue) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You cannot call a cointoss while muted.\n\"", NF_ERROR ) );
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("notify %i\\\"%s" S_COLOR_WHITE " called for a cointoss.\n\"",
		NF_GAMEINFO, ent->client->pers.netname ) );

	level.rnd_mode = 1; // first step
	level.rnd_nextthink = level.time + 2000;
	level.rnd_type = RT_COINTOSS;

}

/*
=================
Cmd_Rnd_f

random select

mmp - code borrowed from KTX: a Quakeworld server MOD
=================
*/
void Cmd_Rnd_f( gentity_t *ent ) {
	int argc, i, rnd_sel;
	char arg[32], buffer[1024] = {0};

	if ( !level.warmupTime ) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"This command can only be called during warm-up, sorry.\n\"", NF_ERROR) );
		return;
	}

	if ( level.rnd_mode ) {
		/*trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Random select is already in progress.\n\"", NF_ERROR) );*/
		Cmd_RndInProgress_f( ent );
		return;
	}

	if (ent->client->sess.mute == qtrue) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You cannot call random select while muted.\n\"", NF_ERROR ) );
		return;
	}

	if ( ( argc = trap_Argc() ) < 3 ) {
		trap_SendServerCommand( ent-g_entities,
				va("notify %i\\\"Usage: rnd <1st 2nd ...>.\n\"", NF_ERROR) );
		return;
	}

	for( buffer[0] = 0, i = 1; i < argc; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );

		strcat(buffer, arg);
		strcat(buffer, ( i + 1 < argc ? S_COLOR_WHITE ", " : S_COLOR_WHITE "" )); // i don't think i need that empty quote string
	}

	trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Random select by %s" S_COLOR_WHITE " from: %s.\n\"",
		NF_GAMEINFO, ent->client->pers.netname, buffer) );

	// predefine the selection (only the server knows)
	rnd_sel = ( random() * (argc - 1) ) + 1;
	trap_Argv( rnd_sel, level.rnd_selected, sizeof( level.rnd_selected ) );

	// test
	/*trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Selected: %s.\n\"",
		NF_GAMEINFO, level.rnd_selected) );*/

	level.rnd_mode = 1; // first step
	level.rnd_nextthink = level.time + 2000;
	level.rnd_type = RT_RNDSELECT;

}

/*
=================
Cmd_RndNum_f

random number
=================
*/
void Cmd_RndNum_f( gentity_t *ent ) {
	int			num1, num2;
	char		str[MAX_TOKEN_CHARS];

	if ( !level.warmupTime ) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"This command can only be called during warm-up, sorry.\n\"", NF_ERROR) );
		return;
	}

	if ( level.rnd_mode ) {
		Cmd_RndInProgress_f( ent );
		return;
	}

	if (ent->client->sess.mute == qtrue) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You cannot call a random number while muted.\n\"", NF_ERROR ) );
		return;
	}

	if ( trap_Argc() != 3 ) {
		trap_SendServerCommand( ent-g_entities,
				va("notify %i\\\"Usage: rndnum <num1 num2>.\n\"", NF_ERROR) );
		return;
	}

	// TODO: code this shit better
	trap_Argv( 1, str, sizeof( str ) );
	num1 = atoi (str);
	trap_Argv( 2, str, sizeof( str ) );
	num2 = atoi (str);

	if ( num1 >= num2 ) {
		trap_SendServerCommand( ent-g_entities,
				va("notify %i\\\"Second number must be higher than the first.\n\"", NF_ERROR) );
		return;
	}

	trap_SendServerCommand( ent-g_entities,
		va("notify %i\\\"%s" S_COLOR_WHITE " called for a random number between %i and %i.\n\"",
		NF_GAMEINFO, ent->client->pers.netname, num1, num2 ) );

	level.rnd_minNum = num1;
	level.rnd_maxNum = num2;
	level.rnd_mode = 1; // first step
	level.rnd_nextthink = level.time + 2000;
	level.rnd_type = RT_RNDNUM;

}



/*
==================
Cmd_SpecMode_f
==================
*/

void Cmd_SpecMode_f( gentity_t *ent ) {
	char	arg[MAX_TOKEN_CHARS];

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR){
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"This command can only be used by spectators, sorry.\n\"", NF_ERROR) );
		return;
	}

	if ( trap_Argc() != 2 ) {
		trap_SendServerCommand( ent-g_entities,
				va("notify %i\\\"Usage: specMode <option ('lastFrag', 'leader' or 'off')>.\n\"", NF_ERROR) );
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (Q_stricmp (arg, "off") == 0) {
		ent->client->sess.specMode = SPECMODE_OFF;
		// TODO: word this message better ffs
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Spectator mode is now off.\n\"", NF_GAMEINFO ) );
	}
	else if (Q_stricmp (arg, "leader") == 0) {
		ent->client->sess.specMode = SPECMODE_LEADER;
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Spectator mode is set to follow the leader.\n\"", NF_GAMEINFO ) );
	}
	else if (Q_stricmp (arg, "lastFrag") == 0) {
		ent->client->sess.specMode = SPECMODE_LASTFRAG;
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Spectator mode is set to follow the player who got the last frag.\n\"", NF_GAMEINFO ) );
	}
	else
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Invalid spectator mode.\n\"", NF_ERROR) );

}


/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message ) {
	int		len;

	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}

	// FIXME - this condition is checked twice, only needs to be check once
	if (ent->client->sess.mute == qtrue) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You are currently muted.\n\"", NF_ERROR ) );
		return;
	}
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	// no chatting to players in tournaments
	/*if ( (g_gametype.integer == GT_TOURNAMENT )
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		return;
	}*/

	// do was have g_specChat unset?  if unset, prevent players from receiving spectator chat
	if ( ( !g_specChat.integer )
		&& other->client->sess.sessionTeam != TEAM_SPECTATOR
		&& ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	// TODO: condense the following 5 lines
	len = Q_PrintStrlen( va("%s%c%c",
		name, Q_COLOR_ESCAPE, color) );

	trap_SendServerCommand( other-g_entities, va("%s %i\\\"%s%c%c%s\"",
		mode == SAY_TEAM ? "tchat" : "chat",
		len, name, Q_COLOR_ESCAPE, color, message));
}

#define EC		"\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int		j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	int			timeMin, timeTen, timeSec;

	if (ent->client->sess.mute == qtrue) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You are currently muted.\n\"", NF_ERROR ) );
		return;
	}

	G_SpamCheck( ent );

	/*if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM && ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		mode = SAY_ALL;
	}*/

	// set up digits for time stamp
	timeSec = ( level.time - level.startTime ) / 1000;
	if (timeSec >= 6000) {
		timeSec = 5999;
	}
	timeMin = timeSec / 60;
	timeSec -= timeMin * 60;
	timeTen = timeSec / 10;
	timeSec -= timeTen * 10;

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
		//Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		if ( ent->client->sess.sessionTeam == TEAM_FREE ) {
			// no need to show team name, since there is only one team
			Com_sprintf (name, sizeof(name), "%2i:%i%i %s%c%c"EC": ",
				timeMin, timeTen, timeSec,
				ent->client->pers.netname,
				Q_COLOR_ESCAPE, COLOR_WHITE );
		} else {
			Com_sprintf (name, sizeof(name), "%2i:%i%i %s%s%c%c"EC": ",
				timeMin, timeTen, timeSec,
				ShortTeamName(ent->client->sess.sessionTeam),
				ent->client->pers.netname,
				Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = ent->client->pers.chatColor;
		if ( !((color >= '0' && color <= '9' ) ||
			(color >= 'a' && color <= 'z' ) ||
			(color >= 'A' && color <= 'Z' )) ) {
			color = COLOR_GREEN; // use default color if not a valid color code
		}
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf (name, sizeof(name), EC"%2i:%i%i (%s%c%c"EC") (%s)"EC": ",
				timeMin, timeTen, timeSec,
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location);
		else
			Com_sprintf (name, sizeof(name), EC"%2i:%i%i (%s%c%c"EC")"EC": ",
				timeMin, timeTen, timeSec,
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = ent->client->pers.teamchatColor;
		if ( !((color >= '0' && color <= '9' ) ||
			(color >= 'a' && color <= 'z' ) ||
			(color >= 'A' && color <= 'Z' )) ) {
			color = COLOR_CYAN; // use default color if not a valid color code
		}
		break;
	case SAY_TELL:
		if (target && target->inuse && target->client && g_gametype.integer >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf (name, sizeof(name), EC"%2i:%i%i [%s%c%c"EC"] (%s)"EC": ",
				timeMin, timeTen, timeSec,
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
		else
			Com_sprintf (name, sizeof(name), EC"%2i:%i%i [%s%c%c"EC"]"EC": ",
				timeMin, timeTen, timeSec,
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_MAGENTA;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_SayTo( ent, other, mode, color, name, text );
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: tell <player id> <message>\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = ClientNumberFromString( ent, arg );
	if ( targetNum == -1 ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}


#ifdef MISSIONPACK
static void G_VoiceTo( gentity_t *ent, gentity_t *other, int mode, const char *id, qboolean voiceonly ) {
	int color;
	char *cmd;

	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( mode == SAY_TEAM && !OnSameTeam(ent, other) ) {
		return;
	}
	// no chatting to players in tournaments
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		return;
	}

	if (mode == SAY_TEAM) {
		color = COLOR_CYAN;
		cmd = "vtchat";
	}
	else if (mode == SAY_TELL) {
		color = COLOR_MAGENTA;
		cmd = "vtell";
	}
	else {
		color = COLOR_GREEN;
		cmd = "vchat";
	}

	trap_SendServerCommand( other-g_entities, va("%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id));
}

void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) {
	int			j;
	gentity_t	*other;

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	if ( target ) {
		G_VoiceTo( ent, target, mode, id, voiceonly );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "voice: %s %s\n", ent->client->pers.netname, id);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_VoiceTo( ent, other, mode, id, voiceonly );
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Voice( ent, NULL, mode, p, voiceonly );
}

/*
==================
Cmd_VoiceTell_f
==================
*/
static void Cmd_VoiceTell_f( gentity_t *ent, qboolean voiceonly ) {
	int			targetNum;
	gentity_t	*target;
	char		*id;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	id = ConcatArgs( 2 );

	G_LogPrintf( "vtell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, id );
	G_Voice( ent, target, SAY_TELL, id, voiceonly );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Voice( ent, ent, SAY_TELL, id, voiceonly );
	}
}


/*
==================
Cmd_VoiceTaunt_f
==================
*/
static void Cmd_VoiceTaunt_f( gentity_t *ent ) {
	gentity_t *who;
	int i;

	if (!ent->client) {
		return;
	}

	// insult someone who just killed you
	if (ent->enemy && ent->enemy->client && ent->enemy->client->lastkilled_client == ent->s.number) {
		// i am a dead corpse
		if (!(ent->enemy->r.svFlags & SVF_BOT)) {
			G_Voice( ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		if (!(ent->r.svFlags & SVF_BOT)) {
			G_Voice( ent, ent,        SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		ent->enemy = NULL;
		return;
	}
	// insult someone you just killed
	if (ent->client->lastkilled_client >= 0 && ent->client->lastkilled_client != ent->s.number) {
		who = g_entities + ent->client->lastkilled_client;
		if (who->client) {
			// who is the person I just killed
			if (who->client->lasthurt_mod == MOD_GAUNTLET) {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );	// and I killed them with a gauntlet
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );
				}
			} else {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );	// and I killed them with something else
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );
				}
			}
			ent->client->lastkilled_client = -1;
			return;
		}
	}

	if (g_gametype.integer >= GT_TEAM) {
		// praise a team mate who just got a reward
		for(i = 0; i < MAX_CLIENTS; i++) {
			who = g_entities + i;
			if (who->client && who != ent && who->client->sess.sessionTeam == ent->client->sess.sessionTeam) {
				if (who->client->rewardTime > level.time) {
					if (!(who->r.svFlags & SVF_BOT)) {
						G_Voice( ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					if (!(ent->r.svFlags & SVF_BOT)) {
						G_Voice( ent, ent, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					return;
				}
			}
		}
	}

	// just say something
	G_Voice( ent, NULL, SAY_ALL, VOICECHAT_TAUNT, qfalse );
}
#endif



static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

static const int numgc_orders = ARRAY_LEN( gc_orders );

void Cmd_GameCommand_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	int			order;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 3 ) {
		trap_SendServerCommand( ent-g_entities, va( "print \"Usage: gc <player id> <order 0-%d>\n\"", numgc_orders - 1 ) );
		return;
	}

	trap_Argv( 2, arg, sizeof( arg ) );
	order = atoi( arg );

	if ( order < 0 || order >= numgc_orders ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Bad order: %i\n\"", order));
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = ClientNumberFromString( ent, arg );
	if ( targetNum == -1 ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->client ) {
		return;
	}

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, gc_orders[order] );
	G_Say( ent, target, SAY_TELL, gc_orders[order] );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, gc_orders[order] );
	}
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos(ent->r.currentOrigin) ) );
}

static const char *gameNames[] = {
	"Free For All",
	"Tournament",
	"Single Player",
	"Team Deathmatch",
	"Capture the Flag",
	"One Flag CTF",
	"Overload",
	"Harvester"
};

/*
==================
Cmd_CallVote_f

TODO: code this better
==================
*/

qboolean G_NextMapPick( int	gt ) {

	qboolean	result = qfalse;

	// pick map from rotation based on game type
	if ( gt == GT_FFA )
		result = MapRotation( g_mapRotation_ffa.string );
	else if ( gt == GT_TOURNAMENT )
		result = MapRotation( g_mapRotation_duel.string );
	else if ( gt == GT_TEAM )
		result = MapRotation( g_mapRotation_tdm.string );
	else if ( gt == GT_CTF )
		result = MapRotation( g_mapRotation_ctf.string );

	// pick map from regular rotation
	if ( result == qfalse ) {
		if ( MapRotation( g_mapRotation.string ) == qfalse ) {
			return qfalse;
		}
	}

	return qtrue;

}

void Cmd_CallVote_f( gentity_t *ent ) {
	char*		c;
	int			i;
	int			sec;
	char		arg1[MAX_STRING_TOKENS];
	char		arg2[MAX_STRING_TOKENS];
	int			allowedVotes;

	if ( !g_allowVote.integer ) {
		//trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Voting is unavailable on this server, sorry.\n\"", NF_ERROR ) );
		return;
	}

	if (ent->client->sess.mute == qtrue) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You are currently muted, and cannot call a vote while muted.\n\"", NF_ERROR ) );
		return;
	}

	//mmp - prevent early connecting players from calling votes early before anyone else connects
	if ( g_voteWaitTime.integer > 0 ) {
		if ( g_voteWaitTime.integer > (level.time-level.voteCoolDownStart)/1000 ) {
			sec = g_voteWaitTime.integer - ((level.time-level.voteCoolDownStart)/1000);
			if ( sec < 2 ) {
				trap_SendServerCommand( ent-g_entities,
					va("notify %i\\\"Please wait " S_COLOR_AMBER "1 second" S_COLOR_WHITE " before voting.\n\"", NF_ERROR ) );
			} else {
				trap_SendServerCommand( ent-g_entities,
					va("notify %i\\\"Please wait " S_COLOR_AMBER "%i seconds" S_COLOR_WHITE " before voting.\n\"", NF_ERROR, sec ) );
			}
			return;
		}
	}

	if ( level.voteTime ) {
		//trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"A vote is already in progress, sorry.\n\"", NF_ERROR ) );
		return;
	}
	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		//trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You have reached the maximum amount of votes called, sorry.\n\"", NF_ERROR ) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && !g_allowSpecCallVote.integer ) {
		//trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot call a vote while spectating, sorry.\n\"", NF_ERROR ) );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	// display valid command options, when no command was entered
	/*if ( arg1[0] == '\0' ) {
		G_AllowedVotes( ent, qtrue );
		return;
	}*/

	// check for command separators in arg2
	for( c = arg2; *c; ++c) {
		switch(*c) {
			case '\n':
			case '\r':
			case ';':
				//trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
				trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Invalid vote string.\n\"", NF_ERROR ) );
				return;
			break;
		}
	}

	allowedVotes = G_AllowedVotes( ent, qfalse );
	//trap_SendServerCommand( ent-g_entities, va("notify %i\\\"DEBUG: %i\n\"", NF_ERROR, allowedVotes ) );

	if ( !Q_stricmp( arg1, "restart" ) && (allowedVotes & VOTE_MAP_RESTART) ) {
	} else if ( !Q_stricmp( arg1, "nextmap" ) && (allowedVotes & VOTE_NEXTMAP) ) {
	} else if ( !Q_stricmp( arg1, "map" ) && (allowedVotes & VOTE_MAP) ) {
	} else if ( !Q_stricmp( arg1, "gametype" ) && (allowedVotes & VOTE_GAMETYPE) ) {
	} else if ( !Q_stricmp( arg1, "matchMode" ) && (allowedVotes & VOTE_MATCHMODE) ) {
	} else if ( !Q_stricmp( arg1, "proMode" ) && (allowedVotes & VOTE_PROMODE) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) && (allowedVotes & VOTE_CLIENTKICK) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) && (allowedVotes & VOTE_KICK) ) {
	} else if ( !Q_stricmp( arg1, "clientmute" ) && (allowedVotes & VOTE_CLIENTMUTE) ) {
	} else if ( !Q_stricmp( arg1, "mute" ) && (allowedVotes & VOTE_MUTE) ) {
	} else if ( !Q_stricmp( arg1, "endWarmup" ) && (allowedVotes & VOTE_ENDWARMUP) ) {
	} else if ( !Q_stricmp( arg1, "rpickup" ) && (allowedVotes & VOTE_RPICKUP) &&
			g_gametype.integer >= GT_TEAM && level.warmupTime ) {
	} else if ( !Q_stricmp( arg1, "giveadmin" ) && (allowedVotes & VOTE_GIVEADMIN) ) {
	} else if ( !Q_stricmp( arg1, "timelimit" ) && (allowedVotes & VOTE_TIMELIMIT) &&
			level.rulesetEnforced == qfalse ) {
	} else if ( !Q_stricmp( arg1, "scorelimit" ) && (allowedVotes & VOTE_SCORELIMIT) &&
			level.rulesetEnforced == qfalse ) {
	} else if ( !Q_stricmp( arg1, "ruleSet" ) && (allowedVotes & VOTE_RULESET) ) {
	} else if ( !Q_stricmp( arg1, "teamSize" ) && (allowedVotes & VOTE_TEAMSIZE) ) {
	} else if ( !Q_stricmp( arg1, "shortGame" ) && (allowedVotes & VOTE_SHORTGAME) ) {
	} else if ( !Q_stricmp( arg1, "bots" ) && (allowedVotes & VOTE_BOTS) ) {
	} else if ( !Q_stricmp( arg1, "ext" ) && (allowedVotes & VOTE_EXT) ) {
	} else {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Invalid vote command.\n\"", NF_ERROR ) );
		G_AllowedVotes( ent, qtrue );
		return;
	}

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "gametype" ) ) {
		// check if we used actual names instead of numbers
		if ( !Q_stricmp( arg2, "ffa" ) )
			i = GT_FFA;
		else if ( !Q_stricmp( arg2, "duel" ) )
			i = GT_TOURNAMENT;
		else if ( !Q_stricmp( arg2, "tdm" ) )
			i = GT_TEAM;
		else if ( !Q_stricmp( arg2, "ctf" ) )
			i = GT_CTF;
		else {
			// otherwise, assume a number is used
			i = atoi( arg2 );

			if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
				trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
				return;
			}
		}

		if ( G_NextMapPick( i ) == qtrue ) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "g_gametype %d; vstr nextmap", i );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "g_gametype %d; map_restart", i );
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Change %s to %s", arg1, gameNames[i] );

		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "matchMode" ) ) {
		i = atoi( arg2 );

		// correct the value incase the vote caller is an asshole
		if ( i < 0) {
			i = 0;
		} else if (i >= MM_NUM_MMODES) {
			i = MM_NUM_MMODES - 1;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "g_matchMode %d; map_restart", i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %d", arg1, i );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "proMode" ) ) {
		i = atoi( arg2 );

		// correct the value incase the vote caller is an asshole
		if ( i < 0) {
			i = 0;
		} else if (i > 1) {
			i = 1;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "g_proMode %d; map_restart", i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %d", arg1, i );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "ext" ) ) {
		char	s[MAX_STRING_CHARS];

		if ( !strlen(arg2) ) {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid extended voting option.\n\"" );
			return;
		}

		trap_Cvar_VariableStringBuffer( va( "ext_%s", arg2 ), s, sizeof(s) );
		if ( !strlen(s) ) {
			trap_SendServerCommand( ent-g_entities, va( "print \"Invalid extended voting option '%s'.\n\"", arg2 ) );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s", s ); // exec cvar ext_[arg2]
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", arg2 );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "ruleSet" ) ) {
		i = atoi( arg2 );

		// correct the value incase the vote caller is an asshole
		if ( i < 0) {
			i = 0;
		} else if (i > 5) {
			i = 5;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "g_ruleSet %d; map_restart", i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %d", arg1, i );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "teamSize" ) ) {
		i = atoi( arg2 );

		// correct the value incase the vote caller is an asshole
		if ( i < 0) {
			i = 0;
		} else if (i > 16) {
			i = 16;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "g_teamSize %d; map_restart", i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %d", arg1, i );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "shortGame" ) ) {
		i = atoi( arg2 );

		Com_sprintf( level.voteString, sizeof( level.voteString ), "g_shortGame %d; map_restart", i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %d", arg1, i );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "bots" ) ) {
		i = atoi( arg2 );

		// correct the value incase the vote caller is an asshole
		if ( i < 0) {
			i = 0;
		} else if (i > 4) {
			i = 4;
		}

		if ( i == 0 ) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "bot_minplayers 0; kickbots" );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "bot_minplayers %d", i );
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "stand-in bots %d", arg1, i );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "map" ) ) {
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (*s) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
		char	s[MAX_STRING_CHARS];

		// pick map from rotation based on game type
		if ( G_NextMapPick( g_gametype.integer ) == qfalse ) {
			trap_SendServerCommand( ent-g_entities, "print \"Map rotation not set.\n\"" );
		}

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			// TODO: condense the two nextmap/rotation error messages
			trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		/*Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );*/
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "nextmap", level.voteString );

		level.currentVoteIsKick = qfalse;

/*	} else if ( !Q_stricmp( arg1, "clientmute" ) || !Q_stricmp( arg1, "mute" ) ) {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "giveadmin" ) ) {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
		level.currentVoteIsKick = qfalse;*/

	} else if ( !Q_stricmp( arg1, "restart" ) ) {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "map_restart", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
		i = atoi( arg2 );

		// correct the value incase the vote caller is an asshole
		if ( i < 3) {
			i = 3;
		} else if (i > 30) {
			i = 30;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d; map_restart", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %d", arg1, i );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "scorelimit" ) ) {
		i = atoi( arg2 );

		// correct the value incase the vote caller is an asshole
		if ( g_gametype.integer == GT_CTF && !level.rs_popCTF ) {
			if ( i < 500) {
				i = 0; // assume the person wants no scorelimit
			} else if (i > 5000) {
				i = 5000;
			}
		} else {
			if ( i < 5) {
				i = 0; // assume the person wants no scorelimit
			} else if (i > 5000) {
				i = 5000;
			}
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d; map_restart", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %d", arg1, i );
		level.currentVoteIsKick = qfalse;

	} else if ( !Q_stricmp( arg1, "clientkick" ) || !Q_stricmp( arg1, "kick" ) ) {
		if ( level.numVotingClients < 4 ) {
			trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Not enough active players to call such a vote, sorry.\n\"", NF_ERROR ) );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
		level.currentVoteIsKick = qtrue;

	} else {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
		level.currentVoteIsKick = qfalse;
	}

	//trap_SendServerCommand( -1, va("print \"%s called a vote.\n\"", ent->client->pers.netname ) );
	trap_SendServerCommand( ent-g_entities, va("notify %i\\\"%s" S_COLOR_WHITE " called a vote.\n\"",
							NF_GAMEINFO, ent->client->pers.netname ) );

	// start the voting
	level.voteTime = level.time;

	// clear vote flags
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}

	// is the caller a spectator?  and if so, are there any active players?
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && level.numVotingClients > 0 ) {
		// don't automatically pass a vote if there are no active players
		level.voteYes = 0;
	} else {
		// player calling the vote automatically votes yes
		level.voteYes = 1;
		ent->client->ps.eFlags |= EF_VOTED;
	}
	level.voteNo = 0;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		//trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"No vote in progress.\n\"", NF_ERROR ) );
		return;
	}
	if (ent->client->sess.mute == qtrue) {
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"You are currently muted, and cannot vote while muted.\n\"", NF_ERROR ) );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		//trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Vote already cast.\n\"", NF_ERROR ) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && !g_allowSpecVote.integer ) {
		//trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Cannot vote while spectating, sorry.\n\"", NF_ERROR ) );
		return;
	}

	//trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );
	trap_SendServerCommand( ent-g_entities, va("notify %i\\\"Vote cast.\n\"", NF_GAMEINFO ) );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || msg[0] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if ( level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, "print \"A team vote is already in progress.\n\"" );
		return;
	}
	if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of team votes.\n\"" );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	for ( i = 2; i < trap_Argc(); i++ ) {
		if (i > 2)
			strcat(arg2, " ");
		trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	// prevent server exploit
	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "leader" ) ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if ( !arg2[0] ) {
			i = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				i = atoi( arg2 );
				if ( i < 0 || i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
					return;
				}

				if ( !g_entities[i].inuse ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxclients ; i++ ) {
					if ( level.clients[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if ( !Q_stricmp(netname, leader) ) {
						break;
					}
				}
				if ( i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if (level.clients[i].sess.sessionTeam == team)
			trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam == team)
			level.clients[i].ps.eFlags &= ~EF_TEAMVOTED;
	}
	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
	trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	int			team, cs_offset;
	char		msg[64];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, "print \"No team vote in progress.\n\"" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_TEAMVOTED ) {
		trap_SendServerCommand( ent-g_entities, "print \"Team vote already cast.\n\"" );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Team vote cast.\n\"" );

	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || msg[0] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, "print \"usage: setviewpos x y z yaw\n\"");
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {
/*
	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex( NULL );

	n = 0;
	for ( i = 0; i < max; i++ ) {
		if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
			n++;
	}

	//trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}


/*
=================
Cmd_Print_RuleSet_f
=================
*/
void Cmd_Print_RuleSet_f( gentity_t *ent ) {

	trap_SendServerCommand( ent-g_entities, "printRuleSet" );

}


/*
=================
Cmd_DropWeapon_f
=================
*/
void Cmd_DropWeapon_f( gentity_t *ent ) {
	gclient_t		*client;
	gitem_t			*item;
	gentity_t		*out;
	int				weapon;

	if ( ent->client->ps.pm_type == PM_DEAD )
		return;

	// can only be used in team based games
	/*if ( g_gametype.integer == GT_AA1 || g_gametype.integer < GT_TEAM )
		return;*/ // mmp - fuck it!  allow it in all gametypes

	// can only be used in matchmodes where weapon-stay is off
	if ( level.rs_matchMode < MM_PICKUP_ALWAYS || level.rs_matchMode > MM_PICKUP_ALWAYS_NOAMMO )
		return;

	weapon = ent->s.weapon;

	// don't drop starter weapon, or go out of range
	if ( weapon <= WP_BLASTER || weapon >= WP_NUM_WEAPONS )
		return;

	// put weapon in a backpack, and drop it
	item = BG_FindItemForBackpack();
	out = Drop_Item_Weapon( ent, item, 0, weapon );
	client = ent->client;
	client->ps.stats[STAT_WEAPONS] ^= ( 1 << weapon );
	G_AddEvent( ent, EV_REMOVE_WEAPON, 0 );

}

/*
=================
Cmd_DropAmmo_f
=================
*/
void Cmd_DropAmmo_f( gentity_t *ent ) {
	gclient_t		*client;
	gitem_t			*item;
	gentity_t		*out;
	int				weapon;

	if ( ent->client->ps.pm_type == PM_DEAD )
		return;

	// can only be used in matchmodes that are not fixed set weapons (MM_ALLWEAPONS_MAXAMMO, MM_ALLWEAPONS, MM_ROCKET_MANIAX)
	if ( level.rs_matchMode > MM_PICKUP_ALWAYS_NOAMMO )
		return;

	// can only be used in matchmodes where weapon-stay is off
	/*if ( level.rs_matchMode < MM_PICKUP_ALWAYS || level.rs_matchMode > MM_PICKUP_ALWAYS_NOAMMO )
		return;*/

	//weapon = ent->s.weapon;

	// put weapon in a backpack, and drop it
	item = BG_FindItemForBackpack();
	out = Drop_Item_Ammo( ent, item, 0 );
	client = ent->client;
	//client->ps.stats[STAT_WEAPONS] ^= ( 1 << weapon );
	G_AddEvent( ent, EV_REMOVE_AMMO, 0 );

}


/*
=================
ClientCommand

TODO: Create table for ClientCommands to run with, instead of using all of these if statements
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if (!ent->client || ent->client->pers.connected != CON_CONNECTED) {
		return;		// not fully in game yet
	}


	trap_Argv( 0, cmd, sizeof( cmd ) );

	if (Q_stricmp (cmd, "say") == 0) {
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0) {
		Cmd_Say_f (ent, SAY_TEAM, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		Cmd_Tell_f ( ent );
		return;
	}
#ifdef MISSIONPACK
	if (Q_stricmp (cmd, "vsay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vsay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vtell") == 0) {
		Cmd_VoiceTell_f ( ent, qfalse );
		return;
	}
	if (Q_stricmp (cmd, "vosay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "vosay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "votell") == 0) {
		Cmd_VoiceTell_f ( ent, qtrue );
		return;
	}
	if (Q_stricmp (cmd, "vtaunt") == 0) {
		Cmd_VoiceTaunt_f ( ent );
		return;
	}
#endif
	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "svinforeq") == 0) {
		G_RuleSetUpdate();
		return;
	}

	// mmp
	if (Q_stricmp (cmd, "cointoss") == 0) {
		Cmd_Cointoss_f(ent);
		return;
	}
	if (Q_stricmp (cmd, "coinflip") == 0) { // deprecated, but kept incase someone forgets it was changed to cointoss
		Cmd_Cointoss_f(ent);
		return;
	}
	if (Q_stricmp (cmd, "rnd") == 0) {
		Cmd_Rnd_f(ent);
		return;
	}
	if (Q_stricmp (cmd, "rndnum") == 0) {
		Cmd_RndNum_f(ent);
		return;
	}

	if (Q_stricmp (cmd, "admin") == 0) {
		Cmd_Admin_f(ent);
		return;
	}

	if (Q_stricmp (cmd, "adminreq") == 0) {
		Cmd_AdminReq_f(ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime) {
		Cmd_Say_f (ent, qfalse, qtrue);
		return;
	}

	if (Q_stricmp (cmd, "giveUp") == 0)
		Cmd_GiveUp_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "ghost") == 0)
		Cmd_Ghost_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "teamtask") == 0)
		Cmd_TeamTask_f (ent);
	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);
	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);
	else if (Q_stricmp (cmd, "follownext") == 0)
		Cmd_FollowCycle_f (ent, 1, FTYPE_NOTHING);
	else if (Q_stricmp (cmd, "followprev") == 0)
		Cmd_FollowCycle_f (ent, -1, FTYPE_NOTHING);
	else if (Q_stricmp (cmd, "follownextpowerup") == 0)
		Cmd_FollowCycle_f (ent, 1, FTYPE_POWERUP);
	else if (Q_stricmp (cmd, "followprevpowerup") == 0)
		Cmd_FollowCycle_f (ent, -1, FTYPE_POWERUP);
	else if (Q_stricmp (cmd, "follownextleader") == 0)
		Cmd_FollowCycle_f (ent, 1, FTYPE_LEADER);
	else if (Q_stricmp (cmd, "followprevleader") == 0)
		Cmd_FollowCycle_f (ent, -1, FTYPE_LEADER);
	else if (Q_stricmp (cmd, "follownextobj") == 0)
		Cmd_FollowCycle_f (ent, 1, FTYPE_OBJECTIVE);
	else if (Q_stricmp (cmd, "followprevobj") == 0)
		Cmd_FollowCycle_f (ent, -1, FTYPE_OBJECTIVE);
	else if (Q_stricmp (cmd, "specMode") == 0)
		Cmd_SpecMode_f( ent );
	else if (Q_stricmp (cmd, "team") == 0) // this command is deprecated
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "join") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0 || Q_stricmp (cmd, "cv") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	/*else if (Q_stricmp (cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f (ent);
	else if (Q_stricmp (cmd, "teamvote") == 0)
		Cmd_TeamVote_f (ent);*/
	else if (Q_stricmp (cmd, "gc") == 0)
		Cmd_GameCommand_f( ent );
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
	/*else if (Q_stricmp (cmd, "stats") == 0)
		Cmd_Stats_f( ent );*/
	else if (Q_stricmp (cmd, "ready") == 0)
		Cmd_Ready_f( ent );
	else if (Q_stricmp (cmd, "players") == 0)
		Cmd_Players_f( ent );
	else if (Q_stricmp (cmd, "ruleSet") == 0 || Q_stricmp (cmd, "rs") == 0)
		Cmd_Print_RuleSet_f( ent );
	else if (Q_stricmp (cmd, "practice") == 0)
		Cmd_Practice_f( ent );

	else if (Q_stricmp (cmd, "dropWeapon") == 0)
		Cmd_DropWeapon_f( ent );
	else if (Q_stricmp (cmd, "dropAmmo") == 0)
		Cmd_DropAmmo_f( ent );

	else
		trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}




