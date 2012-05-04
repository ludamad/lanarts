#include "../connection.h"
#include "ClientNetConnection.h"
#include "ServerNetConnection.h"


NetConnection* create_server_connection(int port){
	return new ServerNetConnection(port);
}

NetConnection* create_client_connection(const char* host, const char* port){
	return new ClientNetConnection(host, port);
}

