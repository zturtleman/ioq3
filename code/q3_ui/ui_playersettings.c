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
#include "ui_local.h"

#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"
#define ART_MODEL0			"menu/art/model_0"
#define ART_MODEL1			"menu/art/model_1"
#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"
#define ART_FX_BASE			"menu/art/fx_base"
#define ART_FX_BLUE			"menu/art/fx_blue"
#define ART_FX_CYAN			"menu/art/fx_cyan"
#define ART_FX_GREEN		"menu/art/fx_grn"
#define ART_FX_RED			"menu/art/fx_red"
#define ART_FX_TEAL			"menu/art/fx_teal"
#define ART_FX_WHITE		"menu/art/fx_white"
#define ART_FX_YELLOW		"menu/art/fx_yel"
#define ART_COLORBAR		"menu/art/colorbar"
#define ART_COLORBAR_SEL	"menu/art/colorbar_select"

#define ID_NAME			10
#define ID_HANDICAP		11
#define ID_COLOR1		12
#define ID_COLOR2		13
#define ID_COLOR3		14
#define ID_BACK			15
#define ID_MODEL		16

#define MAX_NAMELENGTH	20


typedef struct {
	menuframework_s		menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;
	menubitmap_s		player;

	menufield_s			name;
	menulist_s			handicap;
	menutext_s			colortext;
	menulist_s			color1;
	menulist_s			color2;
	menulist_s			color3;

	menubitmap_s		back;
	menubitmap_s		model;
	menubitmap_s		item_null;

	// TODO: remove these 2 items, no longer needed
	qhandle_t			fxBasePic;
	qhandle_t			fxPic[7];

	qhandle_t			colorbar;
	qhandle_t			colorbar_sel;
	playerInfo_t		playerinfo;
	int					current_fx;
	char				playerModel[MAX_QPATH];
} playersettings_t;

static playersettings_t	s_playersettings;

// TODO: remove these tables, they're are not used anymore
static int gamecodetoui[] = {4,2,3,0,5,1,6};
static int uitogamecode[] = {4,6,2,3,1,5,7};

static const char *handicap_items[] = {
	"OFF",
	"+666%",
	"+300%",
	"+275%",
	"+250%",
	"+225%",
	"+200%",
	"+175%",
	"+150%",
	"+125%",
	"+100%",
	"+90%",
	"+80%",
	"+70%",
	"+60%",
	"+50%",
	"+40%",
	"+30%",
	"+20%",
	"+10%",
	NULL
};

static int handicap_values[] = {
	0,
	666,
	300,
	275,
	250,
	225,
	200,
	175,
	150,
	125,
	100,
	90,
	80,
	70,
	60,
	50,
	40,
	30,
	20,
	10,
	0
};
/*
=================
PlayerSettings_DrawName
=================
*/
static void PlayerSettings_DrawName( void *self ) {
	menufield_s		*f;
	qboolean		focus;
	int				style;
	char			*txt;
	char			c;
	float			*color;
	int				n;
	int				basex, x, y;
	char			name[32];

	f = (menufield_s*)self;
	basex = f->generic.x;
	y = f->generic.y;
	focus = (f->generic.parent->cursor == f->generic.menuPosition);

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;
	if( focus ) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	UI_DrawProportionalString( basex, y, "Name", style, color );

	// draw the actual name
	basex += 64;
	y += PROP_HEIGHT;
	txt = f->field.buffer;
	color = g_color_table[ColorIndex(COLOR_WHITE)];
	x = basex;
	while ( (c = *txt) != 0 ) {
		if ( !focus && Q_IsColorString( txt ) ) {
			n = ColorIndex( *(txt+1) );
			if( n == 0 ) {
				n = 7;
			}
			color = g_color_table[n];
			txt += 2;
			continue;
		}
		UI_DrawChar( x, y, c, style, color );
		txt++;
		x += SMALLCHAR_WIDTH;
	}

	// draw cursor if we have focus
	if( focus ) {
		if ( trap_Key_GetOverstrikeMode() ) {
			c = 11;
		} else {
			c = 10;
		}

		style &= ~UI_PULSE;
		style |= UI_BLINK;

		UI_DrawChar( basex + f->field.cursor * SMALLCHAR_WIDTH, y, c, style, color_white );
	}

	// draw at bottom also using proportional font
	Q_strncpyz( name, f->field.buffer, sizeof(name) );
	Q_CleanStr( name );
	UI_DrawProportionalString( 320, 440, name, UI_CENTER|UI_BIGFONT, text_color_normal );
}


