/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2005 by
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

/* $Id: frame.h,v 1.11 2007/10/30 23:45:43 kps Exp $ */

#ifndef FRAME_H
#define FRAME_H

/* need uint8_t */
#ifndef	TYPES_H
#include "types.h"
#endif
#ifndef CONST_H
#include "const.h"
#endif
#ifndef ITEM_H
#include "item.h"
#endif
#ifndef OTHER_H
#include "other.h"
#endif

/*
 * Local types and data for painting.
 */

typedef struct {
	short xa, ya, xb, yb;
} refuel_t;

typedef struct {
	short xa, ya, xb, yb;
	uint8_t tractor;
} connector_t;

typedef struct {
	unsigned char color, dir;
	short x, y, len;
} laser_t;

typedef struct {
	short x, y, dir;
	unsigned char len;
} missile_t;

typedef struct {
	short x, y, id;
	uint8_t style;
} ball_t;

typedef struct {
	short x, y, id;
	float angle;
	uint8_t shield, cloak, eshield;
	uint8_t phased, deflector;
} ship_t;

typedef struct {
	short x, y, teammine, id;
} mine_t;

typedef struct {
	short x, y, type;
} itemtype_t;

typedef struct {
	short x, y, size;
} ecm_t;

typedef struct {
	short xa, ya, xb, yb;
} trans_t;

typedef enum {
	RadarEnemy,
	RadarFriend
} radar_type_t;

typedef struct {
	short x, y, size;
	radar_type_t type;
} radar_t;

typedef struct {
	short x, y, type;
} vcannon_t;

typedef struct {
	short x, y;
	int fuel;
} vfuel_t;

typedef struct {
	short x, y, xi, yi, type;
} vbase_t;

typedef struct {
	uint8_t x, y;
} debris_t;

typedef struct {
	short x, y, xi, yi, type;
} vdecor_t;

typedef struct {
	short x, y;
	uint8_t wrecktype, size, rotation;
} wreckage_t;

typedef struct {
	short x, y;
	uint8_t type, size, rotation;
} asteroid_t;

typedef struct {
	short x, y;
} wormhole_t;

extern refuel_t *refuel_ptr;
extern int num_refuel, max_refuel;
extern connector_t *connector_ptr;
extern int num_connector, max_connector;
extern laser_t *laser_ptr;
extern int num_laser, max_laser;
extern missile_t *missile_ptr;
extern int num_missile, max_missile;
extern ball_t *ball_ptr;
extern int num_ball, max_ball;
extern ship_t *ship_ptr;
extern int num_ship, max_ship;
extern mine_t *mine_ptr;
extern int num_mine, max_mine;
extern itemtype_t *itemtype_ptr;
extern int num_itemtype, max_itemtype;
extern ecm_t *ecm_ptr;
extern int num_ecm, max_ecm;
extern trans_t *trans_ptr;
extern int num_trans, max_trans;
extern radar_t *radar_ptr;
extern int num_radar, max_radar;
extern vcannon_t *vcannon_ptr;
extern int num_vcannon, max_vcannon;
extern vfuel_t *vfuel_ptr;
extern int num_vfuel, max_vfuel;
extern vbase_t *vbase_ptr;
extern int num_vbase, max_vbase;
extern debris_t *debris_ptr[DEBRIS_TYPES];
extern int num_debris[DEBRIS_TYPES], max_debris[DEBRIS_TYPES];
extern debris_t *fastshot_ptr[DEBRIS_TYPES * 2];
extern int num_fastshot[DEBRIS_TYPES * 2], max_fastshot[DEBRIS_TYPES * 2];
extern vdecor_t *vdecor_ptr;
extern int num_vdecor, max_vdecor;
extern wreckage_t *wreckage_ptr;
extern int num_wreckage, max_wreckage;
extern asteroid_t *asteroid_ptr;
extern int num_asteroids, max_asteroids;
extern wormhole_t *wormhole_ptr;
extern int num_wormholes, max_wormholes;

extern long start_loops, end_loops;

extern int eyesId;		/* Player we get frame updates for */
extern other_t *eyes;		/* Player we get frame updates for */
extern bool snooping;		/* are we snooping on someone else? */
extern int eyeTeam;		/* Team of player we get updates for */
extern int scoresChanged;

