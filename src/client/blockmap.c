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

/* $Id: blockmap.c,v 1.1 2007/11/03 18:49:25 kps Exp $ */

#include "sysdeps.h"

#include "bit.h"
#include "rules.h"
#include "error.h"
#include "proto.h"
#include "const.h"
#include "global.h"
#include "frame.h"
#include "setup.h"

int init_blockmap(void)
{
	int i, max, type;
	uint8_t types[256];

	num_fuels = 0;
	num_bases = 0;
	num_cannons = 0;
	num_targets = 0;
	num_treasures = 0;
	num_checks = 0;
	fuels = NULL;
	bases = NULL;
	cannons = NULL;
	targets = NULL;
	checks = NULL;
	memset(types, 0, sizeof types);
	types[SETUP_FUEL] = 1;
	types[SETUP_CANNON_UP] = 2;
	types[SETUP_CANNON_RIGHT] = 2;
	types[SETUP_CANNON_DOWN] = 2;
	types[SETUP_CANNON_LEFT] = 2;
	for (i = SETUP_TARGET; i < SETUP_TARGET + 10; i++)
		types[i] = 3;
	for (i = SETUP_BASE_LOWEST; i <= SETUP_BASE_HIGHEST; i++)
		types[i] = 4;
	for (i = 0; i < OLD_MAX_CHECKS; i++)
		types[SETUP_CHECK + i] = 5;
	for (i = SETUP_TREASURE; i < SETUP_TREASURE + 10; i++)
		types[i] = 6;
	max = Setup->x * Setup->y;
	for (i = 0; i < max; i++) {
		switch (types[Setup->map_data[i]]) {
		case 1:
			num_fuels++;
			break;
		case 2:
			num_cannons++;
			break;
		case 3:
			num_targets++;
			break;
		case 4:
			num_bases++;
			break;
		case 5:
			num_checks++;
			break;
		case 6:
			num_treasures++;
			break;
		default:
			break;
		}
	}
	if (num_bases != 0) {
		bases = XMALLOC(homebase_t, num_bases);
		if (bases == NULL) {
			error("No memory for Map bases (%d)", num_bases);
			return -1;
		}
		num_bases = 0;
	}
	if (num_fuels != 0) {
		fuels = XMALLOC(fuelstation_t, num_fuels);
		if (fuels == NULL) {
			error("No memory for Map fuels (%d)", num_fuels);
			return -1;
		}
		num_fuels = 0;
	}
	if (num_targets != 0) {
		targets = XMALLOC(target_t, num_targets);
		if (targets == NULL) {
			error("No memory for Map targets (%d)", num_targets);
			return -1;
		}
		num_targets = 0;
	}
	if (num_cannons != 0) {
		cannons = XMALLOC(cannontime_t, num_cannons);
		if (cannons == NULL) {
			error("No memory for Map cannons (%d)", num_cannons);
			return -1;
		}
		num_cannons = 0;
	}
	if (num_checks != 0) {
		checks = XMALLOC(checkpoint_t, num_checks);
		if (checks == NULL) {
			error("No memory for Map checks (%d)", num_checks);
			return -1;
		}
		num_checks = 0;
	}

	for (i = 0; i < max; i++) {
		type = Setup->map_data[i];
		switch (types[type]) {
		case 1:
			fuels[num_fuels].pos = i;
			fuels[num_fuels].fuel = MAX_STATION_FUEL;
			num_fuels++;
			break;
		case 2:
			cannons[num_cannons].pos = i;
			cannons[num_cannons].dead_time = 0;
			cannons[num_cannons].dot = 0;
			num_cannons++;
			break;
		case 3:
			targets[num_targets].pos = i;
			targets[num_targets].dead_time = 0;
			targets[num_targets].damage = TARGET_DAMAGE;
			num_targets++;
			break;
		case 4:
			bases[num_bases].pos = i;
			bases[num_bases].bx = i / Setup->y;
			bases[num_bases].by = i % Setup->y;
			bases[num_bases].x = bases[num_bases].bx * BLOCK_SZ + BLOCK_SZ / 2;
			bases[num_bases].y = bases[num_bases].by * BLOCK_SZ + BLOCK_SZ / 2;
			bases[num_bases].id = -1;
			bases[num_bases].team = type % 10;
			bases[num_bases].type = type - (type % 10);
			bases[num_bases].appeartime = 0;
			num_bases++;
			Setup->map_data[i] = type - (type % 10);
			break;
		case 5:
			checks[type - SETUP_CHECK].pos = i;
			num_checks++;
			Setup->map_data[i] = SETUP_CHECK;
			break;
		default:
			break;
		}
	}
	return 0;
}


