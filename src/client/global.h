/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
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

/* $Id: global.h,v 1.35 2007/11/23 10:47:09 kps Exp $ */

#ifndef	GLOBAL_H
#define	GLOBAL_H

#ifndef TYPES_H
#include "types.h"
#endif
#ifndef ITEM_H
#include "item.h"
#endif
#ifndef OTHER_H
#include "other.h"
#endif

extern int RadarWidth;
extern int RadarHeight;

extern bool UpdateRadar;	/* radar update because of polystyle changes? */

extern unsigned char *map_edge_descriptor_array;

extern xp_args_t xpArgs;
extern Connect_param_t connectParam;

extern bool talking;		/* Some talk window is open? */
extern bool teamTalk;		/* Should the next talk message be sent to team only? */
extern bool pointerControl;	/* Pointer (mouse) control is on? */
extern bool restorePointerControl;
  /* Pointer control should be restored later? */
extern bool quitMode;		/* Client is in quit mode? */
extern bool keyConfigMode;	/* Client is in key config mode? */
extern double scaleFactor;
extern double scale;
extern float fscale;
extern double altScaleFactor;

extern bool newbie;
extern char *geometry;

extern int talkMessagesLifeTime;
extern int gameMessagesLifeTime;
extern int messagesToStdout;
extern int maxMessages;
extern int charsPerSecond;	/* Message output speed (config) */
extern bool ignoreColorCodes;

extern int maxLinesInHistory;	/* lines to save in history */
extern selection_t selection;	/* selections in draw and talk window */
extern char *HistoryMsg[MAX_HIST_MSGS];	/* talk window history */

extern char *talk_fast_msgs[TALK_FAST_NR_OF_MSGS];	/* talk macros */

extern score_object_t score_objects[MAX_SCORE_OBJECTS];
extern int score_object;

extern char *shipShape;		/* Shape of player's ship */
extern double power;		/* Force of thrust */
extern double power_s;		/* Saved power fiks */
extern double turnspeed;	/* How fast player acc-turns */
extern double turnspeed_s;	/* Saved turnspeed */
extern double turnresistance;	/* How much is lost in % */
extern double turnresistance_s;	/* Saved (see above) */
extern double displayedPower;	/* What the server is sending us */
extern double displayedTurnspeed;	/* What the server is sending us */
extern double displayedTurnresistance;	/* What the server is sending us */
extern double sparkProb;	/* Sparkling effect configurable */

extern int packet_loss;		/* lost packets per second */
extern int packet_drop;		/* dropped packets per second */
extern int packet_lag;		/* approximate lag in frames */
extern char *packet_measure;	/* packet measurement in a second */
extern long packet_loop;	/* start of measurement */

extern int oldServer;		/* Compatibility mode for old block-based servers */

extern int numItemsTime[NUM_ITEMS];
extern double showItemsTime;
extern double scoreObjectTime;	/* How long to show score objects */

extern bool UpdateRadar;
extern int backgroundPointDist;	/* spacing of navigation points */
extern int backgroundPointSize;	/* size of navigation points */
extern int sparkSize;		/* size of sparks and debris */
extern int shotSize;		/* size of shot */
extern int teamShotSize;	/* size of team shot */
extern bool showNastyShots;	/* show original flavor shots or the new "nasty shots" */
extern bool showRadar;
extern bool showExtHUD;		/* Extended HUD */

extern bool clientRanker;

extern int wallDrawMode;
extern int decorDrawMode;
extern int baseWarningType;

#if 0
extern bool filledDecor;
extern bool filledWorld;
extern bool outlineDecor;
extern bool outlineWorld;
extern bool showDecor;
extern bool texturedDecor;
extern bool texturedWalls;
#endif
extern bool showItems;
extern bool showLivesByShip;
extern bool showMyShipShape;
extern bool showNastyShots;
extern bool showShipShapes;
extern bool showHitArea;
#if 0
extern bool slidingRadar;
#endif

extern bool showTalkMessages;
extern bool showGameMessages;
extern int showScoreListMode;
extern bool showExtHUDRadar;
extern bool showExtHUD;

extern double controlTime;	/* Display control for how long? */
extern uint8_t spark_rand;	/* Sparkling effect */
extern uint8_t old_spark_rand;	/* previous value of spark_rand */

extern short numTanks;		/* Number of tanks */
extern double fuelTime;		/* Display fuel for how long? */
extern int fuelCritical;	/* Fuel critical level */
extern int fuelWarning;		/* Fuel warning level */
extern int fuelNotify;		/* Fuel notify level */

