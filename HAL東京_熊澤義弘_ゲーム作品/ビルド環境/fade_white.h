//=============================================================================
//
// �z���C�g�t�F�[�h���� [fade_white.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************

// �t�F�[�h�̏��
typedef enum
{
	WHITE_NONE = 0,		// �����Ȃ����
	WHITE_IN,			// �t�F�[�h�C������
	WHITE_IN_bless,		// �t�F�[�h�C������(�u���X��)
	WHITE_OUT_charge,	// �t�F�[�h�A�E�g����(�`���[�W��)
	WHITE_OUT_blast,	// �t�F�[�h�A�E�g����(������)
	WHITE_MAX
} WHITE;



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitFadeWhite(void);
void UninitFadeWhite(void);
void UpdateFadeWhite(void);
void DrawFadeWhite(void);

void SetFadeWhite(WHITE fade);
WHITE GetFadeWhite(void);

void ResetInOut(void);


