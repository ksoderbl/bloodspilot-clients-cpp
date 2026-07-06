/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * Copyright (C) 2003-2007 by
 *
 *      Kristian Söderblom   kps at users.sourceforge.net
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

/* $Id: messages.c,v 1.37 2007/11/30 17:28:05 kps Exp $ */

#include "sysdeps.h"
#include "bit.h"
#include "const.h"
#include "error.h"
#include "global.h"
#include "other.h"
#include "proto.h"
#include "rules.h"
#include "setup.h"
#include "types.h"
#include "frame.h"

int talkMessagesLifeTime;
int gameMessagesLifeTime;
int messagesToStdout;
int maxMessages;
int charsPerSecond;		/* Message output speed (config) */
bool ignoreColorCodes;

xp_option_t messages_options[] = {
	XP_INT_OPTION("talkMessagesLifeTime",
		      30,
		      0,
		      300,
		      &talkMessagesLifeTime,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Life time of talk and important game messages in seconds.\n"
		      "New messages are painted in messagesColor.\n"
		      "Old messages are painted in oldMessagesColor.\n"),

	XP_INT_OPTION("gameMessagesLifeTime",
		      10,
		      0,
		      300,
		      &gameMessagesLifeTime,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Life time of less important game messages in seconds.\n"
		      "New messages are painted in messagesColor.\n"
		      "Old messages are painted in oldMessagesColor.\n"),

	XP_INT_OPTION("charsPerSecond",
		      100,
		      1,
		      600,
		      &charsPerSecond,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Rate at which messages appear on screen in characters per second.\n"),

	XP_INT_OPTION("maxMessages",
		      8,
		      1,
		      MAX_MSGS,
		      &maxMessages,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "The maximum number of messages to display at the same time.\n"),

	XP_INT_OPTION("messagesToStdout",
		      2,
		      0,
		      3,
		      &messagesToStdout,
		      NULL,
		      XP_OPTFLAG_CONFIG_DEFAULT,
		      "Send messages to standard output.\n"
		      "0: Don't.\n"
		      "1: Only talk messages.\n"
		      "2: Talk messages and kill-/ballstats.\n"
		      "3: All.\n"),

	XP_BOOL_OPTION("ignoreColorCodes",
		       false,
		       &ignoreColorCodes,
		       NULL,
		       XP_OPTFLAG_CONFIG_DEFAULT,
		       "Ignore color codes in text.\n"),
};

void Store_messages_options(void)
{
	STORE_OPTIONS(messages_options);
}

/* provide cut&paste and message history */
static char *HistoryBlock = NULL;

static message_t *TalkMsg[MAX_MSGS];
static message_t *GameMsg[MAX_MSGS];
static message_t *TalkMsg_pending[MAX_MSGS];	/* store incoming messages */
static message_t *GameMsg_pending[MAX_MSGS];	/* while a cut is pending */

message_t *Get_talk_message(int ind)
{
	if (ind < 0 || ind >= MAX_MSGS)
		return NULL;
	return TalkMsg[ind];
}
message_t *Get_game_message(int ind)
{
	if (ind < 0 || ind >= MAX_MSGS)
		return NULL;
	return GameMsg[ind];
}


static inline message_t *Alloc_message(void)
{
	message_t *msg = XCALLOC(message_t, 1);
	if (!msg)
		return NULL;
	return msg;
}

int Alloc_msgs(void)
{
	int i;

	for (i = 0; i < MAX_MSGS; i++) {
		if ((TalkMsg[i] = Alloc_message()) == NULL)
			goto failed;
	}
	for (i = 0; i < MAX_MSGS; i++) {
		if ((TalkMsg_pending[i] = Alloc_message()) == NULL)
			goto failed;
	}
	for (i = 0; i < MAX_MSGS; i++) {
		if ((GameMsg[i] = Alloc_message()) == NULL)
			goto failed;
	}
	for (i = 0; i < MAX_MSGS; i++) {
		if ((GameMsg_pending[i] = Alloc_message()) == NULL)
			goto failed;
	}

	return 0;

 failed:
	error("No memory for messages");
	return -1;
}

void Free_msgs(void)
{
	int i;

	for (i = 0; i < MAX_MSGS; i++) {
		XFREE(TalkMsg[i]);
		XFREE(TalkMsg_pending[i]);
		XFREE(GameMsg[i]);
		XFREE(GameMsg_pending[i]);
	}
}

int Alloc_history(void)
{
	char *hist_ptr;
	int i;

	/* maxLinesInHistory is a runtime constant */
	if ((hist_ptr = (char *) malloc(maxLinesInHistory * TALK_MAX_CHARS)) == NULL) {
		error("No memory for history");
		return -1;
	}
	HistoryBlock = hist_ptr;

	for (i = 0; i < maxLinesInHistory; i++) {
		HistoryMsg[i] = hist_ptr;
		hist_ptr[0] = '\0';
		hist_ptr += TALK_MAX_CHARS;
	}
	return 0;
}

void Free_selectionAndHistory(void)
{
	XFREE(HistoryBlock);
	XFREE(selection.txt);
}

static void Message_set_text(message_t *msg, const char *text)
{
	strlcpy(msg->txt, text, MSG_LEN);
	msg->len = strlen(msg->txt);
}

typedef enum msg_type {
	TalkMessage,		/* Ends with ']'. */
	ImportantGameMessage,	/* Ends with '>'. */
	GameMessage
} msg_type_t;

msg_type_t Message_type(const char *message)
{
	size_t len = strlen(message);

	if (message[len - 1] == ']')
		return TalkMessage;

	if (strncmp(message, " <", 2) == 0
	    && message[len - 1] == '>')
		return ImportantGameMessage;

	return GameMessage;
}

/*
 * add an incoming talk/game message.
 * however, buffer new messages if there is a pending selection.
 * Add_pending_messages() will be called later in Talk_cut_from_messages().
 */
void Add_message(const char *fmt, ...)
{
	int i;
	message_t *msg, **msg_set;
	msg_type_t msgtype;
	msg_bms_t bmsinfo = BmsNone;
	bool is_drawn_talk_message = false;	/* not pending */
	static char message[2 * MSG_LEN];
	size_t len;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(message, sizeof(message), fmt, ap);
	va_end(ap);

	if ((len = strlen(message)) >= MSG_LEN) {
		warn("Add_message: Max len exceeded (%d,\"%s\")", len, message);
		message[MSG_LEN - 1] = '\0';
	}

 	/*
 	 * Store the message in the appropriate message set.
	 * We put the talk messages and the important game messages
	 * (e.g. ball destruction) at top of the screen.
 	 */
	msgtype = Message_type(message);
	if (msgtype == TalkMessage
	    || msgtype == ImportantGameMessage) {
		if (selection.draw.state == SEL_PENDING) {
			/* the buffer for the pending messages */
			msg_set = TalkMsg_pending;
		}
		else {
			msg_set = TalkMsg;
			is_drawn_talk_message = true;
		}
	}
	else {
		if (selection.draw.state == SEL_PENDING)
			msg_set = GameMsg_pending;
		else
			msg_set = GameMsg;
	}

	if (msgtype == GameMessage) {
		/*printf("game message \"%s\"\n", message);*/
		Msg_scan_game_msg(message);
	}

	else if (msgtype == ImportantGameMessage) {
		/*printf("important game message \"%s\"\n", message);*/
		Msg_scan_angle_bracketed_msg(message);
	}

	else if (msgtype == TalkMessage) {
		static char m[MSG_LEN];
		static char sender_name[MAX_CHARS], receiver_name[MAX_CHARS];
		other_t *sender = NULL, *receiver = NULL;
		int receiver_team = -1;
		const char *sender_short_nick = "";

		strlcpy(m, message, sizeof(m));
		/*printf("talk message \"%s\"\n", m);*/
		Partition_talk_message(m,
				       sender_name, sizeof(sender_name),
				       receiver_name, sizeof(receiver_name));
		/*printf("message:    \"%s\"\n", m);
		printf("sender:     \"%s\"\n", sender_name);
		printf("receiver:  \"%s\"\n", receiver_name);*/

		sender = Other_by_name(sender_name, false);

		if (strlen(receiver_name) == 1
		    && strspn(receiver_name, "0123456789") == 1)
		    receiver_team = atoi(receiver_name);
		else
		    receiver = Other_by_name(receiver_name, false);

		if (self && sender
		    && BIT(Setup->mode, TEAM_PLAY)
		    && self->team == sender->team)
			bmsinfo = Msg_do_bms(m);

		/*
		 * Let's not show the message on screen in the old way.
		 * We construct a new message string.
		 */
		strlcpy(message, "", MSG_LEN);

		if (strncmp(sender_name, "*Client", 7) == 0) {
			sender_short_nick = "*Client";
		}
		else if (strncmp(sender_name, "*Server", 7) == 0) {
			sender_short_nick = "*Server";
		}
		/*
		 * Sender can be NULL if we get a talk message for a non existing
		 * player. This can be for example happen because of some fancy
		 * server feature.
		 */
		else if (sender != NULL) {
			Check_abbrev_nick_name(sender, maxCharsInNames);
			sender_short_nick = sender->abbrev_nick_name;
		}

		/*strlcat(message, "^7", MSG_LEN);*/
		strlcat(message, sender_short_nick, MSG_LEN);
		strlcat(message, ": ", MSG_LEN);
		/*strlcat(message, "^0", MSG_LEN);*/

		if (receiver) {
			Check_abbrev_nick_name(receiver, maxCharsInNames);
			strlcat(message, receiver->abbrev_nick_name, MSG_LEN);
			strlcat(message, ": ", MSG_LEN);
		}
		else if (strlen(receiver_name) > 0) { /* team? */
			strlcat(message, receiver_name, MSG_LEN);
			strlcat(message, ": ", MSG_LEN);
		}
		strlcat(message, m, MSG_LEN);
	}

	if (is_drawn_talk_message) {
		/*
		 * keep the emphasizing (`jumping' from talk window to talk messages)
		 */
		if (selection.keep_emphasizing) {
			selection.keep_emphasizing = false;
			selection.talk.state = SEL_NONE;
			selection.draw.state = SEL_EMPHASIZED;
			selection.draw.y1 = -1;
			selection.draw.y2 = -1;
		}		/* talk window emphasized */
	}			/* talk messages */

	msg = msg_set[maxMessages - 1];
	for (i = maxMessages - 1; i > 0; i--)
		msg_set[i] = msg_set[i - 1];

	msg_set[0] = msg;
	Message_set_age(msg, 0.0);
	Message_set_text(msg, message);
	/*printf("Message_set_text set text to '%s'\n", msg->txt);
	  printf("Message_set_text set len  to %d\n", msg->len);*/
	Message_set_bmsinfo(msg, bmsinfo);
	
	/* Clear bms flags for out of date messages. */
	if (bmsinfo != BmsNone)
		Bms_set_state(bmsinfo);

	/*
	 * scroll also the emphasizing
	 */
	if (is_drawn_talk_message && selection.draw.state == SEL_EMPHASIZED) {
		if (selection.draw.y1 == maxMessages - 1) {
			/*
			 * the emphasizing vanishes, as it's `last' line
			 * is `scrolled away'
			 */
			selection.draw.state = SEL_SELECTED;
		}
		else {
			selection.draw.y1++;
			if (selection.draw.y2 == maxMessages - 1)
				selection.draw.x2 = msg_set[selection.draw.y2]->len - 1;
			else
				selection.draw.y2++;
		}
	}

	/* Print messages to standard output.
	 */
	if (messagesToStdout == 3
	    || (messagesToStdout >= 1 && msgtype == TalkMessage))
		printf("%s\n", message);
}

void Add_newbie_message(const char *message)
{
	char msg[MSG_LEN];

	if (!newbie)
		return;

	snprintf(msg, sizeof(msg), "%s [*Newbie help*]", message);

	Add_alert_message(msg, 10.0);
}

/*
 * clear the buffer for the pending messages
 */
static void Delete_pending_messages(void)
{
	int i;

	for (i = 0; i < maxMessages; i++) {
		Message_clear(TalkMsg_pending[i]);
		Message_clear(GameMsg_pending[i]);
	}
}


/*
 * after a pending cut has been completed,
 * add the (buffered) messages which were coming in meanwhile.
 */
void Add_pending_messages(void)
{
	int i;

	/* just through all messages */
	for (i = maxMessages - 1; i >= 0; i--) {
		if (Message_get_length(TalkMsg_pending[i]) > 0)
			Add_message("%s", Message_get_text(TalkMsg_pending[i]));
		if (Message_get_length(GameMsg_pending[i]) > 0)
			Add_message("%s", Message_get_text(GameMsg_pending[i]));
	}
	Delete_pending_messages();
}


void Roundend(void)
{
	roundend = false;
	killratio_kills = 0;
	killratio_deaths = 0;
	Bms_set_state(BmsNone);

	Client_score_table();
}

void Add_roundend_messages(other_t ** order)
{
	static char hackbuf[MSG_LEN];
	static char hackbuf2[MSG_LEN];
	static char kdratio[16];
	static char killsperround[16];
	char *s;
	int i;
	other_t *other;

	Roundend();

	if (killratio_totalkills == 0)
		sprintf(kdratio, "0");
	else if (killratio_totaldeaths == 0)
		sprintf(kdratio, "inf");
	else
		sprintf(kdratio, "%.2f", (double) killratio_totalkills / killratio_totaldeaths);

	if (rounds_played == 0)
		sprintf(killsperround, "0");
	else
		sprintf(killsperround, "%.2f", (double) killratio_totalkills / rounds_played);

	Add_message("Kill ratio - Round: %d/%d Total: %d/%d (%s) "
		    "Rounds played: %d  Avg.kills/round: %s",
		    killratio_kills, killratio_deaths,
		    killratio_totalkills, killratio_totaldeaths, kdratio,
		    rounds_played, killsperround);

	killratio_kills = 0;
	killratio_deaths = 0;

	Add_message("Ballstats - Cash/Repl/Team/Lost: %d/%d/%d/%d",
		    ballsCashed, ballsReplaced, ballsCashedByTeam, ballsLost);

	s = hackbuf;
	s += sprintf(s, "Points - ");
	/*
	 * Scores are nice to see e.g. in cup recordings.
	 */
	for (i = 0; i < num_others; i++) {
		other = order[i];
		if (other->mychar == 'P')
			continue;

		{
			int sc = other->score;
			sprintf(hackbuf2, "%s: %d ", other->nick_name, sc);
			if ((s - hackbuf) + strlen(hackbuf2) > MSG_LEN) {
				Add_message("%s", hackbuf);
				s = hackbuf;
			}
			s += sprintf(s, "%s", hackbuf2);
		}
	}
	Add_message("%s", hackbuf);
}


/*
 * Print all available messages to stdout.
 */
void Print_messages_to_stdout(void)
{
	int i;

	printf("[talk messages]\n");
	for (i = 0; i < maxMessages; i++) {
		if (TalkMsg[i] && Message_get_length(TalkMsg[i]) > 0)
			printf("  %s\n", Message_get_text(TalkMsg[i]));
	}

	printf("[game messages]\n");
	for (i = maxMessages - 1; i >= 0; i--) {
		if (GameMsg[i] && Message_get_length(GameMsg[i]) > 0)
			printf("  %s\n", Message_get_text(GameMsg[i]));
	}
	printf("\n");
}

