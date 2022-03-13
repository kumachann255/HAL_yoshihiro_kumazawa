//=============================================================================
//
// �o�n���[�g���� [bahamut.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************

// �������@�̃V�[��(�J�������[�N�p)
enum {
	circle,
	pop,
	fold,
	open,
	charge,
	bless,
	bless_2camera,
	blast,
	end,
	sceneMAX,
};


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct BAHAMUT
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)
	XMFLOAT3			move;				// �ړ�����

	BOOL				use;
	BOOL				end;				// �f�B�]���u�J�n�t���O
	BOOL				load;
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
	float				dissolve;			// �f�B�]���u��臒l

	int					waterCount;			// �_�f�l(water�Ŏg�p)

	INTERPOLATION_DATA	*tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	float				move_time;			// ���s����

	// �e�́ANULL�A�q���͐e�̃A�h���X������
	BAHAMUT				*parent;	// �������e�Ȃ�NULL�A�������q���Ȃ�e��player�A�h���X

	BOOL				moveCan;			// �ړ��o���邩�ǂ���
	BOOL				turn;				// �ڕW�ɓ��B���Đ܂�Ԃ�����
	float				radian;				// �T�C���J�[�u�p�̃��W�A��
	float				hight;				// �G�l�~�[�̃x�[�X�̍���(�����_��)
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBahamut(void);
void UninitBahamut(void);
void UpdateBahamut(void);
void DrawBahamut(void);

BAHAMUT *GetBahamut(void);
BOOL GetSummon(void);
bool GetWingOpen(void);

void SetSummon(void);
void SetChargeMorphing(void);
void SetBlessMorphing(void);
void SetBlessEndMorphing(void);

void ResetBahamut(void);
