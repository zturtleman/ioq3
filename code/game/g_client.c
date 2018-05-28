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

// g_client.c -- client functions that don't happen every frame

static vec3_t	playerMins = {-15, -15, -24};
static vec3_t	playerMaxs = {15, 15, 32};

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
alternative spawning position for info_player_deathmatch
when no spawning position is available
*/
void SP_info_player_deathmatch_alt(gentity_t *ent) {

}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit->client) {
			return qtrue;
		}

	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
*** NOT USED -- mmp
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
*** NOT USED - mmp
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint(qboolean isbot) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL && count < MAX_SPAWN_POINTS)
	{
		if(SpotWouldTelefrag(spot))
			continue;

		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			// spot is not for this human/bot player
			continue;
		}

		spots[count] = spot;
		count++;
	}

	if ( !count ) {	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[MAX_SPAWN_POINTS];
	gentity_t	*list_spot[MAX_SPAWN_POINTS];
	int			numSpots, rnd, i, j;
	qboolean	randomSpawn;

	numSpots = 0;
	spot = NULL;

	// TODO: code this randomspawn check better
	if ( (level.time - level.startTime) < 1000 || level.rs_randomSpawn ) {
		randomSpawn = qtrue;
	} else {
		randomSpawn = qfalse;
	}

	while((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if(SpotWouldTelefrag(spot))
			continue;

		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			// spot is not for this human/bot player
			continue;
		}

		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );

		if ( randomSpawn == qfalse ) {

			for (i = 0; i < numSpots; i++)
			{
				if(dist > list_dist[i])
				{
					if (numSpots >= MAX_SPAWN_POINTS)
						numSpots = MAX_SPAWN_POINTS - 1;

					for(j = numSpots; j > i; j--)
					{
						list_dist[j] = list_dist[j-1];
						list_spot[j] = list_spot[j-1];
					}

					list_dist[i] = dist;
					list_spot[i] = spot;

					numSpots++;
					break;
				}
			}

			if(i >= numSpots && numSpots < MAX_SPAWN_POINTS)
			{
				list_dist[numSpots] = dist;
				list_spot[numSpots] = spot;
				numSpots++;
			}

		} else {
			if (numSpots < MAX_SPAWN_POINTS) {
				list_dist[numSpots] = dist;
				list_spot[numSpots] = spot;
				numSpots++;
			}
		}
	}

	// if no slots were found, use alternative spawning points
	//spot = NULL;
	if( !numSpots ) {
		while((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch_alt")) != NULL)
		{
			if(SpotWouldTelefrag(spot))
				continue;

			VectorSubtract( spot->s.origin, avoidPoint, delta );
			dist = VectorLength( delta );

			if(i >= numSpots && numSpots < MAX_SPAWN_POINTS)
			{
				list_dist[numSpots] = dist;
				list_spot[numSpots] = spot;
				numSpots++;
			}
		}
	}

	// if still no slots were found, use last resort
	if(!numSpots)
	{
		spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");

		// if all else fails... well, fuck it
		if (!spot) {
			G_Printf( "*the map creator forgot to put in a spawn point...  what an asshole!\n" );
			spot = &g_entities[0];
		}

		/*if (!spot)
			G_Error( "Couldn't find a spawn point" );*/

		VectorCopy (spot->s.origin, origin);
		origin[2] += 9;
		VectorCopy (spot->s.angles, angles);
		return spot;
	}

	if ( randomSpawn == qfalse ) {
		// select a random spot from the spawn points furthest away
		rnd = random() * (numSpots / 2);
	} else {
		// simply select a random spot, and don't give a fuck
		rnd = random() * numSpots;
		/*rnd = ( ( random() * numSpots ) + g_randomByte.integer );
		rnd %= numSpots;*/
		/*G_Printf ("^8DEBUG: rnd=%i, rb=%i, max=%i\n", rnd, g_randomByte.integer, numSpots);*/
	}

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
TODO - make this cleaner - mmp
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, qboolean isbot ) {
	return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles, isbot );

	/*
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint ( );
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint ( );
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint ( );
		}
	}

	// find a single player start spot
	if (!spot) {
		G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
	*/
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, qboolean isbot ) {
	gentity_t	*spot;

	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if(((spot->flags & FL_NO_BOTS) && isbot) ||
		   ((spot->flags & FL_NO_HUMANS) && !isbot))
		{
			continue;
		}

		if((spot->spawnflags & 0x01))
			break;
	}

	if (!spot || SpotWouldTelefrag(spot))
		return SelectSpawnPoint(vec3_origin, origin, angles, isbot);

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if ( level.time - ent->timestamp > 6500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
#ifdef MISSIONPACK
	gentity_t	*e;
	int i;
#endif
	gentity_t		*body;
	int			contents;

	trap_UnlinkEntity (ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc
#ifdef MISSIONPACK
	if ( ent->s.eFlags & EF_KAMIKAZE ) {
		body->s.eFlags |= EF_KAMIKAZE;

		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < MAX_GENTITIES; i++) {
			e = &g_entities[i];
			if (!e->inuse)
				continue;
			if (e->activator != ent)
				continue;
			if (strcmp(e->classname, "kamikaze timer"))
				continue;
			e->activator = body;
			break;
		}
	}
#endif
	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	// change the animation to the last-frame only, so the sequence
	// doesn't repeat anew for the body
	switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
	case BOTH_DEATH1:
	case BOTH_DEAD1:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
		break;
	case BOTH_DEATH2:
	case BOTH_DEAD2:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
		break;
	case BOTH_DEATH3:
	case BOTH_DEAD3:
	default:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
		break;
	}

	body->r.svFlags = ent->r.svFlags;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

	body->nextthink = level.time + 5000;
	body->think = BodySink;

	body->die = body_die;

	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}


	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity (body);
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
ClientRespawn
================
*/
void ClientRespawn( gentity_t *ent ) {

	CopyToBodyQue (ent);
	ClientSpawn(ent);
}

