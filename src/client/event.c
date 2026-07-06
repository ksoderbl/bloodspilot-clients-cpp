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
 * Copyright (C) 2003-2007 Kristian Söderblom <kps@users.sourceforge.net>
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
#include "bit.h"
#include "const.h"
#include "error.h"
#include "frame.h"
#include "global.h"
#include "proto.h"
#include "option.h"

static BITV_DECL(keyv, NUM_KEYS);
static unsigned char keyv_new[NUM_KEYS];
int max_mouse_button = 0;

static int Key_get_count(keys_t key);
static bool Key_inc_count(keys_t key);
static bool Key_dec_count(keys_t key);
static void Release_all_mouse_buttons(void);

void Pointer_control_newbie_message(void)
{
	xp_option_t *opt = Get_option_by_name("keyExit");
	char msg[MSG_LEN];
	const char *val;

	if (!newbie)
		return;

	if (!opt)
		return;

	val = Option_value_to_string(opt);
	if (strlen(val) == 0)
		return;

	if (pointerControl)
		snprintf(msg, sizeof(msg), "Mouse steering enabled. " "Key(s) to disable it: %s.",
			 val);
	else
		snprintf(msg, sizeof(msg),
			 "Mouse steering disabled. "
			 "Click background with left mouse button to enable it.");

	Add_newbie_message(msg);
}

void Pointer_control_set_state(bool on)
{
	if (pointerControl == on)
		return;
	Platform_specific_pointer_control_set_state(on);
	pointerControl = on;
	if (!restorePointerControl)
		Pointer_control_newbie_message();
}

void Talk_set_state(bool on, bool team_talk)
{
	teamTalk = team_talk;
	if (talking == on)
		return;
	if (on) {
		/* When enabling talking, disable pointer control if it is enabled. */
		if (pointerControl) {
			restorePointerControl = true;
			Pointer_control_set_state(false);
		}
	}
	Platform_specific_talk_set_state(on);
	if (!on) {
		/* When disabling talking, enable pointer control if it was enabled. */
		if (restorePointerControl) {
			Pointer_control_set_state(true);
			restorePointerControl = false;
		}
	}
	talking = on;
}

int Key_init(void)
{
	int i;

	if (sizeof(keyv) != KEYBOARD_SIZE) {
		warn("%s, %d: keyv size %d, KEYBOARD_SIZE is %d", __FILE__, __LINE__, sizeof(keyv),
		     KEYBOARD_SIZE);
		exit(1);
	}
	memset(keyv, 0, sizeof keyv);
	for (i = 0; i < NUM_KEYS; i++)
		keyv_new[i] = 0;

	BITV_SET(keyv, KEY_SHIELD);

	return 0;
}

int Key_update(void)
{
	return Send_keyboard(keyv);
}

static bool Key_check_talk_macro(keys_t key)
{
	if (key >= KEY_MSG_1 && key < KEY_MSG_1 + TALK_FAST_NR_OF_MSGS)
		Talk_macro((int) (key - KEY_MSG_1));
	return true;
}

static bool Key_press_autoshield_hack(void)
{
	if (auto_shield && BITV_ISSET(keyv, KEY_SHIELD))
		BITV_CLR(keyv, KEY_SHIELD);
	return false;
}

static bool Key_press_shield(keys_t key)
{
	if (toggle_shield) {
		shields = !shields;
		if (shields)
			BITV_SET(keyv, key);
		else
			BITV_CLR(keyv, key);
		return true;
	}
	else if (auto_shield)
		shields = true;

	return false;
}

static bool Key_press_fuel(void)
{
	fuelTime = FUEL_NOTIFY_TIME;
	return false;
}

static bool Key_press_swap_settings(void)
{
	double tmp;
#define SWAP(a, b) (tmp = (a), (a) = (b), (b) = tmp)

	SWAP(power, power_s);
	SWAP(turnspeed, turnspeed_s);
	SWAP(turnresistance, turnresistance_s);
	controlTime = CONTROL_TIME;
	Config_redraw();

	return true;
}

static bool Key_press_swap_scalefactor(void)
{
	double a = altScaleFactor;

	Set_altScaleFactor(NULL, scaleFactor);
	Set_scaleFactor(NULL, a);

	return false;
}

static bool Key_press_increase_power(void)
{
	power = power * 1.10;
	power = MIN(power, MAX_PLAYER_POWER);
	Send_power(power);

	Config_redraw();
	controlTime = CONTROL_TIME;
	return false;		/* server doesn't see these keypresses anymore */

}

static bool Key_press_decrease_power(void)
{
	power = power / 1.10;
	power = MAX(power, MIN_PLAYER_POWER);
	Send_power(power);

	Config_redraw();
	controlTime = CONTROL_TIME;
	return false;		/* server doesn't see these keypresses anymore */
}

