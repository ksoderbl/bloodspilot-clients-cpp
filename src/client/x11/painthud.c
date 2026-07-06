/*
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 1991-2004 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

/* $Id: painthud.c,v 1.90 2007/11/20 12:47:45 kps Exp $ */

#include "xpclient_x11.h"
#include "frame.h"
#include "client.h"
#include "setup.h"
#include "rules.h"
#include "error.h"
#include "bit.h"

static int hudColor;		/* Color index for HUD drawing */
static int fuelGaugeColor;	/* Color index for fuel gauge drawing */
#if 0
static int fuelMeterColor;	/* Color index for fuel meter */
static int powerMeterColor;	/* Color index for power meter */
static int turnSpeedMeterColor;	/* Color index for turnspeed meter */
static int packetSizeMeterColor;	/* Color index for packet size meter */
static int packetLossMeterColor;	/* Color index for packet loss meter */
static int packetDropMeterColor;	/* Color index for packet drop meter */
static int packetLagMeterColor;	/* Color index for packet lag meter */
#endif
static int temporaryMeterColor;	/* Color index for temporary meter drawing */
static int meterBorderColor;	/* Color index for meter border drawing */
static int scoreObjectColor;	/* Color index for map score objects */

static int meterWidth = 45;
static int meterHeight = 9;


xp_option_t hud_options[] = {

	COLOR_INDEX_OPTION("hudColor",
			   GREEN,
			   &hudColor,
			   "Which color number to use for drawing the HUD.\n"),

	COLOR_INDEX_OPTION("fuelGaugeColor",
			   NO_COLOR,
			   &fuelGaugeColor,
			   "Which color number to use for drawing the fuel gauge.\n"),

#if 0
	COLOR_INDEX_OPTION("fuelMeterColor",
			   GREEN,
			   &fuelMeterColor,
			   "Which color number to use for drawing the fuel meter.\n"),

	COLOR_INDEX_OPTION("powerMeterColor",
			   GREEN,
			   &powerMeterColor,
			   "Which color number to use for drawing the power meter.\n"),

	COLOR_INDEX_OPTION("turnSpeedMeterColor",
			   WHITE,
			   &turnSpeedMeterColor,
			   "Which color number to use for drawing the turn speed meter.\n"),

	COLOR_INDEX_OPTION("packetSizeMeterColor",
			   WHITE,
			   &packetSizeMeterColor,
			   "Which color number to use for drawing the packet size meter.\n"
			   "Each bar is equavalent to 1024 bytes, for a maximum of 4096 bytes.\n"),

	COLOR_INDEX_OPTION("packetLossMeterColor",
			   WHITE,
			   &packetLossMeterColor,
			   "Which color number to use for drawing the packet loss meter.\n"
			   "This gives the percentage of lost frames due to network failure.\n"),

	COLOR_INDEX_OPTION("packetDropMeterColor",
			   BLUE,
			   &packetDropMeterColor,
			   "Which color number to use for drawing the packet drop meter.\n"
			   "This gives the percentage of dropped frames due to display\n"
			   "slowness.\n"),

	COLOR_INDEX_OPTION("packetLagMeterColor",
			   YELLOW,
			   &packetLagMeterColor,
			   "Which color number to use for drawing the packet lag meter.\n"
			   "This gives the amount of lag in frames over the past one second.\n"),
#endif
	COLOR_INDEX_OPTION("temporaryMeterColor",
			   RED,
			   &temporaryMeterColor,
			   "Which color number to use for drawing temporary meters.\n"),

	COLOR_INDEX_OPTION("meterBorderColor",
			   BLUE,
			   &meterBorderColor,
			   "Which color number to use for drawing borders of meters.\n"),

	COLOR_INDEX_OPTION("scoreObjectColor",
			   GREEN,
			   &scoreObjectColor,
			   "Which color number to use for drawing score objects.\n"),

};