/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			if ( level.clients[i].sess.teamLeader )
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int		counts[TEAM_NUM_TEAMS];

	counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE );
	counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED );

	// check which team has less players
	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}

	// equal team count, so join the team with the lowest score
	/*if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] {
		return TEAM_RED;
	}*/
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] )
		return TEAM_RED;
	else if ( level.teamScores[TEAM_BLUE] < level.teamScores[TEAM_RED] )
		return TEAM_BLUE;
;
	// if all else fails, randomly choose a team
	if ( rand()&1 )
		return TEAM_BLUE;
	else
		return TEAM_RED;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
/*
static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}
*/

/*
===========
ClientCleanName (ClientCheckName)
============
*/
static void ClientCleanName(const char *in, char *out, int outSize)
{
	int		outpos = 0, spaces = 1;

	// discard leading spaces
	for(; *in == ' '; in++);

	for(; *in && outpos < outSize - 1 && outpos < MAX_NAME_LENGTH; in++)
	{
		out[outpos] = *in;

		if(*in == ' ')
		{
			// don't allow too many consecutive spaces
			if(spaces > 1)
				continue;

			spaces++;
		}
		else
		if (*in < ' ' || *in == 0x7F)
		{
			// don't allow wackie-nunu characters
			continue;
		}
		else
		{
			spaces = 0;
		}

		outpos++;
	}

	out[outpos] = '\0';

	// don't allow empty names
	if( *out == '\0')
		Q_strncpyz(out, "UnnamedPlayer", outSize );

	// name filters
	// TODO: code this better
	// UPDATE 150403: yes, this really needs to be put on a fucking table ffs
	// UPDATE 150602: have we come to the point where everyone is listed in the name filter?
	// UPDATE 151210: ok, i removed most of the names, no need for bloat that was nothing more than a quick joke  ;)
	else if (!Q_stricmpf( out, "Sasparillo", outSize )) // from QL
		Q_strncpyz(out, "Corpo" FORCE_NAME_2 "ate " FORCE_NAME_1 FORCE_NAME_0 FORCE_NAME_3 FORCE_NAME_2, outSize );
	else if (!Q_stricmpf( out, "Yakumo", outSize )) // from QL
		Q_strncpyz(out, "Corpo" FORCE_NAME_2 "ate " FORCE_NAME_1 FORCE_NAME_0 FORCE_NAME_3 FORCE_NAME_2, outSize );

	else if (!Q_stricmpf( out, "Sponge", outSize )) // from QL
		Q_strncpyz(out, S_COLOR_PINK "Sponge-", outSize );
	else if (!Q_stricmpf( out, "SyncError", outSize )) // from QL
		Q_strncpyz(out, "Corpo" FORCE_NAME_2 "ate " FORCE_NAME_1 FORCE_NAME_0 FORCE_NAME_3 FORCE_NAME_2, outSize );

}


/*
===========
ClientCleanClan (ClientCheckClan)
============
*/
static void ClientCleanClan(const char *in, char *out, int outSize)
{
	int		outpos = 0, spaces = 1;
	int		charPos = 0;

	// discard leading spaces
	for(; *in == ' '; in++);

	for(; *in && outpos < outSize - 1 && outpos < MAX_NAME_LENGTH && charPos < MAX_CLAN_CHAR_LENGTH; in++)
	{
		out[outpos] = *in;

		if(*in == ' ')
		{
			// don't allow too many consecutive spaces
			if(spaces > 1)
				continue;

			spaces++;
		}
		else
		if (*in < ' ' || *in == 0x7F)
		{
			// don't allow wackie-nunu characters
			continue;
		}
		else
		{
			spaces = 0;
		}

		if ( Q_IsColorString( in ) ) {
			charPos--;
		} else {
			charPos++;
		}
		outpos++;
	}

	out[outpos] = '\0';
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
	gentity_t *ent;
	int		teamTask, teamLeader, team, health; // TODO: the health variable is no longer needed, it can be removed now
	char	*s;
	char	model[MAX_QPATH];
	char	headModel[MAX_QPATH];
	char	oldname[MAX_STRING_CHARS];
	char	oldclan[MAX_STRING_CHARS];
	gclient_t	*client;
	char	c1[MAX_INFO_STRING];
	char	c2[MAX_INFO_STRING];
	char	c3[MAX_INFO_STRING];
	char	redTeam[MAX_INFO_STRING];
	char	blueTeam[MAX_INFO_STRING];
	char	userinfo[MAX_INFO_STRING];
	//char	ipAdd[MAX_INFO_STRING]

	ent = g_entities + clientNum;
	client = ent->client;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate(userinfo) ) {
		strcpy (userinfo, "\\name\\badinfo"); // badd apple
		// don't keep those clients and userinfo
		trap_DropClient(clientNum, "Invalid userinfo");
	}

	// check for local client
	s = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( s, "localhost" ) ) {
		client->pers.localClient = qtrue;
	}

	Q_strncpyz ( client->pers.ip, s, sizeof( client->pers.ip ) );
	// ^ keep an eye on this, it was used before with ClientConnect(...), and caused issues such as changing varibles

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

