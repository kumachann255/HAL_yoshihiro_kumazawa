//=============================================================================
//
// オプション処理 [option.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define OPTION_MAX			(99999)		// スコアの最大値
#define OPTION_DIGIT		(5)			// 桁数

// 変更できるサウンドの種類
enum {
	master,
	bgm,
	se,
	voice,
};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitOption(void);
void UninitOption(void);
void UpdateOption(void);
void DrawOption(void);

BOOL GetOptionUse(void);
void SetOption(BOOL data);
float SetSoundPala(int soundType);

