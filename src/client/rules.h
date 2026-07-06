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

/* $Id: rules.h,v 1.3 2007/09/27 22:19:46 kps Exp $ */

#ifndef RULES_H
#define RULES_H

/*
 * Bitfield definitions for playing mode.
 */
#define LIMITED_LIVES		(1<<3)
#define TIMING			(1<<4)
#define TEAM_PLAY		(1<<8)
#define WRAP_PLAY		(1<<9)

/*
 * Possible object and player status bits.
 * Needed here because client needs them too.
 * The bits that the client needs must fit into a byte,
 * so the first 8 bitvalues are reserved for that purpose.
 */
#define PLAYING			(1L<<0)	/* Not returning to base */
#define OLD_PLAYING		PLAYING
#define PAUSE			(1L<<1)	/* Must stay below 8 */
#define OLD_PAUSE		PAUSE
#define GAME_OVER		(1L<<2)	/* Must stay below 8 */
#define OLD_GAME_OVER		GAME_OVER

#endif
