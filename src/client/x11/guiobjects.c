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
 * Copyright (C) 2003-2004 Kristian Söderblom
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

/* $Id: guiobjects.c,v 1.48 2007/11/23 12:17:02 kps Exp $ */

#include "xpclient_x11.h"
#include "client.h"
#include "astershape.h"
#include "wreckshape.h"
#include "setup.h"
#include "error.h"
#include "bit.h"
#include "rules.h"
#include "xinit.h"

static bool texturedShips = false;	/* Turned this off because the images drawn
					 * don't match the actual shipshape used
					 * for wall collisions by the server. */
static int ballColor;		/* Color index for ball drawing */
static int connColor;		/* Color index for connector drawing */
static int teamShotColor;	/* Color index for harmless shot drawing */
static int zeroLivesColor;	/* Color to associate with 0 lives */
static int oneLifeColor;	/* Color to associate with 1 life */
#if 0
static int twoLivesColor;	/* Color to associate with 2 lives */
static int manyLivesColor;	/* Color to associate with >2 lives */
static int selfLWColor;		/* Color index for selfLifeWarning */
static int enemyLWColor;	/* Color index for enemyLifeWarning */
static int teamLWColor;		/* Color index for teamLifeWarning */
#endif
static int shipNameColor;	/* Color index for ship name drawing */
static int mineNameColor;	/* Color index for mine name drawing */
static int teamShipColor;	/* Color index to associate with team 0 */
static int dirPtrColor;		/* Color index for dirptr drawing */

extern int fuelColor;

static int asteroidRawShapes[NUM_ASTEROID_SHAPES][NUM_ASTEROID_POINTS][2] = {
	{ASTEROID_SHAPE_0},
	{ASTEROID_SHAPE_1},
};


position_t *asteroidShapes[NUM_ASTEROID_SHAPES][NUM_ASTEROID_POINTS];


int Init_asteroids(void)
{
	int shp, i;
	size_t point_size;
	size_t total_size;
	char *dynmem;

	/*
	 * Allocate memory for all the asteroid points.
	 */
	point_size = sizeof(position_t) * RES;
	total_size = point_size * NUM_ASTEROID_POINTS * NUM_ASTEROID_SHAPES;
	if ((dynmem = (char *) malloc(total_size)) == NULL) {
		error("Not enough memory for asteroid shapes");
		return -1;
	}

	/*
	 * For each asteroid-shape rotate all points.
	 */
	for (shp = 0; shp < NUM_ASTEROID_SHAPES; shp++) {
		for (i = 0; i < NUM_ASTEROID_POINTS; i++) {
			asteroidShapes[shp][i] = (position_t *) dynmem;
			dynmem += point_size;
			asteroidShapes[shp][i][0].x = asteroidRawShapes[shp][i][0];
			asteroidShapes[shp][i][0].y = asteroidRawShapes[shp][i][1];
			Old_rotate_point(&asteroidShapes[shp][i][0]);
		}
	}

	return 0;
}


void Gui_paint_item_symbol(int type, Drawable d, GC mygc, int x, int y, int c)
{
	if (!texturedObjects) {
		gcv.stipple = itemBitmaps[type];
		gcv.fill_style = FillStippled;
		gcv.ts_x_origin = x;
		gcv.ts_y_origin = y;
		XChangeGC(dpy, mygc,
			  GCStipple | GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin, &gcv);
		rd.paintItemSymbol(type, d, mygc, x, y, c);
		XFillRectangle(dpy, d, mygc, x, y, ITEM_SIZE, ITEM_SIZE);
		gcv.fill_style = FillSolid;
		XChangeGC(dpy, mygc, GCFillStyle, &gcv);
	}
	else
		Bitmap_paint(d, BM_ALL_ITEMS, x, y, type);
}

void Gui_paint_item(int type, Drawable d, GC mygc, int x, int y)
{
	const int SIZE = ITEM_TRIANGLE_SIZE;
	XPoint points[5];

#ifndef NO_ITEM_TRIANGLES
	points[0].x = x - SIZE;
	points[0].y = y - SIZE;
	points[1].x = x;
	points[1].y = y + SIZE;
	points[2].x = x + SIZE;
	points[2].y = y - SIZE;
	points[3] = points[0];
	Set_fg_color(BLUE);
	rd.drawLines(dpy, d, mygc, points, 4, CoordModeOrigin);
#endif

	Set_fg_color(RED);
#if 0
	str[0] = itemtype_ptr[i].type + '0';
	str[1] = '\0';
	rd.drawString(dpy, d, mygc, x - XTextWidth(gameFont, str, 1) / 2, y + SIZE - 1, str, 1);
#endif
	Gui_paint_item_symbol(type, d, mygc, x - ITEM_SIZE / 2, y - SIZE + 2, ITEM_PLAYFIELD);
}