/*
=================
PlayerSettings_DrawHandicap
=================
*/
static void PlayerSettings_DrawHandicap( void *self ) {
	menulist_s		*item;
	qboolean		focus;
	int				style;
	float			*color;

	item = (menulist_s *)self;
	focus = (item->generic.parent->cursor == item->generic.menuPosition);

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;
	if( focus ) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	UI_DrawProportionalString( item->generic.x, item->generic.y, "Handicap", style, color );
	UI_DrawProportionalString( item->generic.x + 64, item->generic.y + PROP_HEIGHT, handicap_items[item->curvalue], style, color );
}


/*
=================
PlayerSettings_DrawEffects
=================
*/
static void PlayerSettings_DrawEffects( void *self ) {
	menulist_s		*item;
	qboolean		focus;
	int				style;
	float			*color;

	item = (menulist_s *)self;
	focus = (item->generic.parent->cursor == item->generic.menuPosition);

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;
	if( focus ) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	/*
	UI_DrawProportionalString( item->generic.x, item->generic.y, "Color", style, color );
	*/

	/*
	UI_DrawHandlePic( item->generic.x + 64, item->generic.y + PROP_HEIGHT + 8, 128, 8, s_playersettings.fxBasePic );
	UI_DrawHandlePic( item->generic.x + 64 + item->curvalue * 16 + 8, item->generic.y + PROP_HEIGHT + 6, 16, 12, s_playersettings.fxPic[item->curvalue] );
	*/

	UI_DrawHandlePic( item->generic.x + 64, item->generic.y, 128, 16, s_playersettings.colorbar );
	UI_DrawHandlePic( item->generic.x + 64 + item->curvalue * 8 - 4, item->generic.y - 8, 16, 32, s_playersettings.colorbar_sel );

}

/*
=================
PlayerSettings_DrawEffectsText
=================
*/
static void PlayerSettings_DrawEffectsText( void *self ) {
	menulist_s		*item;
	qboolean		focus;
	int				style;
	float			*color;

	item = (menulist_s *)self;
	//focus = (item->generic.parent->cursor == item->generic.menuPosition);

	style = UI_LEFT|UI_SMALLFONT;
	color = text_color_normal;

	UI_DrawProportionalString( item->generic.x, item->generic.y, "Color Highlights", style, color );
}


/*
=================
PlayerSettings_DrawPlayer
=================
*/
static void PlayerSettings_DrawPlayer( void *self ) {
	menubitmap_s	*b;
	vec3_t			viewangles;
	char			buf[MAX_QPATH];

	trap_Cvar_VariableStringBuffer( "model", buf, sizeof( buf ) );
	if ( strcmp( buf, s_playersettings.playerModel ) != 0 ) {
		UI_PlayerInfo_SetModel( &s_playersettings.playerinfo, buf );
		strcpy( s_playersettings.playerModel, buf );

		viewangles[YAW]   = 180 - 30;
		viewangles[PITCH] = 0;
		viewangles[ROLL]  = 0;
		UI_PlayerInfo_SetInfo( &s_playersettings.playerinfo, LEGS_IDLE, TORSO_STAND, viewangles, vec3_origin, WP_SHOTGUN, qfalse );
	}

	b = (menubitmap_s*) self;
	UI_DrawPlayer( b->generic.x, b->generic.y, b->width, b->height, &s_playersettings.playerinfo, uis.realtime/2 );
}


/*
=================
PlayerSettings_SaveChanges
=================
*/
static void PlayerSettings_SaveChanges( void ) {
	// name
	trap_Cvar_Set( "name", s_playersettings.name.field.buffer );

	// handicap
	if ( s_playersettings.handicap.curvalue < 0 || s_playersettings.handicap.curvalue > 19 )
		trap_Cvar_SetValue( "handicap", 0 );
	else
		trap_Cvar_SetValue( "handicap", handicap_values[s_playersettings.handicap.curvalue] );

	//trap_Cvar_SetValue( "handicap", 100 - s_playersettings.handicap.curvalue * 5 );

	// effects color
	trap_Cvar_SetValue( "color1", s_playersettings.color1.curvalue );
	trap_Cvar_SetValue( "color2", s_playersettings.color2.curvalue );
	trap_Cvar_SetValue( "color3", s_playersettings.color3.curvalue );
}


