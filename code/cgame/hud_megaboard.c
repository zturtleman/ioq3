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

#include "cg_local.h"




/*
=================
HUD_MegaDrawIntermissionInfo
=================
*/
#define	MDHUDIntermissionHeight			16

static void HUD_MegaDrawIntermissionInfo( int xpos, int ypos, int style, float scale ) {

	int		w, h;
	int		i, pos, len;
	int		tempYPos;
	qboolean	dropShadow;
	int		preScale;
	vec4_t	hcolor;

	preScale = scale * MDHUDIntermissionHeight;

	trap_R_SetColor( NULL );

	hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
	hcolor[3] = 1.0f;

	pos = HUD_BOARD_TEXT_HEIGHT - (cgs.hudBoardInfoPos % HUD_BOARD_TEXT_HEIGHT);
	for (i = 0; i < HUD_BOARD_TEXT_HEIGHT; i++, pos++) {
		UI_DrawCustomProportionalString( xpos, ypos + ( pos % HUD_BOARD_TEXT_HEIGHT ) * preScale,
				cgs.hudBoardInfoMsgs[i % HUD_BOARD_TEXT_HEIGHT], style, scale, hcolor, qfalse );
	}

}


/*
=================
HUD_MegaDrawOutlinedBox
=================
*/

void HUD_MegaDrawOutlinedBox( int x, int y, int width, int height, const float *color ) {

	vec4_t		colorShadow;

	//trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader );

	// filled color
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( (float)(x) * cgs.screenXScale + 1 + cgs.screenXOffset, (float)(y) * cgs.screenYScale + 1 + cgs.screenYOffset,
					(float)(width) * cgs.screenXScale - 1, (float)(height) * cgs.screenYScale - 1,
					0, 0, 0, 0, cgs.media.whiteShader );

	// outline
	trap_R_SetColor( NULL );
	// top line
	trap_R_DrawStretchPic( (float)(x) * cgs.screenXScale + cgs.screenXOffset, (float)(y) * cgs.screenYScale + cgs.screenYOffset,
					(float)(width) * cgs.screenXScale, (float)1,
					0, 0, 0, 0, cgs.media.whiteShader );
	// bottom line
	trap_R_DrawStretchPic( (float)(x) * cgs.screenXScale + cgs.screenXOffset, (float)(y+height) * cgs.screenYScale + cgs.screenYOffset,
					(float)(width) * cgs.screenXScale, (float)1,
					0, 0, 0, 0, cgs.media.whiteShader );
	// left line
	trap_R_DrawStretchPic( (float)(x) * cgs.screenXScale + cgs.screenXOffset, (float)(y) * cgs.screenYScale + cgs.screenYOffset,
					(float)1, (float)(height) * cgs.screenYScale,
					0, 0, 0, 0, cgs.media.whiteShader );
	// right line
	trap_R_DrawStretchPic( (float)(x+width) * cgs.screenXScale + cgs.screenXOffset, (float)(y) * cgs.screenYScale + cgs.screenYOffset,
					(float)1, (float)(height) * cgs.screenYScale + 1,
					0, 0, 0, 0, cgs.media.whiteShader );

	colorShadow[0] = colorShadow[1] = colorShadow[2] = 0.0;
	colorShadow[3] = 1.0;

	// drop shadow
	trap_R_SetColor( colorShadow );
	// bottom line
	trap_R_DrawStretchPic( (float)(x) * cgs.screenXScale + 1 + cgs.screenXOffset,
					(float)(y+height) * cgs.screenYScale + 1 + cgs.screenYOffset,
					(float)(width) * cgs.screenXScale, (float)1,
					0, 0, 0, 0, cgs.media.whiteShader );
	// right line
	trap_R_DrawStretchPic( (float)(x+width) * cgs.screenXScale + 1 + cgs.screenXOffset,
					(float)(y) * cgs.screenYScale + 1 + cgs.screenYOffset,
					(float)1, (float)(height) * cgs.screenYScale + 1,
					0, 0, 0, 0, cgs.media.whiteShader );

}

/*
=================
HUD_MegaDrawOutlinedBoxFloat
=================
*/

void HUD_MegaDrawOutlinedBoxFloat( float x, float y, float width, float height, const float *color ) {

	vec4_t		colorShadow;

	//trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader );

	// filled color
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( x + 1, y + 1, width - 1, height - 1,
					0, 0, 0, 0, cgs.media.whiteShader );

	// outline
	trap_R_SetColor( NULL );
	// top line
	trap_R_DrawStretchPic( x, y, width, 1,
					0, 0, 0, 0, cgs.media.whiteShader );
	// bottom line
	trap_R_DrawStretchPic( x, y+height, width, 1,
					0, 0, 0, 0, cgs.media.whiteShader );
	// left line
	trap_R_DrawStretchPic( x, y, 1, height,
					0, 0, 0, 0, cgs.media.whiteShader );
	// right line
	trap_R_DrawStretchPic( x+width, y, 1, height + 1,
					0, 0, 0, 0, cgs.media.whiteShader );

	colorShadow[0] = colorShadow[1] = colorShadow[2] = 0.0;
	colorShadow[3] = 1.0;

	// drop shadow
	trap_R_SetColor( colorShadow );
	// bottom line
	trap_R_DrawStretchPic( x+1, y+height+1, width, 1,
					0, 0, 0, 0, cgs.media.whiteShader );
	// right line
	trap_R_DrawStretchPic( x+width+1, y+1, 1, height + 1,
					0, 0, 0, 0, cgs.media.whiteShader );

}

/*
=================
HUD_MegaDrawPingBar
=================
*/

/*
reference
---------
#define SERVER_PING_GREEN	50	// Great ping
#define SERVER_PING_YELLOW	90	// Good ping
#define SERVER_PING_AMBER	130	// OK ping
#define SERVER_PING_RED		999	// Red ping
*/

