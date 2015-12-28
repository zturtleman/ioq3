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
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"

// for the voice chats
#ifdef MISSIONPACK
#include "../../ui/menudef.h"
#endif
//==========================================================================

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=======================
CG_AddToFragInfo

=======================
*/
static void CG_AddToFragInfo( const char *killer, qhandle_t deathIcon, const char *victom ) {
	int len;
	char *p, *ls;
	int lastcolor;
	int hudInfoHeight;

	hudInfoHeight = CHAT_HEIGHT;

	// killer
	len = 0;
	p = cgs.hudFragInfoKiller[cgs.hudFragInfoPos % hudInfoHeight];
	*p = 0;
	lastcolor = '7';
	ls = NULL;
	while (*killer) {
		if ( Q_IsColorString( killer ) ) {
			*p++ = *killer++;
			lastcolor = *killer;
			*p++ = *killer++;
			continue;
		}
		if (*killer == ' ') {
			ls = p;
		}
		*p++ = *killer++;
		len++;
	}
	*p = 0;

	// victom
	len = 0;
	p = cgs.hudFragInfoVictom[cgs.hudFragInfoPos % hudInfoHeight];
	*p = 0;
	lastcolor = '7';
	ls = NULL;
	while (*victom) {
		if ( Q_IsColorString( victom ) ) {
			*p++ = *victom++;
			lastcolor = *victom;
			*p++ = *victom++;
			continue;
		}
		if (*victom == ' ') {
			ls = p;
		}
		*p++ = *victom++;
		len++;
	}
	*p = 0;

	cgs.hudFragInfoDeathIcon[cgs.hudFragInfoPos % hudInfoHeight] = deathIcon;

	cgs.hudFragInfoTimes[cgs.hudFragInfoPos % hudInfoHeight] = cg.time;
	cgs.hudFragInfoPos++;

	if (cgs.hudFragInfoPos - cgs.hudFragInfoLastPos > hudInfoHeight)
		cgs.hudFragInfoLastPos = cgs.hudFragInfoPos - hudInfoHeight;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	const char	*targetInfo;
	const char	*attackerInfo;
	char		targetName[7+32+2];
	char		attackerName[7+32+2];
	char		rawTargetName[32];
	char		rawAttackerName[32];
	gender_t	gender;
	clientInfo_t	*ci;
	qhandle_t	deathIcon;
	int			filter;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}
	ci = &cgs.clientinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	targetName[0] = '\0';
	if (cgs.clientinfo[target].team == TEAM_RED) {
		strcat( targetName, SHORT_RED_TEAM_NAME );
	}
	else if (cgs.clientinfo[target].team == TEAM_BLUE) {
		strcat( targetName, SHORT_BLUE_TEAM_NAME );
	}
	Q_strncpyz( rawTargetName, Info_ValueForKey( targetInfo, "n" ), sizeof(rawTargetName)/* - 2*/);
	strcat( targetName, rawTargetName );
	strcat( targetName, S_COLOR_WHITE );

	message2 = "";

	deathIcon = cgs.media.iconSkull; // default death icon
	filter = hud_notifyBoxFilter.integer & NF_FRAG;

	// check for single client messages
	if(attacker != ENTITYNUM_WORLD)
		message = NULL;
	else {
		switch( mod ) {
		case MOD_NOTHING:
			return; // don't display anything
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a turd";
			break;
		case MOD_SLIME:
			message = "gulped a load of slime";
			break;
		case MOD_LAVA:
			message = "visits the Volcano God";
			deathIcon = cgs.media.iconSkullFlame;
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		case MOD_DRAGON_FRIED:
			message = "was fried by a Dragon";
			deathIcon = cgs.media.iconSkullFlame;
			break;
		case MOD_SHUB_NIGGURATH:
			message = "became one with Shub-Niggurath";
			deathIcon = cgs.media.iconSkullTele;
			break;
		/*default:
			message = NULL;
			break;*/
		}
	}

	if (attacker == target) {
		gender = ci->gender;
		switch (mod) {
		case MOD_GRENADE_SPLASH:
			message = "tries to put the pin back in";
			deathIcon = cg_weapons[WP_GRENADE_LAUNCHER].weaponIcon;
			break;
		case MOD_ROCKET_SPLASH:
			message = "discovers blast radius";
			deathIcon = cg_weapons[WP_ROCKET_LAUNCHER].weaponIcon;
			break;
		case MOD_PLASMA_SPLASH:
			message = "became one with the plasma";
			deathIcon = cg_weapons[WP_PLASMAGUN].weaponIcon;
			break;
		case MOD_SPREADSHOT_SPLASH:
			message = "wanted to see how the spreadshot worked";
			deathIcon = cg_weapons[WP_SPREADSHOT].weaponIcon;
			break;
		case MOD_FLAMETHROWER:
			message = "got burnt up";
			deathIcon = cg_weapons[WP_FLAMETHROWER].weaponIcon;
			break;
		case MOD_LIGHTNING_DISCHARGE:
			message = "discharged the lightning gun";
			deathIcon = cg_weapons[WP_LIGHTNING].weaponIcon;
			break;
		case MOD_CRUSH:
			message = "was crushed";
			break;
		default:
			message = "becomes bored with life";
			break;
		}
	}

	if (message) {
		CG_Printf( "%s %s\n", targetName, message);
		CG_AddToHUDInfo ( hud_notifyBoxRoute.integer, va("%s %s", targetName, message, 0), 0, filter );

		CG_AddToFragInfo ( "", deathIcon, targetName ); // mmp
		return;
	}

	// TODO: add cp function to megadraw

	// check for kill messages from the current clientNum
	if ( attacker == cg.snap->ps.clientNum ) {
		char	*s;

		if ( cgs.gametype < GT_TEAM ) {
			s = va("You fragged %s\n%s place with %i", targetName,
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );

				if( cg_killSound.integer > 0 && cg_killSound.integer < 10 )
					trap_S_StartLocalSound( cgs.media.killSound[cg_killSound.integer - 1], CHAN_LOCAL_SOUND );
				//if (cg_battleText.integer > 0)
					CG_Printf ("^5Thou hast done well in defeating ^7%s^5.\n", targetName);

		} else {
//			s = va("You fragged %s", targetName );

			if (ent->generic1) {
				s = va("You fragged your\n^1TEAMMATE^7 %s", targetName );
				if( cg_teamKillSound.integer > 0 && cg_teamKillSound.integer < 10 )
					trap_S_StartLocalSound( cgs.media.killSound[cg_teamKillSound.integer - 1], CHAN_LOCAL_SOUND );
			} else {
				s = va("You fragged %s", targetName );
				if( cg_killSound.integer > 0 && cg_killSound.integer < 10 )
					trap_S_StartLocalSound( cgs.media.killSound[cg_killSound.integer - 1], CHAN_LOCAL_SOUND );
				//if (cg_battleText.integer > 0)
					CG_Printf ("^5Thou hast done well in defeating ^7%s^5.\n", targetName);
			}
		}
#ifdef MISSIONPACK
		if (!(cg_singlePlayerActive.integer && cg_cameraOrbit.integer)) {
			CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		}
#else
		CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
#endif

		// print the text message as well
	}

	// check for double client messages
	if ( !attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} else {
		attackerName[0] = '\0';
		if (cgs.clientinfo[attacker].team == TEAM_RED) {
			strcat( attackerName, SHORT_RED_TEAM_NAME );
		}
		else if (cgs.clientinfo[attacker].team == TEAM_BLUE) {
			strcat( attackerName, SHORT_BLUE_TEAM_NAME );
		}
		Q_strncpyz( rawAttackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(rawAttackerName)/* - 2*/);
		strcat( attackerName, rawAttackerName );
		strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
		}
	}

	// TODO - have the game check if the name ends with a 's',
	// so the message won't show something like;
	// "Sonic accepts Miles's discharge",
	// where "Miles's" should be "Miles'"
	if ( attacker != ENTITYNUM_WORLD ) {
		switch (mod) {
		case MOD_GRAPPLE:
			message = "was caught by";
			break;
		case MOD_BLASTER:
			message = "was blasted by";
			deathIcon = cg_weapons[WP_BLASTER].weaponIcon;
			break;
		case MOD_GAUNTLET:
			message = "was sliced up by";
			deathIcon = cg_weapons[WP_GAUNTLET].weaponIcon;
			break;
		case MOD_CHAINGUN:
			message = "was perforated by";
			deathIcon = cg_weapons[WP_CHAINGUN].weaponIcon;
			break;
		case MOD_SHOTGUN:
			message = "was gunned down by";
			deathIcon = cg_weapons[WP_SHOTGUN].weaponIcon;
			break;
		case MOD_SUPER_SHOTGUN:
			message = "discovers how powerful";
			message2 = "'s super shotgun is";
			deathIcon = cg_weapons[WP_SUPER_SHOTGUN].weaponIcon;
			break;
		case MOD_GRENADE:
			message = "mistakes";
			message2 = "'s grenade for a pineapple";
			deathIcon = cg_weapons[WP_GRENADE_LAUNCHER].weaponIcon;
			break;
		case MOD_GRENADE_SPLASH:
			message = "was shredded by";
			message2 = "'s shrapnel";
			deathIcon = cg_weapons[WP_GRENADE_LAUNCHER].weaponIcon;
			break;
		case MOD_ROCKET:
			message = "rides";
			message2 = "'s rocket";
			deathIcon = cg_weapons[WP_ROCKET_LAUNCHER].weaponIcon;
			break;
		case MOD_ROCKET_SPLASH:
			message = "didn't dodge";
			message2 = "'s rocket";
			deathIcon = cg_weapons[WP_ROCKET_LAUNCHER].weaponIcon;
			break;
		case MOD_PLASMA:
			message = "was melted by";
			message2 = "'s plasmagun";
			deathIcon = cg_weapons[WP_PLASMAGUN].weaponIcon;
			break;
		case MOD_PLASMA_SPLASH:
			message = "was melted by";
			message2 = "'s plasmagun";
			deathIcon = cg_weapons[WP_PLASMAGUN].weaponIcon;
			break;
		case MOD_SPREADSHOT:
			message = "was overwhelmed by";
			message2 = "'s spreadshot";
			deathIcon = cg_weapons[WP_SPREADSHOT].weaponIcon;
			break;
		case MOD_SPREADSHOT_SPLASH:
			message = "was overwhelmed by";
			message2 = "'s spreadshot";
			deathIcon = cg_weapons[WP_SPREADSHOT].weaponIcon;
			break;
		case MOD_LIGHTNING:
			message = "accepts";
			message2 = "'s shaft";
			deathIcon = cg_weapons[WP_LIGHTNING].weaponIcon;
			break;
		case MOD_LIGHTNING_DISCHARGE:
			/*message = "has been discharged by";*/
			message = "accepts";
			message2 = "'s discharge"; // this version is funnier
			deathIcon = cg_weapons[WP_LIGHTNING].weaponIcon;
			break;
		case MOD_FLAMETHROWER:
			message = "was toasted up and buttered by";
			deathIcon = cg_weapons[WP_FLAMETHROWER].weaponIcon;
			break;
		case MOD_TELEFRAG:
			message = "was telefragged by";
			/*message = "tried to invade";
			message2 = "'s personal space";*/
			deathIcon = cgs.media.iconSkullTele;
			break;
		case MOD_DEFLECTED_TELEFRAG:
			message = "'s telefrag was no match for Satan's power, which benefited";
			deathIcon = cgs.media.iconSkullTele;
			break;
		case MOD_INSTAGIBBED:
			message = "was instagibbed by";
			break;
		case MOD_CRUSH:
			message = "was crushed by";
			/*deathIcon = cgs.media.iconSkullTele;*/
			break;
		default:
			message = "was killed by";
			break;
		}

		if (message) {
			if (message) {
				CG_Printf( "%s %s %s%s\n",
						targetName, message, attackerName, message2);
				CG_AddToHUDInfo ( hud_notifyBoxRoute.integer, va( "%s %s %s%s\n",
						targetName, message, attackerName, message2, 0), 0, filter );
				CG_AddToFragInfo ( attackerName, deathIcon, targetName );
				return;
			}
			/*CG_Printf( "%s %s %s%s\n",
				targetName, message, attackerName, message2);
			return;*/
		}
	}

	// we don't know what it was
	CG_Printf( "%s died\n", targetName );
	CG_AddToHUDInfo ( hud_notifyBoxRoute.integer, va("%s died", targetName, 0), 0, filter );

	CG_AddToFragInfo ( "", deathIcon, targetName ); // mmp
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
	clientInfo_t *ci;
	int			itemNum, clientNum;
	gitem_t		*item;
	entityState_t *es;

	es = &cent->currentState;

	itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
	if ( itemNum < 0 || itemNum > HI_NUM_HOLDABLE ) {
		itemNum = 0;
	}

	// print a message if the local player
	if ( es->number == cg.snap->ps.clientNum ) {
		if ( !itemNum ) {
			CG_CenterPrint( "No item to use", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		} else {
			item = BG_FindItemForHoldable( itemNum );
			CG_CenterPrint( va("Use %s", item->pickup_name), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		}
	}

	switch ( itemNum ) {
	default:
	case HI_NONE:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

	case HI_TELEPORTER:
		break;

	case HI_MEDKIT:
		clientNum = cent->currentState.clientNum;
		if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
			ci = &cgs.clientinfo[ clientNum ];
			ci->medkitUsageTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;

#ifdef MISSIONPACK
	case HI_KAMIKAZE:
		break;

	case HI_PORTAL:
		break;
	case HI_INVULNERABILITY:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useInvulnerabilitySound );
		break;
#endif
	}

}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum ) {
	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;
	// see if it should be the grabbed weapon
	/*if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
		// select it immediately
		if ( cg_autoswitch.integer && bg_itemlist[itemNum].giTag != WP_MACHINEGUN ) {
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = bg_itemlist[itemNum].giTag;
		}
	}*/

	// flash screen on item pickup
	/*if ( bg_itemlist[itemNum].giType != IT_POWERUP ) {*/
		cg.itemGetFlashTime = cg.time + ITEM_GET_FLASH;
	/*}*/

}

