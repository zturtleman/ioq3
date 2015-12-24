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

GAME OPTIONS MENU

=======================================================================
*/


#include "ui_local.h"


/*#define ART_FRAMEL				"menu/art/frame2_l"
#define ART_FRAMER				"menu/art/frame1_r"*/

#define ART_BANNER				"menu/art/banner_lines"
#define ART_BACK0				"menu/art/back_0"
#define ART_BACK1				"menu/art/back_1"

#define TEST_X_POS				320
#define TEST_Y_POS_START		48

#define ID_BACK					1000
#define ID_TIMELIMIT			1001
#define ID_SCORELIMIT			1002

static const char *custom_timelimit[] = {
	"5",
	"10",
	"15",
	"20",
	"25",
	"30",
	"No Limit",
	NULL
};

static const char *custom_scorelimit[] = {
	"8",
	"10",
	"25",
	"50",
	"100",
	"666",
	"No Limit",
	NULL
};

typedef struct {
	menuframework_s		menu;

	menutext_s			banner;
	menubitmap_s		blines;

/*	menubitmap_s		framel;
	menubitmap_s		framer;*/

	menutext_s			back;

	menulist_s			timelimit;
	menulist_s			scorelimit;

	/*
	menuradiobutton_s	test1;
	menuradiobutton_s	test2;
	*/

	//menubitmap_s		back;

	/*qhandle_t			crosshairShader[NUM_CROSSHAIRS];*/
} test_t;

static test_t s_customRules;

/*static const char *teamoverlay_names[] =
{
	"off",
	"upper right",
	"lower right",
	"lower left",
	NULL
};*/

static void CustomRules_SetMenuItems( void ) {
	/*s_preferences.crosshair.curvalue		= (int)trap_Cvar_VariableValue( "cg_drawCrosshair" ) % NUM_CROSSHAIRS;
	s_preferences.simpleitems.curvalue		= trap_Cvar_VariableValue( "cg_simpleItems" ) != 0;
	s_preferences.brass.curvalue			= trap_Cvar_VariableValue( "cg_brassTime" ) != 0;
	s_preferences.wallmarks.curvalue		= trap_Cvar_VariableValue( "cg_marks" ) != 0;
	s_preferences.identifytarget.curvalue	= trap_Cvar_VariableValue( "cg_drawCrosshairNames" ) != 0;
	s_preferences.dynamiclights.curvalue	= trap_Cvar_VariableValue( "r_dynamiclight" ) != 0;
	s_preferences.highqualitysky.curvalue	= trap_Cvar_VariableValue ( "r_fastsky" ) == 0;
	s_preferences.synceveryframe.curvalue	= trap_Cvar_VariableValue( "r_finish" ) != 0;
	s_preferences.forcemodel.curvalue		= trap_Cvar_VariableValue( "cg_forcemodel" ) != 0;
	s_preferences.drawteamoverlay.curvalue	= Com_Clamp( 0, 3, trap_Cvar_VariableValue( "cg_drawTeamOverlay" ) );
	s_preferences.allowdownload.curvalue	= trap_Cvar_VariableValue( "cl_allowDownload" ) != 0;*/
}

/*
=================
CustomRules_Event
=================
*/
static void CustomRules_Event( void* ptr, int event ) {
	int		id;
	id = ((menucommon_s*)ptr)->id;

	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( id ) {
		case ID_BACK:
			UI_PopMenu();
			break;

		case ID_TIMELIMIT:
			break;


		/*case ID_SIMPLEITEMS:
			trap_Cvar_SetValue( "cg_simpleItems", s_customRules.simpleitems.curvalue );
			break;*/
	}

}





/*
===============
CustomRules_Cache

Add textures to be used here
===============
*/
void CustomRules_Cache( void ) {
	int		n;

	/*trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );*/

	trap_R_RegisterShaderNoMip( ART_BANNER );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );

	/*for( n = 0; n < NUM_CROSSHAIRS; n++ ) {
		s_customRules.crosshairShader[n] = trap_R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a' + n ) );
	}*/
}

/*
===============
CustomRules_MenuInit
===============
*/

