//=============================================================================
//
// �r�����̃p�[�e�B�N������ [chant_particle.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once

#define	MAX_CHANT_PARTICLE			(256)	// �p�[�e�B�N���ő吔


//*****************************************************************************
// �v���g�^�C�v�錾
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
