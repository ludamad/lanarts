/* GameNetConnection.cpp:
 * 	Utility class for managing communication with other players
 * 	If no network communication is used, the methods of this class
 * 	return trivial results.
 * 	Provides useful behaviour on top of the src_net library.
 */

#include <net/lanarts_net.h>

#include "../gamestate/GameState.h"

#include "../serialize/SerializeBuffer.h"

#include "../objects/player/PlayerInst.h"

#include "GameNetConnection.h"

GameNetConnection::GameNetConnection(GameChat& chat, PlayerData& pd,
		GameStateInitData& init_data) :
		chat(chat), pd(pd), init_data(init_data), _connection(NULL) {
	_message_buffer = new SerializeBuffer(SerializeBuffer::plain_buffer());
}

GameNetConnection::~GameNetConnection() {
	delete _connection;
	delete _message_buffer;
}

SerializeBuffer& GameNetConnection::grab_buffer(message_t type) {
	_message_buffer->clear();
	_message_buffer->write_int(type);
	return *_message_buffer;
}

void GameNetConnection::send_packet(SerializeBuffer& serializer, int receiver) {
	_connection->send_message(serializer.data(), serializer.size(), receiver);
}

bool GameNetConnection::check_integrity(GameState* gs, int value) {
	return true;
}

void GameNetConnection::initialize_as_client(const char* host, int port) {
	LANARTS_ASSERT(!_connection);
	_connection = create_client_connection(host, port);
	_connection->initialize_connection();
}

void GameNetConnection::initialize_as_server(int port) {
	LANARTS_ASSERT(!_connection);
	_connection = create_server_connection(port);
	_connection->initialize_connection();
}

void GameNetConnection::set_accepting_connections(bool accept) {
	_connection->set_accepting_connections(accept);
}

static void gamenetconnection_packet_callback(receiver_t sender, void* context,
		const char* msg, size_t len) {
	((GameNetConnection*)context)->_handle_message(sender, msg, len);
}
void GameNetConnection::poll_messages(int timeout) {
	_connection->poll(gamenetconnection_packet_callback, (void*)this, timeout);
}
void net_send_player_actions(GameNetConnection& net, int player_number,
		const ActionQueue& actions) {
	SerializeBuffer& sb = net.grab_buffer(GameNetConnection::PACKET_ACTION);
	sb.write_container(actions);
	net.send_packet(sb);
}

// Only the server is concerned with this message
void net_recv_connection_affirm(SerializeBuffer& sb, int sender,
		PlayerData& pd) {
	std::string name;
	class_id classtype;
	sb.read(name);
	sb.read_int(classtype);
	pd.register_player(name, NULL, classtype, sender);
}

void net_send_connection_affirm(GameNetConnection& net, const std::string& name,
		class_id classtype) {
	SerializeBuffer& sb = net.grab_buffer(
			GameNetConnection::PACKET_CLIENT2SERV_CONNECTION_AFFIRM);
	sb.write(name);
	sb.write_int(classtype);
	net.send_packet(sb, NetConnection::SERVER_RECEIVER);
}

void net_recv_game_init_data(SerializeBuffer& sb, int sender,
		GameStateInitData& init_data, PlayerData& pd) {
	//Write seed
	sb.read_int(init_data.seed);
	init_data.seed_set_by_network_message = true;

	//Write player data
	int playern;
	sb.read_int(playern);
	LANARTS_ASSERT(pd.all_players().empty());
	for (int i = 0; i < playern; i++) {
		std::string name;
		class_id classtype;
		int net_id;
		sb.read(name);
		sb.read_int(classtype);
		sb.read_int(net_id);
		pd.register_player(name, NULL, classtype, net_id);
	}

}
void net_send_game_init_data(GameNetConnection& net, PlayerData& pd, int seed) {
	SerializeBuffer& sb = net.grab_buffer(
			GameNetConnection::PACKET_SERV2CLIENT_INITIALPLAYERDATA);
	sb.write_int(seed);
	std::vector<PlayerDataEntry>& plist = pd.all_players();
	sb.write_int(plist.size());
	for (int i = 0; i < plist.size(); i++) {
		PlayerDataEntry& pde = plist[i];
		sb.write(pde.player_name);
		sb.write_int(pde.classtype);
		sb.write_int(pde.net_id);
	}
	//Can send to all (default), wont send to self anyway
	net.send_packet(sb);
}

void net_send_chatmessage(GameNetConnection& net, ChatMessage & message) {
	SerializeBuffer& sb = net.grab_buffer(
			GameNetConnection::PACKET_CHAT_MESSAGE);
	message.serialize(sb);
	net.send_packet(sb);
}

void GameNetConnection::_handle_message(int sender, const char* msg,
		size_t len) {
	message_t type;

	// Fill buffer with message
	_message_buffer->clear();
	_message_buffer->write_raw(msg, len);

	// Read type from buffer
	_message_buffer->read_int(type);

	switch (type) {

	case PACKET_CLIENT2SERV_CONNECTION_AFFIRM: {
		net_recv_connection_affirm(*_message_buffer, sender, pd);
		break;
	}
	case PACKET_SERV2CLIENT_INITIALPLAYERDATA: {
		net_recv_game_init_data(*_message_buffer, sender, init_data, pd);
		break;
	}

	case PACKET_ACTION: {
		int player_number;
		_message_buffer->read_int(player_number);
		PlayerDataEntry& pde = pd.all_players()[player_number];
		ActionQueue actions;

		_message_buffer->read_container(actions);
		pde.player()->enqueue_actions(actions);

		break;
	}
	case PACKET_CHAT_MESSAGE: {
		ChatMessage msg;
		msg.deserialize(*_message_buffer);
		chat.add_message(msg);
		break;
	}
	}
}

//TODO: net redo
//
//void GameNetConnection::broadcast_packet(const NetPacket & packet,
//		bool send_to_new) {
//	if (connect) {
//		connect->broadcast_packet(packet, send_to_new);
//	}
//}
//
//void GameNetConnection::finalize_connections() {
//	if (connect) {
//		connect->finalize_connections();
//	}
//}
//
//void GameNetConnection::wait_for_packet(NetPacket& packet, int packettype) {
//	if (!connect) {
//		return;
//	}
//	while (!connect->get_next_packet(packet)) {
//		//Continue until condition is true
//	}
//}
//
//bool GameNetConnection::get_next_packet(NetPacket& packet, packet_t type) {
//	if (!connect) {
//		return false;
//	}
//	return connect->get_next_packet(packet, type);
//}
//
//bool GameNetConnection::check_integrity(GameState* gs, int value) {
//	NetPacket packet;
//	packet.add_int(value);
//	packet.encode_header();
//	std::vector<NetPacket> packets;
//	gs->net_connection().send_and_sync(packet, packets, false);
//	for (int i = 0; i < packets.size(); i++) {
//		NetPacket& p = packets[i];
//		int theirvalue = p.get_int();
//		if (theirvalue != value) {
//			fprintf(
//					stderr,
//					"Conflicting value theirs 0x%X vs ours 0x%X for sender %d\n",
//					theirvalue, value, i);
//			fflush(stderr);
//			return false;
//		}
//	}
//	return true;
//}
//
//void GameNetConnection::send_and_sync(const NetPacket & packet,
//		std::vector<NetPacket>& received, bool send_to_new) {
//	if (!connect)
//		return;
//	NetPacket local;
//	printf("Sending synched packet\n");
//	connect->broadcast_packet(packet, send_to_new);
//	printf("Waiting for synched packet\n");
//
//	wait_for_packet(local);
//	received.push_back(local);
//}

