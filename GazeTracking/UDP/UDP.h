#pragma once

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

class UDP {
	sockaddr_in sockAddr;
	SOCKET sock;

public:
	/*`
	host��NULL�̏ꍇ�AINADDR_ANY
	*/
	UDP(unsigned short port, char *host = NULL);

	/*
	�\�P�b�g���o�C���h���ăf�[�^���󂯎���悤�ɂ���
	�����|�[�g�Ƀo�C���h�ł���\�P�b�g��1��
	*/
	bool listen();

	/* �f�[�^�𑗂� */
	bool send(unsigned char *data, unsigned int len);

	/*
	�f�[�^���󂯎��
	�f�[�^��������܂Ń��b�N�����
	*/
	bool receive(unsigned char *data);

	~UDP() {
		closesocket(sock);
	}

};