/*
 * Convert a 'space' map block into a dot.
 */
static void Map_make_dot(unsigned char *data)
{
	if (*data == SETUP_SPACE)
		*data = SETUP_SPACE_DOT;
	else if (*data == SETUP_DECOR_FILLED)
		*data = SETUP_DECOR_DOT_FILLED;
	else if (*data == SETUP_DECOR_RU)
		*data = SETUP_DECOR_DOT_RU;
	else if (*data == SETUP_DECOR_RD)
		*data = SETUP_DECOR_DOT_RD;
	else if (*data == SETUP_DECOR_LU)
		*data = SETUP_DECOR_DOT_LU;
	else if (*data == SETUP_DECOR_LD)
		*data = SETUP_DECOR_DOT_LD;
}

/*
 * Optimize the drawing of all blue space dots by converting
 * certain map objects into a specialised form of their type.
 */
void Map_dots(void)
{
	int i, x, y, start;
	unsigned char dot[256];

	/*
	 * Lookup table to recognize dots.
	 */
	memset(dot, 0, sizeof dot);
	dot[SETUP_SPACE_DOT] = 1;
	dot[SETUP_DECOR_DOT_FILLED] = 1;
	dot[SETUP_DECOR_DOT_RU] = 1;
	dot[SETUP_DECOR_DOT_RD] = 1;
	dot[SETUP_DECOR_DOT_LU] = 1;
	dot[SETUP_DECOR_DOT_LD] = 1;

	/*
	 * Restore the map to unoptimized form.
	 */
	for (i = Setup->x * Setup->y; i-- > 0;) {
		if (dot[Setup->map_data[i]]) {
			if (Setup->map_data[i] == SETUP_SPACE_DOT)
				Setup->map_data[i] = SETUP_SPACE;
			else if (Setup->map_data[i] == SETUP_DECOR_DOT_FILLED)
				Setup->map_data[i] = SETUP_DECOR_FILLED;
			else if (Setup->map_data[i] == SETUP_DECOR_DOT_RU)
				Setup->map_data[i] = SETUP_DECOR_RU;
			else if (Setup->map_data[i] == SETUP_DECOR_DOT_RD)
				Setup->map_data[i] = SETUP_DECOR_RD;
			else if (Setup->map_data[i] == SETUP_DECOR_DOT_LU)
				Setup->map_data[i] = SETUP_DECOR_LU;
			else if (Setup->map_data[i] == SETUP_DECOR_DOT_LD)
				Setup->map_data[i] = SETUP_DECOR_LD;
		}
	}

	/*
	 * Lookup table to test for map data which can be turned into a dot.
	 */
	memset(dot, 0, sizeof dot);
	dot[SETUP_SPACE] = 1;
	if (decorDrawMode == DRAW_NONE) {
		dot[SETUP_DECOR_FILLED] = 1;
		dot[SETUP_DECOR_RU] = 1;
		dot[SETUP_DECOR_RD] = 1;
		dot[SETUP_DECOR_LU] = 1;
		dot[SETUP_DECOR_LD] = 1;
	}

	/*
	 * Optimize.
	 */
	if (backgroundPointSize > 0) {
		if (BIT(Setup->mode, WRAP_PLAY)) {
			for (x = 0; x < Setup->x; x++) {
				if (dot[Setup->map_data[x * Setup->y]])
					Map_make_dot(&Setup->map_data[x * Setup->y]);
			}
			for (y = 0; y < Setup->y; y++) {
				if (dot[Setup->map_data[y]])
					Map_make_dot(&Setup->map_data[y]);
			}
			start = backgroundPointDist;
		}
		else
			start = 0;

		if (backgroundPointDist > 0) {
			for (x = start; x < Setup->x; x += backgroundPointDist) {
				for (y = start; y < Setup->y; y += backgroundPointDist) {
					if (dot[Setup->map_data[x * Setup->y + y]])
						Map_make_dot(&Setup->map_data[x * Setup->y + y]);
				}
			}
		}
		for (i = 0; i < num_cannons; i++) {
			x = cannons[i].pos / Setup->y;
			y = cannons[i].pos % Setup->y;
			if ((x == 0 || y == 0) && BIT(Setup->mode, WRAP_PLAY))
				cannons[i].dot = 1;
			else if (backgroundPointDist > 0
				 && x % backgroundPointDist == 0 && y % backgroundPointDist == 0)
				cannons[i].dot = 1;
			else
				cannons[i].dot = 0;
		}
	}
}

