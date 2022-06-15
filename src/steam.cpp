/*************************************************************************/
/*  steam.cpp                                                          */
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

#include "steam.h"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

SteamRef::SteamRef() {
	//UtilityFunctions::print("SteamRef created.");
}

SteamRef::~SteamRef() {
	//UtilityFunctions::print("SteamRef destroyed.");
}

void Steam::_bind_methods() {
	// System
	ClassDB::bind_method(D_METHOD("init"), &Steam::init);
	ClassDB::bind_method(D_METHOD("run_callbacks"), &Steam::run_callbacks);

	// User
	ClassDB::bind_method(D_METHOD("get_persona_name"), &Steam::get_persona_name);

	// Lobby
	ClassDB::bind_method(D_METHOD("create_lobby", "lobby_type", "max_members"), &Steam::create_lobby, DEFVAL(2));
	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "lobby_id")));
	ClassDB::bind_method(D_METHOD("set_lobby_data", "steam_lobby_id", "key", "value"), &Steam::set_lobby_data);
	ClassDB::bind_method(D_METHOD("join_lobby", "steam_lobby_id"), &Steam::join_lobby);
	ClassDB::bind_method(D_METHOD("leave_lobby", "steam_lobby_id"), &Steam::leave_lobby);
	ClassDB::bind_method(D_METHOD("request_lobby_list"), &Steam::request_lobby_list);
	ADD_SIGNAL(MethodInfo("lobby_match_list", PropertyInfo(Variant::ARRAY, "lobbies")));
	ClassDB::bind_method(D_METHOD("add_request_lobby_list_string_filter", "key_to_match", "value_to_match", "comparison_type"), &Steam::add_request_lobby_list_string_filter);
	ClassDB::bind_method(D_METHOD("add_request_lobby_list_distance_filter", "distance_filter"), &Steam::add_request_lobby_list_distance_filter);
}

Steam::Steam() {
	//UtilityFunctions::print("Constructor.");
}

Steam::~Steam() {
	//UtilityFunctions::print("Destructor.");
}

// Methods.
bool Steam::init() {
	return SteamAPI_Init();
}

void Steam::run_callbacks() {
	SteamAPI_RunCallbacks();
}

String Steam::get_persona_name(){
	if(SteamFriends() == NULL){
		return "";
	}
	return String::utf8(SteamFriends()->GetPersonaName());
}


// Lobby
void Steam::create_lobby(int lobby_type, int max_members){
	if(SteamMatchmaking() != NULL){
		SteamAPICall_t api_call = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, max_members);
		callResultCreateLobby.Set(api_call, this, &Steam::lobby_created);
	}
}
void Steam::lobby_created(LobbyCreated_t *lobbyData, bool io_failure){
	if(io_failure){
		UtilityFunctions::printerr("lobby_created");
	}
	else{
		int result = lobbyData->m_eResult;
		CSteamID lobby_id = lobbyData->m_ulSteamIDLobby;
		uint64_t lobby = lobby_id.ConvertToUint64();
		emit_signal("lobby_created", result, lobby);
	}
}

bool Steam::set_lobby_data(uint64_t steam_lobby_id, const String& key, const String& value){
	if(SteamMatchmaking() == NULL){
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->SetLobbyData(lobby_id, key.utf8().get_data(), value.utf8().get_data());
}

void Steam::join_lobby(uint64_t steam_lobby_id){
	if(SteamMatchmaking() != NULL){
		CSteamID lobby_id = (uint64)steam_lobby_id;
		SteamMatchmaking()->JoinLobby(lobby_id);
	}
}

void Steam::leave_lobby(uint64_t steam_lobby_id){
	if(SteamMatchmaking() != NULL){
		CSteamID lobby_id = (uint64)steam_lobby_id;
		SteamMatchmaking()->LeaveLobby(lobby_id);
	}
}

void Steam::request_lobby_list(){
	if(SteamMatchmaking() != NULL){
		SteamAPICall_t api_call = SteamMatchmaking()->RequestLobbyList();
		callResultLobbyList.Set(api_call, this, &Steam::lobby_match_list);
	}
}

void Steam::lobby_match_list(LobbyMatchList_t *call_data, bool io_failure){
	if(io_failure){
		UtilityFunctions::printerr("lobby_match_list");
	}
	else{
		int lobby_count = call_data->m_nLobbiesMatching;
		Array lobbies;
		for(int i = 0; i < lobby_count; i++){
			CSteamID lobby_id = SteamMatchmaking()->GetLobbyByIndex(i);
			uint64_t lobby = lobby_id.ConvertToUint64();
			lobbies.append(lobby);
		}
		emit_signal("lobby_match_list", lobbies);
	}
}

void Steam::add_request_lobby_list_string_filter(const String& key_to_match, const String& value_to_match, int comparison_type){
	if(SteamMatchmaking() != NULL){
		SteamMatchmaking()->AddRequestLobbyListStringFilter(key_to_match.utf8().get_data(), value_to_match.utf8().get_data(), (ELobbyComparison)comparison_type);
	}
}

void Steam::add_request_lobby_list_distance_filter(int distance_filter){
	if(SteamMatchmaking() != NULL){
		SteamMatchmaking()->AddRequestLobbyListDistanceFilter((ELobbyDistanceFilter)distance_filter);
	}
}