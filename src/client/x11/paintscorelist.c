/*
 * BloodsPilot, a multiplayer space war game.
 *
 * Copyright (C) 1991-2002 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * Copyright (C) 2004-2007 by
 *
 *      Kristian Söderblom   <kps@users.sourceforge.net>
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

/* $Id: paintscorelist.c,v 1.15 2007/11/28 18:26:58 kps Exp $ */

#include "xpclient_x11.h"
#include "frame.h"
#include "client.h"
#include "setup.h"
#include "rules.h"
#include "error.h"
#include "bit.h"

#define TEAM_HACK_ID (-2)

static int scoreListColor;	/* Color index for map score list */


xp_option_t paintscorelist_options[] = {

	COLOR_INDEX_OPTION("scoreListColor",
			   GRAY,
			   &scoreListColor,
			   "Which color number to use for drawing score list.\n"),

};


void Store_paintscorelist_options(void)
{
	STORE_OPTIONS(paintscorelist_options);
}


#define SCORE_BORDER		6

#if 0
static void Paint_score_background(int thisLine)
{
	XClearWindow(dpy, playersWindow);
}
#endif

typedef struct {
	char str[MSG_LEN];
	int color;
	int is_team;
} scorebuf_t;

static scorebuf_t scorebuf[50];


static void Paint_score_string(int i, const char *string, int color, int is_team)
{
	int x, y, len;
	int SPACING = scoreListFont->ascent + scoreListFont->descent + 1;
	int MSPACING = messageFont->ascent + messageFont->descent + 1;
	bool colorize = true;

	if (color == NO_COLOR)
		return;

	len = strlen(string);

	Set_gc_fg_color(dpy, scoreListGC, color);

	x = 10;
	y = SPACING * i + scoreListFont->ascent;
	y += 45 + maxMessages * MSPACING;

	if (showScoreListMode < 2)
		colorize = false;

	ColorDrawString(dpy, drawPixmap, scoreListGC, scoreListFont,
			x, y, string, len,
			color, shadowColor, MAX_COLORS + 0 /* color code black (^0) */,
			colorize, true);
}

void Paint_score_list(void)
{
	int i;

	if (showScoreListMode == 0)
		return;

	for (i = 0; i < NELEM(scorebuf); i++) {
		if (scorebuf[i].str[0] != '\0')
			Paint_score_string(i, scorebuf[i].str, scorebuf[i].color, scorebuf[i].is_team);
	}
}


void Paint_score_start(void)
{
	int i;

	for (i = 0; i < NELEM(scorebuf); i++)
		scorebuf[i].str[0] = '\0';

#if 0
	char headingStr[MSG_LEN];
	static int thisLine;

	thisLine = SCORE_BORDER + scoreListFont->ascent;

	if (showRealName) {
		strlcpy(headingStr, "NICK=USER@HOST", sizeof(headingStr));
	}
	else {
		strlcpy(headingStr, "  ", sizeof(headingStr));
		if (BIT(Setup->mode, TIMING)) {
			if (version >= 0x3261) {
				strcat(headingStr, "LAP ");
			}
		}
		if (BIT(Setup->mode, TEAM_PLAY)) {
			strlcpy(headingStr, " TM ", sizeof(headingStr));
		}
		else {
			strlcpy(headingStr, " AL ", sizeof(headingStr));
		}
		strcat(headingStr, "  SCORE  ");
		if (BIT(Setup->mode, LIMITED_LIVES)) {
			strlcat(headingStr, "LIFE", sizeof(headingStr));
		}
		strlcat(headingStr, " NAME", sizeof(headingStr));
	}
	Paint_score_background(thisLine);

	ShadowDrawString(dpy, playersWindow, scoreListGC,
			 SCORE_BORDER, thisLine, headingStr, Get_color_pixel(WHITE),
			 Get_color_pixel(BLACK));

	gcv.line_style = LineSolid;
	XChangeGC(dpy, scoreListGC, GCLineStyle, &gcv);
	XDrawLine(dpy, playersWindow, scoreListGC, SCORE_BORDER, thisLine,
		  players_width - SCORE_BORDER, thisLine);

	gcv.line_style = LineOnOffDash;
	XChangeGC(dpy, scoreListGC, GCLineStyle, &gcv);
#endif
}


