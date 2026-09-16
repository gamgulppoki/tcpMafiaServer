#pragma once
#include "pch.h"
#include "ClientSocket.h"

#define WINCX		500
#define WINCY		700

#define	PURE		= 0

#define VK_MAX		0xff

#define IDC_CHATINPUT 1001

#define CLIENT_MAX 10

extern HWND g_hWnd;
extern HWND g_Dlg;
extern HWND g_hEdit;
extern HINSTANCE hInst;

extern int AllClientNum;
extern bool bGameEnd;
extern bool bMafiaWin;

enum EPlayerJob
{
	PLAYER_JOB_NONE,
	PLAYER_JOB_CITIZEN,
	PLAYER_JOB_MAFIA,
	PLAYER_JOB_POLICE,

	PLAYER_JOB_END
};

struct PlayerInfo
{
	int PlayerIndex;
	WCHAR PlayerName[1024];
	EPlayerJob PlayerJob;
	
};

extern vector<PlayerInfo> PlayerInfo_Vec;
extern vector<ClientSocket> ClientSock_Vec;
extern int ClientNum;

typedef struct tagInfo
{
	float fX;
	float fY;
	float fCX;
	float fCY;

}INFO;

template<typename T>
void Safe_Delete(T& p)
{
	if (p)
	{
		delete p;
		p = nullptr;
	}
}

enum EAssetID
{
	// etc
	ASSET_ETC,

	// Scene Background
	ASSET_TITLE_BACKGROUND,
	ASSET_LOBBY_BACKGROUND,
	ASSET_INGAME_BACKGROUND_DAY,
	ASSET_INGAME_BACKGROUND_NIGHT,
	ASSET_ENDGAME_BACKGROUND,

	// Character Icon
	ASSET_NONE,
	ASSET_CITIZEN,
	ASSET_MAFIA,
	ASSET_POLICE,

	// Button
	ASSET_BUTTON_PLAY,
	ASSET_BUTTON_READY,

	// Etc
	ASSET_PANNEL,
	ASSET_LEADERBOARD_ENTRY,
	ASSET_CHECK,
	ASSET_INPUT_BOX,
	ASSET_CROSSHAIR,
	ASSET_INVEST_MARK,
	ASSET_CROSS_MARK,
	ASSET_DEAD_MARK,

	ASSET_END
};

enum EScene
{
	SCENE_TITLE,
	SCENE_LOBBY,
	SCENE_INGAME,
	SCENE_ENDGAME,

	SCENE_END
};

enum EButtonType
{
	BUTTON_PLAY,
	BUTTON_READY,

	BUTTON_END
};

enum EObjectID
{
	PLAYER,
	BUTTON,
	LEADERBOARD_ENTRY,
	PANNEL,
	INPUT_BOX,
	MOUSE,

	OBJ_END
};

enum EPannelState
{
	PANNEL_STATE_NONE,
	PANNEL_STATE_CITIZEN,
	PANNEL_STATE_MAFIA,
	PANNEL_STATE_POLICE,

	PANNEL_STATE_END
};

enum EPannelPickState
{
	PANNEL_PICK_NONE,
	PANNEL_PICK_VOTED,
	PANNEL_PICK_TARGETED,
	PANNEL_PICK_INVESTIGATED,

	PANNEL_PICK_END
};

enum EInGameState
{
	INGAME_STATE_DAY,
	INGAME_STATE_VOTE,
	INGAME_STATE_NIGHT,

	INGAME_STATE_END
};

struct TChatMessage
{
	WCHAR Message[1024];
	WCHAR MessageOwner[1024];
	RECT ChatRect;
	bool bVisible = true;
};