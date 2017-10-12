#include <winsock2.h>
#include <iostream>
#include "UDP.h"

#pragma comment(lib, "ws2_32.lib")

UDP::UDP(unsigned short port, char *host) {
	memset(&sockAddr, 0, sizeof sockAddr);
	sockAddr.sin_addr.s_addr = host != NULL ? inet_addr(host) : htonl(INADDR_ANY);
	sockAddr.sin_port = htons(port);
	sockAddr.sin_family = AF_INET;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
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
	
	sendto(sock, (char *)data, len, 0, (const sockaddr *)&sockAddr, sizeof sockAddr);
	
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
