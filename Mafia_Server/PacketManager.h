#pragma once
#include "ServerPacket.h"


class PacketManager
{
public:
	PacketManager();
	~PacketManager();

	void MakePacket_PlayerInfoPacket(int ClientIndex, PlayerInfoPacket& Buff);

public:
	static PacketManager* Get_Instance()
	{
		if (Instance == nullptr)
		{
			Instance = new PacketManager;
		}
		return Instance;
	}

	void Destroy_Instance()
	{
		if (Instance)
		{
			delete Instance;
			Instance = nullptr;
		}
	}

private:
	static PacketManager* Instance;

};

