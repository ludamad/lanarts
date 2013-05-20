#ifndef UDT_UTIL_H_
#define UDT_UTIL_H_

#ifndef WIN32
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#endif

#include <udt/udt.h>
#include <udt/ccc.h>

#include <lcommon/strformat.h>
#include <limits>

static const int LNET_MAX_WAIT_CONNECTIONS = 25;

struct LanartsCCC: public CCC {
	LanartsCCC() {
		m_dCWndSize = 4024.0;
	}
	virtual void setRate(int mbps) {
		m_dPktSndPeriod = 0;
		m_dCWndSize = std::numeric_limits<double>::infinity();

	}
};

class CUDPBlast: public CCC {
public:
	CUDPBlast() {
		m_dPktSndPeriod = 1000;
		m_dCWndSize = 83333.0;
		setACKInterval(1);
		setACKTimer(0);
	}

public:
	void setRate(double mbps) {
//		m_dPktSndPeriod = (m_iMSS * 8.0) / mbps;
	}
};

inline bool udt_initialize_connection(UDTSOCKET& socket, int port,
		const char* connect_ip = NULL) {
	addrinfo hints;
	addrinfo* res = NULL, *peer = NULL;

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	std::string port_str = format("%d", port).c_str();

	/* Query information for the port we want to use*/
	if (getaddrinfo(NULL, port_str.c_str(), &hints, &res)) {
		fprintf(stderr,
				"Error creating socket at port %d, possibly busy or invalid\n",
				port);
		return false;
	}

	/* Create the socket */
	socket = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (socket == -1) {
		fprintf(stderr, "Error creating socket: %s\n",
				UDT::getlasterror().getErrorMessage());
		return false;
	}

	/* Use a low buffer size, the default is very large */
	int udt_buffsize = 1600;
	UDT::setsockopt(socket, 0, UDT_SNDBUF, &udt_buffsize, sizeof(int));
	UDT::setsockopt(socket, 0, UDT_RCVBUF, &udt_buffsize, sizeof(int));

//	int udp_fc = 80;
//	UDT::setsockopt(socket, 0, UDT_FC, &udp_fc, sizeof(int));
//	int udp_mss = 400;
//	UDT::setsockopt(socket, 0, UDT_MSS, &udp_mss, sizeof(int));

	UDT::setsockopt(socket, 0, UDT_CC, new CCCFactory<CUDPBlast>,
			sizeof(CCCFactory<CUDPBlast> ));
	int udp_buffsize = 800;
	UDT::setsockopt(socket, 0, UDP_SNDBUF, &udp_buffsize, sizeof(int));
	UDT::setsockopt(socket, 0, UDP_RCVBUF, &udp_buffsize, sizeof(int));

	bool block = true;
	UDT::setsockopt(socket, 0, UDT_SNDSYN, &block, sizeof(bool));
	UDT::setsockopt(socket, 0, UDT_RCVSYN, &block, sizeof(bool));
	bool reuse = true;
	UDT::setsockopt(socket, 0, UDT_REUSEADDR, &reuse, sizeof(bool));
//	bool linger = false;
//	UDT::setsockopt(socket, 0, UDT_LINGER, &linger, sizeof(bool));

	/* Ensure the socket is bound */
	if (connect_ip == NULL) { // We are a server
		if (UDT::bind(socket, res->ai_addr, res->ai_addrlen) == UDT::ERROR) {
			fprintf(stderr, "Error binding socket: %s\n",
					UDT::getlasterror().getErrorMessage());
			return false;
		}
		UDT::listen(socket, LNET_MAX_WAIT_CONNECTIONS);
	} else { // We are a client
		if (getaddrinfo(connect_ip, port_str.c_str(), &hints, &peer)) {
			fprintf(stderr, "Error connecting to %s:%d\n", connect_ip, port);
			return false;
		}
		if (UDT::connect(socket, peer->ai_addr, peer->ai_addrlen)
				== UDT::ERROR) {
			fprintf(stderr, "Error connecting socket: %s\n",
					UDT::getlasterror().getErrorMessage());
			return false;
		}

	}

	freeaddrinfo(res);
	freeaddrinfo(peer);
	return true;
}

#endif