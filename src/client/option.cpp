/*
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 2003-2007 Kristian Söderblom kps at users.sourceforge.net
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

/* $Id: option.c,v 1.20 2007/12/02 22:40:36 kps Exp $ */

#include "sysdeps.h"
#include "option.h"
#include "const.h"
#include "portability.h"
#include "proto.h"
#include "error.h"

int num_options = 0;
int max_options = 0;

xp_option_t *options = NULL;

xp_option_t *Get_option_by_name(const char *name)
{
	int i;

	for (i = 0; i < num_options; i++) {
		if (!strcasecmp(name, options[i].name))
			return &options[i];
	}

	return NULL;
}

void Color_index_to_string(int val, char *buf, size_t size)
{
	if (val >= 0 && val < MAX_COLORS)
		snprintf(buf, size, "%d", val);
	else if (val >= MAX_COLORS && val < MAX_DEFAULT_COLORS)
		snprintf(buf, size, "^%c", Color_code_by_index(val - MAX_COLORS));
	else
		snprintf(buf, size, "off");
}

static const char *Option_default_value_to_string(xp_option_t * opt)
{
	static char buf[4096];

	switch (opt->type) {
	case xp_noarg_option:
		strcpy(buf, "");
		break;
	case xp_bool_option:
		sprintf(buf, "%s", opt->bool_defval ? "yes" : "no");
		break;
	case xp_int_option:
		sprintf(buf, "%d", opt->int_defval);
		break;
	case xp_color_index_option:
		Color_index_to_string(opt->color_index_defval, buf, sizeof(buf));
		break;
	case xp_double_option:
		sprintf(buf, "%.3f", opt->dbl_defval);
		break;
	case xp_string_option:
		if (opt->str_defval && strlen(opt->str_defval) > 0)
			strlcpy(buf, opt->str_defval, sizeof(buf));
		else
			strcpy(buf, "");
		break;
	case xp_key_option:
		if (opt->key_defval && strlen(opt->key_defval) > 0)
			strlcpy(buf, opt->key_defval, sizeof(buf));
		else
			strcpy(buf, "");
		break;
	default:
		assert(0 && "Unknown option type");
	}
	return buf;
}


void Option_print_default_value(xp_option_t * opt)
{
	const char *defval = Option_default_value_to_string(opt);

	switch (opt->type) {
	case xp_noarg_option:
		break;
	case xp_bool_option:
	case xp_int_option:
	case xp_color_index_option:
	case xp_double_option:
	case xp_string_option:
		if (strlen(defval) > 0)
			printf("        The default value is: %s.\n", defval);
		else
			printf("        There is no default value for this option.\n");
		break;

	case xp_key_option:
		if (opt->key_defval && strlen(opt->key_defval) > 0)
			printf("        The default %s: %s.\n",
			       (strchr(opt->key_defval, ' ') == NULL ? "key is" : "keys are"),
			       opt->key_defval);
		else
			printf("        There is no default value for this option.\n");
		break;
	default:
		assert(0 && "Unknown option type");
	}
}

bool Set_noarg_option(xp_option_t * opt, bool value, xp_option_origin_t origin)
{
	assert(opt);
	assert(opt->type == xp_noarg_option);
	assert(opt->noarg_ptr);

	*opt->noarg_ptr = value;
	opt->origin = origin;

	return true;
}


bool Set_bool_option(xp_option_t * opt, bool value, xp_option_origin_t origin)
{
	bool retval = true;

	assert(opt);
	assert(opt->type == xp_bool_option);
	assert(opt->bool_ptr);

	if (opt->bool_setfunc)
		retval = opt->bool_setfunc(opt, value);
	else
		*opt->bool_ptr = value;

	if (retval)
		opt->origin = origin;

	return retval;
}

