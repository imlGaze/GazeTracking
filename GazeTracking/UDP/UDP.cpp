#include <winsock2.h>
#include <iostream>
#include "UDP.h"

#pragma comment(lib, "ws2_32.lib")

UDP::UDP(unsigned short port, char *host) {
	memset(&sockAddr, 0, sizeof sockAddr);
	sockAddr.sin_addr.s_addr = host != NULL ? inet_addr(host) : htonl(INADDR_ANY);
	sockAddr.sin_port = htons(port);
	sockAddr.sin_family = AF_INET;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int tout = 10;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tout, sizeof tout);
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *) &tout, sizeof tout);
}

bool UDP::listen() {
	if (bind(sock, (const sockaddr *)&sockAddr, sizeof sockAddr) == SOCKET_ERROR) {
		std::cout << "bind error" << std::endl;
		return false;
	}

	return true;
}

bool UDP::send(unsigned char *data, unsigned int len) {
	if (sock == INVALID_SOCKET) {
		return false;
	}
	
	
	int rs = sendto(sock, (char *)data, len, 0, (const sockaddr *)&sockAddr, sizeof sockAddr);
	
	if (rs == SOCKET_ERROR) {
		return false;
	}

	return true;
}

bool UDP::receive(unsigned char *data) {
	if (sock == INVALID_SOCKET) {
		return false;
	}

	memset(data, 0, sizeof data);
	
	int socklen = sizeof sockAddr;
	int len = recvfrom(sock, (char *)data, sizeof data, 0, (sockaddr *)&sockAddr, &socklen);
	if (len <= 0) {
		return false;
	}

	return true;
}
