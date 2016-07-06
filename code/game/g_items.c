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

/*

  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  all items should pop when dropped in lava or slime

  Respawnable items don't actually go away when picked up, they are
  just made invisible and untouchable.  This allows them to ride
  movers and respawn apropriately.
*/


#define	RESPAWN_WEAPON			30
#define	RESPAWN_ARMOR1			15
#define	RESPAWN_ARMOR2			20 //
#define	RESPAWN_ARMOR3			25
#define	RESPAWN_HEALTH1			25
#define	RESPAWN_HEALTH2			30 // is 35 in q3a
#define	RESPAWN_AMMO			30 // is 40 in q3a
#define	RESPAWN_AMMO_WEAPONSTAY	15 // is 40 in q3a
#define	RESPAWN_HOLDABLE		120 // 2 minutes, is 60 seconds in q3a
#define	RESPAWN_MEGAHEALTH		20 // 35 in q3a, and was at one point 120  :/
								   // was 30 in earlier mfa proto releases
#define	RESPAWN_QUAD			150 // 2.5 minutes, is 120 in q3a, and 60 in qw
#define	RESPAWN_PENT			300 // 5 minutes
#define	RESPAWN_RING			300 // 5 minutes



//======================================================================

int Pickup_Powerup( gentity_t *ent, gentity_t *other ) {
	int			quantity;
	int			i;
	gclient_t	*client;

	if ( !other->client->ps.powerups[ent->item->giTag] ) {
		// round timing to seconds to make multiple powerup timers
		// count in sync
		other->client->ps.powerups[ent->item->giTag] =
			level.time - ( level.time % 1000 );
	}

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	// mmp - this fix not only prevents the count from overflowing,
	// but keeps the player from stacking up the count (eg: 500 second quad)
	other->client->ps.powerups[ent->item->giTag] = level.time + quantity * 1000;
	/*
	other->client->ps.powerups[ent->item->giTag] += quantity * 1000;
	*/

	// give any nearby players a "denied" anti-reward
	/*
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		vec3_t		delta;
		float		len;
		vec3_t		forward;
		trace_t		tr;

		client = &level.clients[i];
		if ( client == other->client ) {
			continue;
		}
		if ( client->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
			continue;
		}

		// if same team in team game, no sound
		// cannot use OnSameTeam as it expects to g_entities, not clients
		if ( g_gametype.integer >= GT_TEAM && other->client->sess.sessionTeam == client->sess.sessionTeam  ) {
			continue;
		}

		// if too far away, no sound
		VectorSubtract( ent->s.pos.trBase, client->ps.origin, delta );
		len = VectorNormalize( delta );
		if ( len > 192 ) {
			continue;
		}

		// if not facing, no sound
		AngleVectors( client->ps.viewangles, forward, NULL, NULL );
		if ( DotProduct( delta, forward ) < 0.4 ) {
			continue;
		}

		// if not line of sight, no sound
		trap_Trace( &tr, client->ps.origin, NULL, NULL, ent->s.pos.trBase, ENTITYNUM_NONE, CONTENTS_SOLID );
		if ( tr.fraction != 1.0 ) {
			continue;
		}

		// anti-reward
		client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_DENIEDREWARD;
	}
	*/

	switch ( ent->item->giTag ) {
		case PW_QUAD:
			return RESPAWN_QUAD;
		case PW_PENT:
			return RESPAWN_PENT;
		default:
			return RESPAWN_PENT;
	}
}

//======================================================================

#ifdef MISSIONPACK
int Pickup_PersistantPowerup( gentity_t *ent, gentity_t *other ) {
	int		clientNum;
	char	userinfo[MAX_INFO_STRING];
	float	handicap;
	int		max;

	other->client->ps.stats[STAT_PERSISTANT_POWERUP] = ent->item - bg_itemlist;
	other->client->persistantPowerup = ent;

	switch( ent->item->giTag ) {
	case PW_GUARD:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		max = (int)(2 *  handicap);

		other->health = max;
		other->client->ps.stats[STAT_HEALTH] = max;
		other->client->ps.stats[STAT_MAX_HEALTH] = max;
		other->client->ps.stats[STAT_ARMOR] = max;
		other->client->pers.maxHealth = max;

		break;

	case PW_SCOUT:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		other->client->ps.stats[STAT_ARMOR] = 0;
		break;

	case PW_DOUBLER:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		break;
	case PW_AMMOREGEN:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		memset(other->client->ammoTimes, 0, sizeof(other->client->ammoTimes));
		break;
	default:
		clientNum = other->client->ps.clientNum;
		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		handicap = atof( Info_ValueForKey( userinfo, "handicap" ) );
		if( handicap<=0.0f || handicap>100.0f) {
			handicap = 100.0f;
		}
		other->client->pers.maxHealth = handicap;
		break;
	}

	return -1;
}