/*
================
CG_WaterLevel

Returns waterlevel for entity origin
================
*/
int CG_WaterLevel(centity_t *cent) {
	vec3_t point;
	int contents, sample1, sample2, anim, waterlevel;
	int viewheight;

	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

	if (anim == LEGS_WALKCR || anim == LEGS_IDLECR) {
		viewheight = CROUCH_VIEWHEIGHT;
	} else {
		viewheight = DEFAULT_VIEWHEIGHT;
	}

	//
	// get waterlevel, accounting for ducking
	//
	waterlevel = 0;

	point[0] = cent->lerpOrigin[0];
	point[1] = cent->lerpOrigin[1];
	point[2] = cent->lerpOrigin[2] + MINS_Z + 1;
	contents = CG_PointContents(point, -1);

	if (contents & MASK_WATER) {
		sample2 = viewheight - MINS_Z;
		sample1 = sample2 / 2;
		waterlevel = 1;
		point[2] = cent->lerpOrigin[2] + MINS_Z + sample1;
		contents = CG_PointContents(point, -1);

		if (contents & MASK_WATER) {
			waterlevel = 2;
			point[2] = cent->lerpOrigin[2] + MINS_Z + sample2;
			contents = CG_PointContents(point, -1);

			if (contents & MASK_WATER) {
				waterlevel = 3;
			}
		}
	}

	return waterlevel;
}

