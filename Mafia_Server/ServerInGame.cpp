#include "ServerInGame.h"
#include "Define.h"
#include "pch.h"
#include <time.h>
#include <random>
#include "PlayerInfo.h"

ServerInGame::ServerInGame()
	: PreGameState(INGAME_STATE_DAY), CurGameState(INGAME_STATE_DAY)
	, bGameStateChanged(false)
{
}

ServerInGame::~ServerInGame()
{
	Release();
}

void ServerInGame::Initialize()
{
	SetPlayerJob();
	Sleep(100);
	SendPlayerJob();

	for (int i = 0; i < ClientNum; i++)
	{
		ReceiveThread[i] = thread(&ServerInGame::ReceivePacket, this, PlayerInfo_Vec[i].PlayerIndex);
	}

	// 낮에 남은 시간 계산
	AlivePlayerNum = ClientNum;

	// TODO : 시간 설정
	RemainTime = ClientNum * 8;
	LastTime = GetTickCount();

	// 투표 리스트 초기화
	for (int i = 0; i < 10; i++)
	{
		VoteList[i] = -1;
	}

	// 투표 결과 리스트 초기화
	for (int i = 0; i < 10; i++)
	{
		VoteResultList[i] = 0;
	}

	// 죽은 사람 리스트 초기화
	for (int i = 0; i < 10; i++)
	{
		DeadPlayer[i] = 0;
	}

	// 능력 초기화
	MafiaTarget = -1;
	PoliceTarget = -1;
	PoliceTargetIsMafia = false;
}

void ServerInGame::Tick()
{
	while (1)
	{
		if (bGameEnd) return;
		Sleep(100);
		SendPacket();
		Update();
	}
}

void ServerInGame::Update()
{
	if (PreGameEnd)
	{
		bGameEnd = PreGameEnd;
		return;
	}
	// 모든 로직 업데이트
	// + 모든 변수 초기화
	bGameStateChanged = false;
	PoliceTargetIsMafia = false;

	// 게임이 끝났는지 확인
	if (AliveMafiaNum >= (AlivePlayerNum - AliveMafiaNum))
	{
		cout << "게임 종료. 마피아 승" << endl;
		PreGameEnd = true;
		bMafiaWin = true;
	}
	if (AliveMafiaNum <= 0)
	{
		cout << "게임 종료. 플레이어 승" << endl;
		PreGameEnd = true;
		bMafiaWin = false;
	}

	PreGameState = CurGameState;

	// 시간 업데이트
	CurTime = GetTickCount();
	if (CurTime - LastTime > 1000)
	{
		LastTime = CurTime;
		RemainTime -= 1;
		//cout << "현재 남은 시간: " << RemainTime << endl;
	}
	if (RemainTime <= 0)
	{
		CurGameState = (EInGameState)(((int)CurGameState + 1) % (int)EInGameState::INGAME_STATE_END);
		bGameStateChanged = true;
		
		if (CurGameState == INGAME_STATE_DAY)
		{
			RemainTime = AlivePlayerNum * 3;
		}
		else if (CurGameState == INGAME_STATE_VOTE)
		{
			RemainTime = 20;
		}
		else if (CurGameState == INGAME_STATE_NIGHT)
		{
			RemainTime = 20;
		}

		if (PreGameState != CurGameState)
		{
			if (CurGameState == INGAME_STATE_VOTE)
			{
				Update_Day2Vote();
			}
			else if (CurGameState == INGAME_STATE_NIGHT)
			{
				Update_Vote2Night();
			}
			else if (CurGameState == INGAME_STATE_DAY)
			{
				Update_Night2Day();
			}
		}

		// 리스트들 초기화
		// 투표 리스트 초기화
		for (int i = 0; i < 10; i++)
		{
			VoteList[i] = -1;
		}

		// 투표 결과 리스트 초기화
		for (int i = 0; i < 10; i++)
		{
			VoteResultList[i] = 0;
		}

		// 능력 초기화
		MafiaTarget = -1;
	}
}

void ServerInGame::Update_Night2Day()
{
	// 죽은 애, 조사하는 애 판정 
	// MafiaTarget은 그냥 죽은 애 인덱스임 ㅋㅋ
	// PoliceTarget은 마피아인지 아닌지 확인 해야 함
	if (MafiaTarget != -1)
	{
		DeadPlayer[MafiaTarget] = true;
		if (PlayerInfo_Vec[MafiaTarget].PlayerJob == PLAYER_JOB_MAFIA)
		{
			AliveMafiaNum--;
		}
		AlivePlayerNum--;
		MafiaTargetToSend = MafiaTarget;
	}
	else
	{
		MafiaTargetToSend = -1;
	}
	if (PoliceTarget != -1)
	{
		if (PlayerInfo_Vec[PoliceTarget].PlayerJob == PLAYER_JOB_MAFIA)
		{
			PoliceTargetIsMafia = true;
		}
		else
		{
			PoliceTargetIsMafia = false;
		}
		PoliceTargetToSend = PoliceTarget;
	}
	PoliceTarget = -1;
}

