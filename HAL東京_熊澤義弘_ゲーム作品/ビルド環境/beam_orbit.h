//=============================================================================
//
// ビームの軌道処理 [beam_orbit.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once
#define	MAX_BEAM_ORBIT				(1024 * 12)		// パーティクル最大数


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBeamOrbit(void);
void UninitBeamOrbit(void);
void UpdateBeamOrbit(void);
void DrawBeamOrbit(void);

int SetBeamOrbit(XMFLOAT3 pos, float scl, int life);
void SetColorBeamOrbit(int nIdxParticle, XMFLOAT4 col);
void SetBeamOrbitSwitch(BOOL data);
void SetBeamOrbitDelete(void);

