//=============================================================================
//
// ���C�g�j���O���� [lightning.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once

#define MAX_LIGHTNING			(4)			// �����ő唭����
#define	MAX_REFRACTION			(30)		// ���C�g�j���O�ő���ܐ�
#define MAX_BIFURCATION			(2)			// ����ł����

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
typedef struct
{
	XMFLOAT3		pos;			// �ʒu
	XMFLOAT3		rot;			// ��]
	XMFLOAT3		scale;			// �X�P�[��
	XMFLOAT3		move;			// �ړ���
	MATERIAL		material;		// �}�e���A��
	float			fSizeX;			// ��
	float			fSizeY;			// ����
	int				nIdxShadow;		// �eID
	int				nLife;			// ����
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���

	int				vecType;		// �ړ������̃x�N�g���̎��
	int				count;			// ����܂ł̃J�E���g���Ǘ�


} LIGHTNING;

typedef struct
{
	XMFLOAT3		pos;			// �ʒu
	float			fSizeX;			// �����蔻��̉���
	float			fSizeY;			// �����蔻��̍���
	float			fSizeZ;			// �����蔻��̉��s��
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���
} LIGHTNING_COLLI;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitLightning(void);
void UninitLightning(void);
void UpdateLightning(void);
void DrawLightning(void);

LIGHTNING_COLLI *GetLightningCollition(void);
void SetLightning(void);
void SetUpdateLightning(int z, int vectype);
//void SetColorLightning(int nIdxParticle, XMFLOAT4 col);
float RamdomFloat(int digits, float max, float min);
void ResetLightning(int z);
BOOL GetLightningOnOff(void);
void SetLightningOnOff(BOOL data);

