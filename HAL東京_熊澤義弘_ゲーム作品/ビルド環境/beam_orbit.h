//=============================================================================
//
// �r�[���̋O������ [beam_orbit.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once
#define	MAX_BEAM_ORBIT				(1024 * 12)		// �p�[�e�B�N���ő吔


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBeamOrbit(void);
void UninitBeamOrbit(void);
void UpdateBeamOrbit(void);
void DrawBeamOrbit(void);

int SetBeamOrbit(XMFLOAT3 pos, float scl, int life);
void SetColorBeamOrbit(int nIdxParticle, XMFLOAT4 col);
void SetBeamOrbitSwitch(BOOL data);
void SetBeamOrbitDelete(void);

