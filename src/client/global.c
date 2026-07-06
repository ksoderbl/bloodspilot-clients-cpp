/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2001 by
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

#include "sysdeps.h"
#include "global.h"

int RadarWidth = 256;
int RadarHeight = 0;

bool UpdateRadar = false; /* radar update because of polystyle changes? */

xp_args_t xpArgs;
Connect_param_t connectParam;

bool talking = false;        /* Some talk window is open? */
bool teamTalk = false;       /* Should the next talk message be sent to team only? */
bool pointerControl = false; /* Pointer (mouse) control is on? */
bool restorePointerControl = false;
/* Pointer control should be restored later? */
bool quitMode = false;      /* Client is in quit mode? */
bool keyConfigMode = false; /* Client is in key config mode? */
double scaleFactor;
double scale;
float fscale;
double altScaleFactor;

bool newbie = false;
char *geometry;

int maxLinesInHistory = MAX_HIST_MSGS; /* lines to save in talk window history */
char *HistoryMsg[MAX_HIST_MSGS];       /* talk window history */

char *talk_fast_msgs[TALK_FAST_NR_OF_MSGS]; /* talk macros */

score_object_t score_objects[MAX_SCORE_OBJECTS];
int score_object;

char *shipShape;         /* Shape of player's ship */
double power;            /* Force of thrust */
double power_s;          /* Saved power fiks */
double turnspeed;        /* How fast player acc-turns */
double turnspeed_s;      /* Saved turnspeed */
double turnresistance;   /* How much is lost in % */
double turnresistance_s; /* Saved (see above) */
double sparkProb;        /* Sparkling effect configurable */

int packet_loss;      /* lost packets per second */
int packet_drop;      /* dropped packets per second */
int packet_lag;       /* approximate lag in frames */
char *packet_measure; /* packet measurement in a second */
long packet_loop;     /* start of measurement */

int oldServer; /* Compatibility mode for old block-based servers */

int numItemsTime[NUM_ITEMS];
double showItemsTime;
double scoreObjectTime; /* How long to show score objects */

bool UpdateRadar;
int backgroundPointDist; /* spacing of navigation points */
int backgroundPointSize; /* size of navigation points */
int sparkSize;           /* size of sparks and debris */
int shotSize;            /* size of shot */
int teamShotSize;        /* size of team shot */
bool showNastyShots;     /* show original flavor shots or the new "nasty shots" */
bool showRadar;
bool showExtHUD; /* Extended HUD */

bool clientRanker;

int wallDrawMode;
int decorDrawMode;
int baseWarningType;

#if 0
bool filledDecor;
bool filledWorld;
bool outlineDecor;
bool outlineWorld;
bool showDecor;
bool texturedDecor;
bool texturedWalls;
#endif
bool showItems;
bool showLivesByShip;
bool showMyShipShape;
bool showNastyShots;
bool showShipShapes;
bool showHitArea;
#if 0
bool slidingRadar;
#endif

bool showTalkMessages = true;
bool showGameMessages = true;
int showScoreListMode = 1; /* 0 = don't show, 1 = short nick, 2 = long nick + user@host */
bool showExtHUDRadar = true;
bool showExtHUD = true;

double controlTime;     /* Display control for how long? */
uint8_t spark_rand;     /* Sparkling effect */
uint8_t old_spark_rand; /* previous value of spark_rand */

short numTanks;   /* Number of tanks */
int fuelCritical; /* Fuel critical level */
int fuelWarning;  /* Fuel warning level */
int fuelNotify;   /* Fuel notify level */

char nickname[MAX_CHARS];   /* Nick-name of player */
char servername[MAX_CHARS]; /* Name of server connecting to */
unsigned version;           /* Version of the server */
bool toggle_shield;         /* Are shields toggled by a press? */
bool shields;               /* When shields are considered up */
bool auto_shield;           /* drops shield for fire */

int maxFPS; /* Max FPS player wants from server */
int oldMaxFPS = 0;
int recordFPS = 0; /* What FPS to record at */
int clientLag;     /* Time to draw a frame, unit us */
int rounds_played;
bool roundend;
int numClientLagSamples; /* Make xpplot.m file with lag */

bool initialPointerControl = false; /* Start by using mouse for control? */
bool pointerControl;                /* Mouse control enabled? */
int maxMouseTurnsPS = 0;
int mouseMovementInterval = 0;
int cumulativeMouseMovement = 0;

char modBankStr[NUM_MODBANKS][MAX_CHARS]; /* modifier banks strings */

int maxCharsInNames;
uint8_t lose_item;    /* flag and index to drop item */
int lose_item_active; /* one of the lose keys is pressed */

/* store message in history, when it is sent? */
char *talk_fast_msgs[];

bool config_mapped = false; /* Is config window visible */
bool players_exposed;

int num_playing_teams;
long time_left = -1;
int protocolVersion = POLYGON_VERSION;

short ext_view_width;  /* Width of extended visible area */
short ext_view_height; /* Height of extended visible area */
double scaleFactor;
double scaleFactor_s;

double hudRadarScale;    /* Scale for radar drawing */
double hudRadarDotScale; /* Scale for radar dot drawing */
double hudRadarLimit;    /* Limit for radar drawing */

long loops = 0;
int top_width; /* top: main game window */
int top_height;
int draw_width; /* draw: map view window */
int draw_height;
int num_spark_colors;
uint8_t debris_colors;  /* Number of debris intensities from server */
int active_view_width;  /* W. of active map area displayed. */
int active_view_height; /* H. of active map area displayed. */
int ext_view_x_offset;  /* Offset of ext_view_width */
int ext_view_y_offset;  /* Offset of ext_view_height */

int hudSize; /* Size for HUD drawing */

fuelstation_t *fuels = NULL;
int num_fuels = 0;
homebase_t *bases = NULL;
int num_bases = 0;
cannontime_t *cannons = NULL;
int num_cannons = 0;
target_t *targets = NULL;
int num_targets = 0;
int num_treasures = 0;

checkpoint_t old_checks[MAX_CHECKS];
checkpoint_t *checks = NULL;
int num_checks = 0;
xp_polygon_t *polygons = NULL;
int num_polygons = 0, max_polygons = 0;
edge_style_t *edge_styles = NULL;
int num_edge_styles = 0, max_edge_styles = 0;
polygon_style_t *polygon_styles = NULL;
int num_polygon_styles = 0, max_polygon_styles = 0;

score_object_t score_objects[MAX_SCORE_OBJECTS];
int score_object = 0;

int killratio_kills;
int killratio_deaths;
int killratio_totalkills;
int killratio_totaldeaths;

int ballsCashed;
int ballsReplaced;
int ballsCashedByTeam;
int ballsLost;

#ifdef SOUND
char sounds[MAX_CHARS];      /* audio mappings */
char audioServer[MAX_CHARS]; /* audio server */
int maxVolume;               /* maximum volume (in percent) */
#endif                       /* SOUND */
