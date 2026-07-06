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

/* $Id: xpaint.h,v 1.20 2007/11/11 20:25:50 kps Exp $ */

#ifndef XPAINT_H
#define XPAINT_H

#ifndef OTHER_H
#include "other.h"
#endif
#ifndef GLOBAL_H
#include "global.h"
#endif

/* how to draw a selection */
#define DRAW_EMPHASIZED		BLUE

/* The fonts used in the game */
extern XFontStruct *gameFont;
extern XFontStruct *messageFont;
extern XFontStruct *scoreListFont;
extern XFontStruct *buttonFont;
extern XFontStruct *textFont;
extern XFontStruct *talkFont;
extern XFontStruct *motdFont;

/* The name of the fonts used in the game */
#define FONT_LEN	256
extern char gameFontName[FONT_LEN];
extern char messageFontName[FONT_LEN];
extern char scoreListFontName[FONT_LEN];
extern char buttonFontName[FONT_LEN];
extern char textFontName[FONT_LEN];
extern char talkFontName[FONT_LEN];
extern char motdFontName[FONT_LEN];

#define NUM_DASHES		2
#define NUM_CDASHES		2
#define DASHES_LENGTH		12

extern char dashes[NUM_DASHES];
extern char cdashes[NUM_CDASHES];

extern Display *dpy;		/* Display of player (pointer) */
extern Display *kdpy;		/* Keyboard display */
extern int radar_exposures;	/* Is radar window exposed? */
extern bool radar_score_mapped;	/* Is the radar and score window mapped */

#define	ITEM_HUD	0	/* one color for the HUD */
#define	ITEM_PLAYFIELD	1	/* and one color for the playfield */

extern Pixmap itemBitmaps[];
extern GC gameGC, messageGC, radarGC, buttonGC;
extern GC scoreListGC, textGC, talkGC, motdGC;
extern XGCValues gcv;
extern Window topWindow, drawWindow, keyboardWindow;
extern int drawWindowXOffset;
extern Pixmap drawPixmap;	/* Drawing area pixmap */
extern Window talkWindow;	/* Talk window */
extern XColor *colors;		/* Colors */
extern int num_colors;		/* Number of colors in use */
extern Colormap colormap;	/* Private colormap */
extern int maxColors;		/* Max. number of colors to use */
extern bool gotFocus;		/* Do we have the mouse pointer */
extern unsigned long current_foreground;

XColor *Get_color(int color);

extern int ColorTextWidth(XFontStruct *font_struct, const char *string, int count);
extern void ColorDrawString(Display * display, Window w, GC gc, XFontStruct *font,
			    int x, int y, const char *str, int str_len,
			    int fg_color, int bg_color, int black_color,
			    bool colorize, bool record);

static inline int Other_name_width(other_t *other, int max_chars)
{
	Check_abbrev_nick_name(other, max_chars);
	return 2 + ColorTextWidth(gameFont,
				  other->abbrev_nick_name,
				  strlen(other->abbrev_nick_name));
}

static inline void SET_FG(unsigned long fg)
{
	if (fg != current_foreground)
		XSetForeground(dpy, gameGC, current_foreground = fg);
}

static inline void Fullcolor_set_fg(unsigned long fg)
{
	assert(fullColor);
	SET_FG(fg);
}


static inline void Set_fg_color(int color)
{
	unsigned long fg = Get_color(color)->pixel;
	SET_FG(fg);
}

static inline void Set_fg_color_xor(int color1, int color2)
{
	unsigned long fg = Get_color(color1)->pixel ^ Get_color(color2)->pixel;
	SET_FG(fg);
}

static inline unsigned long Get_color_pixel(int color)
{
	return Get_color(color)->pixel;
}

static inline unsigned long Get_color_pixel_xor(int color1, int color2)
{
	return Get_color(color1)->pixel ^ Get_color(color2)->pixel;
}

static inline void Set_gc_fg_color(Display *display, GC gc, int color)
{
	unsigned long pixel = Get_color_pixel(color);
	XSetForeground(display, gc, pixel);
}

extern void Paint_item_symbol(int type, Drawable d, GC mygc, int x, int y, int color);
extern void Paint_item(int type, Drawable d, GC mygc, int x, int y);
extern void Gui_paint_item_symbol(int type, Drawable d, GC mygc, int x, int y, int c);
extern void Gui_paint_item(int type, Drawable d, GC mygc, int x, int y);

extern void Store_xpaint_options(void);

/*
 * colors.c
 */
extern void List_visuals(void);
extern int Colors_init(void);
extern int Colors_init_bitmaps(void);
extern void Colors_free_bitmaps(void);
extern void Colors_cleanup(void);
extern void Colors_debug(void);
extern void Init_spark_colors(void);
extern void Store_color_options(void);

#endif
