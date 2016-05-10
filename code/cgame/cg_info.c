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
// cg_info.c -- display information while data is being loading

#include "cg_local.h"

#define MAX_LOADING_PLAYER_ICONS	16
#define MAX_LOADING_ITEM_ICONS		26

static int			loadingPlayerIconCount;
static int			loadingItemIconCount;
static qhandle_t	loadingPlayerIcons[MAX_LOADING_PLAYER_ICONS];
static qhandle_t	loadingItemIcons[MAX_LOADING_ITEM_ICONS];


/*
===================
CG_DrawLoadingIcons
===================
*/
static void CG_DrawLoadingIcons( void ) {
	int		n;
	int		x, y;
	int		m;

	/*for( n = 0; n < loadingPlayerIconCount; n++ ) {
		x = 16 + n * 78;
		y = 324-40;
		CG_DrawPic( x, y, 64, 64, loadingPlayerIcons[n] );
	}

	for( n = 0; n < loadingItemIconCount; n++ ) {
		y = 400-40;
		if( n >= 13 ) {
			y += 40;
		}
		x = 16 + n % 13 * 48;
		CG_DrawPic( x, y, 32, 32, loadingItemIcons[n] );
	}*/

	// TODO: merge with items below or remove completely, since MFA doesn't care about player models
	/*if (loadingPlayerIconCount>11)
		m = loadingPlayerIconCount-11;
	else
		m = 0;
	//max 11
	y = 383;//375
	x = 28-52;
	for( n = m; n < loadingPlayerIconCount; n++ ) {
		x += 52;
		CG_DrawPic( x, y, 48, 48, loadingPlayerIcons[n] );
	}*/

	if (loadingItemIconCount>29)
		m = loadingItemIconCount-29;
	else
		m = 0;
	//max 29
	//y += 56;
	y = 439;
	//x = 28-20;
	x = 28;
	for( n = m; n < loadingItemIconCount; n++, x += 20 ) {
		//x += 20;
		CG_DrawPic( x, y, 16, 16, loadingItemIcons[n] );
	}

}


/*
======================
CG_LoadingString

======================
*/
void CG_LoadingString( const char *s ) {
	Q_strncpyz( cg.infoScreenText, s, sizeof( cg.infoScreenText ) );

	trap_UpdateScreen();
}

/*
===================
CG_LoadingItem
===================
*/
void CG_LoadingItem( int itemNum ) {
	gitem_t		*item;

	item = &bg_itemlist[itemNum];

	if ( item->icon && loadingItemIconCount < MAX_LOADING_ITEM_ICONS ) {
		loadingItemIcons[loadingItemIconCount++] = trap_R_RegisterShaderNoMip( item->icon );
	}

	CG_LoadingString( item->pickup_name );
}

/*
===================
CG_LoadingClient
===================
*/
void CG_LoadingClient( int clientNum ) {
	const char		*info;
	char			*skin;
	char			personality[MAX_QPATH];
	char			model[MAX_QPATH];
	char			iconName[MAX_QPATH];

	info = CG_ConfigString( CS_PLAYERS + clientNum );

	if ( loadingPlayerIconCount < MAX_LOADING_PLAYER_ICONS ) {
		Q_strncpyz( model, Info_ValueForKey( info, "model" ), sizeof( model ) );
		skin = strrchr( model, '/' );
		if ( skin ) {
			*skin++ = '\0';
		} else {
			skin = "default";
		}

		Com_sprintf( iconName, MAX_QPATH, "models/players/%s/icon_%s.tga", model, skin );

		loadingPlayerIcons[loadingPlayerIconCount] = trap_R_RegisterShaderNoMip( iconName );
		if ( !loadingPlayerIcons[loadingPlayerIconCount] ) {
			Com_sprintf( iconName, MAX_QPATH, "models/players/characters/%s/icon_%s.tga", model, skin );
			loadingPlayerIcons[loadingPlayerIconCount] = trap_R_RegisterShaderNoMip( iconName );
		}
		if ( !loadingPlayerIcons[loadingPlayerIconCount] ) {
			Com_sprintf( iconName, MAX_QPATH, "models/players/%s/icon_%s.tga", DEFAULT_MODEL, "default" );
			loadingPlayerIcons[loadingPlayerIconCount] = trap_R_RegisterShaderNoMip( iconName );
		}
		if ( loadingPlayerIcons[loadingPlayerIconCount] ) {
			loadingPlayerIconCount++;
		}
	}

	Q_strncpyz( personality, Info_ValueForKey( info, "n" ), sizeof(personality) );
	Q_CleanStr( personality );

	if( cgs.gametype == GT_SINGLE_PLAYER ) {
		trap_S_RegisterSound( va( "sound/player/announce/%s.wav", personality ), qtrue );
	}

	CG_LoadingString( personality );
}