static bool Key_press_increase_turnspeed(void)
{
	turnspeed = turnspeed * 1.05;
	turnspeed = MIN(turnspeed, MAX_PLAYER_TURNSPEED);
	Send_turnspeed(turnspeed);

	Config_redraw();
	controlTime = CONTROL_TIME;
	return false;		/* server doesn't see these keypresses anymore */
}

static bool Key_press_decrease_turnspeed(void)
{
	turnspeed = turnspeed / 1.05;
	turnspeed = MAX(turnspeed, MIN_PLAYER_TURNSPEED);
	Send_turnspeed(turnspeed);

	Config_redraw();
	controlTime = CONTROL_TIME;
	return false;		/* server doesn't see these keypresses anymore */
}

static bool Key_press_talk(bool team_talk)
{
	if (pointerControl)
		Release_all_mouse_buttons();

	Talk_set_state(!talking, team_talk);
	return false;		/* server doesn't need to know */
}

static bool Key_press_toggle_items(void)
{
	showItems = !showItems;
	return false;		/* server doesn't need to know */
}

static bool Key_press_pointer_control(void)
{
	Pointer_control_set_state(!pointerControl);
	return false;		/* server doesn't need to know */
}

static bool Key_press_msgs_stdout(void)
{
	Print_messages_to_stdout();
	return false;		/* server doesn't need to know */
}

static bool Key_press_select_lose_item(void)
{
	if (lose_item_active == 1)
		lose_item_active = 2;
	else
		lose_item_active = 1;
	return true;
}

static bool Key_press_toggle_fullscreen(void)
{
	Toggle_fullscreen();
	return false;		/* server doesn't need to know */
}

static bool Key_press_toggle_talk_messages(void)
{
	showTalkMessages = !showTalkMessages;
	return false;		/* server doesn't need to know */
}

static bool Key_press_toggle_game_messages(void)
{
	showGameMessages = !showGameMessages;
	return false;		/* server doesn't need to know */
}

static bool Key_press_toggle_score_list(void)
{
	showScoreListMode++;
	if (showScoreListMode > 2)
		showScoreListMode = 0;
	Client_score_table();
	return false;		/* server doesn't need to know */
}

static bool Key_press_toggle_ext_hud_radar(void)
{
	showExtHUDRadar = !showExtHUDRadar;
	return false;		/* server doesn't need to know */
}

static bool Key_press_toggle_ext_hud(void)
{
	showExtHUD = !showExtHUD;
	return false;		/* server doesn't need to know */
}

static bool Key_press_toggle_record(void)
{
	Record_toggle();
	return false;		/* server doesn't need to know */
}

static bool Key_press_toggle_sound(void)
{
#ifdef SOUND
	sound = !sound;
#endif
	return false;		/* server doesn't need to know */
}



static bool Key_press_yes(void)
{
	/* Handled in other code */
	assert(!quitMode);

	return false;		/* server doesn't need to know */
}

static bool Key_press_no(void)
{
	return false;		/* server doesn't need to know */
}

static bool Key_press_exit(void)
{
	static char msg[MSG_LEN];
	const char *val;
	char valcpy[MSG_LEN];
	xp_option_t *opt = Get_option_by_name("keyYes");

	/* exit pointer control if exit key pressed in pointer control mode */
	if (pointerControl) {
		Release_all_mouse_buttons();

		Pointer_control_set_state(false);
		return false;	/* server doesn't need to know */
	}

	quitMode = true;

	assert(opt != NULL);
	val = Option_value_to_string(opt);
	if (val == NULL
	    || strlen(val) == 0
	    || !strcasecmp(val, "None")) {
		Set_option("keyYes", "F1",
			   xp_option_origin_setcmd /* not really */);
	}
	strlcpy(valcpy, Option_value_to_string(opt), sizeof(valcpy));
	snprintf(msg, sizeof(msg),
		 "Exit game? Press %s for YES.",
		 strtok(valcpy, " ")); /*want only first yes key here*/
	Add_alert_message(msg, 0.0);

	return false;		/* server doesn't need to know */
}

static int Key_get_count(keys_t key)
{
	if (key >= NUM_KEYS)
		return -1;

	return keyv_new[key];
}

static bool Key_inc_count(keys_t key)
{
	if (key >= NUM_KEYS)
		return false;

	if (keyv_new[key] < 255) {
		++keyv_new[key];
		return true;
	}

	return false;
}

static bool Key_dec_count(keys_t key)
{
	if (key >= NUM_KEYS)
		return false;

	if (keyv_new[key] > 0) {
		--keyv_new[key];
		return true;
	}

	return false;
}

void Key_clear_counts(void)
{
	int i;
	bool change = false;

	for (i = 0; i < NUM_KEYS; i++) {
		if (keyv_new[i] > 0) {
			/* set to one so that Key_release(i) will trigger */
			keyv_new[i] = 1;
			change |= Key_release((keys_t) i);
		}
	}

	if (change)
		Net_key_change();
}

