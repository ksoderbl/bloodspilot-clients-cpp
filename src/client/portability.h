/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2005 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *      Kristian S�derblom   kps at users.sourceforge.net
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

/*
 * Include portability related stuff in one file.
 */
#ifndef PORTABILITY_H
#define PORTABILITY_H

#include "sysdeps.h"

#define PATHNAME_SEP '/'

int micro_delay(unsigned usec);

void Get_login_name(char *buf, size_t size);

#ifndef HAVE_STRDUP
char *strdup(const char *);
#endif
char *xp_safe_strdup(const char *old_string);

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *str1, const char *str2);
#endif
#ifndef HAVE_STRNCASECMP
int strncasecmp(const char *str1, const char *str2, size_t n);
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dest, const char *src, size_t size);
#endif
#ifndef HAVE_STRLCAT
size_t strlcat(char *dest, const char *src, size_t size);
#endif

void *xp_safe_malloc(size_t size);
void *xp_safe_realloc(void *oldptr, size_t size);
void *xp_safe_calloc(size_t nmemb, size_t size);
void xp_safe_free(void *p);

#define ServerExit() exit(1);

#define IFWINDOWS(x)
#define IFNWINDOWS(x) x
#define PATHNAME_SEP '/'

/*
 * Prototypes for OS function wrappers in portability.c.
 */
extern int Get_process_id(void); /* getpid */
extern void Get_login_name(char *buf, size_t size);
extern int xpprintf(const char *fmt, ...);

static inline struct timeval seconds_to_timeval(double t)
{
	struct timeval tv;

	tv.tv_sec = (unsigned)t;
	tv.tv_usec = (unsigned)(((t - (double)tv.tv_sec) * 1e6) + 0.5);

	return tv;
}

/* returns 'tv2 - tv1' */
static inline int timeval_sub(struct timeval *tv2, struct timeval *tv1)
{
	int s, us;

	s = tv2->tv_sec - tv1->tv_sec;
	us = tv2->tv_usec - tv1->tv_usec;

	return 1000000 * s + us;
}

#endif /* PORTABILITY_H */