void ServerInGame::Update_Day2Vote()
{
	PoliceTarget = -1;
}

void ServerInGame::Update_Vote2Night()
{
	PickVictim();
}

void ServerInGame::PickVictim()
{
	for (int i = 0; i < 10; i++)
	{
		int tmpTarget = VoteList[i];
		if (tmpTarget != -1)
		{
			VoteResultList[tmpTarget]++;
		}
	}

	int minIndex = -1;
	int minNum = -1;

	for (int i = 0; i < 10; i++)
	{
		if (VoteResultList[i] > minNum)
		{
			minNum = VoteResultList[i];
			minIndex = i;
		}
		else if (VoteResultList[i] == minNum)
		{
			minIndex = -1;
			minNum = -1;
			break;
		}
	}

	if (minNum > 0)
	{
		VictimIndex = minIndex;
		AlivePlayerNum--;
		if (PlayerInfo_Vec[VictimIndex].PlayerJob == PLAYER_JOB_MAFIA) AliveMafiaNum--;
		DeadPlayer[VictimIndex] = 1;
	}
	
	// 결과를 얻은 후 초기화 시켜주기
	for (int i = 0; i < 10; i++)
	{
		VoteResultList[i] = 0;
	}

	for (int i = 0; i < 10; i++)
	{
		VoteList[i] = -1;
	}
}

void ServerInGame::SendPacket()
{	
	bool bSendMessage = false;
	TChatMessage sendMessage;
	if (!NormalChat.empty())
	{
		sendMessage = NormalChat.front();
		lock_guard<mutex> lock(mMessage);
		NormalChat.pop_front();
		bSendMessage = true;
	}
	
	for (int i = 0; i < ClientNum; i++)
	{
		EPlayerJob curPlayerJob = PlayerInfo_Vec[i].PlayerJob;
		SOCKET curPlayerSocket = ClientSock_Vec[i].Socket;

		send(curPlayerSocket, (char*)&PreGameEnd, sizeof(PreGameEnd), 0);
		send(curPlayerSocket, (char*)&bMafiaWin, sizeof(bMafiaWin), 0);
		if (PreGameEnd == true)
		{
			continue;
		}

		send(curPlayerSocket, (char*)&bSendMessage, sizeof(bSendMessage), 0);

		// 메세지 보내기
		if (bSendMessage)
		{
			send(curPlayerSocket, (char*)(sendMessage.MessageOwner), sizeof(sendMessage.MessageOwner), 0);
			send(curPlayerSocket, (char*)(sendMessage.Message), sizeof(sendMessage.Message), 0);
		}

		// 현재 게임 상태와 시간 보내기
		send(curPlayerSocket, (char*)&CurGameState, sizeof(CurGameState), 0);
		send(curPlayerSocket, (char*)&RemainTime, sizeof(RemainTime), 0);

		// 게임 상태가 바뀌었을 때
		send(curPlayerSocket, (char*)&bGameStateChanged, sizeof(bGameStateChanged), 0);
		if (bGameStateChanged)
		{
			if (CurGameState == INGAME_STATE_DAY)
			{
				WCHAR SystemMessage[1024] = L"낮이 되었습니다";
				send(curPlayerSocket, (char*)&SystemMessage, sizeof(SystemMessage), 0);

				// 타겟 보내주기 
				send(curPlayerSocket, (char*)&MafiaTargetToSend, sizeof(MafiaTargetToSend), 0);
				send(curPlayerSocket, (char*)&PoliceTargetToSend, sizeof(PoliceTargetToSend), 0);
				send(curPlayerSocket, (char*)&PoliceTargetIsMafia, sizeof(PoliceTargetIsMafia), 0);
			}
			else if (CurGameState == INGAME_STATE_VOTE)
			{
				WCHAR SystemMessage[1024] = L"투표 시간이 되었습니다. 처형할 사람을 선택해주세요.";
				send(curPlayerSocket, (char*)&SystemMessage, sizeof(SystemMessage), 0);

			}
			else if (CurGameState == INGAME_STATE_NIGHT)
			{
				send(curPlayerSocket, (char*)&VictimIndex, sizeof(VictimIndex), 0);
				WCHAR SystemMessage[1024] = L"밤이 되었습니다. 능력의 대상을 지목해주세요.";
				send(curPlayerSocket, (char*)&SystemMessage, sizeof(SystemMessage), 0);
			}
		}

		if (curPlayerJob == PLAYER_JOB_CITIZEN)
		{
			SendCitizen(curPlayerSocket);
		}
		else if (curPlayerJob == PLAYER_JOB_MAFIA)
		{
			send(curPlayerSocket, (char*)&MafiaTarget, sizeof(MafiaTarget), 0);
			SendMafia(curPlayerSocket);
		}
		else if (curPlayerJob == PLAYER_JOB_POLICE)
		{
			send(curPlayerSocket, (char*)&PoliceTarget, sizeof(PoliceTarget), 0);
			SendPolice(curPlayerSocket);
		}

		// 투표 관련
		bool bVoted = false;
		if (VoteList[i] != -1)
		{
			bVoted = true;
		}

		send(curPlayerSocket, (char*)&bVoted, sizeof(bVoted), 0);
		if (bVoted)
		{
			int tmpIndex = VoteList[i];
			send(curPlayerSocket, (char*)&(tmpIndex), sizeof(tmpIndex), 0);
		}
	}
	VictimIndex = -1;
	bSendMessage = false;
}

