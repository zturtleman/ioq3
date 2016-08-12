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
// cg_drawtools.c -- helper functions called by cg_draw, cg_scoreboard, cg_info, etc
#include "cg_local.h"

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	float		ocX, ocY;

/*
	// adjust for wide screens
	if ( cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		*x += 0.5 * ( cgs.glconfig.vidWidth - ( cgs.glconfig.vidHeight * 640 / 480 ) );
	}
*/

	// scale for screen sizes
	if ( cgs.overscanAdj ) {
		// adjustments for overscan tvs
		*x = *x * cgs.screenXScale + cgs.screenXOffset;
		*y = *y * cgs.screenYScale + cgs.screenYOffset;
		*w = *w * cgs.screenXScale;
		*h = *h * cgs.screenYScale;
	} else {
		*x *= cgs.screenXScale;
		*y *= cgs.screenYScale;
		*w *= cgs.screenXScale;
		*h *= cgs.screenYScale;
	}
}

/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader );

	trap_R_SetColor( NULL );
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenXScale;
	trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenYScale;
	trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color ) {
	trap_R_SetColor( color );

  CG_DrawTopBottom(x, y, width, height, size);
  CG_DrawSides(x, y, width, height, size);

	trap_R_SetColor( NULL );
}



/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader ) {
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}


/*
================
CG_DrawPicExt

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPicExt( float x, float y, float width, float height, float tx1, float ty1, float tx2, float ty2, qhandle_t hShader ) {
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, tx1, ty1, tx2, ty2, hShader );
}



/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar( int x, int y, int width, int height, int ch ) {
	int row, col;
	float frow, fcol;
	float size;
	float	ax, ay, aw, ah;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	trap_R_DrawStretchPic( ax, ay, aw, ah,
					   fcol, frow,
					   fcol + size, frow + size,
					   cgs.media.charsetShader );
}


/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExt( int x, int y, const char *string, const float *setColor,
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars ) {
	vec4_t		color;
	const char	*s;
	int			xx;
	int			cnt;

	if (maxChars <= 0)
		maxChars = 32767; // do them all!

	// draw the drop shadow
	if (shadow) {
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		trap_R_SetColor( color );
		s = string;
		xx = x;
		cnt = 0;
		while ( *s && cnt < maxChars) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
				continue;
			}
			CG_DrawChar( xx + 2, y + 2, charWidth, charHeight, *s );
			cnt++;
			xx += charWidth;
			s++;
		}
	}

	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	trap_R_SetColor( setColor );
	while ( *s && cnt < maxChars) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				color[3] = setColor[3];
				trap_R_SetColor( color );
			}
			s += 2;
			continue;
		}
		CG_DrawChar( xx, y, charWidth, charHeight, *s );
		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor( NULL );
}

void CG_DrawBigString( int x, int y, const char *s, float alpha ) {
	float	color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	CG_DrawStringExt( x, y, s, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
}

void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color ) {
	CG_DrawStringExt( x, y, s, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
}

void CG_DrawSmallString( int x, int y, const char *s, float alpha ) {
	float	color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	CG_DrawStringExt( x, y, s, color, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
}

void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color ) {
	CG_DrawStringExt( x, y, s, color, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
}

/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen( const char *str ) {
	const char *s = str;
	int count = 0;

	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox( int x, int y, int w, int h, qhandle_t hShader ) {
	float	s1, t1, s2, t2;

	s1 = x/64.0;
	t1 = y/64.0;
	s2 = (x+w)/64.0;
	t2 = (y+h)/64.0;
	trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}



/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear( void ) {
	int		top, bottom, left, right;
	int		w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;

	if ( cg.refdef.x == 0 && cg.refdef.y == 0 &&
		cg.refdef.width == w && cg.refdef.height == h ) {
		return;		// full screen rendering
	}

	top = cg.refdef.y;
	bottom = top + cg.refdef.height-1;
	left = cg.refdef.x;
	right = left + cg.refdef.width-1;

	// clear above view screen
	CG_TileClearBox( 0, 0, w, top, cgs.media.backTileShader );

	// clear below view screen
	CG_TileClearBox( 0, bottom, w, h - bottom, cgs.media.backTileShader );

	// clear left of view screen
	CG_TileClearBox( 0, top, left, bottom - top + 1, cgs.media.backTileShader );

	// clear right of view screen
	CG_TileClearBox( right, top, w - right, bottom - top + 1, cgs.media.backTileShader );
}



/*
================
CG_FadeColor
================
*/
float *CG_FadeColor( int startMsec, int totalMsec ) {
	static vec4_t		color;
	int			t;

	if ( startMsec == 0 ) {
		return NULL;
	}

	t = cg.time - startMsec;

	if ( t >= totalMsec ) {
		return NULL;
	}

	// fade out
	if ( totalMsec - t < FADE_TIME ) {
		color[3] = ( totalMsec - t ) * 1.0/FADE_TIME;
	} else {
		color[3] = 1.0;
	}
	color[0] = color[1] = color[2] = 1;

	return color;
}


/*
================
CG_TeamColor
================
*/
float *CG_TeamColor( int team ) {
	static vec4_t	red = {1, 0.2f, 0.2f, 1};
	static vec4_t	blue = {0.2f, 0.2f, 1, 1};
	static vec4_t	other = {1, 1, 1, 1};
	static vec4_t	spectator = {0.7f, 0.7f, 0.7f, 1};

	switch ( team ) {
	case TEAM_RED:
		return red;
	case TEAM_BLUE:
		return blue;
	case TEAM_SPECTATOR:
		return spectator;
	default:
		return other;
	}
}



/*
=================
CG_GetColorForHealth
=================
*/
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor ) {
	int		count;
	int		max;

	// calculate the total points of damage that can
	// be sustained at the current health / armor level
	if ( health <= 0 ) {
		VectorClear( hcolor );	// black
		hcolor[3] = 1;
		return;
	}
	count = armor;
	max = health * ARMOR_PROTECTION / ( 1.0 - ARMOR_PROTECTION );
	if ( max < count ) {
		count = max;
	}
	health += count;

	// set the color based on health
	hcolor[0] = 1.0;
	hcolor[3] = 1.0;
	if ( health >= 100 ) {
		hcolor[2] = 1.0;
	} else if ( health < 66 ) {
		hcolor[2] = 0;
	} else {
		hcolor[2] = ( health - 66 ) / 33.0;
	}

	if ( health > 60 ) {
		hcolor[1] = 1.0;
	} else if ( health < 30 ) {
		hcolor[1] = 0;
	} else {
		hcolor[1] = ( health - 30 ) / 30.0;
	}
}

