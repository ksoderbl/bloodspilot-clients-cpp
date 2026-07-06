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

/* $Id: client.c,v 1.24 2007/11/12 00:13:17 kps Exp $ */

#include "sysdeps.h"

#include "client.h"
#include "bit.h"
#include "rules.h"
#include "error.h"
#include "proto.h"
#include "const.h"
#include "global.h"
#include "frame.h"
#include "setup.h"

static double teamscores[MAX_TEAMS];

static fuelstation_t *Fuelstation_by_pos(int x, int y)
{
	int i, lo, hi, pos;

	lo = 0;
	hi = num_fuels - 1;
	pos = x * Setup->y + y;
	while (lo < hi)
	{
		i = (lo + hi) >> 1;
		if (pos > fuels[i].pos)
			lo = i + 1;
		else
			hi = i;
	}
	if (lo == hi && pos == fuels[lo].pos)
		return &fuels[lo];
	warn("No fuelstation at (%d,%d)", x, y);
	return NULL;
}

int Fuel_by_pos(int x, int y)
{
	fuelstation_t *fuelp;

	if ((fuelp = Fuelstation_by_pos(x, y)) == NULL)
		return 0;
	return fuelp->fuel;
}

int Target_by_index(int ind, int *xp, int *yp, int *dead_time, int *damage)
{
	if (ind < 0 || ind >= num_targets)
		return -1;
	*xp = targets[ind].pos / Setup->y;
	*yp = targets[ind].pos % Setup->y;
	*dead_time = targets[ind].dead_time;
	*damage = targets[ind].damage;
	return 0;
}

int Target_alive(int x, int y, int *damage)
{
	int i, lo, hi, pos;

	lo = 0;
	hi = num_targets - 1;
	pos = x * Setup->y + y;
	while (lo < hi)
	{
		i = (lo + hi) >> 1;
		if (pos > targets[i].pos)
			lo = i + 1;
		else
			hi = i;
	}
	if (lo == hi && pos == targets[lo].pos)
	{
		*damage = targets[lo].damage;
		return targets[lo].dead_time;
	}
	warn("No targets at (%d,%d)", x, y);
	return -1;
}

int Handle_fuel(int ind, int fuel)
{
	if (ind < 0 || ind >= num_fuels)
	{
		warn("Bad fuelstation index (%d)", ind);
		return -1;
	}
	fuels[ind].fuel = fuel;
	return 0;
}

static cannontime_t *Cannon_by_pos(int x, int y)
{
	int i, lo, hi, pos;

	lo = 0;
	hi = num_cannons - 1;
	pos = x * Setup->y + y;
	while (lo < hi)
	{
		i = (lo + hi) >> 1;
		if (pos > cannons[i].pos)
			lo = i + 1;
		else
			hi = i;
	}
	if (lo == hi && pos == cannons[lo].pos)
		return &cannons[lo];
	warn("No cannon at (%d,%d)", x, y);
	return NULL;
}

int Cannon_dead_time_by_pos(int x, int y, int *dot)
{
	cannontime_t *cannonp;

	if ((cannonp = Cannon_by_pos(x, y)) == NULL)
		return -1;
	*dot = cannonp->dot;
	return cannonp->dead_time;
}

int Handle_cannon(int ind, int dead_time)
{
	if (ind < 0 || ind >= num_cannons)
	{
		warn("Bad cannon index (%d)", ind);
		return 0;
	}
	cannons[ind].dead_time = dead_time;
	return 0;
}

int Handle_target(int num, int dead_time, int damage)
{
	if (num < 0 || num >= num_targets)
	{
		warn("Bad target index (%d)", num);
		return 0;
	}
	if (dead_time == 0 && (damage <= 0 || damage > TARGET_DAMAGE))
		warn("BUG target %d, dead %d, damage %f", num, dead_time, damage);

	if (targets[num].dead_time > 0 && dead_time == 0)
	{
		int pos = targets[num].pos;
		Radar_show_target(pos / Setup->y, pos % Setup->y);
	}
	else if (targets[num].dead_time == 0 && dead_time > 0)
	{
		int pos = targets[num].pos;
		Radar_hide_target(pos / Setup->y, pos % Setup->y);
	}

	targets[num].dead_time = dead_time;
	targets[num].damage = damage;

	return 0;
}

