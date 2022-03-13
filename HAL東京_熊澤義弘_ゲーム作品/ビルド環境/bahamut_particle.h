//=============================================================================
//
// �o�n���[�g�̃p�[�e�B�N������ [bahamut_particle.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBahamutParticle(void);
void UninitBahamutParticle(void);
void UpdateBahamutParticle(void);
void DrawBahamutParticle(void);

int SetBahamutParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);
void SetColorBahamutParticle(int nIdxParticle, XMFLOAT4 col);
void SetBahamutParticleSwitch(BOOL data);

