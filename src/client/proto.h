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

/* $Id: proto.h,v 1.39 2007/11/02 00:06:16 kps Exp $ */

#ifndef	PROTO_H
#define	PROTO_H

#ifndef TYPES_H
#include "types.h"
#endif
#ifndef OTHER_H
#include "other.h"
#endif
#ifndef OPTION_H
#include "option.h"
#endif

/*
 * checknames.c
 */
#define NAME_OK		1
#define NAME_ERROR	0

int Check_user_name(char *name);
void Fix_user_name(char *name);
int Check_nick_name(char *name);
void Fix_nick_name(char *name);
int Check_host_name(char *name);
void Fix_host_name(char *name);
int Check_disp_name(char *name);
void Fix_disp_name(char *name);

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
 * color-codes.c
 */
int Index_by_color_code(int ascii_char);
char Color_code_by_index(int ind);
int RGB_by_index(int ind);
int XPilot_color(int ind);

/*
 * configure.c
 */
#define CONFIG_NONE    0
#define CONFIG_DEFAULT 1
#define CONFIG_COLORS  2
#define CONFIG_KEYS    3

void Config_init(void);
int Config(bool, int);
void Config_redraw(void);
void Config_resize(void);
void Config_destroy(void);
void Config_key_event(char *ksname);

/*
 * default.c
 */
void Old_parse_options(int *argcp, char **argvp, char *realName, int *port,
		       int *my_team, int *text, int *list,
		       int *join, char *nickName, char *dispName, char *hostName, char *shut_msg);
void defaultCleanup(void);	/* memory cleanup */
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
 * gui map
 */
void Gui_paint_cannon(int x, int y, int type);
void Gui_paint_fuel(int x, int y, int fuel);
void Gui_paint_base(int x, int y, int xi, int yi, int type);
void Gui_paint_decor(int x, int y, int xi, int yi, int type, bool last, bool more_y);

void Gui_paint_border(int x, int y, int xi, int yi);
void Gui_paint_visible_border(int x, int y, int xi, int yi);
void Gui_paint_hudradar_limit(int x, int y, int xi, int yi);

void Gui_paint_setup_check(int x, int y, int xi, int yi);
void Gui_paint_setup_check_ng(int x, int y, bool isNext);
void Gui_paint_setup_acwise_grav(int x, int y);
void Gui_paint_setup_cwise_grav(int x, int y);
void Gui_paint_setup_pos_grav(int x, int y);
void Gui_paint_setup_neg_grav(int x, int y);
void Gui_paint_setup_up_grav(int x, int y);
void Gui_paint_setup_down_grav(int x, int y);
void Gui_paint_setup_right_grav(int x, int y);
void Gui_paint_setup_left_grav(int x, int y);
void Gui_paint_setup_worm(int x, int y);
void Gui_paint_setup_item_concentrator(int x, int y);
void Gui_paint_setup_asteroid_concentrator(int x, int y);
void Gui_paint_decor_dot(int x, int y, int size);
void Gui_paint_setup_target(int x, int y, int target, int damage, bool own);
void Gui_paint_setup_treasure(int x, int y, int treasure, bool own);
void Gui_paint_walls(int x, int y, int type, int xi, int yi);
void Gui_paint_filled_slice(int bl, int tl, int tr, int br, int y);
void Gui_paint_polygon(int i, int xoff, int yoff);
void Store_guimap_options(void);

/*
 * gui objects
 */
void Paint_items(void);
void Gui_paint_item_object(int type, int x, int y);
void Gui_paint_ball(int x, int y, int style);
void Gui_paint_ball_connector(int xa, int ya, int xb, int yb);
void Gui_paint_mine(int x, int y, int teammine, char *name);
void Gui_paint_spark(int color, int x, int y);
void Gui_paint_wreck(int x, int y, bool deadly, int wtype, int rot, int size);
void Gui_paint_asteroids_begin(void);
void Gui_paint_asteroids_end(void);
void Gui_paint_asteroid(int x, int y, int type, int rot, int size);
void Gui_paint_fastshot(int color, int x, int y);
void Gui_paint_teamshot(int color, int x, int y);
void Gui_paint_missiles_begin(void);
void Gui_paint_missiles_end(void);
void Gui_paint_missile(int x, int y, int len, int dir);
void Gui_paint_lasers_begin(void);
void Gui_paint_lasers_end(void);
void Gui_paint_laser(int color, int xa, int ya, int len, int dir);
void Gui_paint_paused(int x, int y, int count);
void Gui_paint_appearing(int x, int y, int id, int count);
void Gui_paint_ecm(int x, int y, int size);
void Gui_paint_refuel(int xa, int ya, int xb, int yb);
void Gui_paint_connector(int xa, int ya, int xb, int yb, int tractor);
void Gui_paint_transporter(int xa, int ya, int xb, int yb);
void Gui_paint_all_connectors_begin(void);
void Gui_paint_refuel_begin(void);
void Gui_paint_incomplete_ball_connectors_begin(void);
void Gui_paint_transporters_begin(void);
void Gui_paint_ships_begin(void);
void Gui_paint_ships_end(void);
void Gui_paint_ship(int x, int y, float angle, int id, int cloak, int phased, int shield,
		    int deflector, int eshield);
int Gui_calculate_ship_color(other_t * other);
void Store_guiobject_options(void);

/*
 * math.c
 */