/*
================
CG_LavaPain

Returns if swimming in lava
================
*/

qboolean CG_LavaPain(centity_t *cent) {
	vec3_t		point;
	int			contents;

	VectorCopy(cent->lerpOrigin, point);
	point[2] += MINS_Z + 1;

	contents = CG_PointContents(point, -1);

	if (contents & CONTENTS_LAVA) {
		return qtrue;
	}
	return qfalse;
}

/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than four pain sounds a second, was two
	if ( cg.time - cent->pe.painTime < 250 ) { // was 500
		return;
	}

	// determine if player should scream from the pain of swimming in lava
	if (CG_LavaPain(cent) == qfalse) {

		// play a gurp sound along with the normal pain sound
		if (CG_WaterLevel(cent) == 3) {
			if (rand()&1) {
				trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, "sound/player/gurp1.wav"));
			} else {
				trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, "sound/player/gurp2.wav"));
			}
		}

		if ( health < 25 ) {
			snd = "*pain25_1.wav";
		} else if ( health < 50 ) {
			snd = "*pain50_1.wav";
		} else if ( health < 75 ) {
			snd = "*pain75_1.wav";
		} else {
			snd = "*pain100_1.wav";
		}
	} else {
		snd = "*lava.wav";
	}

	trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, snd));

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}



