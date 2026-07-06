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
 * Copyright (C) 2003-2004 Kristian Söderblom kps at users.sourceforge.net
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

#include "sysdeps.h"
#include "const.h"
#include "error.h"
#include "global.h"
#include "option.h"
#include "proto.h"
#include "portability.h"

typedef struct xpilotrc_line {
	xp_option_t *opt;
	const char *comment;
} xpilotrc_line_t;

static xpilotrc_line_t *xpilotrc_lines = NULL;
static int num_xpilotrc_lines = 0, max_xpilotrc_lines = 0;
static int num_ok_options = 0;

/*
 * Function to parse an xpilotrc line if it is of the right form, otherwise
 * it is treated as a comment.
 *
 * Line must have this form to be valid:
 * 1. string "xpilot", checked for case insensitively
 * 2. "." or "*"
 * 3. option name of option recognised by Get_option_by_name().
 * 4. optional whitespace
 * 5. ":"
 * 6. optional whitespace
 * 7. optional value (if it doesn't exist, set option to value "")
 * 8. optional ";" followed by comments
 *
 * Here is some sort of pseudo regular expression:
 * xpilot{.*}option[whitespace]:[whitespace][value][; comment]
 */
static void Parse_xpilotrc_line(const char *line)
{
	char *lcpy = xp_safe_strdup(line);
	char *l = lcpy, *colon, *name, *value, *semicolon, *comment = NULL;
	xpilotrc_line_t t;
	xp_option_t *opt;
	int i;

	memset(&t, 0, sizeof(xpilotrc_line_t));

	if (!(strncasecmp(l, "xpilot.", 7) == 0 || strncasecmp(l, "xpilot*", 7) == 0))
		goto line_is_comment;

	/* line starts with "xpilot." or "xpilot*" (ignoring case) */
	l += strlen("xpilot.");

	colon = strchr(l, ':');
	if (colon == NULL) {
		/* no colon on line, not ok */
		warn("Xpilotrc line %d:", num_xpilotrc_lines + 1);
		warn("Line has no colon after option name, ignoring.");
		goto line_is_comment;
	}

	/*
	 * Divide line into two parts, the first part containing the option
	 * name and possible whitespace, the other one containing possible
	 * white space, the option value and possibly a comment.
	 */
	*colon = '\0';

	/* remove trailing whitespace from option name */
	i = ((int) strlen(l)) - 1;
	while (i >= 0 && isspace(l[i]))
		l[i--] = '\0';

	name = l;
	/*warn("looking for option \"%s\": %s",
	   name, Get_option_by_name(name) ? "found" : "not found"); */

	opt = Get_option_by_name(name);
	if (opt == NULL)
		goto line_is_comment;

	if (Option_get_flags(opt) & XP_OPTFLAG_NEVER_SAVE) {
		/* discard the line */
		warn("Xpilotrc line %d:", num_xpilotrc_lines + 1);
		warn("Option %s must not be specified in xpilotrc.", name);
		warn("It will be removed from xpilotrc if you save configuration.");
		XFREE(lcpy);
		return;
	}

	/* did we see this before ? */
	for (i = 0; i < num_xpilotrc_lines; i++) {
		xpilotrc_line_t *x = &xpilotrc_lines[i];

		if (x->opt == opt) {
			/* same option defined several times in xpilotrc */
			warn("Xpilotrc line %d:", num_xpilotrc_lines + 1);
			warn("Option %s previously given on line %d, ignoring new value.", name,
			     i + 1);
			goto line_is_comment;
		}
	}

	/* option is known: proceed to handle the value */
	l = colon + 1;

	/* skip initial whitespace in value */
	while (isspace(*l))
		l++;

	value = l;
	/* everything after semicolon is comment, ignore it. */
	semicolon = strchr(l, ';');
	if (semicolon) {
		/*warn("found comment \"%s\" on line \"%s\"\n", semicolon, line); */
		comment = xp_safe_strdup(semicolon);
		*semicolon = '\0';
	}

	if (!Set_option(name, value, xp_option_origin_xpilotrc)) {
		warn("Xpilotrc line %d:", num_xpilotrc_lines + 1);
		warn("Failed to set option %s value \"%s\", ignoring.", name, value);
		goto line_is_comment;
	}

	/*warn("option %s value is \"%s\"", name, value); */

	t.opt = opt;
	t.comment = comment;
	STORE(xpilotrc_line_t, xpilotrc_lines, num_xpilotrc_lines, max_xpilotrc_lines, t);
	num_ok_options++;
	XFREE(lcpy);
	return;

      line_is_comment:
	/*
	 * If we can't parse the line, then we just treat it as a comment.
	 */
	/*warn("Comment: \"%s\"", line); */
	XFREE(comment);
	t.opt = NULL;
	t.comment = xp_safe_strdup(line);
	STORE(xpilotrc_line_t, xpilotrc_lines, num_xpilotrc_lines, max_xpilotrc_lines, t);
	XFREE(lcpy);
}