//======================================================================
#endif

int Pickup_Holdable( gentity_t *ent, gentity_t *other ) {

	other->client->ps.stats[STAT_HOLDABLE_ITEM] = ent->item - bg_itemlist;

	if( ent->item->giTag == HI_KAMIKAZE ) {
		other->client->ps.eFlags |= EF_KAMIKAZE;
	}

	return RESPAWN_HOLDABLE;
}


//======================================================================

// TODO: change int weapon to int ammo
void Add_Ammo (gentity_t *ent, int weapon, int count)
{
	// if ammo count is zero, then fuck it
	if ( count == 0 ) {
		return;
	}

	// if negative or no ammo, then fuck it
	/*if ( count <= 0 && !level.warmupTime ) {
		return;
	}*/

	if ( level.rs_doubleAmmo ) {
		count += count;
	}
	ent->client->ps.ammo[weapon] += count;

	/*if ( ent->client->ps.ammo[weapon] > 200 ) {
		ent->client->ps.ammo[weapon] = 200;
	}*/
	switch ( weapon ) {
		case AT_BULLETS:
			if ( ent->client->ps.ammo[weapon] > WC_HIGH_AMMO ) {
				ent->client->ps.ammo[weapon] = WC_HIGH_AMMO;
			}
			break;
		case AT_CELLS:
			if ( ent->client->ps.ammo[weapon] > WC_MED_AMMO ) {
				ent->client->ps.ammo[weapon] = WC_MED_AMMO;
			}
			break;
		default:
			if ( ent->client->ps.ammo[weapon] > WC_LOW_AMMO ) {
				ent->client->ps.ammo[weapon] = WC_LOW_AMMO;
			}
			break;
	}
}

int Pickup_Ammo (gentity_t *ent, gentity_t *other)
{
	int		quantity;

	// don't add ammo in warmup
	if ( !level.warmupTime ) {
		if ( ent->count ) {
			quantity = ent->count;
		} else {
			quantity = ent->item->quantity;
		}

		Add_Ammo (other, ent->item->giTag, quantity);
	}

	return RESPAWN_AMMO;
}

//======================================================================


int Pickup_Weapon (gentity_t *ent, gentity_t *other, int weaponRespawn ) {
	int		quantity;

	if ( level.rs_matchMode == MM_PICKUP_ONCE ) {

		/*
		MM_PICKUP_ONCE,
		MM_PICKUP_ALWAYS,
		MM_ALLWEAPONS_MAXAMMO,
		MM_ALLWEAPONS,
		*/

		// do we have the weapon?  if so, don't pick it up again (qw non-dmm1 style)
		if ( (other->client->ps.stats[STAT_WEAPONS] & ( 1 << ent->item->giTag ) ) ) {
			return 0;
		}
	}

	if ( level.warmupTime ) {
		// don't give a fuck about ammo in warmup
		quantity = -1;
	} else {
		// get the ammo amount
		if ( ent->count )
			quantity = ent->count;
		else
			quantity = ent->item->quantity;
	}

	// add the weapon
	other->client->ps.stats[STAT_WEAPONS] |= ( 1 << ent->item->giTag );

	Add_Ammo( other, ammoGroup[ent->item->giTag].ammo, quantity );

	if ( weaponRespawn ) {
		// return value is expecting the respawn time if the player can grab the weapon
		return weaponRespawn;
	} else {
		// return value is expecting "if" the player can grab the weapon
		return 1;
	}
}


//======================================================================


int Pickup_Keycard (gentity_t *ent, gentity_t *other, int respawnTimer) {
	int		quantity;

	// add keycard to inventory
	other->client->ps.stats[STAT_INVENTORY] |= ( 1 << ent->item->giTag );

	return respawnTimer;

}


//======================================================================


void HealthDecay (gentity_t *ent) {

	if (!ent->activator) {
		ent->think = RespawnItem;
		ent->nextthink = level.time + RESPAWN_MEGAHEALTH * 1000;
		return;
	}

	if (strcmp(ent->activator->classname,"player")
		|| ent->activator->client->ps.stats[STAT_HEALTH] <= 100) {

		// increase item spawn times based on game round and overtimes
		if ( level.rs_dynamicItemSpawns ) {
			ent->nextthink = level.time + ( RESPAWN_MEGAHEALTH + (level.currentRound * 5) + ( level.overtime * 5 ) ) * 1000;
		} else {
			ent->nextthink = level.time + RESPAWN_MEGAHEALTH * 1000;
		}
		ent->think = RespawnItem;
		return;
	}

	ent->nextthink = level.time + 1000;
}


