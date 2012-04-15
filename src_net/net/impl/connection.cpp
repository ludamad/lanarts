#include <asio.hpp>
#include <boost/bind.hpp>
#include "../connection.h"
#include "ClientNetConnection.h"
#include "ServerNetConnection.h"
#include "DummyNetConnection.h"

NetConnection* create_server_connection(int port){
	return new ServerNetConnection(port);
}

NetConnection* create_client_connection(const char* host, const char* port){
	return new ClientNetConnection(host, port);
}

NetConnection* create_dummy_connection(){
	return new DummyNetConnection();
}