void HUD_MegaDrawPingBar( int x, int y, int ping, int type ) {
	vec4_t		color;
	int		i;

	if (type == 1) {
		if ( ping < SERVER_PING_GREEN ) {
			color[0] = color[2] = 0.0;
			color[1] = color[3] = 1.0;
			i = 4;
		} else
		if ( ping < SERVER_PING_YELLOW ) {
			color[2] = 0.0;
			color[0] = color[1] = color[3] = 1.0;
			i = 3;
		} else
		if ( ping < SERVER_PING_AMBER ) {
			color[1] = 0.5;
			color[2] = 0.0;
			color[0] = color[3] = 1.0;
			i = 2;
		} else
		if ( ping < SERVER_PING_RED ) {
			color[1] = color[2] = 0.0;
			color[0] = color[3] = 1.0;
			i = 1;
		} else {
			return; // don't display any bars, if ping is 999 or beyone
		}

		x -= (4 - i) * 6;
		y -= i - 3;

		for ( ; i ; i--) {
			x -= 6;
			y += 1;
			HUD_MegaDrawOutlinedBox ( x, y, 4, 2 + i, color );
		}

	} else {

		if ( ping < SERVER_PING_GREEN ) {
			i = 0;
		} else
		if ( ping < SERVER_PING_YELLOW ) {
			i = 1;
		} else
		if ( ping < SERVER_PING_AMBER ) {
			i = 2;
		} else
		if ( ping < SERVER_PING_RED ) {
			i = 3;
		} else {
			i = 4;
		}

		color[0] = color[1] = color[2] = 0.0;
		color[3] = 1.0;

		x -= 22;
		y--;

		trap_R_SetColor( color );
		trap_R_DrawStretchPic( (float)x * cgs.screenXScale + 1 + cgs.screenXOffset,
						(float)y * cgs.screenYScale + 1 + cgs.screenYOffset,
						(float)(22) * cgs.screenXScale, (float)(8) * cgs.screenYScale,
						0.0625, (float)i * 0.125,
						1.0, (float)(i + 1) * 0.125, cgs.media.hudPing );
		trap_R_SetColor( NULL );
		trap_R_DrawStretchPic( (float)x * cgs.screenXScale + cgs.screenXOffset,
						(float)y * cgs.screenYScale + cgs.screenYOffset,
						(float)(22) * cgs.screenXScale, (float)(8) * cgs.screenYScale,
						0.0625, (float)i * 0.125,
						1.0, (float)(i + 1) * 0.125, cgs.media.hudPing );
	}

}