//======================================================================

int Pickup_Health (gentity_t *ent, gentity_t *other) {
	int			max;
	int			quantity;

	// small and mega healths will go over the max
	if ( ent->item->quantity >= 0 /*ent->item->quantity != 5 && ent->item->quantity != 100*/ ) {
		max = other->client->ps.stats[STAT_MAX_HEALTH];
	} else {
		if ( level.rs_matchMode == MM_ALLWEAPONS_MAXAMMO ) {
			max = other->client->ps.stats[STAT_MAX_HEALTH] * 2.5; // max health is 250
		} else {
			max = MARK_OF_THE_DEVIL; // cannot be more powerful than the devil
		}
		other->client->healthDecayTime = level.time + HEALTH_DECAY_TIME * 1000;

		// increase health decay by the number of megas taken while over max health
		if ( ent->item->quantity <= -100 ) {
			other->client->healthDecayRate++;
		}
	}

	if ( ent->count ) {
		quantity = abs(ent->count);
	} else {
		quantity = abs(ent->item->quantity);
	}

	//G_Printf ("^3DEBUG PICKUP = '%i' '%i'\n", ent->item->quantity, quantity ); // debug

	other->health += quantity;

	if (other->health > max ) {
		other->health = max;
	}
	other->client->ps.stats[STAT_HEALTH] = other->health;

	// mega health respawns slow
	if ( ent->item->quantity <= -100 ) {
		ent->think = HealthDecay;
		ent->activator = other;
		return 1;
		/*return RESPAWN_MEGAHEALTH;*/
	}

	if ( ent->item->quantity < 50 ) {
		return RESPAWN_HEALTH1;
	}

	return RESPAWN_HEALTH2;
}

//======================================================================

int Pickup_Armor( gentity_t *ent, gentity_t *other ) {

	int		respawn;

	/*other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;
	if ( other->client->ps.stats[STAT_ARMOR] > other->client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
		other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
	}*/

	if (ent->item->quantity == AR_TIER3MAXPOINT) // armor_body
	{
		other->client->ps.stats[STAT_ARMOR] = AR_TIER3MAXPOINT;
		other->client->ps.stats[STAT_ARMORTIER] = 3; // armor_body protection
		respawn = RESPAWN_ARMOR3;
	}
	else if (ent->item->quantity == AR_TIER2MAXPOINT) // armor_combat
	{
		other->client->ps.stats[STAT_ARMOR] = AR_TIER2MAXPOINT;
		other->client->ps.stats[STAT_ARMORTIER] = 2; // armor_combat protection
		respawn = RESPAWN_ARMOR2;
	}
	else if (ent->item->quantity == AR_TIER1MAXPOINT) // armor_jacket
	{
		other->client->ps.stats[STAT_ARMOR] = AR_TIER1MAXPOINT;
		other->client->ps.stats[STAT_ARMORTIER] = 1; // armor_jacket protection
		respawn = RESPAWN_ARMOR1;
	}
	else // Shard
	{
		// if no armor tier is set, then give armor_jacket protection by default
		if (other->client->ps.stats[STAT_ARMOR] <= 0)
			other->client->ps.stats[STAT_ARMORTIER] = 1; // armor_jacket protection
		other->client->ps.stats[STAT_ARMOR] += 5;
		respawn = RESPAWN_ARMOR1;

		switch( other->client->ps.stats[STAT_ARMORTIER] ) {
			case 3:
				// cap the armor points to 200
				if (other->client->ps.stats[STAT_ARMOR] > AR_TIER3MAXPOINT)
					other->client->ps.stats[STAT_ARMOR] = AR_TIER3MAXPOINT;
				break;
			case 2:
				// cap the armor points to 150
				if (other->client->ps.stats[STAT_ARMOR] > AR_TIER2MAXPOINT)
					other->client->ps.stats[STAT_ARMOR] = AR_TIER2MAXPOINT;
				break;
			default:
				// cap the armor points to 100
				if (other->client->ps.stats[STAT_ARMOR] > AR_TIER1MAXPOINT)
					other->client->ps.stats[STAT_ARMOR] = AR_TIER1MAXPOINT;
		}
	}

	return respawn;
}


//======================================================================
// TODO: complete this function