void Gui_paint_item_object(int type, int x, int y)
{
	Gui_paint_item(type, drawPixmap, gameGC, WINSCALE(X(x)), WINSCALE(Y(y)));
}

void Gui_paint_ball(int x, int y, int style)
{
	unsigned long rgb = 0;
	int color = ballColor;

	x = X(x);
	y = Y(y);

	/*
	 * kps - currently style 0xff means no style.
	 * This code assumes num_polygon_styles < 256.
	 */
	/* kps - need to handle the color right here ... */
	if (style >= 0 && style < num_polygon_styles) {
		rgb = polygon_styles[style].rgb;
	}

	if (!texturedObjects) {
		Arc_add(color, x - BALL_RADIUS, y - BALL_RADIUS, 2 * BALL_RADIUS, 2 * BALL_RADIUS,
			0, 64 * 360);
	}
	else {
		if (style == 0xff) {
			Bitmap_paint(drawPixmap, BM_BALL, WINSCALE(x - BALL_RADIUS),
				     WINSCALE(y - BALL_RADIUS), 0);
		}
		else {
			Bitmap_paint_blended(drawPixmap, BM_BALL_GRAY,
					     WINSCALE(x - BALL_RADIUS), WINSCALE(y - BALL_RADIUS),
					     rgb);
		}
	}
}


void Gui_paint_ball_connector(int x_1, int y_1, int x_2, int y_2)
{
	x_2 = X(x_2);
	y_2 = Y(y_2);
	x_1 = X(x_1);
	y_1 = Y(y_1);
	Segment_add(connColor, x_1, y_1, x_2, y_2);
}

static void Gui_paint_mine_name(int x, int y, char *name)
{
	int name_len, name_width;

	if (!name || mineNameColor == NO_COLOR)
		return;

	Set_fg_color(mineNameColor);

	name_len = strlen(name);
	name_width = ColorTextWidth(gameFont, name, name_len);

	ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
			WINSCALE(x) - name_width / 2,
			WINSCALE(y + 4) + gameFont->ascent + 1,
			name, name_len,
			mineNameColor, shadowColor, BLACK,
			false, true);
}

void Gui_paint_mine(int x, int y, int teammine, char *name)
{
	if (!texturedObjects) {
		static double lastScaleFactor;
		static XPoint mine_points[21];
		static XPoint world_mine_points[21] = {
			{0, 0},
			{1, 0},
			{0, -1},
			{4, 0},
			{0, -1},
			{6, 0},
			{0, 1},
			{4, 0},
			{0, 1},
			{1, 0},
			{0, 2},
			{-1, 0},
			{0, 1},
			{-4, 0},
			{0, 1},
			{-6, 0},
			{0, -1},
			{-4, 0},
			{0, -1},
			{-1, 0},
			{0, -2}
		};

		if (lastScaleFactor != scaleFactor) {
			int i;
			lastScaleFactor = scaleFactor;
			for (i = 1; i < 21; ++i) {
				mine_points[i].x = WINSCALE(world_mine_points[i].x);
				mine_points[i].y = WINSCALE(world_mine_points[i].y);
			}
		}

		x = X(x);
		y = Y(y);
		mine_points[0].x = WINSCALE(x - 8);
		mine_points[0].y = WINSCALE(y - 1);
		if (teammine == 0) {
			Set_fg_color(BLUE);
			rd.fillRectangle(dpy, drawPixmap, gameGC,
					 WINSCALE(x - 7), WINSCALE(y - 2), UWINSCALE(15),
					 UWINSCALE(5));
		}

		Set_fg_color(WHITE);
		rd.drawLines(dpy, drawPixmap, gameGC, mine_points, 21, CoordModePrevious);

		if (name)
			Gui_paint_mine_name(x, y, name);
	}
	else {
		x = X(x);
		y = Y(y);
		if (teammine == 0) {
			Set_fg_color(BLUE);
			Bitmap_paint(drawPixmap, BM_MINE_OTHER, WINSCALE(x - 10), WINSCALE(y - 7),
				     0);
		}
		else {
			Set_fg_color(WHITE);
			Bitmap_paint(drawPixmap, BM_MINE_TEAM, WINSCALE(x - 10), WINSCALE(y - 7),
				     0);
		}

		if (name)
			Gui_paint_mine_name(x, y, name);
	}
}


void Gui_paint_spark(int color, int x, int y)
{
	color = spark_color[color];

	Rectangle_add(color, x - sparkSize / 2, y - sparkSize / 2, sparkSize, sparkSize);

}


