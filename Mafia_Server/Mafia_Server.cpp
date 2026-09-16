#include "pch.h"
#include "Define.h"

#include "ClientSocket.h"
#include "ClientPacket.h"
#include "PlayerInfo.h"
#include "ServerInGame.h"

#include <atomic>
#include <string>

vector<ClientSocket> ClientSock_Vec;
vector<PlayerInfo> PlayerInfo_Vec;
bool LobbyReady[CLIENT_MAX];

SOCKET gServSock;
int ClientNum = 0;
int AllClientNum = 0;
int ReceiveClientNum = 0;

bool OnGameStart = false;
bool bGameEnd = false;
bool FinishSendPacket = false;
bool bMafiaWin = false;

void ServerLobby();
void ServerLobbySession(int ClientIndex);
void ServerLobbyUpdate(int newClientIndex);
void ServerLobbyReceive(int ClientIndex);
void CloseAllSocket();

int main(int argc, char* argv[])
{
	for (int i = 0; i < 10; i++)
	{
		LobbyReady[i] = false;
	}

	// 서버 소켓 생성
	WSADATA wsaData;

	SOCKADDR_IN ServAddr;

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		cout << ("WSAStartup() error!") << endl;

	gServSock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&ServAddr, 0, sizeof(ServAddr));
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ServAddr.sin_port = htons(atoi(argv[1]));

	if (bind(gServSock, (SOCKADDR*)&ServAddr, sizeof(ServAddr)) == SOCKET_ERROR)
		cout << ("bind() error") << endl;

	if (listen(gServSock, SOMAXCONN) == SOCKET_ERROR)
		cout << ("listen() error") << endl;

	thread(ServerLobby).detach();

	ServerLobbySession(0);

	while (!(OnGameStart))
	{
		
	}

	cout << "게임 시작" << endl;

	Sleep(10);

	// 인게임 
	ServerInGame* InGame = new ServerInGame;

	InGame->Initialize();

	while (!bGameEnd)
	{
		InGame->Tick();
	}

	Sleep(1000);
	CloseAllSocket();
	delete InGame;
	InGame = nullptr;

	return 0;
}

void ServerLobby()
{
	int ReadyCnt = 0;

	int Buff;

	while (1)
	{
		if (OnGameStart)
		{
			return;
		}
		ClientSock_Vec.push_back(ClientSocket());
		ClientSock_Vec[ClientNum].Socket = accept(gServSock, (SOCKADDR*)&(ClientSock_Vec[ClientNum].SocketInfo), &ClientSock_Vec[ClientNum].InfoSize);

		if (OnGameStart) return;

		ClientSock_Vec[ClientNum].Index = ClientNum;
		cout << "Client" << ClientNum << " 접속!" << endl;

		PlayerInfo_Vec.push_back(PlayerInfo());
		PlayerInfo_Vec[ClientNum].PlayerIndex = ClientNum;
		PlayerInfo_Vec[ClientNum].PlayerJob = PLAYER_JOB_NONE;

		wstring resultString = L"Client" + to_wstring(ClientNum);
		wcscpy_s(PlayerInfo_Vec[ClientNum].PlayerName, sizeof(PlayerInfo_Vec[ClientNum].PlayerName) / sizeof(WCHAR), resultString.c_str());

		send(ClientSock_Vec[ClientNum].Socket, (char*)&ClientNum, sizeof(ClientNum), 0);
		//thread(ServerLobbySession, ClientNum).detach();
		thread(ServerLobbyReceive, ClientNum).detach();
		ClientNum++;
		AllClientNum = ClientNum;
	}

	OnGameStart = true;
}

// 아마 클라이언트 index가 나일 것임. 
void ServerLobbySession(int ClientIndex)
{
	while (1)
	{
		Sleep(10);
		FinishSendPacket = false;
		if (OnGameStart) return;

		for (int i = 0; i < AllClientNum; i++)
		{
			bool IsServerStarted = OnGameStart;
			int tmpClientNum = AllClientNum;

			send(ClientSock_Vec[i].Socket, (char*)&IsServerStarted, sizeof(IsServerStarted), 0);
			send(ClientSock_Vec[i].Socket, (char*)&tmpClientNum, sizeof(tmpClientNum), 0);

			for (int j = 0; j < tmpClientNum; j++)
			{
				send(ClientSock_Vec[i].Socket, (char*)&(PlayerInfo_Vec[j].PlayerName), sizeof(PlayerInfo_Vec[j].PlayerName), 0);
				send(ClientSock_Vec[i].Socket, (char*)&(PlayerInfo_Vec[j].PlayerIndex), sizeof(PlayerInfo_Vec[j].PlayerIndex), 0);
				send(ClientSock_Vec[i].Socket, (char*)&(LobbyReady[j]), sizeof(LobbyReady[j]), 0);
			}
			cout << "client" << i << " 에게 로비 패킷 전달" << endl;
		}
		ServerLobbyUpdate(ClientIndex);
		FinishSendPacket = true;
		if (OnGameStart && FinishSendPacket) return;
	}
}

void ServerLobbyReceive(int ClientIndex)
{
	while (1)
	{
		Sleep(10);
		if (OnGameStart) return;
		recv(ClientSock_Vec[ClientIndex].Socket, (char*)&(LobbyReady[ClientIndex]), sizeof(LobbyReady[ClientIndex]), 0);
		LobbyReady[ClientIndex] = !LobbyReady[ClientIndex];
	}
}

void ServerLobbyUpdate(int newClientIndex)
{
	// 현재 게임이 시작 되었는지 확인
	int tmp = 0;
	for (int i = 0; i < AllClientNum; i++)
	{
		if (LobbyReady[i] == true)
			tmp++;
	}

	if (tmp >= 3 && tmp == ClientNum)
	{
		OnGameStart = true;
		return;
	}

	Sleep(10);
}

void CloseAllSocket()
{
	for (int i = 0; i < ClientNum; i++)
	{
		cout << i << "번 째 소켓 close" << endl;
		closesocket(ClientSock_Vec[i].Socket);
	}

	WSACleanup();
}