/*
 * Optimize the drawing of all blue map objects by converting
 * their map type to a bitmask with bits for each blue segment.
 */
static void Map_restore(int startx, int starty, int width, int height)
{
	int i, j, x, y, map_index, type;

	/*
	 * Restore an optimized map to its original unoptimized state.
	 */
	x = startx;
	for (i = 0; i < width; i++, x++) {
		if (x < 0)
			x += Setup->x;
		else if (x >= Setup->x)
			x -= Setup->x;

		y = starty;
		for (j = 0; j < height; j++, y++) {
			if (y < 0)
				y += Setup->y;
			else if (y >= Setup->y)
				y -= Setup->y;

			map_index = x * Setup->y + y;

			type = Setup->map_data[map_index];
			if ((type & BLUE_BIT) == 0) {
				if (type == SETUP_FILLED_NO_DRAW)
					Setup->map_data[map_index] = SETUP_FILLED;
			}
			else if ((type & BLUE_FUEL) == BLUE_FUEL)
				Setup->map_data[map_index] = SETUP_FUEL;

			else if (type & BLUE_OPEN) {
				if (type & BLUE_BELOW)
					Setup->map_data[map_index] = SETUP_REC_RD;
				else
					Setup->map_data[map_index] = SETUP_REC_LU;
			}
			else if (type & BLUE_CLOSED) {
				if (type & BLUE_BELOW)
					Setup->map_data[map_index] = SETUP_REC_LD;
				else
					Setup->map_data[map_index] = SETUP_REC_RU;
			}
			else
				Setup->map_data[map_index] = SETUP_FILLED;
		}
	}
}

static unsigned char blue[256];

static void Make_blue_array(void)
{
	int i;

	/*
	 * Optimize the map for blue.
	 */
	memset(blue, 0, sizeof blue);
	blue[SETUP_FILLED] = BLUE_LEFT | BLUE_UP | BLUE_RIGHT | BLUE_DOWN;
	blue[SETUP_FILLED_NO_DRAW] = blue[SETUP_FILLED];
	blue[SETUP_FUEL] = blue[SETUP_FILLED];
	blue[SETUP_REC_RU] = BLUE_RIGHT | BLUE_UP;
	blue[SETUP_REC_RD] = BLUE_RIGHT | BLUE_DOWN;
	blue[SETUP_REC_LU] = BLUE_LEFT | BLUE_UP;
	blue[SETUP_REC_LD] = BLUE_LEFT | BLUE_DOWN;
	blue[BLUE_BIT | BLUE_OPEN] =
	    blue[BLUE_BIT | BLUE_OPEN | BLUE_LEFT] =
	    blue[BLUE_BIT | BLUE_OPEN | BLUE_UP] =
	    blue[BLUE_BIT | BLUE_OPEN | BLUE_LEFT | BLUE_UP] = blue[SETUP_REC_LU];
	blue[BLUE_BIT | BLUE_OPEN | BLUE_BELOW] =
	    blue[BLUE_BIT | BLUE_OPEN | BLUE_BELOW | BLUE_RIGHT] =
	    blue[BLUE_BIT | BLUE_OPEN | BLUE_BELOW | BLUE_DOWN] =
	    blue[BLUE_BIT | BLUE_OPEN | BLUE_BELOW | BLUE_RIGHT | BLUE_DOWN] = blue[SETUP_REC_RD];
	blue[BLUE_BIT | BLUE_CLOSED] =
	    blue[BLUE_BIT | BLUE_CLOSED | BLUE_RIGHT] =
	    blue[BLUE_BIT | BLUE_CLOSED | BLUE_UP] =
	    blue[BLUE_BIT | BLUE_CLOSED | BLUE_RIGHT | BLUE_UP] = blue[SETUP_REC_RU];
	blue[BLUE_BIT | BLUE_CLOSED | BLUE_BELOW] =
	    blue[BLUE_BIT | BLUE_CLOSED | BLUE_BELOW | BLUE_LEFT] =
	    blue[BLUE_BIT | BLUE_CLOSED | BLUE_BELOW | BLUE_DOWN] =
	    blue[BLUE_BIT | BLUE_CLOSED | BLUE_BELOW | BLUE_LEFT | BLUE_DOWN] = blue[SETUP_REC_LD];
	for (i = BLUE_BIT; i < (int) (sizeof blue); i++) {
		if ((i & BLUE_FUEL) == BLUE_FUEL || (i & (BLUE_OPEN | BLUE_CLOSED)) == 0)
			blue[i] = blue[SETUP_FILLED];
	}
}

