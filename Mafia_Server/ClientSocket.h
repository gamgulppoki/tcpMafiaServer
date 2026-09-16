#pragma once
#include "pch.h"

class ClientSocket
{
public:
	ClientSocket();
	~ClientSocket();

public:
	SOCKET Socket;
	SOCKADDR_IN SocketInfo = { 0 };
	int InfoSize = sizeof(SocketInfo);
	int Index;
};

