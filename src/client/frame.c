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

/* $Id: frame.c,v 1.19 2007/11/04 16:03:35 kps Exp $ */

#include "frame.h"
#include "bit.h"
#include "error.h"
#include "global.h"
#include "other.h"
#include "proto.h"
#include "portability.h"
#include "rules.h"
#include "setup.h"

int Handle_radar(int x, int y, int size);

refuel_t *refuel_ptr;
int num_refuel, max_refuel;
connector_t *connector_ptr;
int num_connector, max_connector;
laser_t *laser_ptr;
int num_laser, max_laser;
missile_t *missile_ptr;
int num_missile, max_missile;
ball_t *ball_ptr;
int num_ball, max_ball;
ship_t *ship_ptr;
int num_ship, max_ship;
mine_t *mine_ptr;
int num_mine, max_mine;
itemtype_t *itemtype_ptr;
int num_itemtype, max_itemtype;
ecm_t *ecm_ptr;
int num_ecm, max_ecm;
trans_t *trans_ptr;
int num_trans, max_trans;
radar_t *radar_ptr;
int num_radar, max_radar;
vcannon_t *vcannon_ptr;
int num_vcannon, max_vcannon;
vfuel_t *vfuel_ptr;
int num_vfuel, max_vfuel;
vbase_t *vbase_ptr;
int num_vbase, max_vbase;
debris_t *debris_ptr[DEBRIS_TYPES];
int num_debris[DEBRIS_TYPES], max_debris[DEBRIS_TYPES];
debris_t *fastshot_ptr[DEBRIS_TYPES * 2];
int num_fastshot[DEBRIS_TYPES * 2], max_fastshot[DEBRIS_TYPES * 2];
vdecor_t *vdecor_ptr;
int num_vdecor, max_vdecor;
wreckage_t *wreckage_ptr;
int num_wreckage, max_wreckage;
asteroid_t *asteroid_ptr;
int num_asteroids, max_asteroids;
wormhole_t *wormhole_ptr;
int num_wormholes, max_wormholes;

long start_loops, end_loops;

int eyesId;					/* Player we get frame updates for */
other_t *eyes = NULL;		/* Player we get frame updates for */
bool snooping;				/* are we snooping on someone else? */
int eyeTeam = TEAM_NOT_SET; /* Team of player we get updates for */
int scoresChanged;

short damaged;	/* Damaged by ECM */
short destruct; /* If self destructing */
short shutdown_delay;
short shutdown_count;
short thrusttime;
short thrusttimemax;
short shieldtime;
short shieldtimemax;
short phasingtime;
short phasingtimemax;

ipos_t selfPos;
ipos_t selfVel;
short heading;
double displayedPower;			/* What the server is sending us */
double displayedTurnspeed;		/* What the server is sending us */
double displayedTurnresistance; /* What the server is sending us */
short lock_id;					/* Id of player locked onto */
short lock_dir;					/* Direction of lock */
short lock_dist;				/* Distance to player locked onto */
short nextCheckPoint;
short autopilotLight;
uint8_t numItems[NUM_ITEMS];
uint8_t lastNumItems[NUM_ITEMS];
short currentTank; /* Number of currently used tank */
int fuelSum;	   /* Sum of fuel in all tanks */
int fuelMax;	   /* How much fuel can you take? */
double fuelTime;   /* Display fuel for how long? */
short selfVisible; /* Are we alive and playing? */
int packet_size;   /* Current frame update packet size */
ipos_t world;
ipos_t realWorld;

char modifiers[MAX_CHARS]; /* Current weapon modifiers */
int roundDelay;
int roundDelayMax;

time_t currentTime = 0;	   /* Current value of time() */
bool newSecond = false;	   /* Second changed this frame */
double clientFPS;		   /* FPS client is drawing at */
double timePerFrame = 0.0; /* Time a frame is shown, unit s */
bool played_this_round;
unsigned twelveHz; /* Attempt to increment this at 12Hz */

