/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 2003-2007 by
 *
 *      Kristian Söderblom   kps at users.sourceforge.net
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

/* $Id: msg-bms.c,v 1.4 2007/10/18 11:31:13 kps Exp $ */

#include "sysdeps.h"
#include "bit.h"
#include "const.h"
#include "error.h"
#include "global.h"
#include "other.h"
#include "proto.h"
#include "rules.h"
#include "setup.h"
#include "types.h"
#include "frame.h"

static bool ball_shout;		/* Someone issued a ball warning? */
static bool need_cover;		/* Someone wanted cover? */

/* Mara's ball message scan */
msg_bms_t Msg_do_bms(const char *message)
{
	static char ball_text1[] = "BALL";
	static char ball_text2[] = "Ball";
	static char ball_text3[] = "VAKK";
	static char ball_text4[] = "B A L L";
	static char ball_text5[] = "ball";
	static char safe_text1[] = "SAFE";
	static char safe_text2[] = "Safe";
	static char safe_text3[] = "safe";
	static char safe_text4[] = "S A F E";
	static char cover_text1[] = "COVER";
	static char cover_text2[] = "Cover";
	static char cover_text3[] = "cover";
	static char cover_text4[] = "INCOMING";
	static char cover_text5[] = "Incoming";
	static char cover_text6[] = "incoming";
	static char pop_text1[] = "POP";
	static char pop_text2[] = "Pop";
	static char pop_text3[] = "pop";

	/*check safe b4 ball */
	if (strstr(message, safe_text1) ||
	    strstr(message, safe_text2) || strstr(message, safe_text3)
	    || strstr(message, safe_text4)) {
		return BmsSafe;
	}

	if (strstr(message, cover_text1) ||
	    strstr(message, cover_text2) ||
	    strstr(message, cover_text3) ||
	    strstr(message, cover_text4) || strstr(message, cover_text5)
	    || strstr(message, cover_text6)) {
		return BmsCover;
	}

	if (strstr(message, pop_text1) || strstr(message, pop_text2) || strstr(message, pop_text3)) {
		return BmsPop;
	}

	if (strstr(message, ball_text1) ||
	    strstr(message, ball_text2) ||
	    strstr(message, ball_text3) || strstr(message, ball_text4)
	    || strstr(message, ball_text5)) {
		return BmsBall;
	}

	return BmsNone;
}

/*
 * Clear bms info for all messages of the specified type.
 */
static void Bms_clear(msg_bms_t type)
{
	int i;

	for (i = 0; i < maxMessages; i++) {
		message_t *msg = Get_talk_message(i);

		if (msg == NULL)
			continue;
		if (Message_get_bmsinfo(msg) == type)
			Message_set_bmsinfo(msg, BmsNone);
	}
}

bool Bms_test_state(msg_bms_t bms)
{
	switch (bms) {
	case BmsBall:
		return ball_shout;
	case BmsCover:
		return need_cover;
	case BmsSafe:
		return !ball_shout;
	case BmsPop:
		return !need_cover;
	default:
		/*dumpcore("Bms_test_state(): invalid message type"); */
		return false;
	}
}

void Bms_set_state(msg_bms_t bms)
{
	switch (bms) {
	case BmsBall:
		ball_shout = true;
		Bms_clear(BmsSafe);
		break;
	case BmsSafe:
		ball_shout = false;
		Bms_clear(BmsBall);
		break;
	case BmsCover:
		need_cover = true;
		Bms_clear(BmsPop);
		break;
	case BmsPop:
		need_cover = false;
		Bms_clear(BmsCover);
		break;
	case BmsNone:
		ball_shout = false;
		need_cover = false;
		Bms_clear(BmsBall);
		Bms_clear(BmsSafe);
		Bms_clear(BmsCover);
		Bms_clear(BmsPop);
		break;
	default:
		/*dumpcore("Bms_set_state(): invalid message type"); */
		return;
	}
}
