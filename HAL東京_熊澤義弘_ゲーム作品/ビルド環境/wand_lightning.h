//=============================================================================
//
// 杖の雷処理 [wand_lightning.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once

#define	MAX_WAND_LIGHTNING			(35)	// パーティクル最大数
#define	MAX_WAND_REFRACTION			(6)	// パーティクル最大数
//#define	MAX_BEAM_PARTICLE			(512 * 4)	// パーティクル最大数


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct WAND_LIGHTNING
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	MATERIAL		material;		// マテリアル
	float			fSizeX;			// 幅
	float			fSizeY;			// 高さ
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか


	XMFLOAT3		ControlPos0;	// 開始位置
	XMFLOAT3		ControlPos1;	// 制御点1 : 基準点からどのくらい離れているか
	XMFLOAT3		ControlPos2;	// 制御点2
	XMFLOAT3		ControlPos3;	// 制御点3
	XMFLOAT3		ControlPos4;	// 制御点4
	XMFLOAT3		ControlPos5;	// 制御点5
	XMFLOAT3		EndPos		;	// 終着点

	XMFLOAT3		distance;		// 移動距離

};

struct WAND_ORBIT
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	MATERIAL		material;		// マテリアル
	float			fSizeX;			// 幅
	float			fSizeY;			// 高さ
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか
};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitWandLightning(void);
void UninitWandLightning(void);
void UpdateWandLightning(void);
void DrawWandLightning(void);

int SetWandOrbit(XMFLOAT3 pos, XMFLOAT4 col, float fSizeX, float fSizeY);
int SetWandLightning(XMFLOAT3 pos, XMFLOAT3 conpos1, XMFLOAT3 conpos2, XMFLOAT3 conpos3, XMFLOAT3 conpos4, XMFLOAT3 conpos5, XMFLOAT4 col, float fSize);
void SetColorWandLightning(int nIdxParticle, XMFLOAT4 col);
void SetWandLightningSwitch(BOOL data);
//void SetBeamDelete(void);
WAND_LIGHTNING *GetWandLightning(void);
void SetChargeRadius(float radius);
BOOL GetBeamOnOff(void);
