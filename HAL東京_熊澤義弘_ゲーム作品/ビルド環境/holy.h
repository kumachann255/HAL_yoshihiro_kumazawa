//=============================================================================
//
// �z�[���[���� [holy.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_HOLY		(5)					// ���[�t�B���O�̐�
#define MAX_HOLY_MOVE	(2)					// ���[�t�B���O�̍ő���

#define	HOLY_SIZE		(7.0f)				// �����蔻��̑傫��


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct HOLY
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)
	XMFLOAT3			direct;				// ������G�܂ł̃x�N�g��

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�
	
	INTERPOLATION_DATA	*tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	float				move_time;			// ���s����

	float				time;				// �x�W�F�Ȑ������̌o�ߎ���
	BOOL				firing;				// ����

};

struct HOLY_MOVE
{
	INTERPOLATION_DATA	*tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	float				move_time;			// ���s����
};




//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitHoly(void);
void UninitHoly(void);
void UpdateHoly(void);
void DrawHoly(void);

HOLY *GetHoly(void);
void SetHoly(void);

void SetHolyVertex(DX11_MODEL *Model);
void SetMove(void);
BOOL GetMove(void);
void SetFiringSword(void);
XMFLOAT3 GetHolyMoveValue(int i);
