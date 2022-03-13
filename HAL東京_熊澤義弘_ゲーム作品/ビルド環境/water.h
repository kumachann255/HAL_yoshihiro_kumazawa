//=============================================================================
//
// �E�H�[�^�[���� [water.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_WATER		(1)					// ���[�t�B���O�̐�
#define MAX_WATER_MOVE	(6)					// ���[�t�B���O�̍ő���

#define	WATER_SIZE		(50.0f)				// �����蔻��̑傫��


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct WATER
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�
	int					life;				// �G�ɓ������Ă��������܂ł̎���
	float				move_time;			// ���s����

	float				time;				// �x�W�F�Ȑ������̌o�ߎ���
	BOOL				firing;				// ����

};

struct WATER_MOVE
{
	INTERPOLATION_DATA	*tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	float				move_time;			// ���s����
};




//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitWater(void);
void UninitWater(void);
void UpdateWater(void);
void DrawWater(void);

WATER *GetWater(void);

void SetWaterVertex(DX11_MODEL *Model);
void SetWaterMove(void);
BOOL GetWaterMove(void);
void SetWater(void);
BOOL GetWaterUse(int i);
void SetFiringWater(void);
void SetPolkaDrop(int i);
void ResetWater(int i);
