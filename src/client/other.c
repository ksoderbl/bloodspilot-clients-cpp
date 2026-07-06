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

/* $Id: other.c,v 1.3 2007/09/27 22:19:46 kps Exp $ */

#include "sysdeps.h"
#include "other.h"
#include "proto.h"

other_t *self;			/* Player info */
other_t *Others = 0;
int num_others = 0, max_others = 0;

other_t *Other_by_id(int id)
{
	int i;

	if (id != -1) {
		for (i = 0; i < num_others; i++) {
			if (Others[i].id == id) {
				return &Others[i];
			}
		}
	}
	return NULL;
}

other_t *Other_by_name(const char *name, bool show_error_msg)
{
	int i;
	other_t *found_other = NULL, *other;
	size_t len;

	if (name == NULL || (len = strlen(name)) == 0)
		goto match_none;

	/* Look for an exact match on player nickname. */
	for (i = 0; i < num_others; i++) {
		other = &Others[i];
		if (!strcasecmp(other->nick_name, name))
			return other;
	}

	/* Look if 'name' matches beginning of only one nick. */
	for (i = 0; i < num_others; i++) {
		other = &Others[i];

		if (!strncasecmp(other->nick_name, name, len)) {
			if (found_other)
				goto match_several;
			found_other = other;
			continue;
		}
	}
	if (found_other)
		return found_other;

	/*
	 * Check what players' name 'name' is a substring of (case insensitively).
	 */
	for (i = 0; i < num_others; i++) {
		int j;
		other = &Others[i];

		for (j = 0; j < 1 + (int) strlen(other->nick_name) - (int) len; j++) {
			if (!strncasecmp(other->nick_name + j, name, len)) {
				if (found_other)
					goto match_several;
				found_other = other;
				break;
			}
		}
	}
	if (found_other)
		return found_other;

      match_none:
	{
		if (show_error_msg)
			Add_message("Name does not match any player. [*Client reply*]");
		return NULL;
	}
      match_several:
	{
		if (show_error_msg)
			Add_message("Name matches several players. [*Client reply*]");
		return NULL;
	}
}

shipshape_t *Ship_by_id(int id)
{
	other_t *other;

	if ((other = Other_by_id(id)) == NULL) {
		return Parse_shape_str(NULL);
	}
	return other->ship;
}
