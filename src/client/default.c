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

/* $Id: default.c,v 1.37 2007/12/11 21:44:56 kps Exp $ */

#include "sysdeps.h"

#include "global.h"
#include "option.h"
#include "proto.h"
#include "error.h"
#include "gfx2d.h"
#include "setup.h"
#include "clientrank.h"
#include "datagram.h"
#include "xpconfig.h"
#include "pack-ng.h"

static double hudScale;		/* Scale for HUD drawing */

static bool Set_nickName(xp_option_t * opt, const char *value)
{
	UNUSED_PARAM(opt);
	assert(value);

	/*
	 * This is a hack. User name will be used as nickname,
	 * look in Set_userName().
	 */
	if (strlen(value) == 0)
		return true;

	strlcpy(connectParam.nick_name, value, sizeof(connectParam.nick_name));

	/*
	 * Allow any nick name. Server will correct it according to its own rules.
	 */
#if 0
	CAP_LETTER(connectParam.nick_name[0]);
	if (connectParam.nick_name[0] < 'A' || connectParam.nick_name[0] > 'Z') {
		warn("Your player name \"%s\" should start with an uppercase letter.",
		     connectParam.nick_name);
		connectParam.nick_name[0] = 'X';
	}

	if (Check_nick_name(connectParam.nick_name) == NAME_ERROR) {
		char nick[MAX_NAME_LEN];

		strlcpy(nick, connectParam.nick_name, sizeof(nick));
		Fix_nick_name(connectParam.nick_name);
		warn("Fixing nick from \"%s\" to \"%s\".\n", nick, connectParam.nick_name);
	}
#endif

	/*warn("Nick name set to \"%s\".\n", connectParam.nick_name); */

	return true;
}

static const char *Get_nickName(xp_option_t * opt)
{
	UNUSED_PARAM(opt);
	return connectParam.nick_name;
}

/* Make sure we have a nick name. */
void Make_sure_we_have_a_nick(void)
{
	if (connectParam.nick_name[0] == '\0') {
		Get_login_name(connectParam.nick_name,
			       sizeof(connectParam.nick_name));
		if (connectParam.nick_name[0] == '\0') {
			strlcpy(connectParam.nick_name,
				PACKAGE_NAME,
				sizeof(connectParam.nick_name));
		}
	}
}

static bool Set_team(xp_option_t * opt, int value)
{
	UNUSED_PARAM(opt);
	if (value >= 0 && value < MAX_TEAMS)
		connectParam.team = value;
	else
		connectParam.team = TEAM_NOT_SET;

	return true;
}

static bool Set_texturePath(xp_option_t * opt, const char *value)
{
	UNUSED_PARAM(opt);
	XFREE(texturePath);
	texturePath = xp_safe_strdup(value);
	if (realTexturePath == NULL)
		realTexturePath = xp_safe_strdup(value);
	return true;
}
static const char *Get_texturePath(xp_option_t * opt)
{
	UNUSED_PARAM(opt);
	return texturePath;
}


/*
 * Ship shape option handling.
 */
static char *shipShapeSetting = NULL;
static char *shipShapeFile = NULL;

/*
 * This function trys to set the shipShape variable.
 *
 * First it looks if shipShapeSetting would be a suitable value.
 * If not, it assumes shipShapeSetting is the name of a shape
 * to be loaded from shipShapeFile.
 */
