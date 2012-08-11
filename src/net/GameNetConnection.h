/* GameNetConnection.h:
 * 	Utility class for managing communication with other players
 * 	If no network communication is used, the methods of this class
 * 	return trivial results.
 * 	Provides useful behaviour on top of the src_net library.
 */

#ifndef GAMENETCONNECTION_H_
#define GAMENETCONNECTION_H_

#include <vector>

class GameState;
class SerializeBuffer;
class NetConnection;

class GameNetConnection {
public:
	/*Packet types*/
	enum packet_type {
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

	SerializeBuffer& grab_cleared_buffer(packet_type type);

	void set_accepting_connections(bool accept);

	void poll();
	void send_packet(SerializeBuffer& serializer);
	bool check_integrity(GameState* gs, int value);

private:
	void receive_packet(const char* msg, size_t len);

	//Keep a back-pointer so that we can alter world state based on messages received
	GameState* gs;
	SerializeBuffer* _message_buffer;
	NetConnection* _connection;
};

#endif /* GAMENETCONNECTION_H_ */