/*
=================
HUD_MegaDrawClientScore
=================
*/
static void HUD_MegaDrawClientScore( int x, int y, score_t *score, int scRank, vec4_t colorWht, vec4_t colorYel, qboolean filterColor ) {
	char		string[64]; // was 1024
	char		string2[64]; // was 1024
	int			lineXPos;
	clientInfo_t	*ci;
	int			ping;

	int			botSkill;
	//char		name[MAX_NAME_LENGTH];
	int			nameWidth;
	int			nameWidth2;
	int			nameWidthCombine;
	float		nameWidthAdj;

	static float	teamColor[3][4] = {
		 { 0, 0, 0, 1.0f },	//black
		 { 1.0f, 0, 0, 1.0f },	//red
		 { 0, 0, 1.0f, 1.0f }	//blue
	};

	/*int		x2, y2;*/

	// is this an out of range client?
	if ( score->client < 0 || score->client >= cgs.maxclients ) {
		/*Com_Printf( "Bad score->client: %i\n", score->client );*/
		return;
	}

	ci = &cgs.clientinfo[score->client];
	botSkill = ( ci->botSkill > 0 && ci->botSkill <= 99 );

	if ( score->client == cg.snap->ps.clientNum ) {
		//CG_DrawPic( x+12, y+1, 320-8, 24, cgs.media.hudHighlight );
		CG_DrawPic( x + 2, y-2, 320-9, 24, cgs.media.hudHighlight );
	}

	// draw flag status
	if (cg.time&384) {
		if ( ci->powerups & ( 1 << PW_REDFLAG ) ) {
			// red
			trap_R_SetColor( teamColor[1] );
			CG_DrawPic( x+12, y+7, 16, 16, cgs.media.flagIconWhite );
			trap_R_SetColor( NULL );
		} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) ) {
			// blue
			trap_R_SetColor( teamColor[2] );
			CG_DrawPic( x+12, y+7, 16, 16, cgs.media.flagIconWhite );
			trap_R_SetColor( NULL );
		}
	}

	Com_sprintf(string, sizeof(string), "%s", ci->name);
	Com_sprintf(string2, sizeof(string2), "%s", ci->clan);
	nameWidth = UI_ReturnStringWidth (string, qfalse) * 0.63;
	nameWidth2 = UI_ReturnStringWidth (string2, qfalse) * 0.4;
	nameWidthCombine = nameWidth + nameWidth2;

	if (nameWidthCombine > 240) {
		nameWidthAdj = 240/(float)nameWidthCombine;
		lineXPos = x + 56;
		HUD_FuncPosLock( string2, 0, lineXPos, 0, y, 0, UI_DROPSHADOW, 0.4 * nameWidthAdj, 0.4, colorWht, filterColor );
		lineXPos += nameWidth2 * nameWidthAdj;
		HUD_FuncPosLock( string, 0, lineXPos, 0, y, 0, UI_DROPSHADOW, 0.63 * nameWidthAdj, 0.63, colorWht, filterColor );
	} else {
		lineXPos = x + 56;
		HUD_FuncPosLock( string2, 0, lineXPos, 0, y, 0, UI_DROPSHADOW, 0.4, 0.4, colorWht, filterColor );
		lineXPos += nameWidth2;
		HUD_FuncPosLock( string, 0, lineXPos, 0, y, 0, UI_DROPSHADOW, 0.63, 0.63, colorWht, filterColor );
	}
	//UI_DrawCustomProportionalString( x + 56, y, string, UI_DROPSHADOW, 0.63, colorWht, filterColor );

	// draw player port
	CG_DrawPic( x + 39, y + 2, 16, 16, cgs.media.hudPlayerPort[score->client] );
	/*trap_R_DrawStretchPic( x + 34, y - 2, 16, 24,
					0.0f, 0.0f, 1.0f, 0.75f, cgs.media.hudDefaultPort );*/

	// draw bot icon
	if ( botSkill ) {
		// if this is a cpu bot, indicate that this is so
//		CG_DrawCustomString( x-1, y+3, 6, 8, "^3BOT", fade );// mmp - need to make new bot icons... if possible  :(
		UI_DrawCustomProportionalString( x + 20, y, "BOT", UI_DROPSHADOW | UI_CENTER, 0.50, colorYel, qfalse );
	} else if ( ci->handicap > 0 ) {
		// draw handicap if any is set
		Com_sprintf( string, sizeof( string ), S_COLOR_GREEN"%2i%%", ci->handicap );
//		CG_DrawCustomString( x, y+3, 8, 8, string, fade );
		UI_DrawCustomProportionalString( x + 20, y, string, UI_DROPSHADOW | UI_CENTER, 0.50, colorWht, qfalse );
	}

	// Show status
	if (/*(cgs.flags & FL_READYREQUIRED) && */cg.warmup && !botSkill && score->client < 64) {
		if( cg.readyMask[score->client >> 4] & ( 1 << ( score->client & 15 ) ) )
			CG_DrawPic( x+12, y+3, 16, 16, cgs.media.scoreIconOK );
		else
			CG_DrawPic( x+12, y+3, 16, 16, cgs.media.scoreIconNo );
	}
	else {
		if ( scRank > 0 && !cg.warmup ) {
			// draw the player's rank
			switch (scRank) {
				case 1:
					HUD_FuncPosLock( "1st", 0, x + 20, 0, y + 9, 0,
							UI_DROPSHADOW | UI_CENTER, 0.63, 0.63, colorWht, qfalse );
					/*UI_DrawCustomProportionalString( x + 20, y+8, va("%ist", scRank),
							UI_DROPSHADOW | UI_CENTER, 0.75, colorWht, qfalse );*/
					break;
				case 2:
					HUD_FuncPosLock( "2nd", 0, x + 20, 0, y + 9, 0,
							UI_DROPSHADOW | UI_CENTER, 0.63, 0.63, colorWht, qfalse );
					break;
				case 3:
					HUD_FuncPosLock( "3rd", 0, x + 20, 0, y + 9, 0,
							UI_DROPSHADOW | UI_CENTER, 0.63, 0.63, colorWht, qfalse );
					break;
				default:
					HUD_FuncPosLock( va("%ith", scRank), 0, x + 20, 0, y + 9, 0,
							UI_DROPSHADOW | UI_CENTER, 0.63, 0.63, colorWht, qfalse );
					break;
			}
		}
		if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << score->client ) ) {
			CG_DrawPic( x+12, y+3, 16, 16, cgs.media.scoreIconOK );
		}
	}

	/*
	---------------
	draw second row
	---------------
	*/
	y += 11;

	ping = score->ping;
	if ( ping != -1 ) {
		UI_DrawCustomProportionalString( x + 56, y, "Score:", UI_DROPSHADOW, 0.50, colorYel, qfalse );
		UI_DrawCustomProportionalString( x + 137, y, va("%i",score->score), UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qtrue );
		UI_DrawCustomProportionalString( x + 155, y, "Ping:", UI_DROPSHADOW, 0.50, colorYel, qfalse );
		if ( hud_scoreboard_pingType.integer == 0 ) {
			HUD_MegaDrawPingBar ( x + 217, y, score->ping, hud_scoreboard_barType.integer );
		} else {
			if ( ci->botSkill <= 0 )
				UI_DrawCustomProportionalString( x + 217, y, va("%i",score->ping), UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qfalse );
			else
				UI_DrawCustomProportionalString( x + 217, y, "666", UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qfalse );
		}
		UI_DrawCustomProportionalString( x + 235, y, "Time:", UI_DROPSHADOW, 0.50, colorYel, qfalse );
		if ( score->time < 99 )
			UI_DrawCustomProportionalString( x + 297, y, va("%im",score->time), UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qfalse );
		else
			UI_DrawCustomProportionalString( x + 297, y, "99m", UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qfalse );
	} else {
		if (cg.time & 256)
			UI_DrawCustomProportionalString( x + 297, y, "Connecting...", UI_DROPSHADOW | UI_RIGHT, 0.50, colorYel, qfalse );
	}

}

