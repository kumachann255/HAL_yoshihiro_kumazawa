//=============================================================================
//
// �o�n���[�g�̃u���X���� [bahamut_bless.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "bahamut_bless.h"
#include "bless_particle.h"
#include "player.h"
#include "enemy.h"
#include "math.h"
#include "collision.h"
#include "sound.h"
#include "tutorial.h"
#include "debugproc.h"
#include "fade_white.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_BLAST			"data/MODEL/bahamut_blast.obj"			// �ǂݍ��ރ��f����

#define	MODEL_SURGE			"data/MODEL/bahamut_surge.obj"			// �ǂݍ��ރ��f����

#define BLESS_SCALE_COUNT	(200)						// ���ʂ���������܂ł̃t���[����

#define BLESS_COLOR_MIN		(0.55f)						// �F�̉���
#define BLESS_COLOR_SPEED	(0.001f)					// �F�̕ω����鑬�x

#define BLAST_ROT_SPEED		(0.5f)						// ��]���x
#define BLAST_SCALE_SPEED	(0.18f)						// �g�呬�x
#define BLAST_SCALE_MAX		(50.0f)						// �g��̍ő�l
#define BLAST_SCALE_FADE_START	(20.0f)					// �t�F�[�h�A�E�g���n�܂�傫��

#define BLESS_SCENE_WAIT_1	(25)						// �u���X�V�[��(1�J��)�̎���
#define BLESS_SCENE_WAIT_2	(60)						// �u���X�V�[��(2�J��)�̎���

#define SURGE_DISSOLVE_SPEED	(0.02f)					// �f�B�]���u�̑���
#define SURGE_ROT_SPEED			(0.05f)					// ��]�̑���
#define SURGE_POS_SPEED			(20)					// �ړ��̑���(���t���[���ŖړI�n�ɓ��B���邩)

#define BLAST_A_LINE			(1.0f)					// �����ɂ������C���̉��~���x

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static BLESS		g_Blast;						// ����
static BLESS		g_Surge;						// �g��

static BOOL			g_Load = FALSE;

static MODEL		g_Bless_Vertex[MAX_BLESS_MOVE];	// ���[�t�B���O�p���f���̒��_��񂪓������f�[�^�z��
static VERTEX_3D	*g_Vertex = NULL;				// �r���o�߂��L�^����ꏊ

static ID3D11Buffer	*g_VertexBuffer = NULL;			// ���_�o�b�t�@

static BOOL			g_move = FALSE;
static int			morphing;

static int			g_SceneWait;					// ���̃V�[���ɐi�ނ܂ł̑ҋ@����

static float		g_SurgeDissolve;				// �g����臒l
static BOOL			g_SurgeFiring;					// �g�����������ǂ���
static XMFLOAT3		g_SurgeMove;					// �g���̓������x

static int			g_count = 0;

static XMFLOAT3		g_ALine;						// �����̓����������l

