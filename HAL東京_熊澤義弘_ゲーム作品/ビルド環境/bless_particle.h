//=============================================================================
//
// �u���X�̃p�[�e�B�N������ [bless_particle.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBlessParticle(void);
void UninitBlessParticle(void);
void UpdateBlessParticle(void);
void DrawBlessParticle(void);

int SetBlessParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int life);
void SetColorBlessParticle(int nIdxParticle, XMFLOAT4 col);
void SetBlessParticleSwitch(BOOL data);