/*
=================
HUD_MegaDrawClientTournamentScore
=================
*/
static void HUD_MegaDrawClientTournamentScore( int x, int y, score_t *score, vec4_t colorWht, vec4_t colorYel, qboolean filterColor ) {
	char		string[64]; // was 1024
	char		string2[64]; // was 1024
	int			lineXPos;
	clientInfo_t	*ci;
	int			ping;

	int			botSkill;
	//char		name[MAX_NAME_LENGTH];
	int			nameWidth;
	int			nameWidth2;
	int			nameWidthCombine;
	float		nameWidthAdj;

	static float	teamColor[3][4] = {
		 { 0, 0, 0, 1.0f },	//black
		 { 1.0f, 0, 0, 1.0f },	//red
		 { 0, 0, 1.0f, 1.0f }	//blue
	};

	// is this an out of range client?
	if ( score->client < 0 || score->client >= cgs.maxclients ) {
		/*Com_Printf( "Bad score->client: %i\n", score->client );*/
		return;
	}

	ci = &cgs.clientinfo[score->client];
	botSkill = ( ci->botSkill > 0 && ci->botSkill <= 99 );

	if ( score->client == cg.snap->ps.clientNum ) {
		CG_DrawPic( x + 2, y-2, 320-9, 48, cgs.media.hudHighlight );
	}

	// draw flag status
	if (cg.time&384) {
		if ( ci->powerups & ( 1 << PW_REDFLAG ) ) {
			// red
			trap_R_SetColor( teamColor[1] );
			CG_DrawPic( x+12, y+7, 16, 16, cgs.media.flagIconWhite );
			trap_R_SetColor( NULL );
		} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) ) {
			// blue
			trap_R_SetColor( teamColor[2] );
			CG_DrawPic( x+12, y+7, 16, 16, cgs.media.flagIconWhite );
			trap_R_SetColor( NULL );
		}
	}

	/*Com_sprintf(string, sizeof(string), "%s", ci->name);
	nameWidth = UI_ReturnStringWidth (string, qfalse) * 0.63;

	if (nameWidth > 240) {
		nameWidthAdj = 240/(float)nameWidth;
		HUD_FuncPosLock( string, 0, x + 56, 0, y, 0, UI_DROPSHADOW, 0.63 * nameWidthAdj, 0.63, colorWht, filterColor );
	} else {
		HUD_FuncPosLock( string, 0, x + 56, 0, y, 0, UI_DROPSHADOW, 0.63, 0.63, colorWht, filterColor );
	}*/

	Com_sprintf(string, sizeof(string), "%s", ci->name);
	Com_sprintf(string2, sizeof(string2), "%s", ci->clan);
	nameWidth = UI_ReturnStringWidth (string, qfalse) * 0.63;
	nameWidth2 = UI_ReturnStringWidth (string2, qfalse) * 0.4;
	nameWidthCombine = nameWidth + nameWidth2;

	if (nameWidthCombine > 240) {
		nameWidthAdj = 240/(float)nameWidthCombine;
		lineXPos = x + 56;
		HUD_FuncPosLock( string2, 0, lineXPos, 0, y, 0, UI_DROPSHADOW, 0.4 * nameWidthAdj, 0.4, colorWht, filterColor );
		lineXPos += nameWidth2 * nameWidthAdj;
		HUD_FuncPosLock( string, 0, lineXPos, 0, y, 0, UI_DROPSHADOW, 0.63 * nameWidthAdj, 0.63, colorWht, filterColor );
	} else {
		lineXPos = x + 56;
		HUD_FuncPosLock( string2, 0, lineXPos, 0, y, 0, UI_DROPSHADOW, 0.4, 0.4, colorWht, filterColor );
		lineXPos += nameWidth2;
		HUD_FuncPosLock( string, 0, lineXPos, 0, y, 0, UI_DROPSHADOW, 0.63, 0.63, colorWht, filterColor );
	}


	// draw player port
	CG_DrawPic( x + 39, y + 2, 16, 16, cgs.media.hudPlayerPort[score->client] );
	/*trap_R_DrawStretchPic( x + 34, y - 2, 16, 24,
					0.0f, 0.0f, 1.0f, 0.75f, cgs.media.hudDefaultPort );*/

	// draw bot icon
	if ( botSkill ) {
		// if this is a cpu bot, indicate that this is so
//		CG_DrawCustomString( x-1, y+3, 6, 8, "^3BOT", fade );// mmp - need to make new bot icons... if possible  :(
		UI_DrawCustomProportionalString( x + 20, y, "BOT", UI_DROPSHADOW | UI_CENTER, 0.50, colorYel, qfalse );
	} else if ( ci->handicap > 0 ) {
		// draw handicap if any is set
		Com_sprintf( string, sizeof( string ), S_COLOR_GREEN"%2i%%", ci->handicap );
//		CG_DrawCustomString( x, y+3, 8, 8, string, fade );
		UI_DrawCustomProportionalString( x + 20, y, string, UI_DROPSHADOW | UI_CENTER, 0.50, colorWht, qfalse );
	}

	// Show status
	if (cg.warmup && !botSkill && score->client < 64) {
		if( cg.readyMask[score->client >> 4] & ( 1 << ( score->client & 15 ) ) )
			CG_DrawPic( x+12, y+3, 16, 16, cgs.media.scoreIconOK );
		else
			CG_DrawPic( x+12, y+3, 16, 16, cgs.media.scoreIconNo );
	}
	else {
		if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << score->client ) ) {
			CG_DrawPic( x+12, y+3, 16, 16, cgs.media.scoreIconOK );
		}
	}

	/*
	-------------------------
	draw second and third row
	-------------------------
	*/
	y += 11;

	ping = score->ping;
	if ( ping != -1 ) {
		/*UI_DrawCustomProportionalString( x + 56, y, "Score:", UI_DROPSHADOW, 0.50, colorYel, qfalse );
		UI_DrawCustomProportionalString( x + 137, y, va("%i",score->score), UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qtrue );*/
		UI_DrawCustomProportionalString( x + 155, y, "Ping:", UI_DROPSHADOW, 0.50, colorYel, qfalse );
		if ( hud_scoreboard_pingType.integer == 0 ) {
			HUD_MegaDrawPingBar ( x + 217, y, score->ping, hud_scoreboard_barType.integer );
		} else {
			if ( ci->botSkill <= 0 )
				UI_DrawCustomProportionalString( x + 217, y, va("%i",score->ping), UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qfalse );
			else
				UI_DrawCustomProportionalString( x + 217, y, "666", UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qfalse );
		}
		UI_DrawCustomProportionalString( x + 235, y, "Time:", UI_DROPSHADOW, 0.50, colorYel, qfalse );
		if ( score->time < 99 )
			UI_DrawCustomProportionalString( x + 297, y, va("%im",score->time), UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qfalse );
		else
			UI_DrawCustomProportionalString( x + 297, y, "99m", UI_DROPSHADOW | UI_RIGHT, 0.50, colorWht, qfalse );

		// third row
		y += 10;
		UI_DrawCustomProportionalString( x + 297, y, va("%i",score->score), UI_DROPSHADOW | UI_RIGHT, 1.75, colorWht, qtrue );

	} else {
		if (cg.time & 256)
			UI_DrawCustomProportionalString( x + 297, y, "Connecting...", UI_DROPSHADOW | UI_RIGHT, 0.50, colorYel, qfalse );
	}

}

