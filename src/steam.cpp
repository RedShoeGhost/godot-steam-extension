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

#define STEAM_LARGE_BUFFER_SIZE 8160

void Steam::_bind_methods() {
	// System
	ClassDB::bind_method(D_METHOD("init"), &Steam::init);
	ClassDB::bind_method(D_METHOD("run_callbacks"), &Steam::run_callbacks);

	// User
	ClassDB::bind_method(D_METHOD("get_steam_id"), &Steam::get_steam_id);
	ClassDB::bind_method(D_METHOD("get_persona_name"), &Steam::get_persona_name);
	ClassDB::bind_method(D_METHOD("get_friend_persona_name", "steam_id"), &Steam::get_friend_persona_name);

	// Lobby
	ClassDB::bind_method(D_METHOD("create_lobby", "lobby_type", "max_members"), &Steam::create_lobby, DEFVAL(2));
	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "lobby_id")));
	ClassDB::bind_method(D_METHOD("set_lobby_data", "steam_lobby_id", "key", "value"), &Steam::set_lobby_data);
	ClassDB::bind_method(D_METHOD("join_lobby", "steam_lobby_id"), &Steam::join_lobby);
	ADD_SIGNAL(MethodInfo("lobby_joined", PropertyInfo(Variant::INT, "lobby"), PropertyInfo(Variant::INT, "permissions"), PropertyInfo(Variant::BOOL, "locked"), PropertyInfo(Variant::INT, "response")));
	ClassDB::bind_method(D_METHOD("leave_lobby", "steam_lobby_id"), &Steam::leave_lobby);
	ClassDB::bind_method(D_METHOD("request_lobby_list"), &Steam::request_lobby_list);
	ADD_SIGNAL(MethodInfo("lobby_match_list", PropertyInfo(Variant::ARRAY, "lobbies")));
	ClassDB::bind_method(D_METHOD("add_request_lobby_list_string_filter", "key_to_match", "value_to_match", "comparison_type"), &Steam::add_request_lobby_list_string_filter);
	ClassDB::bind_method(D_METHOD("add_request_lobby_list_distance_filter", "distance_filter"), &Steam::add_request_lobby_list_distance_filter);
	ADD_SIGNAL(MethodInfo("lobby_data_update", PropertyInfo(Variant::INT, "success"), PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "member_id")));
	ADD_SIGNAL(MethodInfo("lobby_chat_update", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "changed_id"), PropertyInfo(Variant::INT, "making_change_id"), PropertyInfo(Variant::INT, "chat_state")));
	ADD_SIGNAL(MethodInfo("lobby_chat_message", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "user"), PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::INT, "chat_type")));
	ADD_SIGNAL(MethodInfo("lobby_invite", PropertyInfo(Variant::INT, "inviter"), PropertyInfo(Variant::INT, "lobby"), PropertyInfo(Variant::INT, "game")));
	ADD_SIGNAL(MethodInfo("lobby_join_requested", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::STRING, "steam_id")));
	ClassDB::bind_method(D_METHOD("get_lobby_owner", "steam_lobby_id"), &Steam::get_lobby_owner);
	ClassDB::bind_method(D_METHOD("get_num_lobby_members", "steam_lobby_id"), &Steam::get_num_lobby_members);
	ClassDB::bind_method(D_METHOD("get_lobby_member_by_index", "steam_lobby_id", "member"), &Steam::get_lobby_member_by_index);
	ClassDB::bind_method(D_METHOD("send_lobby_chat_message", "steam_lobby_id", "message_body"), &Steam::send_lobby_chat_message);

	// P2P
	ClassDB::bind_method(D_METHOD("accept_p2p_session_with_user", "steam_id_remote"), &Steam::accept_p2p_session_with_user);
	ClassDB::bind_method(D_METHOD("allow_p2p_packet_relay", "allow"), &Steam::allow_p2p_packet_relay);
	ClassDB::bind_method(D_METHOD("close_p2p_session_with_user", "steam_id_remote"), &Steam::close_p2p_session_with_user);
	ClassDB::bind_method(D_METHOD("get_available_p2p_packet_size", "channel"), &Steam::get_available_p2p_packet_size, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("read_p2p_packet", "packet", "channel"), &Steam::read_p2p_packet, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("send_p2p_packet", "steam_id_remote", "data", "send_type", "channel"), &Steam::send_p2p_packet, DEFVAL(0));
	ADD_SIGNAL(MethodInfo("p2p_session_request", PropertyInfo(Variant::INT, "steam_id_remote")));
	ADD_SIGNAL(MethodInfo("p2p_session_connect_fail", PropertyInfo(Variant::INT, "steam_id_remote"), PropertyInfo(Variant::INT, "session_error")));
}

