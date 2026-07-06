/*
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 1991-2002 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * Copyright (C) 2003-2007 by
 *
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

/* $Id: paintmessages.c,v 1.24 2007/11/11 20:31:47 kps Exp $ */

#include "xpclient_x11.h"
#include "frame.h"
#include "client.h"
#include "setup.h"
#include "rules.h"
#include "error.h"
#include "bit.h"

static int messagesColor;	/* Color index for messages */
int shadowColor;		/* Color index for e.g. text shadows */
int msgScanBallColor;		/* Color index for ball msg */
static int msgScanSafeColor;	/* Color index for safe msg */
int msgScanCoverColor;		/* Color index for cover msg */
static int msgScanPopColor;	/* Color index for pop msg */

xp_option_t paintmessages_options[] = {
	COLOR_INDEX_OPTION("messagesColor",
			   GRAY,
			   &messagesColor,
			   "Which color number to use for drawing messages.\n"),

	COLOR_INDEX_OPTION("shadowColor",
			   BLACK,
			   &shadowColor,
			   "Which color number to use for drawing (text) shadows.\n"),

	COLOR_INDEX_OPTION("msgScanBallColor",
			   RED,
			   &msgScanBallColor,
			   "Which color number to use for drawing ball message warning.\n"),

	COLOR_INDEX_OPTION("msgScanSafeColor",
			   GREEN,
			   &msgScanSafeColor,
			   "Which color number to use for drawing safe message.\n"),

	COLOR_INDEX_OPTION("msgScanCoverColor",
			   BLUE,
			   &msgScanCoverColor,
			   "Which color number to use for drawing cover message.\n"),

	COLOR_INDEX_OPTION("msgScanPopColor",
			   YELLOW,
			   &msgScanPopColor,
			   "Which color number to use for drawing pop message.\n"),
};

void Store_paintmessages_options(void)
{
	STORE_OPTIONS(paintmessages_options);
}

int ColorTextWidth(XFontStruct *font_struct, const char *string, int count)
{
	static char buf[MSG_LEN];
	int n = 0, str_offset = 0;

	while (str_offset < count) {
		char c = string[str_offset++];
		if (c != '^') {
			buf[n++] = c;
		}
		else {
			char c2 = string[str_offset++];
			/* String ends, ignore final ^ */
			if (c2 == '\0')
				break;
			else if (c2 == '^') {
				buf[n++] = '^';
			}
		}
	}

	return XTextWidth(font_struct, buf, n);
}

void ColorDrawString(Display * display, Window w, GC gc, XFontStruct *font,
		     int x, int y, const char *str, int str_len,
		     int fg_color, int bg_color, int black_color, /* color to use for ^0 */
		     bool colorize, bool record)
{
	static char buf[MSG_LEN];
	int len = str_len;
	int str_offset = 0;
	int color;
	int buf_offset = 0;
	int start = 0;

	color = fg_color;
	while (color != NO_COLOR && str_offset < str_len) {
		char c = str[str_offset++];
		if (c != '^') {
			buf[buf_offset++] = c;
		}
		else {
			char c2 = str[str_offset++];
			/* String ends, ignore final ^ */
			if (c2 == '\0')
				break;
			else if (c2 == '^') {
				buf[buf_offset++] = '^';
			}
		}
	}
	len = buf_offset;

	/* Possibly draw shadow in one color. */
	if (bg_color != NO_COLOR) {
		Set_gc_fg_color(display, gc, bg_color);
		if (record)
			rd.drawString(display, w, gc, x + 1, y + 1, buf, len);
		else
			XDrawString(display, w, gc, x + 1, y + 1, buf, len);
			
	}
	x--;
	y--;

	color = fg_color;
	str_offset = 0;
	buf_offset = 0;

	while (color != NO_COLOR && str_offset < str_len) {
		int xoff;
		int new_color = NO_COLOR;

		while (str_offset < str_len) {
			char c = str[str_offset++];
			if (c != '^') {
				buf[buf_offset++] = c;
			}
			else {
				char c2 = str[str_offset++];
				/* String ends, ignore final ^ */
				if (c2 == '\0')
					break;
				else if (c2 == '^') {
					buf[buf_offset++] = '^';
				}
				else {
					int ind = Index_by_color_code(c2);
					if (ind != 0
					    || black_color == NO_COLOR
					    || black_color == BLACK)
						new_color = ind + MAX_COLORS;
					else
						/*
						 * hack - ^0 results in painting with
						 * the color specified as argument black_color.
						 */
						new_color = black_color;
					break;
				}
			}
		}

		Set_gc_fg_color(display, gc, color);
		len = buf_offset - start;
		xoff = XTextWidth(font, buf, start);

		if (record)
			rd.drawString(display, w, gc, x + xoff, y, buf + start, len);
		else
			XDrawString(display, w, gc, x + xoff, y, buf + start, len);

		/* Prepare for next part of string. */
		if (colorize && !ignoreColorCodes)
			color = new_color;
		start = buf_offset;

	}
}