homebase_t *Homebase_by_pos(int x, int y)
{
	int i, lo, hi, pos;

	lo = 0;
	hi = num_bases - 1;
	pos = x * Setup->y + y;
	while (lo < hi)
	{
		i = (lo + hi) >> 1;
		if (pos > bases[i].pos)
			lo = i + 1;
		else
			hi = i;
	}
	if (lo == hi && pos == bases[lo].pos)
		return &bases[lo];
	warn("No homebase at (%d,%d)", x, y);
	return NULL;
}

int Base_info_by_pos(int x, int y, int *idp, int *teamp)
{
	homebase_t *basep;

	if ((basep = Homebase_by_pos(x, y)) == NULL)
		return -1;
	*idp = basep->id;
	*teamp = basep->team;
	return 0;
}

int Handle_base(int id, int ind)
{
	int i;

	if (ind < 0 || ind >= num_bases)
	{
		warn("Bad homebase index (%d)", ind);
		return -1;
	}
	for (i = 0; i < num_bases; i++)
	{
		if (bases[i].id == id)
			bases[i].id = -1;
	}
	bases[ind].id = id;

	return 0;
}

int Check_pos_by_index(int ind, int *xp, int *yp)
{
	if (ind < 0 || ind >= num_checks)
	{
		warn("Bad checkpoint index (%d)", ind);
		*xp = 0;
		*yp = 0;
		return -1;
	}
	*xp = checks[ind].pos / Setup->y;
	*yp = checks[ind].pos % Setup->y;
	return 0;
}

int Check_index_by_pos(int x, int y)
{
	int i, pos;

	pos = x * Setup->y + y;
	for (i = 0; i < num_checks; i++)
	{
		if (pos == checks[i].pos)
			return i;
	}
	warn("Can't find checkpoint (%d,%d)", x, y);
	return 0;
}

static int Map_init(void)
{
	return oldServer ? init_blockmap() : init_polymap();
}

static int Map_cleanup(void)
{
	if (num_bases > 0)
	{
		XFREE(bases);
		num_bases = 0;
	}
	if (num_fuels > 0)
	{
		XFREE(fuels);
		num_fuels = 0;
	}
	if (num_targets > 0)
	{
		XFREE(targets);
		num_targets = 0;
	}
	if (num_cannons > 0)
	{
		XFREE(cannons);
		num_cannons = 0;
	}
	return 0;
}

homebase_t *Homebase_by_id(int id)
{
	int i;

	if (id != -1)
	{
		for (i = 0; i < num_bases; i++)
		{
			if (bases[i].id == id)
				return &bases[i];
		}
	}
	return NULL;
}

int Handle_leave(int id)
{
	other_t *other;
	int i;

	if ((other = Other_by_id(id)) != NULL)
	{
		if (other == self)
		{
			warn("Self left?!");
			self = NULL;
		}
		Free_ship_shape(other->ship);
		other->ship = NULL;
		/*
		 * Silent about tanks and robots.
		 */
		if (other->mychar != 'T' && other->mychar != 'R')
		{
			Add_message("%s left this world.", other->nick_name);
		}
		num_others--;
		while (other < &Others[num_others])
		{
			*other = other[1];
			other++;
		}
		scoresChanged = 1;
	}
	for (i = 0; i < num_others; i++)
	{
		other = &Others[i];
		if (other->war_id == id)
		{
			other->war_id = -1;
			scoresChanged = 1;
		}
	}
	return 0;
}

int Handle_player(int id, int player_team, int mychar,
				  char *nick_name, char *user_name, char *host_name, char *shape, int myself)
{
	other_t *other;

	if (BIT(Setup->mode, TEAM_PLAY) && (player_team < 0 || player_team >= MAX_TEAMS))
	{
		warn("Illegal team %d for received player, setting to 0", player_team);
		player_team = 0;
	}
	if ((other = Other_by_id(id)) == NULL)
	{
		if (num_others >= max_others)
		{
			max_others += 5;
			if (num_others == 0)
				Others = XMALLOC(other_t, max_others);
			else
				Others = XREALLOC(other_t, Others, max_others);
			if (Others == NULL)
				fatal("Not enough memory for player info");
			if (self != NULL)
				/* We've made 'self' the first member of Others[]. */
				self = &Others[0];
		}
		other = &Others[num_others++];
	}
	/*
	 * Servers send self as the first player.
	 */
	if (self == NULL)
	{
		if (other != &Others[0])
		{
			/* Make 'self' the first member of Others[]. */
			*other = Others[0];
			other = &Others[0];
		}
		self = other;
	}
	memset(other, 0, sizeof(other_t));
	other->id = id;
	other->team = player_team;
	other->mychar = mychar;
	other->war_id = -1;
	strlcpy(other->nick_name, nick_name, sizeof(other->nick_name));
	strlcpy(other->user_name, user_name, sizeof(other->user_name));
	strlcpy(other->host_name, host_name, sizeof(other->host_name));
	strlcpy(other->abbrev_nick_name, nick_name, sizeof(other->abbrev_nick_name));
	other->max_chars_in_names = -1;
	scoresChanged = 1;
	other->ship = Convert_shape_str(shape);

	return 0;
}