/*
=================
CG_ColorForHealth
=================
*/
void CG_ColorForHealth( vec4_t hcolor ) {

	CG_GetColorForHealth( cg.snap->ps.stats[STAT_HEALTH],
		cg.snap->ps.stats[STAT_ARMOR], hcolor );
}


/*
=================
UI_DrawProportionalString2
=================
*/
static int	propMap[256][3] = {
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 0, 16},
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 0, 16},
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 0, 16},
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 0, 16},
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 0, 16},
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 0, 16},
{0, 0, -1}, {0, 0, -1},
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 0, 12}, // musical note
{0, 0, -1}, {0, 0, -1},
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 0, 16}, // 0x0F

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 1, 16}, // heart
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 1, 16},
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 1, 16},
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 1, 16},
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 1, 16},
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 1, 16},
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 1, 16},

// cock
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 1, 16},
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 1, 16},
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * 1, 16},

{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 1, 16},	// health icon
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 1, 16},	// armor icon

{0, 0, PROP_SPACE_WIDTH},			// SPACE
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 2, 8},	// !
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 2, 16},	// "
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 2, 16},	// #
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 2, 16},	// $
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 2, 16},	// %
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 2, 16},	// &
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 2, 8},	// '
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 2, 8},	// (
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 2, 8},	// )
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 2, 16},	// *
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 2, 16},	// +
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 2, 8},	// ,
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * 2, 16},	// -
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 2, 8},	// .
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 2, 16},	// /

{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 3, 16},	// 0
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 3, 12},	// 1
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 3, 16},	// 2
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 3, 16},	// 3
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 3, 16},	// 4
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 3, 16},	// 5
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 3, 16},	// 6
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 3, 16},	// 7
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 3, 16},	// 8
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 3, 16},	// 9
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 3, 8},	// :
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 3, 8},	// ;
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 3, 16},	// <
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * 3, 16},	// =
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 3, 16},	// >
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 3, 16},	// ?

{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 4, 16},	// @
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 4, 16},	// A
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 4, 16},	// B
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 4, 16},	// C
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 4, 16},	// D
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 4, 16},	// E
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 4, 16},	// F
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 4, 16},	// G
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 4, 16},	// H
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 4, 10},	// I
{PROP_GRID_SIZE * 10 +1, PROP_GRID_SIZE * 4, 15},// J
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 4, 16},	// K
{PROP_GRID_SIZE * 12 +1, PROP_GRID_SIZE * 4, 15},// L
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 0, 18},	// M
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 4, 16},	// N
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 4, 16},	// O

{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 5, 16},	// P
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 5, 16},	// Q
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 5, 16},	// R
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 5, 16},	// S
{PROP_GRID_SIZE * 4 +1, PROP_GRID_SIZE * 5, 15},// T
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 5, 16},	// U
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 5, 16},	// V
{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 0, 20},	// W
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 5, 16},	// X
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 5, 16},	// Y
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 5, 16},	// Z
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 5, 8},	// [
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 5, 16},	// '\'
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * 5, 8},	// ]
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 5, 16},	// ^
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 5, 16},	// _

{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 6, 8},	// ` <-- not ' but `, big differents
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 6, 16},	// a
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 6, 16},	// b
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 6, 16},	// c
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 6, 16},	// d
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 6, 16},	// e
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 6, 16},	// f
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 6, 16},	// g
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 6, 16},	// h
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 6, 10},	// i
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 6, 16},	// j
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 6, 16},	// k
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 6, 10},	// l
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * 6, 16},	// m
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 6, 16},	// n
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 6, 16},	// o

{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 7, 16},	// p
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 7, 16},	// q
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 7, 16},	// r
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 7, 16},	// s
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 7, 16},	// t
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 7, 16},	// u
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 7, 16},	// v
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 7, 16},	// w
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 7, 16},	// x
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 7, 16},	// y
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 7, 16},	// z
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 7, 8},	// {
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 7, 8},	// |
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * 7, 8},	// }
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 7, 16},	// ~
//{0, 0, -1},					// DEL
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 0, 30},	// DEL (used for infinite symbol)

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

/*{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},*/

{0, 0, PROP_SPACE_WIDTH},			// 0xA0
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 10, 8},	// 0xA1
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 10, 16},	// 0xA2
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 10, 16},	// 0xA3
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 10, 14},	// 0xA4
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 10, 16},	// 0xA5
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 10, 8},	// 0xA6
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 10, 16},	// 0xA7
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 10, 10},	// 0xA8
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 10, 14},	// 0xA9
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 10, 10},	// 0xAA
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 10, 16},	// 0xAB
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 10, 16},	// 0xAC
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * 10, 16},	// 0xAD
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 10, 14},	// 0xAE
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 10, 16},	// 0xAF

{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 11, 8},	// 0xB0
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 11, 16},	// 0xB1
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 11, 10},	// 0xB2
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 11, 10},	// 0xB3
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 11, 8},	// 0xB4
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 11, 16},	// 0xB5
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 11, 16},	// 0xB6
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 11, 16},	// 0xB7
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 11, 16},	// 0xB8
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 11, 10},	// 0xB9
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 11, 10},	// 0xBA
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 11, 16},	// 0xBB
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * 11, 16},	// 0xBC
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * 11, 16},	// 0xBD
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 11, 16},	// 0xBE
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 11, 16},	// 0xBF

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}
};