void Client_set_eyes_id(int id)
{
	eyesId = id;
	snooping = self && (id != self->id);
	scoresChanged++;
	/* from ng, not sure if this is any good */
	eyes = Other_by_id(eyesId);
	if (eyes != NULL)
		eyeTeam = eyes->team;
}

int Handle_start(long server_loops)
{
	int i;

	start_loops = server_loops;

	num_refuel = 0;
	num_connector = 0;
	num_missile = 0;
	num_ball = 0;
	num_ship = 0;
	num_mine = 0;
	num_itemtype = 0;
	num_ecm = 0;
	num_trans = 0;
	num_radar = 0;
	num_vcannon = 0;
	num_vfuel = 0;
	num_vbase = 0;
	num_vdecor = 0;
	for (i = 0; i < DEBRIS_TYPES; i++)
	{
		num_debris[i] = 0;
	}

	damaged = 0;
	destruct = 0;
	shutdown_delay = 0;
	shutdown_count = -1;
	thrusttime = -1;
	shieldtime = -1;
	phasingtime = -1;
	return 0;
}

static void update_timing(void)
{
	static int frame_counter = 0;
	static struct timeval old_tv = {0, 0};
	struct timeval now;

	frame_counter++;
	gettimeofday(&now, NULL);
	if (now.tv_sec != old_tv.tv_sec)
	{
		double usecs, fps;

		currentTime = time(NULL); /* ng */
		usecs = 1e6 + (now.tv_usec - old_tv.tv_usec);
		fps = (1e6 * frame_counter) / usecs;
		old_tv = now;
		newSecond = true;
		clientFPS = MAX(1.0, fps);
		timePerFrame = 1.0 / clientFPS;
		recordFPS = (int)(clientFPS + 0.5);
		frame_counter = 0;
		if (!played_this_round && self && !strchr("PW", self->mychar))
			played_this_round = true;
	}
	else
		newSecond = false;

	twelveHz = (unsigned)((12.0 * end_loops) / (double)FPS);
}

int Handle_end(long server_loops)
{
	end_loops = server_loops;
	update_timing();
	Paint_frame();
#ifdef SOUND
	audioUpdate();
#endif
	return 0;
}

int Handle_self_items(uint8_t *newNumItems)
{
	memcpy(numItems, newNumItems, NUM_ITEMS * sizeof(uint8_t));
	return 0;
}

/* this wasn't present in bp1 - probably useless */
static void update_status(int status)
{
	static int old_status = 0;

	if (BIT(old_status, OLD_GAME_OVER) && !BIT(status, OLD_GAME_OVER) && !BIT(status, OLD_PAUSE))
		Raise_window();

	/* Player appeared? */
	if (BIT(old_status, OLD_PLAYING | OLD_PAUSE | OLD_GAME_OVER) != OLD_PLAYING)
	{
		if (BIT(status, OLD_PLAYING | OLD_PAUSE | OLD_GAME_OVER) == OLD_PLAYING)
			Reset_shields();
	}

	old_status = status;
}

