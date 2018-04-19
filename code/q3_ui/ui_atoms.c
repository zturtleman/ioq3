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
/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

uiStatic_t		uis;
qboolean		m_entersound;		// after a frame, so caching won't disrupt the sound

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

/*
=================
UI_ClampCvar
=================
*/
float UI_ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*
=================
UI_StartDemoLoop
=================
*/
void UI_StartDemoLoop( void ) {
	trap_Cmd_ExecuteText( EXEC_APPEND, "d1\n" );
}

/*
=================
UI_PushMenu
=================
*/
void UI_PushMenu( menuframework_s *menu )
{
	int				i;
	menucommon_s*	item;

	// avoid stacking menus invoked by hotkeys
	for (i=0 ; i<uis.menusp ; i++)
	{
		if (uis.stack[i] == menu)
		{
			uis.menusp = i;
			break;
		}
	}

	if (i == uis.menusp)
	{
		if (uis.menusp >= MAX_MENUDEPTH)
			trap_Error("UI_PushMenu: menu stack overflow");

		uis.stack[uis.menusp++] = menu;
	}

	uis.activemenu = menu;

	// default cursor position
	menu->cursor      = 0;
	menu->cursor_prev = 0;

	m_entersound = qtrue;

	trap_Key_SetCatcher( KEYCATCH_UI );

	// force first available item to have focus
	for (i=0; i<menu->nitems; i++)
	{
		item = (menucommon_s *)menu->items[i];
		if (!(item->flags & (QMF_GRAYED|QMF_MOUSEONLY|QMF_INACTIVE)))
		{
			menu->cursor_prev = -1;
			Menu_SetCursor( menu, i );
			break;
		}
	}

	uis.firstdraw = qtrue;
}

/*
=================
UI_PopMenu
=================
*/
void UI_PopMenu (void)
{
	trap_S_StartLocalSound( menu_out_sound, CHAN_LOCAL_SOUND );

	uis.menusp--;

	if (uis.menusp < 0)
		trap_Error ("UI_PopMenu: menu stack underflow");

	if (uis.menusp) {
		uis.activemenu = uis.stack[uis.menusp-1];
		uis.firstdraw = qtrue;
	}
	else {
		UI_ForceMenuOff ();
	}
}

void UI_ForceMenuOff (void)
{
	uis.menusp     = 0;
	uis.activemenu = NULL;

	trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
	trap_Key_ClearStates();
	trap_Cvar_Set( "cl_paused", "0" );
}

/*
=================
UI_LerpColor
=================
*/
void UI_LerpColor(vec4_t a, vec4_t b, vec4_t c, float t)
{
	int i;

	// lerp and clamp each component
	for (i=0; i<4; i++)
	{
		c[i] = a[i] + t*(b[i]-a[i]);
		if (c[i] < 0)
			c[i] = 0;
		else if (c[i] > 1.0)
			c[i] = 1.0;
	}
}

/*
=================
UI_DrawProportionalString2
=================
*/
static int	propMap[256][3] = {
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, /*{0, 0, -1}, {0, 0, -1},*/

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

static int propMapB[/*26*/ 37][3] = {
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

#define PROPB_GAP_WIDTH		4
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
	
	ax = x * uis.xscale + uis.bias;
	ay = y * uis.yscale;

	s = str;
	while ( *s )
	{
		ch = *s & 127;
		if ( ch == ' ' ) {
			ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH)* uis.xscale;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			ch -= 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * uis.xscale;
			ah = (float)PROPB_HEIGHT * uis.yscale;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, uis.charsetPropB );
			ax += (aw + (float)PROPB_GAP_WIDTH * uis.xscale);
		} else if ( ch >= '0' && ch <= '9' ) {
			ch -= ('0' - 26);
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * uis.xscale;
			ah = (float)PROPB_HEIGHT * uis.yscale;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, uis.charsetPropB );
			ax += (aw + (float)PROPB_GAP_WIDTH * uis.xscale);
		} else if ( ch == '.' ) {
			ch = 36;
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * uis.xscale;
			ah = (float)PROPB_HEIGHT * uis.yscale;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, uis.charsetPropB );
			ax += (aw + (float)PROPB_GAP_WIDTH * uis.xscale);
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
		} else if ( ch >= '0' && ch <= '9' ) {
			width += propMapB[ch - ('0' - 26)][2] + PROPB_GAP_WIDTH;
		} else if ( ch >= '.' ) {
			width += propMapB[36][2] + PROPB_GAP_WIDTH;
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
		UI_DrawBannerString2( x+1, y+1, str, drawcolor );
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
		ch = *s & 127;
		charWidth = propMap[ch][2];
		if ( charWidth != -1 ) {
			width += charWidth;
			width += PROP_GAP_WIDTH;
		}
		s++;
	}

	width -= PROP_GAP_WIDTH;
	return width;
}

