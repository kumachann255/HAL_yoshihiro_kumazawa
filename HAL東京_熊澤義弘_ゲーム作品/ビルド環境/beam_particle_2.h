//=============================================================================
//
// ビームのパーティクル処理 [beam_particle_2.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once

#define	MAX_BEAM_PARTICLE2			(512 * 5)	// パーティクル最大数
//#define	MAX_BEAM_PARTICLE			(512 * 4)	// パーティクル最大数


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct BEAM_PARTICLE_2
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		basePos;		// 開始位置
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	XMFLOAT3		move;			// 移動量
	MATERIAL		material;		// マテリアル
	float			fSizeX;			// 幅
	float			fSizeY;			// 高さ
	int				nIdxShadow;		// 影ID
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか
	float			time;			// 時間
	float			radius;			// 回転の半径
	float			radian;			// 回転の角度

	XMFLOAT3		distance;		// 移動距離



};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBeamParticle2(void);
void UninitBeamParticle2(void);
void UpdateBeamParticle2(void);
void DrawBeamParticle2(void);

int SetBeamParticle2(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT3 base, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);
void SetColorBeamParticle2(int nIdxParticle, XMFLOAT4 col);
void SetBeamParticleSwitch2(BOOL data);
void SetBeamParticleCharge2(BOOL data);
void SetBeam2Delete(void);
BEAM_PARTICLE_2 *GetBeamParticle2(void);
void SetChargeRadius2(float radius);
