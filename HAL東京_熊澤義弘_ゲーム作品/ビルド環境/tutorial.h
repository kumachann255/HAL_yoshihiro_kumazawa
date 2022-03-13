//=============================================================================
//
// チュートリアル処理 [tutorial.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TUTORIAL_X		(290.0f)		// チュートリアル中に動けるx座標の最大 

enum {
	tx_start,
	tx_fire,
	tx_lightning,
	tx_water,
	tx_holy,
	tx_clear,
	tx_MAX,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitTutorial(void);
void UninitTutorial(void);
void UpdateTutorial(void);
void DrawTutorial(void);

BOOL GetTutorialUse(void);
BOOL GetTutorialClear(int tutorial);
void SetTutorial(int tutorial);
void SetTutorialClear(int tutorial);

