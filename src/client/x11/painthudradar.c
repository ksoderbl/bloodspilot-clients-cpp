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
 * Copyright (C) 2000-2001 Juha Lindström <juhal@users.sourceforge.net>
 *
 * Copyright (C) 2002-2007 by
 *
 *      Erik Andersson       <maximan@users.sourceforge.net>
 *      Kristian Söderblom   <kps@users.sourceforge.net>
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

/* $Id: painthudradar.c,v 1.12 2007/12/05 22:48:27 kps Exp $ */

#include "xpclient_x11.h"
#include "frame.h"
#include "client.h"
#include "setup.h"
#include "rules.h"
#include "error.h"
#include "bit.h"

static int hudRadarEnemyColor;	/* Color index for enemy radar dots */
static int hudRadarOtherColor;	/* Color index for other radar dots */
static int hudRadarWallColor;	/* Color index for walls on radar */

xp_option_t hudradar_options[] = {

	COLOR_INDEX_OPTION("hudRadarEnemyColor",
			   RED,
			   &hudRadarEnemyColor,
			   "Which color number to use for drawing radar dots\n"
			   "that represent enemy ships.\n"),

	COLOR_INDEX_OPTION("hudRadarOtherColor",
			   GREEN,
			   &hudRadarOtherColor,
			   "Which color number to use for drawing radar dots\n"
			   "that represent friendly ships or other objects.\n"),

	COLOR_INDEX_OPTION("hudRadarWallColor",
			   DARK_GRAY,
			   &hudRadarWallColor,
			   "Which color number to use for drawing walls on radar.\n"),
};


void Store_hudradar_options(void)
{
	STORE_OPTIONS(hudradar_options);
}



#if 0
static int wrap(int *xp, int *yp)
{
	int x = *xp, y = *yp;

	if (x < world.x || x > world.x + ext_view_width) {
		if (x < realWorld.x || x > realWorld.x + ext_view_width)
			return 0;
		*xp += world.x - realWorld.x;
	}
	if (y < world.y || y > world.y + ext_view_height) {
		if (y < realWorld.y || y > realWorld.y + ext_view_height)
			return 0;
		*yp += world.y - realWorld.y;
	}
	return 1;
}
#endif

static void Paint_hudradar_bases(void)
{
	int i;
	float hrscale = hudRadarScale;
	float hrw = hrscale * 256;
	float hrh = hrscale * RadarHeight;
	float xf = hrw / (float) Setup->width;
	float yf = hrh / (float) Setup->height;
	int color, x, y;
	other_t *eyes = Other_by_id(eyesId);

	for (i = 0; i < num_bases; i++) {
		int sz = 1, id;
		other_t *other = NULL;

		x = (bases[i].x - (world.x + ext_view_width / 2)) * xf;
		y = (bases[i].y - (world.y + ext_view_height / 2)) * yf;

		if (BIT(Setup->mode, WRAP_PLAY)) {
			if (x < 0) {
				if (-x > hrw / 2)
					x += hrw;
			}
			else {
				if (x > hrw / 2)
					x -= hrw;
			}

			if (y < 0) {
				if (-y > hrh / 2)
					y += hrh;
			}
			else {
				if (y > hrh / 2)
					y -= hrh;
			}
		}

		color = hudRadarEnemyColor;
		id = bases[i].id;
		other = Other_by_id(id);

		if (eyes && eyes->team == bases[i].team)
			color = hudRadarOtherColor;

		/* teammate ? */
		if (BIT(Setup->mode, TEAM_PLAY)
		    && eyes != NULL && eyes->id != id && other != NULL && eyes->team == other->team)
			color = hudRadarOtherColor;

		/* ally ? */
		if (eyes != NULL
		    && eyes->id != id && other != NULL && eyes->alliance != ' '
		    && eyes->alliance == other->alliance)
			color = hudRadarOtherColor;

		/*
		 * Even if showExtHUDRadar is off, we do the
		 * base warning on HUD radar.
		 */
		if (twelveHz < bases[i].appeartime) {
			if (eyes && eyes->id == id) {
				;	/*
					 * Don't paint base warning on HUD for eyes player,
					 * it can obscure other things.
					 */
			}
			else {
				if (twelveHz & 1) {
					sz = 2;
				}
			}
		}
		else if (!showExtHUDRadar)
			continue;

		if (color == NO_COLOR)
			continue;

		sz = (sz > 0) ? sz * hrscale : hrscale;

		Rectangle_add_unscaled(color,
				       x + draw_width / 2 - sz / 2, -y + draw_height / 2 - sz / 2,
				       sz, sz, false);
	}
}