extern char nickname[MAX_CHARS];	/* Nick-name of player */
extern char servername[MAX_CHARS];	/* Name of server connecting to */
extern unsigned version;	/* Version of the server */
extern int scoresChanged;
extern bool toggle_shield;	/* Are shields toggled by a press? */
extern bool shields;		/* When shields are considered up */
extern bool auto_shield;	/* drops shield for fire */

extern int maxFPS;		/* Max FPS player wants from server */
extern int oldMaxFPS;
extern int recordFPS;		/* What FPS to record at */
extern time_t currentTime;	/* Current value of time() */
extern int clientLag;		/* Time to draw a frame, unit us */
extern int rounds_played;
extern bool roundend;
extern int numClientLagSamples;	/* Make xpplot.m file with lag */

extern bool initialPointerControl;	/* Start by using mouse for control? */
extern bool pointerControl;	/* Mouse control enabled? */
extern ipos_t mousePosition;	/* position of mouse pointer. */
extern int mouseMovement;	/* horizontal mouse movement. */
extern pointer_move_t pointer_moves[MAX_POINTER_MOVES];
extern int pointer_move_next;
extern bool dirPrediction;	/* predict ship direction on server? */
extern int maxMouseTurnsPS;
extern int mouseMovementInterval;
extern int cumulativeMouseMovement;

extern char modBankStr[NUM_MODBANKS][MAX_CHARS];	/* modifier banks strings */

extern int maxCharsInNames;
extern uint8_t lose_item;	/* flag and index to drop item */
extern int lose_item_active;	/* one of the lose keys is pressed */

/* store message in history, when it is sent? */
extern bool save_talk_str;
extern char *talk_fast_msgs[];

extern int motd_viewer;		/* so Windows can clean him up */
extern int keys_viewer;
extern bool config_mapped;	/* Is config window visible */
extern bool players_exposed;

extern int num_playing_teams;
extern long start_loops, end_loops;
extern long time_left;
extern long timeLeft;
extern bool played_this_round;
extern int protocolVersion;

extern short ext_view_width;	/* Width of extended visible area */
extern short ext_view_height;	/* Height of extended visible area */
extern double scaleFactor;
extern double scaleFactor_s;

extern double hudRadarScale;	/* Scale for hudradar drawing */
extern double hudRadarDotScale;	/* Scale for radar dot drawing */
extern double hudRadarLimit;	/* Limit for hudradar drawing */

extern long last_keyboard_ack;
extern int receive_window_size;
extern long last_loops;
extern long loops;
extern int top_width;
extern int top_height;
extern int draw_width;		/* draw: map view window */
extern int draw_height;
extern int num_spark_colors;
extern uint8_t debris_colors;	/* Number of debris intensities from server */
extern int active_view_width;	/* W. of active map area displayed. */
extern int active_view_height;	/* H. of active map area displayed. */
extern int ext_view_x_offset;	/* Offset of ext_view_width */
extern int ext_view_y_offset;	/* Offset of ext_view_height */
extern bool packetMeasurement;
extern display_t server_display;	/* the servers idea about our display */

extern int hudSize;		/* Size for HUD drawing */

extern fuelstation_t *fuels;
extern int num_fuels;
extern homebase_t *bases;
extern int num_bases;
extern cannontime_t *cannons;
extern int num_cannons;
extern target_t *targets;
extern int num_targets;
extern int num_treasures;

extern checkpoint_t old_checks[MAX_CHECKS];
extern checkpoint_t *checks;
extern int num_checks;
extern xp_polygon_t *polygons;
extern int num_polygons, max_polygons;
extern edge_style_t *edge_styles;
extern int num_edge_styles, max_edge_styles;
extern polygon_style_t *polygon_styles;
extern int num_polygon_styles, max_polygon_styles;

extern score_object_t score_objects[MAX_SCORE_OBJECTS];
extern int score_object;

extern int eyesId;		/* Player we get frame updates for */
extern other_t *eyes;		/* Player we get frame updates for */
extern bool snooping;		/* are we snooping on someone else? */
extern int eyeTeam;		/* Team of player we get updates for */

extern int killratio_kills;
extern int killratio_deaths;
extern int killratio_totalkills;
extern int killratio_totaldeaths;

extern int ballsCashed;
extern int ballsReplaced;
extern int ballsCashedByTeam;
extern int ballsLost;

#ifdef SOUND
extern char sounds[MAX_CHARS];	/* audio mappings */
extern char audioServer[MAX_CHARS];	/* audio server */
extern int maxVolume;		/* maximum volume (in percent) */
#endif				/* SOUND */

#endif
