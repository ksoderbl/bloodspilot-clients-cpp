/*
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * Copyright (C) 2003 Kristian Söderblom <kps@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* $Id: xpaint.c,v 1.19 2007/11/20 12:47:45 kps Exp $ */

#include "xpclient_x11.h"
#include "proto.h"
#include "frame.h"
#include "setup.h"

/*
 * Globals.
 */
Display *dpy;			/* Display of player (pointer) */
Display *kdpy;			/* Keyboard display */
short about_page;		/* Which page is the player on? */

GC gameGC;			/* GC for the game area */
GC messageGC;			/* GC for messages in the game area */
GC buttonGC;			/* GC for the buttons */
GC scoreListGC;			/* GC for the player list */
GC textGC;			/* GC for the info text */
GC talkGC;			/* GC for the message window */
GC motdGC;			/* GC for the motd text */
XGCValues gcv;

Window topWindow;		/* Top-level window (topshell) */
Window drawWindow;		/* Main play window */
int drawWindowXOffset = 0;
Window keyboardWindow;		/* Keyboard window */

Pixmap drawPixmap;		/* Saved pixmap for the drawing */
				/* area (monochromes use this) */

Window aboutWindow;
Window about_close_b;		/* About window's close button */
Window about_next_b;		/* About window's next button */
Window about_prev_b;		/* About window's previous button */
Window keys_close_b;		/* Help window's close button */
Window talkWindow;
bool gotFocus;

#if 0
static int clockColor;		/* Clock color index */
static int scoreColor;		/* Score list color indices */
static int scoreSelfColor;	/* Score list own score color index */
static int scoreInactiveColor;	/* Score list inactive player color index */
static int scoreInactiveSelfColor;
				/* Score list inactive self color index */
static int scoreOwnTeamColor;	/* Score list own team color index */
static int scoreEnemyTeamColor;	/* Score list enemy team color index */
#endif

int Paint_init(void)
{
	if (Init_wreckage() == -1)
		return -1;

	if (Init_asteroids() == -1)
		return -1;

	if (Bitmaps_init() == -1)
		return -1;

	return 0;
}

void Paint_cleanup(void)
{
	Bitmaps_cleanup();
}

void Paint_frame(void)
{
	static int prev_damaged = 0;
	static int prev_prev_damaged = 0;

	Paint_frame_start();
	/*Paint_score_table(); */

	Set_fg_color(BLACK);

	rd.newFrame();


	if (damaged <= 0) {
		if (prev_damaged || prev_prev_damaged) {
			/* clean up ecm damage */
			Set_fg_color(BLACK);
			XFillRectangle(dpy, drawWindow, gameGC, 0, 0, draw_width, draw_height);
		}

		Paintdata_start();
		Paint_hudradar_walls();
		Paintdata_end();

		Paintdata_start();
		Paint_world();
		Paintdata_end();

		Paintdata_start();
		if (oldServer) {
			Paint_vfuel();
			Paint_vdecor();
			Paint_vcannon();
			Paint_vbase();
		}
		else
			Paint_objects();
		Paintdata_end();

		Paintdata_start();
		Paint_shots();
		Paintdata_end();

		Paintdata_start();
		Paint_ships();
		Paintdata_end();

		Paintdata_start();
		Paint_meters();
		Paintdata_end();

		Paintdata_start();
		Paint_hudradar();
		Paintdata_end();

		Paintdata_start();
		Paint_HUD();
		Paintdata_end();

		Paintdata_start();
		Paint_recording();
		Paintdata_end();

		Paintdata_start();
		Paint_HUD_values();
		Paintdata_end();

		Paint_messages();
		Paint_score_list();
		/*Paint_radar(); */
		Paint_score_objects();
	}
	else {
		/* Damaged. */

		XSetFunction(dpy, gameGC, GXxor);
		Set_fg_color_xor(BLACK, BLUE);
		XFillRectangle(dpy, drawWindow, gameGC, 0, 0, draw_width, draw_height);
		XSetFunction(dpy, gameGC, GXcopy);
		Set_fg_color(BLACK);
	}
	prev_prev_damaged = prev_damaged;
	prev_damaged = damaged;

	rd.endFrame();

	/* paint radar here */

	if (dbuf_state->type == PIXMAP_COPY) {
		XCopyArea(dpy, drawPixmap, drawWindow, gameGC, 0, 0, draw_width, draw_height, 0, 0);
	}

	dbuff_switch(dbuf_state);

	if (dbuf_state->type == COLOR_SWITCH) {
		XSetPlaneMask(dpy, gameGC, dbuf_state->drawing_planes);
		XSetPlaneMask(dpy, messageGC, dbuf_state->drawing_planes);
	}

	if (!damaged) {
		/* Prepare invisible buffer for next frame by clearing. */

		/*
		 * DBE's XdbeBackground switch option is
		 * probably faster than XFillRectangle.
		 */
		if (dbuf_state->multibuffer_type != MULTIBUFFER_DBE) {
			Set_fg_color(BLACK);
			XFillRectangle(dpy, drawPixmap, gameGC, 0, 0, draw_width, draw_height);
		}
	}

	if (talking) {
		static bool toggle;
		static long last_toggled;

		if (loops >= last_toggled + FPS / 2 || loops < last_toggled) {
			toggle = (toggle == false) ? true : false;
			last_toggled = loops;
		}
		Talk_cursor(toggle);
	}

	XFlush(dpy);
}

#if 0
void ShadowDrawString(Display * display, Window w, GC gc,
		      int x, int y, const char *str, unsigned long fg, unsigned long bg)
{
	XSetForeground(display, gc, bg);
	XDrawString(display, w, gc, x + 1, y + 1, str, (int) strlen(str));
	x--;
	y--;
	XSetForeground(display, gc, fg);
	XDrawString(display, w, gc, x, y, str, (int) strlen(str));
}
#endif

void Play_beep(void)
{
	XBell(dpy, 0);
	XFlush(dpy);
}


xp_option_t xpaint_options[] = {
#if 0
	COLOR_INDEX_OPTION("clockColor",
			   1,
			   &clockColor,
			   "Which color number to use for drawing the clock.\n"
			   "The clock is displayed in the top right of the score window.\n"),

	COLOR_INDEX_OPTION("scoreColor",
			   1,
			   &scoreColor,
			   "Which color number to use for drawing score list entries.\n"),

	COLOR_INDEX_OPTION("scoreSelfColor",
			   3,
			   &scoreSelfColor,
			   "Which color number to use for drawing your own score.\n"),

	COLOR_INDEX_OPTION("scoreInactiveColor",
			   12,
			   &scoreInactiveColor,
			   "Which color number to use for drawing inactive players's scores.\n"),

	COLOR_INDEX_OPTION("scoreInactiveSelfColor",
			   12,
			   &scoreInactiveSelfColor,
			   "Which color number to use for drawing your score when inactive.\n"),

	COLOR_INDEX_OPTION("scoreOwnTeamColor",
			   4,
			   &scoreOwnTeamColor,
			   "Which color number to use for drawing your own team score.\n"),

	COLOR_INDEX_OPTION("scoreEnemyTeamColor",
			   11,
			   &scoreEnemyTeamColor,
			   "Which color number to use for drawing enemy team score.\n"),
#endif
};

void Store_xpaint_options(void)
{
	STORE_OPTIONS(xpaint_options);
}