/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	clientInfo_t	*ci;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( cg_debugEvents.integer ) {
		CG_Printf( "ent:%3i  event:%3i ", es->number, event );
	}

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	switch ( event ) {
	//
	// movement generated events
	//
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ ci->footsteps ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY,
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;


	case EV_FALL_SHORT:
		DEBUGNAME("EV_FALL_SHORT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -8;
			cg.landTime = cg.time;
		}
		break;

	// modified the pain sounds, since the far fall sounded less painful than the med fall
	// med fall is now fall1, and far fall is fall2
	// - mmp

	case EV_FALL_MEDIUM:
		DEBUGNAME("EV_FALL_MEDIUM");
		/*trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) ); // use normal pain sound */

		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) ); // use fall grunt sound
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this - mmp
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -16;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_FAR:
		DEBUGNAME("EV_FALL_FAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall2.wav" ) ); // use heavy fall grunt sound
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -24;
			cg.landTime = cg.time;
		}
		break;

	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
			cg_nopredict.integer || cg_synchronousClients.integer ) {
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
//		CG_Printf( "EV_JUMP_PAD w/effect #%i\n", es->eventParm );
		{
			vec3_t			up = {0, 0, 1};


			CG_SmokePuff( cent->lerpOrigin, up,
						  32,
						  1, 1, 1, 0.33f,
						  1000,
						  cg.time, 0,
						  LEF_PUFF_DONT_SCALE,
						  cgs.media.smokePuffShader );
		}

		// boing sound at origin, jump sound on player
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound );
//		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, va("*jump%c.wav", '1'+(rand()&3) ) ) );
		break;

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
//		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, va("*jump%c.wav", '1'+(rand()&3) ) ) );
		break;
	case EV_TAUNT:
		DEBUGNAME("EV_TAUNT");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, va("*taunt%c.wav", '1'+((cg.time / 1024)&3) ) ) );
		break;