bool Set_int_option(xp_option_t * opt, int value, xp_option_origin_t origin)
{
	bool retval = true;

	assert(opt);
	assert(opt->type == xp_int_option);
	assert(opt->int_ptr);

	if (origin == xp_option_origin_setcmd) {
		if (value < opt->int_minval)
			Add_message("Minimum value for option %s is %d. [*Client reply*]",
				    opt->name, opt->int_minval);
		if (value > opt->int_maxval)
			Add_message("Maximum value for option %s is %d. [*Client reply*]",
				    opt->name, opt->int_maxval);
	}
	else {
		if (!(value >= opt->int_minval && value <= opt->int_maxval)) {
			warn("Bad value %d for option \"%s\", using default...", value, opt->name);
			value = opt->int_defval;
		}
	}

	LIMIT(value, opt->int_minval, opt->int_maxval);

	if (opt->int_setfunc)
		retval = opt->int_setfunc(opt, value);
	else
		*opt->int_ptr = value;

	if (retval)
		opt->origin = origin;

	return retval;
}

bool Set_color_index_option(xp_option_t * opt, int value, xp_option_origin_t origin)
{
	bool retval = true;
	bool ok = false;

	assert(opt);
	assert(opt->type == xp_color_index_option);
	assert(opt->color_index_ptr);

	/*
	 * Figure out what integer value the given string value means.
	 */
	if (value == NO_COLOR)
		ok = true;
	if (value >= 0 && value <= MAX_DEFAULT_COLORS - 1)
		ok = true;

	if (origin == xp_option_origin_setcmd) {
		if (!ok)
			Add_message("Legal color values are 'off', 0 to %d, "
				    "or ^^ followed by a character. [*Client reply*]",
				    MAX_COLORS - 1);
	}
	else {
		if (!ok) {
			warn("Bad value %d for option \"%s\", using default...", value, opt->name);
			value = opt->color_index_defval;
		}
	}

	if (value != NO_COLOR) {
		LIMIT(value, 0, MAX_DEFAULT_COLORS - 1);
	}

	if (opt->color_index_setfunc)
		retval = opt->color_index_setfunc(opt, value);
	else
		*opt->color_index_ptr = value;

	if (retval)
		opt->origin = origin;

	return retval;
}

bool Set_double_option(xp_option_t * opt, double value, xp_option_origin_t origin)
{
	bool retval = true;

	assert(opt);
	assert(opt->type == xp_double_option);
	assert(opt->dbl_ptr);

	if (origin == xp_option_origin_setcmd) {
		if (value < opt->dbl_minval)
			Add_message("Minimum value for option %s is %.3f. [*Client reply*]",
				    opt->name, opt->dbl_minval);

		if (value > opt->dbl_maxval)
			Add_message("Maximum value for option %s is %.3f. [*Client reply*]",
				    opt->name, opt->dbl_maxval);
	}
	else {
		if (!(value >= opt->dbl_minval && value <= opt->dbl_maxval)) {
			warn("Bad value %.3f for option \"%s\", using default...", value,
			     opt->name);
			value = opt->dbl_defval;
		}
	}

	LIMIT(value, opt->dbl_minval, opt->dbl_maxval);

	if (opt->dbl_setfunc)
		retval = opt->dbl_setfunc(opt, value);
	else
		*opt->dbl_ptr = value;

	if (retval)
		opt->origin = origin;

	return retval;
}

bool Set_string_option(xp_option_t * opt, const char *value, xp_option_origin_t origin)
{
	bool retval = true;

	assert(opt);
	assert(opt->type == xp_string_option);
	assert(opt->str_ptr || (opt->str_setfunc && opt->str_getfunc));
	assert(value);		/* allow NULL ? */

	/*
	 * The reason string options don't assume a static area is that that
	 * would not allow dynamically allocated strings of arbitrary size.
	 */
	if (opt->str_setfunc)
		retval = opt->str_setfunc(opt, value);
	else
		strlcpy(opt->str_ptr, value, opt->str_size);

	if (retval)
		opt->origin = origin;

	return retval;
}

xp_keydefs_t *keydefs = NULL;
int num_keydefs = 0;
int max_keydefs = 0;