void Store_hud_options(void)
{
	STORE_OPTIONS(hud_options);
}


/*
 * Draw a meter of some kind on screen.
 * When the x-offset is specified as a negative value then
 * the meter is drawn relative to the right side of the screen,
 * otherwise from the normal left side.
 */
static void Paint_meter(int xoff, int y, const char *title, int val, int max, int meter_color)
{
	const int mw1_4 = meterWidth / 4,
	    mw2_4 = meterWidth / 2, mw3_4 = 3 * meterWidth / 4, mw4_4 = meterWidth, BORDER = 5;
	int x, xstr;

	if (xoff >= 0) {
		x = xoff;
		xstr = (x + (int) meterWidth) + BORDER;
	}
	else {
		x = draw_width - ((int) meterWidth - xoff);
		xstr = (x)
		    - (BORDER + ColorTextWidth(gameFont, title, (int) strlen(title)));
	}

	if (meter_color != NO_COLOR) {
		Set_fg_color(meter_color);
		rd.fillRectangle(dpy, drawPixmap, gameGC,
				 x + 2, y + 2, (int) (((meterWidth - 3) * val) / (max ? max : 1)),
				 meterHeight - 3);
	}

	/* meterBorderColor == NO_COLOR means no meter borders are drawn */
	if (meterBorderColor != NO_COLOR) {
		int color = meterBorderColor;

		Set_fg_color(color);
		rd.drawRectangle(dpy, drawPixmap, gameGC, (x), (y), (meterWidth), (meterHeight));

		/* Paint scale levels(?) */
		Segment_add_unscaled(color, x, y - 4, x, y + meterHeight + 4, true);
		Segment_add_unscaled(color, x + mw4_4, y - 4, x + mw4_4, y + meterHeight + 4, true);
		Segment_add_unscaled(color, x + mw2_4, y - 3, x + mw2_4, y + meterHeight + 3, true);
		Segment_add_unscaled(color, x + mw1_4, y - 1, x + mw1_4, y + meterHeight + 1, true);
		Segment_add_unscaled(color, x + mw3_4, y - 1, x + mw3_4, y + meterHeight + 1, true);
	}

	if (meterBorderColor == NO_COLOR)
		Set_fg_color(meter_color);

	ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
			xstr, (y) + (gameFont->ascent + meterHeight) / 2, title, (int) strlen(title),
			meterBorderColor == NO_COLOR ? meter_color : meterBorderColor,
			shadowColor, BLACK,
			false, true);

	/* texturedObjects - TODO */
	/*int width = ((int)(((meterWidth-3)*val)/(max?max:1))); */

	/*printf("TODO: implement paint meter\n"); */
	/*PaintMeter(drawPixmap, BM_METER,
	   (x), (y),
	   (meterWidth), (11),
	   width); */
	/*Set_fg_color(color); */
}


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


void Paint_score_objects(void)
{
	int i, x, y;

	if (scoreObjectColor == NO_COLOR)
		return;

	for (i = 0; i < MAX_SCORE_OBJECTS; i++) {
		score_object_t *sobj = &score_objects[i];

		if (sobj->life_time <= 0)
			continue;

		if (twelveHz % 3) {
			x = sobj->x * BLOCK_SZ + BLOCK_SZ / 2;
			y = sobj->y * BLOCK_SZ + BLOCK_SZ / 2;
			if (wrap(&x, &y)) {
				if (sobj->msg_width == -1)
					sobj->msg_width =
						ColorTextWidth(gameFont, sobj->msg, sobj->msg_len);
				Set_fg_color(scoreObjectColor);
				x = WINSCALE(X(x)) - sobj->msg_width / 2;
				y = WINSCALE(Y(y)) + gameFont->ascent / 2;
				ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
						x, y, sobj->msg, sobj->msg_len,
						scoreObjectColor, shadowColor, BLACK,
						false, true);
			}
		}
		sobj->life_time -= timePerFrame;
		if (sobj->life_time <= 0.0) {
			sobj->life_time = 0.0;
			sobj->hud_msg_len = 0;
		}
	}
}


