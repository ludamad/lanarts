/*
 * NetConnection.h:
 *  Represents either a server or client connection
 */

#ifndef NETCONNECTION_H_
#define NETCONNECTION_H_

#include <cstdlib>
#include <functional>

typedef std::function<bool()> conn_callback;
typedef int receiver_t;
typedef void (*packet_recv_callback)(receiver_t sender, void* context,
		const char* msg, size_t len);

/**
 * Interface for client & server connections
 */
class NetConnection {
public:
	static const receiver_t ALL_RECEIVERS = -1;
	static const receiver_t SERVER_RECEIVER = 0;

	/*
	 * Start the connection. Necessary before any polling/sending.
	 */
	virtual void initialize_connection(const conn_callback &callback, int timeout) = 0;

	/*
	 * Poll for new messages/connections.
	 * Returns amount of messages received.
	 */
	virtual int poll(packet_recv_callback message_handler,
			void* context = NULL, int timeout = 0) = 0;

	/*
	 * Applies only to servers, set if this connection will
	 * accept new connections
	 */
	virtual void set_accepting_connections(bool accept) = 0;

	/**
	 * Send a message to a given receiver
	 * Messages are sent so that the receiver of the message
	 * is posted first, and then the length. The server decides how to
	 * send along the message based on the receiver.
	 */
	virtual void send_message(const char* msg, int len, receiver_t receiver =
			ALL_RECEIVERS) = 0;

	virtual ~NetConnection() {
	}
};

#endif /* NETCONNECTION_H_ */