static void UI_DrawProportionalString2( int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset )
{
	const char* s;
	unsigned char	ch;
	float	ax;
	float	ay;
	float	aw = 0;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	trap_R_SetColor( color );
	
	ax = x * uis.xscale + uis.bias;
	ay = y * uis.yscale;

	s = str;
	while ( *s )
	{
		ch = *s & 127;
		if ( ch == ' ' ) {
			aw = (float)PROP_SPACE_WIDTH * uis.xscale * sizeScale;
		}
		else if ( propMap[ch][2] != -1 ) {
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * uis.xscale * sizeScale;
			ah = (float)PROP_HEIGHT * uis.yscale * sizeScale;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
		}

		ax += (aw + (float)PROP_GAP_WIDTH * uis.xscale * sizeScale);
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
		return PROP_SMALL_SIZE_SCALE;
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

	if( !str ) {
		return;
	}

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
		UI_DrawProportionalString2( x+2, y+2, str, drawcolor, sizeScale, uis.charsetProp );
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, uis.charsetProp );
		return;
	}

	if ( style & UI_PULSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x, y, str, color, sizeScale, uis.charsetProp );

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( uis.realtime / PULSE_DIVISOR );
		UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, uis.charsetPropGlow );
		return;
	}

	UI_DrawProportionalString2( x, y, str, color, sizeScale, uis.charsetProp );
}

/*
=================
UI_DrawProportionalString_Wrapped
=================
*/
void UI_DrawProportionalString_AutoWrapped( int x, int y, int xmax, int ystep, const char* str, int style, vec4_t color ) {
	int width;
	char *s1,*s2,*s3;
	char c_bcp;
	char buf[1024];
	float   sizeScale;

	if (!str || str[0]=='\0')
		return;
	
	sizeScale = UI_ProportionalSizeScale( style );
	
	Q_strncpyz(buf, str, sizeof(buf));
	s1 = s2 = s3 = buf;

	while (1) {
		do {
			s3++;
		} while (*s3!=' ' && *s3!='\0');
		c_bcp = *s3;
		*s3 = '\0';
		width = UI_ProportionalStringWidth(s1) * sizeScale;
		*s3 = c_bcp;
		if (width > xmax) {
			if (s1==s2)
			{
				// fuck, don't have a clean cut, we'll overflow
				s2 = s3;
			}
			*s2 = '\0';
			UI_DrawProportionalString(x, y, s1, style, color);
			y += ystep;
			if (c_bcp == '\0')
      {
        // that was the last word
        // we could start a new loop, but that wouldn't be much use
        // even if the word is too long, we would overflow it (see above)
        // so just print it now if needed
        s2++;
        if (*s2 != '\0') // if we are printing an overflowing line we have s2 == s3
          UI_DrawProportionalString(x, y, s2, style, color);
				break; 
      }
			s2++;
			s1 = s2;
			s3 = s2;
		}
		else
		{
			s2 = s3;
			if (c_bcp == '\0') // we reached the end
			{
				UI_DrawProportionalString(x, y, s1, style, color);
				break;
			}
		}
	}
}

/*
=================
UI_DrawString2
=================
*/
static void UI_DrawString2( int x, int y, const char* str, vec4_t color, int charw, int charh )
{
	const char* s;
	char	ch;
	int forceColor = qfalse; //APSFIXME;
	vec4_t	tempcolor;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;

	if (y < -charh)
		// offscreen
		return;

	// draw the colored text
	trap_R_SetColor( color );
	
	ax = x * uis.xscale + uis.bias;
	ay = y * uis.yscale;
	aw = charw * uis.xscale;
	ah = charh * uis.yscale;

	s = str;
	while ( *s )
	{
		if ( Q_IsColorString( s ) )
		{
			if ( !forceColor )
			{
				memcpy( tempcolor, g_color_table[ColorIndex(s[1])], sizeof( tempcolor ) );
				tempcolor[3] = color[3];
				trap_R_SetColor( tempcolor );
			}
			s += 2;
			continue;
		}

		ch = *s & 255;
		if (ch != ' ')
		{
			frow = (ch>>4)*0.0625;
			fcol = (ch&15)*0.0625;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol + 0.0625, frow + 0.0625, uis.charset );
		}

		ax += aw;
		s++;
	}

	trap_R_SetColor( NULL );
}

