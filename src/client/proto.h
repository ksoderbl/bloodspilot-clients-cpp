/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell
 *      Ken Ronny Schouten
 *      Bert Gijsbers
 *      Dick Balaska
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
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "types.h"
#include "other.h"
#include "option.h"

/*
 * clientcommand.c
 */
void executeCommand(const char *talk_str);
void crippleTalk(char *msg);

/*
 * colors.c
 */
void List_visuals(void);
int Colors_init(void);
int Colors_init_block_bitmaps(void);
void Colors_free_block_bitmaps(void);
void Colors_cleanup(void);
void Colors_debug(void);

/*
 * default.c
 */
void Old_parse_options(int *argcp, char **argvp, char *realName, int *port,
					   int *my_team, int *text, int *list,
					   int *join, char *nickName, char *dispName, char *hostName, char *shut_msg);
void defaultCleanup(void); /* memory cleanup */
void Get_xpilotrc_file(char *, unsigned);
char *Old_get_keyHelpString(keys_t key);
const char *Old_get_keyResourceString(keys_t key);
void Make_sure_we_have_a_nick(void);

/*
 * somewhere
 */
int Bitmap_add(const char *filename, int count, bool scalable);
void Pointer_control_newbie_message(void);

/*
 * Platform specific code needs to implement these.
 */
void Platform_specific_pointer_control_set_state(bool on);
void Platform_specific_talk_set_state(bool on);
void Record_toggle(void);
void Toggle_fullscreen(void);
void Toggle_radar_and_scorelist(void);

/*
 * event.c
 */
void Pointer_control_set_state(bool on);
void Talk_set_state(bool on, bool team_talk);

void Pointer_button_pressed(int button);
void Pointer_button_released(int button);
void Keyboard_button_pressed(xp_keysym_t ks);
void Keyboard_button_released(xp_keysym_t ks);

int Key_init(void);
int Key_update(void);
void Key_clear_counts(void);
bool Key_press(keys_t key);
bool Key_release(keys_t key);
void Set_auto_shield(bool on);
void Set_toggle_shield(bool on);

void Scale_dashes(void);

bool Key_press(keys_t key);
bool Key_release(keys_t key);

/*
 * math.c
 */
int ON(const char *optval);
int OFF(const char *optval);
int mod(int x, int y);
double findDir(double x, double y);
double rfrac(void);
void Make_table(void);

/*
 * messages
 */
void Add_message(const char *fmt, ...);
void Add_pending_messages(void);
bool Bms_test_state(msg_bms_t bms);
void Bms_set_state(msg_bms_t bms);
int Alloc_msgs(void);
void Free_msgs(void);
int Alloc_history(void);
void Free_selectionAndHistory(void);
void Add_newbie_message(const char *message);
void Add_roundend_messages(other_t **order);
void Add_alert_message(const char *message, double timeout);
void Clear_alert_messages(void);
void Print_messages_to_stdout(void);

void Client_exit(int status);
void Client_score_table(void);
void Client_set_eyes_id(int id);
void Store_messages_options(void);

message_t *Get_talk_message(int ind);
message_t *Get_game_message(int ind);

static inline void Message_clear(message_t *msg)
{
	msg->txt[0] = '\0';
	msg->len = 0;
	msg->age = 0.0;
}

static inline msg_bms_t Message_get_bmsinfo(message_t *msg)
{
	return msg->bmsinfo;
}
static inline void Message_set_bmsinfo(message_t *msg, msg_bms_t type)
{
	msg->bmsinfo = type;
}
static inline void Message_set_age(message_t *msg, double seconds)
{
	msg->age = seconds;
}
static inline double Message_get_age(message_t *msg)
{
	return msg->age;
}
static inline void Message_add_age(message_t *msg, double seconds)
{
	msg->age += seconds;
}
#if 0
/* No support for color codes in this one. */
static inline void Message_set_text(message_t *msg, const char *text)
{
	strlcpy(msg->txt, text, MSG_LEN);
	msg->len = strlen(msg->txt);
}
#endif
static inline int Message_get_length(message_t *msg)
{
	return (int)msg->len;
}
static inline char *Message_get_text(message_t *msg)
{
	return msg->txt;
}

static inline char *Message_get_text_pointer(message_t *msg, size_t ind)
{
	if (ind > sizeof(msg->txt))
		return NULL;
	if (ind > msg->len)
		return NULL;
	return &msg->txt[ind];
}

void Msg_scan_game_msg(const char *message);
void Msg_scan_angle_bracketed_msg(const char *message);

msg_bms_t Msg_do_bms(const char *message);
extern void Partition_talk_message(char *message,
								   char *sender,
								   size_t sender_size,
								   char *reciever,
								   size_t reciever_size);

/*
 * join.c
 */
int Join(Connect_param_t *conpar);

/*
 * paint
 */
int Paint_init(void);
void Paint_cleanup(void);
void Paint_shots(void);
void Paint_ships(void);
void Paint_vcannon(void);
void Paint_vfuel(void);
void Paint_vbase(void);
void Paint_vdecor(void);
void Paint_objects(void);
void Paint_world(void);
void Paint_score_table(void);
void Paint_score_list(void);
void Paint_score_entry(int entry_num, other_t *other, bool is_team);
void Paint_score_start(void);
void Paint_score_objects(void);
void Paint_meters(void);
void Paint_HUD(void);
int Get_message(int *pos, char *message, int req_length, int key);
void Paint_messages(void);
void Paint_recording(void);
void Paint_HUD_values(void);
void Paint_frame(void);
void Paint_frame_start(void);
int Team_color(int);
int Player_state_color(other_t *other);
int Life_color(other_t *other);
int Life_color_by_life(int life);
void Play_beep(void);
int Check_view_dimensions(void);
void Store_hud_options(void);

