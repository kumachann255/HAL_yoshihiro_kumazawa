//=============================================================================
//
// ホーリーのパーティクル処理 [holy_particle.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitHolyParticle(void);
void UninitHolyParticle(void);
void UpdateHolyParticle(void);
void DrawHolyParticle(void);

int SetHolyParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);
void SetColorHolyParticle(int nIdxParticle, XMFLOAT4 col);
void SetParticleSwitch(BOOL data);