/*
=================
HUD_MegaDrawSpectator
=================
*/

void HUD_MegaDrawSpectator ( int x, int y ) {
	int		i;
	int		ssize;
	score_t		*score;
	int		count;
	int		playerCount;
	clientInfo_t	*ci;
	float		color[4];
	qhandle_t	hShader;
	int		yAdj;

	yAdj = y * cgs.screenYScale;

	color[0] = color[1] = color[2] = color[3] = 1.0f;

	// clear char info
	cgs.spectatorInfoScroll[0] = '^';
	cgs.spectatorInfoScroll[1] = '7';
	cgs.spectatorInfoScroll[2] = '\0';

	cgs.spectatorInfo[0] = '\0';

	for( i=0; i < cg.numScores && strlen(cgs.spectatorInfo) < (SINFO_CHAR_BUFFER_SIZE-48); i++ ){

		score = &cg.scores[ i ];
		ci = &cgs.clientinfo[ score->client ];

		//mmp - if player is a stalking creep who loves to stalk his or hers victoms
		/*if (cgs.creepyMode) {
			//mmp - i'm sure there's a better way of doing this
			strcpy( nameTest, ci->name );
			if (nameTest[0] == 46)
				continue;
		}*/

		if( ci->team != TEAM_SPECTATOR ){
			continue;
		}

		strcat( cgs.spectatorInfo, va(S_COLOR_WHITE"%s",ci->name) );
		/*if ( score->ping == -1 ) {
			if (cg.time&384)
				strcat( cgs.spectatorInfo, "^7[^3Conn^7]  " );
			else
				strcat( cgs.spectatorInfo, "^7[    ]  " );
		} else if ( cgs.gametype == GT_TOURNAMENT || cgs.gametype == GT_CTF_DUEL ) {
			strcat( cgs.spectatorInfo, va ("^7 (%i/%i)  ", ci->wins, ci->losses ) );
		} else*/
			strcat( cgs.spectatorInfo, "  " );

	}

	hShader = cgs.media.hudScoreBoardSpec; // scoreboard bg

	trap_R_DrawStretchPic( (x + 144) * cgs.screenXScale + cgs.screenXOffset, yAdj + cgs.screenYOffset,
					488 * cgs.screenXScale, 16 * cgs.screenYScale,
					(float) 8/16, (float) 0/16,
					(float) 10/16, (float) 16/16, hShader ); // left cap

	ssize = UI_ReturnStringLimit ( cgs.spectatorInfo, qfalse, 486 + 4, 0.50, qtrue );
	if ( ssize > 0 && ssize < SINFO_CHAR_BUFFER_SIZE-48 ) {
		cgs.spectatorInfo[ssize] = '\0';
	}
	UI_DrawCustomProportionalString( x + 146, y + 4, cgs.spectatorInfo, UI_DROPSHADOW, 0.50, color, qfalse );

	trap_R_DrawStretchPic( x * cgs.screenXScale + cgs.screenXOffset, yAdj + cgs.screenYOffset,
					10 * cgs.screenXScale, 16 * cgs.screenYScale,
					(float) 0/16, (float) 0/16,
					(float) 5/16, (float) 16/16, hShader ); // left cap
	trap_R_DrawStretchPic( (x + 10) * cgs.screenXScale + cgs.screenXOffset, yAdj + cgs.screenYOffset,
					134 * cgs.screenXScale, 16 * cgs.screenYScale,
					(float) 10/32, (float) 0/16,
					(float) 11/32, (float) 16/16, hShader ); // spectator bg
	trap_R_DrawStretchPic( (x + 632) * cgs.screenXScale + cgs.screenXOffset, yAdj + cgs.screenYOffset,
					8 * cgs.screenXScale, 16 * cgs.screenYScale,
					(float) 12/16, (float) 0/16,
					(float) 16/16, (float) 16/16, hShader ); // right cap

	color[0] = color[1] = color[2] = 0.0f;
	UI_DrawCustomProportionalString( x + 12, y + 3, "SPECTATOR(S)", 0, 0.73, color, qfalse );

}