static inline bool is_noarg_option(const char *name)
{
	xp_option_t *opt = Get_option_by_name(name);

	if (!opt || opt->type != xp_noarg_option)
		return false;
	return true;
}

int Xpilotrc_read(const char *path)
{
	char buf[4096];		/* long enough max line length in xpilotrc? */
	FILE *fp;

	assert(path);
	if (strlen(path) == 0) {
		warn("Xpilotrc_read: Zero length filename.");
		return -1;
	}

	fp = fopen(path, "r");
	if (fp == NULL) {
		error("Xpilotrc_read: Failed to open file \"%s\"", path);
		return -2;
	}

	xpinfo("Reading options from xpilotrc file %s.", path);

	while (fgets(buf, sizeof buf, fp)) {
		char *cp;

		cp = strchr(buf, '\n');
		if (cp)
			*cp = '\0';
		cp = strchr(buf, '\r');
		if (cp)
			*cp = '\0';
		Parse_xpilotrc_line(buf);
	}

	/*warn("Total number of nonempty lines in xpilotrc: %d",
	   num_xpilotrc_lines);
	   warn("Number of options set: %d\n", num_ok_options); */

	fclose(fp);

	return 0;
}


#define TABSIZE 8
static void Xpilotrc_create_line(char *buf, size_t size,
				 xp_option_t * opt, const char *comment, bool comment_whole_line)
{
	int len, numtabs, i;

	assert(buf != NULL);

	if (comment_whole_line) {
		const char *s = "; ";

		assert(size > strlen(s));
		strlcpy(buf, s, size);
		buf += strlen(s);
		size -= strlen(s);
	}
	else
		strcpy(buf, "");

	if (opt) {
		const char *value;

		snprintf(buf, size, "xpilot.%s:", opt->name);
		len = (int) strlen(buf);
		/* assume tabs are max size of TABSIZE */
		numtabs = ((5 * TABSIZE - 1) - len) / TABSIZE;
		for (i = 0; i < numtabs; i++)
			strlcat(buf, "\t", size);
		value = Option_value_to_string(opt);
		if (value)
			strlcat(buf, value, size);
	}

	if (comment)
		strlcat(buf, comment, size);
}

#undef TABSIZE

static void Xpilotrc_write_line(FILE * fp, const char *buf)
{
#ifndef _WINDOWS
	const char *endline = "\n";
#else
	const char *endline = "\r\n";	/* CR LF */
#endif
	/*warn("writing line \"%s\"", buf); */

	fprintf(fp, "%s%s", buf, endline);
}

int Xpilotrc_write(const char *path)
{
	FILE *fp;
	int i;

	assert(path);
	if (strlen(path) == 0) {
		warn("Xpilotrc_write: Zero length filename.");
		return -1;
	}

	fp = fopen(path, "w");
	if (fp == NULL) {
		error("Xpilotrc_write: Failed to open file \"%s\"", path);
		return -2;
	}

	/* make sure all options are in the xpilotrc */
	for (i = 0; i < num_options; i++) {
		xp_option_t *opt = Option_by_index(i);
		xp_option_origin_t origin;
		xpilotrc_line_t t;
		int j;
		bool was_in_xpilotrc = false;

		memset(&t, 0, sizeof(xpilotrc_line_t));

		for (j = 0; j < num_xpilotrc_lines; j++) {
			xpilotrc_line_t *lp = &xpilotrc_lines[j];

			if (lp->opt == opt)
				was_in_xpilotrc = true;
		}

		if (was_in_xpilotrc)
			continue;

		/* If this wasn't in xpilotrc, don't save it */
		if (Option_get_flags(opt) & XP_OPTFLAG_KEEP)
			continue;
		/* Let's not save these */
		if (Option_get_flags(opt) & XP_OPTFLAG_NEVER_SAVE)
			continue;

		origin = Option_get_origin(opt);
		assert(origin != xp_option_origin_xpilotrc);

		if (origin == xp_option_origin_cmdline)
			continue;
		if (origin == xp_option_origin_env)
			continue;

		/*
		 * Let's save commented default values to xpilotrc, unless
		 * such a comment already exists.
		 */
		if (origin == xp_option_origin_default) {
			char buf[4096];
			bool found = false;

			Xpilotrc_create_line(buf, sizeof(buf), opt, NULL, true);

			for (j = 0; j < num_xpilotrc_lines; j++) {
				xpilotrc_line_t *lp = &xpilotrc_lines[j];

				if (lp->opt == NULL && lp->comment != NULL
				    && !strcmp(buf, lp->comment)) {
					found = true;
					break;
				}
			}

			if (found)
				/* was already in xpilotrc */
				continue;

			t.comment = xp_safe_strdup(buf);
		}
		else
			t.opt = opt;

		STORE(xpilotrc_line_t, xpilotrc_lines, num_xpilotrc_lines, max_xpilotrc_lines, t);
	}

	for (i = 0; i < num_xpilotrc_lines; i++) {
		char buf[4096];
		xpilotrc_line_t *lp = &xpilotrc_lines[i];

		Xpilotrc_create_line(buf, sizeof(buf), lp->opt, lp->comment, false);

		Xpilotrc_write_line(fp, buf);
	}

	fclose(fp);

	return 0;
}