void Pickup_Backpack (gentity_t *ent, gentity_t *other) {

	int		quantity; // for health bonus
	int		max;

	int		backpackSlot;

	// get backpack slot
	if ( ent->count )
		backpackSlot = ent->count;
	else
		backpackSlot = 0; // just fuck it

	// check if we're giving bonus health
	if ( backpack[backpackSlot].shells < 0 ||
			backpack[backpackSlot].rockets < 0 ||
			backpack[backpackSlot].cells < 0 ||
			backpack[backpackSlot].bullets < 0 ||
			backpack[backpackSlot].gas < 0 )
	{

		max = MARK_OF_THE_DEVIL; // cannot be more powerful than the devil
		other->client->healthDecayTime = level.time + HEALTH_DECAY_TIME * 1000;

		other->health += 10;

		if (other->health > max ) {
			other->health = max;
		}

		other->client->ps.stats[STAT_HEALTH] = other->health;

	} else {

		// add the weapon
		other->client->ps.stats[STAT_WEAPONS] |= ( 1 << backpack[backpackSlot].lastWeap );

		// add the ammo
		if ( backpack[backpackSlot].shells > 0 )
			Add_Ammo( other, AT_SHELLS, backpack[backpackSlot].shells );
		if ( backpack[backpackSlot].rockets > 0 )
			Add_Ammo( other, AT_ROCKETS, backpack[backpackSlot].rockets );
		if ( backpack[backpackSlot].cells > 0 )
			Add_Ammo( other, AT_CELLS, backpack[backpackSlot].cells );
		if ( backpack[backpackSlot].bullets > 0 )
			Add_Ammo( other, AT_BULLETS, backpack[backpackSlot].bullets );
		if ( backpack[backpackSlot].gas > 0 )
			Add_Ammo( other, AT_GAS, backpack[backpackSlot].gas );

	}

	//G_Printf ("^3DEBUG PICKUP = '%i'\n", backpackSlot); // debug

	/*if (ent->item->giTag == WP_GRAPPLING_HOOK)
		other->client->ps.ammo[ent->item->giTag] = -1; // unlimited ammo
	*/

	return;

}


//======================================================================

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) {
	if (!ent) {
		return;
	}

	// disable power-ups and holdable items via ruleset
	if (!level.rs_powerUps) {
		if (ent->item->giType == IT_POWERUP || ent->item->giType == IT_HOLDABLE) {
			return;
		}
	}

	// disable armor spawns via ruleset
	if (!level.rs_armor) {
		if (ent->item->giType == IT_ARMOR) {
			return;
		}
	}

	// if game type is not ctf, don't spawn ctf flags
	if ( ent->item->giType == IT_TEAM ) {
		if ( ent->item->giTag == PW_REDFLAG || ent->item->giTag == PW_BLUEFLAG ) {
			if ( g_gametype.integer != GT_CTF ) {
				return;
			}
		}
	}

	/*if(ent->item->giType == IT_POWERUP && ent->item->giTag == PW_QUAD && g_quadfactor.value <= 1.0) {
		return;
	}*/

	// disable quad from spawning if quadmode is set
	if(ent->item->giType == IT_POWERUP && ent->item->giTag == PW_QUAD && level.rs_quadMode) {
		return;
	}

	// randomly select from teamed entities
	if (ent->team) {
		gentity_t	*master;
		int	count;
		int choice;

		if ( !ent->teammaster ) {
			G_Error( "RespawnItem: bad teammaster");
		}
		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->teamchain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->teamchain, count++)
			;
	}

	ent->r.contents = CONTENTS_TRIGGER;
	ent->s.eFlags &= ~EF_NODRAW;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	trap_LinkEntity (ent);

	if ( ent->item->giType == IT_POWERUP ) {
		// play powerup spawn sound to all clients
		gentity_t	*te;

		// if the powerup respawn sound should Not be global
		if (ent->speed) {
			te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
		}
		else {
			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		}
		te->s.eventParm = G_SoundIndex( "sound/items/poweruprespawn.wav" );
		te->r.svFlags |= SVF_BROADCAST;
	}

	if ( ent->item->giType == IT_HOLDABLE && ent->item->giTag == HI_KAMIKAZE ) {
		// play powerup spawn sound to all clients
		gentity_t	*te;

		// if the powerup respawn sound should Not be global
		if (ent->speed) {
			te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
		}
		else {
			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		}
		te->s.eventParm = G_SoundIndex( "sound/items/kamikazerespawn.wav" );
		te->r.svFlags |= SVF_BROADCAST;
	}

	// determine what type of item spawn sound to play
	if ( ent->item->giType == IT_POWERUP ||
			( ent->item->giType == IT_ARMOR && ent->item->quantity == AR_TIER3MAXPOINT ) ||
			( ent->item->giType == IT_HEALTH && ent->item->quantity <= -100 ) ) {
		// play the super respawn sound
		G_AddEvent( ent, EV_ITEM_SUPERRESPAWN, 0 );
	} else {
		// play the normal respawn sound only to nearby clients
		G_AddEvent( ent, EV_ITEM_RESPAWN, 0 );
	}

	ent->nextthink = 0;
}