/*
=================
UI_DrawString
=================
*/
void UI_DrawString( int x, int y, const char* str, int style, vec4_t color )
{
	int		len;
	int		charw;
	int		charh;
	vec4_t	newcolor;
	vec4_t	lowlight;
	float	*drawcolor;
	vec4_t	dropcolor;

	if( !str ) {
		return;
	}

	if ((style & UI_BLINK) && ((uis.realtime/BLINK_DIVISOR) & 1))
		return;

	if (style & UI_SMALLFONT)
	{
		charw =	SMALLCHAR_WIDTH;
		charh =	SMALLCHAR_HEIGHT;
	}
	else if (style & UI_GIANTFONT)
	{
		charw =	GIANTCHAR_WIDTH;
		charh =	GIANTCHAR_HEIGHT;
	}
	else
	{
		charw =	BIGCHAR_WIDTH;
		charh =	BIGCHAR_HEIGHT;
	}

	if (style & UI_PULSE)
	{
		lowlight[0] = 0.8*color[0]; 
		lowlight[1] = 0.8*color[1];
		lowlight[2] = 0.8*color[2];
		lowlight[3] = 0.8*color[3];
		UI_LerpColor(color,lowlight,newcolor,0.5+0.5*sin(uis.realtime/PULSE_DIVISOR));
		drawcolor = newcolor;
	}	
	else
		drawcolor = color;

	switch (style & UI_FORMATMASK)
	{
		case UI_CENTER:
			// center justify at x
			len = strlen(str);
			x   = x - len*charw/2;
			break;

		case UI_RIGHT:
			// right justify at x
			len = strlen(str);
			x   = x - len*charw;
			break;

		default:
			// left justify at x
			break;
	}

	if ( style & UI_DROPSHADOW )
	{
		dropcolor[0] = dropcolor[1] = dropcolor[2] = 0;
		dropcolor[3] = drawcolor[3];
		UI_DrawString2(x+2,y+2,str,dropcolor,charw,charh);
	}

	UI_DrawString2(x,y,str,drawcolor,charw,charh);
}

/*
=================
UI_DrawChar
=================
*/
void UI_DrawChar( int x, int y, int ch, int style, vec4_t color )
{
	char	buff[2];

	buff[0] = ch;
	buff[1] = '\0';

	UI_DrawString( x, y, buff, style, color );
}