int ON(const char *optval);
int OFF(const char *optval);
int mod(int x, int y);
DFLOAT findDir(DFLOAT x, DFLOAT y);
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
void Add_roundend_messages(other_t ** order);
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
extern
void Partition_talk_message(char *message,
			    char *sender,
			    size_t sender_size,
			    char *reciever,
			    size_t reciever_size);

/*
 * net client
 */
int Net_setup(void);
int Net_verify(Connect_param_t *conpar);
int Net_init(char *server, int port);
void Net_cleanup(void);
void Net_key_change(void);
int Net_flush(void);
int Net_fd(void);
int Net_start(void);
void Net_init_measurement(void);
void Net_init_lag_measurement(void);
int Net_input(void);
/* void Net_measurement(long loop, int status);*/
int Receive_start(void);
int Receive_end(void);
int Receive_message(void);
int Receive_self(void);
int Receive_self_items(void);
int Receive_modifiers(void);
int Receive_refuel(void);
int Receive_connector(void);
int Receive_laser(void);
int Receive_missile(void);
int Receive_ball(void);
int Receive_ship(void);
int Receive_mine(void);
int Receive_item(void);
int Receive_destruct(void);
int Receive_shutdown(void);
int Receive_thrusttime(void);
int Receive_shieldtime(void);
int Receive_phasingtime(void);
int Receive_rounddelay(void);
int Receive_debris(void);
int Receive_wreckage(void);
int Receive_asteroid(void);
int Receive_wormhole(void);
int Receive_fastshot(void);
int Receive_ecm(void);
int Receive_trans(void);
int Receive_paused(void);
int Receive_radar(void);
int Receive_fastradar(void);
int Receive_damaged(void);
int Receive_leave(void);
int Receive_war(void);
int Receive_seek(void);
int Receive_player(void);
int Receive_score(void);
int Receive_score_object(void);
int Receive_team_score(void);
int Receive_timing(void);
int Receive_fuel(void);
int Receive_cannon(void);
int Receive_target(void);
int Receive_base(void);
int Receive_reliable(void);
int Receive_quit(void);
int Receive_string(void);
int Receive_reply(int *replyto, int *result);
int Receive_team(void);		/* ng */
int Receive_appearing(void);	/* ng */
int Receive_polystyle(void);	/* ng */
int Send_ack(long rel_loops);
int Send_keyboard(uint8_t *);
int Send_shape(char *);
int Send_power(double pwr);
int Send_power_s(double pwr_s);
int Send_turnspeed(double turnspd);
int Send_turnspeed_s(double turnspd_s);
int Send_turnresistance(double turnres);
int Send_turnresistance_s(double turnres_s);
int Send_pointer_move(int movement);
int Receive_audio(void);
int Receive_talk_ack(void);
int Send_talk(void);
int Send_display(void);
int Send_display_ng(int width, int height, int sparks, int spark_colors);
int Send_modifier_bank(int);
int Net_talk(const char *str, bool team_talk);
int Net_ask_for_motd(long offset, long maxlen);
int Receive_time_left(void);
int Receive_eyes(void);
int Receive_motd(void);
int Receive_magic(void);
int Send_audio_request(int onoff);
int Send_fps_request(int fps);
int Receive_loseitem(void);


/*
 * join.c
 */
int Join(Connect_param_t * conpar);

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
void Paint_score_entry(int entry_num, other_t * other, bool is_team);
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
int Player_state_color(other_t * other);
int Life_color(other_t * other);
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
void paintdataCleanup(void);	/* memory cleanup */

/*
 * paintobjects.c
 */
int Init_wreckage(void);
int Init_asteroids(void);


/*
 * query.c
 */
int Query_all(sock_t * sockfd, int port, char *msg, size_t msglen);


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
		      int auto_shutdown, char *shutdown_reason, Connect_param_t * conpar);
int Contact_servers(int count, char **servers,
		    int auto_connect, int list_servers,
		    int auto_shutdown, char *shutdown_message,
		    int find_max, int *num_found,
		    char **server_addresses, char **server_names, unsigned *server_versions,
		    Connect_param_t * conpar);

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

#ifdef	SOUND
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
extern void defaultCleanup(void);	/* memory cleanup */

extern bool Set_scaleFactor(xp_option_t * opt, double val);
extern bool Set_altScaleFactor(xp_option_t * opt, double val);

#ifdef _WINDOWS
extern char *Get_xpilotini_file(int level);
#endif


extern void Store_key_options(void);

extern int Join(Connect_param_t * conpar);
extern void xpilotShutdown(void);

extern int Mapdata_setup(const char *);


extern int metaclient(int, char **);


extern void paintdataCleanup(void);	/* memory cleanup */


extern int Init_wreckage(void);
extern int Init_asteroids(void);


extern int Connect_to_server(int auto_connect, int list_servers,
			     int auto_shutdown, char *shutdown_reason, Connect_param_t * conpar);
extern int Contact_servers(int count, char **servers,
			   int auto_connect, int list_servers,
			   int auto_shutdown, char *shutdown_message,
			   int find_max, int *num_found,
			   char **server_addresses, char **server_names,
			   unsigned *server_versions, Connect_param_t * conpar);

extern int micro_delay(unsigned usec);

extern int Welcome_screen(Connect_param_t * conpar);

extern void Widget_cleanup(void);

#ifdef _WINDOWS
extern void WinXCreateItemBitmaps();
#endif


#endif
