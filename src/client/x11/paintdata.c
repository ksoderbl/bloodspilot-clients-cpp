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

/* $Id: paintdata.c,v 1.23 2007/11/23 12:53:21 kps Exp $ */

#include "xpclient_x11.h"
#include "client.h"
#include "error.h"

typedef struct paintdata {
	/* recorded stuff */
	XRectangle *rect_ptr;
	int num_rect, max_rect;
	XArc *arc_ptr;
	int num_arc, max_arc;
	XSegment *seg_ptr;
	int num_seg, max_seg;
	/* not recorded */
	XRectangle *nr_rect_ptr;
	int num_nr_rect, max_nr_rect;
	XArc *nr_arc_ptr;
	int num_nr_arc, max_nr_arc;
	XSegment *nr_seg_ptr;
	int num_nr_seg, max_nr_seg;
} paintdata_t;

#define PAINTDATA_MAX_COLOR 300
static paintdata_t data[PAINTDATA_MAX_COLOR];		/* enough colors for now */

static int max_color = 0;

static void Rectangle_start(void)
{
	int i;

	for (i = 0; i <= max_color; i++) {
		data[i].num_rect = 0;
		data[i].num_nr_rect = 0;
	}
}

static void Rectangle_end(void)
{
	int i;

	for (i = 0; i <= max_color; i++) {
		if (data[i].num_rect > 0) {
			Set_fg_color(i);
			rd.fillRectangles(dpy, drawPixmap, gameGC, data[i].rect_ptr,
					  data[i].num_rect);
			RELEASE(data[i].rect_ptr, data[i].num_rect, data[i].max_rect);
		}
		if (data[i].num_nr_rect > 0) {
			Set_fg_color(i);
			XFillRectangles(dpy, drawPixmap, gameGC, data[i].nr_rect_ptr,
					data[i].num_nr_rect);
			RELEASE(data[i].nr_rect_ptr, data[i].num_nr_rect, data[i].max_nr_rect);
		}
	}
}

static inline int Check_paintdata_maxcolor(int color)
{
	if (color >= PAINTDATA_MAX_COLOR) {
		warn("paintdata.c: Color index exceeds PAINTDATA_MAX_COLOR (%d>%d)",
		     color, PAINTDATA_MAX_COLOR);
		return -1;
	}
	return 0;
}

int Rectangle_add(int color, int x, int y, int width, int height)
{
	XRectangle t;

	if (color < 0 || Check_paintdata_maxcolor(color) < 0)
		return -1;
	if (color > max_color)
		max_color = color;

	t.x = WINSCALE(x);
	t.y = WINSCALE(y);
	t.width = WINSCALE(width);
	t.height = WINSCALE(height);
	STORE(XRectangle, data[color].rect_ptr, data[color].num_rect, data[color].max_rect, t);

	return 0;
}

int Rectangle_add_unscaled(int color, int x, int y, int width, int height, bool record)
{
	XRectangle t;

	if (color < 0 || Check_paintdata_maxcolor(color) < 0)
		return -1;
	if (color > max_color)
		max_color = color;

	t.x = x;
	t.y = y;
	t.width = width;
	t.height = height;
	if (record)
		STORE(XRectangle, data[color].rect_ptr, data[color].num_rect, data[color].max_rect,
		      t);
	else
		STORE(XRectangle, data[color].nr_rect_ptr, data[color].num_nr_rect,
		      data[color].max_nr_rect, t);

	return 0;
}

static void Arc_start(void)
{
	int i;

	for (i = 0; i <= max_color; i++) {
		data[i].num_arc = 0;
		data[i].num_nr_arc = 0;
	}
}

static void Arc_end(void)
{
	int i;

	for (i = 0; i <= max_color; i++) {
		if (data[i].num_arc > 0) {
			Set_fg_color(i);
			rd.drawArcs(dpy, drawPixmap, gameGC, data[i].arc_ptr, data[i].num_arc);
			RELEASE(data[i].arc_ptr, data[i].num_arc, data[i].max_arc);
		}
	}
}