/*
===============
Touch_Item
===============
*/
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace) {
	int			respawn;
	int			alwaysSpawned = 0;
	qboolean		predict;
	int			weaponRespawn;
	int			respawnValue;
	int			respawnTimeAdd;

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup

	// don't pick up items if not allowed (meant to delay pickups when player spawns on an item)
	if ( other->client->disallowItemPickUp ) {
		return;
	}

	// enemies in AA1 cannot pickup items
	if ( other->client->ps.persistant[PERS_MISC] & PMSC_NEVER_PICKUP_ANY_ITEM ) {
		return;
	}

	// the same pickup rules are used for client side and server side
	if ( !BG_CanItemBeGrabbed( g_gametype.integer, &ent->s, &other->client->ps ) ) {
		return;
	}

	// don't pick up if item has been freshly tossed by player
	if( ent->pickupDelay > level.time )
		return;

	G_VerboseLogPrintf( 2, "Item: %i %s\n", other->s.number, ent->item->classname );

	predict = other->client->pers.predictItemPickup;

	// increase item spawn times based on game round and overtimes
	if ( level.rs_dynamicItemSpawns ) {
		respawnTimeAdd = (level.currentRound * 5) + ( level.overtime * 5 );
	} else {
		respawnTimeAdd = 0;
	}

	// call the item-specific pickup function
	switch( ent->item->giType ) {
	case IT_WEAPON:
		weaponRespawn = level.rs_weaponRespawn;
		if ( level.rs_matchMode == MM_PICKUP_ALWAYS || level.rs_matchMode == MM_PICKUP_ALWAYS_NOAMMO ) {
			if ( weaponRespawn < 1 ) {
				weaponRespawn = RESPAWN_WEAPON;
			}
		}

		if (weaponRespawn > 0) {
			respawn = Pickup_Weapon(ent, other, weaponRespawn) + respawnTimeAdd;
			alwaysSpawned = 0; // this shouldn't be needed, will remove later
		} else {
			alwaysSpawned = Pickup_Weapon(ent, other, 0);
			respawn = 0;
		}
//		predict = qfalse;
		break;
	case IT_AMMO:
		// TODO: code this better
		respawn = Pickup_Ammo(ent, other) + respawnTimeAdd;

		// if matchmode is set to weaponstay, then make ammo respawn more often
		if (respawn == RESPAWN_AMMO && level.rs_matchMode == MM_PICKUP_ONCE) {
			respawn = RESPAWN_AMMO_WEAPONSTAY + respawnTimeAdd;
		}
//		predict = qfalse;
		break;
	case IT_BACKPACK:
		Pickup_Backpack(ent, other);
		respawn = -1;
		break;
	case IT_KEYCARD: // mmp - wip
		respawnValue = level.rs_keycardRespawn;

		if (respawnValue > 0) {
			respawn = Pickup_Keycard(ent, other, respawnValue) + respawnTimeAdd;
			alwaysSpawned = 0; // this shouldn't be needed, will remove later
		} else {
			alwaysSpawned = Pickup_Keycard(ent, other, 0);
			respawn = 0;
		}

//		alwaysSpawned = Pickup_Keycard(ent, other);
//		respawn = -1;
////		respawn = Pickup_Keycard(ent, other);
////		predict = qfalse;
		break;
	case IT_ARMOR:
		respawn = Pickup_Armor(ent, other) + respawnTimeAdd;
		//G_Printf ("^3DEBUG RESPAWN = '%i'\n", respawn); // debug
		break;
	case IT_HEALTH:
		respawn = Pickup_Health(ent, other) + respawnTimeAdd;
		break;
	case IT_POWERUP:
		respawn = Pickup_Powerup(ent, other);
		predict = qfalse;
		break;
#ifdef MISSIONPACK
	case IT_PERSISTANT_POWERUP:
		respawn = Pickup_PersistantPowerup(ent, other);
		break;
#endif
	case IT_TEAM:
		respawn = Pickup_Team(ent, other);
		break;
	case IT_HOLDABLE:
		respawn = Pickup_Holdable(ent, other) + respawnTimeAdd;
		break;
	default:
		return;
	}

	if ( !respawn && !alwaysSpawned ) {
		return;
	}

	// play the normal pickup sound
	if (predict) {
		G_AddPredictableEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
	} else {
		G_AddEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
	}

	// powerup pickups are global broadcasts
	/*if ( ent->item->giType == IT_POWERUP || ent->item->giType == IT_TEAM) {
		// if we want the global sound to play
		if (!ent->speed) {
			gentity_t	*te;

			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
			te->s.eventParm = ent->s.modelindex;
			te->r.svFlags |= SVF_BROADCAST;
		} else {
			gentity_t	*te;

			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
			te->s.eventParm = ent->s.modelindex;
			// only send this temp entity to a single client
			te->r.svFlags |= SVF_SINGLECLIENT;
			te->r.singleClient = other->s.number;
		}
	}*/

	// fire item targets
	G_UseTargets (ent, other);

	// some items and all weapons don't respawn, since they're always available
	if ( alwaysSpawned )
		return;

	// wait of -1 will not respawn
	if ( ent->wait == -1 ) {
		ent->r.svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}

	// non zero wait overrides respawn time
	if ( ent->wait && ent->think != HealthDecay ) {
		respawn = ent->wait;
	}

	// random can be used to vary the respawn time
	if ( ent->random ) {
		respawn += crandom() * ent->random;
		if ( respawn < 1 ) {
			respawn = 1;
		}
	}

	// dropped items will not respawn
	if ( ent->flags & FL_DROPPED_ITEM ) {
		ent->freeAfterEvent = qtrue;
	}

	// picked up items still stay around, they just don't
	// draw anything.  This allows respawnable items
	// to be placed on movers.
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->r.contents = 0;

	// ZOID
	// A negative respawn times means to never respawn this item (but don't
	// delete it).  This is used by items that are respawned by third party
	// events such as ctf flags
	if ( respawn <= 0 ) {
		ent->nextthink = 0;
		ent->think = 0;
	} else {
		ent->nextthink = level.time + respawn * 1000;
		if ( ent->think != HealthDecay ) ent->think = RespawnItem;
		/*ent->think = RespawnItem;*/
	}
	trap_LinkEntity( ent );
}


