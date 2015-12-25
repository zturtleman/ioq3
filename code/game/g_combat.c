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
// g_combat.c

#include "g_local.h"


/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_SCOREPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
============
NumPlum
============
*/
void NumPlum( gentity_t *ent, vec3_t origin, int num ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_NUMPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = num;
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, vec3_t origin, int score ) {
	int		counts[TEAM_NUM_TEAMS];
	int		roundedScore;

	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if ( level.warmupTime ) {
		return;
	}

	// no scoring after the match has ended
	if ( /*level.intermissiontime*/ /*level.matchEnded == qtrue*/ level.intermissionQueued ) {
		return;
	}

	// show score plum
	ScorePlum(ent, origin, score);
	//
	ent->client->ps.persistant[PERS_SCORE] += score;
	if ( g_gametype.integer == GT_TEAM || ( g_gametype.integer == GT_CTF && !level.rs_popCTF ) ) {

		// TODO - prevent the need to count the teams everytime a team scores
		counts[TEAM_BLUE] = TeamCount( -1, TEAM_BLUE );
		counts[TEAM_RED] = TeamCount( -1, TEAM_RED );

		// update the score based on balance
		if ( (!level.rs_scoreBalance) || counts[TEAM_RED] == counts[TEAM_BLUE] ||
						counts[TEAM_RED] == 0 || counts[TEAM_BLUE] == 0 || score < 0 ) {
			level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
		} else if (ent->client->ps.persistant[PERS_TEAM] == TEAM_RED) {
			if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
				level.teamSubScores[ TEAM_RED ] += (float)score * ( (float)counts[TEAM_BLUE] / (float)counts[TEAM_RED] );
				if ( level.teamSubScores[ TEAM_RED ] >= 1 ) {
					roundedScore = floor ((float)level.teamSubScores[ TEAM_RED ]);
					level.teamSubScores[ TEAM_RED ] -= (float)roundedScore;
					level.teamScores[ TEAM_RED ] += roundedScore;
				}
			} else {
				level.teamScores[ TEAM_RED ] += score;
			}
		} else {
			if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
				level.teamSubScores[ TEAM_BLUE ] += (float)score * ( (float)counts[TEAM_RED] / (float)counts[TEAM_BLUE] );
				if ( level.teamSubScores[ TEAM_BLUE ] >= 1 ) {
					//G_Printf( "DEBUG: BLUE SUB=%f\n", level.teamSubScores[ TEAM_BLUE] ); // debug
					roundedScore = floor ((float)level.teamSubScores[ TEAM_BLUE ]);
					level.teamSubScores[ TEAM_BLUE ] -= (float)roundedScore;
					level.teamScores[ TEAM_BLUE ] += roundedScore;
					//G_Printf( "DEBUG: BLUE SUB AFTER=%f\n", level.teamSubScores[ TEAM_BLUE] ); // debug
				}
			} else {
				level.teamScores[ TEAM_BLUE ] += score;
			}
		}
	} else
	// team points are not use the same way as in TDM
	if ( g_gametype.integer == GT_AA1 ) {
		if ( ent->client->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			if ( score > 0 ) {
				level.teamScores[ TEAM_BLUE ] += score;
			} else {
				// target dies, and is eliminated
				level.teamScores[ TEAM_RED ] -= score;
			}
		} else
		if ( score > 0 ) {
			// target dies, and is eliminated
			level.teamScores[ TEAM_RED ] += score;
		}
	}

	CalculateRanks();
}

/*
============
TimeBonus
============
*/
int TimeBonus( void ) {
	int		timeBonus;

	// determine the time point bonus for the attacker
	timeBonus = (level.time - level.startTime) / 1000; // base by seconds
	if ( timeBonus < 30 ) {
		return 100;
	}
	if ( timeBonus < 60 ) {
		return 50;
	}
	if ( timeBonus < 90 ) {
		return 40;
	}
	if ( timeBonus < 120 ) {
		return 30;
	}
	if ( timeBonus < 180 ) {
		return 25;
	}
	if ( timeBonus < 240 ) {
		return 20;
	}
	if ( timeBonus < 300 ) {
		return 15;
	}
	return 10;

}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	int			weapon;
	float		angle;
	int			i;
	gentity_t	*drop;
	int			teamFlag;
	//int			testValue;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.
	if ( weapon == WP_GRAPPLING_HOOK || weapon == WP_BLASTER ) {
		if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
			weapon = self->client->pers.cmd.weapon;
		}
		if ( !( self->client->ps.stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
			weapon = WP_NONE;
		}
	}

