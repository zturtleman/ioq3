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

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"


/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and you can use '*' to match any value
so you can specify an entire class C network with "addip 192.246.40.*"

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

TTimo NOTE: for persistence, bans are stored in g_banIPs cvar MAX_CVAR_VALUE_STRING
The size of the cvar string buffer is limiting the banning to around 20 masks
this could be improved by putting some g_banIPs2 g_banIps3 etc. maybe
still, you should rely on PB for banning instead

==============================================================================
*/

typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			if (*s == '*') // 'match any'
			{
				// b[i] and m[i] to 0
				s++;
				if (!*s)
					break;
				s++;
				continue;
			}
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4] = {0};
	byte	m[4] = {0};
	int		i,j;
	char	iplist_final[MAX_CVAR_VALUE_STRING] = {0};
	char	ip[64] = {0};

	*iplist_final = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		*(unsigned *)m = ipFilters[i].mask;
		*ip = 0;
		for (j = 0 ; j < 4 ; j++)
		{
			if (m[j]!=255)
				Q_strcat(ip, sizeof(ip), "*");
			else
				Q_strcat(ip, sizeof(ip), va("%i", b[j]));
			Q_strcat(ip, sizeof(ip), (j<3) ? "." : " ");
		}		
		if (strlen(iplist_final)+strlen(ip) < MAX_CVAR_VALUE_STRING)
		{
			Q_strcat( iplist_final, sizeof(iplist_final), ip);
		}
		else
		{
			Com_Printf("g_banIPs overflowed at MAX_CVAR_VALUE_STRING\n");
			break;
		}
	}

	trap_Cvar_Set( "g_banIPs", iplist_final );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4] = {0};
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	char		str[MAX_CVAR_VALUE_STRING];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

/*
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
*/

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 3 ) {
		G_Printf("Usage: forceteam <player> <team>\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str );
}

/*
===================
Svcmd_RPickup_f

attn: sago from openarena, you may want to observer the code below for a fully decent random team shuffler.
      the one in openarena is not truly random.  in matter of fact, not even random.

      p.s., yes, i'm also known as m68k, the one who spotted the issue with casted votes being lost from a respawn.
===================
*/
void	Svcmd_RPickup_f( void ) {
	int			i;
	gclient_t	*cl;
	int			clientNum;
	int			playerCount = 0;

	if ( g_gametype.integer < GT_TEAM ) {
		G_Printf("Cannot call random pickup in non-team gametypes\n");
		return;
	}

	if ( !level.warmupTime ) {
		G_Printf("Cannot call random pickup during a match\n");
		return;
	}

	// set everyone to red first
	for( i = 0 ; i < level.numConnectedClients; i++ ) {

		// if the player is a bot, skip it, doesn't like to be changed for some reason
		if ( g_entities[ &level.clients[level.sortedClients[i]] - level.clients].r.svFlags & SVF_BOT ) {
			continue;
		}

		// check if client slot has an active player
		if( level.clients[level.sortedClients[i]].pers.connected == CON_CONNECTED &&
				level.clients[level.sortedClients[i]].sess.sessionTeam != TEAM_SPECTATOR ) {

			level.clients[level.sortedClients[i]].pers.teamState.state = TEAM_BEGIN;
			level.clients[level.sortedClients[i]].pers.ready = qfalse;
			level.clients[level.sortedClients[i]].sess.sessionTeam = TEAM_RED;
			level.clients[level.sortedClients[i]].sess.teamLeader = qfalse;

			playerCount++;
		}
	}

	// if there are no players, don't even bother
	if ( playerCount == 0 ) {
		trap_SendServerCommand( -1, va("notify %i\\\"There are no players for random pickup.\n\"", NF_ERROR) );
		return;
	}

	// only need one player for blue in AA1
	// TODO: code this better
	if ( g_gametype.integer == GT_AA1 ) {
		playerCount = 1;

		while ( playerCount ) {
			i = random() * level.numConnectedClients;

			if ( g_entities[ &level.clients[level.sortedClients[i]] - level.clients].r.svFlags & SVF_BOT ) {
				continue;
			}

			// check if client slot has an active player
			if( level.clients[level.sortedClients[i]].pers.connected == CON_CONNECTED &&
					level.clients[level.sortedClients[i]].sess.sessionTeam == TEAM_RED ) {

				// set this player to blue
				level.clients[level.sortedClients[i]].sess.sessionTeam = TEAM_BLUE;

				// ok, we got our player
				playerCount = 0;

				CheckTeamLeader( TEAM_RED );
				CheckTeamLeader( TEAM_BLUE );

				SendReadymask( -1, 1 );
				trap_SendServerCommand( -1,
						va("notify %i\\\"" S_COLOR_AMBER "Random pickup has been enforced.  Please observe your team.\n\"", NF_GAMEINFO) );

				return;
			}

		}
	}

	// if uneven teams, decide which side will have one extra player
	if ( playerCount & 1 ) {
		playerCount = playerCount >> 1;
		if ( random() * 2 < 1 ) {
			playerCount ++;
		}
	} else {
		playerCount = playerCount >> 1;
	}

	// randomly set some players to blue
	while ( playerCount ) {
		i = random() * level.numConnectedClients;

		if ( g_entities[ &level.clients[level.sortedClients[i]] - level.clients].r.svFlags & SVF_BOT ) {
			continue;
		}

		// check if client slot has an active player
		if( level.clients[level.sortedClients[i]].pers.connected == CON_CONNECTED &&
				level.clients[level.sortedClients[i]].sess.sessionTeam == TEAM_RED ) {

			// set this player to blue
			level.clients[level.sortedClients[i]].sess.sessionTeam = TEAM_BLUE;

			// ok, we're one more closer to being somewhat even
			playerCount--;
		}

	}

	// let's update the needed user info
	for( i = 0 ; i < level.numConnectedClients; i++ ) {

		// if the player is a bot, skip it, doesn't like to be changed for some reason
		if ( g_entities[ &level.clients[level.sortedClients[i]] - level.clients].r.svFlags & SVF_BOT ) {
			continue;
		}

		// check if client slot has an active player
		if( level.clients[level.sortedClients[i]].pers.connected == CON_CONNECTED &&
				level.clients[level.sortedClients[i]].sess.sessionTeam != TEAM_SPECTATOR ) {

			ClientUserinfoChanged( level.sortedClients[i] );
			ClientBegin( level.sortedClients[i] );

		}
	}

	CheckTeamLeader( TEAM_RED );
	CheckTeamLeader( TEAM_BLUE );

	SendReadymask( -1, 1 );
	trap_SendServerCommand( -1,
			va("notify %i\\\"" S_COLOR_AMBER "Random pickup has been enforced.  Please observe your team.\n\"", NF_GAMEINFO) );

}

/*
===================
Svcmd_Mute_f

mute <player>
===================
*/
void	Svcmd_Mute_f( void ) {
	gclient_t	*cl;

	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: mute <player>\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	if ( cl->sess.mute != qtrue ) {

		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_RED " has been muted.\n\"",
			NF_GAMEINFO, cl->pers.netname) );

		// set mute
		cl->sess.mute = qtrue;
	}

	// update number of voting clients
	CalculateRanks();

}

