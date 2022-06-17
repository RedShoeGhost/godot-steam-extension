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

protected:
	static void _bind_methods();

public:
	Steam();
	~Steam();
	
	// System
	bool init();
	void run_callbacks();

	// User
	String get_persona_name();

	// Lobby
	void create_lobby(int lobby_type, int max_members);
	bool set_lobby_data(uint64_t steam_lobby_id, const String& key, const String& value);
	void join_lobby(uint64_t steam_lobby_id);
	void leave_lobby(uint64_t steam_lobby_id);
	void request_lobby_list();
	void add_request_lobby_list_string_filter(const String& key_to_match, const String& value_to_match, int comparison_type);
	void add_request_lobby_list_distance_filter(int distance_filter);
	
};

#endif // ! STEAM_CLASS_H