/*	if ( weapon >= WP_MACHINEGUN && weapon != WP_GRAPPLING_HOOK &&
		self->client->ps.ammo[ weapon ] ) {

		// find the item type for the backpack
		item = BG_FindItemForBackpack();

		// spawn the backpack
		Drop_Item( self, item, 0 );
	}*/

	// randomly set angle
	angle = random() * 360;

	// toss backpack containing ammo and weapon
	if ( !level.warmupTime ) {

		// find the item type for the backpack
		item = BG_FindItemForBackpack();

		// check just incase someone fucked something up
		if ( item ) {
			// spawn the backpack
			// Drop_Item( gentity_t *ent, gitem_t *item, float angle )
			drop = Drop_Item( self, item, angle );

			// remember slot
			drop->count = level.currentBackpackSlot;

			// add ammo and current weapon in backpack
			// TODO: code this better
			backpack[level.currentBackpackSlot].lastWeap = weapon;
			backpack[level.currentBackpackSlot].shells = self->client->ps.ammo[ AT_SHELLS ];
			backpack[level.currentBackpackSlot].rockets = self->client->ps.ammo[ AT_ROCKETS ];
			backpack[level.currentBackpackSlot].cells = self->client->ps.ammo[ AT_CELLS ];
			backpack[level.currentBackpackSlot].bullets = self->client->ps.ammo[ AT_BULLETS ];
			backpack[level.currentBackpackSlot].gas = self->client->ps.ammo[ AT_GAS ];
			//G_Printf ("^3DEBUG = '%i'\n", backpack[level.currentBackpackSlot].shells ); // debug
			//G_Printf ("^3DEBUG DROP = '%i'\n", level.currentBackpackSlot); // debug

			// move to next backpack slot
			level.currentBackpackSlot++;
			if ( level.currentBackpackSlot >= MAX_BACKPACK_CONTENTS ) {
				level.currentBackpackSlot = 0;
			}
		}

	}

	// toss weapon that's not a blaster, and has ammo
	/*if ( weapon != WP_GRAPPLING_HOOK && weapon != WP_BLASTER &&
		self->client->ps.ammo[ ammoGroup[weapon].ammo ] > 0 ) {

		//G_Printf ("^3DEBUG = '%i'\n", self->client->ps.ammo[ ammoGroup[weapon].ammo ]); // debug

		// find the item type for this weapon
		item = BG_FindItemForWeapon( weapon );

		// spawn the item
		Drop_Item( self, item, 0 );
	}*/

	// if ctf, check if victom is holding the flag
	if ( g_gametype.integer == GT_CTF ) {
		if ( self->client->ps.powerups[PW_REDFLAG] ) {
			item = BG_FindItemForPowerup( PW_REDFLAG );
			if ( item ) {
				angle += 45;
				drop = Drop_Item( self, item, angle );
				drop->count = ( self->client->ps.powerups[ PW_REDFLAG ] - level.time ) / 1000;
				if ( drop->count < 5 ) {
					drop->count = 5;
				}
			}
		} else
		if ( self->client->ps.powerups[PW_BLUEFLAG] ) {
			item = BG_FindItemForPowerup( PW_BLUEFLAG );
			if ( item ) {
				angle += 45;
				drop = Drop_Item( self, item, angle );
				drop->count = ( self->client->ps.powerups[ PW_BLUEFLAG ] - level.time ) / 1000;
				if ( drop->count < 5 ) {
					drop->count = 5;
				}
			}
		}
	}

	// drop keycards if enabled
	if ( level.rs_keycardDropable && !(g_gametype.integer == GT_AA1 && self->client->sess.sessionTeam == TEAM_RED )) {
		// TODO: make this look better

		if ( self->client->ps.stats[STAT_INVENTORY] & 1 ) {
			item = BG_FindItemForKeycards( INV_KCARD_BLUE );
			if ( item ) {
				angle += 45;
				drop = Drop_Item( self, item, angle );
			}
		}

		if ( self->client->ps.stats[STAT_INVENTORY] & 2 ) {
			item = BG_FindItemForKeycards( INV_KCARD_RED );
			if ( item ) {
				angle += 45;
				drop = Drop_Item( self, item, angle );
			}
		}

		if ( self->client->ps.stats[STAT_INVENTORY] & 4 ) {
			//testValue = INV_KCARD_YELLOW;
			item = BG_FindItemForKeycards( INV_KCARD_YELLOW );
			if ( item ) {
				angle += 45;
				drop = Drop_Item( self, item, angle );
			}
		}

	}

	// drop all the powerups if not in teamplay
	/*if ( g_gametype.integer != GT_TEAM ) {
		angle = 45;
		for ( i = 1 ; i < PW_NUM_POWERUPS ; i++ ) {
			if ( self->client->ps.powerups[ i ] > level.time ) {
				item = BG_FindItemForPowerup( i );
				if ( !item ) {
					continue;
				}
				drop = Drop_Item( self, item, angle );
				// decide how many seconds it has left
				drop->count = ( self->client->ps.powerups[ i ] - level.time ) / 1000;
				if ( drop->count < 1 ) {
					drop->count = 1;
				}
				angle += 45;
			}
		}
	}*/

}

