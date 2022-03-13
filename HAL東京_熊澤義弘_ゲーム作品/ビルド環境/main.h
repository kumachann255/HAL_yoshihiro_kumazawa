//=============================================================================
//
// メイン処理 [main.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


#pragma warning(push)
#pragma warning(disable:4005)

#define _CRT_SECURE_NO_WARNINGS			// scanf のwarning防止
#include <stdio.h>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>


// 本来はヘッダに書かない方が良い
using namespace DirectX;


#define DIRECTINPUT_VERSION 0x0800		// 警告対処
#include "dinput.h"
#include "mmsystem.h"

#pragma warning(pop)


//*****************************************************************************
// ライブラリのリンク
//*****************************************************************************
#pragma comment (lib, "d3d11.lib")		
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "d3dx11.lib")	
//#pragma comment (lib, "d3dx9.lib")	
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "dxerr.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dinput8.lib")


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define SCREEN_WIDTH	(960)			// ウインドウの幅
#define SCREEN_HEIGHT	(540)			// ウインドウの高さ
#define SCREEN_CENTER_X	(SCREEN_WIDTH / 2)	// ウインドウの中心Ｘ座標
#define SCREEN_CENTER_Y	(SCREEN_HEIGHT / 2)	// ウインドウの中心Ｙ座標

#define	MAP_W			(2000.0f)
#define	MAP_H			(1000.0f)
#define	MAP_TOP			(MAP_H/2)
#define	MAP_DOWN		(-MAP_H/2)
#define	MAP_LEFT		(-MAP_W * 2/3)
#define	MAP_RIGHT		(MAP_W * 2/3)

#define MAP_OFFSET_X	(1150.0f)		// マップをずらして、x座標0.0fをスタート地点にする

// マップ
#define MAP_LIMIT_X_MAX	(2600.0f)		// 右端
#define MAP_LIMIT_X_MIN	(-200.0f)		// 左端
#define MAP_LIMIT_Z_MAX	( 140.0f)		// 上端
#define MAP_LIMIT_Z_MIN	(-140.0f)		// 下端

#define MAP_CIRCLE_LEFT	(1800.0f)		// 円型フィールドのx座標の始まり
#define MAP_CIRCLE_R	(500.0f)		// 円型フィールドの半径
#define MAP_OFFSTT_X	(100.0f)		// 微調整

// 補間用のデータ構造体を定義
struct INTERPOLATION_DATA
{
	XMFLOAT3	pos;		// 頂点座標
	XMFLOAT3	rot;		// 回転
	XMFLOAT3	scl;		// 拡大縮小
	float		frame;		// 実行フレーム数 ( dt = 1.0f/frame )
};

enum
{
	MODE_TITLE = 0,			// タイトル画面
	MODE_TUTORIAL,			// ゲーム説明画面
	MODE_GAME,				// ゲーム画面
	MODE_CLEAR,				// クリア画面
	MODE_OVER,				// ゲームオーバー画面
	MODE_ENDROLL,			// エンドロール画面
	MODE_MAX
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
long GetMousePosX(void);
long GetMousePosY(void);
char* GetDebugStr(void);

void SetMode(int mode);
int GetMode(void);
float RadianSum(float angle);

