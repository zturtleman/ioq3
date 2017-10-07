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
/*
=======================================================================

NETWORK OPTIONS MENU

=======================================================================
*/

#include "ui_local.h"


#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"
#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"

#define ID_GRAPHICS			10
#define ID_DISPLAY			11
#define ID_EFFECTS			12
#define ID_SOUND			13
#define ID_NETWORK			14
#define ID_BACK				15
#define ID_PU_OVERLAY		20
#define ID_ITEM_OVERLAY		21
#define ID_INTERMISSION		22


static const char *something_items[] = {
	"None",
	"5% (Very Low)",
	"10% (Low)",
	"15% (Medium)",
	"20% (High)",
	NULL
};

typedef struct {
	menuframework_s	menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;

	menutext_s			graphics;
	menutext_s			display;
	menutext_s			effects;
	menutext_s			sound;
	menutext_s			network;

	menulist_s			pu_overlay;
	menulist_s			item_overlay;
	menuradiobutton_s	intermission;

	menubitmap_s		back;
} effectsOptionsInfo_t;

static effectsOptionsInfo_t	effectsOptionsInfo;


/*
=================
UI_EffectsOptionsMenu_Event
=================
*/
static void UI_EffectsOptionsMenu_Event( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GRAPHICS:
		UI_PopMenu();
		UI_GraphicsOptionsMenu();
		break;

	case ID_DISPLAY:
		UI_PopMenu();
		UI_DisplayOptionsMenu();
		break;

	case ID_EFFECTS:
		break;

	case ID_SOUND:
		UI_PopMenu();
		UI_SoundOptionsMenu();
		break;

	case ID_NETWORK:
		UI_PopMenu();
		UI_NetworkOptionsMenu();
		break;

	case ID_PU_OVERLAY:
		if( effectsOptionsInfo.pu_overlay.curvalue == 0 ) {
			trap_Cvar_SetValue( "cg_powerupOverlay", 0.00f );
		}
		else if( effectsOptionsInfo.pu_overlay.curvalue == 1 ) {
			trap_Cvar_SetValue( "cg_powerupOverlay", 0.05f );
		}
		else if( effectsOptionsInfo.pu_overlay.curvalue == 2 ) {
			trap_Cvar_SetValue( "cg_powerupOverlay", 0.10f );
		}
		else if( effectsOptionsInfo.pu_overlay.curvalue == 3 ) {
			trap_Cvar_SetValue( "cg_powerupOverlay", 0.15f );
		}
		else if( effectsOptionsInfo.pu_overlay.curvalue == 4 ) {
			trap_Cvar_SetValue( "cg_powerupOverlay", 0.20f );
		}
		break;

	case ID_ITEM_OVERLAY:
		if( effectsOptionsInfo.item_overlay.curvalue == 0 ) {
			trap_Cvar_SetValue( "cg_itemPickUpOverlay", 0.00f );
		}
		else if( effectsOptionsInfo.item_overlay.curvalue == 1 ) {
			trap_Cvar_SetValue( "cg_itemPickUpOverlay", 0.05f );
		}
		else if( effectsOptionsInfo.item_overlay.curvalue == 2 ) {
			trap_Cvar_SetValue( "cg_itemPickUpOverlay", 0.10f );
		}
		else if( effectsOptionsInfo.item_overlay.curvalue == 3 ) {
			trap_Cvar_SetValue( "cg_itemPickUpOverlay", 0.15f );
		}
		else if( effectsOptionsInfo.item_overlay.curvalue == 4 ) {
			trap_Cvar_SetValue( "cg_itemPickUpOverlay", 0.20f );
		}
		break;

	case ID_INTERMISSION:
		trap_Cvar_SetValue( "cg_intermissionEffect", effectsOptionsInfo.intermission.curvalue );
		//Com_Printf( "^5DEBUG: %i\n", effectsOptionsInfo.intermission.curvalue ); // debug
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


/*
===============
UI_EffectsOptionsMenu_Init
===============
*/
static void UI_EffectsOptionsMenu_Init( void ) {
	int		y;
	float	fvalue;
	int		ivalue;

	memset( &effectsOptionsInfo, 0, sizeof(effectsOptionsInfo) );

	UI_EffectsOptionsMenu_Cache();
	effectsOptionsInfo.menu.wrapAround = qtrue;
	effectsOptionsInfo.menu.fullscreen = qtrue;

	effectsOptionsInfo.banner.generic.type		= MTYPE_BTEXT;
	effectsOptionsInfo.banner.generic.flags		= QMF_CENTER_JUSTIFY;
	effectsOptionsInfo.banner.generic.x			= 320;
	effectsOptionsInfo.banner.generic.y			= 16;
	effectsOptionsInfo.banner.string			= "SYSTEM SETUP";
	effectsOptionsInfo.banner.color				= color_white;
	effectsOptionsInfo.banner.style				= UI_CENTER;

	effectsOptionsInfo.framel.generic.type		= MTYPE_BITMAP;
	effectsOptionsInfo.framel.generic.name		= ART_FRAMEL;
	effectsOptionsInfo.framel.generic.flags		= QMF_INACTIVE;
	effectsOptionsInfo.framel.generic.x			= 0;
	effectsOptionsInfo.framel.generic.y			= 78;
	effectsOptionsInfo.framel.width				= 256;
	effectsOptionsInfo.framel.height			= 329;

	effectsOptionsInfo.framer.generic.type		= MTYPE_BITMAP;
	effectsOptionsInfo.framer.generic.name		= ART_FRAMER;
	effectsOptionsInfo.framer.generic.flags		= QMF_INACTIVE;
	effectsOptionsInfo.framer.generic.x			= 376;
	effectsOptionsInfo.framer.generic.y			= 76;
	effectsOptionsInfo.framer.width				= 256;
	effectsOptionsInfo.framer.height			= 334;

	y = 240 - ((BIGCHAR_HEIGHT / 2) * 5);
	effectsOptionsInfo.graphics.generic.type		= MTYPE_PTEXT;
	effectsOptionsInfo.graphics.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	effectsOptionsInfo.graphics.generic.id			= ID_GRAPHICS;
	effectsOptionsInfo.graphics.generic.callback	= UI_EffectsOptionsMenu_Event;
	effectsOptionsInfo.graphics.generic.x			= 136;
	effectsOptionsInfo.graphics.generic.y			= y;
	effectsOptionsInfo.graphics.string				= "GRAPHICS";
	effectsOptionsInfo.graphics.style				= UI_RIGHT;
	effectsOptionsInfo.graphics.color				= color_red;

	y += BIGCHAR_HEIGHT;
	effectsOptionsInfo.display.generic.type			= MTYPE_PTEXT;
	effectsOptionsInfo.display.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	effectsOptionsInfo.display.generic.id			= ID_DISPLAY;
	effectsOptionsInfo.display.generic.callback		= UI_EffectsOptionsMenu_Event;
	effectsOptionsInfo.display.generic.x			= 136;
	effectsOptionsInfo.display.generic.y			= y;
	effectsOptionsInfo.display.string				= "DISPLAY";
	effectsOptionsInfo.display.style				= UI_RIGHT;
	effectsOptionsInfo.display.color				= color_red;

	y += BIGCHAR_HEIGHT;
	effectsOptionsInfo.effects.generic.type			= MTYPE_PTEXT;
	effectsOptionsInfo.effects.generic.flags		= QMF_RIGHT_JUSTIFY;
	effectsOptionsInfo.effects.generic.id			= ID_EFFECTS;
	effectsOptionsInfo.effects.generic.callback		= UI_EffectsOptionsMenu_Event;
	effectsOptionsInfo.effects.generic.x			= 136;
	effectsOptionsInfo.effects.generic.y			= y;
	effectsOptionsInfo.effects.string				= "EFFECTS";
	effectsOptionsInfo.effects.style				= UI_RIGHT;
	effectsOptionsInfo.effects.color				= color_red;

	y += BIGCHAR_HEIGHT;
	effectsOptionsInfo.sound.generic.type			= MTYPE_PTEXT;
	effectsOptionsInfo.sound.generic.flags			= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	effectsOptionsInfo.sound.generic.id				= ID_SOUND;
	effectsOptionsInfo.sound.generic.callback		= UI_EffectsOptionsMenu_Event;
	effectsOptionsInfo.sound.generic.x				= 136;
	effectsOptionsInfo.sound.generic.y				= y;
	effectsOptionsInfo.sound.string					= "SOUND";
	effectsOptionsInfo.sound.style					= UI_RIGHT;
	effectsOptionsInfo.sound.color					= color_red;

	y += BIGCHAR_HEIGHT;
	effectsOptionsInfo.network.generic.type			= MTYPE_PTEXT;
	effectsOptionsInfo.network.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	effectsOptionsInfo.network.generic.id			= ID_NETWORK;
	effectsOptionsInfo.network.generic.callback		= UI_EffectsOptionsMenu_Event;
	effectsOptionsInfo.network.generic.x			= 136;
	effectsOptionsInfo.network.generic.y			= y;
	effectsOptionsInfo.network.string				= "NETWORK";
	effectsOptionsInfo.network.style				= UI_RIGHT;
	effectsOptionsInfo.network.color				= color_red;

	y = 240 - (((BIGCHAR_HEIGHT+2) / 2) * 3);
	effectsOptionsInfo.pu_overlay.generic.type		= MTYPE_SPINCONTROL;
	effectsOptionsInfo.pu_overlay.generic.name		= "Power-up Overlay:";
	effectsOptionsInfo.pu_overlay.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	effectsOptionsInfo.pu_overlay.generic.callback	= UI_EffectsOptionsMenu_Event;
	effectsOptionsInfo.pu_overlay.generic.id		= ID_PU_OVERLAY;
	effectsOptionsInfo.pu_overlay.generic.x			= 400;
	effectsOptionsInfo.pu_overlay.generic.y			= y;
	effectsOptionsInfo.pu_overlay.itemnames			= something_items;

	y += BIGCHAR_HEIGHT+2;
	effectsOptionsInfo.item_overlay.generic.type		= MTYPE_SPINCONTROL;
	effectsOptionsInfo.item_overlay.generic.name		= "Pick-up Flash:";
	effectsOptionsInfo.item_overlay.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	effectsOptionsInfo.item_overlay.generic.callback	= UI_EffectsOptionsMenu_Event;
	effectsOptionsInfo.item_overlay.generic.id			= ID_ITEM_OVERLAY;
	effectsOptionsInfo.item_overlay.generic.x			= 400;
	effectsOptionsInfo.item_overlay.generic.y			= y;
	effectsOptionsInfo.item_overlay.itemnames			= something_items;

	y += BIGCHAR_HEIGHT+2;
	effectsOptionsInfo.intermission.generic.type		= MTYPE_RADIOBUTTON;
	effectsOptionsInfo.intermission.generic.flags		= QMF_SMALLFONT;
	effectsOptionsInfo.intermission.generic.x			= 400;
	effectsOptionsInfo.intermission.generic.y			= y;
	effectsOptionsInfo.intermission.generic.name		= "Intermission Effect:";
	effectsOptionsInfo.intermission.generic.id			= ID_INTERMISSION;
	effectsOptionsInfo.intermission.generic.callback	= UI_EffectsOptionsMenu_Event;

	effectsOptionsInfo.back.generic.type		= MTYPE_BITMAP;
	effectsOptionsInfo.back.generic.name		= ART_BACK0;
	effectsOptionsInfo.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	effectsOptionsInfo.back.generic.callback	= UI_EffectsOptionsMenu_Event;
	effectsOptionsInfo.back.generic.id			= ID_BACK;
	effectsOptionsInfo.back.generic.x			= 0;
	effectsOptionsInfo.back.generic.y			= 480-64;
	effectsOptionsInfo.back.width				= 128;
	effectsOptionsInfo.back.height				= 64;
	effectsOptionsInfo.back.focuspic			= ART_BACK1;

	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.banner );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.framel );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.framer );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.graphics );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.display );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.effects );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.sound );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.network );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.pu_overlay );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.item_overlay );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.intermission );
	Menu_AddItem( &effectsOptionsInfo.menu, ( void * ) &effectsOptionsInfo.back );

	fvalue = trap_Cvar_VariableValue( "cg_powerupOverlay" );
	if( fvalue <= 0.00f ) {
		effectsOptionsInfo.pu_overlay.curvalue = 0;
	}
	else if( fvalue <= 0.05f ) {
		effectsOptionsInfo.pu_overlay.curvalue = 1;
	}
	else if( fvalue <= 0.10f ) {
		effectsOptionsInfo.pu_overlay.curvalue = 2;
	}
	else if( fvalue <= 0.15f ) {
		effectsOptionsInfo.pu_overlay.curvalue = 3;
	}
	else {
		effectsOptionsInfo.pu_overlay.curvalue = 4;
	}
	//effectsOptionsInfo.pu_overlay.curvalue = trap_Cvar_VariableValue( "cg_powerupOverlay") * 20;

	fvalue = trap_Cvar_VariableValue( "cg_itemPickUpOverlay" );
	if( fvalue <= 0.00f ) {
		effectsOptionsInfo.item_overlay.curvalue = 0;
	}
	else if( fvalue <= 0.05f ) {
		effectsOptionsInfo.item_overlay.curvalue = 1;
	}
	else if( fvalue <= 0.10f ) {
		effectsOptionsInfo.item_overlay.curvalue = 2;
	}
	else if( fvalue <= 0.15f ) {
		effectsOptionsInfo.item_overlay.curvalue = 3;
	}
	else {
		effectsOptionsInfo.item_overlay.curvalue = 4;
	}

	ivalue = trap_Cvar_VariableValue( "cg_intermissionEffect" );
	if ( ivalue == 1 )
		effectsOptionsInfo.intermission.curvalue = 1;
	else
		effectsOptionsInfo.intermission.curvalue = 0;
	//Com_Printf( "^5DEBUG: %i, %i\n", ivalue, effectsOptionsInfo.intermission.curvalue ); // debug

}


/*
===============
UI_EffectsOptionsMenu_Cache
===============
*/
void UI_EffectsOptionsMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
}


/*
===============
UI_EffectsOptionsMenu
===============
*/
void UI_EffectsOptionsMenu( void ) {
	UI_EffectsOptionsMenu_Init();
	UI_PushMenu( &effectsOptionsInfo.menu );
	Menu_SetCursorToItem( &effectsOptionsInfo.menu, &effectsOptionsInfo.effects );
}