void Paint_meters(void)
{
#if 0
	int y = 20, color;

	if (fuelMeterColor != NO_COLOR)
		Paint_meter(-10, y += 20, "Fuel", fuelSum, fuelMax, fuelMeterColor);

	if (powerMeterColor != NO_COLOR)
		color = powerMeterColor;
	else if (controlTime > 0.0)
		color = temporaryMeterColor;
	else
		color = NO_COLOR;

	if (color != NO_COLOR)
		Paint_meter(-10, y +=
			    20, "Power", (int) displayedPower, (int) MAX_PLAYER_POWER, color);

	if (turnSpeedMeterColor != NO_COLOR)
		color = turnSpeedMeterColor;
	else if (controlTime > 0.0)
		color = temporaryMeterColor;
	else
		color = NO_COLOR;

	if (color != NO_COLOR)
		Paint_meter(-10, y +=
			    20, "Turnspeed", (int) displayedTurnspeed, (int) MAX_PLAYER_TURNSPEED,
			    color);

	if (controlTime > 0.0) {
		controlTime -= timePerFrame;
		if (controlTime <= 0.0)
			controlTime = 0.0;
	}

	if (packetSizeMeterColor != NO_COLOR)
		Paint_meter(-10, y += 20, "Packet",
			    (packet_size >= 4096) ? 4096 : packet_size, 4096, packetSizeMeterColor);
	if (packetLossMeterColor != NO_COLOR)
		Paint_meter(-10, y += 20, "Loss", packet_loss, FPS, packetLossMeterColor);
	if (packetDropMeterColor != NO_COLOR)
		Paint_meter(-10, y += 20, "Drop", packet_drop, FPS, packetDropMeterColor);
	if (packetLagMeterColor != NO_COLOR)
		Paint_meter(-10, y +=
			    20, "Lag", MIN(packet_lag, 1 * FPS), 1 * FPS, packetLagMeterColor);
#endif
	if (temporaryMeterColor != NO_COLOR) {
		if (thrusttime >= 0 && thrusttimemax > 0)
			Paint_meter((draw_width - 300) / 2 - 32, 2 * draw_height / 3,
				    "Thrust Left",
				    (thrusttime >= thrusttimemax
				     ? thrusttimemax : thrusttime), thrusttimemax,
				    temporaryMeterColor);

		if (shieldtime >= 0 && shieldtimemax > 0)
			Paint_meter((draw_width - 300) / 2 - 32, 2 * draw_height / 3 + 20,
				    "Shields Left",
				    (shieldtime >= shieldtimemax
				     ? shieldtimemax : shieldtime), shieldtimemax,
				    temporaryMeterColor);

		if (phasingtime >= 0 && phasingtimemax > 0)
			Paint_meter((draw_width - 300) / 2 - 32, 2 * draw_height / 3 + 40,
				    "Phasing left",
				    (phasingtime >= phasingtimemax
				     ? phasingtimemax : phasingtime), phasingtimemax,
				    temporaryMeterColor);

		if (destruct > 0)
			Paint_meter((draw_width - 300) / 2 - 32, 2 * draw_height / 3 + 60,
				    "Self destructing", destruct, (int) SELF_DESTRUCT_DELAY,
				    temporaryMeterColor);

		if (shutdown_count >= 0)
			Paint_meter((draw_width - 300) / 2 - 32, 2 * draw_height / 3 + 80,
				    "SHUTDOWN", shutdown_count, shutdown_delay,
				    temporaryMeterColor);
	}
}