//unlagged - client options
	// see if the player has opted out
	s = Info_ValueForKey( userinfo, "cg_delag" );
	if ( !atoi( s ) ) {
		client->pers.delag = 0;
	} else {
		client->pers.delag = atoi( s );
	}

	// see if the player is nudging his shots
	s = Info_ValueForKey( userinfo, "cg_cmdTimeNudge" );
	client->pers.cmdTimeNudge = atoi( s );

	// see if the player wants to debug the backward reconciliation
	/*s = Info_ValueForKey( userinfo, "cg_debugDelag" );
	if ( !atoi( s ) ) {
		client->pers.debugDelag = qfalse;
	}
	else {
		client->pers.debugDelag = qtrue;
	}*/

	// see if the player is simulating incoming latency
	//s = Info_ValueForKey( userinfo, "cg_latentSnaps" );
	//client->pers.latentSnaps = atoi( s );

	// see if the player is simulating outgoing latency
	//s = Info_ValueForKey( userinfo, "cg_latentCmds" );
	//client->pers.latentCmds = atoi( s );

	// see if the player is simulating outgoing packet loss
	//s = Info_ValueForKey( userinfo, "cg_plOut" );
	//client->pers.plOut = atoi( s );
//unlagged - client options

	// set name
	// TODO: disable name changing when muted
	Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey (userinfo, "name");
	ClientCleanName( s, client->pers.netname, sizeof(client->pers.netname) );

	// mmp - clan info is a WIP, and currently incomplete
	s = Info_ValueForKey (userinfo, "tag");
	ClientCleanClan( s, client->pers.clan, sizeof(client->pers.clan) );

	// mmp - chat color
	s = Info_ValueForKey (userinfo, "chatColorCode");
	client->pers.chatColor = *s;
	s = Info_ValueForKey (userinfo, "teamchatColorCode");
	client->pers.teamchatColor = *s;

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
		}
	}

	if ( client->pers.connected == CON_CONNECTED ) {
		if ( strcmp( oldname, client->pers.netname ) ) {
			/*trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname,
				client->pers.netname) );*/
			trap_SendServerCommand( -1, va("notify %i\\\"%s" S_COLOR_WHITE " is now known as %s\n\"",
				NF_NAMECHANGES, oldname, client->pers.netname) );
		}
	}

	// set max health and handicap
	// TODO: differentiate 'client->pers.maxHealth' from 'STAT_MAX_HEALTH',
	//       possibly change maxHealth to damageScale or something
	//
	//       the new handicap will either increase or decrease damage dealt
	//
	//       handicap will be a votable item
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 0 ) {
		client->pers.maxHealth = 0;
	} else if ( client->pers.maxHealth > 666 ) {
		client->pers.maxHealth = 666;
	}
	/*health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	client->pers.maxHealth = health;
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}*/
	client->ps.stats[STAT_MAX_HEALTH] = 100;

	// set model
	if( g_gametype.integer >= GT_TEAM ) {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "team_model"), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey (userinfo, "team_headmodel"), sizeof( headModel ) );
	} else {
		Q_strncpyz( model, Info_ValueForKey (userinfo, "model"), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey (userinfo, "headmodel"), sizeof( headModel ) );
	}

	// bots set their team a few frames later
	if (g_gametype.integer >= GT_TEAM && g_entities[clientNum].r.svFlags & SVF_BOT) {
		s = Info_ValueForKey( userinfo, "team" );
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}
	}
	else {
		team = client->sess.sessionTeam;
	}

/*	NOTE: all client side now

	// team
	switch( team ) {
		case TEAM_RED:
			ForceClientSkin(client, model, "red");
//			ForceClientSkin(client, headModel, "red");
			break;
		case TEAM_BLUE:
			ForceClientSkin(client, model, "blue");
//			ForceClientSkin(client, headModel, "blue");
			break;
	}
	// don't ever use a default skin in teamplay, it would just waste memory
	// however bots will always join a team but they spawn in as spectator
	if ( g_gametype.integer >= GT_TEAM && team == TEAM_SPECTATOR) {
		ForceClientSkin(client, model, "red");
//		ForceClientSkin(client, headModel, "red");
	}
*/

#ifdef MISSIONPACK
	if (g_gametype.integer >= GT_TEAM) {
		client->pers.teamInfo = qtrue;
	} else {
		s = Info_ValueForKey( userinfo, "teamoverlay" );
		if ( ! *s || atoi( s ) != 0 ) {
			client->pers.teamInfo = qtrue;
		} else {
			client->pers.teamInfo = qfalse;
		}
	}
#else
	// teamInfo
	s = Info_ValueForKey( userinfo, "teamoverlay" );
	if ( ! *s || atoi( s ) != 0 ) {
		client->pers.teamInfo = qtrue;
	} else {
		client->pers.teamInfo = qfalse;
	}
