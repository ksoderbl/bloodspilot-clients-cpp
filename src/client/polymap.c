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

/* $Id: polymap.c,v 1.1 2007/11/03 18:49:25 kps Exp $ */

#include "sysdeps.h"

#include "bit.h"
#include "rules.h"
#include "error.h"
#include "proto.h"
#include "const.h"
#include "global.h"
#include "frame.h"
#include "setup.h"

/* Get signed short and advance ptr */
static int get_short(char **ptr)
{
	*ptr += 2;
	return ((signed char) *(*ptr - 2) << 8) + (unsigned char) (*(*ptr - 1));
}

/* Unsigned version */
static unsigned int get_ushort(char **ptr)
{
	*ptr += 2;
	return ((unsigned char) *(*ptr - 2) << 8) + (unsigned char) *(*ptr - 1);
}

static int get_32bit(char **ptr)
{
	int res;

	res = get_ushort(ptr) << 16;
	return res + get_ushort(ptr);
}

static void parse_styles(char **callptr)
{
	int i, num_bmaps;
	char *ptr;

	ptr = *callptr;
	num_polygon_styles = *ptr++ & 0xff;
	num_edge_styles = *ptr++ & 0xff;
	num_bmaps = *ptr++ & 0xff;

	polygon_styles = XMALLOC(polygon_style_t, MAX(1, num_polygon_styles));
	if (polygon_styles == NULL) {
		error("no memory for polygon styles");
		exit(1);
	}

	edge_styles = XMALLOC(edge_style_t, MAX(1, num_edge_styles));
	if (edge_styles == NULL) {
		error("no memory for edge styles");
		exit(1);
	}

	for (i = 0; i < num_polygon_styles; i++) {
		polygon_styles[i].rgb = get_32bit(&ptr);
		polygon_styles[i].texture = *ptr++ & 0xff;
		polygon_styles[i].def_edge_style = *ptr++ & 0xff;
		polygon_styles[i].flags = *ptr++ & 0xff;
	}

	if (num_polygon_styles == 0) {
		/* default polygon style */
		polygon_styles[0].flags = 0;
		polygon_styles[0].def_edge_style = 0;
		num_polygon_styles = 1;
	}

	for (i = 0; i < num_edge_styles; i++) {
		edge_styles[i].width = *ptr++;	/* -1 means hidden */
		edge_styles[i].rgb = get_32bit(&ptr);
		/* kps - what the **** is this ? */
		/* baron - it's line style from XSetLineAttributes */
		/* 0 = LineSolid, 1 = LineOnOffDash, 2 = LineDoubleDash */
		edge_styles[i].style = (*ptr == 1) ? 1 : (*ptr == 2) ? 2 : 0;
		ptr++;
	}

	for (i = 0; i < num_bmaps; i++) {
		char fname[30];
		int flags;

		strlcpy(fname, ptr, 30);
		ptr += strlen(fname) + 1;
		flags = *ptr++ & 0xff;
		Bitmap_add(fname, 1, flags);
	}
	*callptr = ptr;
}

