//=============================================================================
//
// ���K�C�h�̏��� [arrow.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "arrow.h"
#include "lightning.h"
#include "player.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_ARROW				"data/MODEL/arrow.obj"		// �ǂݍ��ރ��f����

#define	VALUE_ROTATE			(XM_PI * 0.02f)				// ��]��

#define DESTINATION_X			(2200.0f)					// �ړI�n��x���W

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ARROW			g_Arrow;				// ��

static BOOL				g_Load = FALSE;


//=============================================================================
// ����������
//=============================================================================
HRESULT InitArrow(void)
{
	LoadModel(MODEL_ARROW, &g_Arrow.model, 0);
	g_Arrow.load = TRUE;
	g_Arrow.pos = XMFLOAT3(0.0f, 70.0f, 0.0f);
	g_Arrow.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Arrow.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Arrow.use = TRUE;			// TRUE:�����Ă�
	// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
	GetModelDiffuse(&g_Arrow.model, &g_Arrow.diffuse[0]);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitArrow(void)
{
	if (g_Load == FALSE) return;

	UnloadModel(&g_Arrow.model);
	g_Arrow.load = FALSE;

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateArrow(void)
{
	// �G�l�~�[�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	if (g_Arrow.use == TRUE)			// ���̃G�l�~�[���g���Ă���H
	{									// Yes
		PLAYER *player = GetPlayer();
		CAMERA *cam = GetCamera();

		// ��ʂ̏�ɍ��킹��
		g_Arrow.pos.x = player->pos.x;
		g_Arrow.pos.z = player->pos.z;

		// �ړI�n�̕���
		float disX, disZ;
		disX = DESTINATION_X - player->pos.x;
		disZ = -player->pos.z;

		// y����]�̒���
		if (disZ > 0.0f)
		{
			g_Arrow.rot.y = (XM_PI) + atanf(disX / disZ);
		}
		else if(disX < 0.0f)
		{
			g_Arrow.rot.y = (XM_PI / 2) - atanf(disZ / disX);
		}
		else
		{
			g_Arrow.rot.y = (-XM_PI / 2) - atanf(disZ / disX);
		}


	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawArrow(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	if (g_Arrow.use == FALSE) return;

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Arrow.scl.x, g_Arrow.scl.y, g_Arrow.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_Arrow.rot.x, g_Arrow.rot.y + XM_PI, g_Arrow.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Arrow.pos.x, g_Arrow.pos.y, g_Arrow.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Arrow.mtxWorld, mtxWorld);

	// ���f���`��
	DrawModel(&g_Arrow.model);
}

//=============================================================================
// �G�l�~�[�̎擾
//=============================================================================
ARROW *GetArrow()
{
	return &g_Arrow;
}


// ���_��������ւ������ɖ؂̕\����؂�ւ���
// TPS���_�ł͎�O�̖؂��\���ɂ���
void SetArrowSwitch(BOOL data)
{
	g_Arrow.use = data;
}