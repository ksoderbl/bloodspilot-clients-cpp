/*
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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

#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WINDOWS
#ifndef _WINSOCKAPI_
#include <winsock.h>
#endif
#endif

#ifndef DRAW_H
/* need shipshape_t */
#include "shipshape.h"
#endif
#ifndef ITEM_H
/* need NUM_ITEMS */
#include "item.h"
#endif
#ifndef OPTION_H
/* need xp_keysym_t */
#include "option.h"
#endif
#ifndef OTHER_H
#include "other.h"
#endif

#include "global.h"
#include "proto.h"
#include "frame.h"

static inline float WINSCALE_f(float x)
{
	return x * fscale;
}

static inline double WINSCALE_d(double x)
{
	return x * scale;
}

#ifdef WINSCALE
#undef WINSCALE
#endif

static inline int WINSCALE(int x)
{
	bool negative = false;
	int y, t = x;
	float f = (float)0.0;

	if (x == 0)
		return 0;

	if (t < 0)
	{
		negative = true;
		t = -t;
	}

	f = WINSCALE_f(t);
	y = (int)(f + (float)0.5);

	if (y < 1)
		y = 1;

	if (negative)
		y = -y;

	return y;
}

#define UWINSCALE(x) ((unsigned)WINSCALE(x))

#define SCALEX(co) ((int)(WINSCALE(co) - WINSCALE(world.x)))
#define SCALEY(co) ((int)(WINSCALE(world.y + ext_view_height) - WINSCALE(co)))
#define X(co) ((int)((co) - world.x))
#define Y(co) ((int)(world.y + ext_view_height - (co)))

int init_blockmap(void);
int init_polymap(void);
int Map_edges(void);

#endif