Steam::Steam() :
	callbackLobbyMatchList(this, &Steam::lobby_match_list),
	callbackLobbyCreated(this, &Steam::lobby_created),
	callbackLobbyJoined(this, &Steam::lobby_joined),
	callbackLobbyDataUpdate(this, &Steam::lobby_data_update),
	callbackLobbyChatUpdate(this, &Steam::lobby_chat_update),
	callbackLobbyMessage(this, &Steam::lobby_chat_message),
	callbackLobbyInvite(this, &Steam::lobby_invite),
	callbackLobbyJoinRequested(this, &Steam::lobby_join_requested),
	
	callbackP2PSessionRequest(this, &Steam::p2p_session_request),
	callbackP2PSessionConnectFail(this, &Steam::p2p_session_connect_fail)
{
	//UtilityFunctions::print("Constructor.");
}

Steam::~Steam() {
	//UtilityFunctions::print("Destructor.");
}

// Internal
CSteamID Steam::createSteamID(uint64_t steam_id, int account_type){
	CSteamID converted_steam_id;
	if(account_type < 0 || account_type >= k_EAccountTypeMax){
		account_type = 1;
	}
	converted_steam_id.Set(steam_id, k_EUniversePublic, EAccountType(account_type));
	return converted_steam_id;
}

// System
bool Steam::init() {
	return SteamAPI_Init();
}

void Steam::run_callbacks() {
	SteamAPI_RunCallbacks();
}

// User
uint64_t Steam::get_steam_id(){
	if(SteamUser() == NULL){
		return 0;
	}
	CSteamID steam_id = SteamUser()->GetSteamID();
	return steam_id.ConvertToUint64();
}

String Steam::get_persona_name(){
	if(SteamFriends() == NULL){
		return "";
	}
	return String::utf8(SteamFriends()->GetPersonaName());
}

String Steam::get_friend_persona_name(uint64_t steam_id){
	if(SteamFriends() != NULL && steam_id > 0){
		CSteamID user_id = (uint64)steam_id;
		bool is_data_loading = SteamFriends()->RequestUserInformation(user_id, true);
		if(!is_data_loading){
			return String::utf8(SteamFriends()->GetFriendPersonaName(user_id));
		}
	}
	return "";
}