#ifdef MISSIONPACK

/*
=================
TossClientCubes
=================
*/
extern gentity_t	*neutralObelisk;

void TossClientCubes( gentity_t *self ) {
	gitem_t		*item;
	gentity_t	*drop;
	vec3_t		velocity;
	vec3_t		angles;
	vec3_t		origin;

	self->client->ps.generic1 = 0;

	// this should never happen but we should never
	// get the server to crash due to skull being spawned in
	if (!G_EntitiesFree()) {
		return;
	}

	if( self->client->sess.sessionTeam == TEAM_RED ) {
		item = BG_FindItem( "Red Cube" );
	}
	else {
		item = BG_FindItem( "Blue Cube" );
	}

	angles[YAW] = (float)(level.time % 360);
	angles[PITCH] = 0;	// always forward
	angles[ROLL] = 0;

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;

	if( neutralObelisk ) {
		VectorCopy( neutralObelisk->s.pos.trBase, origin );
		origin[2] += 44;
	} else {
		VectorClear( origin ) ;
	}

	drop = LaunchItem( item, origin, velocity );

	drop->nextthink = level.time + g_cubeTimeout.integer * 1000;
	drop->think = G_FreeEntity;
	drop->spawnflags = self->client->sess.sessionTeam;
}


/*
=================
TossClientPersistantPowerups
=================
*/
void TossClientPersistantPowerups( gentity_t *ent ) {
	gentity_t	*powerup;

	if( !ent->client ) {
		return;
	}

	if( !ent->client->persistantPowerup ) {
		return;
	}

	powerup = ent->client->persistantPowerup;

	powerup->r.svFlags &= ~SVF_NOCLIENT;
	powerup->s.eFlags &= ~EF_NODRAW;
	powerup->r.contents = CONTENTS_TRIGGER;
	trap_LinkEntity( powerup );

	ent->client->ps.stats[STAT_PERSISTANT_POWERUP] = 0;
	ent->client->persistantPowerup = NULL;
}
#endif

/*
================
GetDamageLevel
================
*/

// FIXME: use a better method of making a rate table
float GetDamageLevel (gentity_t *ent) {
//	float		damageRate = 1.0;
	int		damageLevel;

//	floor ( ps->stats[STAT_LEVEL] / LV_SUBPOINT )

	damageLevel = floor ( ent->client->ps.stats[STAT_LEVEL] / LV_SUBPOINT );
	switch (damageLevel) {
		default:
			return LV_ONE;
		case 1:
			return LV_TWO;
		case 2:
			return LV_THREE;
		case 3:
			return LV_FOUR;
		case 4:
			return LV_FIVE;
	}
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
	gentity_t *ent;
	int i;

	//if this entity still has kamikaze
	if (self->s.eFlags & EF_KAMIKAZE) {
		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < MAX_GENTITIES; i++) {
			ent = &g_entities[i];
			if (!ent->inuse)
				continue;
			if (ent->activator != self)
				continue;
			if (strcmp(ent->classname, "kamikaze timer"))
				continue;
			G_FreeEntity(ent);
			break;
		}
	}
	G_AddEvent( self, EV_GIB_PLAYER, killer );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}
	if ( !g_blood.integer ) {
		self->health = GIB_HEALTH+1;
		return;
	}

	GibEntity( self, 0 );
}


// these are just for logging, the client prints its own messages
char	*modNames[] = {
	"MOD_UNKNOWN",
	"MOD_BLASTER",
	"MOD_SHOTGUN",
	"MOD_SUPER_SHOTGUN",
	"MOD_GAUNTLET",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_PLASMA",
	"MOD_PLASMA_SPLASH",
	"MOD_SPREADSHOT",
	"MOD_SPREADSHOT_SPLASH",
	"MOD_LIGHTNING",
	"MOD_LIGHTNING_DISCHARGE", // The SARACEN's Lightning Discharge
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_CHAINGUN",
#ifdef MISSIONPACK
	"MOD_KAMIKAZE",
	"MOD_JUICED",
#endif
	"MOD_GRAPPLE",
	"MOD_FLAMETHROWER",
	"MOD_NOTHING" // does not display a MOD log message
};

#ifdef MISSIONPACK
/*
==================
Kamikaze_DeathActivate
==================
*/
void Kamikaze_DeathActivate( gentity_t *ent ) {
	G_StartKamikaze(ent);
	G_FreeEntity(ent);
}

/*
==================
Kamikaze_DeathTimer
==================
*/
void Kamikaze_DeathTimer( gentity_t *self ) {
	gentity_t *ent;

	ent = G_Spawn();
	ent->classname = "kamikaze timer";
	VectorCopy(self->s.pos.trBase, ent->s.pos.trBase);
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->think = Kamikaze_DeathActivate;
	ent->nextthink = level.time + 5 * 1000;

	ent->activator = self;
}