static void Paint_lock(int hud_pos_x, int hud_pos_y)
{
	const int BORDER = 2;
	other_t *target;
	char str[50];
	static int mapdiag = 0;

	if (mapdiag == 0) {
		double d = LENGTH(Setup->width, Setup->height);

		mapdiag = (int) d;
	}

	/*
	 * Display direction arrow and miscellaneous target information.
	 */
	if ((target = Other_by_id(lock_id)) == NULL)
		return;

	if (hudColor != NO_COLOR) {
		int color = Player_state_color(target);
		int name_width = Other_name_width(target, maxCharsInNames);

		if (color == WHITE || color == NO_COLOR)
			color = hudColor;
		Set_fg_color(color);

		ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
				hud_pos_x - name_width / 2,
				(hud_pos_y - hudSize + HUD_OFFSET - BORDER)
				- gameFont->descent,
				target->abbrev_nick_name,
				strlen(target->abbrev_nick_name),
				color, shadowColor, BLACK,
				false, true);

		if (!BIT(Setup->mode, LIMITED_LIVES)
		    && lock_dist != 0) {
			sprintf(str, "%03d", lock_dist / BLOCK_SZ);
			ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
					(hud_pos_x + hudSize - HUD_OFFSET + BORDER),
					(hud_pos_y - hudSize + HUD_OFFSET - BORDER)
					- gameFont->descent, str, 3,
					color, shadowColor, BLACK,
					false, true);
		}
	}


}

static void Paint_HUD_items(int hud_pos_x, int hud_pos_y)
{
	const int BORDER = 3;
	char str[50];
	int vert_pos, horiz_pos;
	int i, maxWidth = -1, rect_x, rect_y, rect_width = 0, rect_height = 0;
	static int vertSpacing = -1;

	if (hudColor == NO_COLOR)
		return;

	Set_fg_color(hudColor);

	/* Special itemtypes */
	if (vertSpacing < 0)
		vertSpacing = MAX(ITEM_SIZE, gameFont->ascent + gameFont->descent) + 1;
	/* find the scaled location, then work in pixels */
	vert_pos = (hud_pos_y - hudSize + HUD_OFFSET + BORDER);
	horiz_pos = (hud_pos_x - hudSize + HUD_OFFSET - BORDER);
	rect_width = 0;
	rect_height = 0;
	rect_x = horiz_pos;
	rect_y = vert_pos;

	for (i = 0; i < NUM_ITEMS; i++) {
		int num = numItems[i];

		if (i == ITEM_FUEL)
			continue;

		if (showItems) {
			lastNumItems[i] = num;
			if (num <= 0)
				num = -1;
		}
		else {
			if (num != lastNumItems[i]) {
				numItemsTime[i] = (int) (showItemsTime * (double) FPS);
				lastNumItems[i] = num;
			}
			if (numItemsTime[i]-- <= 0) {
				numItemsTime[i] = 0;
				num = -1;
			}
		}

		if (num >= 0) {
			int len, width;

			/* Paint item symbol */
			Gui_paint_item_symbol(i, drawPixmap, gameGC,
					      horiz_pos - ITEM_SIZE, vert_pos, ITEM_HUD);

			if (i == lose_item) {
				if (lose_item_active != 0) {
					if (lose_item_active < 0)
						lose_item_active++;
					rd.drawRectangle(dpy, drawPixmap, gameGC,
							 horiz_pos - ITEM_SIZE - 2,
							 vert_pos - 2, ITEM_SIZE + 2,
							 ITEM_SIZE + 2);
				}
			}

			/* Paint item count */
			sprintf(str, "%d", num);
			len = strlen(str);
			width = ColorTextWidth(gameFont, str, len);
			ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
					horiz_pos - ITEM_SIZE - BORDER - width,
					vert_pos + ITEM_SIZE / 2 + gameFont->ascent / 2,
					str, len,
					hudColor, shadowColor, BLACK,
					false, true);

			maxWidth = MAX(maxWidth, width + BORDER + ITEM_SIZE);
			vert_pos += vertSpacing;

			if (vert_pos + vertSpacing > (hud_pos_y + hudSize - HUD_OFFSET - BORDER)) {
				rect_width += maxWidth + 2 * BORDER;
				rect_height = MAX(rect_height, vert_pos - rect_y);
				horiz_pos -= maxWidth + 2 * BORDER;
				vert_pos = (hud_pos_y - hudSize + HUD_OFFSET + BORDER);
				maxWidth = -1;
			}
		}
	}
	if (maxWidth != -1)
		rect_width += maxWidth + BORDER;

	if (rect_width > 0) {
		if (rect_height == 0)
			rect_height = vert_pos - rect_y;
		rect_x -= rect_width;
	}

}