/*
{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * , 16},	// 0x0
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * , 12},	// 0x1
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * , 16},	// 0x2
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * , 16},	// 0x3
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * , 16},	// 0x4
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * , 16},	// 0x5
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * , 16},	// 0x6
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * , 16},	// 0x7
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * , 16},	// 0x8
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * , 16},	// 0x9
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * , 8},	// 0xA
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * , 8},	// 0xB
{PROP_GRID_SIZE * 12, PROP_GRID_SIZE * , 16},	// 0xC
{PROP_GRID_SIZE * 13, PROP_GRID_SIZE * , 16},	// 0xD
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * , 16},	// 0xE
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * , 16},	// 0xF
*/

#define DIGIT_GAP_WIDTH		14
static int	digitMap[256][3] = {
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, DIGIT_GAP_WIDTH},			// SPACE
{0, 0, -1},	// !
{0, 0, -1},	// "
{0, 0, -1},	// #
{0, 0, -1},	// $
{0, 0, -1},	// %
{0, 0, -1},	// &
{0, 0, -1},	// '
{0, 0, -1},	// (
{0, 0, -1},	// )
{0, 0, -1},	// *
{0, 0, -1},	// +
{0, 0, -1},	// ,
{PROP_GRID_SIZE * 13 + 1, PROP_GRID_SIZE * 2, 14},	// -
{PROP_GRID_SIZE * 14 + 1, PROP_GRID_SIZE * 2, 5},	// .
{0, 0, -1},	// /

{PROP_GRID_SIZE * 0 + 1, PROP_GRID_SIZE * 3, 14},	// 0
{PROP_GRID_SIZE * 1 + 1, PROP_GRID_SIZE * 3, 14},	// 1
{PROP_GRID_SIZE * 2 + 1, PROP_GRID_SIZE * 3, 14},	// 2
{PROP_GRID_SIZE * 3 + 1, PROP_GRID_SIZE * 3, 14},	// 3
{PROP_GRID_SIZE * 4 + 1, PROP_GRID_SIZE * 3, 14},	// 4
{PROP_GRID_SIZE * 5 + 1, PROP_GRID_SIZE * 3, 14},	// 5
{PROP_GRID_SIZE * 6 + 1, PROP_GRID_SIZE * 3, 14},	// 6
{PROP_GRID_SIZE * 7 + 1, PROP_GRID_SIZE * 3, 14},	// 7
{PROP_GRID_SIZE * 8 + 1, PROP_GRID_SIZE * 3, 14},	// 8
{PROP_GRID_SIZE * 9 + 1, PROP_GRID_SIZE * 3, 14},	// 9
{PROP_GRID_SIZE * 10 + 1, PROP_GRID_SIZE * 3, 5},	// :
{0, 0, -1},	// ;
{0, 0, -1},	// <
{0, 0, -1},	// =
{0, 0, -1},	// >
{0, 0, -1},	// ?

{0, 0, -1},	// @
{PROP_GRID_SIZE * 1 + 1, PROP_GRID_SIZE * 4, 14},	// A
{PROP_GRID_SIZE * 2 + 1, PROP_GRID_SIZE * 4, 14},	// B
{PROP_GRID_SIZE * 3 + 1, PROP_GRID_SIZE * 4, 14},	// C
{PROP_GRID_SIZE * 4 + 1, PROP_GRID_SIZE * 4, 14},	// D
{PROP_GRID_SIZE * 5 + 1, PROP_GRID_SIZE * 4, 14},	// E
{PROP_GRID_SIZE * 6 + 1, PROP_GRID_SIZE * 4, 14},	// F
{0, 0, -1},	// G
{0, 0, -1},	// H
{0, 0, -1},	// I
{0, 0, -1},	// J
{0, 0, -1},	// K
{0, 0, -1},	// L
{0, 0, -1},	// M
{0, 0, -1},	// N
{0, 0, -1},	// O

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
//{0, 0, 5},			// _

{0, 0, -1},	// '
{PROP_GRID_SIZE * 1 + 1, PROP_GRID_SIZE * 4, 14},	// a
{PROP_GRID_SIZE * 2 + 1, PROP_GRID_SIZE * 4, 14},	// b
{PROP_GRID_SIZE * 3 + 1, PROP_GRID_SIZE * 4, 14},	// c
{PROP_GRID_SIZE * 4 + 1, PROP_GRID_SIZE * 4, 14},	// d
{PROP_GRID_SIZE * 5 + 1, PROP_GRID_SIZE * 4, 14},	// e
{PROP_GRID_SIZE * 6 + 1, PROP_GRID_SIZE * 4, 14},	// f
{0, 0, -1},	// g
{0, 0, -1},	// h
{0, 0, -1},	// i
{0, 0, -1},	// j
{0, 0, -1},	// k
{0, 0, -1},	// l
{0, 0, -1},	// m
{0, 0, -1},	// n
{0, 0, -1},	// o

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1},					// DEL

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}
};

static int propMapB[/*26*/37][3] = {
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 4, 16},	// A
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 4, 16},	// B
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 4, 16},	// C
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 4, 16},	// D
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 4, 16},	// E
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 4, 16},	// F
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 4, 16},	// G
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 4, 16},	// H
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 4, 10},	// I
{PROP_GRID_SIZE * 10 +1, PROP_GRID_SIZE * 4, 15},// J
{PROP_GRID_SIZE * 11, PROP_GRID_SIZE * 4, 16},	// K
{PROP_GRID_SIZE * 12 +1, PROP_GRID_SIZE * 4, 15},// L
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 0, 18},	// M
{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 4, 16},	// N
{PROP_GRID_SIZE * 15, PROP_GRID_SIZE * 4, 16},	// O
{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 5, 16},	// P
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 5, 16},	// Q
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 5, 16},	// R
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 5, 16},	// S
{PROP_GRID_SIZE * 4 +1, PROP_GRID_SIZE * 5, 15},// T
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 5, 16},	// U
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 5, 16},	// V
{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 0, 20},	// W
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 5, 16},	// X
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 5, 16},	// Y
{PROP_GRID_SIZE * 10, PROP_GRID_SIZE * 5, 16},	// Z