//		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
/*	case EV_TAUNT1:
		DEBUGNAME("EV_TAUNT1");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt1.wav" ) );
		break;
	case EV_TAUNT2:
		DEBUGNAME("EV_TAUNT2");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt2.wav" ) );
		break;
	case EV_TAUNT3:
		DEBUGNAME("EV_TAUNT3");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt3.wav" ) );
		break;
	case EV_TAUNT4:
		DEBUGNAME("EV_TAUNT4");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt4.wav" ) );
		break;*/
#ifdef MISSIONPACK
	case EV_TAUNT_YES:
		DEBUGNAME("EV_TAUNT_YES");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_YES);
		break;
	case EV_TAUNT_NO:
		DEBUGNAME("EV_TAUNT_NO");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_NO);
		break;
	case EV_TAUNT_FOLLOWME:
		DEBUGNAME("EV_TAUNT_FOLLOWME");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_FOLLOWME);
		break;
	case EV_TAUNT_GETFLAG:
		DEBUGNAME("EV_TAUNT_GETFLAG");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONGETFLAG);
		break;
	case EV_TAUNT_GUARDBASE:
		DEBUGNAME("EV_TAUNT_GUARDBASE");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONDEFENSE);
		break;
	case EV_TAUNT_PATROL:
		DEBUGNAME("EV_TAUNT_PATROL");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONPATROL);
		break;
