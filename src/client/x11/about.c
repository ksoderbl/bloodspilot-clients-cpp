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

/* $Id: about.c,v 1.13 2007/09/27 23:31:05 kps Exp $ */

#include "xpclient_x11.h"
#include "error.h"
#include "option.h"
#include "proto.h"

int keys_viewer = NO_WIDGET;

int Keys_callback(int widget_desc, void *data, const char **unused)
{
	unsigned bufsize = (num_keydefs * 64);
	char *buf = XCALLOC(char, bufsize), *end = buf, *str;
	const char *help;
	int i, len, maxkeylen = 0;

	UNUSED_PARAM(widget_desc);
	UNUSED_PARAM(data);
	UNUSED_PARAM(unused);

	for (i = 0; i < num_keydefs; i++) {
		if ((str = XKeysymToString((KeySym) keydefs[i].keysym)) != NULL
		    && (len = strlen(str)) > maxkeylen) {
			maxkeylen = len;
		}
	}
	for (i = 0; i < num_keydefs; i++) {
		if (!(str = XKeysymToString((KeySym) keydefs[i].keysym))
		    || !(help = Get_keyHelpString(keydefs[i].key)))
			continue;

		if ((end - buf) + (maxkeylen + strlen(help) + 4) >= bufsize) {
			bufsize += 4096;
			xpprintf("realloc: %d\n", bufsize);
			if (!(buf = XREALLOC(char, buf, bufsize))) {
				error("No memory for key list");
				return 0;
			}
		}
		sprintf(end, "%-*s  %s\n", maxkeylen, str, help);
		end += strlen(end);
	}
	keys_viewer =
	    Widget_create_viewer(buf,
				 end - buf,
				 2 * DisplayWidth(dpy, DefaultScreen(dpy)) / 3,
				 4 * DisplayHeight(dpy, DefaultScreen(dpy)) / 5,
				 2, "XPilot - key reference", "XPilot:keys", motdFont);
	if (keys_viewer == NO_WIDGET) {
		warn("Can't create key viewer");
		return 0;
	}
#if 0
	else if (keys_viewer != NO_WIDGET)
		Widget_map(keys_viewer);
#endif
	return 0;
}

void Keys_destroy(void)
{
	Widget_destroy(keys_viewer);
	keys_viewer = NO_WIDGET;
	/*keys_created = false; */
}


#define MAX_MOTD_SIZE	(30*1024)

static char *motd_buf = NULL;
static size_t motd_size;
int motd_viewer = NO_WIDGET;
static bool motd_auto_popup;

int Motd_callback(int widget_desc, void *data, const char **str)
{
	UNUSED_PARAM(widget_desc);
	UNUSED_PARAM(data);
	UNUSED_PARAM(str);

	/* always refresh motd */
	motd_auto_popup = false;
	Net_ask_for_motd(0, MAX_MOTD_SIZE);
	Net_flush();

	if (motd_viewer != NO_WIDGET)
		Widget_map(motd_viewer);
	return 0;
}

void Motd_destroy(void)
{
	Widget_destroy(motd_viewer);
	motd_viewer = NO_WIDGET;
	XFREE(motd_buf);
}

int Handle_motd(long off, char *buf, int len, long filesize)
{
	int i;
	static char no_motd_msg[] = "\nThis server has no MOTD.\n\n";

	if (!motd_buf) {
		motd_size = MIN(filesize, MAX_MOTD_SIZE);
		i = MAX(motd_size, (long) (sizeof no_motd_msg)) + 1;
		if (!(motd_buf = XMALLOC(char, (size_t) i))) {
			error("No memory for MOTD");
			return -1;
		}
		memset(motd_buf, ' ', motd_size);
		for (i = 39; i < (int) motd_size; i += 40)
			motd_buf[i] = '\n';
	}
	else if (filesize < (long) motd_size) {
		motd_size = filesize;
		motd_buf[motd_size] = '\0';
	}
	if (off < (long) motd_size && len > 0) {
		if (off + len > (long) motd_size)
			len = motd_size - off;
		memcpy(motd_buf + off, buf, (size_t) len);
	}
	else if (len == 0 && off > 0)
		return 0;

	if (motd_size == 0) {
		if (motd_auto_popup) {
			XFREE(motd_buf);
			return 0;
		}
		strcpy(motd_buf, no_motd_msg);
		motd_size = strlen(motd_buf);
	}
	if (motd_viewer == NO_WIDGET) {
		char title[100];

		snprintf(title, sizeof(title), "XPilot motd from %s", servername);
		motd_viewer = Widget_create_viewer(motd_buf,
						   (off
						    || len) ? (off + len) : (int) strlen(motd_buf),
						   2 * DisplayWidth(dpy, DefaultScreen(dpy)) / 3,
						   4 * DisplayHeight(dpy, DefaultScreen(dpy)) / 8,
						   2, title, "XPilot:motd", motdFont);
		if (motd_viewer == NO_WIDGET)
			warn("Can't create MOTD viewer");
	}
	else if (len > 0)
		Widget_update_viewer(motd_viewer, motd_buf, off + len);

	return 0;
}

void aboutCleanup(void)
{
	XFREE(motd_buf);
}