int Handle_team(int id, int pl_team)
{
	other_t *other;

	other = Other_by_id(id);
	if (other == NULL)
	{
		warn("Received packet to change team for nonexistent id %d", id);
		return 0;
	}
	if (BIT(Setup->mode, TEAM_PLAY) && (pl_team < 0 || pl_team >= MAX_TEAMS))
	{
		warn("Illegal team %d received for player id %d", pl_team, id);
		return 0;
	}
	other->team = pl_team;
	scoresChanged = 1;

	return 0;
}

int Handle_war(int robot_id, int killer_id)
{
	other_t *robot, *killer;

	if ((robot = Other_by_id(robot_id)) == NULL)
	{
		warn("Can't update war for non-existing player (%d,%d)", robot_id, killer_id);
		return 0;
	}
	if (killer_id == -1)
	{
		/*
		 * Robot is no longer in war mode.
		 */
		robot->war_id = -1;
		return 0;
	}
	if ((killer = Other_by_id(killer_id)) == NULL)
	{
		warn("Can't update war against non-existing player (%d,%d)", robot_id, killer_id);
		return 0;
	}
	robot->war_id = killer_id;
	Add_message("%s declares war on %s.", robot->nick_name, killer->nick_name);
	scoresChanged = 1;

	return 0;
}

int Handle_seek(int programmer_id, int robot_id, int sought_id)
{
	other_t *programmer, *robot, *sought;

	if ((programmer = Other_by_id(programmer_id)) == NULL || (robot = Other_by_id(robot_id)) == NULL || (sought = Other_by_id(sought_id)) == NULL)
	{
		warn("Bad player seek (%d,%d,%d)", programmer_id, robot_id, sought_id);
		return 0;
	}
	robot->war_id = sought_id;
	Add_message("%s has programmed %s to seek %s.", programmer->nick_name, robot->nick_name,
				sought->nick_name);
	scoresChanged = 1;

	return 0;
}

int Handle_score(int id, int score, int life, int mychar, int alliance)
{
	other_t *other;

	if ((other = Other_by_id(id)) == NULL)
	{
		warn("Can't update score for non-existing player %d,%.2f,%d", id, score, life);
		return 0;
	}
	else if (other->score != score || other->life != life || other->mychar != mychar || other->alliance != alliance)
	{
		other->score = score;
		other->life = life;
		other->mychar = mychar;
		other->alliance = alliance;
		scoresChanged = 1;
	}

	return 0;
}

int Handle_team_score(int team, int score)
{
	if (teamscores[team] != score)
	{
		teamscores[team] = score;
		scoresChanged = 1;
	}

	return 0;
}

int Handle_timing(int id, int check, int round, long tloops)
{
	other_t *other;

	if ((other = Other_by_id(id)) == NULL)
	{
		warn("Can't update timing for non-existing player %d,%d,%d", id, check, round);
		return 0;
	}
	else if (other->check != check || other->round != round)
	{
		other->check = check;
		other->round = round;
		other->timing = round * num_checks + check;
		other->timing_loops = tloops;
		scoresChanged = 1;
	}

	return 0;
}

int Handle_score_object(int score, int x, int y, char *msg)
{
	score_object_t *sobj = &score_objects[score_object];

	sobj->score = score;
	sobj->x = x;
	sobj->y = y;
	sobj->life_time = scoreObjectTime;

	/* Initialize sobj->hud_msg (is shown on the HUD) */
	if (msg[0] != '\0')
	{
		sprintf(sobj->hud_msg, "%s %d", msg, score);
		sobj->hud_msg_len = strlen(sobj->hud_msg);
		sobj->hud_msg_width = -1;
	}
	else
		sobj->hud_msg_len = 0;

	/* Initialize sobj->msg data (is shown on game area) */
	sprintf(sobj->msg, "%d", score);
	sobj->msg_len = strlen(sobj->msg);
	sobj->msg_width = -1;

	/* Update global index variable */
	score_object = (score_object + 1) % MAX_SCORE_OBJECTS;

	return 0;
}

