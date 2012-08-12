/* GameNetConnection.h:
 * 	Utility class for managing communication with other players
 * 	If no network communication is used, the methods of this class
 * 	return trivial results.
 * 	Provides useful behaviour on top of the src_net library.
 */

#ifndef GAMENETCONNECTION_H_
#define GAMENETCONNECTION_H_

#include <vector>

#include "../gamestate/ActionQueue.h"

class GameState;
class SerializeBuffer;
class NetConnection;
struct ChatMessage;

class GameNetConnection {
public:
	/*Message types*/
	enum message_t {
		PACKET_ACTION = 0, PACKET_CHAT_MESSAGE = 1
	};
	GameNetConnection(GameState* gs);
	~GameNetConnection();

	void initialize_as_client(const char* host, int port);
	void initialize_as_server(int port);

	bool is_connected() {
		return _connection != NULL;
	}

	NetConnection* connection() {
		return _connection;
	}

	SerializeBuffer& grab_buffer(message_t type);

	void set_accepting_connections(bool accept);

	void poll_messages(int timeout = 0);

	void send_packet(SerializeBuffer& serializer);
	bool check_integrity(GameState* gs, int value);

	//Do-not-call-directly:
	void _handle_message(const char* msg, size_t len);

private:
	//Keep a back-pointer so that we can alter world state based on messages received
	GameState* gs;
	SerializeBuffer* _message_buffer;
	NetConnection* _connection;
};

void net_send_player_actions(GameNetConnection& net, int player_number,
		const ActionQueue& actions);
void net_send_chatmessage(GameNetConnection& net, ChatMessage& message);

#endif /* GAMENETCONNECTION_H_ */