{PROP_GRID_SIZE * 0, PROP_GRID_SIZE * 3, 16},	// 0
{PROP_GRID_SIZE * 1, PROP_GRID_SIZE * 3, 12},	// 1
{PROP_GRID_SIZE * 2, PROP_GRID_SIZE * 3, 16},	// 2
{PROP_GRID_SIZE * 3, PROP_GRID_SIZE * 3, 16},	// 3
{PROP_GRID_SIZE * 4, PROP_GRID_SIZE * 3, 16},	// 4
{PROP_GRID_SIZE * 5, PROP_GRID_SIZE * 3, 16},	// 5
{PROP_GRID_SIZE * 6, PROP_GRID_SIZE * 3, 16},	// 6
{PROP_GRID_SIZE * 7, PROP_GRID_SIZE * 3, 16},	// 7
{PROP_GRID_SIZE * 8, PROP_GRID_SIZE * 3, 16},	// 8
{PROP_GRID_SIZE * 9, PROP_GRID_SIZE * 3, 16},	// 9

{PROP_GRID_SIZE * 14, PROP_GRID_SIZE * 2, 8}	// .
};

#define PROPB_GAP_WIDTH		0
#define PROPB_SPACE_WIDTH	8
#define PROPB_HEIGHT		16

/*
=================
UI_DrawBannerString
=================
*/
static void UI_DrawBannerString2( int x, int y, const char* str, vec4_t color )
{
	const char* s;
	unsigned char	ch;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	trap_R_SetColor( color );

	ax = x * cgs.screenXScale + cgs.screenXBias + cgs.screenXOffset;
	ay = y * cgs.screenYScale + cgs.screenYOffset;

	s = str;
	while ( *s )
	{
		ch = *s; //& 127;
		if ( ch == ' ' ) {
			ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH)* cgs.screenXScale;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			ch -= 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * cgs.screenXScale;
			ah = (float)PROPB_HEIGHT * cgs.screenYScale;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, cgs.media.charsetPropB );
			ax += (aw + (float)PROPB_GAP_WIDTH * cgs.screenXScale);
		}
		s++;
	}

	trap_R_SetColor( NULL );
}

void UI_DrawBannerString( int x, int y, const char* str, int style, vec4_t color ) {
	const char *	s;
	int				ch;
	int				width;
	vec4_t			drawcolor;

	// find the width of the drawn text
	s = str;
	width = 0;
	while ( *s ) {
		ch = *s;
		if ( ch == ' ' ) {
			width += PROPB_SPACE_WIDTH;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
		}
		s++;
	}
	width -= PROPB_GAP_WIDTH;

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			x -= width / 2;
			break;

		case UI_RIGHT:
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_DROPSHADOW ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawBannerString2( x+2, y+2, str, drawcolor );
	}

	UI_DrawBannerString2( x, y, str, color );
}


int UI_ProportionalStringWidth( const char* str ) {
	const char *	s;
	int				ch;
	int				charWidth;
	int				width;

	s = str;
	width = 0;
	while ( *s ) {
		ch = *s; // & 127;
		charWidth = propMap[ch][2];
		if ( charWidth != -1 ) {
			width += charWidth;
//			width += PROP_GAP_WIDTH;
		}
		s++;
	}

//	width -= PROP_GAP_WIDTH;
	return width;
}

// test - remove this
int UI_ProportionalCharWidth( int c ) {
	int				width;

	width = propMap[c][2];

	return width;
}

// mmp

/*
=================
UI_DrawStringFont
=================
*/
void UI_DrawStringFont( int x, int y, const char* str, vec4_t startColor,
						float scaleX, float scaleY, qhandle_t charset, int dshadow, qboolean dcolor,
						qboolean digitChar )
{
	vec4_t		color;
	vec4_t		scolor;
	const char* s;
	unsigned char	ch;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	float	fade;

	// FIXME - this can be done better
	color[0] = startColor[0];
	color[1] = startColor[1];
	color[2] = startColor[2];
	color[3] = startColor[3];

	// shadow color
	scolor[0] = scolor[1] = scolor[2] = 0.0;
	scolor[3] = startColor[3]; // if color's transparent, shadow should be transparent as well

	// draw the colored text
	trap_R_SetColor( color );

	ax = x * cgs.screenXScale + cgs.screenXBias + cgs.screenXOffset;
	ay = y * cgs.screenYScale + cgs.screenYOffset;

	s = str;

	if (digitChar) {
		while ( *s )
		{
			ch = *s & 127;
			if ( ch == ' ' ) {
				aw = (float)DIGIT_GAP_WIDTH * cgs.screenXScale * scaleX;
			} else if ( Q_IsColorString( s ) ) {
				memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				trap_R_SetColor( color );
				s += 2;
				continue;
			} else if ( digitMap[ch][2] != -1 ) {
				fcol = (float)digitMap[ch][0] / 256.0f;
				frow = (float)digitMap[ch][1] / 256.0f;
				fwidth = (float)digitMap[ch][2] / 256.0f;
				fheight = (float)PROP_HEIGHT / 256.0f;
				aw = (float)digitMap[ch][2] * cgs.screenXScale * scaleX;
				ah = (float)PROP_HEIGHT * cgs.screenYScale * scaleY;
				if ( dshadow ) {
					trap_R_SetColor( scolor );
					trap_R_DrawStretchPic( ax+1, ay+1, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
					trap_R_SetColor( color );
				}
				trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
			} else {
				aw = 0;
			}

			ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * scaleX);
			s++;
		}
	} else {
		while ( *s )
		{
			ch = *s;// & 127;
			if ( ch == ' ' || ch == 0xA0 ) {
				aw = (float)PROP_SPACE_WIDTH * cgs.screenXScale * scaleX;
			} else if ( Q_IsColorString( s ) ) {
				if ( !dcolor ) { // shall we filter out colors
					// grab text color from color table
					memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
					//color[3] = fade;
					trap_R_SetColor( color );
				}
				s += 2;
				continue;
			} else if ( propMap[ch][2] != -1 ) {
				fcol = (float)propMap[ch][0] / 256.0f;
				frow = (float)propMap[ch][1] / 256.0f;
				fwidth = (float)propMap[ch][2] / 256.0f;
				fheight = (float)PROP_HEIGHT / 256.0f;
				aw = (float)propMap[ch][2] * cgs.screenXScale * scaleX;
				ah = (float)PROP_HEIGHT * cgs.screenYScale * scaleY;
				if ( dshadow ) {
					trap_R_SetColor( scolor );
					trap_R_DrawStretchPic( ax+1, ay+1, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
					trap_R_SetColor( color );
				}
				trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
			} else {
				aw = 0;
			}

			ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * scaleX);
			s++;
		}
	}

	trap_R_SetColor( NULL );
}


