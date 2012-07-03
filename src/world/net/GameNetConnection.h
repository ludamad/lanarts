/* GameNetConnection.h:
 * 	Utility class for managing communication with other players
 * 	If no network communication is used, the methods of this class
 * 	return trivial results.
 */

#ifndef GAMENETCONNECTION_H_
#define GAMENETCONNECTION_H_

#include <net/connection.h>
#include <net/packet.h>
#include <vector>

class GameState;

class GameNetConnection {
public:
	/*Packet types*/
	enum {
		PACKET_ACTION = 0, PACKET_CHAT_MESSAGE = 1
	};
	GameNetConnection(int our_peer_id) {
		connect = NULL;
	}
	GameNetConnection(NetConnection* connect = NULL);
	~GameNetConnection();

	void add_peer_id(int peer_id);

	NetConnection*& get_connection() {
		return connect;
	}
	void wait_for_packet(NetPacket& packet);
	void send_and_sync(const NetPacket& packet,
			std::vector<NetPacket>& received, bool send_to_new = false);

	template<class T>
	void send_and_sync(const T& ours, std::vector<T>& theirs, bool send_to_new =
			false) {
		NetPacket packet;
		packet.add(ours);
		packet.encode_header();
		std::vector<NetPacket> received;
		send_and_sync(packet, received, send_to_new);
		theirs.resize(received.size());
		for (int i = 0; i < received.size(); i++) {
			received[i].get(theirs[i]);
		}
	}

	void broadcast_packet(const NetPacket& packet, bool send_to_new = false);
	void finalize_connections();
	bool check_integrity(GameState* gs, int value);
private:
	std::vector<int> peer_ids;
	int our_peer_id;
	NetConnection* connect;
};

#endif /* GAMENETCONNECTION_H_ */
