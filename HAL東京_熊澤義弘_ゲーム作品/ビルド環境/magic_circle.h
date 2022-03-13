//=============================================================================
//
// 魔法陣処理 [magic_circle.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once
#define	MAX_CIRCLE		(1)		// パーティクル最大数


//*****************************************************************************
// 構造体定義
//*****************************************************************************
typedef struct
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	MATERIAL		material;		// マテリアル
	BOOL			bUse;			// 使用しているかどうか
	BOOL			end;			// 消え始めるかどうか
	float			dissolve;		// ディゾルブの閾値

} CIRCLE;


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitCircle(void);
void UninitCircle(void);
void UpdateCircle(void);
void DrawCircle(void);

void SetCircle(BOOL use, BOOL end, int magic);
void SetColorCircle(XMFLOAT4 col);
void ResetCircle(void);
CIRCLE *GetCircle(void);

