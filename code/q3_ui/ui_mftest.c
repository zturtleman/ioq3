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
#define ID_TEST1				1001
#define ID_TEST2				1002

typedef struct {
	menuframework_s		menu;

	menutext_s			banner;
	menubitmap_s		blines;

/*	menubitmap_s		framel;
	menubitmap_s		framer;*/

	menutext_s			back;
	menuradiobutton_s	test1;
	menuradiobutton_s	test2;

	//menubitmap_s		back;

	/*qhandle_t			crosshairShader[NUM_CROSSHAIRS];*/
} test_t;

static test_t s_test;

/*static const char *teamoverlay_names[] =
{
	"off",
	"upper right",
	"lower right",
	"lower left",
	NULL
};*/

static void Test_SetMenuItems( void ) {
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


static void Test_Event( void* ptr, int notification ) {
	if( notification != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {

		case ID_BACK:
			UI_PopMenu();
			break;

		/*case ID_SIMPLEITEMS:
			trap_Cvar_SetValue( "cg_simpleItems", s_test.simpleitems.curvalue );
			break;*/
	}
}

static void Test_MenuInit( void ) {
	int				y;

	memset( &s_test, 0 ,sizeof(test_t) );

	Test_Cache();

	s_test.menu.wrapAround = qtrue;
	s_test.menu.fullscreen = qtrue;

	s_test.banner.generic.type	= MTYPE_BTEXT;
	s_test.banner.generic.x		= 320;
	s_test.banner.generic.y		= 16;
	s_test.banner.string		= "0. TEST";
	s_test.banner.color			= color_white;
	s_test.banner.style			= UI_CENTER;

	s_test.blines.generic.type	= MTYPE_BITMAP;
	s_test.blines.generic.name	= ART_BANNER;
	s_test.blines.generic.flags	= QMF_INACTIVE;
	s_test.blines.generic.x		= 0;
	s_test.blines.generic.y		= 32;
	s_test.blines.width			= 640;
	s_test.blines.height		= 8;

/*	s_test.framel.generic.type  = MTYPE_BITMAP;
	s_test.framel.generic.name  = ART_FRAMEL;
	s_test.framel.generic.flags = QMF_INACTIVE;
	s_test.framel.generic.x	   = 0;
	s_test.framel.generic.y	   = 78;
	s_test.framel.width  	   = 256;
	s_test.framel.height  	   = 329;

	s_test.framer.generic.type  = MTYPE_BITMAP;
	s_test.framer.generic.name  = ART_FRAMER;
	s_test.framer.generic.flags = QMF_INACTIVE;
	s_test.framer.generic.x	   = 376;
	s_test.framer.generic.y	   = 76;
	s_test.framer.width  	   = 256;
	s_test.framer.height  	   = 334;*/

	y = TEST_Y_POS_START;

	s_test.back.generic.type		= MTYPE_MENUTEXT;
	//s_test.back.generic.type		= MTYPE_PTEXT;
	s_test.back.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_test.back.generic.x			= TEST_X_POS;
	s_test.back.generic.y			= y;
	s_test.back.generic.id			= ID_BACK;
	s_test.back.generic.callback	= Test_Event; 
	s_test.back.string				= "Back";
	s_test.back.color				= menu_option_color;
	s_test.back.style				= UI_CENTER|UI_SMALLFONT;
	y += BIGCHAR_HEIGHT * 2;

	s_test.test1.generic.type		= MTYPE_RADIOBUTTON;
	s_test.test1.generic.name		= "Test1:";
	s_test.test1.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_test.test1.generic.callback	= Test_Event;
	s_test.test1.generic.id			= ID_TEST1;
	s_test.test1.generic.x			= TEST_X_POS;
	s_test.test1.generic.y			= y;

	y += BIGCHAR_HEIGHT;
	s_test.test2.generic.type		= MTYPE_RADIOBUTTON;
	s_test.test2.generic.name		= "Test2:";
	s_test.test2.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_test.test2.generic.callback	= Test_Event;
	s_test.test2.generic.id			= ID_TEST2;
	s_test.test2.generic.x			= TEST_X_POS;
	s_test.test2.generic.y			= y;


	/*s_test.back.generic.type		= MTYPE_BITMAP;
	s_test.back.generic.name		= ART_BACK0;
	s_test.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_test.back.generic.callback	= Test_Event;
	s_test.back.generic.id			= ID_BACK;
	s_test.back.generic.x			= 0;
	s_test.back.generic.y			= 480-64;
	s_test.back.width				= 128;
	s_test.back.height  			= 64;
	s_test.back.focuspic			= ART_BACK1;*/

	Menu_AddItem( &s_test.menu, &s_test.banner );
	Menu_AddItem( &s_test.menu, &s_test.blines );

	/*Menu_AddItem( &s_test.menu, &s_test.framel );
	Menu_AddItem( &s_test.menu, &s_test.framer );*/

	Menu_AddItem( &s_test.menu, &s_test.back );

	Menu_AddItem( &s_test.menu, &s_test.test1 );
	Menu_AddItem( &s_test.menu, &s_test.test2 );

	//test_SetMenuItems();
}


/*
===============
Test_Cache

Add textures to be used here
===============
*/
void Test_Cache( void ) {
	int		n;

	/*trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );*/

	trap_R_RegisterShaderNoMip( ART_BANNER );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );

	/*for( n = 0; n < NUM_CROSSHAIRS; n++ ) {
		s_test.crosshairShader[n] = trap_R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a' + n ) );
	}*/
}


/*
===============
UI_TestMenu

This is called to access this menu
===============
*/
void UI_TestMenu( void ) {
	Test_MenuInit();
	UI_PushMenu( &s_test.menu );
}