#endif
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			// powerups and team items will have a separate global sound, this one
			// will be played at prediction time
			/*if ( item->giType == IT_POWERUP || item->giType == IT_TEAM) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
			} else if (item->giType == IT_PERSISTANT_POWERUP) {
#ifdef MISSIONPACK
				switch (item->giTag ) {
					case PW_SCOUT:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.scoutSound );
					break;
					case PW_GUARD:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.guardSound );
					break;
					case PW_DOUBLER:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.doublerSound );
					break;
					case PW_AMMOREGEN:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.ammoregenSound );
					break;
				}
#endif
			} else {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}*/

			// mmp
			trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	case EV_GLOBAL_ITEM_PICKUP:
		DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];
			// powerup pickups are global
			if( item->pickup_sound ) {
				trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	case EV_GLOBAL_EXPLOSION:
		DEBUGNAME("EV_GLOBAL_EXPLOSION");
		//trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.media.sfx_exp_lo );
		trap_S_StartLocalSound( cgs.media.sfx_exp_lo, CHAN_AUTO );
		//trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound, qfalse ) );
		break;

	//
	// weapon events
	//
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
		// mmp - FIXME: this is commented out, due to repeated sounds, in part due to server prediction
//		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
		break;
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
		CG_FireWeapon( cent );
		break;
	case EV_REMOVE_WEAPON:
		DEBUGNAME("EV_REMOVE_WEAPON");
		CG_RemoveWeapon();
		break;

	case EV_USE_ITEM0:
		DEBUGNAME("EV_USE_ITEM0");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM1:
		DEBUGNAME("EV_USE_ITEM1");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM2:
		DEBUGNAME("EV_USE_ITEM2");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM3:
		DEBUGNAME("EV_USE_ITEM3");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM4:
		DEBUGNAME("EV_USE_ITEM4");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM5:
		DEBUGNAME("EV_USE_ITEM5");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM6:
		DEBUGNAME("EV_USE_ITEM6");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM7:
		DEBUGNAME("EV_USE_ITEM7");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM8:
		DEBUGNAME("EV_USE_ITEM8");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM9:
		DEBUGNAME("EV_USE_ITEM9");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM10:
		DEBUGNAME("EV_USE_ITEM10");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM11:
		DEBUGNAME("EV_USE_ITEM11");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM12:
		DEBUGNAME("EV_USE_ITEM12");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM13:
		DEBUGNAME("EV_USE_ITEM13");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM14:
		DEBUGNAME("EV_USE_ITEM14");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM15:
		DEBUGNAME("EV_USE_ITEM15");
		CG_UseItem( cent );
		break;

	//=================================================================

	//
	// other events
	//
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_SpawnEffect( position);
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_SpawnEffect(  position);
		break;

	// item despawns
	case EV_ITEM_POP:
		DEBUGNAME("EV_ITEM_POP");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;
	// item spawns
	case EV_ITEM_RESPAWN:
		DEBUGNAME("EV_ITEM_RESPAWN");
		cent->miscTime = cg.time;	// scale up from this
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;
	// top tier tiem spawns (eg: RA, MH, power-ups)
	case EV_ITEM_SUPERRESPAWN:
		DEBUGNAME("EV_ITEM_SUPERRESPAWN");
		cent->miscTime = cg.time;	// scale up from this
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.superRespawnSound );
		break;

	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
		if ( rand() & 1 ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound );
		}
		break;

#ifdef MISSIONPACK
	case EV_PROXIMITY_MINE_STICK:
		DEBUGNAME("EV_PROXIMITY_MINE_STICK");
		if( es->eventParm & SURF_FLESH ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimplSound );
		} else 	if( es->eventParm & SURF_METALSTEPS ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpmSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpdSound );
		}
		break;

	case EV_PROXIMITY_MINE_TRIGGER:
		DEBUGNAME("EV_PROXIMITY_MINE_TRIGGER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbactvSound );
		break;
	case EV_KAMIKAZE:
		DEBUGNAME("EV_KAMIKAZE");
		CG_KamikazeEffect( cent->lerpOrigin );
		break;
	case EV_OBELISKEXPLODE:
		DEBUGNAME("EV_OBELISKEXPLODE");
		CG_ObeliskExplode( cent->lerpOrigin, es->eventParm );
		break;
	case EV_OBELISKPAIN:
		DEBUGNAME("EV_OBELISKPAIN");
		CG_ObeliskPain( cent->lerpOrigin );
		break;
	case EV_INVUL_IMPACT:
		DEBUGNAME("EV_INVUL_IMPACT");
		CG_InvulnerabilityImpact( cent->lerpOrigin, cent->currentState.angles );
		break;
	case EV_JUICED:
		DEBUGNAME("EV_JUICED");
		CG_InvulnerabilityJuiced( cent->lerpOrigin );
		break;
	case EV_LIGHTNINGBOLT:
		DEBUGNAME("EV_LIGHTNINGBOLT");
		CG_LightningBoltBeam(es->origin2, es->pos.trBase);
		break;
#endif
	case EV_SCOREPLUM:
		DEBUGNAME("EV_SCOREPLUM");
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;
	case EV_NUMPLUM:
		DEBUGNAME("EV_NUMPLUM");
		// TODO: make into own function
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;

	//
	// missile impacts
	//
	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum );
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT );
		//CG_Printf ("^9DEBUG: CG EV_MISSILE_MISS\n");
		break;

	case EV_MISSILE_MISS_METAL:
		DEBUGNAME("EV_MISSILE_MISS_METAL");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitWall( es->weapon, 0, position, dir, IMPACTSOUND_METAL );
		break;