void Gui_paint_wreck(int x, int y, bool deadly, int wtype, int rot, int size)
{
	int color, cnt, tx, ty;
	static XPoint points[NUM_WRECKAGE_POINTS + 2];

	for (cnt = 0; cnt < NUM_WRECKAGE_POINTS; cnt++) {
		tx = (int) (wreckageShapes[wtype][cnt][rot].x * size) >> 8;
		ty = (int) (wreckageShapes[wtype][cnt][rot].y * size) >> 8;

		points[cnt].x = WINSCALE(X(x + tx));
		points[cnt].y = WINSCALE(Y(y + ty));

	}
	points[cnt++] = points[0];

	color = (deadly) ? WHITE : RED;
	Set_fg_color(color);
	rd.drawLines(dpy, drawPixmap, gameGC, points, cnt, 0);
}

void Gui_paint_asteroids_begin(void)
{
}

void Gui_paint_asteroids_end(void)
{
}

void Gui_paint_asteroid(int x, int y, int type, int rot, int size)
{
	int cnt, tx, ty;
	static XPoint points[NUM_ASTEROID_POINTS + 2];

	type = type % NUM_ASTEROID_SHAPES;
	for (cnt = 0; cnt < NUM_ASTEROID_POINTS; cnt++) {
		tx = (int) (asteroidShapes[type][cnt][rot].x * size * 1.4);
		ty = (int) (asteroidShapes[type][cnt][rot].y * size * 1.4);

		points[cnt].x = WINSCALE(X(x + tx));
		points[cnt].y = WINSCALE(Y(y + ty));
	}
	points[cnt++] = points[0];

	Set_fg_color(WHITE);
	rd.drawLines(dpy, drawPixmap, gameGC, points, cnt, 0);
}


static void Gui_paint_nastyshot(int color, int x, int y, int size)
{
	int z = size;

	if (rfrac() < 0.5) {
		Segment_add(color, x - z, y - z, x + z, y + z);
		Segment_add(color, x + z, y - z, x - z, y + z);
	}
	else {
		Segment_add(color, x - z, y, x + z, y);
		Segment_add(color, x, y - z, x, y + z);
	}
}


void Gui_paint_fastshot(int color, int x, int y)
{
	/* this is for those pesky invisible shots */
	if (color == 0)
		return;

	if (!texturedObjects) {
		int z = shotSize / 2;

		if (showNastyShots)
			Gui_paint_nastyshot(color, x, y, z);
		else {
			/* Show round shots - jiman392 */
			if (shotSize > 2) {
				Set_fg_color(color);
				rd.fillArc(dpy, drawPixmap, gameGC,
					   WINSCALE(x - z), WINSCALE(y - z),
					   UWINSCALE(shotSize), UWINSCALE(shotSize), 0, 64 * 360);
			}
			else
				Rectangle_add(color, x - z, y - z, shotSize, shotSize);
		}
	}
	else {
		int s_size = MIN(shotSize, 16);
		int z = s_size / 2;

		Bitmap_paint(drawPixmap, BM_BULLET, WINSCALE(x) - z, WINSCALE(y) - z, s_size - 1);
	}
}

void Gui_paint_teamshot(int color, int x, int y)
{
	color = teamShotColor;

	if (color == NO_COLOR)
		return;

	if (!texturedObjects) {
		int z = teamShotSize / 2;

		if (showNastyShots)
			Gui_paint_nastyshot(color, x, y, z);
		else {
			/* Show round shots - jiman392 */
			if (teamShotSize > 2) {
				Set_fg_color(color);
				rd.fillArc(dpy, drawPixmap, gameGC,
					   WINSCALE(x - z), WINSCALE(y - z),
					   UWINSCALE(teamShotSize), UWINSCALE(teamShotSize), 0,
					   64 * 360);
			}
			else
				Rectangle_add(color, x - z, y - z, teamShotSize, teamShotSize);
		}
	}
	else {
		int s_size = MIN(teamShotSize, 16);
		int z = s_size / 2;
		Bitmap_paint(drawPixmap, BM_BULLET_OWN, WINSCALE(x) - z, WINSCALE(y) - z,
			     s_size - 1);
	}
}


void Gui_paint_missiles_begin(void)
{
	Set_fg_color(WHITE);
	XSetLineAttributes(dpy, gameGC, 4, LineSolid, CapButt, JoinMiter);
}


void Gui_paint_missiles_end(void)
{
	XSetLineAttributes(dpy, gameGC, 0, LineSolid, CapButt, JoinMiter);
}


void Gui_paint_missile(int x, int y, int len, int dir)
{
	int x_1, x_2, y_1, y_2;

	x_1 = X(x);
	y_1 = Y(y);
	x_2 = (int) (x_1 - tcos(dir) * len);
	y_2 = (int) (y_1 + tsin(dir) * len);
	rd.drawLine(dpy, drawPixmap, gameGC, WINSCALE(x_1), WINSCALE(y_1), WINSCALE(x_2),
		    WINSCALE(y_2));
}