/*
=================
UI_ReturnStringWidth

Returns the width of a string, based on the proportional size of each character
=================
*/

int UI_ReturnStringWidth( const char* str, qboolean digitChar ) {
	const char 			*s;
	int				ch;
	int				charWidth;
	int				width;

	s = str;
	width = 0;
	if (digitChar) {
		while ( *s ) {
			if ( Q_IsColorString( s ) ) {
				// ignore color strings
				s+=2;
				continue;
			}
			ch = *s & 255; // & 127;
			charWidth = digitMap[ch][2];
			if ( charWidth != -1 ) {
				width += charWidth;
			}
			s++;
		}
	} else {
		while ( *s ) {
			if ( Q_IsColorString( s ) ) {
				// ignore color strings
				s+=2;
				continue;
			}
			ch = *s & 255; // & 127;
			charWidth = propMap[ch][2];
			if ( charWidth != -1 ) {
				width += charWidth;
			}
			s++;
		}
	}

	return width;
}

/*
=================
UI_ReturnStringLimit

Returns the amount of chars that can be printed, before 'widthLimit' is reached
=================
*/

int UI_ReturnStringLimit( const char* str, qboolean digitChar, int widthLimit, float scale, qboolean incCStr ) {
	const char 			*s;
	int				ch;
	int				charWidth;
	int				charLimit;

	s = str;
	charLimit = 0;
	//Com_Printf( "width: %i\n", widthLimit ); // debug
	//Com_Printf( "test str: <%s>\n", str); // debug
	/*if (digitChar) {
		while ( *s ) {
			if ( Q_IsColorString( s ) ) {
				if ( incCStr == qtrue ) {
					charLimit += 2;
				}
				// skip color strings check
				s+=2;
				continue;
			}
			ch = *s & 255; // & 127;
			charWidth = digitMap[ch][2];
			if ( charWidth != -1 ) {
				widthLimit -= (charWidth * scale);
			}
			// if the limit was reached, then return char size
			if ( widthLimit < 0 ) {
				return charLimit;
			}
			charLimit++;
			s++;
		}
	} else {*/
		while ( *s ) {
			if ( Q_IsColorString( s ) ) {
				if ( incCStr == qtrue ) {
					charLimit += 2;
				}
				// skip color strings check
				s+=2;
				continue;
			}
			ch = *s & 255; // & 127;
			charWidth = propMap[ch][2];
			if ( charWidth != -1 ) {
				widthLimit -= (charWidth * scale);
			}
			// if the limit was reached, then return char size
			if ( widthLimit < 0 ) {
				//Com_Printf( "charLimit: %i  widthLimit: %i  charWidth: %i  s: %i\n", charLimit, widthLimit, charWidth, *s & 255 ); // debug
				return charLimit;
			}
			charLimit++;
			//Com_Printf( "charLimit: %i  widthLimit: %i  charWidth: %i  s: %i\n", charLimit, widthLimit, charWidth, *s & 255 ); // debug
			s++;
	/*	}*/
	}

	return charLimit;
}

/*
=================
UI_DrawString
=================
*/
void UI_DrawString( int x, int y, const char* str, int style, float scaleX, float scaleY, vec4_t color, qboolean dcolor ) {
	vec4_t		drawcolor;
	int		width;
	float		scaleTest;

	scaleTest = ( cgs.screenXScale * scaleX + cgs.screenYScale * scaleY ) / 2;

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			if ( style & UI_DIGIT )
				x -= (UI_ReturnStringWidth( str, qtrue ) * scaleX) / 2;
			else
				x -= (UI_ReturnStringWidth( str, qfalse ) * scaleX) / 2;
			break;

		case UI_RIGHT:
			if ( style & UI_DIGIT )
				x -= UI_ReturnStringWidth( str, qtrue ) * scaleX;
			else
				x -= UI_ReturnStringWidth( str, qfalse ) * scaleX;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.5;
		drawcolor[1] = color[1] * 0.5;
		drawcolor[2] = color[2] * 0.5;
		drawcolor[3] = color[3];

		if ( style & UI_DIGIT ) {
			if ( scaleTest >= hud_hqFontThreshold.value ) {
				UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetDigitHQ,
												style & UI_DROPSHADOW, dcolor, qtrue );
			} else {
				UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetDigit,
												style & UI_DROPSHADOW, dcolor, qtrue );
			}
		} else {
			if ( scaleTest >= hud_hqFontThreshold.value ) {
				UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetPropHQ,
												style & UI_DROPSHADOW, dcolor, qfalse );
			} else {
				UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetProp,
												style & UI_DROPSHADOW, dcolor, qfalse );
			}
		}
		return;
	}

	if ( style & UI_PULSE ) {
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		/*if ( style & UI_DIGIT )
			UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetDigit,
										style & UI_DROPSHADOW, dcolor, qtrue );
		else
			UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetProp,
										style & UI_DROPSHADOW, dcolor, qfalse );*/
		if ( style & UI_DIGIT ) {
			if ( scaleTest >= hud_hqFontThreshold.value ) {
				UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetDigitHQ,
												style & UI_DROPSHADOW, dcolor, qtrue );
			} else {
				UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetDigit,
												style & UI_DROPSHADOW, dcolor, qtrue );
			}
		} else {
			if ( scaleTest >= hud_hqFontThreshold.value ) {
				UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetPropHQ,
												style & UI_DROPSHADOW, dcolor, qfalse );
			} else {
				UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetProp,
												style & UI_DROPSHADOW, dcolor, qfalse );
			}
		}

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( cg.time / PULSE_DIVISOR );
		if ( style & UI_DIGIT )
			UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetDigit,
										style & UI_DROPSHADOW, dcolor, qtrue );
		else
			UI_DrawStringFont( x, y, str, drawcolor, scaleX, scaleY, cgs.media.charsetPropGlow,
										style & UI_DROPSHADOW, dcolor, qfalse );
		return;
	}

	if ( style & UI_DIGIT ) {
		if ( scaleTest >= hud_hqFontThreshold.value ) {
			UI_DrawStringFont( x, y, str, color, scaleX, scaleY, cgs.media.charsetDigitHQ,
											style & UI_DROPSHADOW, dcolor, qtrue );
		} else {
			UI_DrawStringFont( x, y, str, color, scaleX, scaleY, cgs.media.charsetDigit,
											style & UI_DROPSHADOW, dcolor, qtrue );
		}
	} else {
		if ( scaleTest >= hud_hqFontThreshold.value ) {
			UI_DrawStringFont( x, y, str, color, scaleX, scaleY, cgs.media.charsetPropHQ,
											style & UI_DROPSHADOW, dcolor, qfalse );
		} else {
			UI_DrawStringFont( x, y, str, color, scaleX, scaleY, cgs.media.charsetProp,
											style & UI_DROPSHADOW, dcolor, qfalse );
		}
	}

}

