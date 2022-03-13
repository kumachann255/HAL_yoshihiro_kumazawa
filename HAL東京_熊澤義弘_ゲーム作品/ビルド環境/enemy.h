//=============================================================================
//
// �G�l�~�[���f������ [enemy.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_WAVE		(4)					// �E�F�C�u��
#define MAX_POP_WAVE1	(3)					// 1�E�F�C�u�ŉ���POP�����邩
#define MAX_POP_WAVE2	(8)					// 2�E�F�C�u�ŉ���POP�����邩
#define MAX_POP_WAVE3	(10)				// 3�E�F�C�u�ŉ���POP�����邩
#define MAX_POP_WAVE4	(15)				// 4�E�F�C�u�ŉ���POP�����邩

// �f�o�b�N�p
//#define MAX_POP_WAVE1	(1)					// 1�E�F�C�u�ŉ���POP�����邩
//#define MAX_POP_WAVE2	(1)					// 2�E�F�C�u�ŉ���POP�����邩
//#define MAX_POP_WAVE3	(1)				// 3�E�F�C�u�ŉ���POP�����邩
//#define MAX_POP_WAVE4	(1)				// 4�E�F�C�u�ŉ���POP�����邩

#define MAX_ENEMY		(MAX_POP_WAVE1 + MAX_POP_WAVE2 + MAX_POP_WAVE3 + MAX_POP_WAVE4)		// �G�l�~�[�̐�

#define	ENEMY_SIZE		(30.0f)				// �����蔻��̑傫��


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct ENEMY
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
	ENEMY				*parent;	// �������e�Ȃ�NULL�A�������q���Ȃ�e��player�A�h���X

	BOOL				moveCan;			// �ړ��o���邩�ǂ���
	BOOL				turn;				// �ڕW�ɓ��B���Đ܂�Ԃ�����
	float				radian;				// �T�C���J�[�u�p�̃��W�A��
	float				hight;				// �G�l�~�[�̃x�[�X�̍���(�����_��)

	int					wave;				// �ǂ̃E�F�[�u�ŏo�������邩
	int					popCount;			// �E�F�[�u���J�n����Ă���ǂ̃^�C�~���O�Ŕ��������邩
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY *GetEnemy(void);

