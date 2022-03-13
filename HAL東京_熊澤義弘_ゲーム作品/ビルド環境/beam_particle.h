//=============================================================================
//
// ビームのパーティクル処理 [beam_particle.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once

#define	MAX_BEAM_PARTICLE			(512 * 5)	// パーティクル最大数
//#define	MAX_BEAM_PARTICLE			(512 * 4)	// パーティクル最大数


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct BEAM_PARTICLE
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	XMFLOAT3		baseMove;		// 基準移動ベクトル
	XMFLOAT3		move;			// 移動量
	MATERIAL		material;		// マテリアル
	float			fSizeX;			// 幅
	float			fSizeY;			// 高さ
	int				nIdxShadow;		// 影ID
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか
	float			time;			// 時間


	XMFLOAT3		ControlPos0;	// 開始位置
	XMFLOAT3		ControlPos1;	// 制御点1
	XMFLOAT3		ControlPos2;	// 制御点2
	XMFLOAT3		ControlPos3;	// 制御点3
	XMFLOAT3		distance;		// 移動距離
	BOOL			first;			// 発生して最初のフレームかどうか



};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBeamParticle(void);
void UninitBeamParticle(void);
void UpdateBeamParticle(void);
void DrawBeamParticle(void);

int SetBeamParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT3 base, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);
void SetColorBeamParticle(int nIdxParticle, XMFLOAT4 col);
void SetBeamParticleSwitch(BOOL data);
void SetBeamParticleCharge(BOOL data);
void SetBeamDelete(void);
BEAM_PARTICLE *GetBeamParticle(void);
void SetChargeRadius(float radius);
BOOL GetBeamOnOff(void);
