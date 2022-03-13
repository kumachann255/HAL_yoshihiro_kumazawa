//=============================================================================
//
// ���f������ [player.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_PLAYER		(1)					// �v���C���[�̐�

#define	PLAYER_SIZE		(10.0f)				// �����蔻��̑傫��

// ���@�̔ԍ�
enum {
	null,
	fire,
	lightning,
	water,
	holy,
	summon,
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct PLAYER
{
	XMFLOAT3			pos;		// �|���S���̈ʒu
	XMFLOAT3			rot;		// �|���S���̌���(��])
	XMFLOAT3			scl;		// �|���S���̑傫��(�X�P�[��)

	XMFLOAT4X4			mtxWorld;	// ���[���h�}�g���b�N�X

	BOOL				load;
	DX11_MODEL			model;		// ���f�����

	float				spd;		// �ړ��X�s�[�h
	float				dir;		// ����
	float				size;		// �����蔻��̑傫��
	int					HP;			// �q�b�g�|�C���g
	BOOL				hitCheck;	// �����蔻�肪�o�����Ԃ�(���G���Ԃł͂Ȃ����ǂ���)
	int					hitTime;	// ���G���Ԃ̊Ǘ�
	BOOL				use;


	// �K�w�A�j���[�V�����p�̃����o�[�ϐ�
	int					tbl_size;	// �o�^�����e�[�u���̃��R�[�h����
	float				move_time;	// ���s����
	INTERPOLATION_DATA	*tbl_adr;	// �A�j���f�[�^�̃e�[�u���擪�A�h���X

	// �e�́ANULL�A�q���͐e�̃A�h���X������
	PLAYER				*parent;	// �������e�Ȃ�NULL�A�������q���Ȃ�e��player�A�h���X

	// �N�H�[�^�j�I��
	XMFLOAT4			quaternion;	// �N�H�[�^�j�I��
	XMFLOAT3			upVector;	// �����������Ă��鏊


};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER *GetPlayer(void);
void SetGait(BOOL data);
int GetHP(void);

