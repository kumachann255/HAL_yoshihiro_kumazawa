//=============================================================================
//
// ホワイトフェード処理 [fade_white.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************

// フェードの状態
typedef enum
{
	WHITE_NONE = 0,		// 何もない状態
	WHITE_IN,			// フェードイン処理
	WHITE_IN_bless,		// フェードイン処理(ブレス時)
	WHITE_OUT_charge,	// フェードアウト処理(チャージ時)
	WHITE_OUT_blast,	// フェードアウト処理(爆風時)
	WHITE_MAX
} WHITE;



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitFadeWhite(void);
void UninitFadeWhite(void);
void UpdateFadeWhite(void);
void DrawFadeWhite(void);

void SetFadeWhite(WHITE fade);
WHITE GetFadeWhite(void);

void ResetInOut(void);


