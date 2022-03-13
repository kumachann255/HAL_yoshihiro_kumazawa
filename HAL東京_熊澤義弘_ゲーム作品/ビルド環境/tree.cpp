//=============================================================================
//
// �؂̏��� [tree.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "tree.h"
#include "lightning.h"
#include "bahamut.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_TREE_0			"data/MODEL/tree_0.obj"		// �ǂݍ��ރ��f����
#define	MODEL_TREE_1			"data/MODEL/tree_1.obj"		// �ǂݍ��ރ��f����
#define	MODEL_TREE_2			"data/MODEL/tree_2.obj"		// �ǂݍ��ރ��f����

#define	VALUE_ROTATE			(XM_PI * 0.02f)				// ��]��


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static TREE				g_Tree[MAX_TREE];				// ��

static BOOL				g_Load = FALSE;


//=============================================================================
// ����������
//=============================================================================
HRESULT InitTree(void)
{
	LoadModel(MODEL_TREE_0, &g_Tree[0].model, 0);
	g_Tree[0].load = TRUE;
	g_Tree[0].pos = XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f);
	g_Tree[0].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Tree[0].scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
	g_Tree[0].use = TRUE;			// TRUE:�����Ă�
	// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
	GetModelDiffuse(&g_Tree[0].model, &g_Tree[0].diffuse[0]);

	LoadModel(MODEL_TREE_1, &g_Tree[1].model, 0);
	g_Tree[1].load = TRUE;
	g_Tree[1].pos = XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f);
	g_Tree[1].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Tree[1].scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
	g_Tree[1].use = FALSE;			// �ŏ��͘��Վ��_�̈ה�\��
	// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
	GetModelDiffuse(&g_Tree[1].model, &g_Tree[1].diffuse[0]);

	LoadModel(MODEL_TREE_2, &g_Tree[2].model, 0);
	g_Tree[2].load = TRUE;
	g_Tree[2].pos = XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f);
	g_Tree[2].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Tree[2].scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
	g_Tree[2].use = TRUE;			// TRUE:�����Ă�
	// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
	GetModelDiffuse(&g_Tree[2].model, &g_Tree[2].diffuse[0]);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTree(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_TREE; i++)
	{
		if (g_Tree[i].load)
		{
			UnloadModel(&g_Tree[i].model);
			g_Tree[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateTree(void)
{
	// �G�l�~�[�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	for (int i = 0; i < MAX_TREE; i++)
	{
		if (g_Tree[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes

		}
	}

	if (GetSummon())
	{
		g_Tree[1].use = TRUE;
	}
	else if(GetOverLook())
	{
		g_Tree[1].use = FALSE;
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTree(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	for (int i = 0; i < MAX_TREE; i++)
	{
		if (g_Tree[i].use == FALSE) continue;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Tree[i].scl.x, g_Tree[i].scl.y, g_Tree[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Tree[i].rot.x, g_Tree[i].rot.y + XM_PI, g_Tree[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Tree[i].pos.x, g_Tree[i].pos.y, g_Tree[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Tree[i].mtxWorld, mtxWorld);

		// ���f���`��
		DrawModel(&g_Tree[i].model);

	}
}

//=============================================================================
// �G�l�~�[�̎擾
//=============================================================================
TREE *GetTree()
{
	return &g_Tree[0];
}


//=============================================================================
// ���_��������ւ������ɖ؂̕\����؂�ւ���
// TPS���_�ł͎�O�̖؂��\���ɂ���
//=============================================================================
void SetTreeSwitch(BOOL data)
{
	g_Tree[1].use = data;
}