static void CustomRules_MenuInit( void ) {
	int				y;

	memset( &s_customRules, 0 ,sizeof(test_t) );

	CustomRules_Cache();

	s_customRules.menu.wrapAround = qtrue;
	s_customRules.menu.fullscreen = qtrue;

	s_customRules.banner.generic.type	= MTYPE_BTEXT;
	s_customRules.banner.generic.x		= 320;
	s_customRules.banner.generic.y		= 16;
	s_customRules.banner.string		= "CUSTOM RULESET";
	s_customRules.banner.color			= color_white;
	s_customRules.banner.style			= UI_CENTER;

	s_customRules.blines.generic.type	= MTYPE_BITMAP;
	s_customRules.blines.generic.name	= ART_BANNER;
	s_customRules.blines.generic.flags	= QMF_INACTIVE;
	s_customRules.blines.generic.x		= 0;
	s_customRules.blines.generic.y		= 32;
	s_customRules.blines.width			= 640;
	s_customRules.blines.height		= 8;

/*	s_customRules.framel.generic.type  = MTYPE_BITMAP;
	s_customRules.framel.generic.name  = ART_FRAMEL;
	s_customRules.framel.generic.flags = QMF_INACTIVE;
	s_customRules.framel.generic.x	   = 0;
	s_customRules.framel.generic.y	   = 78;
	s_customRules.framel.width  	   = 256;
	s_customRules.framel.height  	   = 329;

	s_customRules.framer.generic.type  = MTYPE_BITMAP;
	s_customRules.framer.generic.name  = ART_FRAMER;
	s_customRules.framer.generic.flags = QMF_INACTIVE;
	s_customRules.framer.generic.x	   = 376;
	s_customRules.framer.generic.y	   = 76;
	s_customRules.framer.width  	   = 256;
	s_customRules.framer.height  	   = 334;*/

	y = TEST_Y_POS_START;

	s_customRules.back.generic.type = MTYPE_MENUTEXT;
	//s_customRules.back.generic.type = MTYPE_PTEXT;
	s_customRules.back.generic.flags = QMF_CENTER_JUSTIFY|QMF_SMALLFONT;
	s_customRules.back.generic.x = TEST_X_POS;
	s_customRules.back.generic.y = y;
	s_customRules.back.generic.id = ID_BACK;
	s_customRules.back.generic.callback = CustomRules_Event; 
	s_customRules.back.string = "Back";
	/*s_customRules.back.color = menu_option_color;*/
	s_customRules.back.color = color_orange;
	s_customRules.back.style = UI_CENTER|UI_SMALLFONT;
	y += BIGCHAR_HEIGHT * 2;

	s_customRules.timelimit.generic.type = MTYPE_SPINCONTROL;
	s_customRules.timelimit.generic.name = "Time Limit:";
	s_customRules.timelimit.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_customRules.timelimit.generic.callback = CustomRules_Event;
	s_customRules.timelimit.generic.id = ID_TIMELIMIT;
	s_customRules.timelimit.generic.x = 320;
	s_customRules.timelimit.generic.y = y;
	s_customRules.timelimit.itemnames = custom_timelimit;

	/*
	s_customRules.test1.generic.type		= MTYPE_RADIOBUTTON;
	s_customRules.test1.generic.name		= "Test1:";
	s_customRules.test1.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_customRules.test1.generic.callback	= CustomRules_Event;
	s_customRules.test1.generic.id			= ID_TEST1;
	s_customRules.test1.generic.x			= TEST_X_POS;
	s_customRules.test1.generic.y			= y;

	y += BIGCHAR_HEIGHT;
	s_customRules.test2.generic.type		= MTYPE_RADIOBUTTON;
	s_customRules.test2.generic.name		= "Test2:";
	s_customRules.test2.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_customRules.test2.generic.callback	= CustomRules_Event;
	s_customRules.test2.generic.id			= ID_TEST2;
	s_customRules.test2.generic.x			= TEST_X_POS;
	s_customRules.test2.generic.y			= y;
	*/

	/*s_customRules.back.generic.type		= MTYPE_BITMAP;
	s_customRules.back.generic.name		= ART_BACK0;
	s_customRules.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_customRules.back.generic.callback	= CustomRules_Event;
	s_customRules.back.generic.id			= ID_BACK;
	s_customRules.back.generic.x			= 0;
	s_customRules.back.generic.y			= 480-64;
	s_customRules.back.width				= 128;
	s_customRules.back.height  			= 64;
	s_customRules.back.focuspic			= ART_BACK1;*/

	Menu_AddItem( &s_customRules.menu, &s_customRules.banner );
	Menu_AddItem( &s_customRules.menu, &s_customRules.blines );

	/*Menu_AddItem( &s_customRules.menu, &s_customRules.framel );
	Menu_AddItem( &s_customRules.menu, &s_customRules.framer );*/

	Menu_AddItem( &s_customRules.menu, &s_customRules.back );

	Menu_AddItem( &s_customRules.menu, &s_customRules.timelimit );

	/*
	Menu_AddItem( &s_customRules.menu, &s_customRules.test1 );
	Menu_AddItem( &s_customRules.menu, &s_customRules.test2 );
	*/

	//test_SetMenuItems();
}


/*
===============
UI_CustomRulesMenu

This is called to access this menu
===============
*/
void UI_CustomRulesMenu( void ) {
	CustomRules_MenuInit();
	UI_PushMenu( &s_customRules.menu );
}
