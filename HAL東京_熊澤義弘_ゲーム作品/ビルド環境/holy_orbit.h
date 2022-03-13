//=============================================================================
//
// ホーリーの軌道処理 [holy_orbit.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitHolyOrbit(void);
void UninitHolyOrbit(void);
void UpdateHolyOrbit(void);
void DrawHolyOrbit(void);

int SetHolyOrbit(XMFLOAT3 pos);
void SetColorHolyOrbit(int nIdxParticle, XMFLOAT4 col);
void SetOrbitSwitch(BOOL data);

