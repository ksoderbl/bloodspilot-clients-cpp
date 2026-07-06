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

/* $Id: const.h,v 1.24 2007/12/02 22:40:36 kps Exp $ */

#ifndef CONST_H
#define CONST_H

#ifndef SYSDEPS_H
# include "sysdeps.h"
#endif
#ifndef TYPES_H
#include "types.h"
#endif
#ifndef PKTTYPES_H
#include "pkttypes.h"
#endif

/* Not everyone has PI (or M_PI defined). */
#ifndef	M_PI
#   define PI		3.14159265358979323846
#else
#   define PI		M_PI
#endif

/* Not everyone has LINE_MAX either, *sigh* */
#ifndef LINE_MAX
#   define LINE_MAX 2048
#endif

/* No comment. */
#ifndef PATH_MAX
#   define PATH_MAX	1023
#endif

/*
 * Number of directions in 'dir' variables.
 * Legal values are 0 to RES-1.
 */
#define RES		128

/*
 * Conversion functions of 'dir' to radians and inverse function of that.
 */
static inline float dir_to_angle(float dir)
{
	return (float) (dir * ((2.0f / RES) * M_PI));
}

static inline float angle_to_dir(float angle)
{
	float float_dir = angle * (1.0f / (2 * M_PI)) * RES;

	while (float_dir < 0)
		float_dir += RES;
	while (float_dir >= RES)
		float_dir -= RES;
	return float_dir;
}

static inline int angle_to_int_dir(float angle)
{
	float float_dir = angle * (1.0f / (2 * M_PI)) * RES;
	int dir;

	while (float_dir < 0)
		float_dir += RES;
	dir = (int) (float_dir + 0.5f);
	while (dir >= RES)
		dir -= RES;
	return dir;
}


#define BLOCK_SZ	35

#define TABLE_SIZE	RES

extern DFLOAT tbl_sin[];
extern DFLOAT tbl_cos[];

#if 0
  /* The way it was: one table, and always range checking. */
# define tsin(x)	(tbl_sin[MOD2(x, TABLE_SIZE)])
# define tcos(x)	(tbl_sin[MOD2((x)+TABLE_SIZE/4, TABLE_SIZE)])
#else
# if 0
   /* Range checking: find out where the table size is exceeded. */
#  define CHK2(x, m)	((MOD2(x, m) != x) ? (printf("MOD %s:%d:%s\n", __FILE__, __LINE__, #x), MOD2(x, m)) : (x))
# else
   /* No range checking. */
#  define CHK2(x, m)	(x)
# endif
  /* New table lookup with optional range checking and no extra calculations. */
# define tsin(x)	(tbl_sin[CHK2(x, TABLE_SIZE)])
# define tcos(x)	(tbl_cos[CHK2(x, TABLE_SIZE)])
#endif

#define NELEM(a)	((int)(sizeof(a) / sizeof((a)[0])))

#undef ABS
#define ABS(x)			( (x)<0 ? -(x) : (x) )
#ifndef MAX
#   define MIN(x, y)		( (x)>(y) ? (y) : (x) )
#   define MAX(x, y)		( (x)>(y) ? (x) : (y) )
#endif
#define sqr(x)			( (x)*(x) )
#define DELTA(a, b)		(((a) >= (b)) ? ((a) - (b)) : ((b) - (a)))
#define LENGTH(x, y)		( hypot( (double) (x), (double) (y) ) )
#define VECTOR_LENGTH(v)	( hypot( (double) (v).x, (double) (v).y ) )
#define QUICK_LENGTH(x,y)	( ABS(x)+ABS(y) ) /*-BA Only approx, but v. quick */
#define LIMIT(val, lo, hi)	( val=(val)>(hi)?(hi):((val)<(lo)?(lo):(val)) )

/*
 * Macros to manipulate dynamic arrays.
 */

/*
 * Macro to add one new element of a given type to a dynamic array.
 * T is the type of the element.
 * P is the pointer to the array memory.
 * N is the current number of elements in the array.
 * M is the current size of the array.
 * V is the new element to add.
 * The goal is to keep the number of malloc/realloc calls low
 * while not wasting too much memory because of over-allocation.
 */