/*
=================
PlayerSettings_MenuKey
=================
*/
static sfxHandle_t PlayerSettings_MenuKey( int key ) {
	if( key == K_MOUSE2 || key == K_ESCAPE ) {
		PlayerSettings_SaveChanges();
	}
	return Menu_DefaultKey( &s_playersettings.menu, key );
}


/*
=================
PlayerSettings_SetMenuItems
=================
*/
static void PlayerSettings_SetMenuItems( void ) {
	vec3_t	viewangles;
	int		c;
	int		h;

	// name
	Q_strncpyz( s_playersettings.name.field.buffer, UI_Cvar_VariableString("name"), sizeof(s_playersettings.name.field.buffer) );

	// effects color
	c = trap_Cvar_VariableValue( "color1" );
	s_playersettings.color1.curvalue = c & 15;
	c = trap_Cvar_VariableValue( "color2" );
	s_playersettings.color2.curvalue = c & 15;
	c = trap_Cvar_VariableValue( "color3" );
	s_playersettings.color3.curvalue = c & 15;

	// model/skin
	memset( &s_playersettings.playerinfo, 0, sizeof(playerInfo_t) );

	viewangles[YAW]   = 180 - 30;
	viewangles[PITCH] = 0;
	viewangles[ROLL]  = 0;

	UI_PlayerInfo_SetModel( &s_playersettings.playerinfo, UI_Cvar_VariableString( "model" ) );
	UI_PlayerInfo_SetInfo( &s_playersettings.playerinfo, LEGS_IDLE, TORSO_STAND, viewangles, vec3_origin, WP_SHOTGUN, qfalse );

	// handicap
	/*h = Com_Clamp( 0, 666, trap_Cvar_VariableValue("handicap") );
	s_playersettings.handicap.curvalue = 20 - h / 5;*/
	s_playersettings.handicap.curvalue = 0;
}


/*
=================
PlayerSettings_MenuEvent
=================
*/
static void PlayerSettings_MenuEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_HANDICAP:
		if ( s_playersettings.handicap.curvalue < 0 || s_playersettings.handicap.curvalue > 19 )
			trap_Cvar_Set( "handicap", "0" );
		else
			trap_Cvar_Set( "handicap", va( "%i", handicap_values[s_playersettings.handicap.curvalue] ) );

		//trap_Cvar_Set( "handicap", va( "%i", 100 - 25 * s_playersettings.handicap.curvalue ) );
		break;

	case ID_COLOR1:
	case ID_COLOR2:
	case ID_COLOR3:
		UI_PlayerInfo_SetColors( &s_playersettings.playerinfo, s_playersettings.color1.curvalue,
			s_playersettings.color2.curvalue, s_playersettings.color3.curvalue );
		break;

	case ID_MODEL:
		PlayerSettings_SaveChanges();
		UI_PlayerModelMenu();
		break;

	case ID_BACK:
		PlayerSettings_SaveChanges();
		UI_PopMenu();
		break;
	}
}


