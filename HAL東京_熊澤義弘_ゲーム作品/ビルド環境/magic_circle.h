//=============================================================================
//
// ���@�w���� [magic_circle.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once
#define	MAX_CIRCLE		(1)		// �p�[�e�B�N���ő吔


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
typedef struct
{
	XMFLOAT3		pos;			// �ʒu
	XMFLOAT3		rot;			// ��]
	XMFLOAT3		scale;			// �X�P�[��
	MATERIAL		material;		// �}�e���A��
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���
	BOOL			end;			// �����n�߂邩�ǂ���
	float			dissolve;		// �f�B�]���u��臒l

} CIRCLE;


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitCircle(void);
void UninitCircle(void);
void UpdateCircle(void);
void DrawCircle(void);

void SetCircle(BOOL use, BOOL end, int magic);
void SetColorCircle(XMFLOAT4 col);
void ResetCircle(void);
CIRCLE *GetCircle(void);