static bool have_hudmsg = false;
static char hudmsg[MSG_LEN];

void Del_HUD_message(void);
void Add_HUD_message(const char *message);

void Del_HUD_message(void)
{
	have_hudmsg = false;
}

void Add_HUD_message(const char *message)
{
	if (!message)
		return;

	strlcpy(hudmsg, message, sizeof(hudmsg));
	have_hudmsg = true;
}

void Paint_HUD(void)
{
	const int BORDER = 3;
	char str[50];
	int hud_pos_x, hud_pos_y, size;
	int did_fuel = 0;
	int i, j, modlen = 0;
	static char autopilot[] = "Autopilot";

#if 0
	if (ptr_move_fact != 0.0 && selfVisible && (selfVel.x != 0 || selfVel.y != 0))
		Segment_add(hudColor,
			    draw_width / 2,
			    draw_height / 2,
			    (int) (draw_width / 2 - ptr_move_fact * selfVel.x),
			    (int) (draw_height / 2 + ptr_move_fact * selfVel.y));
#endif

	/* message scan hack by mara and jpv */
	if (Bms_test_state(BmsBall))
		Arc_add(msgScanBallColor, ext_view_width / 2 - 6, ext_view_height / 2 - 6, 12, 12,
			0, 64 * 360);
	if (Bms_test_state(BmsCover))
		Arc_add(msgScanCoverColor, ext_view_width / 2 - 5, ext_view_height / 2 - 5, 10, 10,
			0, 64 * 360);

	/*
	 * Display the HUD
	 */
	hud_pos_x = (int) (draw_width / 2 /*- hud_move_fact * selfVel.x*/ );
	hud_pos_y = (int) (draw_height / 2 /*+ hud_move_fact * selfVel.y */ );

#if 0
	/* HUD frame */
	gcv.line_style = LineOnOffDash;
	XChangeGC(dpy, gameGC, GCLineStyle | GCDashOffset, &gcv);

#define hudHLineColor GREEN
#define hudVLineColor GREEN
	if (hudHLineColor) {
		Set_fg_color(hudHLineColor);
		rd.drawLine(dpy, drawPixmap, gameGC,
			    (hud_pos_x - hudSize),
			    (hud_pos_y - hudSize + HUD_OFFSET),
			    (hud_pos_x + hudSize), (hud_pos_y - hudSize + HUD_OFFSET));
		rd.drawLine(dpy, drawPixmap, gameGC,
			    (hud_pos_x - hudSize),
			    (hud_pos_y + hudSize - HUD_OFFSET),
			    (hud_pos_x + hudSize), (hud_pos_y + hudSize - HUD_OFFSET));
	}
	if (hudVLineColor) {
		Set_fg_color(hudVLineColor);
		rd.drawLine(dpy, drawPixmap, gameGC,
			    (hud_pos_x - hudSize + HUD_OFFSET),
			    (hud_pos_y - hudSize), (hud_pos_x - hudSize + HUD_OFFSET),
			    (hud_pos_y + hudSize));
		rd.drawLine(dpy, drawPixmap, gameGC, (hud_pos_x + hudSize - HUD_OFFSET),
			    (hud_pos_y - hudSize), (hud_pos_x + hudSize - HUD_OFFSET),
			    (hud_pos_y + hudSize));
	}
#endif
	gcv.line_style = LineSolid;
	XChangeGC(dpy, gameGC, GCLineStyle, &gcv);

	Paint_HUD_items(hud_pos_x, hud_pos_y);

	/* Fuel notify, HUD meter on */
	if (hudColor != NO_COLOR && (fuelTime > 0.0 || fuelSum < fuelNotify)) {
		Set_fg_color(hudColor);
		did_fuel = 1;
		sprintf(str, "%04d", fuelSum);
		ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
				(hud_pos_x + hudSize - HUD_OFFSET + BORDER),
				(hud_pos_y + hudSize - HUD_OFFSET + BORDER)
				+ gameFont->ascent, str, (int) strlen(str),
				hudColor, shadowColor, BLACK,
				false, true);
		if (numItems[ITEM_TANK]) {
			if (currentTank == 0)
				strcpy(str, "M ");
			else
				sprintf(str, "T%d", currentTank);
			ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
					(hud_pos_x + hudSize - HUD_OFFSET + BORDER),
					(hud_pos_y + hudSize - HUD_OFFSET + BORDER)
					+ gameFont->descent + 2 * gameFont->ascent, str,
					(int) strlen(str),
					hudColor, shadowColor, BLACK,
					false, true);
		}
	}

	/* Update the lock display */
	Paint_lock(hud_pos_x, hud_pos_y);

	/* kps tmp hack to draw "alert" messages */
	if (have_hudmsg) {
		int len = strlen(hudmsg);
		int width = ColorTextWidth(gameFont, hudmsg, len);

		Set_fg_color(BLUE);
		ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
				(hud_pos_x) - width / 2,
				(hud_pos_y - hudSize /*+ HUD_OFFSET */  - BORDER)
				- gameFont->descent, hudmsg, len,
				BLUE, shadowColor, BLACK,
				false, false); /* no colorize, no rec */
	}

	/* Draw last score on hud if it is an message attached to it */
	if (hudColor != NO_COLOR) {
		Set_fg_color(hudColor);

		for (i = 0, j = 0; i < MAX_SCORE_OBJECTS; i++) {
			score_object_t *sobj =
			    &score_objects[(i + score_object) % MAX_SCORE_OBJECTS];
			if (sobj->hud_msg_len > 0) {
				if (sobj->hud_msg_width == -1)
					sobj->hud_msg_width =
					    ColorTextWidth(gameFont, sobj->hud_msg, sobj->hud_msg_len);
				if (j == 0 && sobj->hud_msg_width > (2 * hudSize - HUD_OFFSET * 2)
				    && (did_fuel /*|| hudVLineColor */ ))
					++j;
				ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
						(hud_pos_x) - sobj->hud_msg_width / 2,
						(hud_pos_y + hudSize - HUD_OFFSET + BORDER)
						+ gameFont->ascent
						+ j * (gameFont->ascent + gameFont->descent),
						sobj->hud_msg, sobj->hud_msg_len,
						hudColor, shadowColor, BLACK,
						false, true);
				j++;
			}
		}

		if (time_left > 0) {
			sprintf(str, "%3d:%02d", (int) (time_left / 60), (int) (time_left % 60));
			size = ColorTextWidth(gameFont, str, (int) strlen(str));
			ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
					(hud_pos_x - hudSize + HUD_OFFSET - BORDER)
					- size, (hud_pos_y - hudSize + HUD_OFFSET - BORDER)
					- gameFont->descent, str, (int) strlen(str),
					hudColor, shadowColor, BLACK,
					false, true);
		}

		/* Update the modifiers */
		modlen = strlen(modifiers);
		ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
				(hud_pos_x - hudSize + HUD_OFFSET - BORDER)
				- ColorTextWidth(gameFont, modifiers, modlen),
				(hud_pos_y + hudSize - HUD_OFFSET + BORDER)
				+ gameFont->ascent, modifiers, (int) strlen(modifiers),
				hudColor, shadowColor, BLACK,
				false, true);

		if (autopilotLight) {
			int text_width = ColorTextWidth(gameFont, autopilot,
						    sizeof(autopilot) - 1);
			ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
					(hud_pos_x) - text_width / 2,
					(hud_pos_y - hudSize + HUD_OFFSET - BORDER)
					- gameFont->descent * 2 - gameFont->ascent,
					autopilot, sizeof(autopilot) - 1,
					hudColor, shadowColor, BLACK,
					false, true);
		}
	}

	if (fuelTime > 0.0) {
		fuelTime -= timePerFrame;
		if (fuelTime <= 0.0)
			fuelTime = 0.0;
	}

	/* draw fuel gauge */
	if (fuelGaugeColor != NO_COLOR && ((fuelTime > 0.0)
					   || (fuelSum < fuelNotify
					       && ((fuelSum < fuelCritical && (twelveHz % 4) < 2)
						   || (fuelSum < fuelWarning
						       && fuelSum > fuelCritical
						       && (twelveHz % 8) < 4)
						   || (fuelSum > fuelWarning))))) {

		Set_fg_color(fuelGaugeColor);
		rd.drawRectangle(dpy, drawPixmap, gameGC,
				 (hud_pos_x + hudSize - HUD_OFFSET
				  + FUEL_GAUGE_OFFSET) - 1,
				 (hud_pos_y - hudSize + HUD_OFFSET
				  + FUEL_GAUGE_OFFSET) - 1,
				 (HUD_OFFSET - (2 * FUEL_GAUGE_OFFSET)) + 3,
				 (HUD_FUEL_GAUGE_SIZE) + 3);

		size = (int) ((HUD_FUEL_GAUGE_SIZE * fuelSum) / fuelMax);
		rd.fillRectangle(dpy, drawPixmap, gameGC,
				 (hud_pos_x + hudSize - HUD_OFFSET
				  + FUEL_GAUGE_OFFSET) + 1,
				 (hud_pos_y - hudSize + HUD_OFFSET
				  + FUEL_GAUGE_OFFSET + HUD_FUEL_GAUGE_SIZE
				  - size) + 1, (HUD_OFFSET - (2 * FUEL_GAUGE_OFFSET)), (size));
	}
}