int Handle_self(int x, int y, int vx, int vy, int newHeading,
				float newPower, float newTurnspeed, float newTurnresistance,
				int newLockId, int newLockDist, int newLockBearing,
				int newNextCheckPoint, int newAutopilotLight,
				uint8_t *newNumItems, int newCurrentTank,
				int newFuelSum, int newFuelMax, int newPacketSize, int status /* kps - ng - ??? */)
{
	selfPos.x = x;
	selfPos.y = y;
	selfVel.x = vx;
	selfVel.y = vy;
	heading = newHeading;
	displayedPower = newPower;
	displayedTurnspeed = newTurnspeed;
	displayedTurnresistance = newTurnresistance;
	lock_id = newLockId;
	lock_dist = newLockDist;
	lock_dir = newLockBearing;
	nextCheckPoint = newNextCheckPoint;
	autopilotLight = newAutopilotLight;
	memcpy(numItems, newNumItems, NUM_ITEMS * sizeof(uint8_t));
	currentTank = newCurrentTank;
	if (newFuelSum > fuelSum && selfVisible)
		fuelTime = FUEL_NOTIFY_TIME;
	fuelSum = newFuelSum;
	fuelMax = newFuelMax;
	selfVisible = 0;
	if (newPacketSize + 16 < packet_size)
	{
		packet_size -= 16;
	}
	else
	{
		packet_size = newPacketSize;
	}
	update_status(status); /* kps - ng - ??? */

	world.x = selfPos.x - (ext_view_width / 2);
	world.y = selfPos.y - (ext_view_height / 2);
	realWorld = world;
	if (BIT(Setup->mode, WRAP_PLAY))
	{
		if (world.x < 0 && world.x + ext_view_width < Setup->width)
		{
			world.x += Setup->width;
		}
		else if (world.x > 0 && world.x + ext_view_width >= Setup->width)
		{
			realWorld.x -= Setup->width;
		}
		if (world.y < 0 && world.y + ext_view_height < Setup->height)
		{
			world.y += Setup->height;
		}
		else if (world.y > 0 && world.y + ext_view_height >= Setup->height)
		{
			realWorld.y -= Setup->height;
		}
	}
	return 0;
}

int Handle_eyes(int id)
{
	/* not interested */
	return 0;
}

int Handle_damaged(int dam)
{
	damaged = dam;
	return 0;
}

int Handle_modifiers(char *m)
{
	strlcpy(modifiers, m, MAX_CHARS);
	return 0;
}

int Handle_destruct(int count)
{
	destruct = count;
	return 0;
}

int Handle_shutdown(int count, int delay)
{
	shutdown_count = count;
	shutdown_delay = delay;
	return 0;
}

int Handle_thrusttime(int count, int max)
{
	thrusttime = count;
	thrusttimemax = max;
	return 0;
}

int Handle_shieldtime(int count, int max)
{
	shieldtime = count;
	shieldtimemax = max;
	return 0;
}

int Handle_phasingtime(int count, int max)
{
	phasingtime = count;
	phasingtimemax = max;
	return 0;
}

int Handle_rounddelay(int count, int max)
{
	roundDelay = count;
	roundDelayMax = max;
	return 0;
}

int Handle_refuel(int xa, int ya, int xb, int yb)
{
	refuel_t t;

	t.xa = xa;
	t.xb = xb;
	t.ya = ya;
	t.yb = yb;
	STORE(refuel_t, refuel_ptr, num_refuel, max_refuel, t);
	return 0;
}

int Handle_connector(int xa, int ya, int xb, int yb, int tractor)
{
	connector_t t;

	t.xa = xa;
	t.xb = xb;
	t.ya = ya;
	t.yb = yb;
	t.tractor = tractor;
	STORE(connector_t, connector_ptr, num_connector, max_connector, t);
	return 0;
}

int Handle_laser(int color, int x, int y, int len, int dir)
{
	laser_t t;

	t.color = color;
	t.x = x;
	t.y = y;
	t.len = len;
	t.dir = dir;
	STORE(laser_t, laser_ptr, num_laser, max_laser, t);
	return 0;
}

int Handle_missile(int x, int y, int len, int dir)
{
	missile_t t;

	t.x = x;
	t.y = y;
	t.dir = dir;
	t.len = len;
	STORE(missile_t, missile_ptr, num_missile, max_missile, t);
	return 0;
}

int Handle_ball(int x, int y, int id, int style)
{
	ball_t t;

	t.x = x;
	t.y = y;
	t.id = id;
	t.style = style;
	STORE(ball_t, ball_ptr, num_ball, max_ball, t);
	return 0;
}

