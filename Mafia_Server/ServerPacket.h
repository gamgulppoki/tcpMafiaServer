#pragma once
#include "Define.h"
#include "pch.h"

/*
	ServerPacket.h

	서버에서 클라이언트에게 보낼 패킷의 구조체 모음
*/

// 어떤 유저가 접속 시에, 이미 접속한 모든 유저에 대한 내용을 뿌려줄 패킷
struct PlayerInfoPacket
{
	WCHAR PlayerName[32];
	int PlayerIndex;
};

// 모두 준비 후, 유저 본인에게만 뿌릴 패킷
// 유저를 Initialize할 때 사용 예정
struct PostReadyPacket
{
	int PlayerIndex;
	EPlayerJob PlayerJob;
	bool bPlayerDead;
};

struct ServerChatPacket
{
	WCHAR Message[1024];
	WCHAR MessageOwnerName[1024];
};

struct InGame_PostVotePacket
{
	bool bExecute;
	int ExcuteeIndex;
};

// 누가 죽었는지/누가 조사받았는지에 따라 다르게 작동 
// 죽은 사람 / 조사한 사람의 직업이 뭔지 공개
struct InGame_PostNightPacket
{
	int TargetIndex;
	EPlayerJob TargetJob;
};