void Paint_hudradar_dots(void)
{
	int i;
	float hrscale = hudRadarScale;
	float hrw = hrscale * 256;
	float hrh = hrscale * RadarHeight;
	float xf = hrw / (float) Setup->width, yf = hrh / (float) Setup->height;
	int color, x, y;

	for (i = 0; i < num_radar; i++) {
		int sz = radar_ptr[i].size;

		if (radar_ptr[i].type == RadarEnemy)
			color = hudRadarEnemyColor;
		else
			color = hudRadarOtherColor;

		if (color == NO_COLOR)
			continue;

		x = radar_ptr[i].x * hrscale - (world.x + ext_view_width / 2) * xf;

		y = radar_ptr[i].y * hrscale - (world.y + ext_view_height / 2) * yf;

		if (BIT(Setup->mode, WRAP_PLAY)) {
			if (x < 0) {
				if (-x > hrw / 2)
					x += hrw;
			}
			else {
				if (x > hrw / 2)
					x -= hrw;
			}

			if (y < 0) {
				if (-y > hrh / 2)
					y += hrh;
			}
			else {
				if (y > hrh / 2)
					y -= hrh;
			}
		}

		/* skip objects that would be drawn over our ship */
		if (x < SHIP_SZ && x > -SHIP_SZ && y < SHIP_SZ && y > -SHIP_SZ)
			continue;

		if (sz <= 0)
			sz = 1;
		sz *= hudRadarDotScale;

		Arc_add_unscaled(color,
				 x + draw_width / 2 - sz / 2, -y + draw_height / 2 - sz / 2, sz, sz,
				 0, 64 * 360);
	}
}

static void Paint_mapradar_dots(void)
{
	int i, x, y, sz = 2*SHIP_SZ;
	double xlimit = (double)active_view_width / 2;
	double ylimit = (double)active_view_height / 2;
	double hrscale = (double) Setup->width / (double) 256;
	int hrw = (int)(hrscale * 256);
	int hrh = (int)(hrscale * RadarHeight);
	double xf = (double) hrw / (double) Setup->width;
	double yf = (double) hrh / (double) Setup->height;

	for (i = 0; i < num_radar; i++) {
		x = (int)(radar_ptr[i].x * hrscale
			  - (world.x + ext_view_width / 2) * xf);
		y = (int)(radar_ptr[i].y * hrscale
			  - (world.y + ext_view_height / 2) * yf);

		if (x < -hrw / 2)
			x += hrw;
		else if (x > hrw / 2)
			x -= hrw;

		if (y < -hrh / 2)
			y += hrh;
		else if (y > hrh / 2)
			y -= hrh;

		if (!((x <= xlimit) && (x >= -xlimit)
		      && (y <= ylimit) && (y >= -ylimit))) {

			x = x + ext_view_width / 2 - sz / 2;
			y = -y + ext_view_height / 2 - sz / 2;

			if (radar_ptr[i].type == RadarEnemy) {
				if (hudRadarEnemyColor >= 1) {
					Arc_add(hudRadarEnemyColor, x, y, sz, sz, 0, 64 * 360);
				}
			} else {
				if (hudRadarOtherColor >= 1) {
					Arc_add(hudRadarOtherColor, x, y, sz, sz, 0, 64 * 360);
				}
			}
		}
	}
}