//======================================================================

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity ) {
	gentity_t	*dropped;

	dropped = G_Spawn();

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	dropped->r.contents = CONTENTS_TRIGGER;

	dropped->touch = Touch_Item;

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->s.eFlags |= EF_BOUNCE_HALF;
#ifdef MISSIONPACK
	if ((g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF)
			&& item->giType == IT_TEAM) // Special case for CTF flags
#else
	if (g_gametype.integer == GT_CTF && item->giType == IT_TEAM) // Special case for CTF flags
#endif
	{
		dropped->think = Team_DroppedFlagThink;
		dropped->nextthink = level.time + ( FLAG_RETURN_TIME - level.c_flagReturnDecrease);
		Team_CheckDroppedItem( dropped );
	} else if (level.rs_keycardDropable && item->giType == IT_KEYCARD) {
		dropped->think = G_FreeEntity;
		dropped->nextthink = level.time + KEYCARD_DESPAWN_TIME;
	} else if (item->giType == IT_BACKPACK) {
		dropped->think = G_FreeEntity;
		dropped->nextthink = level.time + BACKPACK_DESPAWN_TIME;
	} else { // auto-remove after 20 seconds
		dropped->think = G_FreeEntity;
		dropped->nextthink = level.time + DEFAULT_DESPAWN_TIME;
	}

	dropped->flags = FL_DROPPED_ITEM;

	trap_LinkEntity (dropped);

	return dropped;
}

/*
================
LaunchItem_Weapon

Spawns a weapon and tosses it forward
================
*/
gentity_t *LaunchItem_Weapon( gitem_t *item, vec3_t origin, vec3_t velocity, int dropTime, int weapon ) {
	gentity_t	*dropped;

	dropped = G_Spawn();

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	dropped->r.contents = CONTENTS_TRIGGER;

	dropped->touch = Touch_Item;

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->s.eFlags |= EF_BOUNCE_HALF;

	dropped->think = G_FreeEntity;
	dropped->nextthink = level.time + 30000;

	dropped->flags = FL_DROPPED_ITEM;

	dropped->pickupDelay = dropTime + 500;

	// remember slot
	dropped->count = level.currentBackpackSlot;

	// add ammo and current weapon in backpack
	// TODO: code this better
	backpack[level.currentBackpackSlot].lastWeap = weapon;
	backpack[level.currentBackpackSlot].shells = 0;
	backpack[level.currentBackpackSlot].rockets = 0;
	backpack[level.currentBackpackSlot].cells = 0;
	backpack[level.currentBackpackSlot].bullets = 0;
	backpack[level.currentBackpackSlot].gas = 0;

	// move to next backpack slot
	level.currentBackpackSlot++;
	if ( level.currentBackpackSlot >= MAX_BACKPACK_CONTENTS ) {
		level.currentBackpackSlot = 0;
	}

	trap_LinkEntity(dropped);

	return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle ) {
	vec3_t	velocity;
	vec3_t	angles;

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0;	// always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;

	return LaunchItem( item, ent->s.pos.trBase, velocity );
}