/*
=================
PlayerSettings_MenuInit
=================
*/
static void PlayerSettings_MenuInit( void ) {
	int		y;

	memset(&s_playersettings,0,sizeof(playersettings_t));

	PlayerSettings_Cache();

	s_playersettings.menu.key        = PlayerSettings_MenuKey;
	s_playersettings.menu.wrapAround = qtrue;
	s_playersettings.menu.fullscreen = qtrue;

	s_playersettings.banner.generic.type  = MTYPE_BTEXT;
	s_playersettings.banner.generic.x     = 320;
	s_playersettings.banner.generic.y     = 16;
	s_playersettings.banner.string        = "PLAYER SETTINGS";
	s_playersettings.banner.color         = color_white;
	s_playersettings.banner.style         = UI_CENTER;

	s_playersettings.framel.generic.type  = MTYPE_BITMAP;
	s_playersettings.framel.generic.name  = ART_FRAMEL;
	s_playersettings.framel.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_playersettings.framel.generic.x     = 0;
	s_playersettings.framel.generic.y     = 78;
	s_playersettings.framel.width         = 256;
	s_playersettings.framel.height        = 329;

	s_playersettings.framer.generic.type  = MTYPE_BITMAP;
	s_playersettings.framer.generic.name  = ART_FRAMER;
	s_playersettings.framer.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_playersettings.framer.generic.x     = 376;
	s_playersettings.framer.generic.y     = 76;
	s_playersettings.framer.width         = 256;
	s_playersettings.framer.height        = 334;

	y = 144;
	s_playersettings.name.generic.type			= MTYPE_FIELD;
	s_playersettings.name.generic.flags			= QMF_NODEFAULTINIT;
	s_playersettings.name.generic.ownerdraw		= PlayerSettings_DrawName;
	s_playersettings.name.field.widthInChars	= MAX_NAMELENGTH;
	s_playersettings.name.field.maxchars		= MAX_NAMELENGTH;
	s_playersettings.name.generic.x				= 192;
	s_playersettings.name.generic.y				= y;
	s_playersettings.name.generic.left			= 192 - 8;
	s_playersettings.name.generic.top			= y - 8;
	s_playersettings.name.generic.right			= 192 + 200;
	s_playersettings.name.generic.bottom		= y + 2 * PROP_HEIGHT;

	y += 3 * PROP_HEIGHT;
	s_playersettings.handicap.generic.type		= MTYPE_SPINCONTROL;
	s_playersettings.handicap.generic.flags		= QMF_NODEFAULTINIT;
	s_playersettings.handicap.generic.id		= ID_HANDICAP;
	s_playersettings.handicap.generic.ownerdraw	= PlayerSettings_DrawHandicap;
	s_playersettings.handicap.generic.x			= 192;
	s_playersettings.handicap.generic.y			= y;
	s_playersettings.handicap.generic.left		= 192 - 8;
	s_playersettings.handicap.generic.top		= y - 8;
	s_playersettings.handicap.generic.right		= 192 + 200;
	s_playersettings.handicap.generic.bottom	= y + 2 * PROP_HEIGHT;
	s_playersettings.handicap.numitems			= 20;

	y += 3 * PROP_HEIGHT;

	s_playersettings.colortext.generic.type		= MTYPE_NOTHING;
	s_playersettings.colortext.generic.flags	= QMF_LEFT_JUSTIFY|QMF_MOUSEONLY|QMF_SILENT;
	s_playersettings.colortext.generic.ownerdraw	= PlayerSettings_DrawEffectsText;
	s_playersettings.colortext.generic.x		= 192;
	s_playersettings.colortext.generic.y		= y;

	//

	y += PROP_HEIGHT;
	s_playersettings.color1.generic.type		= MTYPE_SPINCONTROL;
	s_playersettings.color1.generic.flags		= QMF_NODEFAULTINIT;
	s_playersettings.color1.generic.id			= ID_COLOR1;
	s_playersettings.color1.generic.ownerdraw	= PlayerSettings_DrawEffects;
	s_playersettings.color1.generic.callback	= PlayerSettings_MenuEvent;
	s_playersettings.color1.generic.x			= 192;
	s_playersettings.color1.generic.y			= y;
	s_playersettings.color1.generic.left		= 192 - 8;
	s_playersettings.color1.generic.top		= y - 8;
	s_playersettings.color1.generic.right		= 192 + 200;
	s_playersettings.color1.generic.bottom		= y + PROP_HEIGHT;
	s_playersettings.color1.numitems			= 16;

	y += PROP_HEIGHT + 8;
	s_playersettings.color2.generic.type		= MTYPE_SPINCONTROL;
	s_playersettings.color2.generic.flags		= QMF_NODEFAULTINIT;
	s_playersettings.color2.generic.id			= ID_COLOR2;
	s_playersettings.color2.generic.ownerdraw	= PlayerSettings_DrawEffects;
	s_playersettings.color2.generic.callback	= PlayerSettings_MenuEvent;
	s_playersettings.color2.generic.x			= 192;
	s_playersettings.color2.generic.y			= y;
	s_playersettings.color2.generic.left		= 192 - 8;
	s_playersettings.color2.generic.top		= y - 8;
	s_playersettings.color2.generic.right		= 192 + 200;
	s_playersettings.color2.generic.bottom		= y + PROP_HEIGHT;
	s_playersettings.color2.numitems			= 16;

	y += PROP_HEIGHT + 8;
	s_playersettings.color3.generic.type		= MTYPE_SPINCONTROL;
	s_playersettings.color3.generic.flags		= QMF_NODEFAULTINIT;
	s_playersettings.color3.generic.id			= ID_COLOR3;
	s_playersettings.color3.generic.ownerdraw	= PlayerSettings_DrawEffects;
	s_playersettings.color3.generic.callback	= PlayerSettings_MenuEvent;
	s_playersettings.color3.generic.x			= 192;
	s_playersettings.color3.generic.y			= y;
	s_playersettings.color3.generic.left		= 192 - 8;
	s_playersettings.color3.generic.top		= y - 8;
	s_playersettings.color3.generic.right		= 192 + 200;
	s_playersettings.color3.generic.bottom		= y + PROP_HEIGHT;
	s_playersettings.color3.numitems			= 16;

	s_playersettings.model.generic.type			= MTYPE_BITMAP;
	s_playersettings.model.generic.name			= ART_MODEL0;
	s_playersettings.model.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_playersettings.model.generic.id			= ID_MODEL;
	s_playersettings.model.generic.callback		= PlayerSettings_MenuEvent;
	s_playersettings.model.generic.x			= 640;
	s_playersettings.model.generic.y			= 480-64;
	s_playersettings.model.width				= 128;
	s_playersettings.model.height				= 64;
	s_playersettings.model.focuspic				= ART_MODEL1;

	s_playersettings.player.generic.type		= MTYPE_BITMAP;
	s_playersettings.player.generic.flags		= QMF_INACTIVE;
	s_playersettings.player.generic.ownerdraw	= PlayerSettings_DrawPlayer;
	s_playersettings.player.generic.x			= 400;
	s_playersettings.player.generic.y			= -40;
	s_playersettings.player.width				= 32*10;
	s_playersettings.player.height				= 56*10;

	s_playersettings.back.generic.type			= MTYPE_BITMAP;
	s_playersettings.back.generic.name			= ART_BACK0;
	s_playersettings.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_playersettings.back.generic.id			= ID_BACK;
	s_playersettings.back.generic.callback		= PlayerSettings_MenuEvent;
	s_playersettings.back.generic.x				= 0;
	s_playersettings.back.generic.y				= 480-64;
	s_playersettings.back.width					= 128;
	s_playersettings.back.height				= 64;
	s_playersettings.back.focuspic				= ART_BACK1;

	s_playersettings.item_null.generic.type		= MTYPE_BITMAP;
	s_playersettings.item_null.generic.flags	= QMF_LEFT_JUSTIFY|QMF_MOUSEONLY|QMF_SILENT;
	s_playersettings.item_null.generic.x		= 0;
	s_playersettings.item_null.generic.y		= 0;
	s_playersettings.item_null.width			= 640;
	s_playersettings.item_null.height			= 480;

	Menu_AddItem( &s_playersettings.menu, &s_playersettings.banner );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.framel );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.framer );

	Menu_AddItem( &s_playersettings.menu, &s_playersettings.name );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.colortext );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.color1 );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.color2 );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.color3 );
	/*Menu_AddItem( &s_playersettings.menu, &s_playersettings.handicap );
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.model );*/
	Menu_AddItem( &s_playersettings.menu, &s_playersettings.back );

	Menu_AddItem( &s_playersettings.menu, &s_playersettings.player );

	Menu_AddItem( &s_playersettings.menu, &s_playersettings.item_null );

	PlayerSettings_SetMenuItems();
}


/*
=================
PlayerSettings_Cache
=================
*/
void PlayerSettings_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_MODEL0 );
	trap_R_RegisterShaderNoMip( ART_MODEL1 );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );

	s_playersettings.fxBasePic = trap_R_RegisterShaderNoMip( ART_FX_BASE );
	s_playersettings.fxPic[0] = trap_R_RegisterShaderNoMip( ART_FX_RED );
	s_playersettings.fxPic[1] = trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	s_playersettings.fxPic[2] = trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	s_playersettings.fxPic[3] = trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	s_playersettings.fxPic[4] = trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	s_playersettings.fxPic[5] = trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	s_playersettings.fxPic[6] = trap_R_RegisterShaderNoMip( ART_FX_WHITE );

	s_playersettings.colorbar = trap_R_RegisterShaderNoMip( ART_COLORBAR );
	s_playersettings.colorbar_sel = trap_R_RegisterShaderNoMip( ART_COLORBAR_SEL );
}


/*
=================
UI_PlayerSettingsMenu
=================
*/
void UI_PlayerSettingsMenu( void ) {
	PlayerSettings_MenuInit();
	UI_PushMenu( &s_playersettings.menu );
}