static void Set_score_entry2(int entry_num, const char *entry)
{
	if (entry_num >= NELEM(scorebuf))
		return;

	sprintf(scorebuf[entry_num].str, "%s", entry);
	scorebuf[entry_num].color = scoreListColor;
	scorebuf[entry_num].is_team = false;
#if 0
	printf("%s\n", scorebuf[entry_num].str);
#endif
}

void Paint_score_entry(int entry_num, other_t * other, bool is_team)
{
}

static void Set_score_entry(int entry_num, other_t * other, bool best)
{
	static char buf[MSG_LEN];
	char *s;
	int color;
	other_t *war;
	bool is_team;

	if (entry_num >= NELEM(scorebuf)
	    || scoreListColor == NO_COLOR
	    || showScoreListMode == 0)
		return;

	s = (char *) &scorebuf[entry_num].str;

	sprintf(buf, "%c ", other->mychar);
	strlcat(s, buf, MSG_LEN);

	sprintf(buf, "%6d ", other->score);
	strlcat(s, buf, MSG_LEN);

	if (BIT(Setup->mode, LIMITED_LIVES)) {
		sprintf(buf, "%2d ", other->life);
		strlcat(s, buf, MSG_LEN);
	}

	is_team = (other && other->id == TEAM_HACK_ID);
	color = Player_state_color(other);
	if (color == NO_COLOR)
		color = scoreListColor;
	if (color == WHITE)
		color = LIGHT_GRAY;
	if (is_team)
		color = scoreListColor;

	scorebuf[entry_num].is_team = is_team;

	if (is_team || showScoreListMode >= 2)
		sprintf(buf, " %-s", other->nick_name);
	else {
		Check_abbrev_nick_name(other, maxCharsInNames);
		sprintf(buf, " %-s", other->abbrev_nick_name);
	}
	strlcat(s, buf, MSG_LEN);

	war = Other_by_id(other->war_id);
	if (war) {
		Check_abbrev_nick_name(war, maxCharsInNames);
		snprintf(buf, 6, " (%s", war->abbrev_nick_name);
		strlcat(s, buf, MSG_LEN);
		strlcat(s, ")", MSG_LEN);
	}

	if (!is_team && showScoreListMode >= 2) {
		sprintf(buf, " / %s@%s", other->user_name, other->host_name);
		strlcat(s, buf, MSG_LEN);
	}

	scorebuf[entry_num].color = color;




#if 0
	printf("%s\n", s);
#endif
#if 0
	static char raceStr[8], teamStr[4], lifeStr[8], label[MSG_LEN];
	static int lineSpacing = -1, firstLine;
	int thisLine;
	char scoreStr[16];

	/*
	 * First time we're here, set up miscellaneous strings for
	 * efficiency and calculate some other constants.
	 */
	if (lineSpacing == -1) {
		memset(raceStr, '\0', sizeof raceStr);
		memset(teamStr, '\0', sizeof teamStr);
		memset(lifeStr, '\0', sizeof lifeStr);
		teamStr[1] = ' ';
		raceStr[2] = ' ';

		lineSpacing = scoreListFont->ascent + scoreListFont->descent + 3;
		firstLine = 2 * SCORE_BORDER + scoreListFont->ascent + lineSpacing;
	}
	thisLine = firstLine + lineSpacing * entry_num;

	/*
	 * Setup the status line
	 */
	if (showRealName) {
		sprintf(label, "%s=%s@%s", other->nick_name, other->user_name, other->host_name);
	}
	else {
		other_t *war = Other_by_id(other->war_id);

		if (BIT(Setup->mode, TIMING)) {
			raceStr[0] = ' ';
			raceStr[1] = ' ';
			if (version >= 0x3261) {
				if ((other->mychar == ' ' || other->mychar == 'R')
				    && other->round + other->check > 0) {
					if (other->round > 99) {
						sprintf(raceStr, "%3d", other->round);
					}
					else {
						sprintf(raceStr, "%d.%c", other->round,
							other->check + 'a');
					}
				}
			}
		}
		if (BIT(Setup->mode, TEAM_PLAY)) {
			teamStr[0] = other->team + '0';
		}
		else {
			sprintf(teamStr, "%c", other->alliance);
		}

		if (BIT(Setup->mode, LIMITED_LIVES))
			sprintf(lifeStr, " %4d", other->life);

		sprintf(scoreStr, "%7d", other->score);
		sprintf(label, "%c %s%s%s%s  %s", other->mychar, raceStr, teamStr, scoreStr,
			lifeStr, other->nick_name);
		if (war) {
			if (strlen(label) + strlen(war->nick_name) + 5 < sizeof(label)) {
				sprintf(label + strlen(label), " (%s)", war->nick_name);
			}
		}
	}

	/*
	 * Draw the line
	 */
	if (other->mychar == 'D' || other->mychar == 'P' || other->mychar == 'W') {
		XSetForeground(dpy, scoreListGC, Get_color_pixel(WHITE));
		XDrawString(dpy, playersWindow, scoreListGC, SCORE_BORDER, thisLine, label,
			    strlen(label));
	}
	else {
		ShadowDrawString(dpy, playersWindow, scoreListGC,
				 SCORE_BORDER, thisLine, label, Get_color_pixel(WHITE),
				 Get_color_pixel(BLACK));
	}

	/*
	 * Underline the best player
	 */
	if (best) {
		XDrawLine(dpy, playersWindow, scoreListGC,
			  SCORE_BORDER, thisLine, players_width - SCORE_BORDER, thisLine);
	}
#endif
}


