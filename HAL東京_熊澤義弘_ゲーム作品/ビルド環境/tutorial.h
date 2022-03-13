//=============================================================================
//
// �`���[�g���A������ [tutorial.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TUTORIAL_X		(290.0f)		// �`���[�g���A�����ɓ�����x���W�̍ő� 

enum {
	tx_start,
	tx_fire,
	tx_lightning,
	tx_water,
	tx_holy,
	tx_clear,
	tx_MAX,
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitTutorial(void);
void UninitTutorial(void);
void UpdateTutorial(void);
void DrawTutorial(void);

BOOL GetTutorialUse(void);
BOOL GetTutorialClear(int tutorial);
void SetTutorial(int tutorial);
void SetTutorialClear(int tutorial);

