/*
 * DummyNetConnection.h
 *
 *  Created on: Apr 14, 2012
 *      Author: 100397561
 */

#ifndef DUMMYNETCONNECTION_H_
#define DUMMYNETCONNECTION_H_

#include "../connection.h"

class DummyNetConnection: public NetConnection {
public:
	DummyNetConnection(){}

	virtual bool get_next_packet(NetPacket& packet){return true;};
	virtual void broadcast_packet(const NetPacket& packet){};
	virtual void join(){}
	virtual int get_peer_id(){ return 0;}
	virtual int get_number_peers(){ return 0;}

};

#endif /* DUMMYNETCONNECTION_H_ */