static int predict_self_dir(int received_dir)
{
	double pointer_delta = 0, dir_delta, new_dir;
	int ind = pointer_move_next - 1;
	int count = 0, int_new_dir;

	if (ind < 0)
		ind = MAX_POINTER_MOVES - 1;

	while (pointer_moves[ind].id > last_keyboard_ack && count < 50)
	{
		pointer_delta += pointer_moves[ind].movement * pointer_moves[ind].turnspeed;
		ind--;
		if (ind < 0)
			ind = MAX_POINTER_MOVES - 1;
		count++;
	}

	dir_delta = pointer_delta / (RES / 2);
	new_dir = (received_dir - dir_delta);
	while (new_dir < 0)
		new_dir += RES;
	while (new_dir >= RES)
		new_dir -= RES;
	int_new_dir = (int)(new_dir + 0.5);
	while (int_new_dir >= RES)
		/* might be == RES */
		int_new_dir -= RES;

	return int_new_dir;
}

int Handle_ship(int x, int y, int id, float angle, int shield, int cloak, int eshield, int phased,
				int deflector)
{
	ship_t t;

	t.x = x;
	t.y = y;
	t.id = id;
	if (dirPrediction && self && self->id == id)
	{
		int dir = angle_to_int_dir(angle);
		dir = predict_self_dir(dir);
		t.angle = dir_to_angle(dir);
	}
	else
		t.angle = angle;
	t.shield = shield;
	t.cloak = cloak;
	t.eshield = eshield;
	t.phased = phased;
	t.deflector = deflector;
	STORE(ship_t, ship_ptr, num_ship, max_ship, t);

	/* if we see a ship in the center of the display, we may be watching
	 * it, especially if it's us!  consider any ship there to be our eyes
	 * until we see a ship that really is us.
	 * BG: XXX there was a bug here.  self was dereferenced at "self->id"
	 * while self could be NULL here.
	 */
	if (!selfVisible && ((x == selfPos.x && y == selfPos.y) || (self && id == self->id)))
	{
		Client_set_eyes_id(id);
		selfVisible = (self && (id == self->id));
		return Handle_radar(x, y, 3);
	}

	return 0;
}

int Handle_mine(int x, int y, int teammine, int id)
{
	mine_t t;

	t.x = x;
	t.y = y;
	t.teammine = teammine;
	t.id = id;
	STORE(mine_t, mine_ptr, num_mine, max_mine, t);
	return 0;
}

int Handle_item(int x, int y, int type)
{
	itemtype_t t;

	t.x = x;
	t.y = y;
	t.type = type;
	STORE(itemtype_t, itemtype_ptr, num_itemtype, max_itemtype, t);
	return 0;
}

#define STORE_DEBRIS(typ_e, _p, _n)                               \
	if (_n > max_)                                                \
	{                                                             \
		if (max_ == 0)                                            \
		{                                                         \
			ptr_ = (debris_t *)malloc(n * sizeof(*ptr_));         \
		}                                                         \
		else                                                      \
		{                                                         \
			ptr_ = (debris_t *)realloc(ptr_, _n * sizeof(*ptr_)); \
		}                                                         \
		if (ptr_ == NULL)                                         \
		{                                                         \
			error("No memory for debris");                        \
			num_ = max_ = 0;                                      \
			return -1;                                            \
		}                                                         \
		max_ = _n;                                                \
	}                                                             \
	else if (_n <= 0)                                             \
	{                                                             \
		printf("debris %d < 0\n", _n);                            \
		return 0;                                                 \
	}                                                             \
	num_ = _n;                                                    \
	memcpy(ptr_, _p, _n * sizeof(*ptr_));                         \
	return 0;

int Handle_fastshot(int type, uint8_t *p, int n)
{
#define num_ (num_fastshot[type])
#define max_ (max_fastshot[type])
#define ptr_ (fastshot_ptr[type])
	STORE_DEBRIS(type, p, n);
#undef num_
#undef max_
#undef ptr_
}