int Arc_add(int color, int x, int y, int width, int height, int angle1, int angle2)
{
	XArc t;

	if (color < 0 || Check_paintdata_maxcolor(color) < 0)
		return -1;
	if (color > max_color)
		max_color = color;

	t.x = WINSCALE(x);
	t.y = WINSCALE(y);
	t.width = WINSCALE(width + x) - t.x;
	t.height = WINSCALE(height + y) - t.y;
	t.angle1 = angle1;
	t.angle2 = angle2;
	STORE(XArc, data[color].arc_ptr, data[color].num_arc, data[color].max_arc, t);

	return 0;
}

int Arc_add_unscaled(int color, int x, int y, int width, int height, int angle1, int angle2)
{
	XArc t;

	if (color < 0 || Check_paintdata_maxcolor(color) < 0)
		return -1;
	if (color > max_color)
		max_color = color;

	t.x = x;
	t.y = y;
	t.width = width + x - t.x;
	t.height = height + y - t.y;
	t.angle1 = angle1;
	t.angle2 = angle2;
	STORE(XArc, data[color].arc_ptr, data[color].num_arc, data[color].max_arc, t);

	return 0;
}

static void Segment_start(void)
{
	int i;

	for (i = 0; i <= max_color; i++) {
		data[i].num_seg = 0;
		data[i].num_nr_seg = 0;
	}
}

static void Segment_end(void)
{
	int i;

	for (i = 0; i <= max_color; i++) {
		if (data[i].num_seg > 0) {
			Set_fg_color(i);
			rd.drawSegments(dpy, drawPixmap, gameGC, data[i].seg_ptr, data[i].num_seg);
			RELEASE(data[i].seg_ptr, data[i].num_seg, data[i].max_seg);
		}
		if (data[i].num_nr_seg > 0) {
			Set_fg_color(i);
			XDrawSegments(dpy, drawPixmap, gameGC, data[i].nr_seg_ptr,
				      data[i].num_nr_seg);
			RELEASE(data[i].nr_seg_ptr, data[i].num_nr_seg, data[i].max_nr_seg);
		}
	}
}

int Segment_add(int color, int xa, int ya, int xb, int yb)
{
	XSegment t;

	if (color < 0 || Check_paintdata_maxcolor(color) < 0)
		return -1;
	if (color > max_color)
		max_color = color;

	t.x1 = WINSCALE(xa);
	t.y1 = WINSCALE(ya);
	t.x2 = WINSCALE(xb);
	t.y2 = WINSCALE(yb);
	STORE(XSegment, data[color].seg_ptr, data[color].num_seg, data[color].max_seg, t);

	return 0;
}

int Segment_add_unscaled(int color, int xa, int ya, int xb, int yb, bool record)
{
	XSegment t;

	if (color < 0 || Check_paintdata_maxcolor(color) < 0)
		return -1;
	if (color > max_color)
		max_color = color;

	t.x1 = xa;
	t.y1 = ya;
	t.x2 = xb;
	t.y2 = yb;
	if (record)
		STORE(XSegment, data[color].seg_ptr, data[color].num_seg, data[color].max_seg, t);
	else
		STORE(XSegment, data[color].nr_seg_ptr, data[color].num_nr_seg,
		      data[color].max_nr_seg, t);

	return 0;
}

void Paintdata_start(void)
{
	Rectangle_start();
	Arc_start();
	Segment_start();
}

void Paintdata_end(void)
{
	Segment_end();
	Arc_end();
	Rectangle_end();
}

void Paintdata_cleanup(void)
{
	int i;

	for (i = 0; i <= max_color; i++) {
		if (data[i].max_rect > 0 && data[i].rect_ptr) {
			data[i].max_rect = 0;
			free(data[i].rect_ptr);
		}
		if (data[i].max_arc > 0 && data[i].arc_ptr) {
			data[i].max_arc = 0;
			free(data[i].arc_ptr);
		}
		if (data[i].max_seg > 0 && data[i].seg_ptr) {
			data[i].max_seg = 0;
			free(data[i].seg_ptr);
		}
		if (data[i].max_nr_rect > 0 && data[i].nr_rect_ptr) {
			data[i].max_nr_rect = 0;
			free(data[i].nr_rect_ptr);
		}
		if (data[i].max_nr_arc > 0 && data[i].nr_arc_ptr) {
			data[i].max_nr_arc = 0;
			free(data[i].nr_arc_ptr);
		}
		if (data[i].max_nr_seg > 0 && data[i].nr_seg_ptr) {
			data[i].max_nr_seg = 0;
			free(data[i].nr_seg_ptr);
		}
	}
}