/* Remember which key we used to exit quit mode. */
static keys_t quit_mode_exit_key = KEY_DUMMY;

static bool Quit_mode_key_press(keys_t key)
{
	if (key == KEY_YES)
		Client_exit(0);

	if (key == KEY_NO || key == KEY_EXIT) {
		quitMode = false;
		Clear_alert_messages();
		quit_mode_exit_key = key;
	}

	return false;
}

bool Key_press(keys_t key)
{
	bool countchange;
	int keycount;

	if (quitMode)
		return Quit_mode_key_press(key);

	countchange = Key_inc_count(key);
	keycount = Key_get_count(key);

	/*
	 * keycount -1 means this was a client only key, we don't count those
	 */
	if (keycount != -1) {
		/*
		 * if countchange is false it means that Key_<inc|dec>_count()
		 * failed to change the count due to being at the end of the range
		 * (should be very rare) keycount != 1 means that this key was
		 * already pressed (multiple key mappings)
		 */
		if ((!countchange) || (keycount != 1))
			return true;
	}

	Key_check_talk_macro(key);

	switch (key) {
	case KEY_FIRE_SHOT:
	case KEY_FIRE_LASER:
	case KEY_FIRE_MISSILE:
	case KEY_FIRE_TORPEDO:
	case KEY_FIRE_HEAT:
	case KEY_DROP_MINE:
	case KEY_DETACH_MINE:
		Key_press_autoshield_hack();
		break;

	case KEY_SHIELD:
		if (Key_press_shield(key))
			return true;
		break;

	case KEY_REFUEL:
	case KEY_REPAIR:
	case KEY_TANK_NEXT:
	case KEY_TANK_PREV:
		Key_press_fuel();
		break;

	case KEY_SWAP_SETTINGS:
		if (!Key_press_swap_settings())
			return false;
		break;

	case KEY_SWAP_SCALEFACTOR:
		if (!Key_press_swap_scalefactor())
			return false;
		break;

	case KEY_INCREASE_POWER:
		return Key_press_increase_power();

	case KEY_DECREASE_POWER:
		return Key_press_decrease_power();

	case KEY_INCREASE_TURNSPEED:
		return Key_press_increase_turnspeed();

	case KEY_DECREASE_TURNSPEED:
		return Key_press_decrease_turnspeed();

	case KEY_TALK:
		return Key_press_talk(false);

	case KEY_TEAM_TALK:
		return Key_press_talk(true);

	case KEY_TOGGLE_OWNED_ITEMS:
		return Key_press_toggle_items();

	case KEY_POINTER_CONTROL:
		if (pointerControl)
			Release_all_mouse_buttons();

		return Key_press_pointer_control();

	case KEY_TOGGLE_RECORD:
		return Key_press_toggle_record();

	case KEY_TOGGLE_SOUND:
		return Key_press_toggle_sound();

	case KEY_PRINT_MSGS_STDOUT:
		return Key_press_msgs_stdout();

	case KEY_SELECT_ITEM:
	case KEY_LOSE_ITEM:
		if (!Key_press_select_lose_item())
			return false;
		break;


	case KEY_TOGGLE_FULLSCREEN:
		return Key_press_toggle_fullscreen();

	case KEY_TOGGLE_TALK_MESSAGES:
		return Key_press_toggle_talk_messages();

	case KEY_TOGGLE_GAME_MESSAGES:
		return Key_press_toggle_game_messages();

	case KEY_TOGGLE_SCORE_LIST:
		return Key_press_toggle_score_list();

	case KEY_TOGGLE_EXT_HUD_RADAR:
		return Key_press_toggle_ext_hud_radar();

	case KEY_TOGGLE_EXT_HUD:
		return Key_press_toggle_ext_hud();


	case KEY_EXIT:
		return Key_press_exit();
	case KEY_YES:
		return Key_press_yes();
	case KEY_NO:
		return Key_press_no();

	default:
		break;
	}

	if (key < NUM_KEYS)
		BITV_SET(keyv, key);

	return true;
}