static void Paint_hudradar_block_walls(void)
{
	int i;
	float hrscale = hudRadarScale;
	float hrw = hrscale * 256;
	float hrh = hrscale * RadarHeight;
	float xf = hrw / (float) Setup->width;
	float yf = hrh / (float) Setup->height;
	int color, x, y;
	int m, bx, by, map_index;
	XPoint map_points[4];
	XPoint hr_points[4];
	XPoint points[2];
	bool in_record = false;

	color = hudRadarWallColor;

	for (by = 0; by < Setup->y; by++) {
		for (bx = 0; bx < Setup->x; bx++) {
			map_index = bx * Setup->y + by;
			m = (int) map_edge_descriptor_array[map_index];

			if (m == 0)
				continue;

			/* map pixel coords */
			map_points[0].x = bx * BLOCK_SZ;
			map_points[0].y = by * BLOCK_SZ;
			map_points[1].x = map_points[0].x + BLOCK_SZ;
			map_points[1].y = map_points[0].y;
			map_points[2].x = map_points[0].x + BLOCK_SZ;
			map_points[2].y = map_points[0].y + BLOCK_SZ;
			map_points[3].x = map_points[0].x;
			map_points[3].y = map_points[0].y + BLOCK_SZ;

			/* hud radar coordinates */
			for (i = 0; i < 4; i++) {
				x = (map_points[i].x - (world.x + ext_view_width / 2)) * xf;
				y = (map_points[i].y - (world.y + ext_view_height / 2)) * yf;
				if (BIT(Setup->mode, WRAP_PLAY)) {
					if (x < 0) {
						if (-x > hrw / 2)
							x += hrw;
					}
					else {
						if (x > hrw / 2)
							x -= hrw;
					}

					if (y < 0) {
						if (-y > hrh / 2)
							y += hrh;
					}
					else {
						if (y > hrh / 2)
							y -= hrh;
					}
				}
				hr_points[i].x = x + draw_width / 2;
				hr_points[i].y = -y + draw_height / 2;
			}

			if (m & 1) {
				points[0] = hr_points[0];
				points[1] = hr_points[1];
				if (QUICK_LENGTH
				    (points[0].x - points[1].x, points[0].y - points[1].y) < 100)
					Segment_add_unscaled(color, points[0].x, points[0].y,
							     points[1].x, points[1].y, in_record);
			}
			if ((m & 2) || (m & 32)) {
				points[0] = hr_points[0];
				points[1] = hr_points[2];
				if (QUICK_LENGTH
				    (points[0].x - points[1].x, points[0].y - points[1].y) < 100)
					Segment_add_unscaled(color, points[0].x, points[0].y,
							     points[1].x, points[1].y, in_record);
			}
			if (m & 4) {
				points[0] = hr_points[1];
				points[1] = hr_points[2];
				if (QUICK_LENGTH
				    (points[0].x - points[1].x, points[0].y - points[1].y) < 100)
					Segment_add_unscaled(color, points[0].x, points[0].y,
							     points[1].x, points[1].y, in_record);
			}
			if ((m & 8) || (m & 128)) {
				points[0] = hr_points[1];
				points[1] = hr_points[3];
				if (QUICK_LENGTH
				    (points[0].x - points[1].x, points[0].y - points[1].y) < 100)
					Segment_add_unscaled(color, points[0].x, points[0].y,
							     points[1].x, points[1].y, in_record);
			}
			if (m & 16) {
				points[0] = hr_points[2];
				points[1] = hr_points[3];
				if (QUICK_LENGTH
				    (points[0].x - points[1].x, points[0].y - points[1].y) < 100)
					Segment_add_unscaled(color, points[0].x, points[0].y,
							     points[1].x, points[1].y, in_record);
			}
			if (m & 64) {
				points[0] = hr_points[3];
				points[1] = hr_points[0];
				if (QUICK_LENGTH
				    (points[0].x - points[1].x, points[0].y - points[1].y) < 100)
					Segment_add_unscaled(color, points[0].x, points[0].y,
							     points[1].x, points[1].y, in_record);
			}
		}
	}
}

static void Compute_radar_bounds(ipos_t * min, ipos_t * max, const irec_t * b)
{
	min->x = (0 - (b->x + b->w)) / Setup->width;
	if (0 > b->x + b->w)
		min->x++;
	max->x = (0 + Setup->width - b->x) / Setup->width;
	if (0 + Setup->width < b->x)
		max->x--;
	min->y = (0 - (b->y + b->h)) / Setup->height;
	if (0 > b->y + b->h)
		min->y++;
	max->y = (0 + Setup->height - b->y) / Setup->height;
	if (0 + Setup->height < b->y)
		max->y--;
}