int Handle_polystyle(int polyind, int newstyle)
{
	xp_polygon_t *poly;

	poly = &polygons[polyind];
	poly->style = newstyle;
	/*warn("polygon %d style set to %d", polyind, newstyle); */
	UpdateRadar = true;
	return 0;
}

int Client_init(char *server, unsigned server_version)
{
	version = server_version;
	if (server_version < 0x4F09)
		oldServer = 1;
	else
		oldServer = 0;

	Make_table();

	if (Paint_init() == -1)
		return -1;

	strlcpy(servername, server, sizeof(servername));

	return 0;
}

int Client_setup(void)
{
	if (Map_init() == -1)
		return -1;

	if (oldServer)
	{
		if (Map_edges() == -1)
			return -1;
		Map_dots();
		Map_blue(0, 0, Setup->x, Setup->y);
		/* kps -remove this, you shouldn't change options this way */
		/* No one wants this on old-style maps anyway, so turn it off.
		 * I do, so turn it on.
		 * This allows people to turn it on in their .xpilotrc for new maps
		 * without affecting old ones. It's still possible to turn in on
		 * from the config menu during play for old maps.
		 * -- But doesn't seem to work anyway if turned on? Well who cares */
		/* kps -removed option */
		/*texturedWalls = false;*/
	}

	RadarHeight = (RadarWidth * Setup->height) / Setup->width;

	if (Init_playing_windows() == -1)
		return -1;

	if (Alloc_msgs() == -1)
		return -1;

	if (Alloc_history() == -1)
		return -1;

	return 0;
}

int Client_fps_request(void)
{
	LIMIT(maxFPS, 1, MAX_SUPPORTED_FPS);
	oldMaxFPS = maxFPS;
	return Send_fps_request(maxFPS);
}

int Check_client_fps(void)
{
	if (oldMaxFPS != maxFPS)
		return Client_fps_request();
	return 0;
}

int Client_power(void)
{
	int i;

	if (Send_power(power) == -1 || Send_power_s(power_s) == -1 || Send_turnspeed(turnspeed) == -1 || Send_turnspeed_s(turnspeed_s) == -1 || Send_turnresistance(turnresistance) == -1 || Send_turnresistance_s(turnresistance_s) == -1)
		return -1;

	if (Check_view_dimensions() == -1)
		return -1;

	for (i = 0; i < NUM_MODBANKS; i++)
	{
		if (Send_modifier_bank(i) == -1)
			return -1;
	}

	return 0;
}

int Client_start(void)
{
	Key_init();

	return 0;
}

void Client_cleanup(void)
{
	int i;

	Pointer_control_set_state(false);
	Platform_specific_cleanup();
	Free_selectionAndHistory();
	Free_msgs();
	if (max_others > 0)
	{
		for (i = 0; i < num_others; i++)
		{
			other_t *other = &Others[i];
			Free_ship_shape(other->ship);
		}
		free(Others);
		num_others = 0;
		max_others = 0;
	}
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
	Map_cleanup();
	Paint_cleanup();
}

int Client_pointer_move(int movement)
{
	if (maxMouseTurnsPS == 0)
		return Send_pointer_move(movement);

	/*
	 * maxMouseTurnsPS is not 0: player wants to limit amount
	 * of pointer move packets sent to server.
	 */
	cumulativeMouseMovement += movement;

	return 0;
}

/*
 * Check if there is any pointer move we need to send to server.
 * Returns how many microseconds to wait in select().
 */
int Client_check_pointer_move_interval(void)
{
	struct timeval now;
	static int last_send_interval_num = -1;
	int interval_num; /* 0 ... maxMouseTurnsPS - 1 */
	int next_interval_start;

	assert(maxMouseTurnsPS > 0);

	/*
	 * Let's see if we've sent any pointer move this interval,
	 * if not and there is something to send, do that now.
	 */
	gettimeofday(&now, NULL);
	interval_num = ((int)now.tv_usec) / mouseMovementInterval;
	if (interval_num != last_send_interval_num && cumulativeMouseMovement != 0)
	{
		Send_pointer_move(cumulativeMouseMovement);
		cumulativeMouseMovement = 0;
		last_send_interval_num = interval_num;
	}

	if (cumulativeMouseMovement != 0)
	{
		/* calculate how long to wait to next interval */
		next_interval_start = (interval_num + 1) * mouseMovementInterval;
		return next_interval_start - (int)now.tv_usec;
	}

	return 1000000;
}

/*
 * Exit the entire client.
 */
void Client_exit(int status)
{
	Net_cleanup();
	Client_cleanup();
	exit(status);
}
