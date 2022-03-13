//=============================================================================
//
// ブレスのパーティクル処理 [bless_particle.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBlessParticle(void);
void UninitBlessParticle(void);
void UpdateBlessParticle(void);
void DrawBlessParticle(void);

int SetBlessParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int life);
void SetColorBlessParticle(int nIdxParticle, XMFLOAT4 col);
void SetBlessParticleSwitch(BOOL data);

