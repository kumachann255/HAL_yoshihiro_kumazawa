//=============================================================================
//
// �z�[���[�̃p�[�e�B�N������ [holy_particle.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitHolyParticle(void);
void UninitHolyParticle(void);
void UpdateHolyParticle(void);
void DrawHolyParticle(void);

int SetHolyParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);
void SetColorHolyParticle(int nIdxParticle, XMFLOAT4 col);
void SetParticleSwitch(BOOL data);

