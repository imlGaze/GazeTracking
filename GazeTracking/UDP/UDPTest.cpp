#include "UDP/UDP.h"
#include <iostream>

void main_udp_test() {
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) return;

	UDP udp(12345, "127.0.0.1"); // do not use DNS: mslookup
	// udp.listen();

	unsigned char sd[] = {'a', 0x7F, 0x80, 0x99, 0xAA, 0xBB, 0x00, 0xFF}; // ‘—‚é
	unsigned char rd[sizeof sd]; // Žó‚¯Žæ‚é

	udp.send(sd, sizeof sd);
	/*
	do {
		udp.send(sd, sizeof sd);
	} while (!udp.receive(rd));

	for (int i = 0; i < sizeof rd; i++) {
		if (rd[i] == 0xAA) {
			std::cout << "!" << std::flush;
		}
		printf("%02X\n", rd[i]);
	}
	*/

	int i;
	std::cin >> i;
	
	WSACleanup();
}