/*
 * paint
 */
void Paint_radar(void);
void Paint_sliding_radar(void);
void Paint_world_radar(void);
void Radar_show_target(int x, int y);
void Radar_hide_target(int x, int y);
void Paint_hudradar(void);
void Paint_hudradar_walls(void);
void Store_paintradar_options(void);
void Store_hudradar_options(void);

void Store_paintmessages_options(void);
void Store_paintscorelist_options(void);

/*
 * paintdata.c
 */
void paintdataCleanup(void); /* memory cleanup */

/*
 * paintobjects.c
 */
int Init_wreckage(void);
int Init_asteroids(void);

/*
 * query.c
 */
int Query_all(sock_t *sockfd, int port, char *msg, size_t msglen);

/*
 * randommt.c
 */
void seedMT(unsigned int seed);
unsigned int reloadMT(void);
unsigned int randomMT(void);

/*
 * record.c
 */
long Record_size(void);
void Record_toggle(void);
void Record_init(const char *filename);
void Record_cleanup(void);
void Store_record_options(void);

/*
 * talk.c
 */
int Talk_macro(int ind);
void Store_talk_macro_options(void);

/*
 * textinterface.c
 */
int Connect_to_server(int auto_connect, int list_servers,
					  int auto_shutdown, char *shutdown_reason, Connect_param_t *conpar);
int Contact_servers(int count, char **servers,
					int auto_connect, int list_servers,
					int auto_shutdown, char *shutdown_message,
					int find_max, int *num_found,
					char **server_addresses, char **server_names, unsigned *server_versions,
					Connect_param_t *conpar);

/*
 * Somewhere.
 */
void Output_copyright_notice(void);
const char *Program_name(void);
void Raise_window(void);
void Reset_shields(void);

int Fuel_by_pos(int x, int y);
int Target_alive(int x, int y, int *damage);
int Target_by_index(int ind, int *xp, int *yp, int *dead_time, int *damage);
int Handle_fuel(int ind, int fuel);
int Cannon_dead_time_by_pos(int x, int y, int *dot);
int Handle_cannon(int ind, int dead_time);
int Handle_target(int num, int dead_time, int damage);
int Base_info_by_pos(int x, int y, int *id, int *team);
int Handle_base(int id, int ind);
int Check_pos_by_index(int ind, int *xp, int *yp);
int Check_index_by_pos(int x, int y);
homebase_t *Homebase_by_id(int id);
homebase_t *Homebase_by_pos(int x, int y);
other_t *Other_by_id(int id);
other_t *Other_by_name(const char *name, bool show_error_msg);
shipshape_t *Ship_by_id(int id);
int Handle_leave(int id);
int Handle_player(int id, int team, int mychar,
				  char *nick_name, char *user_name, char *host_name, char *shape, int myself);
int Handle_team(int id, int pl_team);
int Handle_score(int id, int score, int life, int mychar, int alliance);
int Handle_score_object(int score, int x, int y, char *msg);
int Handle_team_score(int team, int score);
int Handle_timing(int id, int check, int round, long loops);
int Handle_war(int robot_id, int killer_id);
int Handle_seek(int programmer_id, int robot_id, int sought_id);
int Handle_polystyle(int polyind, int newstyle);

void Map_dots(void);
void Map_blue(int startx, int starty, int width, int height);
bool Using_score_decimals(void);
int Client_init(char *server, unsigned server_version);
int Client_setup(void);
void Client_cleanup(void);
int Client_start(void);
int Client_fps_request(void);
int Client_power(void);
int Client_pointer_move(int movement);
int Client_check_pointer_move_interval(void);
int Client_fd(void);
int Client_input(int);
void Client_flush(void);
void Client_sync(void);
int Client_wrap_mode(void);
int Check_client_fps(void);
void Client_set_eyes_id(int id);

#ifdef SOUND
extern void audioEvents();
#endif

int Init_playing_windows(void);
int Key_init(void);
int Key_update(void);
void Raise_window(void);
void Reset_shields(void);
void Platform_specific_cleanup(void);
void Platform_specific_cleanup(void);
void Roundend(void);

#ifdef _WINDOWS
void MarkPlayersForRedraw(void);
#endif

int Check_client_fps(void);

extern int Handle_motd(long off, char *buf, int len, long filesize);
extern void aboutCleanup(void);

#ifdef _WINDOWS
extern void Motd_destroy(void);
extern void Keys_destroy(void);
#endif

extern void Colors_init_style_colors(void);

extern void Store_default_options(void);
extern void defaultCleanup(void); /* memory cleanup */

extern bool Set_scaleFactor(xp_option_t *opt, double val);
extern bool Set_altScaleFactor(xp_option_t *opt, double val);

#ifdef _WINDOWS
extern char *Get_xpilotini_file(int level);
#endif

extern void Store_key_options(void);

extern int Join(Connect_param_t *conpar);
extern void xpilotShutdown(void);

extern int Mapdata_setup(const char *);

extern int metaclient(int, char **);

extern void paintdataCleanup(void); /* memory cleanup */

extern int Init_wreckage(void);
extern int Init_asteroids(void);

extern int Connect_to_server(int auto_connect, int list_servers,
							 int auto_shutdown, char *shutdown_reason, Connect_param_t *conpar);
extern int Contact_servers(int count, char **servers,
						   int auto_connect, int list_servers,
						   int auto_shutdown, char *shutdown_message,
						   int find_max, int *num_found,
						   char **server_addresses, char **server_names,
						   unsigned *server_versions, Connect_param_t *conpar);

extern int micro_delay(unsigned usec);

extern int Welcome_screen(Connect_param_t *conpar);

extern void Widget_cleanup(void);
