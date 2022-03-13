//=============================================================================
//
// �r�[���̃p�[�e�B�N������ [beam_particle.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once

#define	MAX_BEAM_PARTICLE			(512 * 5)	// �p�[�e�B�N���ő吔
//#define	MAX_BEAM_PARTICLE			(512 * 4)	// �p�[�e�B�N���ő吔


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct BEAM_PARTICLE
{
	XMFLOAT3		pos;			// �ʒu
	XMFLOAT3		rot;			// ��]
	XMFLOAT3		scale;			// �X�P�[��
	XMFLOAT3		baseMove;		// ��ړ��x�N�g��
	XMFLOAT3		move;			// �ړ���
	MATERIAL		material;		// �}�e���A��
	float			fSizeX;			// ��
	float			fSizeY;			// ����
	int				nIdxShadow;		// �eID
	int				nLife;			// ����
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���
	float			time;			// ����


	XMFLOAT3		ControlPos0;	// �J�n�ʒu
	XMFLOAT3		ControlPos1;	// ����_1
	XMFLOAT3		ControlPos2;	// ����_2
	XMFLOAT3		ControlPos3;	// ����_3
	XMFLOAT3		distance;		// �ړ�����
	BOOL			first;			// �������čŏ��̃t���[�����ǂ���



};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBeamParticle(void);
void UninitBeamParticle(void);
void UpdateBeamParticle(void);
void DrawBeamParticle(void);

int SetBeamParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT3 base, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife);
void SetColorBeamParticle(int nIdxParticle, XMFLOAT4 col);
void SetBeamParticleSwitch(BOOL data);
void SetBeamParticleCharge(BOOL data);
void SetBeamDelete(void);
BEAM_PARTICLE *GetBeamParticle(void);
void SetChargeRadius(float radius);
BOOL GetBeamOnOff(void);