static void tryToSetShipShape(void)
{
	bool is_shape = false, valid;
	FILE *fp;
	char *ptr, *str, line[1024], *ss_candidate = NULL;

	/*
	 * Let's allow overriding shipshape with another. E.g the first
	 * one is from xpilotrc, the other from command line.
	 */
	XFREE(shipShape);

	/* If there is no shipShapeSetting, there is nothing we can do. */
	if (shipShapeSetting == NULL || strlen(shipShapeSetting) == 0)
		return;

	/*
	 * Let's determine if shipShapeSetting is the actual ship shape or
	 * its name.
	 */
	if (strchr(shipShapeSetting, '('))
		is_shape = true;

	if (is_shape) {
		valid = Validate_shape_str(shipShapeSetting);
		if (valid) {
			shipShape = xp_safe_strdup(shipShapeSetting);
			xpinfo("Your shipShape is valid. Have a nice day.\n");
		}
		else
			warn("Your shipShape isn't valid. Please fix it.");
		return;
	}

	/*
	 * shipShapeSetting is the name of the shipshape. Without a
	 * shipShapeFile we can't proceed.
	 */
	if (!shipShapeFile || strlen(shipShapeFile) == 0)
		return;

	fp = fopen(shipShapeFile, "r");
	if (!fp) {
		error("Can't open shipShapeFile \"%s\".", shipShapeFile);
		return;
	}

	while (fgets(line, sizeof line, fp)) {
		if ((str = strstr(line, "(name:")) != NULL || (str = strstr(line, "(NM:")) != NULL) {

			str = strchr(str, ':');

			while (*++str == ' ');

			if ((ptr = strchr(str, ')')) != NULL)
				*ptr = '\0';
			if (!strcmp(str, shipShapeSetting)) {
				/* Gotcha */
				if (ptr != NULL)
					*ptr = ')';
				ss_candidate = xp_safe_strdup(line);
				break;
			}
		}
	}
	fclose(fp);

	if (!ss_candidate) {
		warn("Could not find the ship \"%s\" in shipShapeFile %s.", shipShapeSetting,
		     shipShapeFile);
		return;
	}

	valid = Validate_shape_str(ss_candidate);
	if (valid) {
		xpinfo("Ship shape \"%s\" is now in use.\n", shipShapeSetting);
		shipShape = ss_candidate;
	}
	else {
		XFREE(ss_candidate);
		warn("Your shipShape \"%s\" isn't valid. Please fix it.", shipShapeSetting);
	}
}

/*
 * Shipshape options.
 */
static bool Set_shipShape(xp_option_t * opt, const char *value)
{
	UNUSED_PARAM(opt);
	XFREE(shipShapeSetting);
	shipShapeSetting = xp_safe_strdup(value);
	tryToSetShipShape();

	return true;
}

static const char *Get_shipShape(xp_option_t * opt)
{
	UNUSED_PARAM(opt);
	return shipShapeSetting;
}

static bool Set_shipShapeFile(xp_option_t * opt, const char *value)
{
	UNUSED_PARAM(opt);
	XFREE(shipShapeFile);
	shipShapeFile = xp_safe_strdup(value);
	tryToSetShipShape();

	return true;
}

static const char *Get_shipShapeFile(xp_option_t * opt)
{
	UNUSED_PARAM(opt);
	return shipShapeFile;
}

static bool Set_power(xp_option_t * opt, double val)
{
	UNUSED_PARAM(opt);
	Send_power(val);
	power = val;
	controlTime = CONTROL_TIME;
	return true;
}
static bool Set_turnSpeed(xp_option_t * opt, double val)
{
	UNUSED_PARAM(opt);
	Send_turnspeed(val);
	turnspeed = val;
	controlTime = CONTROL_TIME;
	return true;
}
static bool Set_turnResistance(xp_option_t * opt, double val)
{
	UNUSED_PARAM(opt);
	Send_turnresistance(val);
	turnresistance = val;
	return true;
}

static bool Set_altPower(xp_option_t * opt, double val)
{
	UNUSED_PARAM(opt);
	Send_power_s(val);
	power_s = val;
	controlTime = CONTROL_TIME;
	return true;
}
static bool Set_altTurnSpeed(xp_option_t * opt, double val)
{
	UNUSED_PARAM(opt);
	Send_turnspeed_s(val);
	turnspeed_s = val;
	controlTime = CONTROL_TIME;
	return true;
}
static bool Set_altTurnResistance(xp_option_t * opt, double val)
{
	UNUSED_PARAM(opt);
	Send_turnresistance_s(val);
	turnresistance_s = val;
	return true;
}

