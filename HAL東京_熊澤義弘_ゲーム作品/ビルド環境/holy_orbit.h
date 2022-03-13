//=============================================================================
//
// �z�[���[�̋O������ [holy_orbit.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitHolyOrbit(void);
void UninitHolyOrbit(void);
void UpdateHolyOrbit(void);
void DrawHolyOrbit(void);

int SetHolyOrbit(XMFLOAT3 pos);
void SetColorHolyOrbit(int nIdxParticle, XMFLOAT4 col);
void SetOrbitSwitch(BOOL data);