static void Paint_hudradar_polygon_walls(void)
{
	int i, j, xoff, yoff;
	ipos_t min, max;
	float hrscale = hudRadarScale;
	float hrw = hrscale * 256;
	float hrh = hrscale * RadarHeight;
	float xf = hrw / (float) Setup->width;
	float yf = hrh / (float) Setup->height;
	int color, x, y;
	XPoint points[2];
	bool in_record = false;
	float len;

	color = hudRadarWallColor;

	Set_fg_color(hudRadarWallColor);

	/* loop through all the polygons */
	for (i = 0; i < num_polygons; i++) {
		if (BIT(polygon_styles[polygons[i].style].flags, STYLE_INVISIBLE_RADAR))
			continue;
		Compute_radar_bounds(&min, &max, &polygons[i].bounds);
#if 0
		/* kps - let's not do this */
		if (fullColor)
			Fullcolor_set_fg(polygon_styles[polygons[i].style].pixel);
#endif
		for (xoff = min.x; xoff <= max.x; xoff++) {
			for (yoff = min.y; yoff <= max.y; yoff++) {
				int px = xoff * Setup->width;
				int py = yoff * Setup->height;

				/* get rid of warning */
				points[0].x = 0;
				points[0].y = 0;
				/* loop through the points in the current polygon */
				for (j = 0; j <= polygons[i].num_points; j++) {
					px += polygons[i].points[j].x;
					py += polygons[i].points[j].y;

					/* last point is first point */
					if (j == polygons[i].num_points) {
						px = xoff * Setup->width + polygons[i].points[0].x;
						py = yoff * Setup->height + polygons[i].points[0].y;
					}

					x = (px - (world.x + ext_view_width / 2)) * xf;
					y = (py - (world.y + ext_view_height / 2)) * yf;
					if (BIT(Setup->mode, WRAP_PLAY)) {
						if (x < 0) {
							if (-x > hrw / 2)
								x += hrw;
						}
						else {
							if (x > hrw / 2)
								x -= hrw;
						}
						if (y < 0) {
							if (-y > hrh / 2)
								y += hrh;
						}
						else {
							if (y > hrh / 2)
								y -= hrh;
						}
					}

					points[1] = points[0];
					points[0].x = x + draw_width / 2;
					points[0].y = -y + draw_height / 2;
					if (j == 0)
						continue; /* we have only one point so far. */
					len = QUICK_LENGTH(points[0].x - points[1].x, points[0].y - points[1].y);
					if (len >= 1 && len < 100) {
						if (in_record)
							rd.drawLine(dpy, drawPixmap, gameGC,
								    points[0].x, points[0].y,
								    points[1].x, points[1].y);
						else
							XDrawLine(dpy, drawPixmap, gameGC,
								  points[0].x, points[0].y,
								  points[1].x, points[1].y);
					}
				}
			}
		}
	}
}

void Paint_hudradar_walls(void)
{
	if (!showExtHUDRadar || hudRadarWallColor == NO_COLOR)
		return;

	if (oldServer)
		Paint_hudradar_block_walls();
	else
		Paint_hudradar_polygon_walls();
}

void Paint_hudradar(void)
{
	if (hudRadarEnemyColor != NO_COLOR || hudRadarOtherColor != NO_COLOR) {
		Paint_hudradar_dots();
		Paint_mapradar_dots();
		Paint_hudradar_bases();
	}
}


/* junk */
void Radar_hide_target(int x, int y)
{
	return;
}
void Paint_sliding_radar(void)
{
	return;
}
void Paint_world_radar(void)
{
	return;
}
void Radar_show_target(int x, int y)
{
	return;
}


#if 0
/* old paintradar.c stuff */

#include "xpclient_x11.h"

Window radarWindow;
Pixmap radarPixmap, radarPixmap2;
				/* Pixmaps for the radar (implements */
				/* the planes hack on the radar for */
				/* monochromes) */
long dpl_1[2], dpl_2[2];	/* Used by radar hack */
int radar_exposures;
int (*radarDrawRectanglePtr)	/* Function to draw player on radar */
 (Display * disp, Drawable d, GC gc, int x, int y, unsigned width, unsigned height);


static int slidingradar_x;	/* sliding radar offsets for windows */
static int slidingradar_y;