/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawInformation( void ) {
	const char	*s;
	const char	*map;
	const char	*info;
	const char	*sysInfo;
	int			y;
	int			value;
	qhandle_t	background;
	qhandle_t	levelshot;
	qhandle_t	detail;
	qhandle_t	border;
	char		buf[1024];
	int			centerPos;
	int			rightPos;

	vec4_t		colorTrans;

	//trap_R_SetColor( NULL );

	colorTrans[0] = colorTrans[1] = colorTrans[2] = 0.0f;
	colorTrans[3] = 0.50f;

	// add a background
	info = CG_ConfigString( CS_SERVERINFO );
	sysInfo = CG_ConfigString( CS_SYSTEMINFO );

	// set the center xpos of the screen
	rightPos = 640 * hud_aspectRatioScale.value;
	centerPos = rightPos / 2;

	// draw background
	background = trap_R_RegisterShaderNoMip( "menu/art/bg_loader.tga" );
	trap_R_DrawStretchPic( 0, 0, cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, 0, 0, 1, 1, background );

	// draw section borders
	// --------------------
	border = trap_R_RegisterShaderNoMip( "menu/art/bg_border1" );

	trap_R_SetColor( colorTrans );

	// levelshot border
	trap_R_DrawStretchPic( 316 * cgs.screenXScale, 86 * cgs.screenYScale + cgs.screenYOffset,
		8 * cgs.screenXScale, 8 * cgs.screenYScale,
		0.75, 0.00, 1.00, 0.25, border );
	trap_R_DrawStretchPic( 316 * cgs.screenXScale, 94 * cgs.screenYScale + cgs.screenYOffset,
		8 * cgs.screenXScale, 232 * cgs.screenYScale,
		0.75, 0.25, 1.00, 0.50, border );
	trap_R_DrawStretchPic( 316 * cgs.screenXScale, 326 * cgs.screenYScale + cgs.screenYOffset,
		8 * cgs.screenXScale, 8 * cgs.screenYScale,
		0.75, 0.75, 1.00, 1.00, border );
	trap_R_DrawStretchPic( 0, 86 * cgs.screenYScale + cgs.screenYOffset,
		316 * cgs.screenXScale, 8 * cgs.screenYScale,
		0.25, 0.00, 0.50, 0.25, border );
	trap_R_DrawStretchPic( 0, 326 * cgs.screenYScale + cgs.screenYOffset,
		316 * cgs.screenXScale, 8 * cgs.screenYScale,
		0.25, 0.75, 0.50, 1.00, border );

	// loaded model border
	trap_R_DrawStretchPic( 20 * cgs.screenXScale, 431 * cgs.screenYScale, 8 * cgs.screenXScale, 32 * cgs.screenYScale,
		0.0, 0.00, 0.25, 1.00, border );
	trap_R_DrawStretchPic( 28 * cgs.screenXScale, 431 * cgs.screenYScale, (rightPos - 56) * cgs.screenXScale, 32 * cgs.screenYScale,
		0.25, 0.00, 0.50, 1.00, border );
	trap_R_DrawStretchPic( (rightPos - 28) * cgs.screenXScale, 431 * cgs.screenYScale, 8 * cgs.screenXScale, 32 * cgs.screenYScale,
		0.75, 0.00, 1.00, 1.00, border );

	// server name
	trap_R_DrawStretchPic( 0, 54 * cgs.screenYScale, rightPos * cgs.screenXScale, 32 * cgs.screenYScale,
		0.25, 0.00, 0.50, 1.00, border );

	trap_R_SetColor( NULL );


	// --------------------

	// draw map levelshot
	map = Info_ValueForKey( info, "mapname" );
	levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", map ) );
	if ( !levelshot ) {
		levelshot = trap_R_RegisterShaderNoMip( "menu/art/unknownmap" );
	}
	trap_R_SetColor( NULL );
	/*CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, levelshot );*/
	/*CG_DrawPic( 0, 90, 320, 240, levelshot );*/
	trap_R_DrawStretchPic( 0, 90 * cgs.screenYScale + cgs.screenYOffset,
		320 * cgs.screenXScale, 240 * cgs.screenYScale,
		0, 0, 1, 1, levelshot );

	// in-game mini map
	cgs.media.miniMap = trap_R_RegisterShaderNoMip( va("levelshots/%s_mini.tga", map) );
	if ( !cgs.media.miniMap ) {
		cgs.media.miniMap = trap_R_RegisterShaderNoMip( "levelshots/default_mini.tga" );
	}
	s = (char *)CG_ConfigString( CS_MINIMAPSCALEX );
	cgs.miniMapXScale = atof(s);
	s = (char *)CG_ConfigString( CS_MINIMAPSCALEY );
	cgs.miniMapYScale = atof(s);

	// blend a detail texture over it
	/*detail = trap_R_RegisterShader( "levelShotDetail" );
	trap_R_DrawStretchPic( 0, 0, cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, 0, 0, 2.5, 2, detail );*/

	// draw the icons of things as they are loaded
	CG_DrawLoadingIcons();

	// the first 150 rows are reserved for the client connection
	// screen to write into
	if ( cg.infoScreenText[0] ) {
		/*UI_DrawProportionalString( 320 * hud_aspectRatioScale.value, 128-32, va("Loading... %s", cg.infoScreenText),
			UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );*/
		UI_DrawString( centerPos, 340, va("Loading \"%s\"...", cg.infoScreenText),
			UI_CENTER|UI_DROPSHADOW, 1.0, 1.0, colorWhite, qtrue );
	} else {
		/*UI_DrawProportionalString( 320 * hud_aspectRatioScale.value, 128-32, "Awaiting snapshot...",
			UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );*/
		UI_DrawString( centerPos, 340, "Awaiting snapshot...",
			UI_CENTER|UI_DROPSHADOW, 1.0, 1.0, colorWhite, qtrue );
	}

	// draw info string information

	y = 180-32;

	// don't print server lines if playing a local game
	trap_Cvar_VariableStringBuffer( "sv_running", buf, sizeof( buf ) );
	if ( !atoi( buf ) ) {
		// server hostname
		Q_strncpyz(buf, Info_ValueForKey( info, "sv_hostname" ), 1024);
		/*Q_CleanStr(buf);
		UI_DrawProportionalString( 320, y, buf,
			UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;*/
		UI_DrawString( 8, 64, buf,
			UI_DROPSHADOW, 1.0, 1.0, colorWhite, qfalse );

		// pure server
		/*s = Info_ValueForKey( sysInfo, "sv_pure" );
		if ( s[0] == '1' ) {
			UI_DrawProportionalString( 320, y, "Pure Server",
				UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}*/

		// server-specific message of the day
		/*s = CG_ConfigString( CS_MOTD );
		if ( s[0] ) {
			UI_DrawProportionalString( 320, y, s,
				UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}*/

		// some extra space after hostname and motd
		//y += 10;
	}

	// map name
	if ( map[0] ) {
		UI_DrawCustomProportionalString( 8, 96, map, UI_LEFT|UI_DROPSHADOW, 0.75, colorWhite, qtrue );
	}

	// map-specific message (long map name)
	s = CG_ConfigString( CS_MESSAGE );
	if ( s[0] ) {
		UI_DrawCustomProportionalString( 8, 110, s, UI_LEFT|UI_DROPSHADOW, 0.5, colorWhite, qtrue );
	}

	// cheats warning
	s = Info_ValueForKey( sysInfo, "sv_cheats" );
	if ( s[0] == '1' ) {
		/*UI_DrawProportionalString( 320, y, "CHEATS ARE ENABLED",
			UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;*/
		UI_DrawCustomProportionalString( 336, 90, "Cheats are enabled", UI_LEFT|UI_DROPSHADOW, 0.6, colorWhite, qtrue );
	}

	// protip message
	if ( map[0] ) {
		UI_DrawCustomProportionalString( centerPos, 464, "PROTIP: By playing MFArena, you are supporting MFArena. Thank you.", UI_CENTER | UI_DROPSHADOW, 0.50, g_color_table[ColorIndex(COLOR_YELLOW)], qtrue );
	}

	// game type
	/*switch ( cgs.gametype ) {
	case GT_FFA:
		s = "Free For All";
		break;
	case GT_SINGLE_PLAYER:
		s = "Single Player";
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
#ifdef MISSIONPACK
	case GT_1FCTF:
		s = "One Flag CTF";
		break;
	case GT_OBELISK:
		s = "Overload";
		break;
	case GT_HARVESTER:
		s = "Harvester";
		break;
#endif
	default:
		s = "Unknown Gametype";
		break;
	}
	UI_DrawProportionalString( 320, y, s,
		UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	y += PROP_HEIGHT;*/

	/*value = atoi( Info_ValueForKey( info, "timelimit" ) );
	if ( value ) {
		UI_DrawProportionalString( 320, y, va( "timelimit %i", value ),
			UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;
	}

	if (cgs.gametype < GT_CTF ) {
		value = atoi( Info_ValueForKey( info, "fraglimit" ) );
		if ( value ) {
			UI_DrawProportionalString( 320, y, va( "fraglimit %i", value ),
				UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}
	}

	if (cgs.gametype >= GT_CTF) {
		value = atoi( Info_ValueForKey( info, "capturelimit" ) );
		if ( value ) {
			UI_DrawProportionalString( 320, y, va( "capturelimit %i", value ),
				UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}
	}*/
}