#ifdef MISSIONPACK
	case EV_RAILTRAIL:
		DEBUGNAME("EV_RAILTRAIL");
		cent->currentState.weapon = WP_RAILGUN;

//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( !(es->clientNum == cg.predictedPlayerState.clientNum &&
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 16) ) ) {

			if(es->clientNum == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
			{
				if(cg_drawGun.integer == 2)
					VectorMA(es->origin2, 8, cg.refdef.viewaxis[1], es->origin2);
				else if(cg_drawGun.integer == 3)
					VectorMA(es->origin2, 4, cg.refdef.viewaxis[1], es->origin2);
			}

			CG_RailTrail(ci, es->origin2, es->pos.trBase);

			// if the end was on a nomark surface, don't make an explosion
			if ( es->eventParm != 255 ) {
				ByteToDir( es->eventParm, dir );
				CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT );
			}

		}
//unlagged - attack prediction #2
#endif

		break;

	case EV_BULLET_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( !(es->clientNum == cg.predictedPlayerState.clientNum &&
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 2) ) ) {
			ByteToDir( es->eventParm, dir );
			CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
		}
//unlagged - attack prediction #2
		break;

	case EV_BULLET_HIT_FLESH:
		DEBUGNAME("EV_BULLET_HIT_FLESH");
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( !(es->clientNum == cg.predictedPlayerState.clientNum &&
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 2) ) ) {
			CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
		}
//unlagged - attack prediction #2
		break;

	case EV_SHOTGUN:
		DEBUGNAME("EV_SHOTGUN");
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( !(es->clientNum == cg.predictedPlayerState.clientNum &&
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 4) ) ) {
			CG_ShotgunFire( es );
		}
//unlagged - attack prediction #2
		break;

	case EV_SUPER_SHOTGUN:
		DEBUGNAME("EV_SUPER_SHOTGUN");
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( !(es->clientNum == cg.predictedPlayerState.clientNum &&
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 4) ) ) {
			CG_SuperShotgunFire( es );
		}
//unlagged - attack prediction #2
		break;

// The SARACEN's Lightning Discharge - START
	case EV_LIGHTNING_DISCHARGE:
		DEBUGNAME("EV_LIGHTNING_DISCHARGE");
		CG_Lightning_Discharge (position, es->eventParm);       // eventParm is duration/size
		break;