int Handle_debris(int type, uint8_t *p, int n)
{
#define num_ (num_debris[type])
#define max_ (max_debris[type])
#define ptr_ (debris_ptr[type])
	STORE_DEBRIS(type, p, n);
#undef num_
#undef max_
#undef ptr_
}

int Handle_wreckage(int x, int y, int wrecktype, int size, int rotation)
{
	wreckage_t t;

	t.x = x;
	t.y = y;
	t.wrecktype = wrecktype;
	t.size = size;
	t.rotation = rotation;
	STORE(wreckage_t, wreckage_ptr, num_wreckage, max_wreckage, t);
	return 0;
}

int Handle_asteroid(int x, int y, int type, int size, int rotation)
{
	asteroid_t t;

	t.x = x;
	t.y = y;
	t.type = type;
	t.size = size;
	t.rotation = rotation;
	STORE(asteroid_t, asteroid_ptr, num_asteroids, max_asteroids, t);
	return 0;
}

int Handle_wormhole(int x, int y)
{
	wormhole_t t;

	t.x = x - BLOCK_SZ / 2;
	t.y = y - BLOCK_SZ / 2;
	STORE(wormhole_t, wormhole_ptr, num_wormholes, max_wormholes, t);
	return 0;
}

int Handle_ecm(int x, int y, int size)
{
	ecm_t t;

	t.x = x;
	t.y = y;
	t.size = size;
	STORE(ecm_t, ecm_ptr, num_ecm, max_ecm, t);
	return 0;
}

int Handle_trans(int xa, int ya, int xb, int yb)
{
	trans_t t;

	t.xa = xa;
	t.ya = ya;
	t.xb = xb;
	t.yb = yb;
	STORE(trans_t, trans_ptr, num_trans, max_trans, t);
	return 0;
}

int Handle_paused(int x, int y, int count)
{
	return 0;
}

int Handle_appearing(int x, int y, int id, int count)
{
	return 0;
}

int Handle_fastradar(int x, int y, int size)
{
	radar_t t;

	t.x = x;
	t.y = y;
	t.type = RadarEnemy;

	if ((size & 0x80) != 0)
	{
		t.type = RadarFriend;
		size &= ~0x80;
	}

	t.size = size;
	STORE(radar_t, radar_ptr, num_radar, max_radar, t);
	return 0;
}

int Handle_radar(int x, int y, int size)
{
	return Handle_fastradar((int)((double)(x * RadarWidth) / Setup->width + 0.5),
							(int)((double)(y * RadarHeight) / Setup->height + 0.5), size);
}

int Handle_message(char *msg)
{
	int i;
	char ignoree[MSG_LEN]; /* kps - was only MAX_CHARS previously. */
	other_t *other;

	if (msg[strlen(msg) - 1] == ']')
	{
		for (i = strlen(msg) - 1; i > 0; i--)
		{
			if (msg[i - 1] == ' ' && msg[i] == '[')
				break;
		}

		if (i == 0)
		{ /* Odd, but let it pass */
			Add_message("%s", msg);
			return 0;
		}

		strcpy(ignoree, &msg[i + 1]);

		for (i = 0; i < (int)strlen(ignoree); i++)
		{
			if (ignoree[i] == ']')
				break;
		}
		ignoree[i] = '\0';

		other = Other_by_name(ignoree, false);

		if (other == NULL)
		{ /* Not in list, probably servermessage */
			Add_message("%s", msg);
			return 0;
		}

		if (other->ignorelevel <= 0)
		{
			Add_message("%s", msg);
			return 0;
		}

		if (other->ignorelevel >= 2)
			return 0;

		/* ignorelevel must be 1 */

		crippleTalk(msg);
		Add_message("%s", msg);
	}
	else
		Add_message("%s", msg);
	return 0;
}

