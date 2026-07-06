/*
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 2007 Kristian Söderblom kps at users.sourceforge.net
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

/* $Id: color-codes.c,v 1.5 2007/11/11 20:25:50 kps Exp $ */

#include "sysdeps.h"
#include "const.h"

/*
 * http://wolfwiki.anime.net/index.php/Color_Codes
 * says these originate in Quake III Arena, therefore
 * we call them q3_colors.
 */
static int q3_colors[] = {
	0x3f3f3f, /* should be 0x000000, black   e.g. 0 P p */
	0xff0000,		/* red     e.g. 1 Q q */
	0x00ff00,		/* green   e.g. 2 R r */
	0xffff00,		/* yellow  e.g. 3 S s */
	0x0000ff,		/* blue    e.g. 4 T t */
	0x00ffff,		/* cyan    e.g. 5 U u */
	0xff00ff,		/* magenta e.g. 6 V v */
	0xffffff,		/* white   e.g. 7 W w */
	0xff7f00,		/* orange  e.g. 8 X x */
	0x7f7f7f,		/* gray    e.g. 9 Y y */
	0xbfbfbf,		/* e.g. : Z z */
	0xbfbfbf,		/* e.g. ; [ { */
	0x007f00,		/* e.g. < \ | */
	0x7f7f00,		/* e.g. = ] } */
	0x00007f,		/* e.g. > ^ ~ */
	0x7f0000,		/* e.g. ? _   */
	0x7f3f00,		/* e.g. @     */
	0xff9919,		/* e.g. ! A a */
	0x007f7f,		/* e.g. " B b */
	0x7f007f,		/* e.g. # C c */
	0x007fff,		/* e.g. $ D d */
	0x7f00ff,		/* e.g. % E e */
	0x3399cc,		/* e.g. & F f */
	0xccffcc,		/* e.g. ' G g */
	0x006633,		/* e.g. ( H h */
	0xff0033,		/* e.g. ) I i */
	0xb21919,		/* e.g. * J j */
	0x993300,		/* e.g. + K k */
	0xcc9933,		/* e.g. , L l */
	0x999933,		/* e.g. - M m */
	0xffffbf,		/* e.g. . N n */
	0xffff7f		/* e.g. / O o */
};

int Index_by_color_code(int ascii_char)
{
	return (ascii_char + 16) & 31;
}

int RGB_by_color_code(int ascii_char)
{
	return q3_colors[Index_by_color_code(ascii_char)];
}

/* Assumes index is between 0 and 31. */
int RGB_by_index(int ind)
{
	return q3_colors[ind];
}

/* Inverse of index_by_color_code(). */
char Color_code_by_index(int ind)
{
	assert(ind >= 0 && ind <= 31);	/* for now */
	ind = ind & 31;
	return (char) ('0' + ind);
}

/*
 * This function maps the color code
 * into one of the xpilot color indices
 * BLACK, WHITE, BLUE, RED, GREEN and YELLOW.
 */
int XPilot_color(int ind)
{
	switch (ind) {
	case 0:
		return BLACK;
	case 7:
	case 9:
	case 10:
	case 11:
	case 23:
	case 30:
		return WHITE;
	case 4:
	case 5:
	case 14:
	case 18:
	case 20:
	case 22:
		return BLUE;
	case 1:
	case 6:
	case 15:
	case 19:
	case 21:
	case 25:
	case 26:
	case 27:
		return RED;
	case 2:
	case 12:
	case 24:
		return GREEN;
	case 3:
	case 8:
	case 13:
	case 16:
	case 17:
	case 28:
	case 29:
	case 31:
		return YELLOW;
	default:
		return NO_COLOR;
	}
}
