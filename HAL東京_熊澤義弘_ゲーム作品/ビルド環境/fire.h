//=============================================================================
//
// �t�@�C�A���� [fire.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once
#define	MAX_FIRE		(512)		// �p�[�e�B�N���ő吔


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
	BOOL			Up;				// �㏸���Ă��邩�ǂ���
	BOOL			atk;			// �����蔻����s������

} FIRE;


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitFire(void);
void UninitFire(void);
void UpdateFire(void);
void DrawFire(void);

int SetFireParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, int nLife, float scaleX, float scaleY);
void SetColorFire(int nIdxParticle, XMFLOAT4 col);
void SetFireSwitch(BOOL data);
void ResetFire(int i);
FIRE *GetFire(void);
void ResetFireTime(void);