#endif
	/*
	s = Info_ValueForKey( userinfo, "cg_pmove_fixed" );
	if ( !*s || atoi( s ) == 0 ) {
		client->pers.pmoveFixed = qfalse;
	}
	else {
		client->pers.pmoveFixed = qtrue;
	}
	*/

	// team task (0 = none, 1 = offence, 2 = defence)
	teamTask = atoi(Info_ValueForKey(userinfo, "teamtask"));
	// team Leader (1 = leader, 0 is normal player)
	teamLeader = client->sess.teamLeader;

	// mmp - color info is not used in MFA, MFA will also create another way of suppling team names

	// colors
	strcpy(c1, Info_ValueForKey( userinfo, "color1" ));
	strcpy(c2, Info_ValueForKey( userinfo, "color2" ));
	strcpy(c3, Info_ValueForKey( userinfo, "color3" ));

	/*
	// team arena team names
	strcpy(redTeam, Info_ValueForKey( userinfo, "g_redteam" ));
	strcpy(blueTeam, Info_ValueForKey( userinfo, "g_blueteam" ));
	*/

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	if (ent->r.svFlags & SVF_BOT)
	{
		if ( g_demonstrationMode.integer & 1 ) {
			s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\hc\\666\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d\\c1\\%i\\c2\\%i\\c3\\%i",
				client->pers.netname, team, model, headModel,
				client->sess.wins, client->sess.losses,
				Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader,
				random() * 16, random() * 16, random() * 16 );
				// note: there's no need to check for out of bounds ranges since it's checked in CG_setColor of cg_players

				//G_Printf( "^3COLORS %i, %i, %i\n", c1, c2, c3 ); // debug
		} else {
			s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d\\c1\\%s\\c2\\%s\\c3\\%s",
				client->pers.netname, team, model, headModel,
				client->pers.maxHealth, client->sess.wins, client->sess.losses,
				Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader,
				c1, c2, c3 );
		}

/*		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d",
			client->pers.netname, team, model, headModel, c1, c2,
			client->pers.maxHealth, client->sess.wins, client->sess.losses,
			Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader );*/
	}
	else
	{
		s = va("n\\%s\\c\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d\\c1\\%s\\c2\\%s\\c3\\%s",
			client->pers.netname, client->pers.clan, client->sess.sessionTeam, model, headModel,
			client->pers.maxHealth, client->sess.wins, client->sess.losses, teamTask, teamLeader,
			c1, c2, c3 );
/*		s = va("n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d",
			client->pers.netname, client->sess.sessionTeam, model, headModel, redTeam, blueTeam, c1, c2,
			client->pers.maxHealth, client->sess.wins, client->sess.losses, teamTask, teamLeader);*/
	}

	trap_SetConfigstring( CS_PLAYERS+clientNum, s );

	// this is not the userinfo, more like the configstring actually
	G_LogPrintf( "ClientUserinfoChanged: No. %i %s\n", clientNum, s );

	if ( client->pers.connected == CON_CONNECTED ) {
		level.updateRuleset = qtrue;
	}

}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournament restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournament
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char		*value;
	char		*ipadd;
//	char		*areabits;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	gentity_t	*ent;

	ent = &g_entities[ clientNum ];

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

 	/*value = Info_ValueForKey( userinfo, "ip" );
 	Q_strncpyz( client->pers.ip, value, sizeof( client->pers.ip ) ); <-- incorrect use? */

 	// IP filtering
 	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
	// TODO: web address doesn't seem valid anymore ^
 	// recommanding PB based IP / GUID banning, the builtin system is pretty limited
	// NOTE: PB will not work with mfarena
	//
 	// check to see if they are on the banned IP list
	ipadd = Info_ValueForKey (userinfo, "ip");
 	//Q_strncpyz( client->pers.ip, ipadd, sizeof( client->pers.ip ) ); // FIXME: make check to prevent server crash from possible null string
	if ( G_FilterPacket( ipadd ) ) {
		//return "You are banned from this server.";

		//G_LogPrintf("ClientBlocked: No.%i - (%s) %s\n", clientNum, ipadd, client->pers.netname );
		G_LogPrintf("ClientBlocked: No. %i - (%s)\n", clientNum, ipadd );

		// TODO: keep from spamming server with blacklisted messages
		/*trap_SendServerCommand( -1, va("notify %i\\\"%s" S_COLOR_WHITE " attempted to connect, but is using an IP address that's blacklisted\n\"", NF_CONNECTION, client->pers.netname) );*/

		return "Your current IP address is blacklisted on this server, sorry.";
	}

	// we don't check password for bots and local client
	// NOTE: local client <-> "ip" "localhost"
	//   this means this client is not running in our current process
	if ( !isBot && (strcmp(ipadd, "localhost") != 0)) {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			strcmp( g_password.string, value) != 0) {
			if ( strlen(value) ) {
				return "Invalid password";
			} else {
				return "Password required";
			}
		}
	}
	// if a player reconnects quickly after a disconnect, the client disconnect may never be called, thus flag can get lost in the ether
	if (ent->inuse) {
		G_LogPrintf("Forcing disconnect on active client: %i\n", clientNum);
		// so lets just fix up anything that should happen on a disconnect
		ClientDisconnect(clientNum);
	}
	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