static bool Set_autoShield(xp_option_t * opt, bool val)
{
	UNUSED_PARAM(opt);
	Set_auto_shield(val);
	return true;
}

static bool Set_toggleShield(xp_option_t * opt, bool val)
{
	UNUSED_PARAM(opt);
	Set_toggle_shield(val);
	return true;
}

static bool Set_maxFPS(xp_option_t * opt, int val)
{
	UNUSED_PARAM(opt);
	maxFPS = val;
	Check_client_fps();
	return true;
}

static bool Set_maxMouseTurnsPS(xp_option_t * opt, int val)
{
	UNUSED_PARAM(opt);
	maxMouseTurnsPS = val;
	if (maxMouseTurnsPS > 0) {
		mouseMovementInterval = 1000000 / maxMouseTurnsPS;
		if (mouseMovementInterval * maxMouseTurnsPS < 1000000)
			mouseMovementInterval++;
	}
	/*warn("mouseMovementInterval = %d", mouseMovementInterval); */
	return true;
}

static bool Set_sparkProb(xp_option_t * opt, double val)
{
	UNUSED_PARAM(opt);
	sparkProb = val;
	spark_rand = (int) (sparkProb * MAX_SPARK_RAND + 0.5);
	Check_view_dimensions();
	return true;
}

static bool Set_hudScale(xp_option_t * opt, double value)
{
	UNUSED_PARAM(opt);
	hudScale = value;
	hudSize = (int) (MIN_HUD_SIZE * hudScale);
	return true;
}

static bool Set_backgroundPointDist(xp_option_t * opt, int val)
{
	UNUSED_PARAM(opt);
	backgroundPointDist = val;
	if (oldServer)
		Map_dots();
	return true;
}

static bool Set_backgroundPointSize(xp_option_t * opt, int val)
{
	UNUSED_PARAM(opt);
	backgroundPointSize = val;
	if (oldServer)
		Map_dots();
	return true;
}

#if 0
static bool Set_slidingRadar(xp_option_t * opt, bool val)
{
	UNUSED_PARAM(opt);
	slidingRadar = val;
	Paint_sliding_radar();
	return true;
}
#endif

static bool Set_wallDrawMode(xp_option_t *opt, int val)
{
	UNUSED_PARAM(opt);
	wallDrawMode = val;
	if (Setup) {
		if (oldServer) {
			Map_blue(0, 0, Setup->x, Setup->y);
		}
		else {
			if (wallDrawMode == DRAW_TEXTURED) {
				Mapdata_setup(Setup->data_url);
			}
		}
	}
	return true;	
}

static bool Set_decorDrawMode(xp_option_t *opt, int val)
{
	UNUSED_PARAM(opt);
	decorDrawMode = val;
	if (!Setup)
		return true;
	if (oldServer)
		Map_dots();
	Paint_world_radar();
	return true;	
}

static bool Set_dirPrediction(xp_option_t * opt, bool val)
{
	UNUSED_PARAM(opt);
	if (val) {
		if (!dirPrediction) {
			/* reset pointer movements */
			int m;
			for (m = 0; m < MAX_POINTER_MOVES; m++)
				pointer_moves[m].id = -1;
			pointer_move_next = 0;
			last_keyboard_ack = 0;
			dirPrediction = true;
		}
	}
	else
		dirPrediction = false;

	return true;
}

/*int protocolVersion = POLYGON_VERSION;*/
static char protocolVersionStr[32];

static bool Set_protocolVersion(xp_option_t * opt, const char *value)
{
	if (sscanf(value, "%x", &protocolVersion) <= 0)
		return false;
	return true;
}

static const char *Get_protocolVersion(xp_option_t * opt)
{
	snprintf(protocolVersionStr, sizeof protocolVersionStr, "%04x", protocolVersion);
	return protocolVersionStr;
}