void Gui_paint_lasers_begin(void)
{
	XSetLineAttributes(dpy, gameGC, 3, LineSolid, CapButt, JoinMiter);
}


void Gui_paint_lasers_end(void)
{
	XSetLineAttributes(dpy, gameGC, 0, LineSolid, CapButt, JoinMiter);
}


void Gui_paint_laser(int color, int x_1, int y_1, int len, int dir)
{
	int x_2, y_2;

	x_2 = (int) (x_1 + len * tcos(dir));
	y_2 = (int) (y_1 + len * tsin(dir));
	if ((unsigned) (color) >= NUM_COLORS)
		color = WHITE;
	Set_fg_color(color);
	rd.drawLine(dpy, drawPixmap, gameGC, WINSCALE(X(x_1)), WINSCALE(Y(y_1)), WINSCALE(X(x_2)),
		    WINSCALE(Y(y_2)));
}

#if 0
void Gui_paint_paused(int x, int y, int count)
{
	if (!texturedObjects) {
		int x_0, y_0;
		static int pauseCharWidth = -1;
		const unsigned half_pause_size = 3 * BLOCK_SZ / 7;

		if (pauseCharWidth < 0)
			pauseCharWidth = XTextWidth(gameFont, "P", 1);

		Set_fg_color(BLUE);
		x_0 = X(x - half_pause_size);
		y_0 = Y(y + half_pause_size);
		rd.fillRectangle(dpy, drawPixmap, gameGC,
				 WINSCALE(x_0), WINSCALE(y_0),
				 UWINSCALE(2 * half_pause_size + 1),
				 UWINSCALE(2 * half_pause_size + 1));
		if (count <= 0 || twelveHz % 10 >= 5) {
			Set_fg_color(WHITE);
			rd.drawRectangle(dpy, drawPixmap, gameGC,
					 WINSCALE(x_0 - 1),
					 WINSCALE(y_0 - 1),
					 UWINSCALE(2 * (half_pause_size + 1)),
					 UWINSCALE(2 * (half_pause_size + 1)));
			rd.drawString(dpy, drawPixmap, gameGC, WINSCALE(X(x)) - pauseCharWidth / 2,
				      WINSCALE(Y(y - 1)) + gameFont->ascent / 2, "P", 1);
		}
	}
	else
		Bitmap_paint(drawPixmap, BM_PAUSED, WINSCALE(X(x - BLOCK_SZ / 2)),
			     WINSCALE(Y(y + BLOCK_SZ / 2)), (count <= 0
							     || twelveHz % 10 >= 5) ? 1 : 0);
}
#endif

void Gui_paint_appearing(int x, int y, int id, int count)
{
	const unsigned hsize = 3 * BLOCK_SZ / 7;
	/*other_t *other = Other_by_id(id); */
	int color = RED;	/*other ? Player_state_color(other) : 0; */
	homebase_t *base = Homebase_by_id(id);
	float angle = 0;

	/*
	 * A blinking rectangle to wake up the player.
	 */
	if (baseWarningType & 1) {
		if ((twelveHz & 1))
			color = WHITE;

		Set_fg_color(color);

		rd.drawRectangle(dpy, drawPixmap, gameGC,
				 SCALEX(x - (int) hsize),
				 SCALEY(y - (int) hsize + (int) ((360 - count) / 180. * hsize + 1)),
				 UWINSCALE(2 * hsize + 1),
				 UWINSCALE((unsigned) ((360 - count) / 180. * hsize + 1)));
	}

	if (baseWarningType & 4) {
		if (base != NULL) {
			switch (base->type) {
			default:
				angle = M_PI / 2;
				break;
			case SETUP_BASE_DOWN:
				angle = 3 * M_PI / 2;
				break;
			case SETUP_BASE_LEFT:
				angle = M_PI;
				break;
			case SETUP_BASE_RIGHT:
				angle = 0;
				break;
			}
		}

		Gui_paint_ship(x, y, angle, id, 0, 0, 0, 0, 0);
			       /* cloak, int phased, int shield,
				  int deflector, int eshield */
	}
}


void Gui_paint_ecm(int x, int y, int size)
{
	Arc_add(WHITE, X(x - size / 2), Y(y + size / 2), size, size, 0, 64 * 360);
}


