//=============================================================================
//
// �o�n���[�g�̃u���X���� [bahamut_bless.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once

#include "bahamut.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_BLESS		(1)					// ���[�t�B���O�̐�
#define MAX_BLESS_MOVE	(6)					// ���[�t�B���O�̍ő���

#define BLAST_X_OFFSET	(590.0f)			// �����̈ʒu����

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct BLESS
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				move_time;			// ���s����
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBless(void);
void UninitBless(void);
void UpdateBless(void);
void DrawBless(void);

BLESS *GetBlast(void);

void SetBlessVertex(DX11_MODEL *Model);
void SetBlessMove(void);
BOOL GetBlessMove(void);
void ResetBless(void);
void SetSurge(void);