qboolean UI_IsFullscreen( void ) {
	if ( uis.activemenu && ( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
		return uis.activemenu->fullscreen;
	}

	return qfalse;
}

static void NeedCDAction( qboolean result ) {
	if ( !result ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}

static void NeedCDKeyAction( qboolean result ) {
	if ( !result ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}

void UI_SetActiveMenu( uiMenuCommand_t menu ) {
	// this should be the ONLY way the menu system is brought up
	// enusure minumum menu data is cached
	Menu_Cache();

	switch ( menu ) {
	case UIMENU_NONE:
		UI_ForceMenuOff();
		return;
	case UIMENU_MAIN:
		UI_MainMenu();
		return;
	case UIMENU_NEED_CD:
		UI_ConfirmMenu( "Insert the CD", 0, NeedCDAction );
		return;
	case UIMENU_BAD_CD_KEY:
		UI_ConfirmMenu( "Bad CD Key", 0, NeedCDKeyAction );
		return;
	case UIMENU_INGAME:
		/*
		//GRank
		UI_RankingsMenu();
		return;
		*/
		trap_Cvar_Set( "cl_paused", "1" );
		UI_InGameMenu();
		return;
		
	case UIMENU_TEAM:
	case UIMENU_POSTGAME:
	default:
#ifndef NDEBUG
	  Com_Printf("UI_SetActiveMenu: bad enum %d\n", menu );
#endif
	  break;
	}
}

/*
=================
UI_KeyEvent
=================
*/
void UI_KeyEvent( int key, int down ) {
	sfxHandle_t		s;

	if (!uis.activemenu) {
		return;
	}

	if (!down) {
		return;
	}

	if (uis.activemenu->key)
		s = uis.activemenu->key( key );
	else
		s = Menu_DefaultKey( uis.activemenu, key );

	if ((s > 0) && (s != menu_null_sound))
		trap_S_StartLocalSound( s, CHAN_LOCAL_SOUND );
}

/*
=================
UI_MouseEvent
=================
*/
void UI_MouseEvent( int dx, int dy )
{
	int				i;
	menucommon_s*	m;

	if (!uis.activemenu)
		return;

	// update mouse screen position
	uis.cursorx += dx;
	if (uis.cursorx < -uis.bias)
		uis.cursorx = -uis.bias;
	else if (uis.cursorx > SCREEN_WIDTH+uis.bias)
		uis.cursorx = SCREEN_WIDTH+uis.bias;

	uis.cursory += dy;
	if (uis.cursory < 0)
		uis.cursory = 0;
	else if (uis.cursory > SCREEN_HEIGHT)
		uis.cursory = SCREEN_HEIGHT;

	// region test the active menu items
	for (i=0; i<uis.activemenu->nitems; i++)
	{
		m = (menucommon_s*)uis.activemenu->items[i];

		if (m->flags & (QMF_GRAYED|QMF_INACTIVE))
			continue;

		if ((uis.cursorx < m->left) ||
			(uis.cursorx > m->right) ||
			(uis.cursory < m->top) ||
			(uis.cursory > m->bottom))
		{
			// cursor out of item bounds
			continue;
		}

		// set focus to item at cursor
		if (uis.activemenu->cursor != i)
		{
			Menu_SetCursor( uis.activemenu, i );
			((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor_prev]))->flags &= ~QMF_HASMOUSEFOCUS;

			if ( !(((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags & QMF_SILENT ) ) {
				trap_S_StartLocalSound( menu_move_sound, CHAN_LOCAL_SOUND );
			}
		}

		((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags |= QMF_HASMOUSEFOCUS;
		return;
	}  

	if (uis.activemenu->nitems > 0) {
		// out of any region
		((menucommon_s*)(uis.activemenu->items[uis.activemenu->cursor]))->flags &= ~QMF_HASMOUSEFOCUS;
	}
}

char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


char *UI_Cvar_VariableString( const char *var_name ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}


/*
=================
UI_Cache
=================
*/
void UI_Cache_f( void ) {
	MainMenu_Cache();
	InGame_Cache();
	ConfirmMenu_Cache();
	PlayerModel_Cache();
	PlayerSettings_Cache();
	Controls_Cache();
	Demos_Cache();
	UI_CinematicsMenu_Cache();
	Preferences_Cache();
	ServerInfo_Cache();
	SpecifyServer_Cache();
	ArenaServers_Cache();
	StartServer_Cache();
	ServerOptions_Cache();
	DriverInfo_Cache();
	GraphicsOptions_Cache();
	UI_DisplayOptionsMenu_Cache();
	UI_SoundOptionsMenu_Cache();
	UI_NetworkOptionsMenu_Cache();
	UI_SPLevelMenu_Cache();
	UI_SPSkillMenu_Cache();
	UI_SPPostgameMenu_Cache();
	TeamMain_Cache();
	UI_AddBots_Cache();
	UI_RemoveBots_Cache();
	UI_SetupMenu_Cache();
//	UI_LoadConfig_Cache();
//	UI_SaveConfigMenu_Cache();
	UI_BotSelectMenu_Cache();
	UI_CDKeyMenu_Cache();
	UI_ModsMenu_Cache();

}


/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	char	*cmd;

	uis.frametime = realTime - uis.realtime;
	uis.realtime = realTime;

	cmd = UI_Argv( 0 );

	// ensure minimum menu data is available
	Menu_Cache();

	if ( Q_stricmp (cmd, "levelselect") == 0 ) {
		UI_SPLevelMenu_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "postgame") == 0 ) {
		UI_SPPostgameMenu_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_cache") == 0 ) {
		UI_Cache_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_cinematics") == 0 ) {
		UI_CinematicsMenu_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_teamOrders") == 0 ) {
		UI_TeamOrdersMenu_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "iamacheater") == 0 ) {
		UI_SPUnlock_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "iamamonkey") == 0 ) {
		UI_SPUnlockMedals_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "ui_cdkey") == 0 ) {
		UI_CDKeyMenu_f();
		return qtrue;
	}

	return qfalse;
}

/*
=================
UI_Shutdown
=================
*/
void UI_Shutdown( void ) {
}

/*
=================
UI_Init
=================
*/
void UI_Init( void ) {
	UI_RegisterCvars();

	UI_InitGameinfo();

	// cache redundant calulations
	trap_GetGlconfig( &uis.glconfig );

	// for 640x480 virtualized screen
	uis.xscale = uis.glconfig.vidWidth * (1.0/640.0);
	uis.yscale = uis.glconfig.vidHeight * (1.0/480.0);
	if ( uis.glconfig.vidWidth * 480 > uis.glconfig.vidHeight * 640 ) {
		// wide screen
		uis.bias = 0.5 * ( uis.glconfig.vidWidth - ( uis.glconfig.vidHeight * (640.0/480.0) ) );
		uis.xscale = uis.yscale;
	}
	else {
		// no wide screen
		uis.bias = 0;
	}

	// initialize the menu system
	Menu_Cache();

	uis.activemenu = NULL;
	uis.menusp     = 0;
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
	*x = *x * uis.xscale + uis.bias;
	*y *= uis.yscale;
	*w *= uis.xscale;
	*h *= uis.yscale;
}

void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname ) {
	qhandle_t	hShader;

	hShader = trap_R_RegisterShaderNoMip( picname );
	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uis.whiteShader );

	trap_R_SetColor( NULL );
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );

	trap_R_DrawStretchPic( x, y, width, 1, 0, 0, 0, 0, uis.whiteShader );
	trap_R_DrawStretchPic( x, y, 1, height, 0, 0, 0, 0, uis.whiteShader );
	trap_R_DrawStretchPic( x, y + height - 1, width, 1, 0, 0, 0, 0, uis.whiteShader );
	trap_R_DrawStretchPic( x + width - 1, y, 1, height, 0, 0, 0, 0, uis.whiteShader );

	trap_R_SetColor( NULL );
}