void Gui_paint_refuel(int x_0, int y_0, int x_1, int y_1)
{
	if (!texturedObjects) {
		if (fuelColor > 0)
			Set_fg_color(fuelColor);
		else
			Set_fg_color(RED);
		rd.drawLine(dpy, drawPixmap, gameGC,
			    WINSCALE(X(x_0)), WINSCALE(Y(y_0)), WINSCALE(X(x_1)), WINSCALE(Y(y_1)));
	}
	else {
		int size = WINSCALE(8);
		double dx, dy;
		int i;
		int fuel[16] = { 1, 2, 3, 3, 2, 1, 0, 1, 2, 3, 2, 1, 2, 3, 3, 2 };

		x_0 = WINSCALE(X(x_0));
		y_0 = WINSCALE(Y(y_0));
		x_1 = WINSCALE(X(x_1));
		y_1 = WINSCALE(Y(y_1));
		dx = (double) (x_1 - x_0) / 16;
		dy = (double) (y_1 - y_0) / 16;
		for (i = 0; i < 16; i++) {
			Bitmap_paint(drawPixmap, BM_REFUEL,
				     (int) (x_0 + (dx * i) - size / 2),
				     (int) (y_0 + (dy * i) - size / 2),
				     fuel[(loops + 16 - i) % 16]);
		}
	}
}


void Gui_paint_connector(int x_0, int y_0, int x_1, int y_1, int tractor)
{
	if (connColor == NO_COLOR)
		return;
	Set_fg_color(connColor);
	if (tractor)
		rd.setDashes(dpy, gameGC, 0, cdashes, NUM_CDASHES);
	else
		rd.setDashes(dpy, gameGC, 0, dashes, NUM_DASHES);

	rd.drawLine(dpy, drawPixmap, gameGC, WINSCALE(X(x_0)), WINSCALE(Y(y_0)), WINSCALE(X(x_1)),
		    WINSCALE(Y(y_1)));
	if (tractor)
		rd.setDashes(dpy, gameGC, 0, dashes, NUM_DASHES);
}


void Gui_paint_transporter(int x_0, int y_0, int x_1, int y_1)
{
	if (connColor == NO_COLOR)
		return;
	Set_fg_color(connColor);
	rd.drawLine(dpy, drawPixmap, gameGC, WINSCALE(X(x_0)), WINSCALE(Y(y_0)), WINSCALE(X(x_1)),
		    WINSCALE(Y(y_1)));
}


void Gui_paint_all_connectors_begin(void)
{
	unsigned long mask;

	if (gcv.line_style != LineOnOffDash) {
		gcv.line_style = LineOnOffDash;
		mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
		mask |= GCDashOffset;
#endif
		XChangeGC(dpy, gameGC, mask, &gcv);
	}
}


void Gui_paint_ships_begin(void)
{
	gcv.dash_offset = WINSCALE(DASHES_LENGTH - (loops % DASHES_LENGTH));
}


void Gui_paint_ships_end(void)
{
	unsigned long mask;
	if (gcv.line_style != LineSolid) {
		gcv.line_style = LineSolid;
		mask = GCLineStyle;
		XChangeGC(dpy, gameGC, mask, &gcv);
	}
	gcv.dash_offset = 0;
}


/*  Here starts the paint functions for ships  (MM) */
static void Gui_paint_ship_name(int x, int y, other_t * other)
{
	int color = Player_state_color(other);

	if (color != NO_COLOR && color != WHITE)
		;
	else if (shipNameColor != NO_COLOR)
		color = shipNameColor;
	else
		color = BLUE;

	Set_fg_color(color);
	if (shipNameColor != NO_COLOR) {
		int name_width = Other_name_width(other, maxCharsInNames);

		ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
				WINSCALE(X(x)) - name_width / 2,
				WINSCALE(Y(y) + 16) + gameFont->ascent,
				other->abbrev_nick_name,
				strlen(other->abbrev_nick_name),
				color, shadowColor, BLACK,
				false, true);
	}

	if (showLivesByShip && BIT(Setup->mode, LIMITED_LIVES)) {
		char keff[4] = "";

		sprintf(keff, "%d", other->life);
		ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
				WINSCALE(X(x) + SHIP_SZ),
				WINSCALE(Y(y) - SHIP_SZ) + gameFont->ascent,
				keff, strlen(keff),
				color, shadowColor, BLACK,
				false, true);
	}
}


static int Gui_is_my_tank(other_t * other)
{
	char tank_name[MAX_NAME_LEN];

	if (self == NULL || other == NULL || other->mychar != 'T' || (BIT(Setup->mode, TEAM_PLAY)
								      && self->team != other->team)) {
		return 0;
	}

	if (strlcpy(tank_name, self->nick_name, MAX_NAME_LEN) < MAX_NAME_LEN)
		strlcat(tank_name, "'s tank", MAX_NAME_LEN);

	if (strcmp(tank_name, other->nick_name))
		return 0;

	return 1;
}