/*
 * This function is used when platform specific code has an event where
 * the user has pressed or released the key defined by the keysym 'ks'.
 * When the key state has changed, the first call to this function should have
 * 'reset' true, then following calls related to the same event should
 * have 'reset' false. For each returned xpilot key, the calling code
 * should call some handler. The function should be called until it returns
 * KEY_DUMMY.
 */
keys_t Generic_lookup_key(xp_keysym_t ks, bool reset)
{
	keys_t ret = KEY_DUMMY;
	static int i = 0;

	if (reset)
		i = 0;

	/*
	 * Variable 'i' is already initialized.
	 * Use brute force linear search to find the key.
	 */
	for (; i < num_keydefs; i++) {
		if (ks == keydefs[i].keysym) {
			ret = keydefs[i].key;
			i++;
			break;
		}
	}

	return ret;
}

static void Store_keydef(int ks, keys_t key)
{
	int i;
	xp_keydefs_t keydef;

	/*
	 * first check if pair (ks, key) already exists 
	 */
	for (i = 0; i < num_keydefs; i++) {
		xp_keydefs_t *kd = &keydefs[i];

		if (kd->keysym == ks && kd->key == key) {
			/*warn("Pair (%d, %d) exist from before", ks, (int) key); */
			/*
			 * already exists, no need to store 
			 */
			return;
		}
	}

	keydef.keysym = ks;
	keydef.key = key;

	/*
	 * find first KEY_DUMMY after lazy deletion 
	 */
	for (i = 0; i < num_keydefs; i++) {
		xp_keydefs_t *kd = &keydefs[i];

		if (kd->key == KEY_DUMMY) {
			assert(kd->keysym == XP_KS_UNKNOWN);
			/*warn("Store_keydef: Found dummy at index %d", i); */
			*kd = keydef;
			return;
		}
	}

	/*
	 * no lazily deleted entry, ok, just store it then
	 */
	STORE(xp_keydefs_t, keydefs, num_keydefs, max_keydefs, keydef);
}

static void Remove_key_from_keydefs(keys_t key)
{
	int i;

	assert(key != KEY_DUMMY);
	for (i = 0; i < num_keydefs; i++) {
		xp_keydefs_t *kd = &keydefs[i];

		/*
		 * lazy deletion 
		 */
		if (kd->key == key) {
			/*warn("Remove_key_from_keydefs: Removing key at index %d", i); */
			kd->keysym = XP_KS_UNKNOWN;
			kd->key = KEY_DUMMY;
		}
	}
}

bool Set_key_option(xp_option_t * opt, const char *value, xp_option_origin_t origin)
{
	/*bool retval = true; */
	char *str, *valcpy;

	assert(opt);
	assert(opt->name);
	assert(opt->type == xp_key_option);
	assert(opt->key != KEY_DUMMY);
	assert(value);

	/*
	 * warn("Setting key option %s to \"%s\"", opt->name, value); 
	 */

	/*
	 * First remove the old setting.
	 */
	XFREE(opt->key_string);
	Remove_key_from_keydefs(opt->key);

	/*
	 * Store the new setting.
	 */
	opt->key_string = xp_safe_strdup(value);
	valcpy = xp_safe_strdup(value);
	for (str = strtok(valcpy, " \t\r\n"); str != NULL; str = strtok(NULL, " \t\r\n")) {
		xp_keysym_t ks = XP_KS_UNKNOWN;

		/*
		 * You can write "none" for keys in xpilotrc to disable the key.
		 */
		if (!strcasecmp(str, "none"))
			continue;

		if (!strncasecmp(str, "mousebutton", 11)) {
			int button = atoi(&str[11]);
			extern int max_mouse_button;

			if (button >= 1) {
				ks = XP_KS_MOUSEBUTTON(button);
				if (button > max_mouse_button)
					max_mouse_button = button;
			}
		}

		if (ks == XP_KS_UNKNOWN)
			ks = String_to_xp_keysym(str);

		if (ks == XP_KS_UNKNOWN) {
			warn("Invalid keysym \"%s\" for key \"%s\".\n", str, opt->name);
			continue;
		}

		Store_keydef(ks, opt->key);
	}

	/* in fact if we only get invalid keysyms we should return false */
	opt->origin = origin;
	XFREE(valcpy);
	return true;
}

