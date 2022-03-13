//=============================================================================
//
// ��̗����� [wand_lightning.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once

#define	MAX_WAND_LIGHTNING			(35)	// �p�[�e�B�N���ő吔
#define	MAX_WAND_REFRACTION			(6)	// �p�[�e�B�N���ő吔
//#define	MAX_BEAM_PARTICLE			(512 * 4)	// �p�[�e�B�N���ő吔


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct WAND_LIGHTNING
{
	XMFLOAT3		pos;			// �ʒu
	XMFLOAT3		rot;			// ��]
	XMFLOAT3		scale;			// �X�P�[��
	MATERIAL		material;		// �}�e���A��
	float			fSizeX;			// ��
	float			fSizeY;			// ����
	int				nLife;			// ����
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���


	XMFLOAT3		ControlPos0;	// �J�n�ʒu
	XMFLOAT3		ControlPos1;	// ����_1 : ��_����ǂ̂��炢����Ă��邩
	XMFLOAT3		ControlPos2;	// ����_2
	XMFLOAT3		ControlPos3;	// ����_3
	XMFLOAT3		ControlPos4;	// ����_4
	XMFLOAT3		ControlPos5;	// ����_5
	XMFLOAT3		EndPos		;	// �I���_

	XMFLOAT3		distance;		// �ړ�����

};

struct WAND_ORBIT
{
	XMFLOAT3		pos;			// �ʒu
	XMFLOAT3		rot;			// ��]
	XMFLOAT3		scale;			// �X�P�[��
	MATERIAL		material;		// �}�e���A��
	float			fSizeX;			// ��
	float			fSizeY;			// ����
	int				nLife;			// ����
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���
};



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitWandLightning(void);
void UninitWandLightning(void);
void UpdateWandLightning(void);
void DrawWandLightning(void);

int SetWandOrbit(XMFLOAT3 pos, XMFLOAT4 col, float fSizeX, float fSizeY);
int SetWandLightning(XMFLOAT3 pos, XMFLOAT3 conpos1, XMFLOAT3 conpos2, XMFLOAT3 conpos3, XMFLOAT3 conpos4, XMFLOAT3 conpos5, XMFLOAT4 col, float fSize);
void SetColorWandLightning(int nIdxParticle, XMFLOAT4 col);
void SetWandLightningSwitch(BOOL data);
//void SetBeamDelete(void);
WAND_LIGHTNING *GetWandLightning(void);
void SetChargeRadius(float radius);
BOOL GetBeamOnOff(void);