/*
 * Calculate color of ship 'other' when viewed
 * by player with id 'eyesId'. Returns blue
 * for harmless ships, white for self or enemy.
 */
int Gui_calculate_ship_color(other_t * other)
{
	int ship_color = WHITE;
	int id = other ? other->id : -1;
	other_t *eyes = Other_by_id(eyesId);

	/* Team-mates are blue. */
	if (BIT(Setup->mode, TEAM_PLAY)
	    && eyes != NULL && eyes->id != id && other != NULL && eyes->team == other->team)
		return BLUE;

	/* Allies are blue. */
	if (eyes != NULL
	    && eyes->id != id && other != NULL && eyes->alliance != ' '
	    && eyes->alliance == other->alliance)
		return BLUE;

	/* Own tanks are blue. */
	if (Gui_is_my_tank(other))
		return BLUE;

	ship_color = Player_state_color(other);
	if (ship_color != NO_COLOR)
		return ship_color;

	return WHITE;
}


#if 0
static void Gui_paint_marking_lights(int id, int x, int y, shipshape_t * ship, int dir)
{
	int lcnt;

	if (((twelveHz + id) & 0xF) == 0) {
		for (lcnt = 0; lcnt < ship->num_l_light; lcnt++) {
			position_t l_light = Ship_get_l_light_position(ship, lcnt, dir);
			Rectangle_add(RED, X(x + l_light.x) - 2, Y(y + l_light.y) - 2, 6, 6);
			Segment_add(RED,
				    X(x + l_light.x) - 8, Y(y + l_light.y), X(x + l_light.x) + 8,
				    Y(y + l_light.y));
			Segment_add(RED, X(x + l_light.x), Y(y + l_light.y) - 8, X(x + l_light.x),
				    Y(y + l_light.y) + 8);
		}
	}
	else if (((twelveHz + id) & 0xF) == 2) {
		for (lcnt = 0; lcnt < ship->num_r_light; lcnt++) {
			int rightLightColor = maxColors > 4 ? 4 : BLUE;
			position_t r_light = Ship_get_r_light_position(ship, lcnt, dir);
			Rectangle_add(rightLightColor, X(x + r_light.x) - 2, Y(y + r_light.y) - 2,
				      6, 6);
			Segment_add(rightLightColor, X(x + r_light.x) - 8, Y(y + r_light.y),
				    X(x + r_light.x) + 8, Y(y + r_light.y));
			Segment_add(rightLightColor, X(x + r_light.x), Y(y + r_light.y) - 8,
				    X(x + r_light.x), Y(y + r_light.y) + 8);
		}
	}
}
#endif

static void Gui_paint_shields_deflectors(int x, int y, int radius, int shield,
					 int deflector, int eshield, int ship_color)
{
	int e_radius = radius + 4;
	int half_radius = radius >> 1;
	int half_e_radius = e_radius >> 1;
	int scolor = -1;
	int ecolor = -1;

	if (shield)
		scolor = ship_color;
	if (deflector)
		ecolor = twelveHz & 0x02 ? RED : BLUE;
	if (eshield && shield) {
		if (ecolor != -1) {
			scolor = ecolor;
			ecolor = ship_color;
		}
		else
			scolor = ecolor = ship_color;
	}

	if (ecolor != -1) {	/* outer shield */
		Set_fg_color(ecolor);
		rd.drawArc(dpy, drawPixmap, gameGC,
			   WINSCALE(X(x - half_e_radius)),
			   WINSCALE(Y(y + half_e_radius)),
			   (unsigned) WINSCALE(e_radius), (unsigned) WINSCALE(e_radius), 0,
			   64 * 360);
	}
	if (scolor != -1) {
		Set_fg_color(scolor);
		rd.drawArc(dpy, drawPixmap, gameGC,
			   WINSCALE(X(x - half_radius)),
			   WINSCALE(Y(y + half_radius)),
			   (unsigned) WINSCALE(radius), (unsigned) WINSCALE(radius), 0, 64 * 360);
	}
}

static void Set_drawstyle_dashed(int ship_color);

static void Gui_paint_ship_cloaked(int ship_color, XPoint * points, int point_count)
{
	Set_drawstyle_dashed(ship_color);
	rd.drawLines(dpy, drawPixmap, gameGC, points, point_count, 0);
}

static void Gui_paint_ship_phased(int ship_color, XPoint * points, int point_count)
{
	Gui_paint_ship_cloaked(ship_color, points, point_count);
}

static void generic_paint_ship(int x, int y, float angle, int ship)
{
	int bmp = angle_to_int_dir(angle);

	Bitmap_paint(drawPixmap, ship, WINSCALE(X(x) - 16), WINSCALE(Y(y) - 16), bmp);
}