int Color_index_string_to_int(const char *value)
{
	int intval = ILLEGAL_COLOR;

	if (OFF(value))
		return NO_COLOR;
	if (strlen(value) == 2 && value[0] == '^')
		/* These have index 16 to 47 with current MAX_COLORS. */
		return Index_by_color_code(value[1]) + MAX_COLORS;
	if (sscanf(value, "%d", &intval) <= 0)
		return ILLEGAL_COLOR;
	if (intval < 0)
		return ILLEGAL_COLOR;
	if (intval >= MAX_COLORS)
		return ILLEGAL_COLOR;
	return intval;
}

static bool is_legal_value(xp_option_type_t type, const char *value)
{
	if (type == xp_noarg_option || type == xp_bool_option) {
		if (ON(value) || OFF(value))
			return true;
		return false;
	}
	if (type == xp_int_option) {
		int foo;

		if (sscanf(value, "%d", &foo) <= 0)
			return false;
		return true;
	}
	if (type == xp_color_index_option) {
		int foo = Color_index_string_to_int(value);
		if (foo == ILLEGAL_COLOR)
			return false;
		return true;
	}
	if (type == xp_double_option) {
		double foo;

		if (sscanf(value, "%lf", &foo) <= 0)
			return false;
		return true;
	}
	return true;
}


bool Set_option(const char *name, const char *value, xp_option_origin_t origin)
{
	xp_option_t *opt;

	opt = Get_option_by_name(name);
	if (!opt)
		/* unknown */
		return false;

	if (!is_legal_value(opt->type, value)) {
		if (origin != xp_option_origin_setcmd)
			warn("Bad value \"%s\" for option %s.", value, opt->name);
		else {
			Add_message("Bad value \"%s\" for option %s. [*Client reply*]",
				    value, opt->name);
			if (opt->type == xp_color_index_option)
				Add_message("Legal values are off, 0 to %d or ^^ "
					    "followed by another character. [*Client reply*]",
					    MAX_COLORS - 1);
		}
		return false;
	}

	switch (opt->type) {
	case xp_noarg_option:
		return Set_noarg_option(opt, ON(value) ? true : false, origin);
	case xp_bool_option:
		return Set_bool_option(opt, ON(value) ? true : false, origin);
	case xp_int_option:
		return Set_int_option(opt, atoi(value), origin);
	case xp_color_index_option:
		{
			int intval = Color_index_string_to_int(value);
			return Set_color_index_option(opt, intval, origin);
		}
	case xp_double_option:
		return Set_double_option(opt, atof(value), origin);
	case xp_string_option:
		return Set_string_option(opt, value, origin);
	case xp_key_option:
		return Set_key_option(opt, value, origin);
	default:
		assert(0 && "TODO");
	}
	return false;
}


/*
 * kps - these commands need some fine tuning. 
 * TODO - unset a value, i.e. set it to empty 
 */
/*
 * Handler for \set client command.
 */
void Set_command(const char *args)
{
	char *name, *value, *valcpy;
	xp_option_t *opt;

	assert(args);

	valcpy = xp_safe_strdup(args);

	name = strtok(valcpy, " \t\r\n");
	value = strtok(NULL, "");

	opt = Get_option_by_name(name);

	if (!opt) {
		Add_message("Unknown option \"%s\". [*Client reply*]", name);
		goto out;
	}

	if (!value) {
		Add_message("Set command needs an option and a value. " "[*Client reply*]");
		goto out;
	}
	else {
#if 0
		const char *newvalue;
		const char *nm = Option_get_name(opt);
#endif
		Set_option(name, value, xp_option_origin_setcmd);
#if 0
		newvalue = Option_value_to_string(opt);
		Add_message("The value of %s is now %s. [*Client reply*]", nm, newvalue);
#endif
	}

      out:
	XFREE(valcpy);
}

