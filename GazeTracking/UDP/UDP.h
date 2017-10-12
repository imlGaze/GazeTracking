#pragma once

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

class UDP {
	sockaddr_in sockAddr;
	SOCKET sock;

public:
	/*`
	hostがNULLの場合、INADDR_ANY
	*/
	UDP(unsigned short port, char *host = NULL);

	/*
	ソケットをバインドしてデータを受け取れるようにする
	同じポートにバインドできるソケットは1つ
	*/
	bool listen();

	/* データを送る */
	bool send(unsigned char *data, unsigned int len);

	/*
	データを受け取る
	データが見つかるまでロックされる
	*/
	bool receive(unsigned char *data);

	~UDP() {
		closesocket(sock);
	}

};