static void Gui_paint_ship_uncloaked(int id, XPoint * points, int ship_color, int point_count)
{
	if (gcv.line_style != LineSolid) {
		gcv.line_style = LineSolid;
		XChangeGC(dpy, gameGC, GCLineStyle, &gcv);
	}
	Set_fg_color(ship_color);
	rd.drawLines(dpy, drawPixmap, gameGC, points, point_count, 0);

	if (lock_id == id && id != -1 && lock_dist != 0)
		rd.fillPolygon(dpy, drawPixmap, gameGC, points, point_count, Complex,
			       CoordModeOrigin);
}


static void Set_drawstyle_dashed(int ship_color)
{
	unsigned long mask;
	if (gcv.line_style != LineOnOffDash) {
		gcv.line_style = LineOnOffDash;
		mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
		mask |= GCDashOffset;
#endif
		XChangeGC(dpy, gameGC, mask, &gcv);
	}
	Set_fg_color(ship_color);
}


static int set_shipshape(int world_x, int world_y, float angle, shipshape_t * ship, XPoint * points)
{
	int cnt;
	position_t ship_point_pos;
	XPoint *xpts = points;
	double off_x, off_y;

	for (cnt = 0; cnt < ship->shape.num_points; cnt++) {
		ship_point_pos = Ship_get_point_position(ship, cnt, angle);
		off_x = ship_point_pos.x / scaleFactor;
		if (off_x > 0.0)
			off_x += 0.5;
		else if (off_x < 0.0)
			off_x -= 0.5;
		off_y = ship_point_pos.y / scaleFactor;
		if (off_y > 0.0)
			off_y += 0.5;
		else if (off_y < 0.0)
			off_y -= 0.5;
		xpts->x = (short) (((world_x - world.x) / scaleFactor) + off_x);
		xpts->y = (short) (((world.y + ext_view_height - world_y)
				    / scaleFactor) - off_y);
		xpts++;
	}
	points[cnt++] = points[0];

	return cnt;
}


void Gui_paint_ship(int x, int y, float angle, int id, int cloak, int phased, int shield,
		    int deflector, int eshield)
{
	int cnt, ship_color;
	other_t *other;
	shipshape_t *ship;
	XPoint points[64];
	int ship_shape;

	ship = Ship_by_id(id);
	other = Other_by_id(id);
	ship_color = WHITE;

	/* mara attempts similar behaviour to the kth ss hack */
	if ((!showShipShapes)
	    && (self != NULL)
	    && (self->id != id))
		cnt = set_shipshape(x, y, angle, Default_ship(), points);
	else if ((!showMyShipShape)
		 && (self != NULL)
		 && (self->id == id))
		cnt = set_shipshape(x, y, angle, Default_ship(), points);
	else
		cnt = set_shipshape(x, y, angle, ship, points);

	/*
	 * Determine if the name of the player should be drawn below
	 * his/her ship.
	 */
	if (other != NULL && other != self)
		Gui_paint_ship_name(x, y, other);

	ship_color = Gui_calculate_ship_color(other);
	if (ship_color == BLUE)
		ship_color = teamShipColor;
	if (ship_color == NO_COLOR)
		return;

	Set_fg_color(ship_color);
	if (self != NULL && self->id == id && selfVisible && dirPtrColor != NO_COLOR) {
		int color = dirPtrColor;
		if (Bms_test_state(BmsCover) && msgScanCoverColor != NO_COLOR)
			color = msgScanCoverColor;
		if (Bms_test_state(BmsBall) && msgScanBallColor != NO_COLOR)
			color = msgScanBallColor;
		Segment_add(color,
			    (int) (ext_view_width / 2 +
				   (100 - 20) * cos(angle)),
			    (int) (ext_view_height / 2 -
				   (100 - 20) * sin(angle)),
			    (int) (ext_view_width / 2 + 100 * cos(angle)),
			    (int) (ext_view_height / 2 - 100 * sin(angle)));
#if 0
		Segment_add(color,
			    (X(x + SHIP_SZ * cos(angle))),
			    (Y(y + SHIP_SZ * sin(angle))),
			    (X(x + (SHIP_SZ + 12) * cos(angle))),
			    (Y(y + (SHIP_SZ + 12) * sin(angle))));
#endif
	}

	if (cloak == 0 && phased == 0) {
		if (!texturedObjects || !texturedShips) {
			Gui_paint_ship_uncloaked(id, points, ship_color, cnt);
			/* shipshapeshack by Mara */
			if (showHitArea) {
				Segment_add(ship_color,
					    (X(x + SHIP_SZ * cos(angle))),
					    (Y(y + SHIP_SZ * sin(angle))),
					    (X(x + (SHIP_SZ + 12) * cos(angle))),
					    (Y(y + (SHIP_SZ + 12) * sin(angle))));
				Arc_add(ship_color, X(x - SHIP_SZ), Y(y + SHIP_SZ), 2 * SHIP_SZ,
					2 * SHIP_SZ, 0, 64 * 360);
			}
		}
		else {
			if (ship_color == BLUE)
				ship_shape = BM_SHIP_FRIEND;
			else if (self != NULL && self->id != id)
				ship_shape = BM_SHIP_ENEMY;
			else
				ship_shape = BM_SHIP_SELF;

			generic_paint_ship(x, y, angle, ship_shape);
		}

	}

	if (phased)
		Gui_paint_ship_phased(ship_color, points, cnt);
	else if (cloak)
		Gui_paint_ship_cloaked(ship_color, points, cnt);

#if 0
	if (markingLights)
		Gui_paint_marking_lights(id, x, y, ship, dir);
#endif

	if (shield || deflector) {
		Set_drawstyle_dashed(ship_color);
		Gui_paint_shields_deflectors(x, y, ship->shield_radius, shield, deflector, eshield,
					     ship_color);
	}
}

