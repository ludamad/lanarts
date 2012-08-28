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
	_synch_buffer = new SerializeBuffer(SerializeBuffer::plain_buffer());
}

GameNetConnection::~GameNetConnection() {
	delete _connection;
	delete _message_buffer;
	delete _synch_buffer;
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
	((GameNetConnection*) context)->_handle_message(sender, msg, len);
}
void GameNetConnection::poll_messages(int timeout) {
	if (_connection) {
		_connection->poll(gamenetconnection_packet_callback, (void*) this,
				timeout);
	}
}
void net_recv_player_actions(SerializeBuffer& sb, int sender, PlayerData& pd) {
	int frame, player_number;
	sb.read_int(frame);
	sb.read_int(player_number);

	PlayerDataEntry& pde = pd.all_players().at(player_number);

	ActionQueue actions;
	sb.read_container(actions);
	pde.action_queue.queue_actions_for_frame(actions, frame);

//	printf("Receiving %d actions for player %d from %d \n", actions.size(),
//			player_number, sender);
}
void net_send_player_actions(GameNetConnection& net, int frame,
		int player_number, const ActionQueue& actions) {
	SerializeBuffer& sb = net.grab_buffer(GameNetConnection::PACKET_ACTION);
	sb.write_int(frame);
	sb.write_int(player_number);
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
	printf("connection affirm read\n");
	pd.register_player(name, NULL, classtype, sender);
	printf("now there are %d players\n", (int) pd.all_players().size());
	pd.set_local_player_idx(0);
}

void net_send_connection_affirm(GameNetConnection& net, const std::string& name,
		class_id classtype) {
	SerializeBuffer& sb = net.grab_buffer(
			GameNetConnection::PACKET_CLIENT2SERV_CONNECTION_AFFIRM);
	sb.write(name);
	sb.write_int(classtype);
	printf("connection affirm sent\n");
	net.send_packet(sb, NetConnection::SERVER_RECEIVER);
}

void net_recv_game_init_data(SerializeBuffer& sb, int sender,
		GameStateInitData& init_data, PlayerData& pd) {
	//Write seed
	sb.read_int(init_data.seed);
	init_data.seed_set_by_network_message = true;

	//Read player data
	int localidx;
	sb.read_int(localidx);
	pd.set_local_player_idx(localidx);
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
	printf("Received init packet: seed = 0x%X, localid = %d, nplayers = %d\n",
			init_data.seed, localidx, playern);
}
void net_send_game_init_data(GameNetConnection& net, PlayerData& pd, int seed) {
	for (int n = 0; n < pd.all_players().size(); n++) {
		int net_id = pd.all_players()[n].net_id;

		if (net_id == 0) {
			continue; // Don't send to self
		}

		//Send a version to each player detailing which player entry is theirs
		//This is a necessary (slight) evil

		SerializeBuffer& sb = net.grab_buffer(
				GameNetConnection::PACKET_SERV2CLIENT_INITIALPLAYERDATA);

		sb.write_int(seed);
		// Send which index is local player to recipient:
		sb.write_int(n);
		std::vector<PlayerDataEntry>& plist = pd.all_players();
		sb.write_int(plist.size());
		for (int i = 0; i < plist.size(); i++) {
			PlayerDataEntry& pde = plist[i];
			sb.write(pde.player_name);
			sb.write_int(pde.classtype);
			sb.write_int(pde.net_id);
		}
		net.send_packet(sb, net_id);
	}
}

static void post_synch(GameState* gs) {
	std::vector<PlayerDataEntry>& pdes = gs->player_data().all_players();
	for (int i = 0; i < pdes.size(); i++) {
		pdes[i].action_queue.clear();
		pdes[i].action_queue.queue_actions_for_frame(ActionQueue(),
				gs->frame());
	}
	gs->local_player()->actions_set() = true;
}