void Client_score_table(void)
{
	/*static char hackbuf[MSG_LEN]; */
	struct team_score {
		int score;
		int life;
		int playing;
		int has_players;
	};
	struct team_score team[MAX_TEAMS];
	other_t *other, **order;
	int i, j, k, l, best = -1, pos = 0;
	DFLOAT ratio, best_ratio = -1e7;
	bool playing = (self && !strchr("PW", self->mychar));

	/*printf("--- client score table\n"); */

#if 0
	if (scoresChanged == 0) {
		return;
	}

	if (players_exposed == false) {
		return;
	}
#endif

	if (num_others < 1) {
		Paint_score_start();
		scoresChanged = 0;
		return;
	}

	if ((order = (other_t **) malloc(num_others * sizeof(other_t *))) == NULL) {
		error("No memory for score");
		return;
	}
	if (BIT(Setup->mode, TEAM_PLAY | TIMING) == TEAM_PLAY) {
		memset(&team[0], 0, sizeof team);
	}
	for (i = 0; i < num_others; i++) {
		other = &Others[i];
		if (BIT(Setup->mode, TIMING)) {
			/*
			 * Sort the score table on position in race.
			 * Put paused and waiting players last as well as tanks.
			 */
			if (strchr("PTW", other->mychar)) {
				j = i;
			}
			else {
				for (j = 0; j < i; j++) {
					if (order[j]->timing < other->timing) {
						break;
					}
					if (strchr("PTW", order[j]->mychar)) {
						break;
					}
					if (order[j]->timing == other->timing) {
						if (order[j]->timing_loops > other->timing_loops) {
							break;
						}
					}
				}
			}
		}
		else {
			if (BIT(Setup->mode, LIMITED_LIVES)) {
				ratio = other->score;
			}
			else {
				ratio = other->score / (other->life + 1);
			}
			if (best == -1 || ratio > best_ratio) {
				best_ratio = ratio;
				best = i;
			}
			for (j = 0; j < i; j++) {
				if (order[j]->score < other->score) {
					break;
				}
			}
		}
		for (k = i; k > j; k--) {
			order[k] = order[k - 1];
		}
		order[j] = other;

		if (BIT(Setup->mode, TEAM_PLAY | TIMING) == TEAM_PLAY) {
			switch (other->mychar) {
			case 'P':
			case 'W':
			case 'T':
				team[other->team].has_players++;
				break;
			case ' ':
			case 'R':
				if (BIT(Setup->mode, LIMITED_LIVES)) {
					team[other->team].life += other->life + 1;
				}
				else {
					team[other->team].life += other->life;
				}
			 /*FALLTHROUGH*/ default:
				team[other->team].has_players++;
				team[other->team].playing++;
				team[other->team].score += other->score;
				break;
			}
		}
	}
	Paint_score_start();
	if (BIT(Setup->mode, TIMING)) {
		best = order[0] - Others;
	}

	if (!BIT(Setup->mode, TEAM_PLAY)) {
		for (i = 0; i < num_others; i++) {
			other = order[i];
			j = other - Others;
			Set_score_entry(i, other, (j == best) ? true : false);
		}
	}
	if (BIT(Setup->mode, TEAM_PLAY | TIMING) == TEAM_PLAY) {
		other_t tmp;

		pos = 1 /*num_others *//*+ 1 */ ;

		num_playing_teams = 0;
		for (i = 0; i < MAX_TEAMS; i++) {
			if (team[i].playing) {
				num_playing_teams++;
			}
		}

		for (i = 1; i <= MAX_TEAMS; i++) {
			int ti = i;

			if (ti == MAX_TEAMS)
				ti = 0;

			if (!team[ti].has_players)
				continue;

			tmp.id = TEAM_HACK_ID;
			tmp.team = ti;
			tmp.war_id = -1;
			tmp.max_chars_in_names = -1;
			tmp.ship = NULL;
			if (team[ti].playing > 0)
				sprintf(tmp.nick_name, "TEAM %d", tmp.team);
			else
				sprintf(tmp.nick_name, "COWARD TEAM %d", tmp.team);
			strcpy(tmp.user_name, "");
			strcpy(tmp.host_name, "");
			tmp.mychar = ' ';
			tmp.score = team[ti].score;
			tmp.life = team[ti].life;
			Set_score_entry(pos++, &tmp, false);

			for (l = 0; l < num_others; l++) {
				other = order[l];
				j = other - Others;
				if (other->team == tmp.team)
					Set_score_entry(pos++, other, (j == best) ? true : false);
			}

			Set_score_entry2(pos++, " ");
		}
	}
	else
		pos = num_others /*+ 1 */ ;



	if (playing && roundend && messagesToStdout >= 2) {
		static char kdratio[16];

		if (rounds_played > 0) {
			printf("Round %d:\n", rounds_played);
		}

		if (killratio_totalkills == 0)
			sprintf(kdratio, "0");
		else if (killratio_totaldeaths == 0)
			sprintf(kdratio, "inf");
		else
			sprintf(kdratio, "%.3f",
				(double) killratio_totalkills / killratio_totaldeaths);
		printf("Total K/D: %d/%d (%s)\n", killratio_totalkills, killratio_totaldeaths,
		       kdratio);

		if (rounds_played > 0) {
			if (killratio_kills == 0)
				sprintf(kdratio, "0");
			else if (killratio_deaths == 0)
				sprintf(kdratio, "inf");
			else
				sprintf(kdratio, "%.2f",
					(double) killratio_kills / killratio_deaths);

			printf("Round K/D: %d/%d (%s)\n", killratio_kills, killratio_deaths,
			       kdratio);
			printf("Avg.kills/round: %.2f\n",
			       (double) killratio_totalkills / rounds_played);
		}

		if (ballsCashed > 0 || ballsCashedByTeam > 0 || ballsLost > 0) {
			printf("Balls Cash/Team/Lost: %d/%d/%d\n", ballsCashed,	/*ballsReplaced, */
			       ballsCashedByTeam, ballsLost);
		}
		printf("----------------------------------------\n");
	}

#if 0
	if (roundend)
		Add_roundend_messages(order);
#else
	if (roundend)
		Roundend();
#endif

	free(order);

	scoresChanged = 0;

#if 0
	printf("--- client score table ends\n");
#endif
}
