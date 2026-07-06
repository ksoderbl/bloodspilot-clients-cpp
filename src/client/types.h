/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2001 by
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

/* $Id: types.h,v 1.24 2007/12/02 22:40:36 kps Exp $ */

#ifndef	TYPES_H
#define	TYPES_H

#ifndef SYSDEPS_H
#include "sysdeps.h"
#endif
#ifndef KEYS_H
#include "keys.h"
#endif
#ifndef PACK_H
#include "pack.h"
#endif
#ifndef SOCKLIB_H
#include "socklib.h"
#endif

/* Do not change - should be in const.h, but needed below. */
#define MAX_CHARS		80
#define TALK_MAX_CHARS		MAX_CHARS
#define MSG_LEN			256

/*
 * On some systems an enum is smaller than an int.
 * On others bool is already a builtin type.
 * Using preprocessor macros to circumvent both situations.
 */
#define false	0
#define true	1

#ifndef	_XPMONNT_
#define bool	int
#endif

/*
 * Windows does all its FPU work in doubles.  Using floats gives warnings
 * and causes everything to be promoted to doubles anyway...
 */
#ifndef _WINDOWS
typedef float DFLOAT;
#else
typedef double DFLOAT;
#endif

typedef struct {
	DFLOAT x, y;
} vector_t;
typedef vector_t position_t;
typedef struct {
	int x, y;
} ivec_t;
typedef ivec_t ipos_t;
typedef struct {
	int x, y, w, h;
} irec_t;

#ifdef _WINDOWS
# define strncasecmp(__s, __t, __l)	strnicmp(__s, __t, __l)
# define strcasecmp(__s, __t)	stricmp(__s, __t)
#endif

typedef struct {
	int pos;		/* Block index */
	long fuel;		/* Amount of fuel available */
	irec_t bounds;		/* Location on map */
} fuelstation_t;

typedef struct {
	int pos;		/* Block index */
	short bx, by;		/* Block x, y */
	int x, y;		/* Pixel x, y */
	short id,		/* Id of owner or -1 */
	 team;			/* Team this base belongs to */
	int type;		/* orientation */
	int appeartime;		/* For base warning */
	irec_t bounds;		/* Location on map */
} homebase_t;

typedef struct {
	int pos;		/* Block index */
	short dead_time,	/* Frames inactive */
	 dot;			/* Draw dot if inactive */
} cannontime_t;

typedef struct {
	int pos;		/* Block index */
	short dead_time;	/* Frames inactive */
	unsigned short damage;	/* Damage to target */
} target_t;

typedef struct {
	int pos;		/* Block index */
	irec_t bounds;		/* Location on map */
} checkpoint_t;

typedef struct {
	DFLOAT life_time;
	int score, x, y, count, hud_msg_len, hud_msg_width, msg_width, msg_len;
	char msg[10], hud_msg[MAX_CHARS + 10];
} score_object_t;

typedef enum msg_bms {
	BmsNone = 0,
	BmsBall,
	BmsSafe,
	BmsCover,
	BmsPop
} msg_bms_t;

typedef struct message {
	char txt[MSG_LEN];
	int len;
	double age; /* age of message in seconds */
	msg_bms_t bmsinfo;
} message_t;

/*
 * is a selection pending (in progress), done, drawn emphasized?
 */
#define SEL_NONE       (1 << 0)
#define SEL_PENDING    (1 << 1)
#define SEL_SELECTED   (1 << 2)
#define SEL_EMPHASIZED (1 << 3)

/*
 * a selection (text, string indices, state,...)
 */
typedef struct {
	/* a selection in the talk window */
	struct {
		bool state;	/* current state of the selection */
		size_t x1;	/* string indices */
		size_t x2;
		bool incl_nl;	/* include a `\n'? */
	} talk;
	/* a selection in the draw window */
	struct {
		bool state;
		int x1;		/* string indices (for TalkMsg[].txt) */
		int x2;		/* they are modified when the emphasized area */
		int y1;		/* is scrolled down by new messages coming in */
		int y2;
	} draw;
	char *txt;		/* allocated when needed */
	size_t txt_size;	/* size of txt buffer */
	size_t len;
	/* when a message `jumps' from talk window to the player messages: */
	bool keep_emphasizing;
} selection_t;

typedef struct {
	int view_width;
	int view_height;
	int spark_rand;
	int num_spark_colors;
} display_t;

typedef struct {
	int movement;
	double turnspeed;
	int id;
} pointer_move_t;

typedef struct {
	bool help;
	bool version;
	bool text;
	bool list_servers;	/* list */
	bool auto_connect;	/* join */
	char shutdown_reason[MAX_CHARS];	/* shutdown reason */
} xp_args_t;

typedef struct Connect_param {
	int contact_port, server_port, login_port;
	char nick_name[MAX_CHARS];
	char user_name[MAX_CHARS];
	char host_name[MAX_CHARS];
	char server_addr[MAX_HOST_LEN];
	char server_name[MAX_HOST_LEN];
	char disp_name[MAX_DISP_LEN];
	unsigned server_version;
	int team;
} Connect_param_t;

typedef struct {
	int width;		/* Line width, -1 means no line */
	unsigned long pixel;	/* Line "pixel" value */
	int rgb;		/* RGB values corresponding to color */
	int style;		/* 0=LineSolid, 1=LineOnOffDash, 2=LineDoubleDash */
} edge_style_t;

typedef struct {
	unsigned long pixel;	/* The "pixel" value if drawn in filled mode */
	int rgb;		/* RGB values corresponding to color */
	int texture;		/* The texture if drawn in texture mode */
	int flags;		/* Flags about this style (see draw.h) */
	int def_edge_style;	/* The default style for edges */
} polygon_style_t;

typedef struct {
	ipos_t *points;		/* points[0] is absolute, rest are relative */
	int num_points;		/* number of points */
	irec_t bounds;		/* bounding box for the polygon */
	int *edge_styles;	/* optional array of indexes to edge_styles */
	int style;		/* index to polygon_styles array */
} xp_polygon_t;

#define XP_KS_UNKNOWN (-1)

typedef int xp_keysym_t;

/*
 * We define keysyms for mouse buttons. These must be values, that
 * are not used for any keyboard keys.
 */
#define XP_KS_MOUSEBUTTON1 0x20001
#define XP_KS_MOUSEBUTTON(x) ((xp_keysym_t)(XP_KS_MOUSEBUTTON1 + (x) - 1))

typedef struct {
	xp_keysym_t keysym;
	keys_t key;
} xp_keydefs_t;

#endif