#define STORE(T,P,N,M,V)						\
    if (N >= M && ((M <= 0)						\
	? (P = (T *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (T *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	error("No memory");						\
	exit(1);							\
    } else								\
	(P[N++] = V)
/*
 * Macro to make room in a given dynamic array for new elements.
 * P is the pointer to the array memory.
 * N is the current number of elements in the array.
 * M is the current size of the array.
 * T is the type of the elements.
 * E is the number of new elements to store in the array.
 * The goal is to keep the number of malloc/realloc calls low
 * while not wasting too much memory because of over-allocation.
 */
#define EXPAND(P,N,M,T,E)						\
    if ((N) + (E) > (M)) {						\
	if ((M) <= 0) {							\
	    M = (E) + 2;						\
	    P = (T *) malloc((M) * sizeof(T));				\
	    N = 0;							\
	} else {							\
	    M = ((M) << 1) + (E);					\
	    P = (T *) realloc(P, (M) * sizeof(T));			\
	}								\
	if (P == NULL) {						\
	    error("No memory");						\
	    N = M = 0;							\
	    return;	/* ! */						\
	}								\
    }

#define UNEXPAND(P,N,M)							\
    if ((N) < ((M) >> 2)) {						\
	free(P);							\
	M = 0;								\
    }									\
    N = 0;

#ifndef PAINT_FREE
# define PAINT_FREE	1
#endif
#if PAINT_FREE
# define RELEASE(P, N, M)					\
do {								\
	if (!(N)) ; else (free(P), (M) = 0, (N) = 0);		\
} while (0)
#else
# define RELEASE(P, N, M)	((N) = 0)
#endif

#ifndef MOD2
#  define MOD2(x, m)		( (x) & ((m) - 1) )
#endif				/* MOD2 */

/* borrowed from autobook */
#define XCALLOC(type, num) \
        ((type *) calloc ((num), sizeof(type)))
#define XMALLOC(type, num) \
        ((type *) malloc ((num) * sizeof(type)))
#define XREALLOC(type, p, num) \
        ((type *) realloc ((p), (num) * sizeof(type)))
#define XFREE(ptr) \
do { \
    if (ptr) { free(ptr);  ptr = NULL; } \
} while (0)

/* Use this to remove unused parameter warning. */
#define UNUSED_PARAM(x) x = x

/* Do NOT change these! */
#define MAX_CHECKS		26
#define OLD_MAX_CHECKS		26
#define MAX_TEAMS		10

#define EXPIRED_MINE_ID		4096	/* assume no player has this id */

#define NUM_MODBANKS		4

#define MAX_PLAYER_TURNSPEED	64.0
#define MIN_PLAYER_TURNSPEED	0.1	/* was 4 */
#define MAX_PLAYER_POWER	55.0
#define MIN_PLAYER_POWER	5.0
#define MAX_PLAYER_TURNRESISTANCE	1.0
#define MIN_PLAYER_TURNRESISTANCE	0.0

#define FUEL_SCALE_BITS         8
#define FUEL_SCALE_FACT         (1<<FUEL_SCALE_BITS)
#define MAX_STATION_FUEL	(500<<FUEL_SCALE_BITS)
#define TARGET_DAMAGE		(250<<FUEL_SCALE_BITS)
#define SELF_DESTRUCT_DELAY	150.0

/*
 * Size (pixels) of radius for legal HIT!
 * Was 14 until 4.2. Increased due to `analytical collision detection'
 * which inspects a real circle and not just a square anymore.
 */
#define SHIP_SZ		        16

#define VISIBILITY_DISTANCE	1000.0

#define BALL_RADIUS		10

#define MISSILE_LEN		15

#define TEAM_NOT_SET		0xffff
#define TEAM_NOT_SET_STR	"4095"

#define DEBRIS_TYPES		(8 * 4 * 4)

#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif
#ifndef false
#define false   0
#endif
#ifndef true
#define true    1
#endif

#ifndef _WINDOWS

#  ifdef DEBUG
#    define D(x)	x ;  fflush(stdout);
#  else
#    define D(x)
#  endif

#else				/* _WINDOWS */

#  ifdef _DEBUG
#    define DEBUG	1
#    define D(x)	x
#  else
#    define D(x)
#  endif

#endif				/* _WINDOWS */

/* paint constants begin */
#define MAX_COLORS		16	/* Max. switched colors ever */
#define MAX_DEFAULT_COLORS	(MAX_COLORS+32)	/* MAX_COLORS + 32 "color code" colors. */
#define MAX_COLOR_LEN		32	/* Max. length of a color name */

#define MIN_HUD_SIZE		90	/* Size/2 of HUD lines */
#define HUD_OFFSET		20	/* Hud line offset */
#define FUEL_GAUGE_OFFSET	6
#define HUD_FUEL_GAUGE_SIZE	(2*(MIN_HUD_SIZE-HUD_OFFSET-FUEL_GAUGE_OFFSET))

#define WARNING_DISTANCE	(VISIBILITY_DISTANCE*0.8)

#define TITLE_DELAY		500	/* Should probably change to seconds */

#define DSIZE			4	/* Size of diamond (on radar) */
/* paint constants end */

/*
 * Abstract (non-display system specific) drawing definitions.
 */
#define DRAW_NONE	0
#define DRAW_GRID	1
#define DRAW_OUTLINE	2
#define DRAW_FILLED	3
#define DRAW_TEXTURED	4

/*
 * The server supports only 4 colors, except for spark/debris, which
 * may have 8 different colors.
 */
#define NUM_COLORS	4

#define ILLEGAL_COLOR	(-2)	/* Not legal value for color option. */
#define NO_COLOR	(-1)	/* Don't draw. */

#define BLACK		0
#define WHITE	 	1
#define BLUE		2
#define RED		3
#define GREEN	 	4
#define DARK_RED	5
#define DARK_GRAY	6
#define YELLOW	 	7
#define DARK_BLUE	8
#define LIGHT_GRAY	12
#define GRAY		13
#define BROWN		15

/*
 * The minimum and maximum playing window sizes supported by the server.
 */
#define MIN_VIEW_SIZE	    384
#define MAX_VIEW_SIZE	    1024
#define DEF_VIEW_SIZE	    768

/*
 * Spark rand limits.
 */
#define MIN_SPARK_RAND	    0	/* Not display spark */
#define MAX_SPARK_RAND	    0x80	/* Always display spark */
#define DEF_SPARK_RAND	    0x55	/* 66% */

#define DSIZE		    4	/* Size of diamond (on radar) */

/*
 * Polygon style flags
 */
#define STYLE_FILLED          (1U << 0)
#define STYLE_TEXTURED        (1U << 1)
#define STYLE_INVISIBLE       (1U << 2)
#define STYLE_INVISIBLE_RADAR (1U << 3)

#define FPS		(Setup->frames_per_second)

#define PACKET_LOSS		0
#define PACKET_DROP		1
#define PACKET_DRAW		2

#define MAX_SCORE_OBJECTS	10

#define MIN_SCALEFACTOR		0.1
#define MAX_SCALEFACTOR		10.0

#define FUEL_NOTIFY_TIME	3.0
#define CONTROL_TIME		8.0

#define MAX_MSGS		15	/* Max. messages displayed ever */
#define MAX_HIST_MSGS		128

#define MIN_RECEIVE_WINDOW_SIZE		1
#define MAX_RECEIVE_WINDOW_SIZE		4
#define MAX_SUPPORTED_FPS	255
#define MAX_POINTER_MOVES 	128	/* Dir prediction hack */

#define TALK_FAST_NR_OF_MSGS		20	/* talk macros */
#define TALK_FAST_MSG_SIZE		400
#define TALK_FAST_MSG_FNLEN		100
#define TALK_FAST_START_DELIMITER	'['
#define TALK_FAST_END_DELIMITER		']'
#define TALK_FAST_MIDDLE_DELIMITER	'|'
#define TALK_FAST_SPECIAL_TALK_CHAR	'#'

#define RECORD_HOST_NAME PACKAGE_STRING

#endif