// The SARACEN's Lightning Discharge - END

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_TEAM_SOUND:	// play from the player's head so it never diminishes
		{
			DEBUGNAME("EV_GLOBAL_TEAM_SOUND");
			switch( es->eventParm ) {
				case GTS_RED_CAPTURE: // CTF: red team captured the blue flag, 1FCTF: red team captured the neutral flag
					if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_BLUE_CAPTURE: // CTF: blue team captured the red flag, 1FCTF: blue team captured the neutral flag
					if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_RED_RETURN: // CTF: blue flag returned, 1FCTF: never used
					if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					//CG_AddBufferedSound( cgs.media.blueFlagReturnedSound );
					break;
				case GTS_BLUE_RETURN: // CTF red flag returned, 1FCTF: neutral flag returned
					if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					//CG_AddBufferedSound( cgs.media.redFlagReturnedSound );
					break;

				case GTS_RED_TAKEN: // CTF: red team took blue flag, 1FCTF: blue team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds

					if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
						trap_S_StartLocalSound( cgs.media.takenOpponentSound, CHAN_ANNOUNCER );
					} else {
						trap_S_StartLocalSound( cgs.media.takenYourTeamSound, CHAN_ANNOUNCER );
					}

					/*if (cg.snap->ps.powerups[PW_BLUEFLAG] || cg.snap->ps.powerups[PW_NEUTRALFLAG]) {
					}
					else {
						if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
								CG_AddBufferedSound( cgs.media.takenOpponentSound );
							CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
								CG_AddBufferedSound( cgs.media.takenYourTeamSound );
 							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}*/
					break;
				case GTS_BLUE_TAKEN: // CTF: blue team took the red flag, 1FCTF red team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds

					if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
						trap_S_StartLocalSound( cgs.media.takenOpponentSound, CHAN_ANNOUNCER );
					} else {
						trap_S_StartLocalSound( cgs.media.takenYourTeamSound, CHAN_ANNOUNCER );
					}

					/*if (cg.snap->ps.powerups[PW_REDFLAG] || cg.snap->ps.powerups[PW_NEUTRALFLAG]) {
					}
					else {
						if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
								CG_AddBufferedSound( cgs.media.takenOpponentSound );
							CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
								CG_AddBufferedSound( cgs.media.takenYourTeamSound );
							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}*/
					break;
#ifdef MISSIONPACK
				case GTS_REDOBELISK_ATTACKED: // Overload: red obelisk is being attacked
					if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;
				case GTS_BLUEOBELISK_ATTACKED: // Overload: blue obelisk is being attacked
					if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;
#endif

				case GTS_REDTEAM_SCORED:
					CG_AddBufferedSound(cgs.media.redScoredSound);
					break;
				case GTS_BLUETEAM_SCORED:
					CG_AddBufferedSound(cgs.media.blueScoredSound);
					break;
				case GTS_REDTEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.redLeadsSound);
					break;
				case GTS_BLUETEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.blueLeadsSound);
					break;
				case GTS_TEAMS_ARE_TIED:
					CG_AddBufferedSound( cgs.media.teamsTiedSound );
					break;
#ifdef MISSIONPACK
				case GTS_KAMIKAZE:
					trap_S_StartLocalSound(cgs.media.kamikazeFarSound, CHAN_ANNOUNCER);
					break;
#endif
				default:
					break;
			}
			break;
		}

	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");
		if ( cent->currentState.number != cg.snap->ps.clientNum ) {
			CG_PainEvent( cent, es->eventParm );
		}
		break;

	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME("EV_DEATHx");

		if (CG_WaterLevel(cent) == 3) {
			trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "*drown.wav"));
		} else {
			trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, va("*death%i.wav", event - EV_DEATH1 + 1)));
		}

		break;


	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
		CG_Obituary( es );
		break;

	//
	// powerup events
	//
	case EV_POWERUP_QUAD:
		DEBUGNAME("EV_POWERUP_QUAD");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_QUAD;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.quadSound );
		break;
	case EV_POWERUP_PENT:
		DEBUGNAME("EV_POWERUP_PENT");
		/*CG_Printf( "^2DEBUG: PENT_ACT\n", es->number, event );*/
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_PENT;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectSound );
		break;
	case EV_POWERUP_REGEN:
		DEBUGNAME("EV_POWERUP_REGEN");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_REGEN;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.regenSound );
		break;

	case EV_GIB_PLAYER:
		DEBUGNAME("EV_GIB_PLAYER");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		if ( !(es->eFlags & EF_KAMIKAZE) ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
		}
		CG_GibPlayer( cent->lerpOrigin );
		break;

	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		trap_S_StopLoopingSound( es->number );
		es->loopSound = 0;
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

	default:
		DEBUGNAME("UNKNOWN");
		CG_Error( "Unknown event: %i", event );
		break;
	}

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
		// if this is a player event set the entity number of the client entity number
		if ( cent->currentState.eFlags & EF_PLAYER_EVENT ) {
			cent->currentState.number = cent->currentState.otherEntityNum;
		}

		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}

