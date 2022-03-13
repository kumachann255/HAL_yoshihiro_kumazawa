//=============================================================================
//
// 詠唱時のパーティクル処理 [chant_particle.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once

#define	MAX_CHANT_PARTICLE			(256)	// パーティクル最大数


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitChantParticle(void);
void UninitChantParticle(void);
void UpdateChantParticle(void);
void DrawChantParticle(void);

int SetChantParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSize, int nLife, float radius);
void SetColorChantParticle(int nIdxParticle, XMFLOAT4 col);
void SetChantParticleSwitch(BOOL data);
void SetChantDelete(void);
//PARTICLE *GetChantParticle(void);
void SetChargeRadius(float radius);