// mmp - end

int UI_ProportionalColorStringWidth( const char* str ) {
	const char 			*s;
	int				ch;
	int				charWidth;
	int				width;

	s = str;
	width = 0;
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s+=2;
			continue;
		}
		// for some fucking reason, i have to put a '& 255', to read chars past 127... wtf?!
		ch = *s & 255; // & 127;
		charWidth = propMap[ch][2];
		if ( charWidth != -1 ) {
			width += charWidth;
//			width += PROP_GAP_WIDTH;
		}
		s++;
	}

//	width -= PROP_GAP_WIDTH;
	return width;
}

int UI_DigitStringWidth( const char* str ) {
	const char *	s;
	int				ch;
	int				charWidth;
	int				width;

	s = str;
	width = 0;
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s+=2;
			continue;
		}
		ch = *s & 127;
		charWidth = digitMap[ch][2];
		if ( charWidth != -1 ) {
			width += charWidth;
//			width += PROP_GAP_WIDTH;
		}
		s++;
	}

//	width -= PROP_GAP_WIDTH;
	return width;
}

static void UI_DrawProportionalString2( int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset )
{
	const char* s;
	unsigned char	ch;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	trap_R_SetColor( color );

	ax = x * cgs.screenXScale + cgs.screenXBias + cgs.screenXOffset;
	ay = y * cgs.screenYScale + cgs.screenYOffset;

	s = str;
	while ( *s )
	{
		ch = *s; // & 127;
		if ( ch == ' ' ) {
			aw = (float)PROP_SPACE_WIDTH * cgs.screenXScale * sizeScale;
		} else if ( propMap[ch][2] != -1 ) {
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * cgs.screenXScale * sizeScale;
			ah = (float)PROP_HEIGHT * cgs.screenYScale * sizeScale;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
		} else {
			aw = 0;
		}

		ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * sizeScale);
		s++;
	}

	trap_R_SetColor( NULL );
}

static void UI_DrawProportionalStringColor( int x, int y, const char* str, vec4_t startColor,
						float sizeScale, qhandle_t charset, int dshadow, qboolean dcolor )
{
	vec4_t		color;
	vec4_t		scolor;
	const char* s;
	unsigned char	ch;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	float	fade;

	// shadow color
	scolor[0] = scolor[1] = scolor[2] = 0.0;
	scolor[3] = 1.0;

	// FIXME - this can be done better
	color[0] = startColor[0];
	color[1] = startColor[1];
	color[2] = startColor[2];
	color[3] = startColor[3];

	// draw the colored text
	trap_R_SetColor( color );

	ax = x * cgs.screenXScale + cgs.screenXBias + cgs.screenXOffset;
	ay = y * cgs.screenYScale + cgs.screenYOffset;

	s = str;  // this isn't even used
	while ( *s )
	{
		ch = *s;// & 127;
		if ( ch == ' ' || ch == 0xA0 ) {
			aw = (float)PROP_SPACE_WIDTH * cgs.screenXScale * sizeScale;
		} else if ( Q_IsColorString( s ) ) {
			if ( !dcolor ) { // shall we filter out colors
				// grab text color from color table
				memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				//color[3] = fade;
				trap_R_SetColor( color );
			}
			s += 2;
			continue;
		} else if ( propMap[ch][2] != -1 ) {
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * cgs.screenXScale * sizeScale;
			ah = (float)PROP_HEIGHT * cgs.screenYScale * sizeScale;
			if ( dshadow ) {
				trap_R_SetColor( scolor );
				trap_R_DrawStretchPic( ax+1, ay+1, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
				trap_R_SetColor( color );
			}
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
		} else {
			aw = 0;
		}

		ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * sizeScale);
		s++;
	}

	trap_R_SetColor( NULL );
}


static void UI_DrawDigitString( int x, int y, const char* str, vec4_t startColor, float sizeScale, qhandle_t charset, int dshadow )
{
	vec4_t		color;
	vec4_t		scolor;
	const char* s;
	unsigned char	ch;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	float	fade;

	// shadow color
	scolor[0] = scolor[1] = scolor[2] = 0.0;
	scolor[3] = 1.0;

	// FIXME - this can be done better
	color[0] = startColor[0];
	color[1] = startColor[1];
	color[2] = startColor[2];
	color[3] = startColor[3];

	// draw the colored text
	trap_R_SetColor( color );

	ax = x * cgs.screenXScale + cgs.screenXBias + cgs.screenXOffset;
	ay = y * cgs.screenYScale + cgs.screenYOffset;

	s = str;
	while ( *s )
	{
		ch = *s & 127;
		if ( ch == ' ' ) {
			aw = (float)DIGIT_GAP_WIDTH * cgs.screenXScale * sizeScale;
		} else if ( Q_IsColorString( s ) ) {
			memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
//			color[3] = fade;
			trap_R_SetColor( color );
			s += 2;
			continue;
		} else if ( digitMap[ch][2] != -1 ) {
			fcol = (float)digitMap[ch][0] / 256.0f;
			frow = (float)digitMap[ch][1] / 256.0f;
			fwidth = (float)digitMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)digitMap[ch][2] * cgs.screenXScale * sizeScale;
			ah = (float)PROP_HEIGHT * cgs.screenYScale * sizeScale;
			if ( dshadow ) {
				trap_R_SetColor( scolor );
				trap_R_DrawStretchPic( ax+1, ay+1, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
				trap_R_SetColor( color );
			}
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
		} else {
			aw = 0;
		}

		ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * sizeScale);
		s++;
	}

	trap_R_SetColor( NULL );
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float UI_ProportionalSizeScale( int style ) {
	if(  style & UI_SMALLFONT ) {
		return 0.75;
	}

	return 1.00;
}