static int wallRadarColor;	/* Color index for walls on radar. */
static int targetRadarColor;	/* Color index for targets on radar. */
static int decorRadarColor;	/* Color index for decorations on radar. */


static void Copy_static_radar(void)
{
	if (radarPixmap2 != radarPixmap) {
		/* Draw static radar onto radar */
		XCopyArea(dpy, radarPixmap2, radarPixmap, gameGC, 0, 0, 256, RadarHeight, 0, 0);
	}
	else {
		/* Clear radar */
		XSetForeground(dpy, radarGC, colors[BLACK].pixel);
		XFillRectangle(dpy, radarPixmap, radarGC, 0, 0, 256, RadarHeight);
	}
	XSetForeground(dpy, radarGC, colors[WHITE].pixel);
}

static void Paint_checkpoint_radar(double xf, double yf)
{
	int x, y;
	XPoint points[5];

	if (BIT(Setup->mode, TIMING)) {
		if (oldServer) {
			Check_pos_by_index(nextCheckPoint, &x, &y);
			x = ((int) (x * BLOCK_SZ * xf + 0.5)) - slidingradar_x;
			y = (RadarHeight - (int) (y * BLOCK_SZ * yf + 0.5) + DSIZE - 1) -
			    slidingradar_y;
		}
		else {
			irec_t b = checks[nextCheckPoint].bounds;
			x = (int) (b.x * xf + 0.5) - slidingradar_x;
			y = (RadarHeight - (int) (b.y * yf + 0.5) + DSIZE - 1) - slidingradar_y;
		}
		if (x <= 0)
			x += 256;
		if (y <= 0)
			y += RadarHeight;

		/* top */
		points[0].x = x;
		points[0].y = y;
		/* right */
		points[1].x = x + DSIZE;
		points[1].y = y - DSIZE;
		/* bottom */
		points[2].x = x;
		points[2].y = y - 2 * DSIZE;
		/* left */
		points[3].x = x - DSIZE;
		points[3].y = y - DSIZE;
		/* top */
		points[4].x = x;
		points[4].y = y;
		XDrawLines(dpy, radarPixmap, radarGC, points, 5, 0);
	}
}

static void Paint_self_radar(double xf, double yf)
{
	int x, y, x_1, y_1, xw, yw;

	if (selfVisible != 0 && loops % 16 < 13) {
		x = (int) (selfPos.x * xf + 0.5) - slidingradar_x;
		y = RadarHeight - (int) (selfPos.y * yf + 0.5) - 1 - slidingradar_y;
		if (x <= 0)
			x += 256;
		if (y <= 0)
			y += RadarHeight;

		x_1 = (int) (x + 8 * tcos(heading));
		y_1 = (int) (y - 8 * tsin(heading));
		XDrawLine(dpy, radarPixmap, radarGC, x, y, x_1, y_1);
		if (BIT(Setup->mode, WRAP_PLAY)) {
			xw = x_1 - (x_1 + 256) % 256;
			yw = y_1 - (y_1 + RadarHeight) % RadarHeight;
			if (xw != 0)
				XDrawLine(dpy, radarPixmap, radarGC, x - xw, y, x_1 - xw, y_1);
			if (yw != 0) {
				XDrawLine(dpy, radarPixmap, radarGC, x, y - yw, x_1, y_1 - yw);
				if (xw != 0)
					XDrawLine(dpy, radarPixmap, radarGC, x - xw, y - yw,
						  x_1 - xw, y_1 - yw);
			}
		}
	}
}

