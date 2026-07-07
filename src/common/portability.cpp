/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2005 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
 *      Kristian Söderblom
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
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

/*
 * This file contains function wrappers around OS specific services.
 */

#include "config.h"

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cctype>

#include <unistd.h>
#include <pwd.h>

#include "portability.h"
#include "xperror.h"

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
#if 0 /* SYSV */
	poll((struct poll *) 0, (size_t) 0, usec / 1000);	/* ms RES */
#endif
    struct timeval timeout;
    timeout.tv_usec = usec % (unsigned long)1000000;
    timeout.tv_sec = usec / (unsigned long)1000000;
    (void)select(0, NULL, NULL, NULL, &timeout);

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

    do
    {
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

    do
    {
        if (n-- <= 0)
        {
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
    new_string = (char *)malloc(string_length + 1);
    if (new_string)
    {
        memcpy(new_string, old_string, string_length + 1);
    }

    return new_string;
}
#endif

char *xp_safe_strdup(const char *old_string)
{
    char *new_string;

    new_string = strdup(old_string);
    if (new_string == NULL)
    {
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
    char *d = dest;
    const char *s = src;
    char *maxd = dest + (size - 1);

    if (size > 0)
    {
        while (*s && d < maxd)
        {
            *d = *s;
            s++;
            d++;
        }
        *d = '\0';
    }
    while (*s)
    {
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
    char *d = dest;
    const char *s = src;
    char *maxd = dest + (size - 1);
    size_t dlen = 0;

    if (size > 0)
    {
        while (*d && d < maxd)
        {
            d++;
        }
        dlen = (d - dest);
        while (*s && d < maxd)
        {
            *d = *s;
            s++;
            d++;
        }
        *d = '\0';
    }
    while (*s)
    {
        s++;
    }
    return dlen + (s - src);
}
#endif

void *xp_safe_malloc(size_t size)
{
    void *p;

    p = (void *)malloc(size);
    if (p == NULL)
    {
        fatal("Not enough memory.");
    }

    return p;
}

void *xp_safe_realloc(void *oldptr, size_t size)
{
    void *p;

    p = (void *)realloc(oldptr, size);
    if (p == NULL)
    {
        fatal("Not enough memory.");
    }

    return p;
}

void *xp_safe_calloc(size_t nmemb, size_t size)
{
    void *p;

    p = (void *)calloc(nmemb, size);
    if (p == NULL)
    {
        fatal("Not enough memory.");
    }

    return p;
}

void xp_safe_free(void *p)
{
    if (p)
    {
        free(p);
    }
}

int Get_process_id(void)
{
    return getpid();
}

void Get_login_name(char *buf, size_t size)
{
    /* Unix */
    struct passwd *p;

    setpwent();
    if ((p = getpwuid(geteuid())) != NULL)
        strlcpy(buf, p->pw_name, size);
    else
        strlcpy(buf, "nameless", size);
    endpwent();
}

int xpprintf(const char *fmt, ...)
{
    int result;
    va_list argp;
    va_start(argp, fmt);
    result = vprintf(fmt, argp);
    va_end(argp);
    return result;
}