/*
=================
UI_DrawProportionalString
=================
*/
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color ) {
	vec4_t	drawcolor;
	int		width;
	float	sizeScale;

	sizeScale = UI_ProportionalSizeScale( style );

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			width = UI_ProportionalStringWidth( str ) * sizeScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			width = UI_ProportionalStringWidth( str ) * sizeScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_DROPSHADOW ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x+2, y+2, str, drawcolor, sizeScale, cgs.media.charsetProp );
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, cgs.media.charsetProp );
		return;
	}

	if ( style & UI_PULSE ) {
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x, y, str, color, sizeScale, cgs.media.charsetProp );

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( cg.time / PULSE_DIVISOR );
		UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, cgs.media.charsetPropGlow );
		return;
	}

	UI_DrawProportionalString2( x, y, str, color, sizeScale, cgs.media.charsetProp );
}

/*
=================
UI_DrawCustomProportionalString
=================
*/
void UI_DrawCustomProportionalString( int x, int y, const char* str, int style, float sizeScale, vec4_t color, qboolean dcolor ) {
	vec4_t		drawcolor;
	int			width;
//	int			dshadow;
	float		scaleTest;

	scaleTest = ( cgs.screenXScale * sizeScale + cgs.screenYScale * sizeScale ) / 2;

//	dshadow = style & UI_DROPSHADOW;

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			if ( style & UI_DIGIT )
				width = UI_DigitStringWidth( str ) * sizeScale;
			else
				width = UI_ProportionalColorStringWidth( str ) * sizeScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			if ( style & UI_DIGIT )
				width = UI_DigitStringWidth( str ) * sizeScale;
			else
				width = UI_ProportionalColorStringWidth( str ) * sizeScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.5;
		drawcolor[1] = color[1] * 0.5;
		drawcolor[2] = color[2] * 0.5;
		drawcolor[3] = color[3];
		/*if ( style & UI_DIGIT )
			UI_DrawDigitString( x, y, str, drawcolor, sizeScale, cgs.media.charsetDigit, style & UI_DROPSHADOW );
		else
			UI_DrawProportionalStringColor( x, y, str, drawcolor, sizeScale, cgs.media.charsetProp,
											style & UI_DROPSHADOW, dcolor );*/
		if ( style & UI_DIGIT ) {
			if ( scaleTest >= hud_hqFontThreshold.value ) {
				UI_DrawDigitString( x, y, str, drawcolor, sizeScale, cgs.media.charsetDigitHQ, style & UI_DROPSHADOW );
			} else {
				UI_DrawDigitString( x, y, str, drawcolor, sizeScale, cgs.media.charsetDigit, style & UI_DROPSHADOW );
			}
		} else {
			if ( scaleTest >= hud_hqFontThreshold.value ) {
				UI_DrawProportionalStringColor( x, y, str, drawcolor, sizeScale, cgs.media.charsetPropHQ,
													style & UI_DROPSHADOW, dcolor );
			} else {
				UI_DrawProportionalStringColor( x, y, str, drawcolor, sizeScale, cgs.media.charsetProp,
													style & UI_DROPSHADOW, dcolor );
			}
		}
		return;
	}

	if ( style & UI_PULSE ) {
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		/*if ( style & UI_DIGIT )
			UI_DrawDigitString( x, y, str, drawcolor, sizeScale, cgs.media.charsetDigit, style & UI_DROPSHADOW );
		else
			UI_DrawProportionalStringColor( x, y, str, drawcolor, sizeScale, cgs.media.charsetProp,
											style & UI_DROPSHADOW, dcolor );*/
		if ( style & UI_DIGIT ) {
			if ( scaleTest >= hud_hqFontThreshold.value ) {
				UI_DrawDigitString( x, y, str, drawcolor, sizeScale, cgs.media.charsetDigitHQ, style & UI_DROPSHADOW );
			} else {
				UI_DrawDigitString( x, y, str, drawcolor, sizeScale, cgs.media.charsetDigit, style & UI_DROPSHADOW );
			}
		} else {
			if ( scaleTest >= hud_hqFontThreshold.value ) {
				UI_DrawProportionalStringColor( x, y, str, drawcolor, sizeScale, cgs.media.charsetPropHQ,
													style & UI_DROPSHADOW, dcolor );
			} else {
				UI_DrawProportionalStringColor( x, y, str, drawcolor, sizeScale, cgs.media.charsetProp,
													style & UI_DROPSHADOW, dcolor );
			}
		}

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( cg.time / PULSE_DIVISOR );
		if ( style & UI_DIGIT )
			UI_DrawDigitString( x, y, str, drawcolor, sizeScale, cgs.media.charsetDigit, style & UI_DROPSHADOW );
		else
			UI_DrawProportionalStringColor( x, y, str, drawcolor, sizeScale, cgs.media.charsetPropGlow,
											style & UI_DROPSHADOW, dcolor );
		return;
	}

	if ( style & UI_DIGIT ) {
		if ( scaleTest >= hud_hqFontThreshold.value ) {
			UI_DrawDigitString( x, y, str, color, sizeScale, cgs.media.charsetDigitHQ, style & UI_DROPSHADOW );
		} else {
			UI_DrawDigitString( x, y, str, color, sizeScale, cgs.media.charsetDigit, style & UI_DROPSHADOW );
		}
	} else {
		if ( scaleTest >= hud_hqFontThreshold.value ) {
			UI_DrawProportionalStringColor( x, y, str, color, sizeScale, cgs.media.charsetPropHQ,
												style & UI_DROPSHADOW, dcolor );
		} else {
			UI_DrawProportionalStringColor( x, y, str, color, sizeScale, cgs.media.charsetProp,
												style & UI_DROPSHADOW, dcolor );
		}
	}
}