void net_send_synch_data(GameNetConnection& net, GameState* gs) {
	SerializeBuffer& sb = net.grab_buffer(GameNetConnection::PACKET_FORCE_SYNC);
	if (!net.is_connected())
		return;
	int mtwistseed = gs->rng().rand();
	gs->rng().init_genrand(mtwistseed);
	sb.write_int(mtwistseed);
	std::vector<PlayerDataEntry>& pdes = gs->player_data().all_players();
	gs->local_player()->actions_set() = false;
	gs->serialize(sb);
	net.send_packet(sb, NetConnection::ALL_RECEIVERS);
	post_synch(gs);
	// Wait for clients to receive the synch data before continuing
	net.wait_for_ack(GameNetConnection::PACKET_SYNC_ACK, true);

	net_send_synch_ack(net, NetConnection::ALL_RECEIVERS);
}

void net_recv_synch_data(SerializeBuffer& sb, GameState* gs) {
	int mtwistseed;
	sb.read_int(mtwistseed);
	gs->rng().init_genrand(mtwistseed);
	int nplayer = gs->player_data().get_local_player_idx();
	gs->deserialize(sb);
	std::vector<PlayerDataEntry>& pdes = gs->player_data().all_players();
	for (int i = 0; i < pdes.size(); i++) {
		pdes[i].player()->set_local_player(false);
	}
	gs->player_data().set_local_player_idx(nplayer);
}

void net_send_synch_ack(GameNetConnection& net, int sender) {
	SerializeBuffer& sb = net.grab_buffer(GameNetConnection::PACKET_SYNC_ACK);
	net.send_packet(sb);
}

bool GameNetConnection::consume_sync_messages(GameState* gs) {
	if (_synch_buffer->empty()) {
		return false;
	}
	printf("Receiving\n");
	net_recv_synch_data(*_synch_buffer, gs);
	_synch_buffer->clear();

	net_send_synch_ack(*this, NetConnection::ALL_RECEIVERS);
	post_synch(gs);
	// Wait for server to receive acks for all clients before continuing
	wait_for_ack(PACKET_SYNC_ACK);

	return true;
}

void net_send_chatmessage(GameNetConnection& net, ChatMessage & message) {
	SerializeBuffer& sb = net.grab_buffer(
			GameNetConnection::PACKET_CHAT_MESSAGE);
	message.serialize(sb);
	net.send_packet(sb);
}

struct recv_ack_helper {
	int mynetid;
	std::vector<bool> received;
	GameNetConnection::message_t expected;
	recv_ack_helper(int mynetid, int nplayers, GameNetConnection::message_t exp) :
			mynetid(mynetid), received(nplayers), expected(exp) {
	}
	bool received_all() {
		for (int i = 0; i < received.size(); i++) {
			if (!received[i] && i != mynetid)
				return false;
		}
		return true;
	}
	bool received_one() {
		for (int i = 0; i < received.size(); i++) {
			if (received[i])
				return true;
		}
		return false;
	}

};
static void gamenetconnection_recv_ack(receiver_t sender, void* context,
		const char* msg, size_t len) {
	int type = *(int*) msg;
	recv_ack_helper* helper = (recv_ack_helper*) context;
	if (type == helper->expected) {
		helper->received[sender] = true;
	}
}

void GameNetConnection::wait_for_ack(message_t msg, bool from_all) {
	const int timeout = 10;
	PlayerDataEntry& pde = pd.local_player_data();
	recv_ack_helper helper(pde.net_id, pd.all_players().size(), msg);
	while (true) {
		_connection->poll(gamenetconnection_recv_ack, (void*) &helper, timeout);
		if (helper.received_one() && !from_all)
			break;
		if (helper.received_all() && from_all)
			break;
	}
}

bool GameNetConnection::has_incoming_sync() {
	return !_synch_buffer->empty();
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
		net_recv_player_actions(*_message_buffer, sender, pd);
		break;
	}
	case PACKET_CHAT_MESSAGE: {
		ChatMessage msg;
		msg.deserialize(*_message_buffer);
		chat.add_message(msg);
		break;
	}

	case PACKET_FORCE_SYNC: {
		printf("Receiving synch message\n");
		std::swap(_synch_buffer, _message_buffer);
		break;
	}
	}

}