void Usage(void)
{
	int i;

	printf("Usage: %s [-options ...] [server]\n" "Where options include:\n" "\n",
	       Program_name());
	for (i = 0; i < num_options; i++) {
		xp_option_t *opt = Option_by_index(i);

		printf("    -%s %s\n", opt->name, (opt->type != xp_noarg_option) ? "<value>" : "");
		if (opt->help && opt->help[0]) {
			const char *str;
			printf("        ");
			for (str = opt->help; *str; str++) {
				putchar(*str);
				if (*str == '\n' && str[1])
					printf("        ");
			}
			if (str[-1] != '\n')
				putchar('\n');
		}
		Option_print_default_value(opt);
		printf("\n");
	}
	printf("Most of these options can also be set in the .xpilotrc file\n"
	       "in your home directory.\n"
	       "Each key option may have multiple keys bound to it and\n"
	       "one key may be used by multiple key options.\n"
	       "If no server is specified on the command line, xpilot will\n"
	       "display a welcome screen where you can select a server.\n");

	exit(1);
}

void Version(void)
{
	printf("%s %s\n", Program_name(), VERSION);
	exit(0);
}

void Parse_options(int *argcp, char **argvp)
{
	int arg_ind, num_remaining_args, num_servers = 0, i;
	char path[PATH_MAX + 1];

	Xpilotrc_get_filename(path, sizeof(path));
	Xpilotrc_read(path);

	/*
	 * Here we step trough argc - 1 arguments, leaving
	 * only the arguments that might be server names.
	 */
	arg_ind = 1;
	num_remaining_args = *argcp - 1;

	while (num_remaining_args > 0) {
		if (argvp[arg_ind][0] == '-') {
			char *arg = &argvp[arg_ind][1];

			/*
			 * kps -
			 * Incomplete GNU style option support, this only works for
			 * options with no argument, e.g. --version
			 * A complete implementation should also support option given
			 * like this:
			 * --option=value
			 */
			if (arg[0] == '-')
				arg++;

			if (is_noarg_option(arg)) {
				Set_option(arg, "true", xp_option_origin_cmdline);
				num_remaining_args--;
				for (i = 0; i < num_remaining_args; i++)
					argvp[arg_ind + i] = argvp[arg_ind + i + 1];
			}
			else {
				bool ok = false;

				if (num_remaining_args >= 2) {
					ok = Set_option(arg, argvp[arg_ind + 1],
							xp_option_origin_cmdline);
					if (ok) {
						num_remaining_args -= 2;
						for (i = 0; i < num_remaining_args; i++)
							argvp[arg_ind + i] = argvp[arg_ind + i + 2];
					}
				}

				if (!ok) {
					warn("Unknown or incomplete option '%s'", argvp[arg_ind]);
					warn("Type: %s -help to see a list of options", argvp[0]);
					exit(1);
				}
			}
		}
		else {
			/* assume this is a server name. */
			arg_ind++;
			num_remaining_args--;
			num_servers++;
		}
	}

	/*
	 * The remaining args are assumed to be names of servers to try to contact.
	 * + 1 is for the program name.
	 */
	for (i = num_servers + 1; i < *argcp; i++)
		argvp[i] = NULL;
	*argcp = num_servers + 1;
}


const char *Get_keyHelpString(keys_t key)
{
	int i;
	char *nl;
	static char buf[MAX_CHARS];

	for (i = 0; i < num_options; i++) {
		xp_option_t *opt = Option_by_index(i);

		if (opt->key == key) {
			strlcpy(buf, opt->help, sizeof buf);
			if ((nl = strchr(buf, '\n')) != NULL)
				*nl = '\0';
			return buf;
		}
	}

	return NULL;
}


const char *Get_keyResourceString(keys_t key)
{
	int i;

	for (i = 0; i < num_options; i++) {
		xp_option_t *opt = Option_by_index(i);

		if (opt->key == key)
			return opt->name;
	}

	return NULL;
}

#ifndef _WINDOWS
void Xpilotrc_get_filename(char *path, size_t size)
{
	const char *home = getenv("HOME");
	const char *defaultFile = ".xpilotrc";
	const char *optionalFile = getenv("XPILOTRC");

	if (optionalFile != NULL)
		strlcpy(path, optionalFile, size);
	else if (home != NULL) {
		strlcpy(path, home, size);
		strlcat(path, "/", size);
		strlcat(path, defaultFile, size);
	}
	else
		strlcpy(path, "", size);
}
#else
void Xpilotrc_get_filename(char *path, size_t size)
{
	strlcpy(path, "xpilotrc.txt", size);
}
#endif				/* _WINDOWS */