//=============================================================================
// ����������
//=============================================================================
HRESULT InitBless(void)
{
	// �����̏�����
	LoadModel(MODEL_BLAST, &g_Blast.model, 0);
	g_Blast.load = TRUE;

	g_Blast.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Blast.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Blast.scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Blast.diffuse[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_Blast.move_time = 0.0f;

	g_Blast.use = FALSE;			// TRUE:�����Ă�


	// �g���̏�����
	LoadModel(MODEL_SURGE, &g_Surge.model, 0);
	g_Surge.load = TRUE;

	g_Surge.pos = XMFLOAT3(0.0f, 100.0f, 0.0f);
	g_Surge.rot = XMFLOAT3(XM_PI / 5, -XM_PI / 2, 0.0f);
	g_Surge.scl = XMFLOAT3(1.4f, 1.4f, 1.4f);
	g_Surge.diffuse[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_Surge.move_time = 0.0f;

	g_Surge.use = FALSE;			// TRUE:�����Ă�

	morphing = 0;
	g_move = FALSE;
	g_SurgeDissolve = 1.0f;
	g_SurgeFiring = FALSE;
	g_SurgeMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_ALine = XMFLOAT3(0.0f, 400.0f, 0.0f);

	g_Load = TRUE;


	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitBless(void)
{
	if (g_Load == FALSE) return;

	if (g_Blast.load)
	{
		UnloadModel(&g_Blast.model);
		g_Blast.load = FALSE;
	}

	if (g_Surge.load)
	{
		UnloadModel(&g_Surge.model);
		g_Surge.load = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateBless(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();
	BAHAMUT *bahamut = GetBahamut();

	if (g_move)
	{
		// ���Ԃ�i�߂�
		g_count++;

		if (g_count > 410)
		{
			// �V�[����ύX
			SetSummonScene(bless);
		}
		
		if (g_count > 460)
		{
			SetSummonScene(bless_2camera);
		}

		if (g_count > 505)
		{
			SetSummonScene(blast);
		}

		if (g_count > 555)
		{
			// ������`��
			if (!g_Blast.use)
			{
				g_Blast.use = TRUE;

				// �u���X�ƒn�ʂ��ݒu���Ă���ꏊ�ɔ�����z�u
				g_Blast.pos.x = bahamut->pos.x + BLAST_X_OFFSET + 150.0f;
			}
		}
	}


	if (g_Blast.use)
	{
		if (g_Blast.scl.x < BLAST_SCALE_MAX)
		{
			// ��]���Ȃ���傫���Ȃ�
			g_Blast.scl.x += BLAST_SCALE_SPEED;
			g_Blast.scl.y += BLAST_SCALE_SPEED;
			g_Blast.scl.z += BLAST_SCALE_SPEED;

			g_Blast.rot.y += BLAST_ROT_SPEED;

			if (g_Blast.scl.x > BLAST_SCALE_FADE_START)
			{
				SetFadeWhite(WHITE_OUT_blast);
			}

			// ��l��������������
			g_ALine.y -= BLAST_A_LINE;

		}
		else
		{
			ResetBless();
		}
	}

	if (g_Surge.use)
	{
		// �g�����ˑO
		if (!g_SurgeFiring)
		{
			// �f�B�]���u��臒l���~�炷
			g_SurgeDissolve += SURGE_DISSOLVE_SPEED;
			if (g_SurgeDissolve > 1.0f) g_SurgeDissolve -= 1.0f;
		}
		else
		{	// �g�����ˌ�
			g_Surge.rot.x += SURGE_ROT_SPEED;

			// �ړ�
			g_Surge.pos.x -= g_SurgeMove.x;
			g_Surge.pos.y -= g_SurgeMove.y;
		}
	}

#ifdef _DEBUG	// �f�o�b�O����\������
	PrintDebugProc("Dissolve :%f  x:%f z:%f\n", g_SurgeDissolve, g_Surge.pos.x, g_Surge.pos.z);


#endif

}


//=============================================================================
// �`�揈��
//=============================================================================
void DrawBless(void)
{
	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ���e�X�g��L����
	SetAlphaTestEnable(TRUE);

	// ���C�e�B���O�𖳌���
	SetLightEnable(FALSE);

	// �t�H�O����
	SetFogEnable(FALSE);

	// �����̕`�揈��
	if(g_Blast.use)
	{
		// �������C�gOn
		SetFuchi(1);

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Blast.scl.x, g_Blast.scl.y, g_Blast.scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Blast.rot.x, g_Blast.rot.y + XM_PI, g_Blast.rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Blast.pos.x, g_Blast.pos.y, g_Blast.pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		XMStoreFloat4x4(&g_Blast.mtxWorld, mtxWorld);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		// �����ɂ����l���X�V
		SetShaderBahamutCircle(g_ALine, TRUE);

		// ���f���`��
		DrawModel(&g_Blast.model);

		// �������C�gOff
		SetFuchi(0);
	}

	// �g���̕`�揈��
	//if (g_Surge.use)
	//{
	//	// �V�F�[�_�[�̕ύX
	//	if(!g_SurgeFiring) SetShaderMode(SHADER_MODE_BAHAMUT_SURGE);

	//	// ���Z�����ɐݒ�
	//	SetBlendState(BLEND_MODE_ADD);

	//	// ���[���h�}�g���b�N�X�̏�����
	//	mtxWorld = XMMatrixIdentity();

	//	// �X�P�[���𔽉f
	//	mtxScl = XMMatrixScaling(g_Surge.scl.x, g_Surge.scl.y, g_Surge.scl.z);
	//	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	//	// ��]�𔽉f
	//	mtxRot = XMMatrixRotationRollPitchYaw(g_Surge.rot.x, g_Surge.rot.y + XM_PI, g_Surge.rot.z);
	//	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	//	// �ړ��𔽉f
	//	mtxTranslate = XMMatrixTranslation(g_Surge.pos.x, g_Surge.pos.y, g_Surge.pos.z);
	//	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	//	XMStoreFloat4x4(&g_Surge.mtxWorld, mtxWorld);

	//	// ���[���h�}�g���b�N�X�̐ݒ�
	//	SetWorldMatrix(&mtxWorld);

	//	// �f�B�]���u�̐ݒ�
	//	SetDissolve(g_SurgeDissolve);

	//	// ���f���`��
	//	DrawModel(&g_Surge.model);

	//	// �ʏ�u�����h�ɖ߂�
	//	SetBlendState(BLEND_MODE_ALPHABLEND);
	//}

	// ���e�X�g�𖳌���
	SetAlphaTestEnable(FALSE);

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// �t�H�O�L��
	SetFogEnable(GetFogSwitch());

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// �����̎擾
//=============================================================================
BLESS *GetBlast(void)
{
	return &g_Blast;
}


//=============================================================================
// ���_���̏�������
//=============================================================================
void SetBlessVertex(DX11_MODEL *Model)
{
	// ���_�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * g_Bless_Vertex[0].VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_Bless_Vertex[0].VertexArray;

		GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer);
	}
}


//=============================================================================
// �`���[�W�J�n
//=============================================================================
void SetBlessMove(void)
{
	g_move = TRUE;
	SetBlessParticleSwitch(TRUE);
}

//=============================================================================
// ���[�t�B���O���������Ă��邩
//=============================================================================
BOOL GetBlessMove(void)
{
	return g_move;
}


//=============================================================================
// �u���X�̃��Z�b�g����
//=============================================================================
void ResetBless(void)
{
	// ���Ԃ����Z�b�g
	g_SceneWait = 0;
	g_count = 0;
	g_move = FALSE;

	// ���������Z�b�g
	g_Blast.use = FALSE;
	g_Blast.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Blast.scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_ALine = XMFLOAT3(0.0f, 400.0f, 0.0f);

	// �g�������Z�b�g
	g_Surge.use = FALSE;
	g_Surge.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Surge.rot = XMFLOAT3(-XM_PI / 5, -XM_PI / 2, 0.0f);

	g_SurgeDissolve = 0.0f;
	g_SurgeFiring = FALSE;
	g_SurgeMove = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// BGM���t�F�[�h�A�E�g
	SetSoundFade(SOUND_LABEL_BGM_bahamut, SOUNDFADE_OUT, 0.0f, 1.0f);
}


//=============================================================================
// �g�����΂�
//=============================================================================
void SetSurge(void)
{
	g_SurgeFiring = TRUE;

	// ���W����
	BAHAMUT *bahamut = GetBahamut();
	g_Surge.pos = bahamut->pos;
	g_Surge.pos.y += 125.0f;
	g_Surge.pos.x += 85.0f;
}