int init_polymap(void)
{
	int i, j, startx, starty, ecount, edgechange, current_estyle;
	int dx, dy, cx, cy, pc;
	int *styles;
	xp_polygon_t *poly;
	ipos_t *points, min, max;
	char *ptr, *edgeptr;

	oldServer = 0;
	ptr = (char *) Setup->map_data;

	parse_styles(&ptr);

	num_polygons = get_ushort(&ptr);
	polygons = XMALLOC(xp_polygon_t, num_polygons);
	if (polygons == NULL) {
		error("no memory for polygons");
		exit(1);
	}

	for (i = 0; i < num_polygons; i++) {
		poly = &polygons[i];
		poly->style = *ptr++ & 0xff;
		current_estyle = polygon_styles[poly->style].def_edge_style;
		dx = 0;
		dy = 0;
		ecount = get_ushort(&ptr);
		edgeptr = ptr;
		if (ecount)
			edgechange = get_ushort(&edgeptr);
		else
			edgechange = INT_MAX;
		ptr += ecount * 2;
		pc = get_ushort(&ptr);
		if ((points = XMALLOC(ipos_t, pc)) == NULL) {
			error("no memory for points");
			exit(1);
		}
		if (ecount) {
			if ((styles = XMALLOC(int, pc)) == NULL) {
				error("no memory for special edges");
				exit(1);
			}
		}
		else
			styles = NULL;
		startx = get_ushort(&ptr);
		starty = get_ushort(&ptr);
		points[0].x = cx = min.x = max.x = startx;
		points[0].y = cy = min.y = max.y = starty;

		if (!edgechange) {
			current_estyle = get_ushort(&edgeptr);
			ecount--;
			if (ecount)
				edgechange = get_ushort(&edgeptr);
		}
		if (styles)
			styles[0] = current_estyle;

		for (j = 1; j < pc; j++) {
			dx = get_short(&ptr);
			dy = get_short(&ptr);
			cx += dx;
			cy += dy;
			if (min.x > cx)
				min.x = cx;
			if (min.y > cy)
				min.y = cy;
			if (max.x < cx)
				max.x = cx;
			if (max.y < cy)
				max.y = cy;
			points[j].x = dx;
			points[j].y = dy;

			if (edgechange == j) {
				current_estyle = get_ushort(&edgeptr);
				ecount--;
				if (ecount)
					edgechange = get_ushort(&edgeptr);
			}
			if (styles)
				styles[j] = current_estyle;
		}
		poly->points = points;
		poly->edge_styles = styles;
		poly->num_points = pc;
		poly->bounds.x = min.x;
		poly->bounds.y = min.y;
		poly->bounds.w = max.x - min.x;
		poly->bounds.h = max.y - min.y;
	}
	num_bases = *ptr++ & 0xff;
	bases = XMALLOC(homebase_t, num_bases);
	if (bases == NULL) {
		error("No memory for Map bases (%d)", num_bases);
		exit(1);
	}
	for (i = 0; i < num_bases; i++) {
		/* base.pos is not used */
		bases[i].id = -1;
		bases[i].team = *ptr++ & 0xff;
		cx = get_ushort(&ptr);
		cy = get_ushort(&ptr);
		bases[i].bounds.x = cx - BLOCK_SZ / 2;
		bases[i].bounds.y = cy - BLOCK_SZ / 2;
		bases[i].bounds.w = BLOCK_SZ;
		bases[i].bounds.h = BLOCK_SZ;
		bases[i].bx = cx / BLOCK_SZ;
		bases[i].by = cy / BLOCK_SZ;
		bases[i].x = cx;
		bases[i].y = cy;
		if (*ptr < 16)
			bases[i].type = SETUP_BASE_RIGHT;
		else if (*ptr < 48)
			bases[i].type = SETUP_BASE_UP;
		else if (*ptr < 80)
			bases[i].type = SETUP_BASE_LEFT;
		else if (*ptr < 112)
			bases[i].type = SETUP_BASE_DOWN;
		else
			bases[i].type = SETUP_BASE_RIGHT;
		bases[i].appeartime = 0;
		ptr++;
	}
	num_fuels = get_ushort(&ptr);
	if (num_fuels != 0) {
		fuels = XMALLOC(fuelstation_t, num_fuels);
		if (fuels == NULL) {
			error("No memory for Map fuels (%d)", num_fuels);
			exit(1);
		}
	}
	for (i = 0; i < num_fuels; i++) {
		cx = get_ushort(&ptr);
		cy = get_ushort(&ptr);
		fuels[i].fuel = MAX_STATION_FUEL;
		fuels[i].bounds.x = cx - BLOCK_SZ / 2;
		fuels[i].bounds.y = cy - BLOCK_SZ / 2;
		fuels[i].bounds.w = BLOCK_SZ;
		fuels[i].bounds.h = BLOCK_SZ;
	}
	num_checks = *ptr++ & 0xff;
	if (num_checks != 0) {

		checks = XMALLOC(checkpoint_t, num_checks);
		if (checks == NULL) {
			error("No memory for checkpoints (%d)", num_checks);
			exit(1);
		}
	}
	for (i = 0; i < num_checks; i++) {
		cx = get_ushort(&ptr);
		cy = get_ushort(&ptr);
		checks[i].bounds.x = cx - BLOCK_SZ / 2;
		checks[i].bounds.y = cy - BLOCK_SZ / 2;
		checks[i].bounds.w = BLOCK_SZ;
		checks[i].bounds.h = BLOCK_SZ;
	}

	/*
	 * kps - hack.
	 * Player can disable downloading of textures by having texturedWalls off.
	 */
	if (wallDrawMode == DRAW_TEXTURED
	    && Setup->data_url[0])
		Mapdata_setup(Setup->data_url);
	Colors_init_style_colors();

	return 0;
}