bool Key_release(keys_t key)
{
	bool countchange;
	int keycount;

	/*
	 * Make sure nothing is done when we release the button we used
	 * to exit quit mode with.
	 */
	if (key == quit_mode_exit_key) {
		//assert(key != KEY_DUMMY); kps - this assertion should be put back
		quit_mode_exit_key = KEY_DUMMY;
		return false;
	}

	countchange = Key_dec_count(key);
	keycount = Key_get_count(key);

	/* -1 means this was a client only key, we don't count those */
	if (keycount != -1) {
		/*
		 * if countchange is false it means that Key_<inc|dec>_count()
		 * failed to change the count due to being at the end of the range
		 * (happens to most key releases let through from talk mode)
		 * keycount != 0 means that some physical keys remain pressed
		 * that map to this xpilot key 
		 */
		if ((!countchange) || (keycount != 0))
			return true;
	}

	switch (key) {
	case KEY_TOGGLE_OWNED_ITEMS:
	case KEY_TOGGLE_FULLSCREEN:
	case KEY_TOGGLE_TALK_MESSAGES:
	case KEY_TOGGLE_GAME_MESSAGES:
	case KEY_TOGGLE_SCORE_LIST:
	case KEY_TOGGLE_EXT_HUD_RADAR:
	case KEY_TOGGLE_EXT_HUD:
	case KEY_TOGGLE_RECORD:
	case KEY_EXIT:
	case KEY_YES:
	case KEY_NO:
	case KEY_TALK:
	case KEY_TEAM_TALK:
		return false;	/* server doesn't need to know */

		/* Don auto-shield hack */
		/* restore shields */
	case KEY_FIRE_SHOT:
	case KEY_FIRE_LASER:
	case KEY_FIRE_MISSILE:
	case KEY_FIRE_TORPEDO:
	case KEY_FIRE_HEAT:
	case KEY_DROP_MINE:
	case KEY_DETACH_MINE:
		if (auto_shield && shields && !BITV_ISSET(keyv, KEY_SHIELD)) {
			/* Here We need to know if any other weapons are still on */
			/*      before we turn shield back on   */
			BITV_CLR(keyv, key);
			if (!BITV_ISSET(keyv, KEY_FIRE_SHOT) &&
			    !BITV_ISSET(keyv, KEY_FIRE_LASER) &&
			    !BITV_ISSET(keyv, KEY_FIRE_MISSILE) &&
			    !BITV_ISSET(keyv, KEY_FIRE_TORPEDO) &&
			    !BITV_ISSET(keyv, KEY_FIRE_HEAT) &&
			    !BITV_ISSET(keyv, KEY_DROP_MINE) && !BITV_ISSET(keyv, KEY_DETACH_MINE))
				BITV_SET(keyv, KEY_SHIELD);
		}
		break;

	case KEY_SHIELD:
		if (toggle_shield)
			return false;
		else if (auto_shield)
			shields = false;
		break;

	case KEY_REFUEL:
	case KEY_REPAIR:
		fuelTime = FUEL_NOTIFY_TIME;
		break;

	case KEY_SELECT_ITEM:
	case KEY_LOSE_ITEM:
		if (lose_item_active == 2)
			lose_item_active = 1;
		else
			lose_item_active = -(int) (clientFPS + 0.5);
		break;

	default:
		break;
	}
	if (key < NUM_KEYS)
		BITV_CLR(keyv, key);

	return true;
}

void Reset_shields(void)
{
	if (toggle_shield || auto_shield) {
		BITV_SET(keyv, KEY_SHIELD);
		shields = true;
		if (auto_shield) {
			if (BITV_ISSET(keyv, KEY_FIRE_SHOT) ||
			    BITV_ISSET(keyv, KEY_FIRE_LASER) ||
			    BITV_ISSET(keyv, KEY_FIRE_MISSILE) ||
			    BITV_ISSET(keyv, KEY_FIRE_TORPEDO) ||
			    BITV_ISSET(keyv, KEY_FIRE_HEAT) ||
			    BITV_ISSET(keyv, KEY_DROP_MINE) || BITV_ISSET(keyv, KEY_DETACH_MINE))
				BITV_CLR(keyv, KEY_SHIELD);
		}
		Net_key_change();
	}
}

void Set_auto_shield(bool on)
{
	auto_shield = on;
}

void Set_toggle_shield(bool on)
{
	toggle_shield = on;
	if (toggle_shield) {
		if (auto_shield)
			shields = true;
		else
			shields = (BITV_ISSET(keyv, KEY_SHIELD)) ? true : false;
	}
}

/*
 * Function to call when a button of a pointing device has been pressed.
 * Argument 'button' should be 1 for the first pointer button, etc.
 */
void Pointer_button_pressed(int button)
{
	bool change = false;
	xp_keysym_t ks;
	keys_t key;

	if (button < 1 || button > max_mouse_button) {
		warn("Unknown mouse button %d pressed.", button);
		return;
	}

	ks = XP_KS_MOUSEBUTTON(button);

	for (key = Generic_lookup_key(ks, true);
	     key != KEY_DUMMY;
	     key = Generic_lookup_key(ks, false))
		change |= Key_press(key);

	if (change)
		Net_key_change();
}

void Pointer_button_released(int button)
{
	bool change = false;
	xp_keysym_t ks;
	keys_t key;

	if (button < 1 || button > max_mouse_button) {
		warn("Unknown mouse button %d released.", button);
		return;
	}

	ks = XP_KS_MOUSEBUTTON(button);

	for (key = Generic_lookup_key(ks, true);
	     key != KEY_DUMMY;
	     key = Generic_lookup_key(ks, false))
		change |= Key_release(key);

	if (change)
		Net_key_change();
}

/*
 * This releases mouse, so we clear the mouse buttons so they
 * don't lock on.
 */
