//=============================================================================
//
// ライトニング処理 [lightning.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once

#define MAX_LIGHTNING			(4)			// 同時最大発動個数
#define	MAX_REFRACTION			(30)		// ライトニング最大屈折数
#define MAX_BIFURCATION			(2)			// 分岐できる回数

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

	int				vecType;		// 移動方向のベクトルの種類
	int				count;			// 分岐までのカウントを管理


} LIGHTNING;

typedef struct
{
	XMFLOAT3		pos;			// 位置
	float			fSizeX;			// 当たり判定の横幅
	float			fSizeY;			// 当たり判定の高さ
	float			fSizeZ;			// 当たり判定の奥行き
	BOOL			bUse;			// 使用しているかどうか
} LIGHTNING_COLLI;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitLightning(void);
void UninitLightning(void);
void UpdateLightning(void);
void DrawLightning(void);

LIGHTNING_COLLI *GetLightningCollition(void);
void SetLightning(void);
void SetUpdateLightning(int z, int vectype);
//void SetColorLightning(int nIdxParticle, XMFLOAT4 col);
float RamdomFloat(int digits, float max, float min);
void ResetLightning(int z);
BOOL GetLightningOnOff(void);
void SetLightningOnOff(BOOL data);

