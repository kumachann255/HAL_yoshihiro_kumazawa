//=============================================================================
//
// ファイア処理 [fire.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once
#define	MAX_FIRE		(512)		// パーティクル最大数


//*****************************************************************************
// 構造体定義
//*****************************************************************************
typedef struct
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	XMFLOAT3		move;			// 移動量
	MATERIAL		material;		// マテリアル
	float			fSizeX;			// 幅
	float			fSizeY;			// 高さ
	int				nIdxShadow;		// 影ID
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか
	BOOL			Up;				// 上昇しているかどうか
	BOOL			atk;			// 当たり判定を行ったか

} FIRE;


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitFire(void);
void UninitFire(void);
void UpdateFire(void);
void DrawFire(void);

int SetFireParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, int nLife, float scaleX, float scaleY);
void SetColorFire(int nIdxParticle, XMFLOAT4 col);
void SetFireSwitch(BOOL data);
void ResetFire(int i);
FIRE *GetFire(void);
void ResetFireTime(void);