//	areabits = client->areabits;

	memset( client, 0, sizeof(*client) );

	client->pers.connected = CON_CONNECTING;

	// read or initialize the session data
	if ( firstTime || level.newSession ) {
		G_InitSessionData( client, userinfo );
	}
	G_ReadSessionData( client );

	if( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
	}

	if (isBot) {
		G_LogPrintf("ClientConnect: No. %i - (BOT)\n", clientNum );
	} else {
		G_LogPrintf("ClientConnect: No. %i - (%s)\n", clientNum, ipadd );
	}
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
//		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname) );
		trap_SendServerCommand( -1, va("notify %i\\\"%s" S_COLOR_WHITE " connected\n\"", NF_CONNECTION, client->pers.netname) );
		trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_CONNECT ) );
	}

	if ( g_gametype.integer >= GT_TEAM &&
		client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BroadcastTeamChange( client, -1 );
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	// for statistics
//	client->areabits = areabits;
//	if ( !client->areabits )
//		client->areabits = G_Alloc( (trap_AAS_PointReachabilityAreaIndex( NULL ) + 7) / 8 );

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	gentity_t	*ent;
	gclient_t	*client;
	int		flags;
	int		miscFlags;
	int		activePlayers, totalClients;
	int		i;

	ent = g_entities + clientNum;

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;

	// clear player stats
	client->pers.kills = 0;
	client->pers.teamKills = 0;
	client->pers.deaths = 0;
	client->pers.suicides = 0;
	client->pers.captures = 0;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;

	// locate ent at a spawn point
	ClientSpawn( ent );

	// mmp - update misc info
	//       all of this use to be called 'g_physicsMode'
	if (g_proMode.integer == 1) {
		// promode physics
		client->ps.persistant[PERS_MISC] &= (~PMSC_RESTRICTED_PHYSICS);
		// bots don't like vq1 physics, so switch to vq3
		if (ent->r.svFlags & SVF_BOT) {
			client->ps.persistant[PERS_MISC] |= PMSC_PHYSICS_SELECTION;
		}
	} else {
		// restricted physics
		client->ps.persistant[PERS_MISC] |= PMSC_RESTRICTED_PHYSICS;
		client->ps.persistant[PERS_MISC] &= (~PMSC_PHYSICS_SELECTION);
	}

	// weapon mode client flags
	if ( level.rs_matchMode > MM_PICKUP_ONCE ) {
		if ( level.rs_matchMode >= MM_ALLWEAPONS_MAXAMMO ) {
			client->ps.persistant[PERS_MISC] |= PMSC_NEVER_PICKUP_WEAPONS;
			if ( level.rs_matchMode == MM_ALLWEAPONS_MAXAMMO ) {
				client->ps.persistant[PERS_MISC] |= PMSC_NEVER_PICKUP_AMMO;
			} else {
				client->ps.persistant[PERS_MISC] &= (~PMSC_NEVER_PICKUP_AMMO);
			}
		} else {
			client->ps.persistant[PERS_MISC] |= PMSC_ALWAYS_PICKUP_WEAPONS;
			client->ps.persistant[PERS_MISC] &= (~PMSC_NEVER_PICKUP_WEAPONS);
			client->ps.persistant[PERS_MISC] &= (~PMSC_NEVER_PICKUP_AMMO);
		}
	} else {
		client->ps.persistant[PERS_MISC] &= (~PMSC_ALWAYS_PICKUP_WEAPONS);
		client->ps.persistant[PERS_MISC] &= (~PMSC_NEVER_PICKUP_WEAPONS);
		client->ps.persistant[PERS_MISC] &= (~PMSC_NEVER_PICKUP_AMMO);
	}

	// enemies in AA1 cannot pickup items
	if ( g_gametype.integer == GT_AA1 && client->sess.sessionTeam == TEAM_RED ) {
		client->ps.persistant[PERS_MISC] |= PMSC_NEVER_PICKUP_ANY_ITEM | PMSC_SLOW_RELOAD;
	} else {
		client->ps.persistant[PERS_MISC] &= (~PMSC_NEVER_PICKUP_ANY_ITEM);
		client->ps.persistant[PERS_MISC] &= (~PMSC_SLOW_RELOAD);
	}

	/*if ( client->sess.sessionTeam == TEAM_FREE ) {
		trap_SendServerCommand( -1, va("notify %i\\\"%s" S_COLOR_WHITE " entered the game\n\"", NF_GAMEINFO, client->pers.netname) );
	} else {
		trap_SendServerCommand( -1, va("notify %i\\\"%s" S_COLOR_WHITE " is spectating\n\"", NF_GAMEINFO, client->pers.netname) );
	}*/

	// TODO: prevent spectating message, if client is a bot
	switch (client->sess.sessionTeam) {
		case TEAM_SPECTATOR:
			trap_SendServerCommand( -1,
				va("notify %i\\\"%s" S_COLOR_WHITE " is spectating\n\"",
				NF_GAMEINFO, client->pers.netname) );
			break;
		case TEAM_RED:
			trap_SendServerCommand( -1,
				va("notify %i\\\"%s" S_COLOR_WHITE " joins the " S_COLOR_RED "RED" S_COLOR_WHITE " team\n\"",
				NF_GAMEINFO, client->pers.netname) );
			break;
		case TEAM_BLUE:
			trap_SendServerCommand( -1,
				va("notify %i\\\"%s" S_COLOR_WHITE " joins the " S_COLOR_BLUE "BLUE" S_COLOR_WHITE " team\n\"",
				NF_GAMEINFO, client->pers.netname) );
			break;
		default:
			trap_SendServerCommand( -1,
				va("notify %i\\\"%s" S_COLOR_WHITE " enters the game\n\"",
				NF_GAMEINFO, client->pers.netname) );
			break;
	}
	G_LogPrintf( "ClientBegin: No. %i\n", clientNum );
	/*if( ent->r.svFlags & SVF_BOT ) {
		G_LogPrintf("ClientBegin: No.%i - %s\n", clientNum, client->pers.netname );
	} else {
		G_LogPrintf("ClientBegin: No.%i - (%s) %s\n", clientNum, client->pers.ip, client->pers.netname );
	}*/

	// update warmup ready status
	SendReadymask( ent-g_entities, 1 );

	// send ruleset info to client
	/*trap_SendServerCommand( -1, va("ruleSet %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
					g_ruleset.integer,
					level.rs_timelimit, level.rs_overtime, level.rs_scorelimit, g_proMode.integer, level.rs_friendlyFire,
					level.rs_weaponRespawn, level.rs_forceRespawn, level.rs_teamLocOverlay, level.rs_hitSound, level.rs_scoreBalance,
					level.rs_quadMode, level.rs_selfDamage, level.rs_doubleAmmo, level.rs_keycardRespawn, level.rs_keycardDropable,
					level.rs_randomSpawn
					) );*/

	/*trap_SendServerCommand( ent-g_entities, va("ruleSet %i %i %i %i %i",
						level.rs_timelimit, level.rs_overtime, level.rs_scorelimit,
						level.rs_teamLocOverlay, level.rs_hitSound ) );*/

	// send overtime and round report, for new or transitioning client
	trap_SetConfigstring( CS_OVERTIME, va("%i", level.overtime ) );
	trap_SetConfigstring( CS_ROUND, va("%i", level.currentRound ) );

	// count current clients and rank for scoreboard
	CalculateRanks();

	// update client count
	G_ClientCount();

	// update team count
	if ( g_gametype.integer >= GT_TEAM ) {
		G_TeamCount();
	}

	// TODO: remove this recount, since it was just done about 7 lines ago
	activePlayers = totalClients = 0;
	/*humanPlayers = trap_Cvar_VariableIntegerValue( "sv_realHumanPlayers" );*/
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) {
			if (!(g_entities[i].r.svFlags & SVF_BOT) ) {
				if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) {
					activePlayers ++;
				}
				totalClients ++;
			}
		}
	}

	// update bookkeeping max player amount
	if (activePlayers > level.maxPlayersAchieved) {
		level.maxPlayersAchieved = activePlayers;
	}
	if (totalClients > level.maxClientsAchieved) {
		level.maxClientsAchieved = totalClients;
	}

}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t *ent) {
	int				index;
	vec3_t			spawn_origin, spawn_angles;
	gclient_t		*client;
	int				i, r;
	int				healthSet;
	int				rnd;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int				persistant[MAX_PERSISTANT];
	gentity_t		*spawnPoint;
	gentity_t 		*tent;
	int				flags;
	int				savedPing;
//	char			*savedAreaBits;
	int				accuracy_hits, accuracy_shots;
	int				eventSequence;
	char			userinfo[MAX_INFO_STRING];

	int				miscFlags;

	index = ent - g_entities;
	client = ent->client;

	VectorClear(spawn_origin);

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		spawnPoint = SelectSpectatorSpawnPoint (
						spawn_origin, spawn_angles);
	} else if (g_gametype.integer >= GT_CTF && g_gametype.integer != GT_AA1 ) {
		// all base oriented team games use the CTF spawn points
		spawnPoint = SelectCTFSpawnPoint (
						client->sess.sessionTeam,
						client->pers.teamState.state,
						spawn_origin, spawn_angles,
						!!(ent->r.svFlags & SVF_BOT));
	}
	else
	{
		// the first spawn should be at a good looking spot
		if ( !client->pers.initialSpawn && client->pers.localClient )
		{
			client->pers.initialSpawn = qtrue;
			spawnPoint = SelectInitialSpawnPoint(spawn_origin, spawn_angles,
							     !!(ent->r.svFlags & SVF_BOT));
		}
		else
		{
			// don't spawn near existing origin if possible
			spawnPoint = SelectSpawnPoint (
				client->ps.origin,
				spawn_origin, spawn_angles, !!(ent->r.svFlags & SVF_BOT));
		}
	}
	client->pers.teamState.state = TEAM_ACTIVE;

	// always clear the kamikaze flag
	ent->s.eFlags &= ~EF_KAMIKAZE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