void UI_SetColor( const float *rgba ) {
	trap_R_SetColor( rgba );
}

void UI_UpdateScreen( void ) {
	trap_UpdateScreen();
}

/*
=================
UI_Refresh
=================
*/
void UI_Refresh( int realtime )
{
	uis.frametime = realtime - uis.realtime;
	uis.realtime  = realtime;

	if ( !( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	UI_UpdateCvars();

	if ( uis.activemenu )
	{
		if (uis.activemenu->fullscreen)
		{
			// draw the background
			if( uis.activemenu->showlogo ) {
				UI_DrawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackShader );
			}
			else {
				UI_DrawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackNoLogoShader );
			}
		}

		if (uis.activemenu->draw)
			uis.activemenu->draw();
		else
			Menu_Draw( uis.activemenu );

		if( uis.firstdraw ) {
			UI_MouseEvent( 0, 0 );
			uis.firstdraw = qfalse;
		}
	}

	// draw cursor
	UI_SetColor( NULL );
	//UI_DrawHandlePic( uis.cursorx-16, uis.cursory-16, 32, 32, uis.cursor);
	if (ui_sharpCursor.integer) {
		trap_R_DrawStretchPic( floor( uis.cursorx * uis.xscale + uis.bias - 16 ), floor( uis.cursory * uis.yscale - 16 ),
								32, 32,
								0.0, 0.0, 1.0, 1.0, uis.cursor );
	} else {
		trap_R_DrawStretchPic( uis.cursorx * uis.xscale + uis.bias - 16, uis.cursory * uis.yscale - 16,
								32, 32,
								0.0, 0.0, 1.0, 1.0, uis.cursor );
	}

#ifndef NDEBUG
	if (uis.debug)
	{
		// cursor coordinates
		UI_DrawString( 0, 0, va("(%d,%d)",uis.cursorx,uis.cursory), UI_LEFT|UI_SMALLFONT, colorRed );
	}
#endif

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if (m_entersound)
	{
		trap_S_StartLocalSound( menu_in_sound, CHAN_LOCAL_SOUND );
		m_entersound = qfalse;
	}
}

void UI_DrawTextBox (int x, int y, int width, int lines)
{
	UI_FillRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorBlack );
	UI_DrawRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorWhite );
}

qboolean UI_CursorInRect (int x, int y, int width, int height)
{
	if (uis.cursorx < x ||
		uis.cursory < y ||
		uis.cursorx > x+width ||
		uis.cursory > y+height)
		return qfalse;

	return qtrue;
}