const char *Option_value_to_string(xp_option_t * opt)
{
	static char buf[MSG_LEN];

	switch (opt->type) {
	case xp_noarg_option:
		sprintf(buf, "%s", *opt->noarg_ptr ? "yes" : "no");
		break;
	case xp_bool_option:
		sprintf(buf, "%s", *opt->bool_ptr ? "yes" : "no");
		break;
	case xp_int_option:
		sprintf(buf, "%d", *opt->int_ptr);
		break;
	case xp_color_index_option:
		Color_index_to_string(*opt->color_index_ptr, buf, sizeof(buf));
		break;
	case xp_double_option:
		sprintf(buf, "%.3f", *opt->dbl_ptr);
		break;
	case xp_string_option:
		/*
		 * Assertion in Store_option guarantees one of these is not NULL. 
		 */
		if (opt->str_getfunc)
			return opt->str_getfunc(opt);
		else
			return opt->str_ptr;
	case xp_key_option:
		assert(opt->key_string);
		return opt->key_string;
	default:
		assert(0 && "Unknown option type");
	}
	return buf;
}


/*
 * Handler for \get client command.
 */
void Get_command(const char *args)
{
	char *name, *valcpy;
	xp_option_t *opt;

	assert(args);

	valcpy = xp_safe_strdup(args);

	name = strtok(valcpy, " \t\r\n");
	opt = Get_option_by_name(name);

	if (opt) {
		const char *val = Option_value_to_string(opt);
		const char *nm = Option_get_name(opt);

		if (val && strlen(val) > 0)
			Add_message("The value of %s is %s. [*Client reply*]", nm, val);
		else
			Add_message("The option %s has no value. [*Client reply*]", nm);
	}
	else
		Add_message("No client option named \"%s\". [*Client reply*]", name);

	XFREE(valcpy);
}

/*
 * NOTE: Store option assumes the passed pointers will remain valid.
 */
void Store_option(xp_option_t * opt)
{
	xp_option_t option;

	assert(opt->name);
	assert(strlen(opt->name) > 0);
	assert(opt->help);
	assert(strlen(opt->help) > 0);

	/*
	 * Let's not allow several options with the same name 
	 */
	if (Get_option_by_name(opt->name) != NULL) {
		warn("Trying to store duplicate option \"%s\"", opt->name);
		assert(0);
	}

	/*
	 * Check that default value is in range 
	 * NOTE: these assertions will hold also for options of other types 
	 */
	assert(opt->int_defval >= opt->int_minval);
	assert(opt->int_defval <= opt->int_maxval);
	assert(opt->dbl_defval >= opt->dbl_minval);
	assert(opt->dbl_defval <= opt->dbl_maxval);
	assert(NO_COLOR == -1);
	assert(opt->color_index_defval >= -1);
	assert(opt->color_index_defval <= MAX_DEFAULT_COLORS - 1);

	memcpy(&option, opt, sizeof(xp_option_t));

	STORE(xp_option_t, options, num_options, max_options, option);

	opt = Get_option_by_name(opt->name);
	assert(opt);

	/* Set the default value. */
	switch (opt->type) {
	case xp_noarg_option:
		Set_noarg_option(opt, false, xp_option_origin_default);
		break;
	case xp_bool_option:
		Set_bool_option(opt, opt->bool_defval, xp_option_origin_default);
		break;
	case xp_int_option:
		Set_int_option(opt, opt->int_defval, xp_option_origin_default);
		break;
	case xp_color_index_option:
		Set_color_index_option(opt, opt->color_index_defval, xp_option_origin_default);
		break;
	case xp_double_option:
		Set_double_option(opt, opt->dbl_defval, xp_option_origin_default);
		break;
	case xp_string_option:
		assert(opt->str_defval);
		assert(opt->str_ptr || (opt->str_setfunc && opt->str_getfunc));
		Set_string_option(opt, opt->str_defval, xp_option_origin_default);
		break;
	case xp_key_option:
		assert(opt->key_defval);
		assert(opt->key != KEY_DUMMY);
		Set_key_option(opt, opt->key_defval, xp_option_origin_default);
		break;
	default:
		warn("Could not set default value for option %s", opt->name);
		break;
	}

}