void defaultCleanup(void)
{
	XFREE(keydefs);
	XFREE(texturePath);
	XFREE(shipShape);

#ifdef SOUND
	audioCleanup();
#endif				/* SOUND */
}

xp_option_t default_options[] = {
#if 0
	XP_BOOL_OPTION("newbie",
		       true,
		       &newbie,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Enable newbie help.\n"),
#endif

	XP_NOARG_OPTION("help",
			&xpArgs.help,
			XP_OPTFLAG_NEVER_SAVE,
			"Display this help message.\n"),

	XP_NOARG_OPTION("version",
			&xpArgs.version,
			XP_OPTFLAG_NEVER_SAVE,
			"Show the source code version.\n"),

	XP_NOARG_OPTION("join",
			&xpArgs.auto_connect,
			XP_OPTFLAG_NEVER_SAVE,
			"Join the game immediately, no questions asked.\n"),

	XP_NOARG_OPTION("text",
			&xpArgs.text,
			XP_OPTFLAG_NEVER_SAVE,
			"Use the simple text interface to contact a server\n"
			"instead of the graphical user interface.\n"),

	XP_NOARG_OPTION("list",
			&xpArgs.list_servers,
			XP_OPTFLAG_NEVER_SAVE,
			"List all servers running on the local network.\n"),

	XP_STRING_OPTION("shutdown",
			 "",
			 xpArgs.shutdown_reason,
			 sizeof xpArgs.shutdown_reason,
			 NULL, NULL, NULL,
			 XP_OPTFLAG_NEVER_SAVE,
			 "Shutdown the server with a message.\n"
			 "The message used is the first argument to this option.\n"),

	XP_STRING_OPTION("name",
			 "",
			 NULL, 0,
			 Set_nickName, NULL, Get_nickName,
			 XP_OPTFLAG_KEEP,
			 "Set your nickname.\n"),

	XP_STRING_OPTION("user",
			 "user",
			 connectParam.user_name,
			 sizeof(connectParam.user_name),
			 NULL, NULL, NULL,
			 XP_OPTFLAG_KEEP,
			 "Deprecated: optionally set your username.\n"),

	XP_STRING_OPTION("host",
			 "host",
			 connectParam.host_name,
			 sizeof(connectParam.host_name),
			 NULL, NULL, NULL,
			 XP_OPTFLAG_KEEP,
			 "Deprecated: optionally set your hostname.\n"),

	XP_INT_OPTION("team",
		      TEAM_NOT_SET,
		      0,
		      TEAM_NOT_SET,
		      &connectParam.team,
		      Set_team,
		      XP_OPTFLAG_KEEP,
		      "Set the team to join.\n"),

	XP_INT_OPTION("port",
		      SERVER_PORT,
		      0,
		      65535,
		      &connectParam.contact_port,
		      NULL,
		      XP_OPTFLAG_KEEP,
		      "Set the port number of the server.\n"
		      "Almost all servers use the default port, which is the recommended\n"
		      "policy.  You can find out about which port is used by a server by\n"
		      "querying the XPilot Meta server.\n"),

	XP_INT_OPTION("clientPortStart",
		      0,
		      0,
		      65535,
		      &clientPortStart,
		      NULL,
		      XP_OPTFLAG_KEEP,
		      "Use UDP ports clientPortStart - clientPortEnd (for firewalls).\n"
		      /* TODO: describe what value 0 means */ ),

	XP_INT_OPTION("clientPortEnd",
		      0,
		      0,
		      65535,
		      &clientPortEnd,
		      NULL,
		      XP_OPTFLAG_KEEP,
		      "Use UDP ports clientPortStart - clientPortEnd (for firewalls).\n"),

	XP_DOUBLE_OPTION("power",
			 55.0,
			 MIN_PLAYER_POWER,
			 MAX_PLAYER_POWER,
			 &power,
			 Set_power,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Set the engine power.\n" "Valid values are in the range 5-55.\n"),

	XP_DOUBLE_OPTION("turnSpeed",
			 10.0,
			 MIN_PLAYER_TURNSPEED,
			 MAX_PLAYER_TURNSPEED,
			 &turnspeed,
			 Set_turnSpeed,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Set the ship's turn speed.\n" "Valid values are in the range 4-64.\n"),

	XP_DOUBLE_OPTION("turnResistance",
			 0.0,
			 MIN_PLAYER_TURNRESISTANCE,
			 MAX_PLAYER_TURNRESISTANCE,
			 &turnresistance,
			 Set_turnResistance,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Set the ship's turn resistance.\n"
			 "This determines the speed at which a ship stops turning.\n"
			 "Valid values are in the range 0.0-1.0.\n"
			 "This should always be 0, other values are for compatibility.\n"
			 "See also turnSpeed.\n"),

	XP_DOUBLE_OPTION("scaleFactor",
			 1.0,
			 MIN_SCALEFACTOR,
			 MAX_SCALEFACTOR,
			 &scaleFactor,
			 Set_scaleFactor,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Specifies scaling factor for the drawing window.\n"),

	XP_DOUBLE_OPTION("altScaleFactor",
			 2.0,
			 MIN_SCALEFACTOR,
			 MAX_SCALEFACTOR,
			 &altScaleFactor,
			 Set_altScaleFactor,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Specifies alternative scaling factor for the drawing window.\n"),

	XP_INT_OPTION("maxFPS",
		      140,	/* 1.4.0 */
		      1,
		      MAX_SUPPORTED_FPS,
		      &maxFPS,
		      Set_maxFPS,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Set maximum FPS supported by the client. The server will try to\n"
		      "send at most this many frames per second to the client.\n"),

	XP_INT_OPTION("maxMouseTurnsPS",
		      0,
		      0,
		      MAX_SUPPORTED_FPS * 2,
		      &maxMouseTurnsPS,
		      Set_maxMouseTurnsPS,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Set maximum number of mouse turns sent per second\n"
		      "Set to 0 to disable this feature (its mostly useful on modem)\n"),

	XP_INT_OPTION("sparkSize",
		      1,
		      1,
		      32,
		      &sparkSize,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Size of sparks in pixels.\n"),

	XP_DOUBLE_OPTION("sparkProb",
			 0.1,
			 0.0,
			 1.0,
			 &sparkProb,
			 Set_sparkProb,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "The probablilty that sparks are drawn.\n"
			 "This gives a sparkling effect.\n"
			 "Valid values are in the range [0.0-1.0]\n"),

	XP_DOUBLE_OPTION("hudRadarScale",
			 2.0,
			 0.1,
			 20.0,
			 &hudRadarScale,
			 NULL,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "The relative size of the radar.\n"),

	XP_DOUBLE_OPTION("hudRadarDotScale",
			 3.0,
			 0.1,
			 20.0,
			 &hudRadarDotScale,
			 NULL,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Factor for scaling the radar dots.\n"),

#if 0
	XP_DOUBLE_OPTION("hudRadarLimit",
			 0.05,
			 0.05,
			 5.0,
			 &hudRadarLimit,
			 NULL,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Radar dots closer than this to your ship are not drawn.\n"
			 "A value of 1.0 means that the dots are not drawn for ships in\n"
			 "your active view area.\n"),
#endif

	XP_DOUBLE_OPTION("hudScale",
			 3.0,
			 1.0,
			 10.0,
			 &hudScale,
			 Set_hudScale,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "How much larger than minimum size to draw the hud.\n"),

	XP_INT_OPTION("maxCharsInNames",
		      32,
		      0,
		      MAX_NAME_LEN - 1,
		      &maxCharsInNames,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Maximum number of characters to paint in names on game area.\n"),

#if 0
	XP_BOOL_OPTION("slidingRadar",
		       true,
		       &slidingRadar,
		       Set_slidingRadar,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "If the game is in edgewrap mode then the radar will keep your\n"
		       "position on the radar in the center and raw the rest of the radar\n"
		       "around it.  Note that this requires a fast graphics system.\n"),
#endif

	XP_BOOL_OPTION("dirPrediction",
		       true,
		       &dirPrediction,
		       Set_dirPrediction,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Client paints the wanted direction of your ship instead of what\n"
		       "the server sent you. Possible differences are corrected in\n"
		       "roundtrip time.\n"),

	XP_BOOL_OPTION("showShipShapes",
		       true,
		       &showShipShapes,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Should others' shipshapes be displayed.\n"),

	XP_BOOL_OPTION("showMyShipShape",
		       true,
		       &showMyShipShape,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Should your own shipshape be displayed.\n"),

	XP_BOOL_OPTION("showHitArea",
		       false,
		       &showHitArea,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Should the hit area be displayed on ships.\n"),

	XP_BOOL_OPTION("showLivesByShip",
		       false,
		       &showLivesByShip,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Paint remaining lives next to ships.\n"),

	XP_BOOL_OPTION("showItems",
		       true,
		       &showItems,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Should owned items be displayed permanently on the HUD?\n"),

	XP_DOUBLE_OPTION("showItemsTime",
			 5.0,
			 0.0,
			 300.0,
			 &showItemsTime,
			 NULL,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "If showItems is false, the time in seconds to display item\n"
			 "information on the HUD when it has changed.\n"),

	XP_INT_OPTION("wallDrawMode",
		      3,
		      1,
		      4,
		      &wallDrawMode,
		      Set_wallDrawMode,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The way to draw the walls on the map.\n"
		      "1: Grid mode. Each wall block is painted.\n"
		      "2: Outline mode. Draws only wall outlines. Fast.\n"
		      "3: Filled mode. Fills walls in a single color.\n"
		      "4: Textured walls. Fills with textures if available.\n"
		      ),

	XP_INT_OPTION("decorDrawMode",
		      1,
		      0,
		      4,
		      &decorDrawMode,
		      Set_decorDrawMode,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The way to draw decorations on the map.\n"
		      "0: Don't draw decorations.\n"
		      "1: Grid mode. Each decor block is painted.\n"
		      "2: Outline mode. Draws only decoration outlines. Fast.\n"
		      "3: Filled mode. Fills decorations in a single color.\n"
		      "4: Textured decor. Fills with textures if available.\n"
		      ),

	XP_INT_OPTION(
		"baseWarningType",
		7,
		0,
		7,
		&baseWarningType,
		NULL,
		XP_OPTFLAG_CONFIG_DEFAULT,
		"Which type of base warning you prefer.\n"
		"0: disable base warning.\n"
		"1: draw a growing box on the base when a ship is appearing.\n"
		"2: flash base name when a ship is appearing.\n"
		"3: combine the effects of values 1 and 2.\n"
		"4: draw a ship on the base.\n"
		"5: draw a growing box and a ship.\n"
		"6: flash the base name and draw a ship.\n"
		"7: combine the effects of values 1, 2 and 4.\n"
		),

	XP_STRING_OPTION("protocolVersion",
			 "",
			 NULL, 0,
			 Set_protocolVersion, NULL, Get_protocolVersion,
			 XP_OPTFLAG_KEEP,
			 "Which protocol version to prefer when joining servers.\n"),

	XP_BOOL_OPTION("clientRanker",
		       false,
		       &clientRanker,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Scan messages and make personal kill/death ranking.\n"),

	XP_INT_OPTION("shotSize",
		      10,
		      1,
		      32,
		      &shotSize,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The size of shots in pixels.\n"),

	XP_INT_OPTION("teamShotSize",
		      4,
		      1,
		      32,
		      &teamShotSize,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The size of team shots in pixels.\n"
		      "Note that team shots are drawn in teamShotColor.\n"),

	XP_INT_OPTION("backgroundPointDist",
		      8,
		      0,
		      10,
		      &backgroundPointDist,
		      Set_backgroundPointDist,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The distance between points in the background measured in blocks.\n"
		      "These are drawn in empty map regions to keep feeling for which\n"
		      "direction the ship is moving to.\n"),

	XP_INT_OPTION("backgroundPointSize",
		      0,
		      0,
		      20,
		      &backgroundPointSize,
		      Set_backgroundPointSize,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Specifies the size of the background points.  0 means no points.\n"),

	XP_DOUBLE_OPTION("scoreObjectTime",
			 4.0,
			 0.0,
			 10.0,
			 &scoreObjectTime,
			 NULL,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "How many seconds score objects remain visible on the map.\n"),

	XP_BOOL_OPTION("toggleShield",
		       false,
		       &toggle_shield,
		       Set_toggleShield,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Are shields toggled by a keypress only?\n"),

	XP_BOOL_OPTION("autoShield",	/* Don auto-shield hack */
		       true,
		       &auto_shield,
		       Set_autoShield,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Are shields lowered automatically for weapon fire?\n"),

	XP_INT_OPTION("fuelNotify",
		      500,
		      0,
		      1000,
		      &fuelNotify,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The limit when the HUD fuel bar will become visible.\n"),

	XP_INT_OPTION("fuelWarning",
		      200,
		      0,
		      1000,
		      &fuelWarning,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The limit when the HUD fuel bar will start flashing.\n"),

	XP_INT_OPTION("fuelCritical",
		      100,
		      0,
		      1000,
		      &fuelCritical,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The limit when the HUD fuel bar will flash faster.\n"),

	XP_DOUBLE_OPTION("altPower",
			 55.0,
			 MIN_PLAYER_POWER,
			 MAX_PLAYER_POWER,
			 &power_s,
			 Set_altPower,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Set the ship's alternate engine power.\n"
			 "See also the keySwapSettings option.\n"),

	XP_DOUBLE_OPTION("altTurnSpeed",
			 16.0,
			 MIN_PLAYER_TURNSPEED,
			 MAX_PLAYER_TURNSPEED,
			 &turnspeed_s,
			 Set_altTurnSpeed,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Set the ship's alternate turn speed.\n"
			 "See also the keySwapSettings option.\n"),

	XP_DOUBLE_OPTION("altTurnResistance",
			 0.0,
			 MIN_PLAYER_TURNRESISTANCE,
			 MAX_PLAYER_TURNRESISTANCE,
			 &turnresistance_s,
			 Set_altTurnResistance,
			 XP_OPTFLAG_CONFIG_DEFAULT,
			 "Set the ship's alternate turn resistance.\n"
			 "See also the keySwapSettings option.\n"),

	XP_STRING_OPTION("modifierBank1",
			 "",
			 modBankStr[0], sizeof modBankStr[0],
			 NULL, NULL, NULL,	/* kps - add set and get functions */
			 XP_OPTFLAG_DEFAULT,
			 "The default weapon modifier values for the first modifier bank.\n"),

	XP_STRING_OPTION("modifierBank2",
			 "",
			 modBankStr[1], sizeof modBankStr[1],
			 NULL, NULL, NULL,
			 XP_OPTFLAG_DEFAULT,
			 "The default weapon modifier values for the second modifier bank.\n"),

	XP_STRING_OPTION("modifierBank3",
			 "",
			 modBankStr[2], sizeof modBankStr[2],
			 NULL, NULL, NULL,
			 XP_OPTFLAG_DEFAULT,
			 "The default weapon modifier values for the third modifier bank.\n"),

	XP_STRING_OPTION("modifierBank4",
			 "",
			 modBankStr[3], sizeof modBankStr[3],
			 NULL, NULL, NULL,
			 XP_OPTFLAG_DEFAULT,
			 "The default weapon modifier values for the fourth modifier bank.\n"),

	XP_STRING_OPTION("shipShape",
			 "",
			 NULL, 0,
			 Set_shipShape, NULL, Get_shipShape,
			 XP_OPTFLAG_DEFAULT,
			 "Define the ship shape to use.  Because the argument to this option\n"
			 "is rather large (up to 500 bytes) the recommended way to set\n"
			 "this option is in the .xpilotrc file in your home directory.\n"
			 "The exact format is defined in the file doc/README.SHIPS in the\n"
			 "XPilot distribution. Note that there is a nifty Unix tool called\n"
			 "editss for easy ship creation. There is XPShipEditor for Windows\n"
			 "and Ship Shaper for Java.  See the XPilot FAQ for details.\n"
			 "See also the \"shipShapeFile\" option below.\n"),

	XP_STRING_OPTION("shipShapeFile",
			 CONF_SHIP_FILE,
			 NULL, 0,
			 Set_shipShapeFile, NULL, Get_shipShapeFile,
			 XP_OPTFLAG_DEFAULT,
			 "An optional file where shipshapes can be stored.\n"
			 "If this resource is defined and it refers to an existing file\n"
			 "then shipshapes can be referenced to by their name.\n"
			 "For instance if you define shipShapeFile to be\n"
			 "/home/myself/.shipshapes and this file contains one or more\n"
			 "shipshapes then you can select the shipshape by starting xpilot as:\n"
			 "	xpilot -shipShape myshipshapename\n"
			 "Where \"myshipshapename\" should be the \"name:\" or \"NM:\" of\n"
			 "one of the shipshapes defined in /home/myself/.shipshapes.\n"
			 "Each shipshape definition should be defined on only one line,\n"
			 "where all characters up to the first left parenthesis don't matter.\n"
			 /* shipshopshapshepshit getting nuts from all these shpshp-s. */ ),

	XP_STRING_OPTION("texturePath",
			 CONF_TEXTUREDIR,
			 NULL, 0,
			 Set_texturePath, NULL, Get_texturePath,
			 XP_OPTFLAG_KEEP,
			 "Search path for texture files.\n"
			 "This is a list of one or more directories separated by colons.\n"),

	XP_STRING_OPTION("clientRankFile",
			 "",
			 clientRankFile,
			 sizeof clientRankFile,
			 NULL, NULL, NULL,
			 XP_OPTFLAG_DEFAULT,
			 "An optional file where clientside kill/death rank is stored.\n"),

	XP_STRING_OPTION("clientRankHTMLFile",
			 "",
			 clientRankHTMLFile,
			 sizeof clientRankHTMLFile,
			 NULL, NULL, NULL,
			 XP_OPTFLAG_DEFAULT,
			 "An optional file where clientside kill/death rank is\n"
			 "published in HTML format.\n"),

	XP_STRING_OPTION("clientRankHTMLNOJSFile",
			 "",
			 clientRankHTMLNOJSFile,
			 sizeof clientRankHTMLNOJSFile,
			 NULL, NULL, NULL,
			 XP_OPTFLAG_DEFAULT,
			 "An optional file where clientside kill/death rank is\n"
			 "published in HTML format, w/o JavaScript.\n"),

#ifdef SOUND
	XP_STRING_OPTION("soundFile",
			 CONF_SOUNDFILE,
			 soundFile, sizeof soundFile,
			 NULL, NULL, NULL,
			 XP_OPTFLAG_DEFAULT,
			 "Specifies the sound file.\n"),

	XP_INT_OPTION("maxVolume",
		      100,
		      0,
		      100,
		      &maxVolume,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Specifies the volume to play sounds with (0-100%%).\n"),

	XP_BOOL_OPTION("sound",
		       true,
		       &sound,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Is sound enabled? (set to false to mute client).\n"),
#endif

};

void Store_default_options(void)
{
	STORE_OPTIONS(default_options);
}
