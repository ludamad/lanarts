

#ifndef GAMENETCONNECTION_H_
#define GAMENETCONNECTION_H_

#include <net/connection.h>
#include <net/packet.h>
#include <vector>

class GameNetConnection {
public:
	GameNetConnection(){connect=NULL;}
	GameNetConnection(NetConnection* connect);
	~GameNetConnection();

	NetConnection*& get_connection() { return connect; }
	void send_and_sync(const NetPacket& packet, std::vector<NetPacket>& receieved);
private:
	NetConnection* connect;
};

#endif /* GAMENETCONNECTION_H_ */