extern short damaged;		/* Damaged by ECM */
extern short destruct;		/* If self destructing */
extern short shutdown_delay;
extern short shutdown_count;
extern short thrusttime;
extern short thrusttimemax;
extern short shieldtime;
extern short shieldtimemax;
extern short phasingtime;
extern short phasingtimemax;



extern ipos_t selfPos;
extern ipos_t selfVel;
extern short heading;
extern double displayedPower;	/* What the server is sending us */
extern double displayedTurnspeed;	/* What the server is sending us */
extern double displayedTurnresistance;	/* What the server is sending us */
extern short lock_id;		/* Id of player locked onto */
extern short lock_dir;		/* Direction of lock */
extern short lock_dist;		/* Distance to player locked onto */
extern short nextCheckPoint;
extern short autopilotLight;
extern uint8_t numItems[NUM_ITEMS];
extern uint8_t lastNumItems[NUM_ITEMS];
extern short currentTank;	/* Number of currently used tank */
extern int fuelSum;		/* Sum of fuel in all tanks */
extern int fuelMax;		/* How much fuel can you take? */
extern double fuelTime;		/* Display fuel for how long? */
extern short selfVisible;	/* Are we alive and playing? */
extern int packet_size;		/* Current frame update packet size */
extern ipos_t world;
extern ipos_t realWorld;

extern char modifiers[MAX_CHARS];	/* Current weapon modifiers */
extern int roundDelay;
extern int roundDelayMax;

extern bool newSecond;		/* Second changed this frame */
extern double clientFPS;	/* FPS client is drawing at */
extern double timePerFrame;	/* Time a frame is shown, unit s */
extern bool played_this_round;
extern unsigned twelveHz;	/* Attempt to increment this at 12Hz */

void Paint_frame(void);
void frameCleanup(void);

int Handle_start(long server_loops);
int Handle_end(long server_loops);
int Handle_self(int x, int y, int vx, int vy, int newHeading,
		float newPower, float newTurnspeed, float newTurnresistance,
		int newLockId, int newLockDist, int newLockBearing,
		int newNextCheckPoint, int newAutopilotLight,
		uint8_t * newNumItems, int newCurrentTank,
		int newFuelSum, int newFuelMax, int newPacketSize,
		int status /* kps - ng - ??? */ );
int Handle_self_items(uint8_t * newNumItems);
int Handle_modifiers(char *m);
int Handle_damaged(int damaged);
int Handle_destruct(int count);
int Handle_shutdown(int count, int delay);
int Handle_thrusttime(int count, int max);
int Handle_shieldtime(int count, int max);
int Handle_phasingtime(int count, int max);
int Handle_rounddelay(int count, int max);
int Handle_refuel(int xa, int ya, int xb, int yb);
int Handle_connector(int xa, int ya, int xb, int yb, int tractor);
int Handle_laser(int color, int x, int y, int len, int dir);
int Handle_missile(int x, int y, int dir, int len);
int Handle_ball(int x, int y, int id, int style);
int Handle_ship(int x, int y, int id, float angle, int shield, int cloak, int eshield, int phased,
		int deflector);
int Handle_mine(int x, int y, int teammine, int id);
int Handle_item(int x, int y, int type);
int Handle_fastshot(int type, uint8_t * p, int n);
int Handle_debris(int type, uint8_t * p, int n);
int Handle_wreckage(int x, int y, int wrecktype, int size, int rotation);
int Handle_asteroid(int x, int y, int type, int size, int rotation);
int Handle_wormhole(int x, int y);
int Handle_ecm(int x, int y, int size);
int Handle_trans(int xa, int ya, int xb, int yb);
int Handle_paused(int x, int y, int count);
int Handle_appearing(int x, int y, int id, int count);
int Handle_radar(int x, int y, int size);
int Handle_fastradar(int x, int y, int size);
int Handle_vcannon(int x, int y, int type);
int Handle_vfuel(int x, int y, int fuel);
int Handle_vbase(int x, int y, int xi, int yi, int type);
int Handle_vdecor(int x, int y, int xi, int yi, int type);
int Handle_message(char *msg);
int Handle_eyes(int id);
int Handle_time_left(int sec);

#endif
