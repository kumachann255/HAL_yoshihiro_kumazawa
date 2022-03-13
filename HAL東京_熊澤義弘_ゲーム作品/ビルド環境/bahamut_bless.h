//=============================================================================
//
// バハムートのブレス処理 [bahamut_bless.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once

#include "bahamut.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_BLESS		(1)					// モーフィングの数
#define MAX_BLESS_MOVE	(6)					// モーフィングの最大種類

#define BLAST_X_OFFSET	(590.0f)			// 爆風の位置調整

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct BLESS
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				move_time;			// 実行時間
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBless(void);
void UninitBless(void);
void UpdateBless(void);
void DrawBless(void);

BLESS *GetBlast(void);

void SetBlessVertex(DX11_MODEL *Model);
void SetBlessMove(void);
BOOL GetBlessMove(void);
void ResetBless(void);
void SetSurge(void);