/*
================
Drop_Item_Weapon

Spawns an item and tosses it forward
================
*/
//TODO: Switch to next weapon

gentity_t *Drop_Item_Weapon( gentity_t *ent, gitem_t *item, float angle, int weapon ) {
	vec3_t	velocity;
	vec3_t	angles;
	vec3_t	position;
	int		ammoCount;
	int		dropTime;

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0; // always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 0, velocity );
	VectorAdd( velocity, ent->s.pos.trBase, position );

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;

/*	ammoCount = ent->client->ps.ammo[item->giTag];

	ent->client->ps.ammo[item->giTag] = 0;
	ent->client->ps.stats[STAT_WEAPONS] &= ~( 1 << item->giTag );*/

	dropTime = level.time;

	return LaunchItem_Weapon( item, position, velocity, dropTime, weapon );
}


/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	RespawnItem( ent );
}

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningItem( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		dest;

	VectorSet( ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );

	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex
	ent->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	ent->r.contents = CONTENTS_TRIGGER;
	ent->touch = Touch_Item;
	// using an item causes it to respawn
	ent->use = Use_Item;

	if ( ent->spawnflags & 1 ) {
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} else {
		// drop to floor
		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
		if ( tr.startsolid ) {
			G_Printf ("FinishSpawningItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity( ent );
			return;
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}

	// team slaves and targeted items aren't present at start
	if ( ( ent->flags & FL_TEAMSLAVE ) || ent->targetname ) {
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		return;
	}

	// mmp
	if ( level.rs_matchMode >= MM_ALLWEAPONS_MAXAMMO ) {
		if (ent->item->giType == IT_WEAPON) {
			ent->s.eFlags |= EF_NODRAW;
			ent->r.contents = 0;
			return;
		}
		if ( level.rs_matchMode == MM_ALLWEAPONS_MAXAMMO || level.rs_matchMode == MM_ROCKET_MANIAX ) {
			if (ent->item->giType == IT_AMMO) {
				ent->s.eFlags |= EF_NODRAW;
				ent->r.contents = 0;
				return;
			}
		}
	} else if ( level.rs_matchMode == MM_PICKUP_ALWAYS_NOAMMO ) {
		if (ent->item->giType == IT_AMMO) {
			ent->s.eFlags |= EF_NODRAW;
			ent->r.contents = 0;
			return;
		}
	}

	// powerups don't spawn in for a while
	if ( ent->item->giType == IT_POWERUP ) {
		float	respawn;

//		respawn = 45 + crandom() * 15;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
//		ent->nextthink = level.time + respawn * 1000;
		if ( ent->item->giTag == PW_QUAD ) {
			ent->nextthink = level.time + RESPAWN_QUAD * 1000;
		} else if ( ent->item->giTag == PW_PENT ) {
			ent->nextthink = level.time + RESPAWN_PENT * 1000;
		} else {
			ent->nextthink = level.time + 30 * 1000; // spawn item in 30 seconds
		}
		ent->think = RespawnItem;
		return;
	}


	trap_LinkEntity (ent);
}


qboolean	itemRegistered[MAX_ITEMS];

/*
==================
G_CheckTeamItems
==================
*/
void G_CheckTeamItems( void ) {

	// Set up team stuff
	Team_InitGame();

	if( g_gametype.integer == GT_CTF ) {
		gitem_t	*item;

		// check for the two flags
		item = BG_FindItem( "Red Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map\n" );
		}
		item = BG_FindItem( "Blue Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map\n" );
		}
	}
#ifdef MISSIONPACK
	if( g_gametype.integer == GT_1FCTF ) {
		gitem_t	*item;

		// check for all three flags
		item = BG_FindItem( "Red Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map\n" );
		}
		item = BG_FindItem( "Blue Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map\n" );
		}
		item = BG_FindItem( "Neutral Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_neutralflag in map\n" );
		}
	}

	if( g_gametype.integer == GT_OBELISK ) {
		gentity_t	*ent;

		// check for the two obelisks
		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_redobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_redobelisk in map\n" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_blueobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_blueobelisk in map\n" );
		}
	}

	if( g_gametype.integer == GT_HARVESTER ) {
		gentity_t	*ent;

		// check for all three obelisks
		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_redobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_redobelisk in map\n" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_blueobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_blueobelisk in map\n" );
		}

		ent = NULL;
		ent = G_Find( ent, FOFS(classname), "team_neutralobelisk" );
		if( !ent ) {
			G_Printf( S_COLOR_YELLOW "WARNING: No team_neutralobelisk in map\n" );
		}
	}