void Map_blue(int startx, int starty, int width, int height)
{
	int i, j, x, y, map_index, type, newtype;
	bool outline = false;

	Map_restore(startx, starty, width, height);

	if (wallDrawMode == DRAW_OUTLINE
	    || wallDrawMode == DRAW_FILLED
	    || wallDrawMode == DRAW_TEXTURED)
		outline = true;

	Make_blue_array();

	x = startx;
	for (i = 0; i < width; i++, x++) {
		if (x < 0)
			x += Setup->x;
		else if (x >= Setup->x)
			x -= Setup->x;

		y = starty;
		for (j = 0; j < height; j++, y++) {
			if (y < 0)
				y += Setup->y;
			else if (y >= Setup->y)
				y -= Setup->y;

			map_index = x * Setup->y + y;

			type = Setup->map_data[map_index];
			newtype = 0;
			switch (type) {
			case SETUP_FILLED:
			case SETUP_FILLED_NO_DRAW:
			case SETUP_FUEL:
				newtype = BLUE_BIT;
				if (type == SETUP_FUEL) {
					newtype |= BLUE_FUEL;
				}
				if ((x == 0)
				    ? (!BIT(Setup->mode, WRAP_PLAY) ||
				       !(blue[Setup->map_data[(Setup->x - 1) * Setup->y + y]]
					 & BLUE_RIGHT))
				    : !(blue[Setup->map_data[(x - 1) * Setup->y + y]]
					& BLUE_RIGHT))
					newtype |= BLUE_LEFT;
				if ((y == 0)
				    ? (!BIT(Setup->mode, WRAP_PLAY) ||
				       !(blue[Setup->map_data[x * Setup->y + Setup->y - 1]]
					 & BLUE_UP))
				    : !(blue[Setup->map_data[x * Setup->y + (y - 1)]]
					& BLUE_UP))
					newtype |= BLUE_DOWN;
				if (!outline || ((x == Setup->x - 1)
						 ? (!BIT(Setup->mode, WRAP_PLAY)
						    || !(blue[Setup->map_data[y]]
							 & BLUE_LEFT))
						 : !(blue[Setup->map_data[(x + 1) * Setup->y + y]]
						     & BLUE_LEFT)))
					newtype |= BLUE_RIGHT;
				if (!outline || ((y == Setup->y - 1)
						 ? (!BIT(Setup->mode, WRAP_PLAY)
						    || !(blue[Setup->map_data[x * Setup->y]]
							 & BLUE_DOWN))
						 : !(blue[Setup->map_data[x * Setup->y + (y + 1)]]
						     & BLUE_DOWN)))
					newtype |= BLUE_UP;
				break;

			case SETUP_REC_LU:
				newtype = BLUE_BIT | BLUE_OPEN;
				if (x == 0
				    ? (!BIT(Setup->mode, WRAP_PLAY) ||
				       !(blue[Setup->map_data[(Setup->x - 1) * Setup->y + y]]
					 & BLUE_RIGHT))
				    : !(blue[Setup->map_data[(x - 1) * Setup->y + y]]
					& BLUE_RIGHT))
					newtype |= BLUE_LEFT;
				if (!outline || ((y == Setup->y - 1)
						 ? (!BIT(Setup->mode, WRAP_PLAY)
						    || !(blue[Setup->map_data[x * Setup->y]]
							 & BLUE_DOWN))
						 : !(blue[Setup->map_data[x * Setup->y + (y + 1)]]
						     & BLUE_DOWN)))
					newtype |= BLUE_UP;
				break;

			case SETUP_REC_RU:
				newtype = BLUE_BIT | BLUE_CLOSED;
				if (!outline || ((x == Setup->x - 1)
						 ? (!BIT(Setup->mode, WRAP_PLAY)
						    || !(blue[Setup->map_data[y]]
							 & BLUE_LEFT))
						 : !(blue[Setup->map_data[(x + 1) * Setup->y + y]]
						     & BLUE_LEFT)))
					newtype |= BLUE_RIGHT;
				if (!outline || ((y == Setup->y - 1)
						 ? (!BIT(Setup->mode, WRAP_PLAY)
						    || !(blue[Setup->map_data[x * Setup->y]]
							 & BLUE_DOWN))
						 : !(blue[Setup->map_data[x * Setup->y + (y + 1)]]
						     & BLUE_DOWN)))
					newtype |= BLUE_UP;
				break;

			case SETUP_REC_LD:
				newtype = BLUE_BIT | BLUE_BELOW | BLUE_CLOSED;
				if ((x == 0)
				    ? (!BIT(Setup->mode, WRAP_PLAY) ||
				       !(blue[Setup->map_data[(Setup->x - 1) * Setup->y + y]]
					 & BLUE_RIGHT))
				    : !(blue[Setup->map_data[(x - 1) * Setup->y + y]]
					& BLUE_RIGHT))
					newtype |= BLUE_LEFT;
				if ((y == 0)
				    ? (!BIT(Setup->mode, WRAP_PLAY) ||
				       !(blue[Setup->map_data[x * Setup->y + Setup->y - 1]]
					 & BLUE_UP))
				    : !(blue[Setup->map_data[x * Setup->y + (y - 1)]]
					& BLUE_UP))
					newtype |= BLUE_DOWN;
				break;

			case SETUP_REC_RD:
				newtype = BLUE_BIT | BLUE_BELOW | BLUE_OPEN;
				if (!outline || ((x == Setup->x - 1)
						 ? (!BIT(Setup->mode, WRAP_PLAY)
						    || !(blue[Setup->map_data[y]]
							 & BLUE_LEFT))
						 : !(blue[Setup->map_data[(x + 1) * Setup->y + y]]
						     & BLUE_LEFT)))
					newtype |= BLUE_RIGHT;
				if ((y == 0)
				    ? (!BIT(Setup->mode, WRAP_PLAY) ||
				       !(blue[Setup->map_data[x * Setup->y + Setup->y - 1]]
					 & BLUE_UP))
				    : !(blue[Setup->map_data[x * Setup->y + (y - 1)]]
					& BLUE_UP))
					newtype |= BLUE_DOWN;
				break;

			default:
				continue;
			}
			if (newtype != 0) {
				if (newtype == BLUE_BIT)
					newtype = SETUP_FILLED_NO_DRAW;
				Setup->map_data[map_index] = newtype;
			}
		}
	}
}