void Paint_recording(void)
{
	int w, x, y, len;
	char buf[32];
	double mb;

	if (!recording || (twelveHz % 16) < 8)
		return;

	Set_fg_color(RED);
	mb = Record_size() / 1e6;
	sprintf(buf, "REC %.1f MB", mb);
	len = strlen(buf);
	w = ColorTextWidth(gameFont, buf, len);
	x = draw_width - 10 - w;
	y = 10 + gameFont->ascent;
	ColorDrawString(dpy, drawPixmap, gameGC, gameFont,
			x, y, buf, len,
			RED, shadowColor, BLACK,
			false, false);
}

static void Paint_HUD_value(int y, const char *string)
{
	int x, w, len;

	len = strlen(string);
	w = ColorTextWidth(scoreListFont, string, len);

	x = (draw_width) - 10 - w;
	y = y + scoreListFont->ascent;
	ColorDrawString(dpy, drawPixmap, scoreListGC, scoreListFont,
			x, y, string, len,
			hudColor, shadowColor, BLACK,
			false, true);
}

void Paint_HUD_values(void)
{
	static char buf[MSG_LEN];
	static char kdratio[16];
	static char killsperround[16];
	int color = hudColor;
	other_t *eyes = Other_by_id(eyesId);
	static int hour = -1, minute, second;
	int SPACING = scoreListFont->ascent + scoreListFont->descent + 1;
	int y = 10;

	if (!showExtHUD)
		return;

	if (color == NO_COLOR)
		return;

	Set_gc_fg_color(dpy, scoreListGC, color);
	if (hour < 0 || newSecond) {
		time_t t;
		struct tm *m;

		time(&t);
		m = localtime(&t);
		second = m->tm_sec;
		minute = m->tm_min;
		hour = m->tm_hour;
	}

	sprintf(buf, "Time: %02d:%02d:%02d", hour, minute, second);
	Paint_HUD_value(y += SPACING, buf);
	sprintf(buf, "Power: %.1f", displayedPower);
	Paint_HUD_value(y += SPACING, buf);
	sprintf(buf, "Turnspeed: %.1f", displayedTurnspeed);
	Paint_HUD_value(y += SPACING, buf);
	sprintf(buf, "Packet: %d", packet_size);
	Paint_HUD_value(y += SPACING, buf);
	sprintf(buf, "Loss: %d/%d", packet_loss, FPS);
	Paint_HUD_value(y += SPACING, buf);
	sprintf(buf, "Drop: %d/%d", packet_drop, FPS);
	Paint_HUD_value(y += SPACING, buf);
	sprintf(buf, "Lag: %d ms", (int) (((1e3 * packet_lag) / FPS) + 0.5));
	Paint_HUD_value(y += SPACING, buf);
	sprintf(buf, "FPS: %.3f", clientFPS);
	Paint_HUD_value(y += SPACING, buf);
	sprintf(buf, "CL.Lag: %.1f ms", 1e-3 * clientLag);
	Paint_HUD_value(y += SPACING, buf);
	Check_abbrev_nick_name(eyes, maxCharsInNames);
	sprintf(buf, "View: %s", eyes ? eyes->abbrev_nick_name : "none");
	Paint_HUD_value(y += SPACING, buf);

	if (rounds_played > 0) {
		sprintf(buf, "Rounds: %d", rounds_played);
		Paint_HUD_value(y += SPACING, buf);
	}

	if (killratio_totalkills == 0)
		sprintf(kdratio, "0");
	else if (killratio_totaldeaths == 0)
		sprintf(kdratio, "inf");
	else
		sprintf(kdratio, "%.1f", (double) killratio_totalkills / killratio_totaldeaths);
	sprintf(buf, "K/D: %d/%d (%s)", killratio_totalkills, killratio_totaldeaths, kdratio);
	Paint_HUD_value(y += SPACING, buf);

	if (rounds_played > 0) {
		sprintf(killsperround, "%.1f", (double) killratio_totalkills / rounds_played);

		if (killratio_kills == 0)
			sprintf(kdratio, "0");
		else if (killratio_deaths == 0)
			sprintf(kdratio, "inf");
		else
			sprintf(kdratio, "%.1f", (double) killratio_kills / killratio_deaths);

		sprintf(buf, "R K/D: %d/%d (%s)", killratio_kills, killratio_deaths, kdratio);
		Paint_HUD_value(y += SPACING, buf);

		sprintf(buf, "Avg.K/R: %s", killsperround);
		Paint_HUD_value(y += SPACING, buf);
	}

	if (ballsCashed > 0 || ballsCashedByTeam > 0 || ballsLost > 0) {
		sprintf(buf, "B C/T/L: %d/%d/%d", ballsCashed,	/*ballsReplaced, */
			ballsCashedByTeam, ballsLost);
		Paint_HUD_value(y += SPACING, buf);
	}


#if 0
	{
		other_t *lock = Other_by_id(lock_id);
		sprintf(buf, "Lock: %s", lock ? lock->nick_name : "none");
		Paint_HUD_value(y += SPACING, buf);
	}

	snprintf(buf, sizeof(buf), "Modifiers: %s", (modifiers[0] == '\0' ? "none" : modifiers));
	Paint_HUD_value(y += SPACING, buf);

	if (fuelSum <= fuelWarning)
		color = GREEN;
	Set_fg_color(color);
	sprintf(buf, "Fuel: %d", fuelSum);
	Paint_HUD_value(y += SPACING, buf);
#endif
}