#endif

/*
==================
CheckAlmostCapture
==================
*/
void CheckAlmostCapture( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if this player was carrying a flag
	if ( self->client->ps.powerups[PW_REDFLAG] ||
		self->client->ps.powerups[PW_BLUEFLAG] ||
		self->client->ps.powerups[PW_NEUTRALFLAG] ) {
		// get the goal flag this player should have been going for
		if ( g_gametype.integer == GT_CTF ) {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_blueflag";
			}
			else {
				classname = "team_CTF_redflag";
			}
		}
		else {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_redflag";
			}
			else {
				classname = "team_CTF_blueflag";
			}
		}
		ent = NULL;
		do
		{
			ent = G_Find(ent, FOFS(classname), classname);
		} while (ent && (ent->flags & FL_DROPPED_ITEM));
		// if we found the destination flag and it's not picked up
		if (ent && !(ent->r.svFlags & SVF_NOCLIENT) ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
==================
CheckAlmostScored
==================
*/
void CheckAlmostScored( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if the player was carrying cubes
	if ( self->client->ps.generic1 ) {
		if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
			classname = "team_redobelisk";
		}
		else {
			classname = "team_blueobelisk";
		}
		ent = G_Find(NULL, FOFS(classname), classname);
		// if we found the destination obelisk
		if ( ent ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
	int			anim;
	int			contents;
	int			killer;
	int			i;
	char		*killerName, *obit;
	int			comboCounterRank;

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// no scoring after the match has ended
	if ( /*level.intermissiontime*/ /*level.matchEnded == qtrue*/ level.intermissionQueued ) {
		return;
	}

	// check for an almost capture
	CheckAlmostCapture( self, attacker );
	// check for a player that almost brought in cubes
	CheckAlmostScored( self, attacker );

	if (self->client && self->client->hook) {
		Weapon_HookFree(self->client->hook);
	}
#ifdef MISSIONPACK
	if ((self->client->ps.eFlags & EF_TICKING) && self->activator) {
		self->client->ps.eFlags &= ~EF_TICKING;
		self->activator->think = G_FreeEntity;
		self->activator->nextthink = level.time;
	}
#endif
	self->client->ps.pm_type = PM_DEAD;

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( meansOfDeath < 0 || meansOfDeath >= ARRAY_LEN( modNames ) ) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[meansOfDeath];
	}

	G_VerboseLogPrintf(1, "Kill: %i %i %i: %s killed %s by %s\n",
		killer, self->s.number, meansOfDeath, killerName,
		self->client->pers.netname, obit );

	// broadcast the death event to everyone
	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = meansOfDeath;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone

	if ( g_gametype.integer >= GT_TEAM )
		ent->s.generic1 = OnSameTeam (self, attacker); // looks like generic can very well be generic
	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++;
	level.killTotal++; // increase global kill total

	if (attacker && attacker->client) {
		attacker->client->lastkilled_client = self->s.number;

		if ( attacker == self ) {
			// subtract 2 points on those who commit suicide, what pussies  ;)
			if ( meansOfDeath == MOD_SUICIDE ) {
				AddScore( attacker, self->r.currentOrigin, -2 );
				attacker->client->pers.suicides++; // add to stats
				attacker->client->pers.deaths++; // add to stats
				/*attacker->client->sess.demerits+=2; // add demerit*/
				UpdateDemerits ( attacker, 2 ); // add demerit
			}
			// MOD_NOTHING will not alter the score, since the MOD is not part of the game combat
			else if ( meansOfDeath != MOD_NOTHING ) {
				// otherwise, subtract a point for getting yourself killed
				AddScore( attacker, self->r.currentOrigin, -1 );
				attacker->client->pers.deaths++; // add to stats
			}
		} else if (OnSameTeam (self, attacker) ) {
			// subtract for killing teammate
			AddScore( attacker, self->r.currentOrigin, -1 );
			attacker->client->pers.teamKills++; // add to stats
			self->client->pers.deaths++; // add to target's stats
			//attacker->client->sess.demerits+=1; // add demerit
			UpdateDemerits ( attacker, 1 ); // add demerit
		} else {
			// we got a frag
			if ( g_gametype.integer != GT_AA1 ) {
				AddScore( attacker, self->r.currentOrigin, 1 );
			} else {
				if ( attacker->client->sess.sessionTeam == TEAM_BLUE ) {
					AddScore( attacker, self->r.currentOrigin, 1 );
				} else {
					AddScore( attacker, self->r.currentOrigin, TimeBonus() );
				}
			}
			attacker->client->pers.kills++; // add to stats
			self->client->pers.deaths++; // add to target's stats
			//attacker->client->sess.demerits-=1; // remove demerit
			UpdateDemerits ( attacker, -1 ); // remove demerit

			// kill streak
			attacker->client->curKillStreak++; // increase current kill streak
			// is this the highest kill streak
			if ( attacker->client->curKillStreak > attacker->client->pers.killStreak ) {
				// update highest kill streak
				attacker->client->pers.killStreak = attacker->client->curKillStreak;
			}
			if ( (attacker->client->curKillStreak % 5) == 0 ) {
				trap_SendServerCommand( -1, va("notify %i\\\"%s"S_COLOR_WHITE" has a killing streak of %i frags\n\"",
							NF_STREAKS, attacker->client->pers.netname, attacker->client->curKillStreak) );
			}

			// combo breaker
			if ( self->client->comboCounter > 1 ) {
				if ( level.time - self->client->lastKillTime < CARNAGE_REWARD_TIME ) {
					trap_SendServerCommand( attacker-g_entities, va("sndCall \"%i\"", SC_AN_COMBO_BREAKER ) );
				}
			}

			// specmode update
			level.sm_lastFrag = attacker->s.number;
			level.sm_lastFragTime  = level.time + 1000;

			if( meansOfDeath == MOD_GAUNTLET ) {

				// play humiliation on player
				attacker->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;

				// add the sprite over the player's head
				attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
				attacker->client->ps.eFlags |= EF_AWARD_GAUNTLET;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

				// also play humiliation on target
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
			}

			// check for two kills in a short amount of time
			// if this is close enough to the last kill, give a reward sound
			if ( level.time - attacker->client->lastKillTime < CARNAGE_REWARD_TIME ) {
				/*
				// play excellent on player
				attacker->client->ps.persistant[PERS_EXCELLENT_COUNT]++;

				// add the sprite over the player's head
				attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
				attacker->client->ps.eFlags |= EF_AWARD_EXCELLENT;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
				*/
				attacker->client->comboCounter++;
				if ( !(attacker->client->comboCounter & 1) ) {
					comboCounterRank = attacker->client->comboCounter / 2;
					if ( comboCounterRank > 8 ) {
						comboCounterRank = 8;
					}
					/*trap_SendServerCommand( attacker-g_entities, va("notify %i\\\"" S_COLOR_YELLOW "DEBUG: COMBO RANK=%i (%i)\n\"", NF_GAMEINFO, comboCounterRank, attacker->client->comboCounter) );*/
					trap_SendServerCommand( attacker-g_entities, va("sndCall \"%i\"", SC_AN_SHIT + comboCounterRank ) );
				}

			} else {
				attacker->client->comboCounter = 1; // you have at least 1 combo kill
			}
			attacker->client->lastKillTime = level.time;

		}
	} else {
		AddScore( self, self->r.currentOrigin, -1 );
		self->client->pers.deaths++; // add to stats
	}

	// Add team bonuses
	Team_FragBonuses(self, inflictor, attacker);

	// if I committed suicide, the flag does not fall, it returns.
	if (meansOfDeath == MOD_SUICIDE) {
		if ( self->client->ps.powerups[PW_NEUTRALFLAG] ) {		// only happens in One Flag CTF
			Team_ReturnFlag( TEAM_FREE );
			self->client->ps.powerups[PW_NEUTRALFLAG] = 0;
		}
		else if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
			self->client->ps.powerups[PW_REDFLAG] = 0;
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
			self->client->ps.powerups[PW_BLUEFLAG] = 0;
		}
	}

	TossClientItems( self );
#ifdef MISSIONPACK
	TossClientPersistantPowerups( self );
	if( g_gametype.integer == GT_HARVESTER ) {
		TossClientCubes( self );
	}
#endif

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed

	self->s.weapon = WP_NONE;
	self->s.powerups = 0;
	self->r.contents = CONTENTS_CORPSE;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	LookAtKiller (self, inflictor, attacker);

	VectorCopy( self->s.angles, self->client->ps.viewangles );

	self->s.loopSound = 0;

	self->r.maxs[2] = -8;

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 1700 + level.c_spawnDelay;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// never gib in a nodrop
	contents = trap_PointContents( self->r.currentOrigin, -1 );

	if ( (self->health <= GIB_HEALTH && !(contents & CONTENTS_NODROP) && g_blood.integer) || meansOfDeath == MOD_SUICIDE) {
		// gib death
		GibEntity( self, killer );
	} else {
		// normal death
		static int i;

		switch ( i ) {
		case 0:
			anim = BOTH_DEATH1;
			break;
		case 1:
			anim = BOTH_DEATH2;
			break;
		case 2:
		default:
			anim = BOTH_DEATH3;
			break;
		}

		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH+1;
		}

		self->client->ps.legsAnim =
			( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
		self->client->ps.torsoAnim =
			( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

		G_AddEvent( self, EV_DEATH1 + i, killer );

		// the body can still be gibbed
		self->die = body_die;

		// globally cycle through the different death animations
		i = ( i + 1 ) % 3;

#ifdef MISSIONPACK
		if (self->s.eFlags & EF_KAMIKAZE) {
			Kamikaze_DeathTimer( self );
		}
#endif
	}

	trap_LinkEntity (self);

}


/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			count;
	float			tierRate;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];

	// if tier type armor mode, then protect player depending on armor tier
	tierRate = (client->ps.stats[STAT_ARMORTIER] <= 2) ? ((client->ps.stats[STAT_ARMORTIER] == 1) ? AR_TIER1PROTECTION : AR_TIER2PROTECTION) : AR_TIER3PROTECTION;

	save = ceil( damage * tierRate ); // was ARMOR_PROTECTION
	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

