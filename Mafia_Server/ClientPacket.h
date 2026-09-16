#pragma once
#include "Define.h"

/*
	ClientPacket.h

	클라이언트에서 서버에게 보낼 패킷의 구조체 모음
*/

// 준비 딸깍딸깍할 때 쓸 것
struct ReadyPacket
{
	int ClientIndex;
};

// InGamePacket
// 어떤 패킷이 필요한가?
// 낮 시작 -> 낮 시간, 직업, 메세지 패킷
// 투표 시작 -> 투표 시간, 직업, 선택한 사람, 메세지 패킷
// 밤 시작 -> 밤 시간, 직업, 선택한 사람
//		- 여기서 경합 발생 가능. 마피아가 2명일 경우 ..

// 근데 메세지 왔다갔다 하는 것과
// 현재 어떤 state인지를 따로 해줘야하는 것 아닌가?
// client에도, 서버에도 thread를 2개 만들어서
// 각각 다른 일을 처리할 수 있도록 해주어야 함
// ex) 유저가 10명인 경우 10*2 = 20개의 스레드가 일해야 함

struct ClientChatPacket
{
	WCHAR Message[1024];
};

// 필요 없을 듯 ...?
struct InGame_DayPacket
{

};

struct InGame_VotePacket
{
	int SourceIndex;
	int TargetIndex;
};

// 일반 시민에게는 필요 없음
// 직업이 있는 사람에게만 필요
struct InGame_NightPacket
{
	int SourceIndex;
	int TargetIndex;
};

struct GameOverPacket
{
	EPlayerJob WinTeam; // 이긴 팀 
};