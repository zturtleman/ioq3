/*
===========================================================================
Copyright (C) 2012 MMP Games.

This file is part of MFArena source code.

MFArena source code is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

MFArena source code is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with MFArena source code; if not, write to the Free Software Foundation,
Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*
===========================================================================
Visit 'www.mmpgames.wordpress.com' for more information, on MFArena.
===========================================================================
*/

//
// cg_megadraw -- MFA HUD functions (styled scoreboard, hud, and custom hud)
#include "cg_local.h"
//#include "../ui/ui_shared.h"

/*
===========================================================================
Draw functions
===========================================================================
*/

#define	HUD_HEALTH_OVER				100
#define	HUD_HEALTH_LOW				25
#define	HUD_HEALTH_VERY_LOW			10

/*
=================
HUD_FuncColorGet
=================
*/

#define	HUD_NUM_OF_COLOR_FIELDS		4
#define	HUD_SIZE_OF_HEX_COLORS		6

void HUD_FuncColorGet ( const char* s, vec4_t color ) {

	int	cfield = 0;
	int	hexPos;
	int	colorInt;
	float	colorFloat;

	// if string is empty, make it completely transparent
	if ( *s == '\0' ) {
		color[0] = color[1] = color[2] = color[3] = 0.0;
		return;
	}

	// if we're using a color string (^#), then use it
	if ( Q_IsColorString( s ) ) {
		//memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) ); // doesn't work?
		//memcpy( color, colorTest, sizeof( color ) ); // didn't work either, wtf

		// why the fuck doesn't memcpy() work here?  :b
		color[0] = g_color_table[ColorIndex(*(s+1))][0];
		color[1] = g_color_table[ColorIndex(*(s+1))][1];
		color[2] = g_color_table[ColorIndex(*(s+1))][2];
		color[3] = g_color_table[ColorIndex(*(s+1))][3];
		return;
	}

	// use hex values instead (RRGGBBAA)
	while ( *s && cfield < HUD_NUM_OF_COLOR_FIELDS ) {

		if ( *s == '\0' ) {
			// well, it's the player's fault for using a faulty value
			return;
		}

		// get high 4 bits of byte
		if ( *s >= '0' && *s <= '9' ) {
			colorInt = *s - '0';
		} else
		if ( *s >= 'A' && *s <= 'F' ) {
			colorInt = *s - ( 'A' - 10);
		} else
		if ( *s >= 'a' && *s <= 'f' ) {
			colorInt = *s - ( 'a' - 10);
		} else {
			colorInt = 15;
		}

		s++;
		colorInt <<= 4;

		if ( *s == '\0' ) {
			// well, it's the player's fault for using a faulty value
			return;
		}

		// get low 4 bits of byte
		if ( *s >= '0' && *s <= '9' ) {
			colorInt |= *s - '0';
		} else
		if ( *s >= 'A' && *s <= 'F' ) {
			colorInt |= *s - ( 'A' - 10);
		} else
		if ( *s >= 'a' && *s <= 'f' ) {
			colorInt |= *s - ( 'a' - 10);
		} else {
			colorInt |= 15;
		}

		s++;
		color[cfield] = (float)colorInt / 256; // convert into a usable float value
		cfield++;

	}
}

/*
=================
HUD_FuncLock
=================
*/

void HUD_FuncPosLock( const char* str, int posLock, int xpos, int xoff, int ypos, int sizeY, int style,
							float scaleX, float scaleY, vec4_t color, qboolean dcolor ) {

	if ( posLock > 0 && posLock <= HUD_POS_LOCK_TYPES ) {
		// text alignment is stripped
		style &= (~UI_FORMATMASK);

		/*
		123
		456
		*/

		// text position is stripped as well
		switch( posLock ) {
			case 2:
			case 5:
				style |= UI_CENTER;
				xpos = 320 * hud_aspectRatioScale.value;
				break;

			case 3:
			case 6:
				style |= UI_RIGHT;
				xpos = 640 * hud_aspectRatioScale.value - 4;
				break;

			default:
				xpos = 4;
		}

		if ( posLock > 3 ) {
			cg.ypos[posLock-1] -= sizeY * scaleY + 4;
		}
		UI_DrawString( xpos, cg.ypos[posLock-1], str, style, scaleX, scaleY, color, dcolor );
		if ( posLock < 4 ) {
			cg.ypos[posLock-1] += sizeY * scaleY + 4;
		}

		return;
	} else {
		UI_DrawString( xpos + xoff, ypos, str, style, scaleX, scaleY, color, dcolor );
	}
}


//#define UI_LEFT		0x00000000	// default
//#define UI_CENTER		0x00000001
//#define UI_RIGHT		0x00000002
//#define UI_FORMATMASK		0x00000007
//#define UI_SMALLFONT		0x00000010
//#define UI_BIGFONT		0x00000020	// default
//#define UI_GIANTFONT		0x00000040
//#define UI_DROPSHADOW		0x00000800
//#define UI_BLINK		0x00001000
//#define UI_INVERSE		0x00002000
//#define UI_PULSE		0x00004000

/*
=================
HUD_FuncStyleSet
=================
*/

#define HUD_SHADOW		0x00000001

static HUD_FuncStyleSet( int align, int style ) {
	int	styleOut = 0;

	if ( style & HUD_SHADOW ) {
		styleOut |= UI_DROPSHADOW;
	}

	switch ( align ) {
		case 1:
			styleOut |= UI_CENTER;
			break;
		case 2:
			styleOut |= UI_RIGHT;
			break;
	}

	return styleOut;

}

/*
=================
HUD_DrawTest
=================
*/
void HUD_DrawTest( int xpos, int ypos, int style, float scale, vec4_t color ) {

	char		string[1024];
	int		y = ypos;
	int		w;

	Com_sprintf(string, sizeof(string), "^B0123456789ABCDEF");
	UI_DrawString( xpos, ypos+16*1, string, style, 0.5, 1.0, color, qfalse );

	/*Com_sprintf(string, sizeof(string), "^0!^1!^2!^3!^4!^5!^6!^7!^8!^9!");
	UI_DrawString( xpos, ypos+16*1, string, style, 1.0, color, qfalse );
	Com_sprintf(string, sizeof(string), "^a!^b!^c!^d!^e!^f!^g!^h!^i!^j!^k!^l!^m!");
	UI_DrawString( xpos, ypos+16*2, string, style, 1.0, color, qfalse );
	Com_sprintf(string, sizeof(string), "^n!^o!^p!^q!^r!^s!^t!^u!^v!^w!^x!^y!^z!");
	UI_DrawString( xpos, ypos+16*3, string, style, 1.0, color, qfalse );
	Com_sprintf(string, sizeof(string), "^A!^B!^C!^D!^E!^F!^G!^H!^I!^J!^K!^L!^M!");
	UI_DrawString( xpos, ypos+16*4, string, style, 1.0, color, qfalse );
	Com_sprintf(string, sizeof(string), "^N!^O!^P!^Q!^R!^S!^T!^U!^V!^W!^X!^Y!^Z!");
	UI_DrawString( xpos, ypos+16*5, string, style, 1.0, color, qfalse );*/

}

void HUD_DrawPrototypeNotice( int xpos, int ypos, int mtype, int style, vec4_t color ) {

	char		string[1024];
	int		y = ypos;
	int		w;
	vec4_t		blkBG;

	/*
	blkBG[0] = blkBG[1] = blkBG[2] = 0.0;
	blkBG[3] = 0.75;

	if (mtype) {
		CG_FillRect ( xpos - 4, ypos - 4, 284, 98, blkBG );
	} else {
		CG_FillRect ( xpos - 4, ypos - 4, 284, 74, blkBG );
	}

	Com_sprintf(string, sizeof(string), "^BTHIS IS A PROTOTYPE DEMO:");
	UI_DrawCustomProportionalString( xpos, y, string, style, 0.8, color, qfalse );
	y += 18;
	Com_sprintf(string, sizeof(string), "^OMany features are not complete, and");
	UI_DrawCustomProportionalString( xpos, y, string, style, 0.5, color, qfalse );
	y += 8;
	Com_sprintf(string, sizeof(string), "^Omeant just for showcase.");
	UI_DrawCustomProportionalString( xpos, y, string, style, 0.5, color, qfalse );
	y += 16;
	Com_sprintf(string, sizeof(string), "^OTextures are placeholders as well, so");
	UI_DrawCustomProportionalString( xpos, y, string, style, 0.5, color, qfalse );
	y += 8;
	Com_sprintf(string, sizeof(string), "^Oplease don't bother complaining about");
	UI_DrawCustomProportionalString( xpos, y, string, style, 0.5, color, qfalse );
	y += 8;
	Com_sprintf(string, sizeof(string), "^Othem.  Thank you.");
	UI_DrawCustomProportionalString( xpos, y, string, style, 0.5, color, qfalse );
	if (mtype) {
		y += 16;
		Com_sprintf(string, sizeof(string), "^OSet CVAR 'hud_iUnderstand' to 1, to");
		UI_DrawCustomProportionalString( xpos, y, string, style, 0.5, color, qfalse );
		y += 8;
		Com_sprintf(string, sizeof(string), "^Oremove this message.");
		UI_DrawCustomProportionalString( xpos, y, string, style, 0.5, color, qfalse );
	}
	*/

	/*y += 8;
	Com_sprintf(string, sizeof(string), "^a!^b!^c!^d!^e!^f!^g!^h!^i!^j!^k!^l!^m!");
	UI_DrawCustomProportionalString( xpos, y, string, style, 1.0, color, qfalse );*/

}

/*
=================
HUD_MegaMiniMap
=================
*/
#define PLAYER_ICON_SIZE	8.0
#define MINI_MAP_SIZE		64.0
#define MINI_HALF_MAP_SIZE	MINI_MAP_SIZE / 2
void HUD_MegaMiniMap( int xpos, int xoff, int ypos, int posLock, int align, float scaleX, float scaleY ) {

	char		*s;
	vec_t		*origin;
	float		playXPos, playYPos;
	int		playXDir, playYDir;
	float		mapXPos, mapYPos;
	float		centerOffX, centerOffY;
	vec4_t		color;

	clientInfo_t	*ci;
	int		count,i;
	float		miniMapScaleX,miniMapScaleY;

	float		mapXScale, mapYScale;

	// position lock
		if ( posLock > 0 && posLock <= HUD_POS_LOCK_TYPES ) {
		/*
		123
		456
		*/

		// text position is stripped as well
		switch( posLock ) {
			case 2:
			case 5:
				xpos = 320 * hud_aspectRatioScale.value - ( MINI_HALF_MAP_SIZE * scaleX );
				break;

			case 3:
			case 6:
				xpos = 640 * hud_aspectRatioScale.value - ( 4 + ( MINI_MAP_SIZE * scaleX ) ) ;
				break;

			default:
				xpos = 4;
		}

		if ( posLock > 3 ) {
			cg.ypos[posLock-1] -= MINI_MAP_SIZE * scaleY + 4;
			ypos = cg.ypos[posLock-1];
		}
		else {
			ypos = cg.ypos[posLock-1];
			cg.ypos[posLock-1] += MINI_MAP_SIZE * scaleY + 4;
		}

	} else {
		xpos += xoff;
		switch ( align ) {
			case 1: // center
				xpos += MINI_HALF_MAP_SIZE * scaleX;
				break;
			case 2: // right
				xpos += MINI_MAP_SIZE * scaleX;
		}
	}

	//
	origin = cg.snap->ps.origin;

	centerOffX = 28 * scaleX;
	centerOffY = 28 * scaleY;

	// scale of the whole map, based on the bsp file
	mapXScale = cgs.miniMapXScale;
	mapYScale = cgs.miniMapYScale;
	/*mapXScale = 0.25;
	mapYScale = 0.5;*/

	mapXPos = (0.5 - (mapXScale * 0.5)) + (origin[0] * mapXScale * ((float)1/2048));
	mapYPos = (0.5 - (mapYScale * 0.5)) - (origin[1] * mapYScale * ((float)1/2048));

	// draw mini map
	HUD_FuncColorGet ( hud_gameMap_colorBG.string, color );
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( (float)xpos * cgs.screenXScale + cgs.screenXOffset, (float)ypos * cgs.screenYScale + cgs.screenYOffset,
				(float)MINI_MAP_SIZE * cgs.screenXScale * scaleX, (float)MINI_MAP_SIZE * cgs.screenYScale * scaleY,
				mapXPos, mapYPos, mapXPos + mapXScale, mapYPos + mapYScale, cgs.media.miniMap );

	//trap_R_SetColor( NULL );

	/*
	s = va( "X=%f, Y=%f", mapXPos, mapYPos );
	UI_DrawCustomProportionalString( xpos, ypos-16, s, 0, 0.5, color, qfalse );
	s = va( "YAW=[%i] %f", (int)((cg.refdefViewAngles[YAW]+185.625) / 11.25) & 31, cg.refdefViewAngles[YAW] );
	UI_DrawCustomProportionalString( xpos, ypos-8, s, 0, 0.5, color, qfalse );
	*/

	playXDir = (int)((cg.refdefViewAngles[YAW]+185.625) / 11.25) & 31;
	playYDir = playXDir >> 3;
	playXDir &= 7;

	// draw player icon
	HUD_FuncColorGet ( hud_gameMap_colorPlayer.string, color );
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( (float)((xpos+((MINI_MAP_SIZE / 2) - (PLAYER_ICON_SIZE / 2)) * scaleX)) * cgs.screenXScale + cgs.screenXOffset,
				(float)((ypos+((MINI_MAP_SIZE / 2) - (PLAYER_ICON_SIZE / 2)) * scaleY)) * cgs.screenYScale + cgs.screenYOffset,
				(float)PLAYER_ICON_SIZE * cgs.screenXScale * scaleX,
				(float)PLAYER_ICON_SIZE * cgs.screenYScale * scaleY,
				playXDir * 0.125, playYDir * 0.25, (playXDir + 1) * 0.125, (playYDir + 1) * 0.25, cgs.media.miniMapPlyr );

	// draw team player icons
	if ( cgs.gametype < GT_TEAM ) {
		return;
	}
	if ( !cgs.teamLocOverlay ) {
		return;
	}
	HUD_FuncColorGet ( hud_gameMap_colorTeam.string, color );
	trap_R_SetColor( color );
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;

	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid /*&& ci->team == cg.snap->ps.persistant[PERS_TEAM]*/ &&
				cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR &&
				sortedTeamPlayers[i] != cg.snap->ps.clientNum ){
			playXPos = (ci->posXLoc + -origin[0]) * ((float)1/32) * scaleX;
			playYPos = (-ci->posYLoc + origin[1]) * ((float)1/32) * scaleY;

			miniMapScaleX = (MINI_MAP_SIZE / 2) * scaleX;
			miniMapScaleY = (MINI_MAP_SIZE / 2) * scaleY;
			if ( playXPos < miniMapScaleX && playXPos > -miniMapScaleX &&
					playYPos < miniMapScaleY && playYPos > -miniMapScaleY ) {
				trap_R_DrawStretchPic( (float)((playXPos+xpos+((MINI_MAP_SIZE / 2) - (PLAYER_ICON_SIZE / 2)) * scaleX))
									* cgs.screenXScale + cgs.screenXOffset,
							(float)((playYPos+ypos+((MINI_MAP_SIZE / 2) - (PLAYER_ICON_SIZE / 2)) * scaleY))
									* cgs.screenYScale + cgs.screenYOffset,
							(float)PLAYER_ICON_SIZE * cgs.screenXScale * scaleX,
							(float)PLAYER_ICON_SIZE * cgs.screenYScale * scaleY,
							0.0, 0.0, 1.0, 1.0, cgs.media.miniMapTeam );
			}
		}
	}

	/*
	playXPos = (origin[0] * ((float)1/32)) + xpos + centerOff;
	playYPos = (origin[1] * ((float)1/32)) + ypos + centerOff;

	trap_R_DrawStretchPic( (float)xpos * cgs.screenXScale, (float)ypos * cgs.screenYScale,
				(float)MINI_MAP_SIZE * cgs.screenXScale, (float)MINI_MAP_SIZE * cgs.screenYScale,
				0.0, 0.0, 1.0, 1.0, cgs.media.miniMap );
	trap_R_DrawStretchPic( playXPos, playYPos, 8.0 * scaleX, 8.0 * scaleY,
					(float) 1/8, (float) 5/8,
					(float) 2/8, (float) 6/8, cgs.media.hudScoreBoard [ TEAM_FREE ] );
	*/

}

/*
=================
HUD_MegaDrawFPS
=================
*/
#define	FPS_FRAMES	4
void HUD_MegaDrawFPS( int xpos, int xoff, int ypos, int posLock, int align, int style,
						float scaleX, float scaleY, const char* colorStr ) {

	char		*s;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int			i, total;
	int			fps;
	static	int	previous;
	int			t, frameTime;

	vec4_t		color;

	if ( !hud_fps_show.integer ) {
		return;
	}

	HUD_FuncColorGet ( colorStr, color );

	style = HUD_FuncStyleSet ( align, style );

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%i FPS", fps );

		//UI_DrawCustomProportionalString( xpos, ypos, s, style, scale, color, qfalse );
		HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );
	}

	//return 16 * scale + 4;

}

/*
=================
HUD_MegaDrawTimer
=================
*/
static HUD_MegaDrawTimer( int xpos, int ypos, int style, float scale, vec4_t color ) {

	char		*s;
	int			mins, seconds, tens;
	int			msec;
	int			timeLimit;

	int			timeLimitHalfMin;

	if ( !hud_gameClock_show.integer ) {
		return 0;
	}

	msec = cg.time - cgs.levelStartTime;
	timeLimit = cgs.timelimit;
	timeLimitHalfMin = (cgs.timelimit * 10) - (timeLimit * 10);
	timeLimit += (cgs.overtimeSets * cgs.overtime);

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	if (timeLimit) {
		if (timeLimitHalfMin)
			s = va( "%i:%i%i/%i:30", mins, tens, seconds, timeLimit );
		else
			s = va( "%i:%i%i/%i:00", mins, tens, seconds, timeLimit );
	} else
		s = va( "%i:%i%i", mins, tens, seconds );

	UI_DrawCustomProportionalString( xpos, ypos, s, style, scale, color, qfalse );

	return 16 * scale + 4;

}