/*
================
RaySphereIntersections
================
*/
int RaySphereIntersections( vec3_t origin, float radius, vec3_t point, vec3_t dir, vec3_t intersections[2] ) {
	float b, c, d, t;

	//	| origin - (point + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	//	c = (point[0] - origin[0])^2 + (point[1] - origin[1])^2 + (point[2] - origin[2])^2 - radius^2;

	// normalize dir so a = 1
	VectorNormalize(dir);
	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	c = (point[0] - origin[0]) * (point[0] - origin[0]) +
		(point[1] - origin[1]) * (point[1] - origin[1]) +
		(point[2] - origin[2]) * (point[2] - origin[2]) -
		radius * radius;

	d = b * b - 4 * c;
	if (d > 0) {
		t = (- b + sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[0]);
		t = (- b - sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[1]);
		return 2;
	}
	else if (d == 0) {
		t = (- b ) / 2;
		VectorMA(point, t, dir, intersections[0]);
		return 1;
	}
	return 0;
}

#ifdef MISSIONPACK
/*
================
G_InvulnerabilityEffect
================
*/
int G_InvulnerabilityEffect( gentity_t *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir ) {
	gentity_t	*impact;
	vec3_t		intersections[2], vec;
	int			n;

	if ( !targ->client ) {
		return qfalse;
	}
	VectorCopy(dir, vec);
	VectorInverse(vec);
	// sphere model radius = 42 units
	n = RaySphereIntersections( targ->client->ps.origin, 42, point, vec, intersections);
	if (n > 0) {
		impact = G_TempEntity( targ->client->ps.origin, EV_INVUL_IMPACT );
		VectorSubtract(intersections[0], targ->client->ps.origin, vec);
		vectoangles(vec, impact->s.angles);
		impact->s.angles[0] += 90;
		if (impact->s.angles[0] > 360)
			impact->s.angles[0] -= 360;
		if ( impactpoint ) {
			VectorCopy( intersections[0], impactpoint );
		}
		if ( bouncedir ) {
			VectorCopy( vec, bouncedir );
			VectorNormalize( bouncedir );
		}
		return qtrue;
	}
	else {
		return qfalse;
	}
}
#endif
/*
============
G_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	gclient_t	*attackingClient;
	int			take;
	int			asave;
	int			knockback;
	int			max;
	int			levelGain;
	int			lastHit;

	float		quadFactor;

	if (!targ->takedamage) {
		return;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) {
		return;
	}

	// mmp bugfix - added matchEnded check, since i was able to score a frag after the match has ended
	// seems the above code to check if intermission was queued is flawed
	if ( level.matchEnded == qtrue ) {
		return;
	}

	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// disable player damage when in ghost mode
	if ( targ->client->ps.pm_type == PM_GHOST || attacker->client->ps.pm_type == PM_GHOST ) {
		return;
	}

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) {
		// prevent doors and buttons from activating from splash damage
		// TODO: code this check better
		if ( targ->ignoresplash == qtrue && (mod == MOD_GRENADE_SPLASH || mod == MOD_ROCKET_SPLASH || mod == MOD_PLASMA_SPLASH) ) {
			return;
		} else {
			if ( targ->use && targ->moverState == MOVER_POS1 ) {
				targ->use( targ, inflictor, attacker );
			}
			return;
		}
		/*return;*/
	}

	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	/*if ( attacker->client && attacker != targ ) {
		max = attacker->client->ps.stats[STAT_MAX_HEALTH];
		damage = damage * max / 100;
	}*/
	// mmp - sorry, this was abused

	// increase damage if in overtime/sudden death
	if ( attacker->client && attacker != targ ) {
		// if extra damage is over 1.0f
		if ( level.c_extraDamage > 1.0f ) {
			damage *= level.c_extraDamage;
		}
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	// check if attacker has quad damage
	if (attacker->client->ps.powerups[PW_QUAD] || level.rs_quadMode ) {
		quadFactor = g_quadfactor.value;
	} else {
		quadFactor = 1;
	}

	// increase lightning gun damage with consistant hits
	if ( mod == MOD_LIGHTNING ) {
		attackingClient = attacker->client;
		lastHit = level.time - attackingClient->lightningLastHit;
		//G_Printf( "^3DEBUG: listHit=%i\n", lastHit ); // debug
		if ( lastHit < 100 ) {
			damage += 4 * GetDamageLevel( attacker ) * quadFactor;
			attackingClient->lightningLastHit = level.time;
		} else
		if ( lastHit < 200 ) {
			damage += 2 * GetDamageLevel( attacker ) * quadFactor;
			attackingClient->lightningLastHit = level.time;
		} else {
			attackingClient->lightningLastHit = level.time - 100;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}

	knockback = damage;
	if ( mod == MOD_SUPER_SHOTGUN ) {
		// give extra push back from super shotgun
		knockback *= 2;
	} else
	if ( mod == MOD_LIGHTNING ) {
		knockback *= 2;
	}
	if ( knockback > 666 ) {
		knockback = 666; // was 200
	}

	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	/*if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}*/

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client ) {
		vec3_t	kvel;
		float	mass;

		mass = 200;

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 4; // was * 2
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 666 ) {
				t = 666; // was 200
			}
			targ->client->ps.pm_time += t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team

		if ( targ != attacker && OnSameTeam (targ, attacker)  ) {

			if ( !level.rs_friendlyFire /*!g_friendlyFire.integer*/ ) {
				return;
			} else {
				if ( level.rs_quadMode ) {
					damage *= 0.125; // reduce amount of damage on teammate
				} else {
					damage *= 0.5; // reduce amount of damage on teammate
				}
			}
		}

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}
	}

	// for reference
	//
	// mmp - "battlesuit" will be replaced with the "pentagram of protection" [DONE]
	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	/*if ( client && client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( ( dflags & DAMAGE_RADIUS ) || ( mod == MOD_FALLING ) ) {
			return;
		}
		damage *= 0.5;
	}*/

	// enemies in AA1 do half damage by default, but can be modified in a custom ruleset
	if ( g_gametype.integer == GT_AA1 && attacker->client->sess.sessionTeam == TEAM_RED ) {
		damage *= level.rs_enemyAttackLevel;
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if ( attacker->client && client
			&& targ != attacker && targ->health > 0
			&& targ->s.eType != ET_MISSILE
			&& targ->s.eType != ET_GENERAL) {
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS] -= damage;
		} else {
//			attacker->client->ps.persistant[PERS_HITS]++;
			attacker->client->ps.persistant[PERS_HITS] += damage;
		}
	}

	// no damage during pre-match warmup
	/*if ( level.warmupTime && !( mod == MOD_CRUSH || mod == MOD_TELEFRAG || mod == MOD_TRIGGER_HURT ) ) {
		return;
	}*/

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if ( targ == attacker) {

		// did the player discharge the lightning gun?
		if ( mod == MOD_LIGHTNING_DISCHARGE ) {
			damage = 666;
		} else {

			// do we have self damage disabled
			if ( !level.rs_selfDamage ) {
				return;
			}

			// suggestion by Jewjitsu
			// keep self-damage 50% of normal weapon splash damage, regardless of level
			switch (attacker->client->ps.stats[STAT_LEVEL] / LV_SUBPOINT) {
				// TODO: this should be made into a table, which would work better
				case 4:
					damage *= 0.5 / LV_FIVE;
					break;
				case 3:
					damage *= 0.5 / LV_FOUR;
					break;
				case 2:
					damage *= 0.5 / LV_THREE;
					break;
				case 1:
					damage *= 0.5 / LV_TWO;
					break;
				default:
					damage *= 0.5;
			}

			// if in quad mode, reduce self damage more
			if ( level.rs_quadMode ) {
				damage *= 0.25;
			}
			/*damage *= 0.5;*/

		}

	} else // otherwise allow the attacker to gain level
	if ( !level.warmupTime && !OnSameTeam( targ, attacker ) ){
		levelGain = attacker->client->ps.stats[STAT_LEVEL] + damage;
		if ( levelGain > LV_MAXPOINT)
			levelGain = LV_MAXPOINT;
		attacker->client->ps.stats[STAT_LEVEL] = levelGain;
	}

	// do we want falling damage
	if ( mod == MOD_FALLING ) {
		// do we have self damage disabled
		if ( !level.rs_selfDamage ) {
			return;
		}
	}

	if ( damage < 1 ) {
		damage = 1;
	}
	take = damage;

	// pentagram of protection protects you from pretty much all damage other than world damage.
	// however armor still gets chiped away, and also you cannot get telefragged,
	// but instead they'll be telefragged.
	if ( client && client->ps.powerups[PW_PENT] && !( /*mod == MOD_CRUSH ||*/ mod == MOD_TRIGGER_HURT || mod == MOD_TELEFRAG ) ) {

		G_AddEvent( targ, EV_POWERUP_PENT, 0 );

		if ( !level.warmupTime ) {
			// chip armor away
			CheckArmor (targ, take, dflags);
		}
		return;

	}

	// no damage during pre-match warmup
	if ( level.warmupTime && !( mod == MOD_CRUSH || mod == MOD_TELEFRAG || mod == MOD_TRIGGER_HURT ) ) {
		// show damage output in warmup
		if ( take > 0 ) {
			NumPlum(attacker, targ->r.currentOrigin, take);
		}
		return;
	}

	// save some from armor
	asave = CheckArmor (targ, take, dflags);
	take -= asave;

	//if ( g_debugDamage.integer ) {
		G_Printf( "%i: client:%i health:%i damage:%i armor:%i ... inflictor:%i attacker:%i\n", level.time, targ->s.number,
			targ->health, take, asave, inflictor->s.number, attacker->s.number );
	//}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
	if( g_gametype.integer == GT_CTF) {
		Team_CheckHurtCarrier(targ, attacker);
	}

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
	}

	// do the damage
	if (take) {
		targ->health = targ->health - take;
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}

		if ( targ->health <= 0 ) {
			if ( client )
				targ->flags |= FL_NO_KNOCKBACK;

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			targ->die (targ, inflictor, attacker, take, mod);
			return;
		} else if ( targ->pain ) {
			targ->pain (targ, attacker, take);
		}
	}

	// set delay for regen
	targ->client->healthRegenTime = level.time + 5000;

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/