static void Paint_objects_radar(void)
{
	int i, x, y, xw, yw, color;

	for (i = 0; i < num_radar; i++) {
		int rs = radar_ptr[i].size;
		unsigned s = (rs <= 0 ? 1 : radar_ptr[i].size);

		color = WHITE;
		if (radar_ptr[i].type == RadarFriend) {
			if (maxColors > 4)
				color = 4;
			else if (!colorSwitch)
				color = RED;
		}
		XSetForeground(dpy, radarGC, colors[color].pixel);
		x = radar_ptr[i].x - s / 2 - slidingradar_x;
		y = RadarHeight - radar_ptr[i].y - 1 - s / 2 - slidingradar_y;

		if (x <= 0)
			x += 256;
		if (y <= 0)
			y += RadarHeight;

		(*radarDrawRectanglePtr) (dpy, radarPixmap, radarGC, x, y, s, s);
		if (BIT(Setup->mode, WRAP_PLAY)) {
			xw = (x < 0) ? -256 : (x + s >= 256) ? 256 : 0;
			yw = (y < 0) ? -RadarHeight : (y + s >= RadarHeight) ? RadarHeight : 0;
			if (xw != 0)
				(*radarDrawRectanglePtr) (dpy, radarPixmap, radarGC, x - xw, y, s,
							  s);
			if (yw != 0) {
				(*radarDrawRectanglePtr) (dpy, radarPixmap, radarGC, x, y - yw, s,
							  s);

				if (xw != 0)
					(*radarDrawRectanglePtr) (dpy, radarPixmap, radarGC, x - xw,
								  y - yw, s, s);
			}
		}
		/*XSetForeground(dpy, radarGC, colors[WHITE].pixel); */
	}
	if (num_radar)
		RELEASE(radar_ptr, num_radar, max_radar);
}


void Paint_radar(void)
{
	const double xf = 256.0 / (double) Setup->width, yf =
	    (double) RadarHeight / (double) Setup->height;

	if (radar_exposures == 0)
		return;

	slidingradar_x = 0;
	slidingradar_y = 0;

	Copy_static_radar();

	/* Checkpoints */
	Paint_checkpoint_radar(xf, yf);

	Paint_self_radar(xf, yf);
	Paint_objects_radar();
}


void Paint_sliding_radar(void)
{
	if (!Setup)
		return;

	if (BIT(Setup->mode, WRAP_PLAY) == 0)
		return;

	if (radarPixmap != radarPixmap2)
		return;

	if (slidingRadar) {
		if (radarPixmap2 != radarWindow)
			return;

		radarPixmap2 = XCreatePixmap(dpy, radarWindow, 256, RadarHeight, dispDepth);
		radarPixmap = radarPixmap2;
		if (radar_exposures > 0)
			Paint_world_radar();
	}
	else {
		if (radarPixmap2 == radarWindow)
			return;
		XFreePixmap(dpy, radarPixmap2);
		radarPixmap2 = radarWindow;
		radarPixmap = radarWindow;
		if (radar_exposures > 0)
			Paint_world_radar();
	}
}

/*
 * Try and draw an area of the radar which represents block position
 * 'xi' 'yi'.  If 'draw' is zero the area is cleared.
 */
static void Paint_radar_block(int xi, int yi, int color)
{
	double xs, ys;
	int xp, yp, xw, yw;

	if (radarPixmap2 == radarPixmap) {
		XSetPlaneMask(dpy, radarGC, AllPlanes & ~(dpl_1[0] | dpl_1[1]));
	}
	XSetForeground(dpy, radarGC, colors[color].pixel);

	if (Setup->x >= 256) {
		xs = (double) (256 - 1) / (Setup->x - 1);
		ys = (double) (RadarHeight - 1) / (Setup->y - 1);
		xp = (int) (xi * xs + 0.5);
		yp = RadarHeight - 1 - (int) (yi * ys + 0.5);
		XDrawPoint(dpy, radarPixmap2, radarGC, xp, yp);
	}
	else {
		xs = (double) (Setup->x - 1) / (256 - 1);
		ys = (double) (Setup->y - 1) / (RadarHeight - 1);
		/*
		 * Calculate the min and max points on the radar that would show
		 * block position 'xi' and 'yi'.  Note 'xp' is the minimum x coord
		 * for 'xi',which is one more than the previous xi value would give,
		 * and 'xw' is the maximum, which is then changed to a width value.
		 * Similarly for 'yw' and 'yp' (the roles are reversed because the
		 * radar is upside down).
		 */
		xp = (int) ((xi - 0.5) / xs) + 1;
		xw = (int) ((xi + 0.5) / xs);
		yw = (int) ((yi - 0.5) / ys) + 1;
		yp = (int) ((yi + 0.5) / ys);
		xw -= xp;
		yw = yp - yw;
		yp = RadarHeight - 1 - yp;
		XFillRectangle(dpy, radarPixmap2, radarGC, xp, yp, (unsigned) xw + 1,
			       (unsigned) yw + 1);
	}
	if (radarPixmap2 == radarPixmap)
		XSetPlaneMask(dpy, radarGC, AllPlanes & ~(dpl_2[0] | dpl_2[1]));
}

