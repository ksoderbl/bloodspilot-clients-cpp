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

#ifndef PAINTDATA_H
#define PAINTDATA_H

/* need bool */
#ifndef	TYPES_H
#include "types.h"
#endif

extern int Paintdata_init(void);
extern void Paintdata_cleanup(void);
extern void Paintdata_start(void);
extern void Paintdata_end(void);

extern int Rectangle_add(int color, int x, int y, int width, int height);
extern int Rectangle_add_unscaled(int color, int x, int y, int width, int height, bool record);

extern int Arc_add(int color, int x, int y, int width, int height, int angle1, int angle2);
extern int Arc_add_unscaled(int color, int x, int y, int width, int height, int angle1, int angle2);
extern int Segment_add_unscaled(int color, int xa, int ya, int xb, int yb, bool record);
extern int Segment_add(int color, int xa, int ya, int xb, int yb);

#endif