static void Release_all_mouse_buttons(void)
{
	int i;

	for (i = 1; i <= max_mouse_button; i++)
		Pointer_button_released(i);
}

void Keyboard_button_pressed(xp_keysym_t ks)
{
	bool change = false;
	keys_t key;

#if 0
	{
		char foo[80];

		sprintf(foo, "keysym = %d (0x%x) []", (int) ks, (int) ks);
		Add_message(foo);
	}
#endif

	for (key = Generic_lookup_key(ks, true);
	     key != KEY_DUMMY;
	     key = Generic_lookup_key(ks, false))
		change |= Key_press(key);

	if (change)
		Net_key_change();
}

void Keyboard_button_released(xp_keysym_t ks)
{
	bool change = false;
	keys_t key;

	for (key = Generic_lookup_key(ks, true);
	     key != KEY_DUMMY;
	     key = Generic_lookup_key(ks, false))
		change |= Key_release(key);

	if (change)
		Net_key_change();
}



/*
 * Generic key options.
 */
xp_option_t key_options[] = {
	XP_KEY_OPTION("keyTurnLeft",
		      "Left",	/* was "a" */
		      KEY_TURN_LEFT,
		      "Turn left (anti-clockwise).\n"),

	XP_KEY_OPTION("keyTurnRight",
		      "Right",	/* was "s" */
		      KEY_TURN_RIGHT,
		      "Turn right (clockwise).\n"),

	XP_KEY_OPTION("keyThrust",
		      "mouseButton2 mouseButton3 Up", /* "Shift_R Shift_L", */
		      KEY_THRUST,
		      "Thrust.\n"),

	XP_KEY_OPTION("keyShield",
		      "space",
		      KEY_SHIELD,
		      "Raise or toggle the shield.\n"),

	XP_KEY_OPTION("keyFireShot",
		      "mouseButton1 Down", /* was "Return Linefeed" */
		      KEY_FIRE_SHOT,
		      "Fire shot.\n" "Note that shields must be down to fire.\n"),

	XP_KEY_OPTION("keyFireMissile",
		      "backslash",
		      KEY_FIRE_MISSILE,
		      "Fire smart missile.\n"),

	XP_KEY_OPTION("keyFireTorpedo",
		      "quoteright",
		      KEY_FIRE_TORPEDO,
		      "Fire unguided torpedo.\n"),

	XP_KEY_OPTION("keyFireHeat",
		      "semicolon",
		      KEY_FIRE_HEAT,
		      "Fire heatseeking missile.\n"),

	XP_KEY_OPTION("keyFireLaser",
		      "slash",
		      KEY_FIRE_LASER,
		      "Activate laser beam.\n"),

	XP_KEY_OPTION("keyDropMine",
		      "", /* was "Tab" */
		      KEY_DROP_MINE,
		      "Drop a stationary mine.\n"),

	XP_KEY_OPTION("keyDetachMine",
		      "bracketright",
		      KEY_DETACH_MINE,
		      "Detach a moving mine.\n"),

	XP_KEY_OPTION("keyDetonateMines",
		      "equal",
		      KEY_DETONATE_MINES,
		      "Detonate the closest mine you have dropped or thrown.\n"),

	XP_KEY_OPTION("keyLockClose",
		      "r", /* was "Up" */
		      KEY_LOCK_CLOSE,
		      "Lock on closest player.\n"),

	XP_KEY_OPTION("keyLockNextClose",
		      "e", /* was "Down" */
		      KEY_LOCK_NEXT_CLOSE,
		      "Lock on next closest player.\n"),

	XP_KEY_OPTION("keyLockNext",
		      "w Next",	/* was "Right Next" - Next is a.k.a. Page Down */
		      KEY_LOCK_NEXT,
		      "Lock on next player.\n"),

	XP_KEY_OPTION("keyLockPrev",
		      "Prior",	/* was "Left Prior" - Prior is a.k.a. Page Up */
		      KEY_LOCK_PREV,
		      "Lock on previous player.\n"),

	XP_KEY_OPTION("keyRefuel",
		      "f", /* was "f Control_L Control_R" */
		      KEY_REFUEL,
		      "Refuel.\n"),

	XP_KEY_OPTION("keyRepair",
		      "f",
		      KEY_REPAIR,
		      "Repair target.\n"),

	XP_KEY_OPTION("keyCloak",
		      "Delete BackSpace",
		      KEY_CLOAK,
		      "Toggle cloakdevice.\n"),

	XP_KEY_OPTION("keyEcm",
		      "bracketleft",
		      KEY_ECM,
		      "Use ECM.\n"),

	XP_KEY_OPTION("keySelfDestruct",
		      "End",
		      KEY_SELF_DESTRUCT,
		      "Toggle self destruct.\n"),

	XP_KEY_OPTION("keyPause",
		      "Pause",
		      KEY_PAUSE,
		      "Toggle pause mode.\n" "When the ship is stationary on its homebase.\n"),

	XP_KEY_OPTION("keySwapSettings",
		      "",
		      KEY_SWAP_SETTINGS,
		      "Swap to alternate control settings.\n"
		      "These are the power, turn speed and turn resistance settings.\n"),

	XP_KEY_OPTION("keySwapScaleFactor",
		      "",
		      KEY_SWAP_SCALEFACTOR,
		      "Swap scalefactor settings.\n"),

	XP_KEY_OPTION("keyChangeHome",
		      "Home h",
		      KEY_CHANGE_HOME,
		      "Change home base.\n" "When the ship is close to a suitable base.\n"),

	XP_KEY_OPTION("keyConnector",
		      "f", /* was "f Control_L Control_R",*/
		      KEY_CONNECTOR,
		      "Connect to a ball.\n"),

	XP_KEY_OPTION("keyDropBall",
		      "d",
		      KEY_DROP_BALL,
		      "Drop a ball.\n"),

	XP_KEY_OPTION("keyTankNext",
		      "", /* was "e" */
		      KEY_TANK_NEXT,
		      "Use the next tank.\n"),

	XP_KEY_OPTION("keyTankPrev",
		      "", /* was "w" */
		      KEY_TANK_PREV,
		      "Use the the previous tank.\n"),

	XP_KEY_OPTION("keyTankDetach",
		      "", /* was "r" */
		      KEY_TANK_DETACH,
		      "Detach the current tank.\n"),

	XP_KEY_OPTION("keyIncreasePower",
		      "KP_Multiply",
		      KEY_INCREASE_POWER,
		      "Increase engine power.\n"),

	XP_KEY_OPTION("keyDecreasePower",
		      "KP_Divide",
		      KEY_DECREASE_POWER,
		      "Decrease engine power.\n"),

	XP_KEY_OPTION("keyIncreaseTurnspeed",
		      "KP_Add",
		      KEY_INCREASE_TURNSPEED,
		      "Increase turnspeed.\n"),

	XP_KEY_OPTION("keyDecreaseTurnspeed",
		      "KP_Subtract",
		      KEY_DECREASE_TURNSPEED,
		      "Decrease turnspeed.\n"),

	XP_KEY_OPTION("keyTransporter",
		      "", /* was t */
		      KEY_TRANSPORTER,
		      "Use transporter to steal an item.\n"),

	XP_KEY_OPTION("keyDeflector",
		      "o",
		      KEY_DEFLECTOR,
		      "Toggle deflector.\n"),

	XP_KEY_OPTION("keyHyperJump",
		      "q",
		      KEY_HYPERJUMP,
		      "Teleport.\n"),

	XP_KEY_OPTION("keyPhasing",
		      "p",
		      KEY_PHASING,
		      "Use phasing device.\n"),

	XP_KEY_OPTION("keyTalk",
		      "t", /* was "m" */
		      KEY_TALK,
		      "Talk.\n"),

	XP_KEY_OPTION("keyTeamTalk",
		      "y",
		      KEY_TEAM_TALK,
		      "Talk to team.\n"),

	XP_KEY_OPTION("keyToggleNuclear",
		      "n",
		      KEY_TOGGLE_NUCLEAR,
		      "Toggle nuclear weapon modifier.\n"),

	XP_KEY_OPTION("keyToggleCluster",
		      "c",
		      KEY_TOGGLE_CLUSTER,
		      "Toggle cluster weapon modifier.\n"),

	XP_KEY_OPTION("keyToggleImplosion",
		      "i",
		      KEY_TOGGLE_IMPLOSION,
		      "Toggle implosion weapon modifier.\n"),

	XP_KEY_OPTION("keyToggleVelocity",
		      "v",
		      KEY_TOGGLE_VELOCITY,
		      "Toggle explosion velocity weapon modifier.\n"),

	XP_KEY_OPTION("keyToggleMini",
		      "x",
		      KEY_TOGGLE_MINI,
		      "Toggle mini weapon modifier.\n"),

	XP_KEY_OPTION("keyToggleSpread",
		      "z",
		      KEY_TOGGLE_SPREAD,
		      "Toggle spread weapon modifier.\n"),

	XP_KEY_OPTION("keyTogglePower",
		      "b",
		      KEY_TOGGLE_POWER,
		      "Toggle power weapon modifier.\n"),

	XP_KEY_OPTION("keyToggleLockDisplay",
		      "KP_7",
		      KEY_TOGGLE_LOCK_DISPLAY,
		      "Toggle lock display on HUD.\n"),

	XP_KEY_OPTION("keyToggleAutoPilot",
		      "h",
		      KEY_TOGGLE_AUTOPILOT,
		      "Toggle automatic pilot mode.\n"),

	XP_KEY_OPTION("keyToggleLaser",
		      "l",
		      KEY_TOGGLE_LASER,
		      "Toggle laser modifier.\n"),

	XP_KEY_OPTION("keyEmergencyThrust",
		      "j",
		      KEY_EMERGENCY_THRUST,
		      "Pull emergency thrust handle.\n"),

	XP_KEY_OPTION("keyEmergencyShield",
		      "Caps_Lock",
		      KEY_EMERGENCY_SHIELD,
		      "Toggle emergency shield power.\n"),

	XP_KEY_OPTION("keyTractorBeam",
		      "comma",
		      KEY_TRACTOR_BEAM,
		      "Use tractor beam in attract mode.\n"),

	XP_KEY_OPTION("keyPressorBeam",
		      "period",
		      KEY_PRESSOR_BEAM,
		      "Use tractor beam in repulse mode.\n"),

	XP_KEY_OPTION("keyClearModifiers",
		      "k",
		      KEY_CLEAR_MODIFIERS,
		      "Clear current weapon modifiers.\n"),

	XP_KEY_OPTION("keyLoadModifiers1",
		      "", /* was "1" */
		      KEY_LOAD_MODIFIERS_1,
		      "Load the weapon modifiers from bank 1.\n"),

	XP_KEY_OPTION("keyLoadModifiers2",
		      "", /* was "2" */
		      KEY_LOAD_MODIFIERS_2,
		      "Load the weapon modifiers from bank 2.\n"),

	XP_KEY_OPTION("keyLoadModifiers3",
		      "", /* was "3" */
		      KEY_LOAD_MODIFIERS_3,
		      "Load the weapon modifiers from bank 3.\n"),

	XP_KEY_OPTION("keyLoadModifiers4",
		      "", /* was "4" */
		      KEY_LOAD_MODIFIERS_4,
		      "Load the weapon modifiers from bank 4.\n"),

	XP_KEY_OPTION("keyToggleOwnedItems",
		      "KP_8",
		      KEY_TOGGLE_OWNED_ITEMS,
		      "Toggle list of owned items on HUD.\n"),

	XP_KEY_OPTION("keyReprogram",
		      "quoteleft",
		      KEY_REPROGRAM,
		      "Reprogram modifier or lock bank.\n"),

	XP_KEY_OPTION("keyLoadLock1",
		      "", /* was "5" */
		      KEY_LOAD_LOCK_1,
		      "Load player lock from bank 1.\n"),

	XP_KEY_OPTION("keyLoadLock2",
		      "", /* was "6" */
		      KEY_LOAD_LOCK_2,
		      "Load player lock from bank 2.\n"),

	XP_KEY_OPTION("keyLoadLock3",
		      "", /* was "7" */
		      KEY_LOAD_LOCK_3,
		      "Load player lock from bank 3.\n"),

	XP_KEY_OPTION("keyLoadLock4",
		      "", /* was "8" */
		      KEY_LOAD_LOCK_4,
		      "Load player lock from bank 4.\n"),

	XP_KEY_OPTION("keyToggleRecord",
		      "F12",
		      KEY_TOGGLE_RECORD,
		      "Toggle recording of session (see recordFile).\n"),

#ifdef SOUND
	XP_KEY_OPTION("keyToggleSound",
		      "",
		      KEY_TOGGLE_SOUND,
		      "Toggle sound. Changes value of option 'sound'.\n"),
#endif
	XP_KEY_OPTION("keyToggleTalkMessages",
		      "",
		      KEY_TOGGLE_TALK_MESSAGES,
		      "Toggle showing of talk messages and important game messages.\n"),

	XP_KEY_OPTION("keyToggleGameMessages",
		      "",
		      KEY_TOGGLE_GAME_MESSAGES,
		      "Toggle showing of less important game messages.\n"),

	XP_KEY_OPTION("keyToggleScoreList",
		      "Tab",
		      KEY_TOGGLE_SCORE_LIST,
		      "Toggles the score list on and off.\n"),

	XP_KEY_OPTION("keyToggleExtHUDRadar",
		      "s",
		      KEY_TOGGLE_EXT_HUD_RADAR,
		      "Toggles the HUD radar walls and bases on and off.\n"),

	XP_KEY_OPTION("keyToggleExtHUD",
		      "",
		      KEY_TOGGLE_EXT_HUD,
		      "Toggles the extended HUD on and off.\n"),

	/* kps -disabled because this doesn't work in x11 client */
#if 0
	XP_KEY_OPTION("keyToggleFullScreen",
		      "F11",
		      KEY_TOGGLE_FULLSCREEN,
		      "Toggles between fullscreen mode and window mode.\n"),
#endif

	XP_KEY_OPTION("keySelectItem",
		      "KP_0 KP_Insert",
		      KEY_SELECT_ITEM,
		      "Select an item to lose.\n"),

	XP_KEY_OPTION("keyLoseItem",
		      "KP_Delete KP_Decimal",
		      KEY_LOSE_ITEM,
		      "Lose the selected item.\n"),

	XP_KEY_OPTION("keyPrintMessagesStdout",
		      "Print",
		      KEY_PRINT_MSGS_STDOUT,
		      "Print the current messages to stdout.\n"),

	XP_KEY_OPTION("keyTalkCursorLeft",
		      "Left",
		      KEY_TALK_CURSOR_LEFT,
		      "Move Cursor to the left in the talk window.\n"),

	XP_KEY_OPTION("keyTalkCursorRight",
		      "Right",
		      KEY_TALK_CURSOR_RIGHT,
		      "Move Cursor to the right in the talk window.\n"),

	XP_KEY_OPTION("keyTalkCursorUp",
		      "Up",
		      KEY_TALK_CURSOR_UP,
		      "Browsing in the history of the talk window.\n"),

	XP_KEY_OPTION("keyTalkCursorDown",
		      "Down",
		      KEY_TALK_CURSOR_DOWN,
		      "Browsing in the history of the talk window.\n"),

	XP_KEY_OPTION("keyExit",
		      "Escape",
		      KEY_EXIT,
		      "Generic exit key.\n"
		      "Used for example to exit mouse mode or quit the client.\n"),

	XP_KEY_OPTION("keyYes",
		      "F1",
		      KEY_YES,
		      "Positive reply key.\n" "Used to reply 'yes' to client questions.\n"),

	XP_KEY_OPTION("keyNo",
		      "F2",
		      KEY_NO,
		      "Negative reply key.\n" "Used to reply 'no' to client questions.\n"),

	XP_KEY_OPTION("keySendMsg1",
		      "space", /* BALL */
		      KEY_MSG_1,
		      "Sends the talkmessage stored in msg1.\n"),

	XP_KEY_OPTION("keySendMsg2",
		      "a", /* SAFE */
		      KEY_MSG_2,
		      "Sends the talkmessage stored in msg2.\n"),

	XP_KEY_OPTION("keySendMsg3",
		      "c", /* COVER */
		      KEY_MSG_3,
		      "Sends the talkmessage stored in msg3.\n"),

	XP_KEY_OPTION("keySendMsg4",
		      "v", /* POP */
		      KEY_MSG_4,
		      "Sends the talkmessage stored in msg4.\n"),

	XP_KEY_OPTION("keySendMsg5",
		      "F5",
		      KEY_MSG_5,
		      "Sends the talkmessage stored in msg5.\n"),

	XP_KEY_OPTION("keySendMsg6",
		      "g", /* #ln has BALL */
		      KEY_MSG_6,
		      "Sends the talkmessage stored in msg6.\n"),

	XP_KEY_OPTION("keySendMsg7",
		      "F7",
		      KEY_MSG_7,
		      "Sends the talkmessage stored in msg7.\n"),

	XP_KEY_OPTION("keySendMsg8",
		      "F8",
		      KEY_MSG_8,
		      "Sends the talkmessage stored in msg8.\n"),

	XP_KEY_OPTION("keySendMsg9",
		      "F9",
		      KEY_MSG_9,
		      "Sends the talkmessage stored in msg9.\n"),

	XP_KEY_OPTION("keySendMsg10",
		      "F10",
		      KEY_MSG_10,
		      "Sends the talkmessage stored in msg10.\n"),

	XP_KEY_OPTION("keySendMsg11",
		      "",	/* F11 is keyToggleFullScreen now */
		      KEY_MSG_11,
		      "Sends the talkmessage stored in msg11.\n"),

	XP_KEY_OPTION("keySendMsg12",
		      "",
		      KEY_MSG_12,
		      "Sends the talkmessage stored in msg12.\n"),

	XP_KEY_OPTION("keySendMsg13",
		      "",
		      KEY_MSG_13,
		      "Sends the talkmessage stored in msg13.\n"),

	XP_KEY_OPTION("keySendMsg14",
		      "",
		      KEY_MSG_14,
		      "Sends the talkmessage stored in msg14.\n"),

	XP_KEY_OPTION("keySendMsg15",
		      "",
		      KEY_MSG_15,
		      "Sends the talkmessage stored in msg15.\n"),

	XP_KEY_OPTION("keySendMsg16",
		      "",
		      KEY_MSG_16,
		      "Sends the talkmessage stored in msg16.\n"),

	XP_KEY_OPTION("keySendMsg17",
		      "",
		      KEY_MSG_17,
		      "Sends the talkmessage stored in msg17.\n"),

	XP_KEY_OPTION("keySendMsg18",
		      "",
		      KEY_MSG_18,
		      "Sends the talkmessage stored in msg18.\n"),

	XP_KEY_OPTION("keySendMsg19",
		      "",
		      KEY_MSG_19,
		      "Sends the talkmessage stored in msg19.\n"),

	XP_KEY_OPTION("keySendMsg20",
		      "",
		      KEY_MSG_20,
		      "Sends the talkmessage stored in msg20.\n"),
};

void Store_key_options(void)
{
	STORE_OPTIONS(key_options);
}