//unlagged - backward reconciliation #3
	// we don't want players being backward-reconciled to the place they died
	G_ResetHistory( ent );
	// and this is as good a time as any to clear the saved state
	ent->client->saved.leveltime = 0;
//unlagged - backward reconciliation #3

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
//	savedAreaBits = client->areabits;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}
	eventSequence = client->ps.eventSequence;

	Com_Memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;
	client->lastBlasterShot = level.time; // reset blaster charge

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	client->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	trap_GetUserinfo( index, userinfo, sizeof(userinfo) );
	// set handicap
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 0 ) {
		client->pers.maxHealth = 0;
	} else if ( client->pers.maxHealth > 666 ) {
		client->pers.maxHealth = 666;
	}

	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = 100 /*client->pers.maxHealth*/;
	client->ps.eFlags = flags;

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;

	VectorCopy (playerMins, ent->r.mins);
	VectorCopy (playerMaxs, ent->r.maxs);

	client->ps.clientNum = index;

	// mmp - update misc info
	if (g_proMode.integer == 1) {
		// promode physics
		client->ps.persistant[PERS_MISC] = client->ps.persistant[PERS_MISC] & (~PMSC_RESTRICTED_PHYSICS);
//		client->ps.persistant[PERS_MISC] ~= PMSC_RESTRICTED_PHYSICS;
	} else {
		// restricted physics
		client->ps.persistant[PERS_MISC] |= PMSC_RESTRICTED_PHYSICS;
	}

	// set up default weapon
	client->ps.stats[STAT_WEAPONS] = ( 1 << WP_BLASTER );
	client->ps.ammo[AT_INFINITY] = -1; // infinity ammo should be infinity

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {

//		client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SUPER_SHOTGUN ); // test
//		client->ps.ammo[AT_SHELLS] = -1; // test

		if ( g_gametype.integer == GT_AA1 && client->sess.sessionTeam == TEAM_RED ) {

				// AA1 enemy (red), they start with some weapons, with infinite ammo

				// shotgun, random weapon and max ammo
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SHOTGUN );
				client->ps.ammo[AT_SHELLS] = -1;

				rnd = random() * 3;
				switch (rnd) {
					case 0:
						client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GRENADE_LAUNCHER );
						client->ps.ammo[AT_ROCKETS] = -1;
						break;
					case 1:
						client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_ROCKET_LAUNCHER );
						client->ps.ammo[AT_ROCKETS] = -1;
						break;
					case 2:
					default:
						client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_PLASMAGUN );
						client->ps.ammo[AT_CELLS] = -1;
						break;
				}

				client->ps.stats[STAT_INVENTORY] |= 7; // start with all keycards

				// start health at 30
				healthSet = client->ps.stats[STAT_MAX_HEALTH] * .3;
				if ( healthSet < 1 ) {
					healthSet = 1; // just so they won't start with 0 health
				}
				ent->health = client->ps.stats[STAT_HEALTH] = healthSet;

				// no armor for enemies
				client->ps.stats[STAT_ARMOR] = 0;
				client->ps.stats[STAT_ARMORTIER] = 1; // armor_body protection

		} else {

			/*
			client->ps.powerups[PW_PENT] = level.time + 30000; // test pent
			client->ps.powerups[PW_QUAD] = level.time + 30000; // test quad
			*/

			// give player a fighting chance in AA1
			if ( g_gametype.integer == GT_AA1 ) {
				client->ps.powerups[PW_PENT] = level.time + 3000; // test pent
			}

			if ( level.rs_matchMode == MM_ALLWEAPONS_MAXAMMO ) {
				// all weapons + max ammo, but no major spam weapons including grenade launcher
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SHOTGUN );
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SUPER_SHOTGUN );
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_ROCKET_LAUNCHER );
				if (!level.rs_noArenaLightningGun) {
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_LIGHTNING );
				}
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_PLASMAGUN );
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_CHAINGUN );

				client->ps.ammo[AT_SHELLS] = -1;
				client->ps.ammo[AT_ROCKETS] = -1;
				client->ps.ammo[AT_CELLS] = -1;
				client->ps.ammo[AT_BULLETS] = -1;

				// start player with pent, since this mode can be spammy
				client->ps.powerups[PW_PENT] = level.time + 3000;

				// start health at 250
				ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] * 2.5;

				// start with tier 3 armor at 200
				client->ps.stats[STAT_ARMOR] = AR_TIER3MAXPOINT;
				client->ps.stats[STAT_ARMORTIER] = 3; // armor_body protection

				client->disallowItemPickUp = 1; // meant to delay pickup on items when spawning on them
			} else if ( level.rs_matchMode == MM_ALLWEAPONS ) {
				// all weapons + max ammo, but no major spam weapons including grenade launcher
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SHOTGUN );
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SUPER_SHOTGUN );
				if (!level.rs_noArenaGrenades) {
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GRENADE_LAUNCHER );
				}
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_ROCKET_LAUNCHER );
				if (!level.rs_noArenaLightningGun) {
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_LIGHTNING );
				}
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_PLASMAGUN );
				/*client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SPREADSHOT );*/
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_CHAINGUN );
				client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_FLAMETHROWER );

				if ( level.warmupTime ) {
					client->ps.ammo[AT_SHELLS] = -1;
					client->ps.ammo[AT_ROCKETS] = -1;
					client->ps.ammo[AT_CELLS] = -1;
					client->ps.ammo[AT_BULLETS] = -1;
					client->ps.ammo[AT_GAS] = -1;
				} else {
				// ammo amounts based on quake's deathmatch mode 5, with exceptions to the cell ammo
					client->ps.ammo[AT_SHELLS] = 30;
					client->ps.ammo[AT_ROCKETS] = 10;
					client->ps.ammo[AT_CELLS] = 120;
					client->ps.ammo[AT_BULLETS] = 80;
					client->ps.ammo[AT_GAS] = 100;
				}

				// start health at 200
				ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] * 2;

				// start wither tier 3 armor at 200
				client->ps.stats[STAT_ARMOR] = AR_TIER3MAXPOINT;
				client->ps.stats[STAT_ARMORTIER] = 3; // armor_body protection

				client->disallowItemPickUp = 1; // meant to delay pickup on items when spawning on them
			} else if ( level.rs_matchMode == MM_ROCKET_MANIAX ) {
				// rockets maniax arena
				client->ps.stats[STAT_WEAPONS] = ( 1 << WP_ROCKET_LAUNCHER ); // make sure to remove the blaster from inventory as well

				client->ps.ammo[AT_INFINITY] = 0;
				client->ps.ammo[AT_SHELLS] = 0;
				client->ps.ammo[AT_ROCKETS] = -1;
				client->ps.ammo[AT_CELLS] = 0;
				client->ps.ammo[AT_BULLETS] = 0;
				client->ps.ammo[AT_GAS] = 0;

				// start health at 100
				ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];

				// start with no armor
				client->ps.stats[STAT_ARMOR] = 0;
				client->ps.stats[STAT_ARMORTIER] = 1; // armor_body protection

				client->disallowItemPickUp = 1; // meant to delay pickup on items when spawning on them
			} else if ( level.rs_matchMode == MM_RANDOM_LOADOUTS ) {
				// start health at 100
				ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];

				if ( level.warmupTime ) {
					// TODO: code the following better
					client->ps.stats[STAT_WEAPONS] = ( 1 << WP_SHOTGUN );
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SUPER_SHOTGUN );
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GRENADE_LAUNCHER );
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_ROCKET_LAUNCHER );
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_LIGHTNING );
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_PLASMAGUN );
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_CHAINGUN );
					client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_FLAMETHROWER );
				} else {
					client->ps.stats[STAT_WEAPONS] = level.randomLoadOuts; // give random loadouts
				}

				// give infinite ammo for a flashier demonstration
				client->ps.ammo[AT_SHELLS] = -1;
				client->ps.ammo[AT_ROCKETS] = -1;
				client->ps.ammo[AT_CELLS] = -1;
				client->ps.ammo[AT_BULLETS] = -1;
				client->ps.ammo[AT_GAS] = -1;
			} else {
				// start health at 100
				ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];

				// demostration mode, not used in normal play
				if ( g_demonstrationMode.integer & 2 ) {
					r = (random() * 7) + 2;
					client->ps.stats[STAT_WEAPONS] = 1 << r; // give random starting weapon

					// give infinite ammo for a flashier demonstration
					client->ps.ammo[AT_SHELLS] = -1;
					client->ps.ammo[AT_ROCKETS] = -1;
					client->ps.ammo[AT_CELLS] = -1;
					client->ps.ammo[AT_BULLETS] = -1;
					client->ps.ammo[AT_GAS] = -1;
				}
			}
		}
	} else {
		// start health at 100
		ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];
	}

	// time until health starts to decay
	client->healthDecayTime = level.time + HEALTH_DECAY_TIME * 1000;

	// reference
	/*if ( level.warmupTime )
		client->ps.ammo[WP_MACHINEGUN] = -1; // don't care about ammo in warmup
	else
		client->ps.ammo[WP_MACHINEGUN] = 100;
	*/

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
	SetClientViewAngle( ent, spawn_angles );
	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	// set default animations
	client->ps.torsoAnim = TORSO_STAND;
	client->ps.legsAnim = LEGS_IDLE;

	if (!level.intermissiontime) {
		if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
			G_KillBox(ent);
			// force the base weapon up
			client->ps.weapon = WP_GAUNTLET;
			client->ps.weaponstate = WEAPON_READY;
			// fire the targets of the spawn point
			G_UseTargets(spawnPoint, ent);
			// select the highest weapon number available, after any spawn given items have fired
			client->ps.weapon = 1;

			for (i = WP_NUM_WEAPONS - 1 ; i > 0 ; i--) {
				if (client->ps.stats[STAT_WEAPONS] & (1 << i)) {
					client->ps.weapon = i;
					break;
				}
			}
			// positively link the client, even if the command times are weird
			VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);

			tent = G_TempEntity(ent->client->ps.origin, EV_PLAYER_TELEPORT_IN);
			tent->s.clientNum = ent->s.clientNum;

			trap_LinkEntity (ent);
		}
	} else {
		// move players to intermission
		MoveClientToIntermission(ent);
	}
	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities );
	// run the presend to set anything else, follow spectators wait
	// until all clients have been reconnected after map_restart
	if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		ClientEndFrame( ent );
	}

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;
	gentity_t	*tent;
	int			i;

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin( clientNum );

	ent = g_entities + clientNum;
	if (!ent->client || ent->client->pers.connected == CON_DISCONNECTED) {
		return;
	}

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}
	}

	// update warmup ready status
	ent->client->pers.ready = qfalse;
	SendReadymask( -1, 1 );

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );
#ifdef MISSIONPACK
		TossClientPersistantPowerups( ent );
		if( g_gametype.integer == GT_HARVESTER ) {
			TossClientCubes( ent );
		}
#endif

	}

	G_LogPrintf( "ClientDisconnect: No. %i\n", clientNum );
	/*trap_SendServerCommand( -1, va("notify %i\\\"%s" S_COLOR_WHITE " dropped\n\"", NF_CONNECTION, ent->client->pers.netname) );*/
	trap_SendServerCommand( -1, va("sndCall \"%i\"", SC_DISCONNECT ) );

	// if we are playing in tourney mode and losing, give a win to the other player
	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& !level.intermissiontime
		&& !level.warmupTime && level.sortedClients[1] == clientNum ) {
		level.clients[ level.sortedClients[0] ].sess.wins++;
		ClientUserinfoChanged( level.sortedClients[0] );
	}

	if( g_gametype.integer == GT_TOURNAMENT &&
		ent->client->sess.sessionTeam == TEAM_FREE &&
		level.intermissiontime ) {

		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;
		level.changemap = NULL;
		level.intermissiontime = 0;
	}

	trap_UnlinkEntity (ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;

	trap_SetConfigstring( CS_PLAYERS + clientNum, "");

	// count current clients and rank for scoreboard
	CalculateRanks();

	// update client count
	G_ClientCount();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum, qfalse );
	}
}