#define	OFFSET_BOUNDS		30 // 15

qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;
	vec3_t	offsetmins = {-OFFSET_BOUNDS, -OFFSET_BOUNDS, -OFFSET_BOUNDS};
	vec3_t	offsetmaxs = {OFFSET_BOUNDS, OFFSET_BOUNDS, OFFSET_BOUNDS};

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy(midpoint, dest);
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	// this should probably check in the plane of projection,
	// rather than in world coordinate
	VectorCopy(midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmaxs[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy(midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmaxs[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy(midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmaxs[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy(midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmaxs[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy(midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmins[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy(midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmins[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy(midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmins[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy(midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmins[2];
	dest[2] += offsetmins[2];
	trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

	if (tr.fraction == 1.0)
		return qtrue;

	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;
	float		mandatoryDamage;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		// simulate damage radius from Q1
		mandatoryDamage = damage / 3;
		points = ( (damage - mandatoryDamage) * ( 1.0 - dist / radius ) ) + mandatoryDamage;

		//points = damage * ( 1.0 - dist / radius );

		if( CanDamage (ent, origin) ) {
			if( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}

/*
============
G_WaterRadiusDamage for The SARACEN's Lightning Discharge
============
*/
qboolean G_WaterRadiusDamage (vec3_t origin, gentity_t *attacker, float damage, float radius,
					gentity_t *ignore, int mod) {
	float           points, dist;
	gentity_t       *ent;
	int             entityList[MAX_GENTITIES];
	int             numListedEntities;
	vec3_t          mins, maxs;
	vec3_t          v;
	vec3_t          dir;
	int             i, e;
	qboolean        hitClient = qfalse;

	if (!(trap_PointContents (origin, -1) & MASK_WATER)) return qfalse;
		// if we're not underwater, forget it!

	if (radius < 100) radius = 100;

	for (i = 0 ; i < 3 ; i++)
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox (mins, maxs, entityList, MAX_GENTITIES);

	for (e = 0 ; e < numListedEntities ; e++)
	{
		ent = &g_entities[entityList[e]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		// find the distance from the edge of the bounding box
		for (i = 0 ; i < 3 ; i++)
		{
			if (origin[i] < ent->r.absmin[i]) v[i] = ent->r.absmin[i] - origin[i];
			else if (origin[i] > ent->r.absmax[i]) v[i] = origin[i] - ent->r.absmax[i];
			else v[i] = 0;
		}

		dist = VectorLength(v);
		if (dist >= radius)
			continue;

		points = damage * (1.0 - dist / radius);
		if (points > 666) {
			points = 666; // damage cannot be more powerful than the devil
		}

		/*if (CanDamage (ent, origin) && ent->waterlevel) // must be in the water, somehow!
		{
			if (LogAccuracyHit (ent, attacker)) hitClient = qtrue;
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}*/
		if (CanDamage (ent, origin)) // mmp - allow discharge to extend out of the water just like in quake 1
		{
			if (LogAccuracyHit (ent, attacker)) hitClient = qtrue;
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}
