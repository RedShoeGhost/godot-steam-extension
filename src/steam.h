/*************************************************************************/
/*  steam.h                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef STEAM_CLASS_H
#define STEAM_CLASS_H

// We don't need windows.h in this plugin but many others do and it throws up on itself all the time
// So best to include it and make sure CI warns us when we use something Microsoft took for their own goals....
#ifdef WIN32
#include <windows.h>
#endif

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include <steam/steam_api.h>

using namespace godot;

class SteamRef : public RefCounted {
	GDCLASS(SteamRef, RefCounted);

protected:
	static void _bind_methods() {}

public:
	SteamRef();
	~SteamRef();
};

class Steam : public Control {
	GDCLASS(Steam, Control);

private:
	// Lobby
	STEAM_CALLBACK(Steam, lobby_match_list, LobbyMatchList_t, callbackLobbyMatchList);
	STEAM_CALLBACK(Steam, lobby_created, LobbyCreated_t, callbackLobbyCreated);
	STEAM_CALLBACK(Steam, lobby_joined, LobbyEnter_t, callbackLobbyJoined);
	STEAM_CALLBACK(Steam, lobby_data_update, LobbyDataUpdate_t, callbackLobbyDataUpdate);
	STEAM_CALLBACK(Steam, lobby_chat_update, LobbyChatUpdate_t, callbackLobbyChatUpdate);
	STEAM_CALLBACK(Steam, lobby_chat_message, LobbyChatMsg_t, callbackLobbyMessage);
	STEAM_CALLBACK(Steam, lobby_invite, LobbyInvite_t, callbackLobbyInvite);
	STEAM_CALLBACK(Steam, lobby_join_requested, GameLobbyJoinRequested_t, callbackLobbyJoinRequested);

	// P2P
	STEAM_CALLBACK(Steam, p2p_session_request, P2PSessionRequest_t, callbackP2PSessionRequest);
	STEAM_CALLBACK(Steam, p2p_session_connect_fail, P2PSessionConnectFail_t, callbackP2PSessionConnectFail);

protected:
	static void _bind_methods();

public:
	Steam();
	~Steam();

	// Internal
	CSteamID createSteamID(uint64_t steam_id, int account_type = -1);
	
	// System
	bool init();
	void run_callbacks();

	// User
	uint64_t get_steam_id();
	String get_persona_name();
	String get_friend_persona_name(uint64_t steam_id);

	// Lobby
	void create_lobby(int lobby_type, int max_members);
	bool set_lobby_data(uint64_t steam_lobby_id, const String& key, const String& value);
	void join_lobby(uint64_t steam_lobby_id);
	void leave_lobby(uint64_t steam_lobby_id);
	void request_lobby_list();
	void add_request_lobby_list_string_filter(const String& key_to_match, const String& value_to_match, int comparison_type);
	void add_request_lobby_list_distance_filter(int distance_filter);
	uint64_t get_lobby_owner(uint64_t steam_lobby_id);
	int get_num_lobby_members(uint64_t steam_lobby_id);
	uint64_t get_lobby_member_by_index(uint64_t steam_lobby_id, int member);
	bool send_lobby_chat_message(uint64_t steam_lobby_id, const String& message_body);

	// P2P
	bool accept_p2p_session_with_user(uint64_t steam_id_remote);
	bool allow_p2p_packet_relay(bool allow);
	bool close_p2p_session_with_user(uint64_t steam_id_remote);
	uint32_t get_available_p2p_packet_size(int channel = 0);
	Dictionary read_p2p_packet(uint32_t packet, int channel = 0);
	bool send_p2p_packet(uint64_t steam_id_remote, const PackedByteArray data, int send_type, int channel = 0);
	
};

#endif // ! STEAM_CLASS_H