/*
=================
HUD_MegaDrawScoreBoard
=================
*/
qboolean HUD_MegaDrawScoreBoard( int posCenter ) {

	int			x, y, y2, w, h, i, n1, n2;
	float		xf, yf, wf, hf, sxf, syf;
	char		*s;
	int			maxClients;
	score_t		*score;
	int			count;
	clientInfo_t	*ci;
	int			posLeft, posRight;
	qboolean	filterColor;
	int			totalDispPlayers;

	team_t		team;
	vec4_t		colorWht, colorYel, colorBlk, colorCus;
	int		cnt1, cnt2;

	int		prevScore = 99999;
	int		curRank = 0;
	int		intRank = 0;

	qhandle_t	hShader, hShader2;

	/*vec4_t		colorTest; // test

	// test
	colorTest[0] = 1.0;
	colorTest[1] = 0.5;
	colorTest[2] = 0.0;
	colorTest[3] = 1.0;
	HUD_MegaDrawOutlinedBox ( 16, 16, 6, 6, colorTest );*/

	posLeft = posCenter - 320;
	posRight = posCenter + 320;

	colorWht[0] = colorWht[1] = colorWht[2] = colorWht[3] = 1.0; // rgba
	colorYel[0] = colorYel[1] = colorYel[3] = 1.0; // rg-a
	colorYel[2] = 0.0; // blue
	colorBlk[0] = colorBlk[1] = colorBlk[2] = 0.0; // rgb-
	colorBlk[3] = 1.0; // alpha

	// was drawing the scoreboard requested
	if ( !cg.showScores && cg.predictedPlayerState.pm_type != PM_DEAD &&
		 cg.predictedPlayerState.pm_type != PM_INTERMISSION ) {

		// next time scoreboard comes up, don't print killer
		cg.deferredPlayerLoading = 0;
		cg.killerName[0] = 0;
		return qfalse;
	}

	if ( cg.numScores < 1 ) {
		//s = "SCOREBOARD FAILED TO LOAD, PLEASE RETRY";
		s = "SCOREBOARD LOADING...";
		y = 64;
		UI_DrawCustomProportionalString( posCenter, y, s, UI_DROPSHADOW | UI_CENTER, 0.5, colorWht, qfalse );
		return qtrue; // though, the scoreboard failed to get information
			      // regardless, we must still show that the bind is working
	}

	// fragged by ... line
	/*if ( cg.killerName[0] ) {
		s = va("Fragged by %s", cg.killerName );
		y = 40;
		UI_DrawCustomProportionalString( posCenter, y, s, UI_DROPSHADOW | UI_CENTER, 0.5, colorWht, qfalse );
	}*/

	if ( hud_filterColors.integer & FC_SCOREBOARD_NAMES )
		filterColor = qtrue;
	else
		filterColor = qfalse;

	// current rank
	if ( cgs.gametype < GT_TEAM) {
		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
			s = va("%s place with %i",
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );
			y = 60;
			UI_DrawCustomProportionalString( posCenter, y, s, UI_DROPSHADOW | UI_CENTER, 1.0, colorWht, qfalse );
		}
	} else {
		if ( cg.teamScores[0] == cg.teamScores[1] ) {
			s = va("Teams are tied at %i", cg.teamScores[0] );
		} else if ( cg.teamScores[0] >= cg.teamScores[1] ) {
			s = va("Red leads %i to %i",cg.teamScores[0], cg.teamScores[1] );
		} else {
			s = va("Blue leads %i to %i",cg.teamScores[1], cg.teamScores[0] );
		}

		y = 60;
		UI_DrawCustomProportionalString( posCenter, y, s, UI_DROPSHADOW | UI_CENTER, 1.0, colorWht, colorYel, qfalse );
	}

