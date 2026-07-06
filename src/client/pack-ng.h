/* 
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 2000-2004 Uoti Urpala
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

/* $Id: pack-ng.h,v 1.2 2007/08/30 22:04:17 kps Exp $ */

#ifndef	PACK_NG_H
#define	PACK_NG_H

/*
 * Polygon branch
 * 4.F.0.9: 4.3.0.0 + xp2 map format
 * 4.F.1.0: Send_player(): Additional %c (1 when sending player's own info).
 * 4.F.1.1: support for everything in 4.5.0.1
 * 4.F.1.2: Show ships about to appear on bases, new team change packet.
 * 4.F.1.3: cumulative turning
 * 4.F.1.4: balls use polygon styles
 * 4.F.1.5: Possibility to change polygon styles.
 */
#define POLYGON_VERSION		0x4F15
#define OLD_VERSION		0x4501

/*
 * Which server versions can this client join.
 */
#define MIN_SERVER_VERSION	0x4F09
#define MAX_SERVER_VERSION	MY_VERSION

/*
 * We want to keep support for servers using the old map format in the client,
 * but make incompatible changes while developing the new format. Therefore
 * there is a separate "old" range of allowed servers.
 */
#define MIN_OLD_SERVER_VERSION  0x4203
#define MAX_OLD_SERVER_VERSION  0x4501
/* Which old-style (non-polygon) protocol version we support. */
#define COMPATIBILITY_MAGIC 0x4501F4ED

#endif