void ServerInGame::SendCitizen(SOCKET socket)
{
}

void ServerInGame::SendMafia(SOCKET socket)
{
}

void ServerInGame::SendPolice(SOCKET socket)
{
}

void ServerInGame::Release()
{
	for (int i = 0; i < PlayerNum; i++)
	{
		ReceiveThread[i].join();
	}
}

void ServerInGame::SetPlayerJob()
{
	PlayerNum = ClientNum;

	if (PlayerNum < 8)
	{
		MafiaNum = 1;
	}
	else
	{
		MafiaNum = 2;
	}
	AliveMafiaNum = MafiaNum;

	srand(time(NULL));
	vector<int> ChoosenIndex_Vec;

	// 마피아 선정
	while (MafiaIndex_vec.size() != MafiaNum)
	{
		int randIndex = rand() % PlayerNum;

		int tmpFlag = 0;
		for (auto vec : ChoosenIndex_Vec)
		{
			if (vec == randIndex) tmpFlag = 1;
		}

		if (!tmpFlag)
		{
			MafiaIndex_vec.push_back(randIndex);
			ChoosenIndex_Vec.push_back(randIndex);
			PlayerInfo_Vec[randIndex].PlayerJob = PLAYER_JOB_MAFIA;
		}
	}

	// 경찰 선정
	while (PoliceIndex_vec.size() != 1)
	{
		int randIndex = rand() % PlayerNum;

		int tmpFlag = 0;
		for (auto vec : ChoosenIndex_Vec)
		{
			if (vec == randIndex) tmpFlag = 1;
		}

		if (!tmpFlag)
		{
			PoliceIndex_vec.push_back(randIndex);
			ChoosenIndex_Vec.push_back(randIndex);
			PlayerInfo_Vec[randIndex].PlayerJob = PLAYER_JOB_POLICE;
		}
	}

	// 나머지 시민
	for (int i = 0; i < PlayerNum; i++)
	{
		int flag = 1;

		for (auto choosenindex : ChoosenIndex_Vec)
		{
			if (choosenindex == i)
			{
				flag = 0;
				break;
			}
		}

		if (flag)
		{
			CitizenIndex_vec.push_back(i);
			PlayerInfo_Vec[i].PlayerJob = PLAYER_JOB_CITIZEN;
		}
	}	
}

void ServerInGame::SendPlayerJob()
{
	for (int i = 0; i < PlayerNum; i++)
	{
		Sleep(10);
		EPlayerJob tmpPlayerJobBuffer = PlayerInfo_Vec[i].PlayerJob;
		send(ClientSock_Vec[i].Socket, (char*)&tmpPlayerJobBuffer, sizeof(tmpPlayerJobBuffer), 0);
		send(ClientSock_Vec[i].Socket, (char*)&tmpPlayerJobBuffer, sizeof(tmpPlayerJobBuffer), 0);

		send(ClientSock_Vec[i].Socket, (char*)&(PlayerNum), sizeof(PlayerNum), 0);

		// 여기서 기본적인 플레이어들의 정보까지 모두 넣어주자
		for (int j = 0; j < PlayerNum; j++)
		{
			send(ClientSock_Vec[i].Socket, (char*)&(PlayerInfo_Vec[j].PlayerIndex), sizeof(PlayerInfo_Vec[j].PlayerIndex), 0);
			send(ClientSock_Vec[i].Socket, (char*)&(PlayerInfo_Vec[j].PlayerName), sizeof(PlayerInfo_Vec[j].PlayerName), 0);
		}
		cout << "client" << i << " 에게 직업 전달" << endl;
	}
}