int Player_state_color(other_t *other)
{
	if (other == NULL)
		return NO_COLOR;

	if (other->mychar == 'D' || other->mychar == 'P' || other->mychar == 'W')
		return NO_COLOR;

	if (BIT(Setup->mode, LIMITED_LIVES)) {
		int color = WHITE;
		/*
		 * If a life color is off, we return WHITE.
		 */
		if (other->life == 0)
			color = zeroLivesColor;
		else if (other->life == 1)
			color = oneLifeColor;
		
		return color == NO_COLOR ? WHITE : color;
	}

	return WHITE;
}


static xp_option_t guiobject_options[] = {
	COLOR_INDEX_OPTION("teamShotColor",
			   BLUE,
			   &teamShotColor,
			   "Which color number to use for drawing harmless shots.\n"),

	COLOR_INDEX_OPTION("ballColor",
			   WHITE,
			   &ballColor,
			   "Which color number to use for drawing balls.\n"),

	COLOR_INDEX_OPTION("connColor",
			   BLUE,
			   &connColor,
			   "Which color number to use for drawing connectors.\n"),

	COLOR_INDEX_OPTION("zeroLivesColor",
			   RED,
			   &zeroLivesColor,
			   "Which color to associate with ships with zero lives left.\n"
			   "This can be used to paint for example ship and base names.\n"),

	COLOR_INDEX_OPTION("oneLifeColor",
			   YELLOW,
			   &oneLifeColor,
			   "Which color to associate with ships with one life left.\n"
			   "This can be used to paint for example ship and base names.\n"),

#if 0
	COLOR_INDEX_OPTION("twoLivesColor",
			   4,
			   &twoLivesColor,
			   "Which color to associate with ships with two lives left.\n"
			   "This can be used to paint for example ship and base names.\n"),

	COLOR_INDEX_OPTION("manyLivesColor",
			   0,
			   &manyLivesColor,
			   "Which color to associate with ships with more than two lives left.\n"
			   "This can be used to paint for example ship and base names.\n"),

	COLOR_INDEX_OPTION("selfLWColor",
			   3,
			   &selfLWColor,
			   "Which color to use to paint your ship in when on last life.\n"
			   "Original color for this is red.\n"),

	COLOR_INDEX_OPTION("enemyLWColor",
			   3,
			   &enemyLWColor,
			   "Which color to use to paint enemy ships in when on last life.\n"
			   "Original color for this is red.\n"),

	COLOR_INDEX_OPTION("teamLWColor",
			   2,
			   &teamLWColor,
			   "Which color to use to paint teammate ships in when on last life.\n"
			   "Original color for this is green.\n"),
#endif

	COLOR_INDEX_OPTION("shipNameColor",
			   BLUE,
			   &shipNameColor,
			   "Which color number to use for drawing names of ships\n"
			   "(unless drawn in one of the life colors).\n"),

	COLOR_INDEX_OPTION("mineNameColor",
			   WHITE,
			   &mineNameColor,
			   "Which color number to use for drawing names of mines.\n"),

	COLOR_INDEX_OPTION("teamShipColor",
			   BLUE,
			   &teamShipColor,
			   "Which color number to use for drawing your teammates.\n"),

	COLOR_INDEX_OPTION("dirPtrColor",
			   NO_COLOR,	/* off */
			   &dirPtrColor,
			   "Which color number to use for drawing the direction pointer hack.\n"),
};

void Store_guiobject_options(void)
{
	STORE_OPTIONS(guiobject_options);
}
