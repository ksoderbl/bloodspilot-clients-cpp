/*
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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

/* $Id: xpconfig.c,v 1.5 2007/09/27 22:19:46 kps Exp $ */

#include "sysdeps.h"
#include "xpconfig.h"
#include "error.h"

char *Conf_datadir(void)
{
	static char conf[] = CONF_DATADIR;

	return conf;
}

char *Conf_defaults_file_name(void)
{
	static char conf[] = CONF_DEFAULTS_FILE_NAME;

	return conf;
}

char *Conf_fontdir(void)
{
	static char conf[] = CONF_FONTDIR;

	return conf;
}

char *Conf_default_map(void)
{
	static char conf[] = CONF_DEFAULT_MAP;

	return conf;
}

/* needed by client/default.c */
char conf_ship_file_string[] = CONF_SHIP_FILE;

char *Conf_ship_file(void)
{
	static char conf[] = CONF_SHIP_FILE;

	return conf;
}

char *Conf_texturedir(void)
{
	static char conf[] = CONF_TEXTUREDIR;

	return conf;
}

char *Conf_zcat_ext(void)
{
	static char conf[] = CONF_ZCAT_EXT;

	return conf;
}

char *Conf_zcat_format(void)
{
	static char conf[] = CONF_ZCAT_FORMAT;

	return conf;
}

#ifdef SOUND
char *Conf_sounddir(void)
{
	static char conf[] = CONF_SOUNDDIR;

	return conf;
}

char *Conf_soundfile(void)
{
	static char conf[] = CONF_SOUNDFILE;

	return conf;
}
#endif

void Conf_print(void)
{
	warn("======================================================================");
	warn("VERSION            = %s", VERSION);
	// warn("PACKAGE            = %s", PACKAGE);
	warn("PACKAGE_BUGREPORT  = %s", PACKAGE_BUGREPORT);

#ifdef DBE
	warn("DBE");
#endif
#ifdef MBX
	warn("MBX");
#endif
#ifdef PLOCKSERVER
	warn("PLOCKSERVER");
#endif
#ifdef DEVELOPMENT
	warn("DEVELOPMENT");
#endif

	warn("Conf_datadir()     = %s", Conf_datadir());
	warn("Conf_ship_file()   = %s", Conf_ship_file());
	warn("Conf_texturedir()  = %s", Conf_texturedir());
	warn("Conf_fontdir()     = %s", Conf_fontdir());
#ifdef SOUND
	warn("Conf_sounddir()    = %s", Conf_sounddir());
	warn("Conf_soundfile()   = %s", Conf_soundfile());
#endif
	warn("======================================================================");
}