/*
----------------------
Player score and stats
----------------------
*/

	y = 111;
	x = posCenter - 315;

	if ( /*cgs.gametype < GT_TEAM*/ cgs.gametype == GT_FFA ) {

		// teamless scoreboard

		// bg
		colorCus[0] = colorCus[1] = colorCus[2] = 0.5;
		colorCus[3] = 1.0;
		trap_R_SetColor( colorCus );
		hShader = cgs.media.hudScoreBoardBG; // scoreboard bg
		trap_R_DrawStretchPic( (float)(x+1) * cgs.screenXScale + cgs.screenXOffset,
						(float)(y - 2) * cgs.screenYScale + cgs.screenYOffset,
						(float)628.0 * cgs.screenXScale, (float)240.0 * cgs.screenYScale,
						(float) 0.0, (float) 0.0,
						(float) 1.0, (float) 1.0, hShader );
		trap_R_SetColor( NULL );

		// text
		for ( i = 0, cnt1 = 0; i < cg.numScores; i++ ) {
			score = &cg.scores[i];
			ci = &cgs.clientinfo[ score->client ];

			// spectators are not in the game
			if ( ci->team == TEAM_SPECTATOR )
				continue;

			if ( cnt1 < 20 ) {
				intRank++;
				if ( score->score < prevScore ) {
					curRank = intRank;
					prevScore = score->score;
				}
				HUD_MegaDrawClientScore( x, y, score, curRank, colorWht, colorYel, filterColor );

				if (cnt1 != 9)
					y += 24;
				else {
					x += 315;
					y = 111;
				}
			}
			cnt1 ++;
		}

		// top
		hShader = cgs.media.hudScoreBoard [ TEAM_FREE ]; // score board for team free games
		sxf = 8 * cgs.screenXScale;
		syf = 32 * cgs.screenYScale;
		yf = 78 * cgs.screenYScale + cgs.screenYOffset;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 0/8, (float) 0/8,
						(float) 1/8, (float) 4/8, hShader );
		xf += 8 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, 128.0 * cgs.screenXScale, syf,
						(float) 1/8, (float) 0/8,
						(float) 3/16, (float) 4/8, hShader );
		xf += 128 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, 40.0 * cgs.screenXScale, syf,
						(float) 1/8, (float) 0/8,
						(float) 6/8, (float) 4/8, hShader );
		syf = 8 * cgs.screenYScale;
		yf += 24 * cgs.screenYScale;
		xf += 40 * cgs.screenXScale;
		wf = ( ( posRight * cgs.screenXScale + cgs.screenXOffset ) - (xf + 8 * cgs.screenXScale) );
		trap_R_DrawStretchPic( xf, yf, wf, syf,
						(float) 2/8, (float) 4/8,
						(float) 3/8, (float) 5/8, hShader );
		xf += wf;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 7/8, (float) 3/8,
						(float) 8/8, (float) 4/8, hShader );

		// middle
		yf += 8 * cgs.screenYScale;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		hf = ( 24 * 10 - 2 ) * cgs.screenYScale;
		trap_R_DrawStretchPic( xf, yf, sxf, hf,
						(float) 0/8, (float) 4/8,
						(float) 1/8, (float) 5/8, hShader );
		xf = ( posRight - 8 ) * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, hf,
						(float) 7/8, (float) 4/8,
						(float) 8/8, (float) 5/8, hShader );

		// bottom
		yf += hf;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 0/8, (float) 5/8,
						(float) 1/8, (float) 6/8, hShader );
		xf += 8 * cgs.screenXScale;
		wf = ( ( posRight * cgs.screenXScale + cgs.screenXOffset ) - (xf + 8 * cgs.screenXScale) );
		trap_R_DrawStretchPic( xf, yf, wf, syf,
						(float) 1/8, (float) 5/8,
						(float) 2/8, (float) 6/8, hShader );
		xf += wf;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 7/8, (float) 5/8,
						(float) 8/8, (float) 6/8, hShader );


		// player amount
		if ( cnt1 == 0 )
			s = "No players";
		else if ( cnt1 == 1 )
			s = "1 player";
		else
			s = va("%i players", cnt1 );
		UI_DrawCustomProportionalString( posLeft + 12, 87, s, UI_DROPSHADOW, 0.85, colorWht, qfalse );

	} else if ( cgs.gametype == GT_TOURNAMENT ) {

		// tournament scoreboard

		// bg
		colorCus[0] = colorCus[1] = colorCus[2] = 0.5;
		colorCus[3] = 1.0;
		trap_R_SetColor( colorCus );
		trap_R_DrawStretchPic( (float)(x+1) * cgs.screenXScale + cgs.screenXOffset,
						(float)(y - 2) * cgs.screenYScale + cgs.screenYOffset,
						(float)628.0 * cgs.screenXScale, (float)48.0 * cgs.screenYScale,
						(float) 0.0, (float) 0.0,
						(float) 1.0, (float) 0.1, cgs.media.hudScoreBoardBG );
		trap_R_SetColor( NULL );

		// text
		for ( i = 0, totalDispPlayers = 0; i < cg.numScores && totalDispPlayers < 2; i++ ) {
			score = &cg.scores[i];
			ci = &cgs.clientinfo[ score->client ];

			// spectators are not in the game
			if ( ci->team == TEAM_SPECTATOR ) {
				continue;
			}

			totalDispPlayers++;

			HUD_MegaDrawClientTournamentScore( x, y, score, colorWht, colorYel, filterColor );
			x += 315;

		}

		// top
		hShader = cgs.media.hudScoreBoard [ TEAM_FREE ]; // score board for team free games
		sxf = 8 * cgs.screenXScale;
		syf = 8 * cgs.screenYScale;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		yf = 102 * cgs.screenYScale + cgs.screenYOffset;
		wf = ( ( posRight * cgs.screenXScale + cgs.screenXOffset ) - (xf + 16 * cgs.screenXScale) );

		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 6/8, (float) 0/8,
						(float) 7/8, (float) 1/8, hShader );
		xf += sxf;
		trap_R_DrawStretchPic( xf, yf, wf, syf,
						(float) 2/8, (float) 4/8,
						(float) 3/8, (float) 5/8, hShader );
		xf += wf;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 7/8, (float) 3/8,
						(float) 8/8, (float) 4/8, hShader );

		// middle
		yf += 8 * cgs.screenYScale;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		hf = ( 46 ) * cgs.screenYScale;
		trap_R_DrawStretchPic( xf, yf, sxf, hf,
						(float) 0/8, (float) 4/8,
						(float) 1/8, (float) 5/8, hShader );
		xf = ( posRight - 8 ) * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, hf,
						(float) 7/8, (float) 4/8,
						(float) 8/8, (float) 5/8, hShader );

		// bottom
		yf += hf;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 0/8, (float) 5/8,
						(float) 1/8, (float) 6/8, hShader );
		xf += 8 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, wf, syf,
						(float) 1/8, (float) 5/8,
						(float) 2/8, (float) 6/8, hShader );
		xf += wf;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 7/8, (float) 5/8,
						(float) 8/8, (float) 6/8, hShader );

	} else {

		// team scoreboard
		y2 = 111;

		// bg
		colorCus[0] = 1.00;
		colorCus[1] = 0.00;
		colorCus[2] = 0.00;
		colorCus[3] = 1.00;
		trap_R_SetColor( colorCus );
		hShader = cgs.media.hudScoreBoardBG; // scoreboard bg
		trap_R_DrawStretchPic( (float)(x+1) * cgs.screenXScale + cgs.screenXOffset,
						(float)(y - 2) * cgs.screenYScale + cgs.screenYOffset,
						(float)308.0 * cgs.screenXScale, (float)240.0 * cgs.screenYScale,
						(float) 0.0, (float) 0.0,
						(float) 1.0, (float) 1.0, hShader );
		colorCus[2] = 1.00;
		colorCus[0] = 0.00;
		trap_R_SetColor( colorCus );
		trap_R_DrawStretchPic( (float)(posCenter+6) * cgs.screenXScale + cgs.screenXOffset,
						(float)(y - 2) * cgs.screenYScale + cgs.screenYOffset,
						(float)308.0 * cgs.screenXScale, (float)240.0 * cgs.screenYScale,
						(float) 0.0, (float) 0.0,
						(float) 1.0, (float) 1.0, hShader );
		trap_R_SetColor( NULL );

		for ( i = 0, cnt1 = 0, cnt2 = 0; i < cg.numScores; i++ ) {
			score = &cg.scores[i];
			ci = &cgs.clientinfo[ score->client ];

			// spectators are not in the game
			if ( ci->team == TEAM_SPECTATOR )
				continue;

			if ( ci->team == TEAM_RED ) {
				if ( cnt1 < 10 ) {
					HUD_MegaDrawClientScore( x, y, score, -1, colorWht, colorYel, filterColor );
					y += 24;
				}
				cnt1 ++;
			} else {
				if ( cnt2 < 10 ) {
					HUD_MegaDrawClientScore( posCenter, y2, score, -1, colorWht, colorYel, filterColor );
					y2 += 24;
				}
				cnt2 ++;
			}

		}

		// top red
		hShader = cgs.media.hudScoreBoard [ TEAM_RED ]; // score board for team free games
		sxf = 8 * cgs.screenXScale;
		syf = 32 * cgs.screenYScale;
		yf = 78 * cgs.screenYScale + cgs.screenYOffset;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 0/8, (float) 0/8,
						(float) 1/8, (float) 4/8, hShader );
		xf += 8 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, 128.0 * cgs.screenXScale, syf,
						(float) 1/8, (float) 0/8,
						(float) 3/16, (float) 4/8, hShader );
		xf += 128 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, 40.0 * cgs.screenXScale, syf,
						(float) 1/8, (float) 0/8,
						(float) 6/8, (float) 4/8, hShader );
		syf = 8 * cgs.screenYScale;
		yf += 24 * cgs.screenYScale;
		xf += 40 * cgs.screenXScale;
		wf = ( ( posCenter * cgs.screenXScale + cgs.screenXOffset ) - (xf + 8 * cgs.screenXScale) );
		trap_R_DrawStretchPic( xf, yf, wf, syf,
						(float) 2/8, (float) 4/8,
						(float) 3/8, (float) 5/8, hShader );
		xf += wf;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 7/8, (float) 3/8,
						(float) 8/8, (float) 4/8, hShader );

		// top blue
		hShader2 = cgs.media.hudScoreBoard [ TEAM_BLUE ]; // score board for team free games
		syf = 32 * cgs.screenYScale;
		yf = 78 * cgs.screenYScale + cgs.screenYOffset;
		xf += 8 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 0/8, (float) 0/8,
						(float) 1/8, (float) 4/8, hShader2 );
		xf += 8 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, 128.0 * cgs.screenXScale, syf,
						(float) 1/8, (float) 0/8,
						(float) 3/16, (float) 4/8, hShader2 );
		xf += 128 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, 40.0 * cgs.screenXScale, syf,
						(float) 1/8, (float) 0/8,
						(float) 6/8, (float) 4/8, hShader2 );
		syf = 8 * cgs.screenYScale;
		yf += 24 * cgs.screenYScale;
		xf += 40 * cgs.screenXScale;
		wf = ( ( posRight * cgs.screenXScale + cgs.screenXOffset ) - (xf + 8 * cgs.screenXScale) );
		trap_R_DrawStretchPic( xf, yf, wf, syf,
						(float) 2/8, (float) 4/8,
						(float) 3/8, (float) 5/8, hShader2 );
		xf += wf;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 7/8, (float) 3/8,
						(float) 8/8, (float) 4/8, hShader2 );

		// middle red
		yf += 8 * cgs.screenYScale;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		hf = ( 24 * 10 - 2 ) * cgs.screenYScale;
		trap_R_DrawStretchPic( xf, yf, sxf, hf,
						(float) 0/8, (float) 4/8,
						(float) 1/8, (float) 5/8, hShader );
		xf = ( posCenter - 8 ) * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, hf,
						(float) 7/8, (float) 4/8,
						(float) 8/8, (float) 5/8, hShader );

		// middle blue
		xf += 8 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, sxf, hf,
						(float) 0/8, (float) 4/8,
						(float) 1/8, (float) 5/8, hShader );
		xf = ( posRight - 8 ) * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, hf,
						(float) 7/8, (float) 4/8,
						(float) 8/8, (float) 5/8, hShader );

		// bottom red
		yf += hf;
		xf = posLeft * cgs.screenXScale + cgs.screenXOffset;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 0/8, (float) 5/8,
						(float) 1/8, (float) 6/8, hShader );
		xf += 8 * cgs.screenXScale;
		wf = ( ( posCenter * cgs.screenXScale + cgs.screenXOffset ) - (xf + 8 * cgs.screenXScale) );
		trap_R_DrawStretchPic( xf, yf, wf, syf,
						(float) 1/8, (float) 5/8,
						(float) 2/8, (float) 6/8, hShader );
		xf += wf;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 7/8, (float) 5/8,
						(float) 8/8, (float) 6/8, hShader );

		// bottom blue
		xf +=8 * cgs.screenXScale;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 0/8, (float) 5/8,
						(float) 1/8, (float) 6/8, hShader );
		xf += 8 * cgs.screenXScale;
		wf = ( ( posRight * cgs.screenXScale + cgs.screenXOffset ) - (xf + 8 * cgs.screenXScale) );
		trap_R_DrawStretchPic( xf, yf, wf, syf,
						(float) 1/8, (float) 5/8,
						(float) 2/8, (float) 6/8, hShader );
		xf += wf;
		trap_R_DrawStretchPic( xf, yf, sxf, syf,
						(float) 7/8, (float) 5/8,
						(float) 8/8, (float) 6/8, hShader );

		// player amount
		if ( cnt1 == 0 )
			s = "No players";
		else if ( cnt1 == 1 )
			s = "1 player";
		else
			s = va("%i players", cnt1 );
		UI_DrawCustomProportionalString( posLeft + 12, 87, s, UI_DROPSHADOW, 0.85, colorWht, qfalse );

		if ( cnt2 == 0 )
			s = "No players";
		else if ( cnt2 == 1 )
			s = "1 player";
		else
			s = va("%i players", cnt2 );
		UI_DrawCustomProportionalString( posCenter + 12, 87, s, UI_DROPSHADOW, 0.85, colorWht, qfalse );

	}


/*
-----------------
draw spectator info
-----------------
*/

	HUD_MegaDrawSpectator (posCenter - 320, 356);


/*
-----------------
draw intermission and/or other chat things
-----------------
*/

	if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		HUD_MegaDrawIntermissionInfo( 4, 380, UI_DROPSHADOW, 0.5 );
	}

/*
-----------------
*/

	return qtrue; // yes, the scoreboard was drawn

}



