void ServerInGame::ReceivePacket(int ClientIndex)
{
	SOCKET curSocket = ClientSock_Vec[ClientIndex].Socket;
	EPlayerJob PlayerJob = PlayerInfo_Vec[ClientIndex].PlayerJob;

	if (bGameEnd) return;

	if (PlayerJob == PLAYER_JOB_CITIZEN)
	{
		while (1)
		{
			Sleep(10);
			EPacketType PacketType = PACKETTYPE_END;
			recv(curSocket, (char*)&PacketType, sizeof(PacketType), 0);
			Sleep(10);
			if (PacketType == CHAT_PACKET)
			{
				ReceiveAllChat(ClientIndex);
			}
			else if (PacketType == VOTE_PACKET)
			{
				ReceiveVote(ClientIndex);
			}
			else if (PacketType == TARGET_PACKET)
			{
				ReceiveTarget(ClientIndex);
			}
		}
	}
	else if (PlayerJob == PLAYER_JOB_MAFIA)
	{
		while (1)
		{
			Sleep(10);
			EPacketType PacketType = PACKETTYPE_END;
			recv(curSocket, (char*)&PacketType, sizeof(PacketType), 0);

			if (PacketType == CHAT_PACKET)
			{
				ReceiveAllChat(ClientIndex);
			}
			else if (PacketType == VOTE_PACKET)
			{
				ReceiveVote(ClientIndex);
			}
			else if (PacketType == TARGET_PACKET)
			{
				ReceiveTarget(ClientIndex);
			}
		}
	}
	else if (PlayerJob == PLAYER_JOB_POLICE)
	{
		while (1)
		{
			Sleep(10);
			EPacketType PacketType = PACKETTYPE_END;
			recv(curSocket, (char*)&PacketType, sizeof(PacketType), 0);

			if (PacketType == CHAT_PACKET)
			{
				ReceiveAllChat(ClientIndex);
			}
			else if (PacketType == VOTE_PACKET)
			{
				ReceiveVote(ClientIndex);
			}
			else if (PacketType == TARGET_PACKET)
			{
				ReceiveTarget(ClientIndex);
			}
		}
	}
}

void ServerInGame::ReceiveAllChat(int ClientIndex)
{
	SOCKET curSocket = ClientSock_Vec[ClientIndex].Socket;

	WCHAR MessageOwner[1024];
	recv(curSocket, (char*)MessageOwner, sizeof(MessageOwner), 0);
	WCHAR Message[1024];
	recv(curSocket, (char*)Message, sizeof(Message), 0);

	TChatMessage newMessage;
	wcscpy_s(newMessage.MessageOwner, sizeof(newMessage.MessageOwner) / sizeof(WCHAR), PlayerInfo_Vec[ClientIndex].PlayerName);
	wcscpy_s(newMessage.Message, sizeof(newMessage.Message) / sizeof(WCHAR), Message);

	cout << "Client" << ClientIndex << ": ";
	wcout << Message << endl;

	if (DeadPlayer[ClientIndex] == 0)
	{
		mMessage.lock();
		NormalChat.push_back(newMessage);
		mMessage.unlock();
	}
}

void ServerInGame::ReceiveVote(int ClientIndex)
{
	SOCKET curSocket = ClientSock_Vec[ClientIndex].Socket;

	int Victim = -1;
	recv(curSocket, (char*)&Victim, sizeof(Victim), 0);

	if (Victim != -1 && DeadPlayer[Victim] == 0)
	{
		if (VoteList[ClientIndex] == -1)
		{
			VoteList[ClientIndex] = Victim;
			cout << "Client" << ClientIndex << "가 Client" << Victim << "을 투표 대상으로 지정했습니다." << endl;
		}
	}
}

void ServerInGame::ReceiveTarget(int ClientIndex)
{
	SOCKET curSocket = ClientSock_Vec[ClientIndex].Socket;

	int Target = -1;
	recv(curSocket, (char*)&Target, sizeof(Target), 0);

	// 플레이어의 직업에 따라 나누기
	EPlayerJob CurPlayerJob = PlayerInfo_Vec[ClientIndex].PlayerJob;
	if (CurPlayerJob == PLAYER_JOB_CITIZEN)
	{

	}
	else if (CurPlayerJob == PLAYER_JOB_POLICE)
	{
		lock_guard<mutex> lock(mPoliceTarget);
		PoliceTarget = Target;
		cout << "경찰이 client" << Target << "을 지목했습니다." << endl;
	}
	else if (CurPlayerJob == PLAYER_JOB_MAFIA)
	{
		lock_guard<mutex> lock(mMafiaTarget);
		MafiaTarget = Target;
		cout << "마피아가 client" << Target << "을 지목했습니다" << endl;
	}
}