int Handle_time_left(int sec)
{
#if 0
	/* kps - Let's not bother. */
	if (sec >= 0 && sec < 10 && (time_left > sec || sec == 0))
		Play_beep();
	time_left = (sec >= 0) ? sec : 0;
#endif
	return 0;
}

int Handle_vcannon(int x, int y, int type)
{
	vcannon_t t;

	t.x = x;
	t.y = y;
	t.type = type;
	STORE(vcannon_t, vcannon_ptr, num_vcannon, max_vcannon, t);
	return 0;
}

int Handle_vfuel(int x, int y, int fuel)
{
	vfuel_t t;

	t.x = x;
	t.y = y;
	t.fuel = fuel;
	STORE(vfuel_t, vfuel_ptr, num_vfuel, max_vfuel, t);
	return 0;
}

int Handle_vbase(int x, int y, int xi, int yi, int type)
{
	vbase_t t;

	t.x = x;
	t.y = y;
	t.xi = xi;
	t.yi = yi;
	t.type = type;
	STORE(vbase_t, vbase_ptr, num_vbase, max_vbase, t);
	return 0;
}

int Handle_vdecor(int x, int y, int xi, int yi, int type)
{
	vdecor_t t;

	t.x = x;
	t.y = y;
	t.xi = xi;
	t.yi = yi;
	t.type = type;
	STORE(vdecor_t, vdecor_ptr, num_vdecor, max_vdecor, t);
	return 0;
}

void frameCleanup(void)
{
	if (max_refuel > 0 && refuel_ptr)
	{
		max_refuel = 0;
		XFREE(refuel_ptr);
	}
	if (max_connector > 0 && connector_ptr)
	{
		max_connector = 0;
		XFREE(connector_ptr);
	}
	if (max_laser > 0 && laser_ptr)
	{
		max_laser = 0;
		XFREE(laser_ptr);
	}
	if (max_missile > 0 && missile_ptr)
	{
		max_missile = 0;
		XFREE(missile_ptr);
	}
	if (max_ball > 0 && ball_ptr)
	{
		max_ball = 0;
		XFREE(ball_ptr);
	}
	if (max_ship > 0 && ship_ptr)
	{
		max_ship = 0;
		XFREE(ship_ptr);
	}
	if (max_mine > 0 && mine_ptr)
	{
		max_mine = 0;
		XFREE(mine_ptr);
	}
	if (max_ecm > 0 && ecm_ptr)
	{
		max_ecm = 0;
		XFREE(ecm_ptr);
	}
	if (max_trans > 0 && trans_ptr)
	{
		max_trans = 0;
		XFREE(trans_ptr);
	}
	if (max_radar > 0 && radar_ptr)
	{
		max_radar = 0;
		XFREE(radar_ptr);
	}
	if (max_vcannon > 0 && vcannon_ptr)
	{
		max_vcannon = 0;
		XFREE(vcannon_ptr);
	}
	if (max_vfuel > 0 && vfuel_ptr)
	{
		max_vfuel = 0;
		XFREE(vfuel_ptr);
	}
	if (max_vbase > 0 && vbase_ptr)
	{
		max_vbase = 0;
		XFREE(vbase_ptr);
	}
	if (max_vdecor > 0 && vdecor_ptr)
	{
		max_vdecor = 0;
		XFREE(vdecor_ptr);
	}
	if (max_itemtype > 0 && itemtype_ptr)
	{
		max_itemtype = 0;
		XFREE(itemtype_ptr);
	}
	if (max_wreckage > 0 && wreckage_ptr)
	{
		max_wreckage = 0;
		XFREE(wreckage_ptr);
	}
	if (max_asteroids > 0 && asteroid_ptr)
	{
		max_asteroids = 0;
		XFREE(asteroid_ptr);
	}
	if (max_wormholes > 0 && wormhole_ptr)
	{
		max_wormholes = 0;
		XFREE(wormhole_ptr);
	}
}