/*
 * Return map index of block at (x + x_offset, y + y_offset),
 * wrapping if necessary.
 */

static int Get_map_index(int x, int y, int x_offset, int y_offset)
{
	x += x_offset;
	y += y_offset;

	while (x < 0)
		x += Setup->x;
	while (x >= Setup->x)
		x -= Setup->x;
	while (y < 0)
		y += Setup->y;
	while (y >= Setup->y)
		y -= Setup->y;

	return x * Setup->y + y;
}

unsigned char *map_edge_descriptor_array = NULL;	/* kps - should be freed */

int Map_edges(void)
{
	int x, y, map_index, map_index2, m, m2;
	unsigned char *tmparray;

#if 0
	for (y = Setup->y - 1; y >= 0; y--) {
		printf("\n");
		for (x = 0; x < Setup->x; x++) {
			map_index = x * Setup->y + y;
			m = Setup->map_data[map_index];
			switch (m) {
			case SETUP_FILLED:
			case SETUP_FUEL:
				printf("x");
				break;
			case SETUP_REC_RU:
			case SETUP_REC_RD:
			case SETUP_REC_LU:
			case SETUP_REC_LD:
				printf(".");
				break;
			default:
				printf(" ");
				break;
			}
		}
	}
#endif

	/* q = REC_RD  w = REC_LD  a = REC_RU  s = REC_LU */

	tmparray = XCALLOC(unsigned char, (Setup->x * Setup->y));
	if (tmparray == NULL)
		return -1;

	for (y = 0; y < Setup->y; y++) {
		for (x = 0; x < Setup->x; x++) {
			map_index = x * Setup->y + y;
			m = Setup->map_data[map_index];
			switch (m) {
			case SETUP_FILLED:
			case SETUP_FUEL:
				tmparray[map_index] = 1 | 4 | 16 | 64;
				break;
			case SETUP_REC_LU:
				tmparray[map_index] = 2 | 16 | 64;
				break;
			case SETUP_REC_RU:
				tmparray[map_index] = 4 | 16 | 128;
				break;
			case SETUP_REC_LD:
				tmparray[map_index] = 1 | 8 | 64;
				break;
			case SETUP_REC_RD:
				tmparray[map_index] = 1 | 4 | 32;
				break;
			default:
				tmparray[map_index] = 0;
				break;
			}
		}
	}

	for (y = 0; y < Setup->y; y++) {
		for (x = 0; x < Setup->x; x++) {
			map_index = x * Setup->y + y;
			m = tmparray[map_index];

			/* look down */
			m = tmparray[map_index];
			map_index2 = Get_map_index(x, y, 0, -1);
			m2 = tmparray[map_index2];
			if ((m & 1) && (m2 & 16)) {
				tmparray[map_index] &= ~1;
				tmparray[map_index2] &= ~16;
			}

			/* look right */
			m = tmparray[map_index];
			map_index2 = Get_map_index(x, y, 1, 0);
			m2 = tmparray[map_index2];
			if ((m & 4) && (m2 & 64)) {
				tmparray[map_index] &= ~4;
				tmparray[map_index2] &= ~64;
			}

			/* look up */
			m = tmparray[map_index];
			map_index2 = Get_map_index(x, y, 0, 1);
			m2 = tmparray[map_index2];
			if ((m & 16) && (m2 & 1)) {
				tmparray[map_index] &= ~16;
				tmparray[map_index2] &= ~1;
			}

			/* look left */
			m = tmparray[map_index];
			map_index2 = Get_map_index(x, y, -1, 0);
			m2 = tmparray[map_index2];
			if ((m & 64) && (m2 & 4)) {
				tmparray[map_index] &= ~64;
				tmparray[map_index2] &= ~4;
			}
		}
	}

#if 0
	for (y = Setup->y - 1; y >= 0; y--) {
		printf("\n");
		for (x = 0; x < Setup->x; x++) {
			map_index = x * Setup->y + y;
			m = tmparray[map_index];
			if (m & 2)	/* LU */
				printf("s");
			else if (m & 8)	/* LD */
				printf("w");
			else if (m & 32)	/* RD */
				printf("q");
			else if (m & 128)	/* RU */
				printf("a");
			else if (m & 1)
				printf("v");
			else if (m & 4)
				printf(">");
			else if (m & 16)
				printf("^");
			else if (m & 64)
				printf("<");
			else
				printf(" ");
		}
	}
#endif

	/* Now what remains in tmparray are the edges we are looking for. */
	map_edge_descriptor_array = tmparray;

	return 0;
}