/*
=================
HUD_MegaDrawDigitTimer
=================
*/
void HUD_MegaDrawDigitTimer( int xpos, int xoff, int ypos, int posLock, int align, int style,
						float scaleX, float scaleY, const char* colorStr ) {

	char		*s;
	int			mins, seconds, tens;
	int			msec;
	int			timeLimit;
	int			gameClock;

	vec4_t		color;

	if ( !hud_digitalClock_show.integer ) {
		return;
	}

	HUD_FuncColorGet ( colorStr, color );

	style = HUD_FuncStyleSet ( align, style );

	msec = cg.time - cgs.levelStartTime;
	if ( cgs.roundBasedMatches ) {
		timeLimit = ((cgs.timelimit * ( cgs.currentRound + 1 )) + (cgs.overtimeSets * cgs.overtime)) * 60000;
	} else {
		timeLimit = (cgs.timelimit + (cgs.overtimeSets * cgs.overtime)) * 60000;
	}

	if (timeLimit && !cg.warmup) {
		// count down
		gameClock = timeLimit - msec;
		if (gameClock >= 6000000) {
			gameClock = 5999000;
		}

		if (gameClock >= 60000) {
			seconds = gameClock / 1000;
			mins = seconds / 60;
			seconds -= mins * 60;
			tens = seconds / 10;
			seconds -= tens * 10;
			s = va( "%2i:%i%i", mins, tens, seconds );
		}
		else if (gameClock >= 0) {
			// draw seconds and milisecond
			tens = gameClock / 100;
			seconds = tens / 10;
			tens -= seconds * 10;
			s = va( "%2i.%i ", seconds, tens );
		}
		else
			s = va( " 0.0 " );
	} else {
		// count up
		gameClock = msec;
		if (gameClock >= 6000000) {
			gameClock = 5999000;
		}

		if (gameClock >= 0) {
			seconds = gameClock / 1000;
			mins = seconds / 60;
			seconds -= mins * 60;
			tens = seconds / 10;
			seconds -= tens * 10;
			s = va( "%2i:%i%i", mins, tens, seconds );
		} else
			s = va( " 0:00" );
	}

	//UI_DrawCustomProportionalString( xpos, ypos, s, style | UI_DIGIT, scale, color, qfalse );
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style | UI_DIGIT, scaleX, scaleY, color, qfalse );

	return; // 16 * scale + 4

}

/*
=================
HUD_MegaDrawUPS
=================
*/
#define	FPS_FRAMES	4
void HUD_MegaDrawUPS( int xpos, int xoff, int ypos, int posLock, int align, int style,
						float scaleX, float scaleY, const char* colorStr ) {

	char        *s;
	vec_t       *vel;
	int         speed;
	centity_t	*cent;
	playerState_t	*ps;

	vec4_t		color;

	if ( !hud_ups_show.integer ) {
		return;
	}

	// only show UPS during warmup, this is not warsow
	if ( !cg.warmup ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	// also, showing the UPS with the default physics is pointless
	if ( ps->persistant[PERS_MISC] & PMSC_RESTRICTED_PHYSICS ) {
		return;
	}

	HUD_FuncColorGet ( colorStr, color );

	style = HUD_FuncStyleSet ( align, style );

	vel = cg.snap->ps.velocity;
	/* ignore vertical component of velocity */
	speed = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);

	s = va( "%i UPS", speed );

	//UI_DrawCustomProportionalString( xpos, ypos, s, style, scale, color, qfalse );
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );

}

/*
=================
HUD_MegaDrawBriefScoreGrad
=================
*/
void HUD_MegaDrawBriefScoreGrad( int xpos, int xoff, int ypos, int posLock, int align, int style,
						float scaleX, float scaleY, int spos, vec4_t color /*const char *colorStr*/ ) {

	/*vec4_t		color;*/

	// position lock
	if (posLock > 0 && posLock <= 6) {
		switch( posLock ) {
			case 2:
			case 5:
				style |= UI_CENTER;
				if ( cgs.gametype == GT_CTF ) {
					xpos = (320 * hud_aspectRatioScale.value) + 46 * scaleX;
				} else {
					xpos = (320 * hud_aspectRatioScale.value) + 38 * scaleX;
				}
				break;

			case 3:
			case 6:
				style |= UI_RIGHT;
				xpos = 640 * hud_aspectRatioScale.value - 4;
				break;

			default:
				if ( cgs.gametype == GT_CTF ) {
					xpos = 92 * scaleX + 4;
				} else {
					xpos = 76 * scaleX + 4;
				}

		}

		if ( posLock > 3 ) {
			ypos = cg.ypos[posLock-1] - (32 * scaleY + 4);
		} else {
			ypos = cg.ypos[posLock-1];
		}

	} else {
		xpos += xoff;
		if ( cgs.gametype == GT_CTF ) {
			switch ( align ) {
				case 2:
					break;
				case 1:
					xpos += 46 * scaleX;
					break;
				default:
					xpos += 92 * scaleX;
			}
		} else {
			switch ( align ) {
				case 2:
					break;
				case 1:
					xpos += 38 * scaleX;
					break;
				default:
					xpos += 76 * scaleX;
			}
		}
	}

	if ( cgs.gametype == GT_CTF ) {
		xpos -= 16 * scaleX;
	}

	ypos += (spos * 14 + 2) * scaleY;

	//HUD_FuncColorGet ( colorStr, color );
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( (float)(xpos - ( 43 * scaleX ) ) * cgs.screenXScale + cgs.screenXOffset,
				(float)ypos * cgs.screenYScale + cgs.screenYOffset,
				(float)41 * scaleX * cgs.screenXScale, (float)14 * scaleY * cgs.screenYScale,
				0.0625, 0.0, 0.9375, 1.0, cgs.media.hudGradRCurve );

}


/*
=================
HUD_MegaDrawMatchInfo
=================
*/
void HUD_MegaDrawMatchInfo( int xpos, int xoff, int ypos, int posLock, int align, int style,
						float scaleX, float scaleY, const char* colorStr ) {

	char        *s;
	vec_t       *vel;
	centity_t	*cent;
	playerState_t	*ps;

	vec4_t		color;

	if ( !hud_matchInfo_show.integer ) {
		return;
	}

	HUD_FuncColorGet ( colorStr, color );

	style = HUD_FuncStyleSet ( align, style );

	if ( cg.warmup ) {
		s = va( "WARMUP" );
	} else if ( cgs.overtimeSets ) {
		s = va( "OVERTIME" );
	} else {
		if ( cgs.roundBasedMatches ) {
			s = va( "ROUND %i", cgs.currentRound + 1 );
		} else {
			s = va( "SINGLE ROUND" );
		}
	}

	//UI_DrawCustomProportionalString( xpos, ypos, s, style, scale, color, qfalse );
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );

}


/*
=================
HUD_MegaDrawBriefScoreLimitFlash

This is just to check if player/team is near the score or mercy limit.
If so, the score for that player/team will/should flash...
					   Unless someone made some shitty custom ruleset.
=================
*/

int HUD_MegaDrawBriefScoreLimitFlash( int a, int b, int lim ) {

	int		c;

	// don't bother if in warm up
	if ( cg.warmup )
		return 1;

	// flash
	if ( cg.time & 128 )
		return 1;

	// don't give a fuck unless player/team is in lead
	if ( a <= b )
		return 1;

	c = a - b;

	// are we near the mercy limit?
	if ( cgs.mercylimit >= 25 ) {
		if ( c >= (cgs.mercylimit - lim) )
			return 0;
	} else if ( cgs.mercylimit >= 10 ) {
		if ( c >= (cgs.mercylimit - (lim >> 1)) )
			return 0;
	}

	// are we near the score limit?
	if ( cgs.scorelimit >= 25 ) {
		if ( a >= (cgs.scorelimit - lim) )
			return 0;
	} else if ( cgs.scorelimit >= 10 ) {
		if ( a >= (cgs.scorelimit - (lim >> 1)) )
			return 0;
	}

	return 1;

}

