/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2005 by
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

/* $Id: other.h,v 1.4 2007/10/11 11:32:48 kps Exp $ */

#ifndef OTHER_H
#define OTHER_H

#ifndef TYPES_H
# include "types.h"
#endif
#ifndef SHIPSHAPE_H
# include "shipshape.h"
#endif

typedef struct {
	DFLOAT ratio;
	short id;
	short team;
	int score;
	short check;
	short round;
	short timing;
	long timing_loops;
	short life;
	short mychar;
	short alliance;
	short war_id;
	short max_chars_in_names;
	short ignorelevel;
	shipshape_t *ship;
	char nick_name[MAX_CHARS];
	char user_name[MAX_CHARS];
	char host_name[MAX_CHARS];
	char abbrev_nick_name[MAX_CHARS];	/* Used to support maxCharsInNames option. */
	short name_width;	/* In pixels */
	short name_len;		/* In chars */
} other_t;

extern other_t *self;		/* Player info */
extern other_t *Others;
extern int num_others, max_others;

other_t *Other_by_id(int id);
other_t *Other_by_name(const char *name, bool show_error_msg);
shipshape_t *Ship_by_id(int id);

static inline void Check_abbrev_nick_name(other_t *other, int max_chars_in_names)
{
	int str_offset = 0, str_len, buf_offset = 0;
	char *str, *buf;

	if (other == NULL)
		return;
	if (other->max_chars_in_names == max_chars_in_names)
		return;

	str = other->nick_name;
	str_len = strlen(other->nick_name);
	buf = other->abbrev_nick_name;

	while (str_offset < str_len
	       && buf_offset < max_chars_in_names) {
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
	buf[buf_offset] = '\0';
#if 0
	other->name_width = 2 + ColorTextWidth(gameFont, buf, buf_offset);
	other->name_len = buf_offset;
#endif
	other->max_chars_in_names = max_chars_in_names;
}

#endif
