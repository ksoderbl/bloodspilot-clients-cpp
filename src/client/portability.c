/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2005 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

/* $Id: portability.c,v 1.7 2007/09/27 22:19:46 kps Exp $ */

/*
 * This file contains function wrappers around OS specific services.
 */

#include "portability.h"
#include "error.h"

/*
 * OS dependant implementation of usleep().
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * Revision History:
 * 30-Aug-90: written.
 *
 * 07-Dec-94: Bert Gijsbers
 * Changed "void usleep(unsigned long)" to "int usleep(unsigned)"
 * as this is what it seems to be on systems which do have usleep(3) (AIX).
 * Changed usleep into micro_delay to forego any possible prototype clashes.
 */

int micro_delay(unsigned usec)
{
#if 0				/* SYSV */
	poll((struct poll *) 0, (size_t) 0, usec / 1000);	/* ms RES */
#endif
	struct timeval timeout;
	timeout.tv_usec = usec % (unsigned long) 1000000;
	timeout.tv_sec = usec / (unsigned long) 1000000;
	(void) select(0, NULL, NULL, NULL, &timeout);

	return 0;
}

/*
 * By Ian Malcom Brown.
 * Changes by BG: prototypes with const,
 * moved the ++ expressions out of the macro.
 * Only test for the null byte in one string.
 */
#ifndef HAVE_STRCASECMP
int strcasecmp(const char *str1, const char *str2)
{
	int c1, c2;

	do {
		c1 = *str1++;
		c2 = *str2++;
		c1 = tolower(c1);
		c2 = tolower(c2);
	} while (c1 == c2 && c1 != 0);

	return (c1 - c2);
}
#endif

/*
 * By Bert Gijsbers, derived from Ian Malcom Brown's strcasecmp().
 */
#ifndef HAVE_STRNCASECMP
int strncasecmp(const char *str1, const char *str2, size_t n)
{
	int c1, c2;

	do {
		if (n-- <= 0) {
			return 0;
		}
		c1 = *str1++;
		c2 = *str2++;
		c1 = tolower(c1);
		c2 = tolower(c2);
	} while (c1 == c2 && c1 != 0);

	return (c1 - c2);
}
#endif

#ifndef HAVE_STRDUP
char *strdup(const char *old_string)
{
	char *new_string;
	size_t string_length;

	string_length = strlen(old_string);
	new_string = (char *) malloc(string_length + 1);
	if (new_string) {
		memcpy(new_string, old_string, string_length + 1);
	}

	return new_string;
}
#endif

char *xp_safe_strdup(const char *old_string)
{
	char *new_string;

	new_string = strdup(old_string);
	if (new_string == NULL) {
		fatal("Not enough memory.");
	}

	return new_string;
}

/*
    NAME
	strlcpy
    ARGS
	char *dest
	const char *src
	size_t size
    DESC
	Copy src to dest.
	Dest may hold at most size - 1 characters
	and will always be NUL terminated,
	except if size equals zero.
	Return strlen(src).
	There was not enough room in dest if the
	return value is bigger or equal than size.
*/
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dest, const char *src, size_t size)
{
	register char *d = dest;
	register const char *s = src;
	register char *maxd = dest + (size - 1);

	if (size > 0) {
		while (*s && d < maxd) {
			*d = *s;
			s++;
			d++;
		}
		*d = '\0';
	}
	while (*s) {
		s++;
	}
	return (s - src);
}
#endif

/*
    NAME
	strlcat
    ARGS
	char *dest
	const char *src
	size_t size
    DESC
	Append src to dest.
	Dest may hold at most size - 1 characters
	and will always be NUL terminated,
	except if size equals zero.
	Return strlen(src) + strlen(dest).
	There was not enough room in dest if the
	return value is bigger or equal than size.
*/
#ifndef HAVE_STRLCAT
size_t strlcat(char *dest, const char *src, size_t size)
{
	register char *d = dest;
	register const char *s = src;
	register char *maxd = dest + (size - 1);
	size_t dlen = 0;

	if (size > 0) {
		while (*d && d < maxd) {
			d++;
		}
		dlen = (d - dest);
		while (*s && d < maxd) {
			*d = *s;
			s++;
			d++;
		}
		*d = '\0';
	}
	while (*s) {
		s++;
	}
	return dlen + (s - src);
}
#endif

void *xp_safe_malloc(size_t size)
{
	void *p;

	p = (void *) malloc(size);
	if (p == NULL) {
		fatal("Not enough memory.");
	}

	return p;
}

void *xp_safe_realloc(void *oldptr, size_t size)
{
	void *p;

	p = (void *) realloc(oldptr, size);
	if (p == NULL) {
		fatal("Not enough memory.");
	}

	return p;
}

void *xp_safe_calloc(size_t nmemb, size_t size)
{
	void *p;

	p = (void *) calloc(nmemb, size);
	if (p == NULL) {
		fatal("Not enough memory.");
	}

	return p;
}

void xp_safe_free(void *p)
{
	if (p) {
		free(p);
	}
}


int Get_process_id(void)
{
#if defined(_WINDOWS)
	return _getpid();
#else
	return getpid();
#endif
}

void Get_login_name(char *buf, size_t size)
{
#if defined(_WINDOWS)
	long nsize = size;
	GetUserName(buf, &nsize);
	buf[size - 1] = '\0';
#else
	/* Unix */
	struct passwd *p;

	setpwent();
	if ((p = getpwuid(geteuid())) != NULL)
		strlcpy(buf, p->pw_name, size);
	else
		strlcpy(buf, "nameless", size);
	endpwent();
#endif
}

int xpprintf(const char *fmt, ...)
{
	int result;
	va_list argp;
	va_start(argp, fmt);
	result = vprintf(fmt, argp);
	va_end(argp);
#ifdef _WINDOWS
	fflush(stdout);
#endif
	return result;
}

#if 0
bool is_this_windows(void)
{
#ifdef _WINDOWS
	return true;
#else
	return false;
#endif
}
#endif

/*
 * Round to nearest integer.
 */
#ifdef _WINDOWS
double rint(double x)
{
	return floor((x < 0.0) ? (x - 0.5) : (x + 0.5));
}

#ifdef NEED_GETTIMEOFDAY
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	LARGE_INTEGER li;
	__int64 t;
	static int tzflag;

	if (tv) {
		GetSystemTimeAsFileTime(&ft);
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		t = li.QuadPart;	/* In 100-nanosecond intervals */
		t -= EPOCHFILETIME;	/* Offset to the Epoch time */
		t /= 10;	/* In microseconds */
		tv->tv_sec = (long) (t / 1000000);
		tv->tv_usec = (long) (t % 1000000);
	}

	if (tz) {
		if (!tzflag) {
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;

}
#endif
#endif
