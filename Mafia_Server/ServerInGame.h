#pragma once
#include "pch.h"
#include "Define.h"
#include "NetWorkDefine.h"
#include "ClientPacket.h"
#include "ServerPacket.h"

class ServerInGame
{
public:
	ServerInGame();
	~ServerInGame();

	void Initialize();
	void Tick();
	void Release();

	void SetPlayerJob();
	void SendPlayerJob();

	void SendPacket();
	void SendCitizen(SOCKET socket);
	void SendMafia(SOCKET socket);
	void SendPolice(SOCKET socket);

	void ReceivePacket(int ClientIndex);
	void ReceiveAllChat(int ClientIndex);
	void ReceiveVote(int ClientIndex);
	void ReceiveTarget(int ClientIndex);

	void Update();
	void Update_Night2Day();
	void Update_Day2Vote();
	void Update_Vote2Night();

	void PickVictim();

private:
	vector<int> CitizenIndex_vec;
	vector<int> MafiaIndex_vec;
	vector<int> PoliceIndex_vec;

	int PlayerNum;
	int MafiaNum;
	int PoliceNum = 1;
	int AlivePlayerNum;
	int AliveMafiaNum;
	
	// chat buffer vector
	deque<TChatMessage> NormalChat;
	deque<TChatMessage> MafiaChat;

	EInGameState PreGameState;
	EInGameState CurGameState;
	bool bGameStateChanged;
	bool PreGameEnd = false;

	long long CurTime;       // 현재 시간
	long long LastTime;      // 마지막으로 시간을 쟀던 타이밍
	long long RemainTime;						// 남은 시간

	thread ReceiveThread[CLIENT_MAX];

	mutex mMessage;
	mutex mMafiaTarget;
	mutex mPoliceTarget;

	int VoteList[10];
	int MafiaTarget;
	int MafiaTargetToSend = -1;
	int PoliceTarget;
	int PoliceTargetToSend = -1;
	bool PoliceTargetIsMafia = false;

	int DeadPlayer[10];
	int VoteResultList[10];
	int VictimIndex = -1;
};