static void Paint_world_radar_new(void)
{
	int i, j, xoff, yoff;
	ipos_t min, max;
	static XPoint poly[10000];

	/* what the heck is this? */
	radar_exposures = 2;

	if (radarPixmap2 == radarPixmap)
		XSetPlaneMask(dpy, radarGC, AllPlanes & (~(dpl_1[0] | dpl_1[1])));

	if (radarPixmap2 != radarWindow) {
		/* Clear radar */
		XSetForeground(dpy, radarGC, colors[BLACK].pixel);
		XFillRectangle(dpy, radarPixmap2, radarGC, 0, 0, 256, RadarHeight);
	}
	else
		XClearWindow(dpy, radarWindow);

	XSetForeground(dpy, radarGC, colors[wallRadarColor].pixel);

	/* loop through all the polygons */
	for (i = 0; i < num_polygons; i++) {
		if (BIT(polygon_styles[polygons[i].style].flags, STYLE_INVISIBLE_RADAR))
			continue;
		Compute_radar_bounds(&min, &max, &polygons[i].bounds);
		for (xoff = min.x; xoff <= max.x; xoff++) {
			for (yoff = min.y; yoff <= max.y; yoff++) {
				int x, y;

				x = xoff * Setup->width;
				y = yoff * Setup->height;

				/* loop through the points in the current polygon */
				for (j = 0; j < polygons[i].num_points; j++) {
					x += polygons[i].points[j].x;
					y += polygons[i].points[j].y;
					poly[j].x = (x * 256) / Setup->width;
					poly[j].y =
					    (int) RadarHeight -
					    ((y * (int) RadarHeight) / Setup->height);
				}

				XSetForeground(dpy, radarGC, fullColor ?
					       polygon_styles[polygons[i].style].
					       color : colors[wallRadarColor].pixel);
				XFillPolygon(dpy, radarPixmap2, radarGC, poly,
					     polygons[i].num_points, Nonconvex, CoordModeOrigin);
			}
		}
	}

	if (radarPixmap2 == radarPixmap)
		XSetPlaneMask(dpy, radarGC, AllPlanes & (~(dpl_2[0] | dpl_2[1])));
}

void Paint_world_radar(void)
{
	if (oldServer)
		Paint_world_radar_old();
	else
		Paint_world_radar_new();
}

void Radar_show_target(int x, int y)
{
	Paint_radar_block(x, y, targetRadarColor);
}

void Radar_hide_target(int x, int y)
{
	Paint_radar_block(x, y, BLACK);
}


static bool Set_wallRadarColor(xp_option_t * opt, int value)
{
	UNUSED_PARAM(opt);

	wallRadarColor = value;
	return true;
}

static bool Set_decorRadarColor(xp_option_t * opt, int value)
{
	UNUSED_PARAM(opt);

	decorRadarColor = value;
	return true;
}

static bool Set_targetRadarColor(xp_option_t * opt, int value)
{
	UNUSED_PARAM(opt);

	targetRadarColor = value;
	return true;
}


static xp_option_t paintradar_options[] = {

	COLOR_INDEX_OPTION_WITH_SETFUNC("wallRadarColor",
					BLUE,
					&wallRadarColor,
					Set_wallRadarColor,
					"Which color number to use for drawing walls on the radar.\n"
					"Valid values all even numbers smaller than maxColors.\n"),

	COLOR_INDEX_OPTION_WITH_SETFUNC("decorRadarColor",
					6,
					&decorRadarColor,
					Set_decorRadarColor,
					"Which color number to use for drawing decorations on the radar.\n"
					"Valid values are all even numbers smaller than maxColors.\n"),

	COLOR_INDEX_OPTION_WITH_SETFUNC("targetRadarColor",
					4,
					&targetRadarColor,
					Set_targetRadarColor,
					"Which color number to use for drawing targets on the radar.\n"
					"Valid values are all even numbers smaller than maxColors.\n"),
};

void Store_paintradar_options(void)
{
	STORE_OPTIONS(paintradar_options);
}
#endif