static void Paint_message(message_t *msg, int i, int msg_color, int x, int y)
{
	const char *msg_txt = Message_get_text(msg);
	size_t len;

	len = (int) (charsPerSecond * Message_get_age(msg));
	len = MIN(msg->len, len);

	/*
	 * it's an emphasized talk message
	 */
	if (selection.draw.state == SEL_EMPHASIZED
	    && i < maxMessages && i >= selection.draw.y1 && i <= selection.draw.y2) {

		/*
		 * three strings (ptr), where they begin (xoff) and their
		 * length (l):
		 *   1st is an umemph. string to the left of a selection,
		 *   2nd an emphasized part itself,
		 *   3rd an unemph. part to the right of a selection.
		 * set the according variables if a part exists.
		 * e.g: a selection of several lines 'stopping' somewhere in
		 *   the middle of a line -> ptr2,ptr3 are needed to draw
		 *   this line
		 */
		const char *ptr = NULL;
		int xoff = 0, l = 0;
		const char *ptr2 = NULL;
		int xoff2 = 0, l2 = 0;
		const char *ptr3 = NULL;
		int xoff3 = 0, l3 = 0;

		if (i > selection.draw.y1 && i < selection.draw.y2) {
			/* all emphasized on this line */
			/*xxxxxxxxx */
			ptr2 = msg_txt;
			l2 = len;
			xoff2 = 0;
		}
		else if (i == selection.draw.y1) {
			/* first/only line */
			/*___xxx[___]*/
			ptr = msg_txt;
			xoff = 0;
			if ((int) len < selection.draw.x1)
				l = len;
			else {
				/* at least two parts */
				/*___xxx[___]*/
				/*    ^      */
				l = selection.draw.x1;
				ptr2 = Message_get_text_pointer(msg, selection.draw.x1);
				xoff2 =	XTextWidth(messageFont, msg_txt,
						   selection.draw.x1);

				if (i < selection.draw.y2) {
					/* first line */
					/*___xxxxxx*/
					/*     ^   */
					l2 = len - selection.draw.x1;
				}
				else {
					/* only line */
					/*___xxx___*/
					if ((int) len <= selection.draw.x2)
						/*___xxx___*/
						/*    ^    */
						l2 = len - selection.draw.x1;
					else {
						/*___xxx___*/
						/*       ^ */
						l2 = selection.draw.x2 - selection.draw.x1 + 1;
						ptr3 = Message_get_text_pointer(msg,
										selection.draw.x2 + 1);
						xoff3 = XTextWidth(messageFont, msg_txt,
								   selection.draw.x2 + 1);
						l3 = len - selection.draw.x2 - 1;
					}
				}	/* only line */
			}	/* at least two parts */
		}
		else {
			/* last line */
			/*xxxxxx[___] */
			ptr2 = msg_txt;
			xoff2 = 0;
			if ((int) len <= selection.draw.x2 + 1)
				/* all blue */
				/*xxxxxx[___] */
				/*  ^        */
				l2 = len;
			else {
				/*xxxxxx___ */
				/*       ^ */
				l2 = selection.draw.x2 + 1;
				ptr3 = Message_get_text_pointer(msg,
								selection.draw.x2 + 1);
				xoff3 =
					XTextWidth(messageFont, msg_txt,
						   selection.draw.x2 + 1);
				l3 = len - selection.draw.x2 - 1;
			}
		}	/* last line */

		if (ptr) {
			Set_gc_fg_color(dpy, messageGC, msg_color);
			rd.drawString(dpy, drawPixmap, messageGC, x + xoff, y, ptr, l);
		}
		if (ptr2) {
			Set_gc_fg_color(dpy, messageGC, DRAW_EMPHASIZED);
			rd.drawString(dpy, drawPixmap, messageGC, x + xoff2, y, ptr2, l2);
		}
		if (ptr3) {
			Set_gc_fg_color(dpy, messageGC, msg_color);
			rd.drawString(dpy, drawPixmap, messageGC, x + xoff3, y, ptr3, l3);
		}
		
	}
	else {		/* not emphasized */
		ColorDrawString(dpy, drawPixmap, messageGC, messageFont,
				x, y, msg_txt, (int) len,
				msg_color, shadowColor, messagesColor,
				true, true);
	}
}

void Paint_messages(void)
{
	int i, x, y, top_y, bot_y, msg_color;
	int X_BORDER = 10 /*+ (256 - drawWindowXOffset) */ , TOP_Y_BORDER = 10, BOT_Y_BORDER = 10;
	int SPACING = messageFont->ascent + messageFont->descent + 1;
	message_t *msg;
	int life_time;

	if (button_form_mapped)
		TOP_Y_BORDER = 40;

	top_y = TOP_Y_BORDER + messageFont->ascent;
	bot_y = draw_height - messageFont->descent - BOT_Y_BORDER;

	for (i = 0; i < 2 * maxMessages; i++) {
		if (i < maxMessages) {
			msg = Get_talk_message(i);
			life_time = talkMessagesLifeTime;
		}
		else {
			msg = Get_game_message(i - maxMessages);
			life_time = gameMessagesLifeTime;
		}
		if (Message_get_length(msg) == 0)
			continue;

		Message_add_age(msg, timePerFrame);
		if (Message_get_age(msg) > life_time) {
			Message_clear(msg);
			continue;
		}

		/* If paused, don't bother to paint messages in mscScan* colors. */
		if (self && strchr("P", self->mychar))
			msg_color = messagesColor;
		else {
			switch (Message_get_bmsinfo(msg)) {
			case BmsBall:
				msg_color = msgScanBallColor;
				break;
			case BmsSafe:
				msg_color = msgScanSafeColor;
				break;
			case BmsCover:
				msg_color = msgScanCoverColor;
				break;
			case BmsPop:
				msg_color = msgScanPopColor;
				break;
			default:
				msg_color = messagesColor;
				break;
			}
		}

		if (msg_color == NO_COLOR)
			continue;

		if (i < maxMessages) {
			if (!showTalkMessages)
				continue;
			x = X_BORDER;
			y = top_y;
			top_y += SPACING;
		}
		else {
			if (!showGameMessages)
				continue;
			x = X_BORDER;
			y = bot_y;
			bot_y -= SPACING;
		}
		Paint_message(msg, i, msg_color, x, y);
	}
}
