//=============================================================================
//
// �X�J�C�h�[���̏��� [skydome.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "skydome.h"
#include "lightning.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_SKY				"data/MODEL/sky.obj"		// �ǂݍ��ރ��f����

#define	VALUE_ROTATE			(XM_PI * 0.02f)				// ��]��

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static SKY				g_Sky;				// ��

static BOOL				g_Load = FALSE;


//=============================================================================
// ����������
//=============================================================================
HRESULT InitSky(void)
{
	LoadModel(MODEL_SKY, &g_Sky.model, 0);
	g_Sky.load = TRUE;
	g_Sky.pos = XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f);
	g_Sky.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Sky.scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
	g_Sky.use = TRUE;			// TRUE:�����Ă�
	// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
	GetModelDiffuse(&g_Sky.model, &g_Sky.diffuse[0]);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitSky(void)
{
	if (g_Load == FALSE) return;

	UnloadModel(&g_Sky.model);
	g_Sky.load = FALSE;

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateSky(void)
{

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawSky(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// �J�����O����
	SetCullingMode(CULL_MODE_FRONT);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Sky.scl.x, g_Sky.scl.y, g_Sky.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_Sky.rot.x, g_Sky.rot.y + XM_PI, g_Sky.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Sky.pos.x, g_Sky.pos.y, g_Sky.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Sky.mtxWorld, mtxWorld);

	// ���f���`��
	DrawModel(&g_Sky.model);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// �G�l�~�[�̎擾
//=============================================================================
SKY *GetSky()
{
	return &g_Sky;
}