/*
=================
UI_DrawNumChar
=================
*/

#define NUMCHAR_SIZE		32

int UI_DrawNumChar( int x, int y, int size, int n, int c ) {

	int		row, col;
	int		o;
	float	frow, fcol;
	float	sizeFloat;
	float	ax, ay, aw, ah;

	ax = x;
	ay = y;
	aw = size;
	ah = size;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = c & 1;
	col = n & 15;

	frow = row*0.5;
	fcol = col*0.0625;

	trap_R_DrawStretchPic( ax, ay, aw, ah,
					   fcol, frow,
					   fcol + 0.0625, frow + 0.5,
					   cgs.media.numChar ); // cgs.media.numChar

	o = size * 0.75;

	return o;

}


/*
=================
UI_DrawNumCharInteger
=================
*/

void UI_DrawNumCharInteger( int x, int y, int size, int n, int c, qboolean leftAlign ) {

	int		l, i1, i2, s;

	i1 = n;

	if ( leftAlign == qtrue ) {
		s = size * 0.75;

		if (i1 < 0) {
			i1 = -i1;
			x += s;
		}

		i2 = i1;
		while (i2) {
			i2 = ( i2 - (i2 % 10) ) / 10;
			x += s;
		}

	} else {
		if (i1 < 0) {
			i1 = -i1;
		}
	}

	x -= size * 0.75; // alignment correction

	// draw number
	while (i1) {
		i2 = i1 % 10;
		x -= UI_DrawNumChar(x, y, size, i2, c);
		i1 = ( i1 - i2 ) / 10;
	}

	// if negative number, then draw the minus
	if (n < 0) {
		UI_DrawNumChar(x, y, size, 11, c);
	}

}


/*
=================
UI_TestNum
=================
*/

/*int UI_TestNum( void ) {

	int		row, col;
	int		o;
	float	frow, fcol;
	float	sizeFloat;
	float	ax, ay, aw, ah;
	vec4_t	color;

	ax = 16;
	ay = 16;
	aw = 32;
	ah = 32;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = 0 & 1;
	col = 2 & 15;

	frow = row*0.5;
	fcol = col*0.125;

	color[0] = 1.0f;
	color[1] = 0.0f;
	color[2] = 1.0f;
	color[3] = 1.0f;

	trap_R_SetColor( color );

	trap_R_DrawStretchPic( ax, ay, aw, ah,
					   fcol, frow,
					   fcol + 0.125, frow + 0.5,
					   cgs.media.charsetBigNumGrad ); // cgs.media.numChar

	trap_R_SetColor( NULL );

	o = 0;

	return o;

}*/

/*
=================
UI_DrawBigNum

TODO: code this better, it's garbage (use tables)
=================
*/

int UI_DrawBigNum( int x, int y, int size, int ch, vec4_t color, int noDraw ) {

	int		row, col;
	int		o;
	float	frow, fcol;
	float	sizeFloat;
	float	ax, ay, aw, ah, ac;
	int		ich;

	ax = x;
	ay = y;
	ah = size;

	if ( ch >= '0' && ch <= '9' ) {
		ich = ch - '0';
		row = ich >> 3;
		col = (ich & 7) << 1;

		if ( ich == 1 ) {
			aw = size * 0.75;
			ac = 0.09375;
			o = size * 0.75;
		} else {
			aw = size;
			ac = 0.125;
			o = size;
		}

	} else {

		switch ( ch ) {
			case ':':
				row = 1;
				col = 4;
				aw = size * 0.5;
				ac = 0.0625;
				break;

			case '.':
				row = 1;
				col = 5;
				aw = size * 0.5;
				ac = 0.0625;
				break;

			case '-':
				row = 1;
				col = 6;
				aw = size;
				ac = 0.125;
				break;

			// st
			case 's':
				row = 1;
				col = 8;
				aw = size * 0.5;
				ac = 0.0625;
				break;

			// nd
			case 'n':
				row = 1;
				col = 10;
				aw = size * 0.5;
				ac = 0.0625;
				break;

			// rd
			case 'r':
				row = 1;
				col = 12;
				aw = size * 0.5;
				ac = 0.0625;
				break;

			// th
			case 't':
				row = 1;
				col = 14;
				aw = size * 0.5;
				ac = 0.0625;
				break;

			default:
				// nothing to draw, just make a space
				o = size * .25;
				return o;
		}

		o = aw;
	}

	if ( !noDraw ) {

		CG_AdjustFrom640( &ax, &ay, &aw, &ah );

		frow = row*0.5;
		fcol = col*0.0625;

		trap_R_SetColor( color );

		trap_R_DrawStretchPic( ax, ay, aw, ah,
						fcol, frow,
						fcol + ac, frow + 0.5,
						cgs.media.charsetBigNumGrad ); // cgs.media.numChar

		trap_R_SetColor( NULL );

	}

	return o;

}

int UI_DrawBigNumString( int x, int y, int size, const char* str, vec4_t color, int align ) {
	const char 		*s;
	int				ch;
	int				charWidth;
	int				width;

	s = str;
	width = 0;
	align &= 3;

	if ( align != UI_LEFT ) {

		// get string width first
		while ( *s ) {
			ch = *s & 127;
			charWidth = UI_DrawBigNum( 0, 0, size, ch, color, 1); // just get the width of each character
			width += charWidth;
			s++;
		}

		// if not right, then it must be center
		if ( align == UI_RIGHT ) {
			x -= width;
		} else {
			x -= width * 0.5;
		}

		s = str; // reset the position

		while ( *s ) {
			ch = *s & 127;
			charWidth = UI_DrawBigNum( x, y, size, ch, color, 0);
			x += charWidth;
			s++;
		}

	} else {

		while ( *s ) {
			ch = *s & 127;
			charWidth = UI_DrawBigNum( x, y, size, ch, color, 0);
			x += charWidth;
			width += charWidth;
			s++;
		}

	}

	return width;
}






















