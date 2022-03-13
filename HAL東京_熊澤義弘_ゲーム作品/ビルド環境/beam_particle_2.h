//=============================================================================
//
// �r�[���̃p�[�e�B�N������ [beam_particle_2.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once

#define	MAX_BEAM_PARTICLE2			(512 * 5)	// �p�[�e�B�N���ő吔
//#define	MAX_BEAM_PARTICLE			(512 * 4)	// �p�[�e�B�N���ő吔


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct BEAM_PARTICLE_2
{
	XMFLOAT3		pos;			// �ʒu
	XMFLOAT3		basePos;		// �J�n�ʒu
	XMFLOAT3		rot;			// ��]
	XMFLOAT3		scale;			// �X�P�[��
	XMFLOAT3		move;			// �ړ���
	MATERIAL		material;		// �}�e���A��
	float			fSizeX;			// ��
	float			fSizeY;			// ����
	int				nIdxShadow;		// �eID
	int				nLife;			// ����
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���
	float			time;			// ����
	float			radius;			// ��]�̔��a
	float			radian;			// ��]�̊p�x

	XMFLOAT3		distance;		// �ړ�����



};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBeamParticle2(void);
void UninitBeamParticle2(void);
void UpdateBeamParticle2(void);
void DrawBeamParticle2(void);

int SetBeamParticle2(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT3 base, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);
void SetColorBeamParticle2(int nIdxParticle, XMFLOAT4 col);
void SetBeamParticleSwitch2(BOOL data);
void SetBeamParticleCharge2(BOOL data);
void SetBeam2Delete(void);
BEAM_PARTICLE_2 *GetBeamParticle2(void);
void SetChargeRadius2(float radius);