#endif
}

/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) {
	memset( itemRegistered, 0, sizeof( itemRegistered ) );

	// players always start with the base weapon
	RegisterItem( BG_FindItemForWeapon( WP_BLASTER ) );

	// players always leave behind a backpack when fragged
	RegisterItem( BG_FindItemForBackpack() );

	// slaughtered players will leave some yummy food
	RegisterItem( BG_FindItemForFood(0) );

	// test weapon - mmp
	//RegisterItem( BG_FindItemForWeapon( WP_SUPER_SHOTGUN ) );

#ifdef MISSIONPACK
	if( g_gametype.integer == GT_HARVESTER ) {
		RegisterItem( BG_FindItem( "Red Cube" ) );
		RegisterItem( BG_FindItem( "Blue Cube" ) );
	}
#endif
}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item ) {
	if ( !item ) {
		G_Error( "RegisterItem: NULL" );
	}
	itemRegistered[ item - bg_itemlist ] = qtrue;
}


/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems( void ) {
	char	string[MAX_ITEMS+1];
	int		i;
	int		count;

	count = 0;
	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( itemRegistered[i] ) {
			count++;
			string[i] = '1';
		} else {
			string[i] = '0';
		}
	}
	string[ bg_numItems ] = 0;

	G_Printf( "%i items registered\n", count );
	trap_SetConfigstring(CS_ITEMS, string);
}

/*
============
G_ItemDisabled
============
*/
int G_ItemDisabled( gitem_t *item ) {

	char name[128];

	Com_sprintf(name, sizeof(name), "disable_%s", item->classname);
	return trap_Cvar_VariableIntegerValue( name );
}

/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnItem (gentity_t *ent, gitem_t *item) {
	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnFloat( "wait", "0", &ent->wait );

	RegisterItem( item );
	if ( G_ItemDisabled(item) )
		return;

	// disable armor spawns via ruleset
	if (!level.rs_armor) {
		if (item->giType == IT_ARMOR) {
			return;
		}
	}

	// if game type is not ctf, don't spawn ctf flags
	if ( item->giType == IT_TEAM ) {
		if ( item->giTag == PW_REDFLAG || item->giTag == PW_BLUEFLAG ) {
			if ( g_gametype.integer != GT_CTF ) {
				return;
			}
		}
	}

	if ( level.rs_matchMode >= MM_ALLWEAPONS_MAXAMMO ) {
		if (item->giType == IT_WEAPON) {
			return;
		}
		if ( level.rs_matchMode == MM_ALLWEAPONS_MAXAMMO ) {
			if (item->giType == IT_AMMO) {
				return;
			}
		}
	} else if ( level.rs_matchMode == MM_PICKUP_ALWAYS_NOAMMO ) {
		if (item->giType == IT_AMMO) {
			return;
		}
	}

	/*if ( level.rs_matchMode >= MM_ALLWEAPONS_MAXAMMO ) {
		if (item->giType == IT_WEAPON) {
			return;
		}
		if (item->giType == IT_AMMO) {
			return;
		}
	}*/

	ent->item = item;

	// some movers spawn on the second frame, so delay item
	// spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	ent->think = FinishSpawningItem;

	ent->physicsBounce = 0.50;		// items are bouncy

	if ( item->giType == IT_POWERUP ) {
		G_SoundIndex( "sound/items/poweruprespawn.wav" );
		G_SpawnFloat( "noglobalsound", "0", &ent->speed);
	}

#ifdef MISSIONPACK
	if ( item->giType == IT_PERSISTANT_POWERUP ) {
		ent->s.generic1 = ent->spawnflags;
	}
#endif

}


/*
================
G_BounceItem

================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

	// check for stop
	if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
		trace->endpos[2] += 1.0;	// make sure it is off ground
		SnapVector( trace->endpos );
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;
		return;
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_RunItem

================
*/
void G_RunItem( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if its groundentity has been set to none, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == ENTITYNUM_NONE ) {
		if ( ent->s.pos.trType != TR_GRAVITY ) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY ) {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;
	}
	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin,
		ent->r.ownerNum, mask );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) {
		tr.fraction = 0;
	}

	trap_LinkEntity( ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 ) {
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		if (ent->item && ent->item->giType == IT_TEAM) {
			Team_FreeEntity(ent);
		} else {
			G_FreeEntity( ent );
		}
		return;
	}

	G_BounceItem( ent, &tr );
}