/*
===================
Svcmd_Unmute_f

unmute <player>
===================
*/
void	Svcmd_Unmute_f( void ) {
	gclient_t	*cl;

	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: unmute <player>\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	if ( cl->sess.mute != qfalse ) {

		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_GREEN " is no longer muted.\n\"",
			NF_GAMEINFO, cl->pers.netname) );

		// unset mute
		cl->sess.mute = qfalse;
	}

	// update number of voting clients
	CalculateRanks();

}

/*
===================
Svcmd_Unsuspend_f

unsuspend <player>
===================
*/
void	Svcmd_Unsuspend_f( void ) {
	gclient_t	*cl;

	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: unsuspend <player>\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	if ( cl->sess.suspended != qfalse ) {

		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_WHITE " is no longer suspended from the game.\n\"",
			NF_GAMEINFO, cl->pers.netname) );

		// unset suspended
		cl->sess.suspended = qfalse;
	}

}

/*
===================
Svcmd_GiveAdmin_f

giveadmin <player>
===================
*/
void	Svcmd_GiveAdmin_f( void ) {

	gclient_t	*cl;

	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: giveadmin <player>\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	if ( cl->sess.admin != qtrue ) {

		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_GREEN " has been given administrator rights.\n\"",
			NF_GAMEINFO, cl->pers.netname) );

		// set admin
		cl->sess.admin = qtrue;
	}

}


/*
===================
Svcmd_RemoveAdmin_f

removeadmin <player>
===================
*/
void	Svcmd_RemoveAdmin_f( void ) {

	gclient_t	*cl;

	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: removeadmin <player>\n");
		return;
	}

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	if ( cl->sess.admin != qfalse ) {

		trap_SendServerCommand( -1,
			va("notify %i\\\"%s" S_COLOR_RED " got administrator rights removed.\n\"",
			NF_GAMEINFO, cl->pers.netname) );

		// set admin
		cl->sess.admin = qfalse;
	}

}

/*
===================
Svcmd_EndWarmup_f
===================
*/
void	Svcmd_EndWarmup_f( void ) {

	if ( level.warmupTime ) {
		level.endWarmup = qtrue;
		trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_AMBER "Warm-up has been forced to end.\n\"", NF_GAMEINFO) );
	}

}

/*
===================
Svcmd_Test_f
===================
*/
/*
void	Svcmd_Test_f( void ) {

	G_Printf("Test: %i\n", Q_stricmpf( "ABC", "ABC", 1024 ));
	G_Printf("Test: %i\n", Q_stricmpf( "BBC", "ABC", 1024 ));
	G_Printf("Test: %i\n", Q_stricmpf( "TE-ST", "TEST", 1024 ));
	G_Printf("Test: %i\n", Q_stricmpf( "TEST_", "TEST", 1024 ));


}
*/


char	*ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	/*
	if ( Q_stricmp (cmd, "test") == 0 ) {
		Svcmd_Test_f();
		return qtrue;
	}
	*/

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "rpickup") == 0 ) {
		Svcmd_RPickup_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "abort_podium") == 0) {
		Svcmd_AbortPodium_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}

	if (Q_stricmp (cmd, "mute") == 0) {
		Svcmd_Mute_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "unmute") == 0) {
		Svcmd_Unmute_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "unsuspend") == 0) {
		Svcmd_Unsuspend_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "giveadmin") == 0) {
		Svcmd_GiveAdmin_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeadmin") == 0) {
		Svcmd_RemoveAdmin_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "endwarmup") == 0) {
		Svcmd_EndWarmup_f();
		return qtrue;
	}

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "spam") == 0) {
			trap_SendServerCommand( -1, va("notify %i\\\"" S_COLOR_LTGRAY "Server Announcement: %s\n\"", NF_SPAM, ConcatArgs(1)) );
			return qtrue;
		}

		// this should not be used for regular spam announcments
		/*if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"Server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}*/
	}

	// why the fuck would any admin want this?!  come on!
	/*if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}
		// everything else will also be printed as a say command (mmp - wtf?!!!)
		trap_SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(0) ) );
		return qtrue;
	}*/

	return qfalse;
}


