/*
=================
HUD_MegaDrawBriefScore
=================
*/
void HUD_MegaDrawBriefScore( /*int xpos, int ypos, int layout, int style, float scale,*/
						int xpos, int xoff, int ypos, int posLock, int align, int style,
						float scaleX, float scaleY, vec4_t color
						/*char *colorStr1, char *colorStr2,*/ /*const char *colorStr*/ ) {

	const char	*s;
	int			s1, s2, score;
	int			x, y;
	int			altStyle;
	vec4_t		bgColor;
	vec4_t		amberColor;
	gitem_t		*item;
	float		axscale, ayscale;
	qhandle_t	hShader;
	int			flagInfo;
	float		gt_x, gt_y;
	int			scoreWidth;
	float		scoreWidthAdj;
	int			limitAlert;

	/*vec4_t		color;*/

	if ( !hud_briefScore_show.integer ) {
		return;
	}

	/*HUD_FuncColorGet ( colorStr1, color1 );
	HUD_FuncColorGet ( colorStr2, color2 );
	HUD_FuncColorGet ( colorStr, color );*/

	// position lock
	if (posLock > 0 && posLock <= 6) {
		switch( posLock ) {
			case 2:
			case 5:
				style |= UI_CENTER;
				if ( cgs.gametype == GT_CTF ) {
					xpos = (320 * hud_aspectRatioScale.value) + 46 * scaleX;
				} else {
					xpos = (320 * hud_aspectRatioScale.value) + 38 * scaleX;
				}
				break;

			case 3:
			case 6:
				style |= UI_RIGHT;
				xpos = 640 * hud_aspectRatioScale.value - 4;
				break;

			default:
				if ( cgs.gametype == GT_CTF ) {
					xpos = 92 * scaleX + 4;
				} else {
					xpos = 76 * scaleX + 4;
				}

		}

		if ( posLock > 3 ) {
			cg.ypos[posLock-1] -= 32 * scaleY + 4;
		}
		ypos = cg.ypos[posLock-1];
		if ( posLock < 4 ) {
			cg.ypos[posLock-1] += 32 * scaleY + 4;
		}
	} else {
		xpos += xoff;
		if ( cgs.gametype == GT_CTF ) {
			switch ( align ) {
				case 2:
					break;
				case 1:
					xpos += 46 * scaleX;
					break;
				default:
					xpos += 92 * scaleX;
			}
		} else {
			switch ( align ) {
				case 2:
					break;
				case 1:
					xpos += 38 * scaleX;
					break;
				default:
					xpos += 76 * scaleX;
			}
		}
	}

	style = HUD_FuncStyleSet ( 0, style );

	style &= UI_DROPSHADOW;
	altStyle = style & (~UI_DROPSHADOW);

	axscale = scaleX * cgs.screenXScale;
	ayscale = scaleY * cgs.screenYScale;

	trap_R_SetColor( NULL ); // just incase

	if ( cgs.gametype == GT_CTF ) {
		limitAlert = 150;
		hShader = cgs.media.hudFlagStatus [ 0 ];

		item = BG_FindItemForPowerup( PW_REDFLAG );
		if (item) {
			flagInfo = cgs.redflag;
			if( flagInfo >= 0 && flagInfo <= 2 ) {
				if ((flagInfo & 1) && cg.time & 128) {
					flagInfo += 2;
				}
				trap_R_DrawStretchPic( (float) (xpos - 16 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
					(float) ypos * cgs.screenYScale + cgs.screenYOffset,
					(float) 16 * axscale, (float) 16 * ayscale,
					0.25 * flagInfo, 0.0, 0.25 * (flagInfo + 1), 0.5, hShader );
			}
		}

		item = BG_FindItemForPowerup( PW_BLUEFLAG );
		if (item) {
			flagInfo = cgs.blueflag;
			if( flagInfo >= 0 && flagInfo <= 2 ) {
				if ((flagInfo & 1) && cg.time & 128) {
					flagInfo += 2;
				}
				trap_R_DrawStretchPic( (float) (xpos - 16 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
					(float) (ypos + 16 * scaleY) * cgs.screenYScale + cgs.screenYOffset,
					(float) 16 * axscale, (float) 16 * ayscale,
					0.25 * flagInfo, 0.5, 0.25 * (flagInfo + 1), 1.0, hShader );
			}
		}

		xpos -= 16 * scaleX; // move normal score over, so flag status doesn't get covered
	} else {
		limitAlert = 10;
	}

	hShader = cgs.media.hudMiniScoreShaders [ 0 ];

	trap_R_DrawStretchPic( (float) (xpos - 76 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
					(float) ypos * cgs.screenYScale + cgs.screenYOffset,
					(float) 32 * axscale, (float) 32 * ayscale,
					0.0, 0.0, 0.5, 1.0, hShader );

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	if ( cgs.gametype < GT_TEAM ) {
		qboolean	spectator;

		score = cg.snap->ps.persistant[PERS_SCORE];
		//spectator = ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR );

		trap_R_DrawStretchPic( (float) (xpos - 44 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
						(float) ypos * cgs.screenYScale + cgs.screenYOffset,
						(float) 16 * axscale, (float) 32 * ayscale,
						0.5, 0.0, 0.75, 1.0, hShader );
		trap_R_DrawStretchPic( (float) (xpos - 28 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
						(float) ypos * cgs.screenYScale + cgs.screenYOffset,
						(float) 12 * axscale, (float) 32 * ayscale,
						0.70, 0.0, 0.80, 1.0, hShader );
		trap_R_DrawStretchPic( (float) (xpos - 16 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
						(float) ypos * cgs.screenYScale + cgs.screenYOffset,
						(float) 16 * axscale, (float) 32 * ayscale,
						0.75, 0.0, 1.0, 1.0, hShader );

		/*HUD_FuncColorGet ( colorStr, color );*/
		x = ( 5 * scaleX );
		if ( s1 != SCORE_NOT_PRESENT ) {
			if ( HUD_MegaDrawBriefScoreLimitFlash(s1, s2, limitAlert) ) {
				s = va( "%i", s1 );
				y = ( 4 * scaleY );
				scoreWidth = UI_ReturnStringWidth (s, qfalse); // not going to pre-scale...
				if (scoreWidth > 48) { // 48 would be based on the actual size of the font, if it wasn't scaled
					scoreWidthAdj = 36/(float)scoreWidth;
				} else {
					scoreWidthAdj = 0.75; // regular width
				}
				HUD_FuncPosLock( s, 0, xpos - x, 0, ypos + y, 0, style | UI_RIGHT,
										scaleX * scoreWidthAdj, scaleY * 0.75, color, qfalse );
			}
		}

		if ( s2 != SCORE_NOT_PRESENT ) {
			if ( HUD_MegaDrawBriefScoreLimitFlash(s2, s1, limitAlert) ) {
				s = va( "%i", s2 );
				y = ( 18 * scaleY );
				scoreWidth = UI_ReturnStringWidth (s, qfalse); // not going to pre-scale...
				if (scoreWidth > 48) { // 48 would be based on the actual size of the font, if it wasn't scaled
					scoreWidthAdj = 36/(float)scoreWidth;
				} else {
					scoreWidthAdj = 0.75; // regular width
				}
				HUD_FuncPosLock( s, 0, xpos - x, 0, ypos + y, 0, style | UI_RIGHT,
										scaleX * scoreWidthAdj, scaleY * 0.75, color, qfalse );
			}
		}

	} else {

		trap_R_DrawStretchPic( (float) (xpos - 44 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
						(float) ypos * cgs.screenYScale + cgs.screenYOffset,
						(float) 16 * axscale, (float) 32 * ayscale,
						0.5, 0.0, 0.75, 1.0, hShader );
		trap_R_DrawStretchPic( (float) (xpos - 28 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
						(float) ypos * cgs.screenYScale + cgs.screenYOffset,
						(float) 12 * axscale, (float) 32 * ayscale,
						0.70, 0.0, 0.80, 1.0, hShader );
		trap_R_DrawStretchPic( (float) (xpos - 16 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
						(float) ypos * cgs.screenYScale + cgs.screenYOffset,
						(float) 16 * axscale, (float) 32 * ayscale,
						0.75, 0.0, 1.0, 1.0, hShader );

		/*HUD_FuncColorGet ( colorStr, color );*/
		x = ( 5 * scaleX );

		if ( HUD_MegaDrawBriefScoreLimitFlash(s1, s2, limitAlert) ) {
			s = va( "%i", s1 );
			y = ( 4 * scaleY );
			scoreWidth = UI_ReturnStringWidth (s, qfalse); // not going to pre-scale...
			if (scoreWidth > 48) { // 48 would be based on the actual size of the font, if it wasn't scaled
				scoreWidthAdj = 36/(float)scoreWidth;
			} else {
				scoreWidthAdj = 0.75; // regular width
			}
			HUD_FuncPosLock( s, 0, xpos - x, 0, ypos + y, 0, style | UI_RIGHT,
										scaleX * scoreWidthAdj, scaleY * 0.75, color, qfalse );
		}

		if ( HUD_MegaDrawBriefScoreLimitFlash(s2, s1, limitAlert) ) {
			s = va( "%i", s2 );
			y = ( 18 * scaleY );
			scoreWidth = UI_ReturnStringWidth (s, qfalse); // not going to pre-scale...
			if (scoreWidth > 48) { // 48 would be based on the actual size of the font, if it wasn't scaled
				scoreWidthAdj = 36/(float)scoreWidth;
			} else {
				scoreWidthAdj = 0.75; // regular width
			}
			HUD_FuncPosLock( s, 0, xpos - x, 0, ypos + y, 0, style | UI_RIGHT,
										scaleX * scoreWidthAdj, scaleY * 0.75, color, qfalse );
		}

	}

	// game type
	gt_x = floor ((float)cgs.gametype / 3);
	gt_y = ((float)cgs.gametype - (gt_x * 3)) * 0.3125;
	gt_x *= 0.3125;

	/*switch ( cgs.gametype ) {
		case GT_TOURNAMENT:
			hShader = cgs.media.gametypeIcon_duel;
			break;
		case GT_TEAM:
			hShader = cgs.media.gametypeIcon_tdm;
			break;
		case GT_CTF:
			hShader = cgs.media.gametypeIcon_ctf;
			break;
		default:
			hShader = cgs.media.gametypeIcon_dm;
	}*/

	// score limit
	trap_R_SetColor( NULL );
	if (cgs.scorelimit) {
		trap_R_DrawStretchPic( (float) (xpos - 66 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
						(float) (ypos + 4 * scaleY) * cgs.screenYScale + cgs.screenYOffset,
						(float) 16 * axscale, (float) 16 * ayscale,
						gt_x, gt_y, gt_x + 0.25, gt_y + 0.25,
						cgs.media.gametypeIcons );
		s = va( "%i", cgs.scorelimit );

		x = 58 * scaleX;
		y = 23 * scaleY;
		//UI_DrawCustomProportionalString( xpos - x, ypos + y, s, altStyle | UI_CENTER, scale * 0.50, color, qfalse );
		HUD_FuncPosLock( s, 0, xpos - x, 0, ypos + y, 0, altStyle | UI_CENTER,
									scaleX * 0.50, scaleY * 0.50, color, qfalse );
	} else {
		trap_R_DrawStretchPic( (float) (xpos - 70 * scaleX) * cgs.screenXScale + cgs.screenXOffset,
						(float) (ypos + 4 * scaleY) * cgs.screenYScale + cgs.screenYOffset,
						(float) 24 * axscale, (float) 24 * ayscale,
						gt_x, gt_y, gt_x + 0.25, gt_y + 0.25,
						cgs.media.gametypeIcons );
	}

	//return 32 * scale + 4;

}


/*
=================
HUD_MegaDrawFragInfo
=================
*/
#define	MDHUDFragTextHeight			16
#define	MDHUDFragTextIconSize		16

static void HUD_MegaDrawFragInfo( int xpos, int ypos, int xsize, int ysize, int ydir, int align, int style, float scale ) {

	int			w, h;
	int			i, len;
	int			tempYPos;
	qboolean	dropShadow;
	int			preScale;
	int			killerWidth, weaponSize, xposAdj, victomWidth;
	int			killerPos, weaponPos, victomPos;
	int			tempYPosOffset;
	vec4_t		hcolorBlk;
	vec4_t		hcolor;

	if (ysize <= 0) {
		return; // disabled
	}

	if ( ysize > HUD_TEXT_HEIGHT ) {
		ysize = HUD_TEXT_HEIGHT;
	}

	preScale = scale * MDHUDFragTextHeight;
	weaponSize = MDHUDFragTextIconSize * scale;
	align = align&3;

	hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0f;

	hcolorBlk[0] = hcolorBlk[1] = hcolorBlk[2] = 0.0f;
	hcolorBlk[3] = 1.0f;

	if (cgs.hudFragInfoLastPos != cgs.hudFragInfoPos) {
		if (cg.time - cgs.hudFragInfoTimes[cgs.hudFragInfoLastPos % ysize] > 5000) {
			cgs.hudFragInfoLastPos++;
		}

		trap_R_SetColor( hcolor ); // reset texture color tint

		// TODO: code this better

		if (ydir) {
			for (i = cgs.hudFragInfoLastPos; i < cgs.hudFragInfoPos; i++) {

				tempYPosOffset = ypos + ( i - cgs.hudFragInfoPos ) * preScale;

				switch ( align & 3 ) {

					case 1:
						// align center

						killerWidth = UI_ProportionalColorStringWidth( cgs.hudFragInfoKiller[i % ysize] ) * scale;
						victomWidth = UI_ProportionalColorStringWidth( cgs.hudFragInfoVictom[i % ysize] ) * scale;

						xposAdj = xpos - ( killerWidth + weaponSize + victomWidth ) / 2;

						killerPos = xposAdj;
						weaponPos = (xposAdj + killerWidth) * cgs.screenXScale + cgs.screenXOffset;
						victomPos = xposAdj + killerWidth + weaponSize;

						break;

					case 2:
						// align right
						killerWidth = UI_ProportionalColorStringWidth( cgs.hudFragInfoKiller[i % ysize] ) * scale;
						victomWidth = UI_ProportionalColorStringWidth( cgs.hudFragInfoVictom[i % ysize] ) * scale;

						killerPos = xpos - (killerWidth + weaponSize + victomWidth);
						weaponPos = (xpos - (weaponSize + victomWidth)) * cgs.screenXScale + 1 + cgs.screenXOffset;
						victomPos = xpos - victomWidth;

						break;

					default:
						// align left
						killerWidth = UI_ProportionalColorStringWidth( cgs.hudFragInfoKiller[i % ysize] ) * scale;

						killerPos = xpos;
						weaponPos = (xpos + killerWidth) * cgs.screenXScale + 1 + cgs.screenXOffset;
						victomPos = xpos + killerWidth + weaponSize;

						break;
				}

				UI_DrawProportionalStringColor( killerPos,
						tempYPosOffset, cgs.hudFragInfoKiller[i % ysize], hcolor, scale, cgs.media.charsetProp,
						style, qfalse );
				if (style & UI_DROPSHADOW) {
					trap_R_SetColor( hcolorBlk );
					trap_R_DrawStretchPic( (float)weaponPos + 1,
								(float)tempYPosOffset * cgs.screenYScale + 1 + cgs.screenYOffset,
								(float)weaponSize * cgs.screenXScale, (float)weaponSize * cgs.screenYScale,
								0.0, 0.0, 1.0, 1.0, cgs.hudFragInfoDeathIcon[i % ysize] );
					trap_R_SetColor( hcolor ); // reset texture color tint
				}
				trap_R_DrawStretchPic( (float)weaponPos,
							(float)tempYPosOffset * cgs.screenYScale + cgs.screenYOffset,
							(float)weaponSize * cgs.screenXScale, (float)weaponSize * cgs.screenYScale,
							0.0, 0.0, 1.0, 1.0, cgs.hudFragInfoDeathIcon[i % ysize] );
				UI_DrawProportionalStringColor( victomPos,
						tempYPosOffset, cgs.hudFragInfoVictom[i % ysize], hcolor, scale, cgs.media.charsetProp,
						style, qfalse );

			}
		}

	}

}

/*
=================
HUD_MegaDrawHUDInfo
=================
*/
#define	MDHUDTextHeight			16

static void HUD_MegaDrawHUDInfo( int xpos, int ypos, int xsize, int ysize, int ydir, int hudBox, int style, float scale ) {

	int			w, h;
	int			i, len;
	int			tempYPos;
	qboolean	dropShadow;
	int			preScale;
	vec4_t		hcolor;

	if (ysize <= 0) {
		return; // disabled
	}

	if ( hudBox < 0 || hudBox >= TOTAL_HUD_TEXT_BOXES ) {
		return; // disabled
	}

	if ( ysize > HUD_TEXT_HEIGHT ) {
		ysize = HUD_TEXT_HEIGHT;
	}

	preScale = scale * MDHUDTextHeight;

	if (cgs.hudInfoLastPos[hudBox] != cgs.hudInfoPos[hudBox]) {
		if (cg.time - cgs.hudInfoMsgTimes[hudBox][cgs.hudInfoLastPos[hudBox] % ysize] > 5000) {
			cgs.hudInfoLastPos[hudBox]++;
		}

		trap_R_SetColor( NULL );

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		if (ydir) {
			for (i = cgs.hudInfoLastPos[hudBox]; i < cgs.hudInfoPos[hudBox]; i++) {
				UI_DrawCustomProportionalString( xpos, ypos + ( i - cgs.hudInfoPos[hudBox] ) * preScale,
						cgs.hudInfoMsgs[hudBox][i % ysize], style, scale, hcolor, qfalse );
			}
		} else {
			tempYPos = -1;
			for (i = cgs.hudInfoLastPos[hudBox]; i < cgs.hudInfoPos[hudBox]; i++) {
				tempYPos++;
				UI_DrawCustomProportionalString( xpos, ypos + tempYPos * preScale,
						cgs.hudInfoMsgs[hudBox][i % ysize], style, scale, hcolor, qfalse );
			}
		}


	}
}

/*
=================
HUD_MegaDrawChat

TODO: Remove this
=================
*/
#define	MDCharHeight			16

static void HUD_MegaDrawChat( int xpos, int ypos, int xsize, int ysize, int ydir, int style, float scale, vec4_t hcolor ) {

	int			w, h;
	int			i, len;

	int			tempYPos;

	qboolean	dropShadow;

	int			preScale;

	if (ysize <= 0)
		return; // disabled

	// TODO - the '16' need to become a local define label
	preScale = scale * MDCharHeight;

	if (cgs.lastChatPos != cgs.chatPos) {
		if (cg.time - cgs.chatMsgTimes[cgs.lastChatPos % ysize] > 5000) {
			cgs.lastChatPos++;
		}

		trap_R_SetColor( NULL );

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		if (ydir) {
			for (i = cgs.lastChatPos; i < cgs.chatPos; i++) {
				UI_DrawCustomProportionalString( xpos, ypos + ( i - cgs.chatPos ) * preScale, cgs.chatMsgs[i % ysize], style, scale, hcolor, qfalse );
			}
		} else {
			tempYPos = -1;
			for (i = cgs.lastChatPos; i < cgs.chatPos; i++) {
				tempYPos++;
				UI_DrawCustomProportionalString( xpos, ypos + tempYPos * preScale, cgs.chatMsgs[i % ysize], style, scale, hcolor, qfalse );
			}
		}


	}
}

/*
=================
HUD_MegaDrawCrosshair
=================
*/

static void HUD_MegaDrawCrosshair( float xpos, float ypos, float xsize, float ysize ) {
	qhandle_t	hShader;
	vec4_t		color;
	int			typeA;
	int			typeB;

	xpos -= xsize / 2;
	ypos -= ysize / 2;

	typeA = hud_crosshair_innerType.integer;
	typeB = hud_crosshair_outsideType.integer;

	CG_AdjustFrom640( &xpos, &ypos, &xsize, &ysize );

	if (typeA % NUM_CROSSHAIRS) {
		HUD_FuncColorGet ( hud_crosshair_innerColor.string, color );
		trap_R_SetColor( color );
		hShader = cgs.media.crosshairTypeA[ (typeA % NUM_CROSSHAIRS) - 1 ];
		trap_R_DrawStretchPic( xpos, ypos, xsize, ysize, 0, 0, 1, 1, hShader );
	}

	if (typeB % NUM_CROSSHAIRS) {
		HUD_FuncColorGet ( hud_crosshair_outsideColor.string, color );
		trap_R_SetColor( color );
		hShader = cgs.media.crosshairTypeB[ (typeB % NUM_CROSSHAIRS) - 1 ];
		trap_R_DrawStretchPic( xpos, ypos, xsize, ysize, 0, 0, 1, 1, hShader );
	}

	trap_R_SetColor( NULL );

}

/*
=================
HUD_MegaDrawHudPart

HUD parts are divided into 16 pieces from a 4x4 grid texture
=================
*/

#define HUD_PART_SIZE		0.25

static void HUD_MegaDrawHudPart( float xpos, float ypos, float xsize, float ysize, qhandle_t hShader, int fcol, int frow, int fcol2, int frow2, vec4_t color) {
//	qhandle_t	hShader;
	float		fcolStart, frowStart, fcolSize, frowSize;

	fcolStart = (float)(fcol*HUD_PART_SIZE);
	frowStart = (float)(frow*HUD_PART_SIZE);
	fcolSize = (float)((fcol2+1)*HUD_PART_SIZE)+fcolStart;
	frowSize = (float)((frow2+1)*HUD_PART_SIZE)+frowStart;

	CG_AdjustFrom640( &xpos, &ypos, &xsize, &ysize );

	trap_R_SetColor( color );
//	hShader = cgs.media.hudBarShaders [ ptype ];
	trap_R_DrawStretchPic( xpos, ypos, xsize, ysize, fcolStart, frowStart, fcolSize, frowSize, hShader );

	trap_R_SetColor( NULL );

}

/*
=================
HUD_MegaDrawBorder
=================
*/

static void HUD_MegaDrawBorder( float xpos, float ypos, float xsize, float ysize, int ptype, vec4_t color) {

	HUD_MegaDrawHudPart((float)xpos-8, (float)ypos-8, (float)8, (float)8,
						cgs.media.hudBorderShaders [ ptype ], 0, 0, 0, 0, color);
	HUD_MegaDrawHudPart((float)xpos+xsize, (float)ypos-8, (float)8, (float)8,
						cgs.media.hudBorderShaders [ ptype ], 3, 0, 0, 0, color);
	HUD_MegaDrawHudPart((float)xpos-8, (float)ypos+ysize, (float)8, (float)8,
						cgs.media.hudBorderShaders [ ptype ], 0, 2, 0, 0, color);
	HUD_MegaDrawHudPart((float)xpos+xsize, (float)ypos+ysize, (float)8, (float)8,
						cgs.media.hudBorderShaders [ ptype ], 3, 2, 0, 0, color);
	HUD_MegaDrawHudPart((float)xpos, (float)ypos-8, (float)xsize, (float)8,
						cgs.media.hudBorderShaders [ ptype ], 1, 0, 0, 0, color);
	HUD_MegaDrawHudPart((float)xpos, (float)ypos+ysize, (float)xsize, (float)8,
						cgs.media.hudBorderShaders [ ptype ], 1, 2, 0, 0, color);
	HUD_MegaDrawHudPart((float)xpos-8, (float)ypos, (float)8, (float)ysize,
						cgs.media.hudBorderShaders [ ptype ], 0, 1, 0, 0, color);
	HUD_MegaDrawHudPart((float)xpos+xsize, (float)ypos, (float)8, (float)ysize,
						cgs.media.hudBorderShaders [ ptype ], 3, 1, 0, 0, color);

}

/*
===============
HUD_MegaDrawWeaponBarVert
===============
*/

#define	HUD_BAR_VERT_WIDTH					18 // 54
#define	HUD_AMMOBAR_VERT_WIDTH				54 - 14
#define	HUD_AMMOBAR_VERT_TEXT				HUD_AMMOBAR_VERT_WIDTH - 2
#define	HUD_BAR_VERT_HEIGHT					18
#define	HUD_BAR_VERT_HEIGHT_SOLO_SPACE		2
#define	HUD_BAR_VERT_HEIGHT_SPACE			HUD_BAR_VERT_HEIGHT_SOLO_SPACE + HUD_BAR_VERT_HEIGHT
#define	HUD_BAR_VERT_HEIGHT_HALF_SPACE		HUD_BAR_VERT_HEIGHT_SPACE / 2

void HUD_MegaDrawWeaponBarVert(int xPos, int xoff, int yPos, int align, int cPos, int bits){

	int y = yPos;
	int x = xPos + xoff;
	int i;
	int jumpAdd;

	//mmp
	int currentWeapon;
	centity_t	*cent;

	int ammo;
	int max;
	int w;
	char *s;

	float dkcyan[4];
	float white[4];
	float black[4];
	float blue[4];
	float red[4];
	//float whiteFont[4];
	//float redFont[4];

	if ( !hud_weaponBar_show.integer ) {
		return;
	}

	HUD_FuncColorGet ( hud_weaponBar_colorFlash2.string, dkcyan );
	HUD_FuncColorGet ( hud_weaponBar_colorFlash1.string, white );
	//HUD_FuncColorGet ( hud_weaponBar_colorDead.string, whiteFont );
	HUD_FuncColorGet ( hud_weaponBar_colorDead.string, black );
	HUD_FuncColorGet ( hud_weaponBar_colorNormal.string, blue );
	HUD_FuncColorGet ( hud_weaponBar_colorLow.string, red );
	//HUD_FuncColorGet ( hud_weaponBar_colorDead.string, redFont );

	if ( align == 1 ) {
		x -= HUD_BAR_VERT_WIDTH / 2;
	} else if ( align == 2 ) {
		x -= HUD_BAR_VERT_WIDTH;
	}

	if (cPos & 2) {
		jumpAdd = -(HUD_BAR_VERT_HEIGHT_SPACE);
		y -= 20;
		if (!(cPos & 4))
			y += WP_FLAMETHROWER * (HUD_BAR_VERT_HEIGHT_SPACE);
	} else {
		jumpAdd = HUD_BAR_VERT_HEIGHT_SPACE;
		if (cPos & 4)
			y -= WP_FLAMETHROWER * (HUD_BAR_VERT_HEIGHT_SPACE);
	}

	//mmp - weapon cursor is no longer stuck when spectating someone
	/*if (cg.snap->ps.pm_flags & PMF_FOLLOW) {
		cent = &cg_entities[cg.snap->ps.clientNum];
		currentWeapon = cent->currentState.weapon;
		} else
		currentWeapon = cg.weaponSelect;// this is still around, otherwise the above code will only update the cursor when the weapon is fully changed*/

	// FIXME: the code above doesn't work for demos
	// if/when fixed, the two lines below can be removed
	cent = &cg_entities[cg.snap->ps.clientNum];
	currentWeapon = cent->currentState.weapon;

	//cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR

	for ( i = WP_BLASTER ; i <= WP_FLAMETHROWER ; i++ ) {
		if ( !( bits & ( 1 << i ) ) ) {
			CG_FillRect( x, y, HUD_BAR_VERT_WIDTH, HUD_BAR_VERT_HEIGHT, black );
			y += jumpAdd;
		} else {

			ammo = cg.snap->ps.ammo[ammoGroup[i].ammo];

			switch(i) {
				case WP_ROCKET_LAUNCHER:
				case WP_GRENADE_LAUNCHER:
				case WP_SHOTGUN:
				case WP_SUPER_SHOTGUN:
					max = 5;
					break;
				default:
					max = 25;
					break;
			}

			if (ammo == 0) {
				CG_FillRect( x, y, HUD_BAR_VERT_WIDTH, HUD_BAR_VERT_HEIGHT, black );
			}
			else if (ammo < 0 || ammo>=max || cg.time&128) {
				CG_FillRect( x, y, HUD_BAR_VERT_WIDTH, HUD_BAR_VERT_HEIGHT, blue );
			}
			else {
				CG_FillRect( x, y, HUD_BAR_VERT_WIDTH, HUD_BAR_VERT_HEIGHT, red );
			}

			// draw weapon icon
			CG_DrawPic( x+1, y+1, 16, 16, cg_weapons[i].weaponIcon );

			/** Draw Weapon Ammo **/
			/*if (ammo >= 999)
				s = "999"; // showing more would just overlap the weapon icon
			else if (ammo < 0)
				s = "\x7f"; // infinite symbol
			else
				s = va("%i", ammo );
			if (ammo == 0)
				UI_DrawCustomProportionalString( x+52, y+4, s, UI_DROPSHADOW | UI_RIGHT, 0.75, redFont, qfalse );
			else
				UI_DrawCustomProportionalString( x+52, y+4, s, UI_DROPSHADOW | UI_RIGHT, 0.75, whiteFont, qfalse );*/

			if ( i == currentWeapon ) {
				if (cg.time & 256)
					CG_DrawRect( x, y, HUD_BAR_VERT_WIDTH ,HUD_BAR_VERT_HEIGHT ,2, dkcyan);
				else
					CG_DrawRect( x, y, HUD_BAR_VERT_WIDTH ,HUD_BAR_VERT_HEIGHT ,2, white);
			}

			y += jumpAdd;
		}

	}
}

/*
===============
HUD_MegaDrawAmmoBarVert
===============
*/

void HUD_MegaDrawAmmoBarVert(int xPos, int xoff, int yPos, int align, int cPos, int bits){

	int		y = yPos;
	int		x = xPos + xoff;
	int		i;
	int		jumpAdd;
	int		curTextPos;
	int		curJumpAdd;
	int		curYSize;
	int		boxYPos;
	qboolean	boxExt;

	//mmp
	int		currentWeapon;
	centity_t	*cent;

	int		ammo;
	int		max;
	int		low;
	int		w;
	char	*s;
	float	dkcyan[4];
	float	white[4];

	float	black[4];
	float	blue[4];
	float	red[4];
	float	redFont[4];
	float	whiteFont[4];

	if ( !hud_ammoBar_show.integer ) {
		return;
	}

	HUD_FuncColorGet ( hud_ammoBar_colorFlash2.string, dkcyan );
	HUD_FuncColorGet ( hud_ammoBar_colorFlash1.string, white );
	HUD_FuncColorGet ( hud_ammoBar_colorText.string, whiteFont );
	HUD_FuncColorGet ( hud_ammoBar_colorDead.string, black );
	HUD_FuncColorGet ( hud_ammoBar_colorNormal.string, blue );
	HUD_FuncColorGet ( hud_ammoBar_colorLow.string, red );
	HUD_FuncColorGet ( hud_ammoBar_colorTextDead.string, redFont );

	if ( align == 1 ) {
		x -= HUD_AMMOBAR_VERT_WIDTH / 2;
	} else if ( align == 2 ) {
		x -= HUD_AMMOBAR_VERT_WIDTH;
	}

	/*if (cPos & 2) {
		jumpAdd = HUD_BAR_VERT_HEIGHT_SPACE;
		if (cPos & 4)
			y -= (WP_FLAMETHROWER + 1) * HUD_BAR_VERT_HEIGHT_HALF_SPACE;
	} else {
		jumpAdd = -HUD_BAR_VERT_HEIGHT_SPACE;
		y -= 20;
		if (cPos & 4)
			y += (WP_FLAMETHROWER + 1) * HUD_BAR_VERT_HEIGHT_HALF_SPACE;
	}*/

	if (cPos & 2) {
		jumpAdd = -(HUD_BAR_VERT_HEIGHT_SPACE);
		y -= 20;
		if (!(cPos & 4))
			y += WP_FLAMETHROWER * (HUD_BAR_VERT_HEIGHT_SPACE);
	} else {
		jumpAdd = HUD_BAR_VERT_HEIGHT_SPACE;
		if (cPos & 4)
			y -= WP_FLAMETHROWER * (HUD_BAR_VERT_HEIGHT_SPACE);
	}

	//mmp - weapon cursor is no longer stuck when spectating someone
	/*if (cg.snap->ps.pm_flags & PMF_FOLLOW) {
		cent = &cg_entities[cg.snap->ps.clientNum];
		currentWeapon = cent->currentState.weapon;
		} else
		currentWeapon = cg.weaponSelect;// this is still around, otherwise the above code will only update the cursor when the weapon is fully changed*/

	// FIXME: the code above doesn't work for demos
	// if/when fixed, the two lines below can be removed
	cent = &cg_entities[cg.snap->ps.clientNum];
	currentWeapon = ammoGroup[cent->currentState.weapon].ammo;

	//cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR

	for ( i = AT_INFINITY ; i <= AT_GAS ; i++ ) {

		ammo = cg.snap->ps.ammo[i];

		// TODO: fix the broken match in this switch section
		switch(i) {
			case AT_SHELLS:
				low = 5;
				max = WC_LOW_AMMO;
				curJumpAdd = jumpAdd*2;
				curYSize = HUD_BAR_VERT_HEIGHT_SPACE * 2;
				boxExt = qtrue;
				break;
			case AT_ROCKETS:
				low = 5;
				max = WC_VLOW_AMMO;
				curJumpAdd = jumpAdd*2;
				curYSize = HUD_BAR_VERT_HEIGHT_SPACE * 2;
				boxExt = qtrue;
				break;
			case AT_CELLS:
				low = 25;
				max = WC_MED_AMMO;
				curJumpAdd = jumpAdd*2;
				curYSize = HUD_BAR_VERT_HEIGHT_SPACE * 2;
				boxExt = qtrue;
				break;
			case AT_BULLETS:
				low = 25;
				max = WC_HIGH_AMMO;
				curJumpAdd = jumpAdd;
				curYSize = HUD_BAR_VERT_HEIGHT;
				boxExt = qfalse;
				break;
			default:
				low = 25;
				max = WC_LOW_AMMO;
				curJumpAdd = jumpAdd;
				curYSize = HUD_BAR_VERT_HEIGHT;
				boxExt = qfalse;
				break;
		}

		if (cPos & 2) {
			y += curJumpAdd;
			boxYPos = y + HUD_BAR_VERT_HEIGHT_SPACE;
			if (boxExt)
				curTextPos = (y+4) + (HUD_BAR_VERT_HEIGHT_HALF_SPACE) + (HUD_BAR_VERT_HEIGHT_SPACE);
			else
				curTextPos = (y+4) + (HUD_BAR_VERT_HEIGHT_SPACE);
		} else {
			boxYPos = y;
			if (boxExt)
				curTextPos = y+4 + HUD_BAR_VERT_HEIGHT_HALF_SPACE;
			else
				curTextPos = y+4;
		}

		if (ammo == 0 || low < 0) {
			CG_FillRect( x, boxYPos, HUD_AMMOBAR_VERT_WIDTH, curYSize, black );
		}
		else if (ammo < 0 || ammo>=low || cg.time&128) {
			CG_FillRect( x, boxYPos, HUD_AMMOBAR_VERT_WIDTH, curYSize, blue );
		}
		else {
			CG_FillRect( x, boxYPos, HUD_AMMOBAR_VERT_WIDTH, curYSize, red );
		}

		// draw weapon icon
		//CG_DrawPic( x+1, y+1, 16, 16, cg_weapons[i].weaponIcon );

		/** Draw Weapon Ammo **/
		if (ammo >= 999)
			s = "999"; // showing more would just overlap the weapon icon
		else if (ammo < 0)
			s = "\x7f"; // infinite symbol
		else if (ammo == max && (cg.time & 256))
			s = "MAX";
		else
			s = va("%i", ammo );
		if (ammo == 0)
			UI_DrawCustomProportionalString( x+HUD_AMMOBAR_VERT_TEXT, curTextPos, s, UI_DROPSHADOW | UI_RIGHT, 0.75, redFont, qfalse );
		else
			UI_DrawCustomProportionalString( x+HUD_AMMOBAR_VERT_TEXT, curTextPos, s, UI_DROPSHADOW | UI_RIGHT, 0.75, whiteFont, qfalse );

		if ( i == currentWeapon ) {
			if (cg.time & 256)
				CG_DrawRect( x, boxYPos, HUD_AMMOBAR_VERT_WIDTH ,curYSize ,2, dkcyan);
			else
				CG_DrawRect( x, boxYPos, HUD_AMMOBAR_VERT_WIDTH ,curYSize ,2, white);
		}

		if (!(cPos & 2))
			y += curJumpAdd;

	}
}

/*
===============
HUD_MegaDrawWeaponBarHor
===============
*/

// mmp - edit
void HUD_MegaDrawWeaponBarHor(int xPos, int yPos, int cPos, int count, int bits, float *color){

	int y = yPos;
	int x = xPos;
	int i;
	int jumpAdd;

	//mmp
	int currentWeapon;
	centity_t	*cent;

	float ammo;
	float max;// mmp - need to test if this really needs to be a float as the weaponbar mode below has it as integer
	int w;
	char *s;
	float dkcyan[4];
	float white[4];
	float boxColor[4];

	float black[4];
	float blue[4];
	float red[4];
	float redFont[4];

	boxColor[1]=0;
	boxColor[3]=0.4f;

	dkcyan[0] = 0;
	dkcyan[1] = 0.69f;
	dkcyan[2] = 1.0f;
	dkcyan[3] = 1.0f;

	white[0] = 1.0f;
	white[1] = 1.0f;
	white[2] = 1.0f;
	white[3] = 1.0f;

	black[0] = black[1] = black[2] = 0;
	black[3] = 0.4f;

	blue[0] = blue[1] = 0;
	blue[2] = 1.0f;
	blue[3] = 0.4f;

	red[0] = 1.0f;
	red[1] = red[2] = 0;
	red[3] = 0.4f;

	redFont[0] = 1.0f;
	redFont[1] = redFont[2] = 0;
	redFont[3] = 1.0f;


	if (cPos & 2) {
		jumpAdd = -30;
		x -= 30;
		if (cPos & 4)
			x += count * 15;
	} else {
		jumpAdd = 30;
		if (cPos & 4)
			x -= count * 15;
	}

	//mmp - weapon cursor is no longer stuck when spectating someone
	if (cg.snap->ps.pm_flags & PMF_FOLLOW) {
		cent = &cg_entities[cg.snap->ps.clientNum];
		currentWeapon = cent->currentState.weapon;
		} else
		currentWeapon = cg.weaponSelect;// this is still around, otherwise the above code will only update the cursor when the weapon does change

	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
		if(i==10)
			continue; // move grappling hook placement to the front
		if(i==0)
			i=10;
		if ( !( bits & ( 1 << i ) ) ) {
			if(i==10)
				i=0;
			continue;
		}

		ammo = cg.snap->ps.ammo[i];

		switch(i) {
			case WP_ROCKET_LAUNCHER:
			case WP_GRENADE_LAUNCHER:
			case WP_SHOTGUN:
#ifdef MISSIONPACK
			case WP_NAILGUN:
			case WP_RAILGUN:
			case WP_PROX_LAUNCHER:
#endif
				max = 5;
				break;
			/*case WP_SPREADSHOT:
				max = 10;
				break;*/
			default:
				max = 25;
				break;
		}

		if (ammo == 0)
			CG_FillRect( x, y, 30 ,38, black );
		else if (ammo < 0 || ammo>=max || cg.time&128)
			CG_FillRect( x, y, 30 ,38, blue );
		else
			CG_FillRect( x, y, 30 ,38, red );

		// draw weapon icon
		CG_DrawPic( x+7, y+2, 16, 16, cg_weapons[i].weaponIcon );

		/** Draw Weapon Ammo **/
		if(ammo != -1){
			if (ammo >= 999 )
				s = "MAX";
			else
				s = va("%i", cg.snap->ps.ammo[ i ] );
			w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
			if (ammo == 0)
				CG_DrawSmallStringColor(x - w/2 + 15, y+20, s, redFont);
			else
				CG_DrawSmallStringColor(x - w/2 + 15, y+20, s, color);
		}

		if ( i == currentWeapon ) { // mmp - was cg.weaponSelect
			if (cg.time & 256)
				CG_DrawRect( x, y, 30 ,38 ,2, dkcyan);
			else
				CG_DrawRect( x, y, 30 ,38 ,2, white);
		}

		x += jumpAdd;

		if(i==10)
			i=0; // revert changes of grappling hook placement
	}
}

/*
=================
HUD_MegaDrawTestStat
=================
*/
void HUD_MegaDrawTestStat( int xpos, int ypos, int team, int style, float scale, vec4_t color ) {
	int		x, y, w;
	char		*s;
	centity_t	*cent;
	playerState_t	*ps;
	int		value;
	int		i, inv;
	qhandle_t	hShader;
	int		flash;

	/*vec4_t		color1, color2;

	color1[3] = 1.0;
	color2[3] = 1.0;
	if (team == TEAM_RED) {
		// dark color
		color1[0] = 1.0;
		color1[1] = color1[2] = 0.0;

		// light color
		color2[0] = 1.0;
		color2[1] = 0.5;
		color2[2] = 0.0;
	} else if (team == TEAM_BLUE) {
		// dark color
		color1[0] = color1[1] = 0.0;
		color1[2] = 1.0;

		// light color
		color2[0] = 0.0;
		color2[1] = 0.5;
		color2[2] = 1.0;
	} else {
		// dark color
		color1[0] = 0.0;
		color1[1] = 0.5;
		color1[2] = 1.0;

		// light color
		color2[0] = color2[1] = color2[2] = 1.0;
	}*/

	if ( !hud_testStatus_show.integer ) {
		return;
	}

	x = ( 16 * scale );
	y = ( 10 * scale );

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = ps->stats[STAT_HEALTH];
	flash = ( value > 25 || (value > 10 && !(cg.time & 256)) || (value < 11 && !(cg.time & 128)) );

	// base color palette on current team
	if (team == TEAM_RED) {
		if (flash) {
			s = va(S_COLOR_AMBER"%i"S_COLOR_RED"\x1E", value );
		} else {
			s = va(S_COLOR_RED"\x1E", value );
		}
		UI_DrawCustomProportionalString( xpos - x, ypos, s, UI_DROPSHADOW | UI_RIGHT, scale, color, qfalse );

		value = floor ( ps->stats[STAT_LEVEL] / LV_SUBPOINT ) + 1;
		s = va(S_COLOR_AMBER"LV %i", value );
		UI_DrawCustomProportionalString( xpos - x, ypos - y, s, UI_DROPSHADOW | UI_RIGHT, scale / 2, color, qfalse );

		value = ps->stats[STAT_ARMOR];
		s = va( S_COLOR_RED"\x1F"S_COLOR_AMBER"%i", value );
		UI_DrawCustomProportionalString( xpos + x, ypos, s, UI_DROPSHADOW, scale, color, qfalse );

		if (value > 0) {
			value = ps->stats[STAT_ARMORTIER];
			s = va(S_COLOR_AMBER"LV %i", value );
			UI_DrawCustomProportionalString( xpos + x, ypos - y, s, UI_DROPSHADOW, scale / 2, color, qfalse );
		}

		if ( !(ps->persistant[PERS_MISC] & PMSC_RESTRICTED_PHYSICS) ) {
			if ( ps->persistant[PERS_MISC] & PMSC_PHYSICS_SELECTION ) {
				s = S_COLOR_AMBER"A" /*"ARENA"*/;
			} else {
				s = S_COLOR_AMBER"W" /*"WORLD"*/;
			}
			UI_DrawCustomProportionalString( xpos, ypos, s, UI_DROPSHADOW | UI_CENTER, scale / 2, color, qfalse );
		}
	} else if (team == TEAM_BLUE) {
		if (flash) {
			s = va(S_COLOR_SEA"%i"S_COLOR_BLUE"\x1E", value );
		} else {
			s = va(S_COLOR_BLUE"\x1E", value );
		}
		UI_DrawCustomProportionalString( xpos - x, ypos, s, UI_DROPSHADOW | UI_RIGHT, scale, color, qfalse );

		value = floor ( ps->stats[STAT_LEVEL] / LV_SUBPOINT ) + 1;
		s = va(S_COLOR_SEA"LV %i", value );
		UI_DrawCustomProportionalString( xpos - x, ypos - y, s, UI_DROPSHADOW | UI_RIGHT, scale / 2, color, qfalse );

		value = ps->stats[STAT_ARMOR];
		s = va( S_COLOR_BLUE"\x1F"S_COLOR_SEA"%i", value );
		UI_DrawCustomProportionalString( xpos + x, ypos, s, UI_DROPSHADOW, scale, color, qfalse );

		if (value > 0) {
			value = ps->stats[STAT_ARMORTIER];
			s = va(S_COLOR_SEA"LV %i", value );
			UI_DrawCustomProportionalString( xpos + x, ypos - y, s, UI_DROPSHADOW, scale / 2, color, qfalse );
		}

		if ( !(ps->persistant[PERS_MISC] & PMSC_RESTRICTED_PHYSICS) ) {
			if ( ps->persistant[PERS_MISC] & PMSC_PHYSICS_SELECTION ) {
				s = S_COLOR_SEA"A" /*"ARENA"*/;
			} else {
				s = S_COLOR_SEA"W" /*"WORLD"*/;
			}
			UI_DrawCustomProportionalString( xpos, ypos, s, UI_DROPSHADOW | UI_CENTER, scale / 2, color, qfalse );
		}
	} else {
		if (flash) {
			s = va(S_COLOR_WHITE"%i"S_COLOR_SEA"\x1E", value );
		} else {
			s = va(S_COLOR_SEA"\x1E", value );
		}
		UI_DrawCustomProportionalString( xpos - x, ypos, s, UI_DROPSHADOW | UI_RIGHT, scale, color, qfalse );

		value = floor ( ps->stats[STAT_LEVEL] / LV_SUBPOINT ) + 1;
		s = va(S_COLOR_WHITE"LV %i", value );
		UI_DrawCustomProportionalString( xpos - x, ypos - y, s, UI_DROPSHADOW | UI_RIGHT, scale / 2, color, qfalse );

		value = ps->stats[STAT_ARMOR];
		s = va( S_COLOR_SEA"\x1F"S_COLOR_WHITE"%i", value );
		UI_DrawCustomProportionalString( xpos + x, ypos, s, UI_DROPSHADOW, scale, color, qfalse );

		if (value > 0) {
			value = ps->stats[STAT_ARMORTIER];
			s = va(S_COLOR_WHITE"LV %i", value );
			UI_DrawCustomProportionalString( xpos + x, ypos - y, s, UI_DROPSHADOW, scale / 2, color, qfalse );
		}

		if ( !(ps->persistant[PERS_MISC] & PMSC_RESTRICTED_PHYSICS) ) {
			if ( ps->persistant[PERS_MISC] & PMSC_PHYSICS_SELECTION ) {
				s = "A" /*"ARENA"*/;
			} else {
				s = "W" /*"WORLD"*/;
			}
			UI_DrawCustomProportionalString( xpos, ypos, s, UI_DROPSHADOW | UI_CENTER, scale / 2, color, qfalse );
		}
	}

	/*
	value = ps->stats[STAT_HEALTH];
	s = va("%i\x1E", value );
	UI_DrawCustomProportionalString( xpos - x, ypos, s, UI_DROPSHADOW | UI_RIGHT, scale, color, qfalse );

	value = floor ( ps->stats[STAT_LEVEL] / LV_SUBPOINT ) + 1;
	s = va("LV %i", value );
	UI_DrawCustomProportionalString( xpos - x, ypos - y, s, UI_DROPSHADOW | UI_RIGHT, scale / 2, color, qfalse );

	value = ps->stats[STAT_ARMOR];
	s = va( "\x1F%i", value );
	UI_DrawCustomProportionalString( xpos + x, ypos, s, UI_DROPSHADOW, scale, color, qfalse );

	if (value > 0) {
		value = ps->stats[STAT_ARMORTIER];
		s = va("LV %i", value );
		UI_DrawCustomProportionalString( xpos + x, ypos - y, s, UI_DROPSHADOW, scale / 2, color, qfalse );
	}

	if ( !(ps->persistant[PERS_MISC] & PMSC_RESTRICTED_PHYSICS) ) {
		if ( ps->persistant[PERS_MISC] & PMSC_PHYSICS_SELECTION ) {
			s = "ARENA";
		} else {
			s = "WORLD";
		}
		UI_DrawCustomProportionalString( xpos, ypos, s, UI_DROPSHADOW | UI_CENTER, scale / 2, color, qfalse );
	}

	*/

	// just for testing
	/*s = va("DEBUG: %i", ps->persistant[PERS_MISC] );
	UI_DrawCustomProportionalString( xpos + x, ypos - (y * 2), s, UI_DROPSHADOW, scale / 2, color, qfalse );*/
	/*value = floor ( ps->stats[STAT_LEVEL] / LV_SUBPOINT ) + 1;
	s = va("(%i) LV %i", ps->stats[STAT_LEVEL], value );
	UI_DrawCustomProportionalString( xpos - x, ypos - y, s, UI_DROPSHADOW | UI_RIGHT, scale / 2, color, qfalse );
	s = va("LV %i", ps->stats[STAT_ARMORTIER] );
	UI_DrawCustomProportionalString( xpos + x, ypos - y, s, UI_DROPSHADOW, scale / 2, color, qfalse );
	s = va("DEBUG: %i", ps->persistant[PERS_HITS] );
	UI_DrawCustomProportionalString( xpos + x, ypos - (y * 2), s, UI_DROPSHADOW, scale / 2, color, qfalse );*/

	// keycard inventory
	x = ( 52 * scale );
	inv = ps->stats[STAT_INVENTORY] & 7;
	hShader = cgs.media.hudKeycards; // keycards

	if (inv) {
		trap_R_DrawStretchPic( (float) (xpos + x) * cgs.screenXScale + cgs.screenXOffset,
						(float) (ypos - y) * cgs.screenYScale + cgs.screenYOffset,
						(float) 16 * scale * cgs.screenXScale, (float) 8 * scale * cgs.screenYScale,
						0.0, (float) inv / 8, 1.0, (float) inv / 8 + 0.125, hShader );
	}

}

/*
=================
HUD_MegaDrawWarmup
=================

TODO: redo the code
*/
void HUD_MegaDrawWarmup( int xpos, int ypos, float scale) {

	int				w;
	int				sec, tenth, hundredth;
	int				i;
	int				cw;
	qboolean		spectator;

	clientInfo_t	*ci1, *ci2;
	const char		*s;

	vec4_t			colorWht, colorBlk;

	colorWht[0] = colorWht[1] = colorWht[2] = colorWht[3] = 1.0; // rgba

	colorBlk[0] = colorBlk[1] = colorBlk[2] = 0.0; // rgb
	colorBlk[3] = 1.0; // alpha

	sec = cg.warmup;
	if ( !sec || cgs.voteTime ) {
		return;
	}

	spectator = ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR );

	if ( sec < 0 ) {

		if (spectator) {
			if (cg.time & 512)
				s = "Come on!";
			else
				s = "Join to battle!";

			UI_DrawCustomProportionalString( xpos, ypos, s, UI_DROPSHADOW | UI_CENTER, scale, colorWht, qfalse );
		} else {
			// TODO: check if current player needs to ready up
//			s = "Waiting for players";
			if (cg.time & 512) {
				s = "Waiting for more players!";
			} else {
				s = "All players should ready up!";
			}
			UI_DrawCustomProportionalString( xpos, ypos, s, UI_DROPSHADOW | UI_CENTER, scale, colorWht, qfalse );
			cg.warmupCount = 0;
		}
		return;
	}

	hundredth = ( cg.warmup - cg.time ) / 10;
	if ( hundredth < 0)
		hundredth = 0;
	tenth = hundredth / 10;
	sec = tenth / 10;
	hundredth -= tenth * 10;
	tenth -= sec * 10;

	// mmp - do we need this?
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}

	s = "GET READY TO BATTLE!";
	UI_DrawCustomProportionalString( xpos, ypos, s, UI_DROPSHADOW | UI_CENTER, scale, colorWht, qfalse );

	s = va ("T - %i.%i%i", sec, tenth, hundredth );
	UI_DrawCustomProportionalString( xpos, ypos + 16, s, UI_DROPSHADOW | UI_CENTER, scale / 2, colorWht, qfalse );

}

#define	SHOWKEYSWIDTH		80
/*
=================
HUD_MegaDrawShowKeys
=================
*/
void HUD_MegaDrawShowKeys( int xpos, int xoff, int ypos, int align, float scale ) {

//cg_keys_show
	float		x, y;
	char		*s;
	centity_t	*cent;
	playerState_t	*ps;
	int		value;
	qhandle_t	hShader;

	float		tileScale;
	int		keyGet;

	vec4_t		color1;
	vec4_t		color2;

	if ( align == 1)
		xpos -= ((SHOWKEYSWIDTH / 2) * scale);
	else if ( align == 2)
		xpos -= (SHOWKEYSWIDTH * scale);

	xpos += xoff;

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	hShader = cgs.media.hudButtonPress; // joypad

	HUD_FuncColorGet ( hud_keys_color.string, color1 );
	HUD_FuncColorGet ( hud_keys_pressColor.string, color2 );
	trap_R_SetColor( color1 ); // normal color

	tileScale = 32 * scale;
	trap_R_DrawStretchPic( (float)xpos * cgs.screenXScale + cgs.screenXOffset, (float)ypos * cgs.screenYScale + cgs.screenYOffset,
					tileScale * cgs.screenXScale, tileScale * cgs.screenYScale,
					(float) 0/4, (float) 0/4,
					(float) 2/4, (float) 2/4, hShader );

	tileScale *= 0.5;

	if ( ps->stats[STAT_CTRL] & CTRL_MOVEMENTDIR ) {
		trap_R_SetColor( color2 ); // highlight color

		keyGet = cg.snap->ps.movementDir;
		// dir ref:
		// 107
		// 2 6
		// 345

		// up
		if (keyGet == 1 || keyGet == 0 || keyGet == 7 ) {
			x = xpos + (8 * scale);
			y = ypos;
			trap_R_DrawStretchPic( (float)x * cgs.screenXScale + cgs.screenXOffset, (float)y * cgs.screenYScale + cgs.screenYOffset,
							tileScale * cgs.screenXScale, tileScale * cgs.screenYScale,
							(float) 2/4, (float) 0/4,
							(float) 3/4, (float) 1/4, hShader );
		}

		// down
		if (keyGet == 3 || keyGet == 4 || keyGet == 5 ) {
			x = xpos + (8 * scale);
			y = ypos + (16 * scale);
			trap_R_DrawStretchPic( (float)x * cgs.screenXScale + cgs.screenXOffset, (float)y * cgs.screenYScale + cgs.screenYOffset,
							tileScale * cgs.screenXScale, tileScale * cgs.screenYScale,
							(float) 3/4, (float) 0/4,
							(float) 4/4, (float) 1/4, hShader );
		}

		// left
		if (keyGet == 1 || keyGet == 2 || keyGet == 3 ) {
			x = xpos;
			y = ypos + (8 * scale);
			trap_R_DrawStretchPic( (float)x * cgs.screenXScale + cgs.screenXOffset, (float)y * cgs.screenYScale + cgs.screenYOffset,
							tileScale * cgs.screenXScale, tileScale * cgs.screenYScale,
							(float) 2/4, (float) 1/4,
							(float) 3/4, (float) 2/4, hShader );
		}

		// right
		if (keyGet == 5 || keyGet == 6 || keyGet == 7 ) {
			x = xpos + (16 * scale);
			y = ypos + (8 * scale);
			trap_R_DrawStretchPic( (float)x * cgs.screenXScale + cgs.screenXOffset, (float)y * cgs.screenYScale + cgs.screenYOffset,
							tileScale * cgs.screenXScale, tileScale * cgs.screenYScale,
							(float) 3/4, (float) 1/4,
							(float) 4/4, (float) 2/4, hShader );
		}
	}

	keyGet = ps->stats[STAT_CTRL];

	// attack
	x = xpos + (32 * scale);
	y = ypos + (16 * scale);
	if ( keyGet & CTRL_ATTACKPRESS )
		trap_R_SetColor( color2 ); // highlight color
	else
		trap_R_SetColor( color1 ); // normal color
	trap_R_DrawStretchPic( (float)x * cgs.screenXScale + cgs.screenXOffset, (float)y * cgs.screenYScale + cgs.screenYOffset,
					tileScale * cgs.screenXScale, tileScale * cgs.screenYScale,
					(float) 0/4, (float) 2/4,
					(float) 1/4, (float) 3/4, hShader );

	// jump
	x += (16 * scale);
	y -= (8 * scale);
	if ( keyGet & CTRL_JUMPPRESS )
		trap_R_SetColor( color2 ); // highlight color
	else
		trap_R_SetColor( color1 ); // normal color
	trap_R_DrawStretchPic( (float)x * cgs.screenXScale + cgs.screenXOffset, (float)y * cgs.screenYScale + cgs.screenYOffset,
					tileScale * cgs.screenXScale, tileScale * cgs.screenYScale,
					(float) 1/4, (float) 2/4,
					(float) 2/4, (float) 3/4, hShader );

	// duck
	x += (16 * scale);
	y -= (8 * scale);
	if ( keyGet & CTRL_DUCKPRESS )
		trap_R_SetColor( color2 ); // highlight color
	else
		trap_R_SetColor( color1 ); // normal color
	trap_R_DrawStretchPic( (float)x * cgs.screenXScale + cgs.screenXOffset, (float)y * cgs.screenYScale + cgs.screenYOffset,
					tileScale * cgs.screenXScale, tileScale * cgs.screenYScale,
					(float) 2/4, (float) 2/4,
					(float) 3/4, (float) 3/4, hShader );

	/*if ( ps->stats[STAT_CTRL] & CTRL_MOVEMENTDIR ) {
		s = va("%i", cg.snap->ps.movementDir );
		UI_DrawCustomProportionalString( xpos, ypos, s, style, scale, color1, qfalse );
	}

	s = va("%i", ps->stats[STAT_CTRL]);
	UI_DrawCustomProportionalString( xpos + 16 , ypos, s, style, scale, color1, qfalse );*/

}


/*
=================
HUD_MegaDrawClientNum

draws the client number of who the spectator is following
=================
*/
void HUD_MegaDrawClientNum( int xpos, int ypos ) {
	char		*s;
	vec4_t		color;

	color[0] = color[1] = color[2] = color[3] = 1.0;
	trap_R_SetColor( NULL );
	CG_DrawPic( xpos, ypos, 16, 16, cgs.media.iconCam );

	s = va("%i", cg.snap->ps.clientNum);
	UI_DrawCustomProportionalString( xpos + 16 , ypos + 1, s, 0, 1.0, color, qtrue );
}

/*
=========
LAGOMETER
=========
*/

#define	LAG_SAMPLES		128


typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;

/*
==============
HUD_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void HUD_AddLagometerFrameInfo( void ) {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
HUD_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void HUD_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
HUD_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void HUD_DrawDisconnect( void ) {
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;
	const char		*s;
	int			w;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}

	// also add text in center of screen
	s = "Connection Interrupted";
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString( 320 - w/2, 100, s, 1.0F);

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	x = 640 - 48;
	y = 480 - 48;

	CG_DrawPic( x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga" ) );
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

#define	HUDALIGN_LEFT		3
#define	HUDALIGN_RIGHT		4
#define	HUDALIGN_TOP		1
#define	HUDALIGN_BOTTOM		2

/*
==============
HUD_DrawLagometer
==============
*/
static void HUD_DrawLagometer( int x, int y, int w, int h, int align, vec4_t bgColor1, vec4_t bgColor2 ) {
	int		a, i, h2, alx, aly;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	vec4_t		colorWht;
	colorWht[0] = colorWht[1] = colorWht[2] = colorWht[3] = 1.0; // rgba

	if ( !cg_lagometer.integer /*|| cgs.localServer*/ ) {
		/*HUD_DrawDisconnect();*/
		return;
	}

	//
	// draw the graph
	//
/*
	x = 640 - 48;
	y = 480 - 48;
*/

	h2 = h / 2; // half of height size
	trap_R_SetColor( bgColor1 );
	CG_DrawPic( x, y, w, h2, cgs.media.whiteShader );
	trap_R_SetColor( bgColor2 );
	CG_DrawPic( x, y + h2, w, h2, cgs.media.whiteShader );

	trap_R_SetColor( NULL );
	//CG_DrawPic( x, y, 48, 48, cgs.media.lagometerShader );

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	trap_R_SetColor( NULL );

	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		UI_DrawCustomProportionalString( x + (w / 2), y, "SNC", UI_DROPSHADOW || UI_CENTER, 1.0, colorWht, qtrue );
	}

	/*HUD_DrawDisconnect();*/
}


/*
=================
HUD_MegaDrawTeamOverlay

TODO: allow element to be aligned and such
=================
*/

#define	HUD_TEAM_OVERLAY_WIDTH			232 // was 224
#define	HUD_TEAM_OVERLAY_WIDTHADJ		222
#define	HUD_TEAM_OVERLAY_GUTTER			4
#define	HUD_TEAM_OVERLAY_NAMEYOFFSET	4
#define	HUD_TEAM_OVERLAY_MAXNAMEWIDTH	92
#define	HUD_TEAM_OVERLAY_MAXLOCWIDTH	124
#define	HUD_TEAM_OVERLAY_SLOTHEIGHT		16
#define	HUD_TEAM_OVERLAY_HALFHEIGHT		8
#define	HUD_TEAM_OVERLAY_HEALTH			160
#define	HUD_TEAM_OVERLAY_DMGLVL			128
#define	HUD_TEAM_OVERLAY_ARMOR			176
#define	HUD_TEAM_OVERLAY_ARMORLVL		208
#define	HUD_TEAM_OVERLAY_WEAPON			164
#define HUD_TEAM_OVERLAY_POWERUP		96

void HUD_MegaDrawTeamOverlay ( int xpos, int xoff, int ypos, int align, float scale ) {

	int			x, y, /*w, */h, xx, yy, yset_name, yset_hi, yset_lo;
	int			bgx1, bgx2, bgw1, bgw2, bgy, bgh;
	int			i, j, num, len;
	int			strXSize;
	float		strXScale;
	const char	*p;
	vec4_t		hcolor;
	vec4_t		bgcolor;
	int			pwidth, lwidth;
	int			plyrs;
	char		string[32];
	clientInfo_t	*ci;
	gitem_t		*item;
	int			ret_y, count;

	if ( !hud_teamOverlay_show.integer ) {
		return;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE ) {
		return; // Not on any team
	}

	switch(align) {
		case 1:
			// align center
			x = xpos - (HUD_TEAM_OVERLAY_WIDTH / 2) * scale;
			break;
		case 2:
			// align right
			x = xpos - HUD_TEAM_OVERLAY_WIDTH * scale;
			break;
		default:
			// align left
			x = xpos;
			break;
	}

	y = ypos;// * cgs.screenYScale;
	plyrs = 0;

	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
			plyrs++;
		}
	}

	// if there are no players, don't bother
	if (!plyrs) {
		return;
	}

	//w = HUD_TEAM_OVERLAY_WIDTH * scale;
	x += xoff + (HUD_TEAM_OVERLAY_GUTTER * scale);
	yy = HUD_TEAM_OVERLAY_SLOTHEIGHT * scale;
	h = plyrs * yy;

	bgx1 = x;
	bgw1 = 112 * scale;
	bgx2 = x + bgw1;
	bgw2 = 112 * scale;
	bgh = 8 * scale;

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		bgcolor[0] = 1.0f;
		bgcolor[1] = 0.0f;
		bgcolor[2] = 0.0f;
		bgcolor[3] = 0.33f;
	} else { // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		bgcolor[0] = 0.0f;
		bgcolor[1] = 0.0f;
		bgcolor[2] = 1.0f;
		bgcolor[3] = 0.33f;
	}
	/*trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );*/

	hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
	hcolor[3] = 1.0f;

	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {

			yset_hi = y + ( i * yy );
			yset_lo = y + ( i * yy ) + ( HUD_TEAM_OVERLAY_HALFHEIGHT * scale );
			yset_name = y + ( i * yy ) + ( HUD_TEAM_OVERLAY_NAMEYOFFSET * scale );

			trap_R_SetColor( bgcolor );
			CG_DrawPicExt( bgx1, yset_lo + 1, bgw1, bgh, 0, 0.25, 0.875, 0.5, cgs.media.teamBG );
			CG_DrawPicExt( bgx2, yset_lo + 1, bgw2, bgh, 0.125, 0.75, 1, 1, cgs.media.teamBG );
			trap_R_SetColor( NULL );

			// client name
			Com_sprintf(string, sizeof(string), ci->name);
			strXSize = UI_ReturnStringWidth (string, qfalse) * 0.5;
			if ( strXSize > HUD_TEAM_OVERLAY_MAXNAMEWIDTH ) {
				strXScale = ( HUD_TEAM_OVERLAY_MAXNAMEWIDTH / (float)strXSize) * 0.5;
			} else {
				strXScale = 0.5;
			}
			UI_DrawString( x + 2 * (int)scale, yset_name, string, UI_DROPSHADOW, strXScale * scale, 0.5 * scale, hcolor, qfalse );

			// client loc
			p = CG_ConfigString(CS_LOCATIONS + ci->location);
			if (!p || !*p) {
				p = "Fucking around";
			}

			strXSize = UI_ReturnStringWidth (p, qfalse) * 0.5;
			if ( strXSize > HUD_TEAM_OVERLAY_MAXLOCWIDTH ) {
				strXScale = ( HUD_TEAM_OVERLAY_MAXLOCWIDTH / (float)strXSize) * 0.5;
			} else {
				strXScale = 0.5;
			}
			UI_DrawString( x + (int)((float)HUD_TEAM_OVERLAY_WIDTHADJ * scale), yset_hi, p, UI_DROPSHADOW | UI_RIGHT, strXScale * scale, 0.5 * scale, hcolor, qfalse );

			// health, armor, etc.
			Com_sprintf(string, sizeof(string), va("%i\x1E", ci->health));
			UI_DrawString( x + (int)((float)HUD_TEAM_OVERLAY_HEALTH * scale), yset_lo, string, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, 0.5 * scale, hcolor, qfalse );

			Com_sprintf(string, sizeof(string), va("LV%i", ci->damageLvl));
			UI_DrawString( x + (int)((float)HUD_TEAM_OVERLAY_DMGLVL * scale), yset_lo, string, UI_DROPSHADOW | UI_RIGHT, 0.3 * scale, 0.3 * scale, hcolor, qfalse );

			num = ci->armor;
			Com_sprintf(string, sizeof(string), va("\x1F%i", ci->armor));
			UI_DrawString( x + (int)((float)HUD_TEAM_OVERLAY_ARMOR * scale), yset_lo, string, UI_DROPSHADOW, 0.5 * scale, 0.5 * scale, hcolor, qfalse );

			if (num) {
				Com_sprintf(string, sizeof(string), va("LV%i", ci->armorLvl));
				UI_DrawString( x + (int)((float)HUD_TEAM_OVERLAY_ARMORLVL * scale), yset_lo, string, UI_DROPSHADOW, 0.3 * scale, 0.3 * scale, hcolor, qfalse );
			}

			if ( cg_weapons[ci->curWeapon].weaponIcon ) {
				CG_DrawPic(  x + (int)((float)HUD_TEAM_OVERLAY_WEAPON * scale), yset_lo, HUD_TEAM_OVERLAY_HALFHEIGHT * scale, HUD_TEAM_OVERLAY_HALFHEIGHT * scale,
					cg_weapons[ci->curWeapon].weaponIcon );
			} else {
				CG_DrawPic(  x + (int)((float)HUD_TEAM_OVERLAY_WEAPON * scale), yset_lo, HUD_TEAM_OVERLAY_HALFHEIGHT * scale, HUD_TEAM_OVERLAY_HALFHEIGHT * scale,
					cgs.media.deferShader );
			}

			// power ups
			// TODO: perhaps a less cpu wasting way of displaying powerup icons
			// TODO: maybe fit info about keycards
			xx = x + (int)((float)HUD_TEAM_OVERLAY_POWERUP * scale);

			// if we have more than two, flash between the two...  if more, well, fuck it, just show two  :/
			if ( cg.time & 256 ) {

				for (j = 0; j <= PW_NUM_POWERUPS; j++) {
					if (ci->powerups & (1 << j)) {
						item = BG_FindItemForPowerup( j );

						if (item) {
							CG_DrawPic( xx, yset_lo, HUD_TEAM_OVERLAY_HALFHEIGHT * scale, HUD_TEAM_OVERLAY_HALFHEIGHT * scale,
							trap_R_RegisterShader( item->icon ) );
							//xx -= HUD_TEAM_OVERLAY_HALFHEIGHT * scale;
							break;
						}
					}
				}

			} else {

				for (j = PW_NUM_POWERUPS - 1; j >= 0; j--) {
					if (ci->powerups & (1 << j)) {
						item = BG_FindItemForPowerup( j );

						if (item) {
							CG_DrawPic( xx, yset_lo, HUD_TEAM_OVERLAY_HALFHEIGHT * scale, HUD_TEAM_OVERLAY_HALFHEIGHT * scale,
							trap_R_RegisterShader( item->icon ) );
							//xx -= HUD_TEAM_OVERLAY_HALFHEIGHT * scale;
							break;
						}
					}
				}

			}


		}

	}


	// reference
	// =========
	/*

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
			plyrs++;
			len = CG_DrawStrlen(ci->name);
			if (len > pwidth)
				pwidth = len;
		}
	}

	if (!plyrs)
		return;

	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	for (i = 1; i < MAX_LOCATIONS; i++) {
		p = CG_ConfigString(CS_LOCATIONS + i);
		if (p && *p) {
			len = CG_DrawStrlen(p);
			if (len > lwidth)
				lwidth = len;
		}
	}

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	x = xpos - w;

	h = plyrs * TINYCHAR_HEIGHT;

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		hcolor[0] = 1.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 0.0f;
		hcolor[3] = 0.33f;
	} else { // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = 0.33f;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );

	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt( xx, y,
				ci->name, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);

			if (lwidth) {
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if (!p || !*p)
					p = "unknown";

				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
				CG_DrawStringExt( xx, y,
					p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					TEAM_OVERLAY_MAXLOCATION_WIDTH);
			}

			CG_GetColorForHealth( ci->health, ci->armor, hcolor );

			Com_sprintf (st, sizeof(st), "%3i %3i", ci->health,	ci->armor);

			xx = x + TINYCHAR_WIDTH * 3 +
				TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt( xx, y,
				st, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if ( cg_weapons[ci->curWeapon].weaponIcon ) {
				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					cg_weapons[ci->curWeapon].weaponIcon );
			} else {
				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					cgs.media.deferShader );
			}

			// Draw powerup icons
			if (right) {
				xx = x;
			} else {
				xx = x + w - TINYCHAR_WIDTH;
			}
			for (j = 0; j <= PW_NUM_POWERUPS; j++) {
				if (ci->powerups & (1 << j)) {

					item = BG_FindItemForPowerup( j );

					if (item) {
						CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
						trap_R_RegisterShader( item->icon ) );
						if (right) {
							xx -= TINYCHAR_WIDTH;
						} else {
							xx += TINYCHAR_WIDTH;
						}
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	*/

	return;
//#endif
}


/*
=================
HUD_MegaDrawStatus...
=================
*/
#define	HUD_STATUS_ICON_SIZE		16
#define	HUD_STATUS_ICON_HALF_SIZE	HUD_STATUS_ICON_SIZE / 2

#define	HUD_STATUS_KEYCARDLAYOUT_XSIZE		32
#define	HUD_STATUS_KEYCARDLAYOUT_HALF_XSIZE	HUD_STATUS_KEYCARDLAYOUT_XSIZE / 2
#define	HUD_STATUS_KEYCARDLAYOUT_YSIZE		16

void HUD_MegaDrawStatusHealth( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		color;
	centity_t	*cent;
	playerState_t	*ps;
	clientInfo_t	*ci;
	char		*s;
	int			value;

	if ( !hud_statusHealth_show.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	ci = &cgs.clientinfo[cg.snap->ps.clientNum];

	value = ps->stats[STAT_HEALTH];

	if (hud_statusHealth_colorType.integer==1) {
		switch ( team) {
			case TEAM_RED:
				if (value > HUD_HEALTH_OVER) {
					// over 100 health
					HUD_FuncColorGet ( hud_statusHealth_colorHighRed.string, color );
				} else if (value >= HUD_HEALTH_LOW) {
					// 25 - 100 health
					HUD_FuncColorGet ( hud_statusHealth_colorNormalRed.string, color );
				} else {
					// hide if value is zero, and if clients wants it that way
					if (value <= 0 && !hud_statusHealth_showZero.integer) {
						return;
					} else if ( (value > HUD_HEALTH_VERY_LOW && !(cg.time & 256)) || (value < HUD_HEALTH_VERY_LOW + 1 && !(cg.time & 128) ) ){
						// < 25 health
						HUD_FuncColorGet ( hud_statusHealth_colorLowRed.string, color );
					} else {
						// < 25 health flash
						HUD_FuncColorGet ( hud_statusHealth_colorLowFlashRed.string, color );
					}
				}
				break;

			case TEAM_BLUE:
				if (value > HUD_HEALTH_OVER) {
					// over 100 health
					HUD_FuncColorGet ( hud_statusHealth_colorHighBlue.string, color );
				} else if (value >= HUD_HEALTH_LOW) {
					// 25 - 100 health
					HUD_FuncColorGet ( hud_statusHealth_colorNormalBlue.string, color );
				} else {
					// hide if value is zero, and if clients wants it that way
					if (value <= 0 && !hud_statusHealth_showZero.integer) {
						return;
					} else if ( (value > HUD_HEALTH_VERY_LOW && !(cg.time & 256)) || (value < HUD_HEALTH_VERY_LOW + 1 && !(cg.time & 128) ) ){
						// < 25 health
						HUD_FuncColorGet ( hud_statusHealth_colorLowBlue.string, color );
					} else {
						// < 25 health flash
						HUD_FuncColorGet ( hud_statusHealth_colorLowFlashBlue.string, color );
					}
				}
				break;

			default:
				if (value > HUD_HEALTH_OVER) {
					// over 100 health
					HUD_FuncColorGet ( hud_statusHealth_colorHigh.string, color );
				} else if (value >= HUD_HEALTH_LOW) {
					// 25 - 100 health
					HUD_FuncColorGet ( hud_statusHealth_colorNormal.string, color );
				} else {
					// hide if value is zero, and if clients wants it that way
					if (value <= 0 && !hud_statusHealth_showZero.integer) {
						return;
					} else if ( (value > HUD_HEALTH_VERY_LOW && !(cg.time & 256)) || (value < HUD_HEALTH_VERY_LOW + 1 && !(cg.time & 128) ) ){
						// < 25 health
						HUD_FuncColorGet ( hud_statusHealth_colorLow.string, color );
					} else {
						// < 25 health flash
						HUD_FuncColorGet ( hud_statusHealth_colorLowFlash.string, color );
					}
				}
		}
	} else {
		HUD_FuncColorGet ( hud_statusHealth_colorNormal.string, color );
	}

	style = HUD_FuncStyleSet ( align, style );
	s = va( "%i", value );
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );

}

void HUD_MegaDrawStatusHealthIcon( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		color;
	vec4_t		colorBlack;
	centity_t	*cent;
	playerState_t	*ps;
	clientInfo_t	*ci;
	char		*s;
	int		value;

	if ( !hud_statusHealthIcon_show.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	ci = &cgs.clientinfo[cg.snap->ps.clientNum];

	value = ps->stats[STAT_HEALTH];

	if (hud_statusHealthIcon_colorType.integer==1) {
		switch ( team) {
			case TEAM_RED:
				if (value > HUD_HEALTH_OVER) {
					// over 100 health
					HUD_FuncColorGet ( hud_statusHealthIcon_colorHighRed.string, color );
				} else if (value >= HUD_HEALTH_LOW) {
					// 25 - 100 health
					HUD_FuncColorGet ( hud_statusHealthIcon_colorNormalRed.string, color );
				} else {
					// hide if value is zero, and if clients wants it that way
					if (value <= 0 && !hud_statusHealthIcon_showZero.integer) {
						return;
					} else if ( (value > HUD_HEALTH_VERY_LOW && !(cg.time & 256)) || (value < HUD_HEALTH_VERY_LOW + 1 && !(cg.time & 128) ) ){
						// < 25 health
						HUD_FuncColorGet ( hud_statusHealthIcon_colorLowRed.string, color );
					} else {
						// < 25 health flash
						HUD_FuncColorGet ( hud_statusHealthIcon_colorLowFlashRed.string, color );
					}
				}
				break;

			case TEAM_BLUE:
				if (value > HUD_HEALTH_OVER) {
					// over 100 health
					HUD_FuncColorGet ( hud_statusHealthIcon_colorHighBlue.string, color );
				} else if (value >= HUD_HEALTH_LOW) {
					// 25 - 100 health
					HUD_FuncColorGet ( hud_statusHealthIcon_colorNormalBlue.string, color );
				} else {
					// hide if value is zero, and if clients wants it that way
					if (value <= 0 && !hud_statusHealthIcon_showZero.integer) {
						return;
					} else if ( (value > HUD_HEALTH_VERY_LOW && !(cg.time & 256)) || (value < HUD_HEALTH_VERY_LOW + 1 && !(cg.time & 128) ) ){
						// < 25 health
						HUD_FuncColorGet ( hud_statusHealthIcon_colorLowBlue.string, color );
					} else {
						// < 25 health flash
						HUD_FuncColorGet ( hud_statusHealthIcon_colorLowFlashBlue.string, color );
					}
				}
				break;

			default:
				if (value > HUD_HEALTH_OVER) {
					// over 100 health
					HUD_FuncColorGet ( hud_statusHealthIcon_colorHigh.string, color );
				} else if (value >= HUD_HEALTH_LOW) {
					// 25 - 100 health
					HUD_FuncColorGet ( hud_statusHealthIcon_colorNormal.string, color );
				} else {
					// hide if value is zero, and if clients wants it that way
					if (value <= 0 && !hud_statusHealthIcon_showZero.integer) {
						return;
					} else if ( (value > HUD_HEALTH_VERY_LOW && !(cg.time & 256)) || (value < HUD_HEALTH_VERY_LOW + 1 && !(cg.time & 128) ) ){
						// < 25 health
						HUD_FuncColorGet ( hud_statusHealthIcon_colorLow.string, color );
					} else {
						// < 25 health flash
						HUD_FuncColorGet ( hud_statusHealthIcon_colorLowFlash.string, color );
					}
				}
		}
	} else {
		HUD_FuncColorGet ( hud_statusHealthIcon_colorNormal.string, color );
	}

	// position lock
	if ( posLock > 0 && posLock <= HUD_POS_LOCK_TYPES ) {
	/*
	123
	456
	*/

	// text position is stripped as well
	switch( posLock ) {
		case 2:
		case 5:
			xpos = 320 * hud_aspectRatioScale.value - ( HUD_STATUS_ICON_HALF_SIZE * scaleX );
			break;

		case 3:
		case 6:
			xpos = 640 * hud_aspectRatioScale.value - ( 4 + ( HUD_STATUS_ICON_SIZE * scaleX ) ) ;
			break;

		default:
			xpos = 4;
	}

	if ( posLock > 3 ) {
		cg.ypos[posLock-1] -= HUD_STATUS_ICON_SIZE * scaleY + 4;
		ypos = cg.ypos[posLock-1];
	}
	else {
		ypos = cg.ypos[posLock-1];
		cg.ypos[posLock-1] += HUD_STATUS_ICON_SIZE * scaleY + 4;
	}

	} else {
		xpos += xoff;
		switch ( align ) {
			case 1: // center
				xpos -= HUD_STATUS_ICON_HALF_SIZE * scaleX;
				break;
			case 2: // right
				xpos -= HUD_STATUS_ICON_SIZE * scaleX;
		}
	}

	if ( style == 1 ) {
		colorBlack[0] = colorBlack[1] = colorBlack[2] = 0.0;
		colorBlack[3] = color[3];
		trap_R_SetColor( colorBlack );
		trap_R_DrawStretchPic( ( xpos * cgs.screenXScale ) + 1 + cgs.screenXOffset,
								( ypos * cgs.screenYScale ) + 1 + cgs.screenYOffset,
								HUD_STATUS_ICON_SIZE * scaleX * cgs.screenXScale,
								HUD_STATUS_ICON_SIZE * scaleY * cgs.screenYScale,
								0, 0, 1, 1, cgs.media.healthIconWhite );
	}
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( xpos * cgs.screenXScale + cgs.screenXOffset
								, ypos * cgs.screenYScale + cgs.screenYOffset,
								HUD_STATUS_ICON_SIZE * scaleX * cgs.screenXScale,
								HUD_STATUS_ICON_SIZE * scaleY * cgs.screenYScale,
								0, 0, 1, 1, cgs.media.healthIconWhite );

}

void HUD_MegaDrawStatusArmor( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		color;
	centity_t	*cent;
	playerState_t	*ps;
	char		*s;
	int			value;
	int			tier;
	int			breakPoint;

	if ( !hud_statusArmor_show.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = ps->stats[STAT_ARMOR];
	tier = ps->stats[STAT_ARMORTIER];
	if (tier == 3) {
		breakPoint = ceil (AR_BREAKPOINT_3TO2);
	} else if (tier == 2) {
		breakPoint = ceil (AR_BREAKPOINT_2TO1);
	} else {
		breakPoint = 666;
	}

	if (value <= 0 && !hud_statusArmor_showZero.integer) {
		return;
	}

	if (hud_statusArmor_colorType.integer == 2) {
		switch ( tier ) {
			case 1:
				HUD_FuncColorGet ( hud_statusArmor_colorTier1.string, color );
				break;
			case 2:
				HUD_FuncColorGet ( hud_statusArmor_colorTier2.string, color );
				break;
			case 3:
				HUD_FuncColorGet ( hud_statusArmor_colorTier3.string, color );
				break;
			default:
				// there should never be an out of range tier, but just incase
				HUD_FuncColorGet ( hud_statusArmor_color.string, color );
		}
	} else if (hud_statusArmor_colorType.integer == 1) {
		if ( hud_statusArmor_colorBreakPoint.integer && value >= breakPoint ) {
			switch ( team ) {
				case TEAM_RED:
					HUD_FuncColorGet ( hud_statusArmor_colorRedBreak.string, color );
					break;

				case TEAM_BLUE:
					HUD_FuncColorGet ( hud_statusArmor_colorBlueBreak.string, color );
					break;

				default:
					HUD_FuncColorGet ( hud_statusArmor_colorBreak.string, color );
			}
		} else {
			switch ( team ) {
				case TEAM_RED:
					HUD_FuncColorGet ( hud_statusArmor_colorRed.string, color );
					break;

				case TEAM_BLUE:
					HUD_FuncColorGet ( hud_statusArmor_colorBlue.string, color );
					break;

				default:
					HUD_FuncColorGet ( hud_statusArmor_color.string, color );
			}
		}
	} else {
		HUD_FuncColorGet ( hud_statusArmor_color.string, color );
	}

	style = HUD_FuncStyleSet ( align, style );
	s = va( "%i", value );
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );

}

void HUD_MegaDrawStatusArmorIcon( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		color;
	vec4_t		colorBlack;
	centity_t	*cent;
	playerState_t	*ps;
	char		*s;
	int		value;

	if ( !hud_statusArmorIcon_show.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = ps->stats[STAT_ARMOR];

	if (value <= 0 && !hud_statusArmorIcon_showZero.integer) {
		return;
	}

	if (hud_statusArmorIcon_colorType.integer == 2) {
		switch ( ps->stats[STAT_ARMORTIER] ) {
			case 1:
				HUD_FuncColorGet ( hud_statusArmorIcon_colorTier1.string, color );
				break;
			case 2:
				HUD_FuncColorGet ( hud_statusArmorIcon_colorTier2.string, color );
				break;
			case 3:
				HUD_FuncColorGet ( hud_statusArmorIcon_colorTier3.string, color );
				break;
			default:
				// there should never be an out of range tier, but just incase
				HUD_FuncColorGet ( hud_statusArmorIcon_color.string, color );
		}
	} else if (hud_statusArmorIcon_colorType.integer == 1) {
		switch ( team ) {
			case TEAM_RED:
				HUD_FuncColorGet ( hud_statusArmorIcon_colorRed.string, color );
				break;

			case TEAM_BLUE:
				HUD_FuncColorGet ( hud_statusArmorIcon_colorBlue.string, color );
				break;

			default:
				HUD_FuncColorGet ( hud_statusArmorIcon_color.string, color );
		}
	} else {
		HUD_FuncColorGet ( hud_statusArmorIcon_color.string, color );
	}

	// position lock
	if ( posLock > 0 && posLock <= HUD_POS_LOCK_TYPES ) {
	/*
	123
	456
	*/

	// text position is stripped as well
	switch( posLock ) {
		case 2:
		case 5:
			xpos = 320 * hud_aspectRatioScale.value - ( HUD_STATUS_ICON_HALF_SIZE * scaleX );
			break;

		case 3:
		case 6:
			xpos = 640 * hud_aspectRatioScale.value - ( 4 + ( HUD_STATUS_ICON_SIZE * scaleX ) ) ;
			break;

		default:
			xpos = 4;
	}

	if ( posLock > 3 ) {
		cg.ypos[posLock-1] -= HUD_STATUS_ICON_SIZE * scaleY + 4;
		ypos = cg.ypos[posLock-1];
	}
	else {
		ypos = cg.ypos[posLock-1];
		cg.ypos[posLock-1] += HUD_STATUS_ICON_SIZE * scaleY + 4;
	}

	} else {
		xpos += xoff;
		switch ( align ) {
			case 1: // center
				xpos -= HUD_STATUS_ICON_HALF_SIZE * scaleX;
				break;
			case 2: // right
				xpos -= HUD_STATUS_ICON_SIZE * scaleX;
		}
	}

	if ( style == 1 ) {
		colorBlack[0] = colorBlack[1] = colorBlack[2] = 0.0;
		colorBlack[3] = color[3];
		trap_R_SetColor( colorBlack );
		trap_R_DrawStretchPic( ( xpos * cgs.screenXScale ) + 1 + cgs.screenXOffset, ( ypos * cgs.screenYScale ) + 1 + cgs.screenYOffset,
								HUD_STATUS_ICON_SIZE * scaleX * cgs.screenXScale,
								HUD_STATUS_ICON_SIZE * scaleY * cgs.screenYScale,
								0, 0, 1, 1, cgs.media.armorIconWhite );
	}
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( xpos * cgs.screenXScale + cgs.screenXOffset, ypos * cgs.screenXScale + cgs.screenYOffset,
								HUD_STATUS_ICON_SIZE * scaleX * cgs.screenXScale,
								HUD_STATUS_ICON_SIZE * scaleY * cgs.screenYScale,
								0, 0, 1, 1, cgs.media.armorIconWhite );

}

void HUD_MegaDrawStatusArmorTier( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		color;
	centity_t	*cent;
	playerState_t	*ps;
	char		*s;
	int		value;

	if ( !hud_statusArmorTier_show.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = ps->stats[STAT_ARMOR];

	if (value <= 0 && !hud_statusArmorTier_showZero.integer) {
		return;
	}

	value = ps->stats[STAT_ARMORTIER];

	if (hud_statusArmorTier_colorType.integer == 2) {
		switch ( value ) {
			case 1:
				HUD_FuncColorGet ( hud_statusArmorTier_colorTier1.string, color );
				break;
			case 2:
				HUD_FuncColorGet ( hud_statusArmorTier_colorTier2.string, color );
				break;
			case 3:
				HUD_FuncColorGet ( hud_statusArmorTier_colorTier3.string, color );
				break;
			default:
				// there should never be an out of range tier, but just incase
				HUD_FuncColorGet ( hud_statusArmorTier_color.string, color );
		}
	} else if (hud_statusArmorTier_colorType.integer == 1) {
		switch ( team ) {
			case TEAM_RED:
				HUD_FuncColorGet ( hud_statusArmorTier_colorRed.string, color );
				break;

			case TEAM_BLUE:
				HUD_FuncColorGet ( hud_statusArmorTier_colorBlue.string, color );
				break;

			default:
				HUD_FuncColorGet ( hud_statusArmorTier_color.string, color );
		}
	} else {
		HUD_FuncColorGet ( hud_statusArmorTier_color.string, color );
	}

	style = HUD_FuncStyleSet ( align, style );
	switch ( hud_statusArmorTier_type.integer ) {
		case 1:
			s = va( "LV %i", value );
			break;
		case 2:
			s = va( "LEVEL %i", value );
			break;
		case 3:
			s = va( "TIER %i", value );
			break;
		default:
			s = va( "%i", value );
	}
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );

}

void HUD_MegaDrawStatusLevel( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		color;
	centity_t	*cent;
	playerState_t	*ps;
	char		*s;
	int		value;

	if ( !hud_statusLevel_show.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = floor ( ps->stats[STAT_LEVEL] / LV_SUBPOINT ) + 1;

	if (hud_statusLevel_colorType.integer == 1) {
		switch ( team ) {
			case TEAM_RED:
				HUD_FuncColorGet ( hud_statusLevel_colorRed.string, color );
				break;

			case TEAM_BLUE:
				HUD_FuncColorGet ( hud_statusLevel_colorBlue.string, color );
				break;

			default:
				HUD_FuncColorGet ( hud_statusLevel_color.string, color );
		}
	} else {
		HUD_FuncColorGet ( hud_statusLevel_color.string, color );
	}

	style = HUD_FuncStyleSet ( align, style );
	switch ( hud_statusLevel_type.integer ) {
		case 1:
			s = va( "LV %i", value );
			break;
		case 2:
			s = va( "LEVEL %i", value );
			break;
		default:
			s = va( "%i", value );
	}
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );

}

void HUD_MegaDrawStatusPhysics( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		color;
	centity_t	*cent;
	playerState_t	*ps;
	char		*s;
	int		value;

	if ( !hud_statusPhysics_show.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	if ( ps->persistant[PERS_MISC] & PMSC_RESTRICTED_PHYSICS) {
		return;
	}

	if (hud_statusPhysics_colorType.integer == 1) {
		switch ( team ) {
			case TEAM_RED:
				HUD_FuncColorGet ( hud_statusPhysics_colorRed.string, color );
				break;

			case TEAM_BLUE:
				HUD_FuncColorGet ( hud_statusPhysics_colorBlue.string, color );
				break;

			default:
				HUD_FuncColorGet ( hud_statusPhysics_color.string, color );
		}
	} else {
		HUD_FuncColorGet ( hud_statusPhysics_color.string, color );
	}

	style = HUD_FuncStyleSet ( align, style );
	switch ( hud_statusPhysics_type.integer ) {
		case 1:
			if ( ps->persistant[PERS_MISC] & PMSC_PHYSICS_SELECTION ) {
				s = "ARENA";
			} else {
				s = "WORLD";
			}
			break;
		default:
			if ( ps->persistant[PERS_MISC] & PMSC_PHYSICS_SELECTION ) {
				s = "A"; // arena physics
			} else {
				s = "W"; // world physics
			}
	}
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );

}

void HUD_MegaDrawStatusKeycards( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		colorBlack;
	centity_t	*cent;
	playerState_t	*ps;
	char		*s;
	int		value;

	if ( !hud_statusKeycards_show.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = ps->stats[STAT_INVENTORY] & 7;

	if (value == 0) {
		return;
	}

	// position lock
	if ( posLock > 0 && posLock <= HUD_POS_LOCK_TYPES ) {
	/*
	123
	456
	*/

	// text position is stripped as well
	switch( posLock ) {
		case 2:
		case 5:
			xpos = 320 * hud_aspectRatioScale.value - ( HUD_STATUS_KEYCARDLAYOUT_HALF_XSIZE * scaleX );
			break;

		case 3:
		case 6:
			xpos = 640 * hud_aspectRatioScale.value - ( 4 + ( HUD_STATUS_KEYCARDLAYOUT_XSIZE * scaleX ) ) ;
			break;

		default:
			xpos = 4;
	}

	if ( posLock > 3 ) {
		cg.ypos[posLock-1] -= HUD_STATUS_KEYCARDLAYOUT_YSIZE * scaleY + 4;
		ypos = cg.ypos[posLock-1];
	}
	else {
		ypos = cg.ypos[posLock-1];
		cg.ypos[posLock-1] += HUD_STATUS_KEYCARDLAYOUT_YSIZE * scaleY + 4;
	}

	} else {
		xpos += xoff;
		switch ( align ) {
			case 1: // center
				xpos -= HUD_STATUS_KEYCARDLAYOUT_HALF_XSIZE * scaleX;
				break;
			case 2: // right
				xpos -= HUD_STATUS_KEYCARDLAYOUT_XSIZE * scaleX;
		}
	}

	if ( style == 1 ) {
		colorBlack[0] = colorBlack[1] = colorBlack[2] = 0.0;
		colorBlack[3] = 1.0;
		trap_R_SetColor( colorBlack );
		trap_R_DrawStretchPic( xpos * cgs.screenXScale + cgs.screenXOffset + 1, ypos * cgs.screenYScale + cgs.screenYOffset + 1,
							HUD_STATUS_KEYCARDLAYOUT_XSIZE * scaleX * cgs.screenXScale,
							HUD_STATUS_KEYCARDLAYOUT_YSIZE * scaleY * cgs.screenYScale,
							0.0, (float) value / 8, 1.0, (float) value / 8 + 0.125, cgs.media.hudKeycards );
	}
	trap_R_SetColor( NULL );
	trap_R_DrawStretchPic( xpos * cgs.screenXScale + cgs.screenXOffset, ypos * cgs.screenYScale + cgs.screenYOffset,
							HUD_STATUS_KEYCARDLAYOUT_XSIZE * scaleX * cgs.screenXScale,
							HUD_STATUS_KEYCARDLAYOUT_YSIZE * scaleY * cgs.screenYScale,
							0.0, (float) value / 8, 1.0, (float) value / 8 + 0.125, cgs.media.hudKeycards );

	//HUD_STATUS_KEYCARDLAYOUT_XSIZE

}

void HUD_MegaDrawStatusHoldable( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		colorBlack;
	char		*s;
	int		value;

	if ( !hud_statusHoldable_show.integer ) {
		return;
	}

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];

	if (value == 0) {
		return;
	}

	// position lock
	if ( posLock > 0 && posLock <= HUD_POS_LOCK_TYPES ) {
	/*
	123
	456
	*/

	// text position is stripped as well
	switch( posLock ) {
		case 2:
		case 5:
			xpos = 320 * hud_aspectRatioScale.value - ( HUD_STATUS_ICON_HALF_SIZE * scaleX );
			break;

		case 3:
		case 6:
			xpos = 640 * hud_aspectRatioScale.value - ( 4 + ( HUD_STATUS_ICON_SIZE * scaleX ) ) ;
			break;

		default:
			xpos = 4;
	}

	if ( posLock > 3 ) {
		cg.ypos[posLock-1] -= HUD_STATUS_ICON_SIZE * scaleY + 4;
		ypos = cg.ypos[posLock-1];
	}
	else {
		ypos = cg.ypos[posLock-1];
		cg.ypos[posLock-1] += HUD_STATUS_ICON_SIZE * scaleY + 4;
	}

	} else {
		xpos += xoff;
		switch ( align ) {
			case 1: // center
				xpos -= HUD_STATUS_ICON_HALF_SIZE * scaleX;
				break;
			case 2: // right
				xpos -= HUD_STATUS_ICON_SIZE * scaleX;
		}
	}

	CG_RegisterItemVisuals( value );

	if ( style == 1 ) {
		colorBlack[0] = colorBlack[1] = colorBlack[2] = 0.0;
		colorBlack[3] = 1.0;
		trap_R_SetColor( colorBlack );
		trap_R_DrawStretchPic( xpos * cgs.screenXScale + cgs.screenXOffset + 1, ypos * cgs.screenYScale + cgs.screenYOffset + 1,
							HUD_STATUS_ICON_SIZE * scaleX * cgs.screenXScale,
							HUD_STATUS_ICON_SIZE * scaleY * cgs.screenYScale,
							0.0, 0.0, 1.0, 1.0,
							cg_items[ value ].icon );
	}
	trap_R_SetColor( NULL );
	trap_R_DrawStretchPic( xpos * cgs.screenXScale + cgs.screenXOffset, ypos * cgs.screenYScale + cgs.screenYOffset,
							HUD_STATUS_ICON_SIZE * scaleX * cgs.screenXScale,
							HUD_STATUS_ICON_SIZE * scaleY * cgs.screenYScale,
							0.0, 0.0, 1.0, 1.0,
							cg_items[ value ].icon );

}

void HUD_MegaDrawStatusItem( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY, int team ) {

	vec4_t		colorBlack;
	vec4_t		color;
	char		*s;

	if ( !hud_statusItem_show.integer ) {
		return;
	}

	if( cg.predictedPlayerState.powerups[PW_REDFLAG] ) {
		if (!(cg.time & 384)) {
			return;
		}
		color[1] = color[2] = 0.0;
		color[0] = color[3] = 1.0;
	} else if( cg.predictedPlayerState.powerups[PW_BLUEFLAG] ) {
		if (!(cg.time & 384)) {
			return;
		}
		color[0] = color[1] = 0.0;
		color[2] = color[3] = 1.0;
	} else {
		return;
	}

	// position lock
	if ( posLock > 0 && posLock <= HUD_POS_LOCK_TYPES ) {
	/*
	123
	456
	*/

	// text position is stripped as well
	switch( posLock ) {
		case 2:
		case 5:
			xpos = 320 * hud_aspectRatioScale.value - ( HUD_STATUS_ICON_HALF_SIZE * scaleX );
			break;

		case 3:
		case 6:
			xpos = 640 * hud_aspectRatioScale.value - ( 4 + ( HUD_STATUS_ICON_SIZE * scaleX ) ) ;
			break;

		default:
			xpos = 4;
	}

	if ( posLock > 3 ) {
		cg.ypos[posLock-1] -= HUD_STATUS_ICON_SIZE * scaleY + 4;
		ypos = cg.ypos[posLock-1];
	}
	else {
		ypos = cg.ypos[posLock-1];
		cg.ypos[posLock-1] += HUD_STATUS_ICON_SIZE * scaleY + 4;
	}

	} else {
		xpos += xoff;
		switch ( align ) {
			case 1: // center
				xpos -= HUD_STATUS_ICON_HALF_SIZE * scaleX;
				break;
			case 2: // right
				xpos -= HUD_STATUS_ICON_SIZE * scaleX;
		}
	}


	if ( style == 1 ) {
		colorBlack[0] = colorBlack[1] = colorBlack[2] = 0.0;
		colorBlack[3] = 1.0;
		trap_R_SetColor( colorBlack );
		trap_R_DrawStretchPic( xpos * cgs.screenXScale + cgs.screenXOffset + 1, ypos * cgs.screenYScale + cgs.screenYOffset + 1,
							HUD_STATUS_ICON_SIZE * scaleX * cgs.screenXScale,
							HUD_STATUS_ICON_SIZE * scaleY * cgs.screenYScale,
							0.0, 0.0, 1.0, 1.0,
							cgs.media.flagIconWhite );
	}
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( xpos * cgs.screenXScale + cgs.screenXOffset, ypos * cgs.screenYScale + cgs.screenYOffset,
							HUD_STATUS_ICON_SIZE * scaleX * cgs.screenXScale,
							HUD_STATUS_ICON_SIZE * scaleY * cgs.screenYScale,
							0.0, 0.0, 1.0, 1.0,
							cgs.media.flagIconWhite );

}

/*
=================
CG_DrawVote
=================
*/
static void HUD_MegaDrawVote( int xpos, int xoff, int ypos, int posLock, int align, int style,
									float scaleX, float scaleY ) {
	char		*s;
	int			sec;
	vec4_t		color;
	vec4_t		bgColor;
	int			bgXSize;
	int			bgYSize;
	int			bgXSizeHalf;
	int			bgYSizeHalf;
	int			bgXPos;
	int			bgYPos;
	int			flashByte;
	float		flashFloat;

	if ( !cgs.voteTime ) {
		return;
	}

	// play a talk beep whenever it is modified
	if ( cgs.voteModified ) {
		cgs.voteModified = qfalse;
		/*if ( cgs.voteFlash < (cg.time + 512 ) ) {
			cgs.voteFlash = cg.time + 256; // flash once
		}*/
		cgs.voteFlash = cg.time + 256; // flash once
		//trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		trap_S_StartLocalSound( cgs.media.voteUpdate, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}

	color[0] = color[1] = color[2] = color[3] = 1.0;

	if ( cgs.voteFlash >= cg.time ) {
		flashByte = ( cgs.voteFlash - cg.time ) & 255;
		bgColor[0] = bgColor[1] = bgColor[2] = ((float)flashByte / 256);
	} else {
		bgColor[0] = bgColor[1] = bgColor[2] = 0;
	}
	bgColor[3] = 0.5;

	s = va("VOTE(%i):%s Yes:%i No:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo ); // vote string


	// draw bg

	bgXSize = UI_ProportionalColorStringWidth( s ) * scaleX;
	bgXSizeHalf = bgXSize >> 1;
	bgYSize = 16 * scaleY;
	// position lock
	/*
	123
	456
	*/
	if ( posLock > 0 && posLock <= HUD_POS_LOCK_TYPES ) {

		// text position is stripped as well
		switch( posLock ) {
			case 2:
			case 5:
				bgXPos = 320 * hud_aspectRatioScale.value - bgXSizeHalf;
				break;

			case 3:
			case 6:
				bgXPos = 640 * hud_aspectRatioScale.value - ( 4 + ( bgXSize ) ) ;
				break;

			default:
				bgXPos = 4;
		}

		if ( posLock > 3 ) {
			bgYPos = cg.ypos[posLock-1] - (bgYSize + 4);
		} else {
			bgYPos = cg.ypos[posLock-1];
		}

	} else {
		bgYPos = ypos;
		bgXPos = xpos + xoff;
		switch ( align ) {
			case 1: // center
				bgXPos -= bgXSizeHalf;
				break;
			case 2: // right
				bgXPos -= bgXSize;
		}
	}
	CG_FillRect( bgXPos - 2, bgYPos - 2, (UI_ProportionalColorStringWidth( s ) * scaleX) + 4, bgYSize + 4, bgColor );


	// draw vote string

	style = HUD_FuncStyleSet ( align, style );
	//CG_DrawSmallString( 0, 58, s, 1.0F );
	HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 16, style, scaleX, scaleY, color, qfalse );
}


/*
=================
HUD_MegaRuleSet
=================
*/

void HUD_MegaRuleSet_BooleanStats( int x, int y, float scale, int result, vec4_t color ) {
	char		*s;
	if ( result > 0 ) {
		s = "Yes";
	} else {
		s = "No";
	}
	UI_DrawCustomProportionalString( x, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
}

void HUD_MegaRuleSet_FloatValueStats( int x, int y, float scale, float result, const char *afterString, vec4_t color ) {
	char		*s;
	char		*sf;
	int			r;
	float		rf;

	r = result;
	if ( (result >= 0.25f && result < 1.0f) || (result >= 1.25f) ) {
		rf = result - floor(result);
		if (rf >= .75)
			sf = "\xBE";
		else if (rf >= .50)
			sf = "\xBD";
		else if (rf >= .25)
			sf = "\xBC";
		else
			sf = "";
		s = va("%i%s %ss", r, sf, afterString );
	} else if ( result >= 1.0f && result < 1.25f ) {
		s = va("1 %s", afterString );
	} else {
		s = "No";
	}
	UI_DrawCustomProportionalString( x, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
}

void HUD_MegaRuleSet_ValueStats( int x, int y, float scale, int result, const char *afterString, vec4_t color ) {
	char		*s;
	if ( result > 1 ) {
		s = va("%i %ss", result, afterString );
	} else if ( result == 1 ) {
		s = va("1 %s", afterString );
	} else {
		s = "No";
	}
	UI_DrawCustomProportionalString( x, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
}

static void HUD_MegaRuleSet( int xpos, int ypos, float scale ) {
	char		*s;
	int			sec;
	vec4_t		color;
	vec4_t		colorAmber;
	vec4_t		colorLAmber;
	vec4_t		colorLGray;
	vec4_t		colorBG;

	int			y;
	int			x, xend, xst, xtit, xmde;

	color[0] = color[1] = color[2] = color[3] = 1.0;
	colorAmber[0] = colorAmber[3] = 1.0;
	colorAmber[1] = 0.5;
	colorAmber[2] = 0.0;
	colorLAmber[0] = colorLAmber[3] = 1.0;
	colorLAmber[1] = 0.8;
	colorLAmber[2] = 0.0;
	colorLGray[0] = colorLGray[1] = colorLGray[2] = 0.75;
	colorLGray[3] = 1.0;
	colorBG[0] = colorBG[1] = colorBG[2] = 0.0;
	colorBG[3] = 0.3984375;

	y = ypos;
	x = xpos - (128 * scale);
	xst = xpos - (126 * scale);
	xtit = xpos - (122 * scale);
	xend = xpos + (126 * scale);

	sec = cg.warmup;
	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
		if ( sec <= 0 || cgs.voteTime ) {
			return;
		}
	} else {
		if ( !hud_showRulesetInFreeFloat.integer ) {
			return;
		}
	}

	//CG_FillRect( x - 2, y - 2, (256 + 4) * scale, (152 + 4) * scale, colorBG );

	//CG_FillRect( x - 2, y - 2, (256 + 4) * scale, (256 + 4) * scale, colorBG ); // resized
	trap_R_SetColor( NULL );
	CG_DrawPic ( x * scale, (y - 2) * scale, 256 * scale, 256 * scale, cgs.media.rulesBG256 );
	//trap_R_DrawStretchPic( x * scale, (y - 2) * scale, 256 * scale, 256 * scale, 0, 0, 1, 1, cgs.media.rulesBG256 );

	s = "RULE SET  ";
	xmde = UI_ReturnStringWidth (s, qfalse) * 0.73;
	UI_DrawCustomProportionalString( xtit, y + 1, s, 0, 0.73 * scale, colorBlack, qfalse ); // 0.75 -> 0.73

	switch (cgs.ruleSet) {
		case 1:
			s = "(Normal)";
			break;
		case 2:
			s = "(Hardcore)";
			break;
		case 3:
			s = "(Nightmare!)";
			break;
		case 4:
			s = "(Arena)";
			break;
		case 5:
			s = "(Rockets)";
			break;
		default:
			s = "(Custom)";
			break;
	}
	UI_DrawCustomProportionalString( xtit + xmde, y + 2, s, 0, 0.64 * scale, colorBlack, qfalse ); // 0.75 -> 0.73
	y += 16 * scale;

	s = "GAME TYPE:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	switch ( cgs.gametype ) {
	case GT_FFA:
		s = "Deathmatch";
		break;
	case GT_TOURNAMENT:
		s = "Tournament";
		break;
	case GT_TEAM:
		s = "Team Deathmatch";
		break;
	case GT_CTF:
		s = "Capture The Flag";
		break;
	default:
		s = "Custom";
		break;
	}
	UI_DrawCustomProportionalString( xend, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
	y += 8 * scale;

	s = "MATCH MODE:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	switch ( cgs.matchMode ) {
	case MM_PICKUP_ONCE:
		s = "Weapon-stay";
		break;
	case MM_PICKUP_ALWAYS:
		s = "Weapon always pickup";
		break;
	case MM_PICKUP_ALWAYS_NOAMMO:
		s = "No ammo";
		break;
	case MM_ALLWEAPONS_MAXAMMO:
		s = "Most weapons, max ammo";
		break;
	case MM_ALLWEAPONS:
		s = "All weapons";
		break;
	case MM_ROCKET_MANIAX:
		s = "Rocket-maniax";
		break;
	default:
		s = "WTF???";
		break;
	}
	UI_DrawCustomProportionalString( xend, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
	y += 8 * scale;

	if (cgs.timelimit) {
		s = "TIME LIMIT:";
		UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		//HUD_MegaRuleSet_ValueStats( xend, y, scale, cgs.fullTimelimit, "minute", color); // TODO: add seconds from timelimit
		HUD_MegaRuleSet_FloatValueStats( xend, y, scale, cgs.fullTimelimit, "minute", color);
		y += 8 * scale;
		if (cgs.overtime) {
			s = "OVERTIME:";
			UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
			if (cgs.overtime > 1) {
				s = va("Extension of %i minutes", cgs.overtime);
			} else if (cgs.overtime == 1) {
				s = va("Extension of %i minute", cgs.overtime);
			} else {
				s = va("Sudden death", cgs.overtime);
			}
			UI_DrawCustomProportionalString( xend, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
			y += 8 * scale;
		}
	}

	s = "ROUND FORMAT:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	if ( cgs.roundBasedMatches ) {
			s = "Double rounds";
	} else {
			s = "Single round";
	}
	UI_DrawCustomProportionalString( xend, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
	y += 8 * scale;

	if (cgs.scorelimit) {
		s = "SCORE LIMIT:";
		UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		HUD_MegaRuleSet_ValueStats( xend, y, scale, cgs.scorelimit, "point", color);
		y += 8 * scale;
	}

	if (cgs.mercylimit) {
		s = "MERCY LIMIT:";
		UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		HUD_MegaRuleSet_ValueStats( xend, y, scale, cgs.mercylimit, "point", color);
		y += 8 * scale;
	}

	s = "PRO-MODE:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	if (cgs.physicsMode == 1) {
		s = "Free-select physics";
	} else {
		s = "Restricted physics";
	}
	UI_DrawCustomProportionalString( xend, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
	y += 12 * scale;

//	y += 4 * scale;

	s = "WEAPON RESPAWN:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	if (cgs.weaponRespawn) {
		HUD_MegaRuleSet_ValueStats( xend, y, scale, cgs.weaponRespawn, "second", color);
	} else {
		s = "No (weapon-stay)";
		UI_DrawCustomProportionalString( xend, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
	}
	y += 8 * scale;

	s = "KEYCARD RESPAWN:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	if (cgs.keycardRespawn) {
		HUD_MegaRuleSet_ValueStats( xend, y, scale, cgs.keycardRespawn, "second", color);
	} else {
		s = "No (keycard-stay)";
		UI_DrawCustomProportionalString( xend, y, s, UI_DROPSHADOW | UI_RIGHT, 0.5 * scale, color, qfalse );
	}
	y += 8 * scale;

	s = "KEYCARD DROPABLE:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.keycardDropable, color);
	y += 8 * scale;

	s = "FORCED PLAYER RESPAWN:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_ValueStats( xend, y, scale, cgs.forceRespawn, "second", color);
	y += 8 * scale;

	s = "SELF DAMAGE:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.selfDamage, color);
	y += 8 * scale;

	if ( cgs.gametype >= GT_TEAM ) {

	s = "FRIENDLY FIRE:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.friendlyFire, color);
	y += 8 * scale;

	s = "TEAMMATE'S LOCATION ON MINIMAP:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.teamLocOverlay, color);
	y += 8 * scale;

		if ( cgs.gametype == GT_TEAM ) {
			s = "TEAM SCORE BALANCE:";
			UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
			HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.scoreBalance, color);
			y += 8 * scale;
		}

	}

	s = "HIT SOUND:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.hitSound, color);
	y += 8 * scale;

	s = "TOTALLY RANDOM SPAWNS:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.randomSpawn, color);
	y += 8 * scale;

	s = "POWER UPS:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.powerUps, color);
	y += 8 * scale;

	s = "ARMOR:";
	UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
	HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.armor, color);
	y += 8 * scale;

	if ( cgs.ruleSet < 1 || cgs.ruleSet > 2 ) {
		y += 4 * scale;

		s = "QUAD MODE:";
		UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.quadMode, color);
		y += 8 * scale;

		s = "DOUBLE AMMO:";
		UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.doubleAmmo, color);
		y += 8 * scale;

	}

	if (cgs.matchMode == MM_ALLWEAPONS) {
		s = "NO ARENA GRENADES:";
		UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.noArenaGrenades, color);
		y += 8 * scale;

		s = "NO ARENA LIGHTNING GUN:";
		UI_DrawCustomProportionalString( xst, y, s, UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		HUD_MegaRuleSet_BooleanStats( xend, y, scale, cgs.noArenaLightningGun, color);
		y += 8 * scale;
	}

	//void HUD_MegaRuleSet_ValueStats( int x, int y, float scale, int result, const char *afterString, vec4_t color )
	//HUD_FuncPosLock( s, posLock, xpos, xoff, ypos, 64, style, scaleX, scaleY, color, qfalse );

}

/*
=================
HUD_MegaObjective
=================
*/

static void HUD_MegaObjective( int xpos, int ypos, float scale, int team ) {
	char		*s;
	int			sec;
	vec4_t		color;
	vec4_t		colorAmber;
	vec4_t		colorLAmber;
	vec4_t		colorLGray;
	vec4_t		colorBG;

	int			y;
	int			x, xInd, xst, xtit;

	color[0] = color[1] = color[2] = color[3] = 1.0;
	colorAmber[0] = colorAmber[3] = 1.0;
	colorAmber[1] = 0.5;
	colorAmber[2] = 0.0;
	colorLAmber[0] = colorLAmber[3] = 1.0;
	colorLAmber[1] = 0.8;
	colorLAmber[2] = 0.0;
	colorLGray[0] = colorLGray[1] = colorLGray[2] = 0.75;
	colorLGray[3] = 1.0;
	colorBG[0] = colorBG[1] = colorBG[2] = 0.0;
	colorBG[3] = 0.3984375;

	y = ypos;
	x = xpos - (128 * scale);
	xst = xpos - (126 * scale);
	xtit = xpos - (122 * scale);
	xInd = x + ( 16 * scale );

	sec = cg.warmup;
	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
		if ( sec <= 0 || cgs.voteTime ) {
			return;
		}
	} else {
		if ( !hud_showRulesetInFreeFloat.integer ) {
			return;
		}
	}

	//CG_FillRect( x - 2, y - 2, (256 + 4) * scale, (60 + 4) * scale, colorBG );
	trap_R_SetColor( NULL );
	CG_DrawPic ( x * scale, (y - 2) * scale, 256 * scale, 64 * scale, cgs.media.rulesBG064 );

	//UI_DrawCustomProportionalString( x, y, "OBJECTIVE:", UI_DROPSHADOW, 0.75 * scale, colorAmber, qfalse );
	UI_DrawCustomProportionalString( xtit, y + 1, "OBJECTIVE", 0, 0.73 * scale, colorBlack, qfalse ); // 0.75 -> 0.73
	y += 16 * scale;

	switch ( cgs.gametype ) {
	case GT_FFA:
		UI_DrawCustomProportionalString( xst, y,
			"FREE FOR ALL (DEATH MATCH)",
			UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		y += 12 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"1. Shoot shit",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"2. Read No.1",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"3. Grab power ups, and shoot more",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"shit",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );

		break;
	case GT_TOURNAMENT:
		UI_DrawCustomProportionalString( xst, y,
			"TOURNAMENT",
			UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		y += 12 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"1. Grab the best weapons",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"2. Kill your opponent",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"3. Win the match",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );

		break;
	case GT_TEAM:
		UI_DrawCustomProportionalString( xst, y,
			"TEAM DEATH MATCH",
			UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		y += 12 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"1. Kill the enemy",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"2. Don't shoot your teammates",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"3. Grab power ups, and keep killing",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"your enemies",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );

		break;
	case GT_CTF:
		UI_DrawCustomProportionalString( xst, y,
			"CAPTURE THE FLAG",
			UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		y += 12 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"1. Grab the enemy's flag",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"2. Capture it, by bringing it to",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"your team's base",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );
		y += 8 * scale;

		UI_DrawCustomProportionalString( xInd, y,
			"3. Prevent the enemy from scoring",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );

		break;

	case GT_AA1:

		UI_DrawCustomProportionalString( xst, y,
		"ALL AGAINST ONE",
		UI_DROPSHADOW, 0.5 * scale, colorLAmber, qfalse );
		y += 12 * scale;

		if ( team == TEAM_BLUE ) {

			UI_DrawCustomProportionalString( xInd, y,
				"1. Survive",
				UI_DROPSHADOW, 0.5 * scale, color, qfalse );
			y += 8 * scale;

			UI_DrawCustomProportionalString( xInd, y,
				"2. ?",
				UI_DROPSHADOW, 0.5 * scale, color, qfalse );
			y += 8 * scale;

			UI_DrawCustomProportionalString( xInd, y,
				"3. Profit!",
				UI_DROPSHADOW, 0.5 * scale, color, qfalse );

		} else {

			UI_DrawCustomProportionalString( xInd, y,
				"1. Find the target player",
				UI_DROPSHADOW, 0.5 * scale, color, qfalse );
			y += 8 * scale;

			UI_DrawCustomProportionalString( xInd, y,
				"2. Attack the target player",
				UI_DROPSHADOW, 0.5 * scale, color, qfalse );
			y += 8 * scale;

			UI_DrawCustomProportionalString( xInd, y,
				"3. Kill for a victory",
				UI_DROPSHADOW, 0.5 * scale, color, qfalse );

		}

		break;
	default:
		UI_DrawCustomProportionalString( xInd, y,
			"Win the match... Yeah, do that",
			UI_DROPSHADOW, 0.5 * scale, color, qfalse );

		break;
	}


}


/*
===========================================================================
HUD_DrawMetalHUD
===========================================================================
*/

#define	HUD_NUMCHAR_SPACE		14

void HUD_DrawMetalHUD ( int posRight ) {

	char		*s;
	centity_t	*cent;
	playerState_t	*ps;
	clientInfo_t	*ci;

	int			posCenter;
	int			x, y, w;
	int			i, inv;
	qhandle_t	hShader;
	int			flash;

	int			currentWeapon;
	int			ammo;
	int			value;
	int			bits;
	int			max;

/*
---------------------------------------------------------------------------
*/

	posCenter = posRight / 2;

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;
	ci = &cgs.clientinfo[cg.snap->ps.clientNum];

	// health
	if ( value > HUD_HEALTH_OVER ) {
		UI_DrawNumCharInteger( posCenter - 24, 460, 16, value, 0, qfalse );
	} else
	if ( ( value >= HUD_HEALTH_LOW ) || (value > HUD_HEALTH_VERY_LOW && !(cg.time & 256)) || (value < HUD_HEALTH_VERY_LOW + 1 && !(cg.time & 128) ) ){
		UI_DrawNumCharInteger( posCenter - 24, 460, 16, value, 1, qfalse );
	}

	// armor
	UI_DrawNumCharInteger( posCenter + 24 + HUD_NUMCHAR_SPACE * 2, 460, 16, ps->stats[STAT_ARMOR], 1, qfalse ); // test

	// weapon status
	currentWeapon = cent->currentState.weapon;
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];

	x = 4;
	y = 128;
	for ( i = WP_BLASTER ; i <= WP_FLAMETHROWER ; i++ ) {
		if ( bits & ( 1 << i ) ) {

			ammo = cg.snap->ps.ammo[ammoGroup[i].ammo];

			switch(i) {
				case WP_ROCKET_LAUNCHER:
				case WP_GRENADE_LAUNCHER:
				case WP_SHOTGUN:
				case WP_SUPER_SHOTGUN:
					max = 5;
					break;
				default:
					max = 25;
					break;
			}

			UI_DrawNumCharInteger( x, y, 8, i, 1, qtrue );
		}
		y += 20;

	}


}


/*
===================
HUD_DrawPickupItem

TODO: clean up this mess
===================
*/

#define	MDHUDPickUpIconSize			32
#define	MDHUDPickUpHeight			MDHUDPickUpIconSize / 2
#define	MDHUDPickUpTextOffset		MDHUDPickUpIconSize / 4

static void HUD_DrawPickupInfo( int xpos, int ypos, int align, int style, float scale ) {
	int			value;
	int			w, h;
	int			i, len;
	int			tempYPos;
	qboolean	dropShadow;
	int			preScale;
	int			iconSize;
	int			yPosOffset;
	vec4_t		hcolorBlk;
	vec4_t		hcolor;
	int			itemLength, multiLength;
	int			iconPos, itemPos, multiPos;
	char		*multiText;

	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	if ( hud_pickUpInfo_time.integer > 0 && ( cg.time > (cg.itemPickupTime + (hud_pickUpInfo_time.integer * 1000)) ) ) {
		return;
	}

	style = HUD_FuncStyleSet ( align, style );
	style &= (~UI_FORMATMASK); // text alignment is stripped

	preScale = scale * MDHUDPickUpHeight;
	iconSize = MDHUDPickUpIconSize * scale;
	yPosOffset = MDHUDPickUpTextOffset * scale;
	align = align&3;

	hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0f;

	hcolorBlk[0] = hcolorBlk[1] = hcolorBlk[2] = 0.0f;
	hcolorBlk[3] = 1.0f;

	value = cg.itemPickup;

	if ( value ) {

		itemLength = UI_ProportionalColorStringWidth( bg_itemlist[ value ].pickup_name ) * scale;
		if ( cg.itemPickupMultiCount ) {
			multiText = va( " x%i", cg.itemPickupMultiCount + 1 ); // TODO: use the 'times' symbol instead of 'x'
			multiLength = UI_ProportionalColorStringWidth( multiText ) * scale;
		} else {
			multiLength = 0;
		}

		switch(align) {
			case 1:
				// align center
				iconPos = xpos - (iconSize + itemLength + multiLength) / 2;
				break;
			case 2:
				// align right
				iconPos = xpos - (iconSize + itemLength + multiLength);
				break;
			default:
				// align left
				iconPos = xpos;
				break;
		}
		itemPos = iconPos + iconSize;
		multiPos = itemPos + itemLength;

		if (style & UI_DROPSHADOW) {
			trap_R_SetColor( hcolorBlk );
			trap_R_DrawStretchPic( (float)iconPos * cgs.screenXScale + cgs.screenXOffset + 1, (float)ypos * cgs.screenYScale + cgs.screenYOffset + 1,
						(float)iconSize * cgs.screenXScale, (float)iconSize * cgs.screenYScale,
						0, 0, 1, 1, cg_items[ value ].icon );
			trap_R_SetColor( hcolor ); // reset texture color tint
		}
		trap_R_DrawStretchPic( (float)iconPos * cgs.screenXScale + cgs.screenXOffset, (float)ypos * cgs.screenYScale + cgs.screenYOffset,
					(float)iconSize * cgs.screenXScale, (float)iconSize * cgs.screenYScale,
					0, 0, 1, 1, cg_items[ value ].icon );

		UI_DrawCustomProportionalString( itemPos, ypos + yPosOffset,
			bg_itemlist[ value ].pickup_name,
			UI_DROPSHADOW, scale, hcolor, qfalse );

		if ( multiLength ) {
			UI_DrawCustomProportionalString( multiPos, ypos + yPosOffset,
				multiText,
				UI_DROPSHADOW, scale, hcolor, qfalse );
		}
	}

	/*y -= ICON_SIZE;

	value = cg.itemPickup;
	if ( value ) {
		fadeColor = CG_FadeColor( cg.itemPickupTime, 3000 );
		if ( fadeColor ) {
			CG_RegisterItemVisuals( value );
			trap_R_SetColor( fadeColor );
			CG_DrawPic( 8, y, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
			CG_DrawBigString( ICON_SIZE + 16, y + (ICON_SIZE/2 - BIGCHAR_HEIGHT/2), bg_itemlist[ value ].pickup_name, fadeColor[0] );
			trap_R_SetColor( NULL );
		}
	}

	return y;*/
}



/*
===========================================================================
HUD_Draw2D
===========================================================================
*/
