// Lobby
void Steam::create_lobby(int lobby_type, int max_members){
	if(SteamMatchmaking() != NULL){
		SteamMatchmaking()->CreateLobby((ELobbyType)lobby_type, max_members);
	}
}
void Steam::lobby_created(LobbyCreated_t *lobbyData){
	int result = lobbyData->m_eResult;
	CSteamID lobby_id = lobbyData->m_ulSteamIDLobby;
	uint64_t lobby = lobby_id.ConvertToUint64();
	emit_signal("lobby_created", result, lobby);
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
void Steam::lobby_joined(LobbyEnter_t* lobbyData){
	CSteamID steam_lobby_id = lobbyData->m_ulSteamIDLobby;
	uint64_t lobby_id = steam_lobby_id.ConvertToUint64();
	uint32_t permissions = lobbyData->m_rgfChatPermissions;
	bool locked = lobbyData->m_bLocked;
	uint32_t response = lobbyData->m_EChatRoomEnterResponse;
	emit_signal("lobby_joined", lobby_id, permissions, locked, response);
}

void Steam::leave_lobby(uint64_t steam_lobby_id){
	if(SteamMatchmaking() != NULL){
		CSteamID lobby_id = (uint64)steam_lobby_id;
		SteamMatchmaking()->LeaveLobby(lobby_id);
	}
}

void Steam::request_lobby_list(){
	if(SteamMatchmaking() != NULL){
		SteamMatchmaking()->RequestLobbyList();
	}
}

void Steam::lobby_match_list(LobbyMatchList_t *call_data){
	int lobby_count = call_data->m_nLobbiesMatching;
	Array lobbies;
	for(int i = 0; i < lobby_count; i++){
		CSteamID lobby_id = SteamMatchmaking()->GetLobbyByIndex(i);
		uint64_t lobby = lobby_id.ConvertToUint64();
		lobbies.append(lobby);
	}
	emit_signal("lobby_match_list", lobbies);
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

void Steam::lobby_data_update(LobbyDataUpdate_t* call_data){
	uint64_t member_id = call_data->m_ulSteamIDMember;
	uint64_t lobby_id = call_data->m_ulSteamIDLobby;
	uint8 success = call_data->m_bSuccess;
	emit_signal("lobby_data_update", success, lobby_id, member_id);
}

void Steam::lobby_chat_update(LobbyChatUpdate_t* call_data){
	uint64_t lobby_id = call_data->m_ulSteamIDLobby;
	uint64_t changed_id = call_data->m_ulSteamIDUserChanged;
	uint64_t making_change_id = call_data->m_ulSteamIDMakingChange;
	uint32 chat_state = call_data->m_rgfChatMemberStateChange;
	emit_signal("lobby_chat_update", lobby_id, changed_id, making_change_id, chat_state);
}

void Steam::lobby_chat_message(LobbyChatMsg_t* call_data){
	CSteamID lobby_id = call_data->m_ulSteamIDLobby;
	CSteamID user_id = call_data->m_ulSteamIDUser;
	uint8 chat_type = call_data->m_eChatEntryType;
	// Convert the chat type over
	EChatEntryType type = (EChatEntryType)chat_type;
	// Get the chat message data
	char buffer[STEAM_LARGE_BUFFER_SIZE];
	int size = SteamMatchmaking()->GetLobbyChatEntry(lobby_id, call_data->m_iChatID, &user_id, &buffer, STEAM_LARGE_BUFFER_SIZE, &type);
	uint64_t lobby = lobby_id.ConvertToUint64();
	uint64_t user = user_id.ConvertToUint64();
	emit_signal("lobby_chat_message", lobby, user, String::utf8(buffer, size), chat_type);
}

void Steam::lobby_invite(LobbyInvite_t* lobbyData){
	CSteamID inviter_id = lobbyData->m_ulSteamIDUser;
	uint64_t inviter = inviter_id.ConvertToUint64();
	CSteamID lobby_id = lobbyData->m_ulSteamIDLobby;
	uint64_t lobby = lobby_id.ConvertToUint64();
	CSteamID game_id = lobbyData->m_ulGameID;
	uint64_t game = game_id.ConvertToUint64();
	emit_signal("lobby_invite", inviter, lobby, game);
}

void Steam::lobby_join_requested(GameLobbyJoinRequested_t* call_data){
	CSteamID lobby_id = call_data->m_steamIDLobby;
	uint64_t lobby = lobby_id.ConvertToUint64();
	CSteamID friend_id = call_data->m_steamIDFriend;
	uint64_t steam_id = friend_id.ConvertToUint64();
	emit_signal("lobby_join_requested", lobby, steam_id);
}

uint64_t Steam::get_lobby_owner(uint64_t steam_lobby_id){
	if(SteamMatchmaking() == NULL){
		return 0;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	CSteamID owner_id = SteamMatchmaking()->GetLobbyOwner(lobby_id);
	return owner_id.ConvertToUint64();
}

int Steam::get_num_lobby_members(uint64_t steam_lobby_id){
	if(SteamMatchmaking() == NULL){
		return 0;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->GetNumLobbyMembers(lobby_id);
}

uint64_t Steam::get_lobby_member_by_index(uint64_t steam_lobby_id, int member){
	if(SteamMatchmaking() == NULL){
		return 0;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	CSteamID lobbyMember = SteamMatchmaking()->GetLobbyMemberByIndex(lobby_id, member);
	return lobbyMember.ConvertToUint64();
}

bool Steam::send_lobby_chat_message(uint64_t steam_lobby_id, const String& message_body){
	if(SteamMatchmaking() == NULL){
		return false;
	}
	CSteamID lobby_id = (uint64)steam_lobby_id;
	return SteamMatchmaking()->SendLobbyChatMsg(lobby_id, message_body.utf8().get_data(), message_body.length() + 1);
}

// P2P
bool Steam::accept_p2p_session_with_user(uint64_t steam_id_remote) {
	if (SteamNetworking() == NULL) {
		return false;
	}
	CSteamID steam_id = createSteamID(steam_id_remote);
	return SteamNetworking()->AcceptP2PSessionWithUser(steam_id);
}

bool Steam::allow_p2p_packet_relay(bool allow) {
	if (SteamNetworking() == NULL) {
		return false;
	}
	return SteamNetworking()->AllowP2PPacketRelay(allow);
}

bool Steam::close_p2p_session_with_user(uint64_t steam_id_remote) {
	if (SteamNetworking() == NULL) {
		return false;
	}
	CSteamID steam_id = createSteamID(steam_id_remote);
	return SteamNetworking()->CloseP2PSessionWithUser(steam_id);
}

uint32_t Steam::get_available_p2p_packet_size(int channel){
	if (SteamNetworking() == NULL) {
		return 0;
	}
	uint32_t messageSize = 0;
	return (SteamNetworking()->IsP2PPacketAvailable(&messageSize, channel)) ? messageSize : 0;
}

Dictionary Steam::read_p2p_packet(uint32_t packet, int channel){
	Dictionary result;
	if (SteamNetworking() == NULL) {
		return result;
	}
	PackedByteArray data;
	data.resize(packet);
	CSteamID steam_id;
	uint32_t bytesRead = 0;
	void* ptr;
	ptr = data.ptrw();
	if (SteamNetworking()->ReadP2PPacket(ptr, packet, &bytesRead, &steam_id, channel)){
		data.resize(bytesRead);
		uint64_t steam_id_remote = steam_id.ConvertToUint64();
		result["data"] = data;
		result["steam_id_remote"] = steam_id_remote;
	}
	else {
		data.resize(0);
	}
	return result;
}

bool Steam::send_p2p_packet(uint64_t steam_id_remote, PackedByteArray data, int send_type, int channel){
	if (SteamNetworking() == NULL) {
		return false;
	}
	CSteamID steam_id = createSteamID(steam_id_remote);
	const void* ptr;
	ptr = data.ptr();
	return SteamNetworking()->SendP2PPacket(steam_id, ptr, data.size(), EP2PSend(send_type), channel);
}

void Steam::p2p_session_request(P2PSessionRequest_t* call_data){
	uint64_t steam_id_remote = call_data->m_steamIDRemote.ConvertToUint64();
	emit_signal("p2p_session_request", steam_id_remote);
}

void Steam::p2p_session_connect_fail(P2PSessionConnectFail_t* call_data) {
	uint64_t steam_id_remote = call_data->m_steamIDRemote.ConvertToUint64();
	uint8_t session_error = call_data->m_eP2PSessionError;
	emit_signal("p2p_session_connect_fail", steam_id_remote, session_error);
}