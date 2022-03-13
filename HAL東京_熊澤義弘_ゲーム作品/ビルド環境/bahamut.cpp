//=============================================================================
//
// �o�n���[�g���� [bahamut.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "bahamut.h"
#include "bahamut_particle.h"
#include "bahamut_bless.h"
#include "bless_particle.h"
#include "beam_orbit.h"
#include "beam_particle.h"
#include "beam_particle_2.h"
#include "lightning.h"
#include "player.h"
#include "sound.h"
#include "option.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_BAHAMUT		"data/MODEL/bahamut_body.obj"		// �ǂݍ��ރ��f����
#define	MODEL_BAHAMUT_WING_0		"data/MODEL/bahamut_wing_fold.obj"		// �ǂݍ��ރ��f����
#define	MODEL_BAHAMUT_WING_1		"data/MODEL/bahamut_wing_open_mid.obj"	// �ǂݍ��ރ��f����
#define	MODEL_BAHAMUT_WING_2		"data/MODEL/bahamut_wing_open.obj"		// �ǂݍ��ރ��f����
#define	MODEL_BAHAMUT_WING_3		"data/MODEL/bahamut_wing_base.obj"		// �ǂݍ��ރ��f����
#define MODEL_BAHAMUT_CIRCLE_0		"data/MODEL/bahamut_circle_0.obj"		// �ǂݍ��ރ��f����
#define MODEL_BAHAMUT_CIRCLE_1		"data/MODEL/bahamut_circle_1.obj"		// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(5.0f)				// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)		// ��]��

#define BAHAMUT_PARTS_MAX	(3)					// �v���C���[�̃p�[�c�̐�
#define MAX_BAHAMUT_MOVE	(4)					// �o�n���[�g�̗��̃��[�t�B���O���f����

#define ROT_SPEED			(0.01f)				// ���@�w����]�����鑬�x

#define POP_SPEED			(0.7f)				// �o�n���[�g���o�����n�߂鑬�x(�㏸���x)

#define BAHAMUT_BASE_HIGHT	(170.0f)			// �o�n���[�g�̍����̃x�[�X

#define BAHAMUT_DISSOLVE	(0.004f)			// ��̃f�B�]���u���x

#define BLESS_WAIT_TIME		(150)				// �������I����Ă���u���X���J�n����܂ł̎���
#define OPEN_SCENE_WAIT		(60)				// �u���X�V�[���ɍs���܂ł̑ҋ@����

#define CIRCLE_ROT_SPEED	(0.001f)				// ���@�w�̉�]���x

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
//void RunBahamutPartsAnimation(int i);
//void LoadParts(char *FileName, int i);
//void SetPopBahamut(int i);
//void SetBahamutVoice(void);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static BAHAMUT			g_Bahamut;							// �o�n���[�g

static BAHAMUT			g_Parts[BAHAMUT_PARTS_MAX];			// �o�n���[�g�̃p�[�c�p

static MODEL			g_Bahamut_Vertex[MAX_BAHAMUT_MOVE];	// ���[�t�B���O�p���f���̒��_��񂪓������f�[�^�z��
static VERTEX_3D		*g_Vertex = NULL;					// �r���o�߂��L�^����ꏊ

static BOOL				g_Summon;				// �������@���������Ă��邩�ǂ���
static float			g_time;					// ���[�t�B���O�̎��Ԃ��Ǘ�
static int				morphingNo;				// ���[�t�B���O�̃��f���ԍ����Ǘ�
static BOOL				g_Entry;				// �o�ꃂ�[�V�����Ɉڍs���邩�ǂ���
static int				g_StopTime;				// �����L�������Ɏ~�߂鎞�Ԃ��Ǘ�
static bool				g_WingOpen;				// �����L�������ǂ���

static int				g_BlessCount;			// �u���X�𔭎˂���܂ł̑ҋ@����
static BOOL				g_POP;					// �o�����n�߂邩�ǂ���
static BOOL				g_Charge;				// �u���X���`���[�W���Ă���^�C�~���O���ǂ���
static BOOL				g_Bless;				// �u���X�����s���Ă��邩�ǂ���

static BOOL				g_Load = FALSE;

static float			g_Dissolve;					// ��̃f�B�]���u�Ǘ�




//=============================================================================
// ����������
//=============================================================================
HRESULT InitBahamut(void)
{
	LoadModel(MODEL_BAHAMUT, &g_Bahamut.model, 0);
	g_Bahamut.load = TRUE;

	g_Bahamut.pos = XMFLOAT3(0.0f, BAHAMUT_BASE_HIGHT, 0.0f);
	g_Bahamut.rot = XMFLOAT3(0.0f, -XM_PI / 2, 0.0f);
	//ag_Bahamut.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Bahamut.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Bahamut.move = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Bahamut.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
	GetModelDiffuse(&g_Bahamut.model, &g_Bahamut.diffuse[0]);

	g_Bahamut.move_time = 0.0f;	// ���`��ԗp�̃^�C�}�[���N���A
	g_Bahamut.tbl_adr = NULL;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Bahamut.tbl_size = 0;		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

	g_Bahamut.use = TRUE;			// TRUE:�����Ă�
	g_Bahamut.end = FALSE;			// FALSE:�����Ă���

	// �K�w�A�j���[�V�����p�̏���������
	g_Bahamut.parent = NULL;			// �{�́i�e�j�Ȃ̂�NULL������

	// �f�B�]���u��臒l
	g_Bahamut.dissolve = 1.0f;

	// �ړ��ł��邩�̃t���O
	g_Bahamut.moveCan = TRUE;

	// �ڕW�ɓ��B���Đ܂�Ԃ�����
	g_Bahamut.turn = TRUE;

	// �T�C���J�[�u�p�̃��W�A��
	g_Bahamut.radian = 0.0f;


	// �p�[�c�̏�����
	for (int i = 0; i < BAHAMUT_PARTS_MAX; i++)
	{
		g_Parts[i].use = TRUE;

		// �ʒu�E��]�E�X�P�[���̏����ݒ�
		g_Parts[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		// �K�w�A�j���[�V�����p�̃����o�[�ϐ��̏�����
		g_Parts[i].tbl_adr = NULL;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[i].move_time = 0.0f;	// ���s���Ԃ��N���A
		g_Parts[i].tbl_size = 0;		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

		// �p�[�c�̓ǂݍ��݂͂܂����Ă��Ȃ�
		g_Parts[i].load = 0;
	}


	// �e�ւ̃y�A�����O
	g_Parts[0].parent = &g_Bahamut;		// �E���O���̐e�͖{��
	//g_Parts[1].parent = &g_Bahamut;	// �E���O���̐e�͖{��
	//g_Parts[2].parent = &g_Bahamut;	// �E���O���̐e�͖{��

	g_Parts[2].pos = XMFLOAT3(0.0f, -BAHAMUT_BASE_HIGHT - 50.0f, 0.0f);

	// �p�[�c���f���̓ǂݍ���
	LoadModel(MODEL_BAHAMUT_WING_0, &g_Parts[0].model, 1);
	LoadModel(MODEL_BAHAMUT_CIRCLE_0, &g_Parts[1].model, 0);
	LoadModel(MODEL_BAHAMUT_CIRCLE_1, &g_Parts[2].model, 0);


	// ���[�t�B���O����I�u�W�F�N�g�̓ǂݍ���
	LoadObj(MODEL_BAHAMUT_WING_0, &g_Bahamut_Vertex[0]);
	LoadObj(MODEL_BAHAMUT_WING_1, &g_Bahamut_Vertex[1]);
	LoadObj(MODEL_BAHAMUT_WING_2, &g_Bahamut_Vertex[2]);
	LoadObj(MODEL_BAHAMUT_WING_3, &g_Bahamut_Vertex[3]);

	// ���g��z��Ƃ��Ďg�p�ł���悤�Ɏd�l�ύX
	g_Vertex = new VERTEX_3D[g_Bahamut_Vertex[0].VertexNum];

	// ����(�r���o��)�̏�����
	for (int i = 0; i < g_Bahamut_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Bahamut_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse  = g_Bahamut_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal   = g_Bahamut_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Bahamut_Vertex[0].VertexArray[i].TexCoord;
	}

	// �ϐ��̏�����
	g_Summon = FALSE;
	g_time = 0.0f;
	morphingNo = 0;
	g_Entry = FALSE;
	g_StopTime = 0;
	g_POP = FALSE;
	g_Dissolve = 0.0f;
	g_WingOpen = false;
	g_Charge = FALSE;
	g_Bless = FALSE;

	//g_Parts[1].use = FALSE;
	//g_Parts[2].scl = XMFLOAT3(1.1f, 1.1f, 1.1f);
	//g_Parts[1].pos.y = 50.0f;
	//g_Parts[2].pos.y = 50.0f;

	//g_Parts[0].use = TRUE;
	//g_Parts[0].tbl_adr = move_tbl_wing0_light;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	//g_Parts[0].tbl_size = sizeof(move_tbl_wing0_light) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	//g_Parts[0].load = 1;


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitBahamut(void)
{
	if (g_Load == FALSE) return;

	if (g_Bahamut.load)
	{
		UnloadModel(&g_Bahamut.model);
		g_Bahamut.load = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateBahamut(void)
{
	// �������@���������Ă���ꍇ
	if (g_Summon)
	{
		PLAYER *player = GetPlayer();

		// ���󂵂Ă��閂�@�w�̏���
		if (g_Parts[1].use)
		{
			g_Parts[1].pos.x = g_Bahamut.pos.x;
			g_Parts[1].pos.z = g_Bahamut.pos.z;

			g_Parts[2].pos.x = g_Bahamut.pos.x;
			g_Parts[2].pos.z = g_Bahamut.pos.z;

			// ������]������
			g_Parts[1].rot.y += CIRCLE_ROT_SPEED;
			g_Parts[2].rot.y -= CIRCLE_ROT_SPEED;
		}

		// ���@�w�̃f�B�]���u����
		if (g_Dissolve < 1.0f)
		{
			g_Dissolve += BAHAMUT_DISSOLVE;
		}
		else
		{	// ���@�w������������o�n���[�g���o��������
			g_POP = TRUE;
			SetSoundFade(SOUND_LABEL_SE_BAHAMUT_circle, SOUNDFADE_OUT, 0.0f, 0.5f);
		}

		if ((g_POP) && (g_Parts[2].pos.y < BAHAMUT_BASE_HIGHT + 100.0f))
		{
			g_Parts[2].pos.y += POP_SPEED;

			// ���@�w���n�ʂ���o�Ă�����V�[���ύX
			if (g_Parts[2].pos.y > -BAHAMUT_BASE_HIGHT - 18.0f) SetSummonScene(pop);

			// �o�n���[�g�̑S�g���o��������V�[���ύX
			if (g_Parts[2].pos.y > BAHAMUT_BASE_HIGHT + 70.0f) SetSummonScene(fold);
		}
		else if (g_POP)
		{
			g_Entry = TRUE;
		}

		if (g_Bahamut.use == TRUE)			// ���̃o�n���[�g���g���Ă���H
		{									// Yes



				// ���[�t�B���O�̏���
			{
				int after, brfore;

				// ���[�t�B���O���f���̔ԍ�����
				if (g_Bless)
				{
					after = 2;
					brfore = 1;
				}
				else if (g_Charge)
				{
					after = 1;
					brfore = 3;
				}
				else
				{
					after = (morphingNo + 1) % MAX_BAHAMUT_MOVE;
					brfore = morphingNo % MAX_BAHAMUT_MOVE;
				}

				// ���[�t�B���O����
				for (int p = 0; p < g_Bahamut_Vertex[0].VertexNum; p++)
				{
					g_Vertex[p].Position.x = g_Bahamut_Vertex[after].VertexArray[p].Position.x - g_Bahamut_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y = g_Bahamut_Vertex[after].VertexArray[p].Position.y - g_Bahamut_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z = g_Bahamut_Vertex[after].VertexArray[p].Position.z - g_Bahamut_Vertex[brfore].VertexArray[p].Position.z;

					g_Vertex[p].Position.x *= g_time;
					g_Vertex[p].Position.y *= g_time;
					g_Vertex[p].Position.z *= g_time;

					g_Vertex[p].Position.x += g_Bahamut_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y += g_Bahamut_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z += g_Bahamut_Vertex[brfore].VertexArray[p].Position.z;
				}

				//// ���Ԃ�i�߂�
				if (g_Entry)
				{	// 1��̃��[�t�B���O�����s��
					if (g_time < 1.0f)
					{
						// �������X�ɍL����
						if (morphingNo == 0)	g_time += 0.012f;

						// ������C�ɍL����
						if (morphingNo == 1)
						{
							g_time += 0.15f;
							g_WingOpen = true;
							SetSummonScene(open);
						}

						// �x�[�X�̈ʒu�ɗ���߂��ۂɎ��Ԃɂ���đ��x��ς���
						if (g_StopTime <= 40)	g_time += 0.001f;
						if (g_StopTime > 40)	g_time += 0.018f;

						// �u���X���͈�C�ɗ����J��
						if (morphingNo == 3)	g_time += 0.15f;
					}
					else
					{
						// ���̃��[�t�B���O�ֈȍ~
						if (morphingNo < 2)
						{
							morphingNo++;
							g_time = 0.0f;
						}
					}

					// �����L���I����Ă���J�E���g�J�n
					if (morphingNo == 2) g_StopTime++;

					// ���[�t�B���O���I����Ă���J�E���g�J�n
					if ((morphingNo == 2) && (g_time >= 1.0f))
					{
						g_BlessCount++;

						// �u���X���J�n
						if (g_BlessCount == BLESS_WAIT_TIME) SetBlessMove();

						// �V�[����ύX
						if(g_BlessCount > OPEN_SCENE_WAIT) SetSummonScene(charge);
					}
				}

				D3D11_SUBRESOURCE_DATA sd;
				ZeroMemory(&sd, sizeof(sd));
				sd.pSysMem = g_Vertex;

				// ���_�o�b�t�@�ɒl���Z�b�g����
				D3D11_MAPPED_SUBRESOURCE msr;
				GetDeviceContext()->Map(g_Parts[0].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
				VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

				// �S���_���𖈉�㏑�����Ă���̂�DX11�ł͂��̕�����������ł�
				memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Bahamut_Vertex[0].VertexNum);

				GetDeviceContext()->Unmap(g_Parts[0].model.VertexBuffer, 0);
			}
		}
	}

	if ((GetKeyboardPress(DIK_J)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		SetSummon();
	}

#ifdef _DEBUG	// �f�o�b�O����\������

	if ((GetKeyboardPress(DIK_J)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		SetSummon();
	}

	if ((GetKeyboardPress(DIK_K)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_Summon = FALSE;
		g_Entry = FALSE;
		g_POP = FALSE;
		g_Charge = FALSE;
		g_Bless = FALSE;
		SetBlessParticleSwitch(FALSE);

		g_BlessCount = 0;
		morphingNo = 0;
		g_time = 0.0f;
		g_StopTime = 0;
		g_Dissolve = 0.0f;
		g_WingOpen = false;
		g_Bahamut.pos = XMFLOAT3(0.0f, BAHAMUT_BASE_HIGHT, 0.0f);
		g_Parts[2].pos = XMFLOAT3(0.0f, -BAHAMUT_BASE_HIGHT - 30.0f, 0.0f);
		SetBahamutParticleSwitch(FALSE);

		ResetSummonScene();

	}

	if ((GetKeyboardPress(DIK_H)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_POP = TRUE;
	}

	if ((GetKeyboardPress(DIK_I)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		morphingNo = 0;
		g_time = 0.0f;
		g_StopTime = 0;
		g_WingOpen = false;
		SetBahamutParticleSwitch(FALSE);
	}

	if ((GetKeyboardPress(DIK_N)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		ResetBless();
	}
#endif
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawBahamut(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���C�e�B���O�𖳌���
	SetLightEnable(TRUE);

	// ���e�X�g��L����
	SetAlphaTestEnable(TRUE);

	// �������C�gOn
	SetFuchi(1);

	// �V�F�[�_�[���[�h��ύX
	SetShaderMode(SHADER_MODE_BAHAMUT);

	// �萔�o�b�t�@���X�V
	SetShaderBahamutCircle(g_Parts[2].pos, g_WingOpen);

	//if (g_Bahamut.use == FALSE) return;

	if (g_Bahamut.use)
	{
		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Bahamut.scl.x, g_Bahamut.scl.y, g_Bahamut.scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Bahamut.rot.x, g_Bahamut.rot.y + XM_PI, g_Bahamut.rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Bahamut.pos.x, g_Bahamut.pos.y, g_Bahamut.pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Bahamut.mtxWorld, mtxWorld);

		// ���f���`��
		DrawModel(&g_Bahamut.model);

		// �p�[�c�̊K�w�A�j���[�V����
		for (int i = 0; i < BAHAMUT_PARTS_MAX; i++)
		{
			if ((i == 1) || (i == 2)) continue;

			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// ��]�𔽉f
			mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			if (g_Parts[i].parent != NULL)	// �q����������e�ƌ�������
			{
				mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
			}

			XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

			// �g���Ă���Ȃ珈������B�����܂ŏ������Ă��闝�R�͑��̃p�[�c�����̃p�[�c���Q�Ƃ��Ă���\�������邩��B
			if (g_Parts[i].use == FALSE) continue;

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// ���f���`��
			DrawModel(&g_Parts[i].model);
		}
	}


	// �������C�gOff
	SetFuchi(0);

	// �V�F�[�_�[���[�h��ύX
	SetShaderMode(SHADER_MODE_BAHAMUT_CIRCLE);

	for (int i = 1; i < 3; i++)
	{
		// �o�n���[�g�𕕈󂵂Ă��閂�@�w��`��
		if (g_Parts[i].use)
		{
			// ���Z�����ɐݒ�
			SetBlendState(BLEND_MODE_ADD);

			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// ��]�𔽉f
			mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			if (g_Parts[i].parent != NULL)	// �q����������e�ƌ�������
			{
				mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
			}

			XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �f�B�]���u�̐ݒ�
			SetDissolve(g_Dissolve);

			// ���f���`��
			DrawModel(&g_Parts[i].model);

			// �ʏ�u�����h�ɖ߂�
			SetBlendState(BLEND_MODE_ALPHABLEND);
		}
	}

	// ���e�X�g�𖳌���
	SetAlphaTestEnable(FALSE);

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// �o�n���[�g�̎擾
//=============================================================================
BAHAMUT *GetBahamut()
{
	return &g_Bahamut;
}


//=============================================================================
// �������@���������Ă��邩���擾
//=============================================================================
BOOL GetSummon(void)
{
	return g_Summon;
}


//=============================================================================
// �o�n���[�g�̗����J���Ă��邩�ǂ������擾
//=============================================================================
bool GetWingOpen(void)
{
	return g_WingOpen;
}


//=============================================================================
// �o�n���[�g�̏I������
//=============================================================================
void ResetBahamut(void)
{
	g_Summon = FALSE;
	g_Entry = FALSE;
	g_POP = FALSE;
	g_Charge = FALSE;
	g_Bless = FALSE;
	SetBlessParticleSwitch(FALSE);

	g_BlessCount = 0;
	morphingNo = 0;
	g_time = 0.0f;
	g_StopTime = 0;
	g_Dissolve = 0.0f;
	g_WingOpen = false;
	g_Bahamut.pos = XMFLOAT3(0.0f, BAHAMUT_BASE_HIGHT, 0.0f);
	g_Parts[2].pos = XMFLOAT3(0.0f, -BAHAMUT_BASE_HIGHT - 50.0f, 0.0f);
	SetBahamutParticleSwitch(FALSE);

	ResetSummonScene();
	ResetBless();
	SetBeamOrbitDelete();
	SetBeamDelete();
	SetBeam2Delete();
	SetBeamOrbitSwitch(FALSE);


	// �����̃��Z�b�g
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_circle,	1.0f * SetSoundPala(se));
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_pop,		1.0f * SetSoundPala(se));
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_open,	1.5f * SetSoundPala(se));
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_charge,	0.7f * SetSoundPala(se));
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_bless,	1.0f * SetSoundPala(se));

	//g_Bahamut.use = FALSE;			// TRUE:�����Ă�
	//g_Bahamut.end = TRUE;			// TRUE:�����Ă�

	//// �f�B�]���u��臒l
	//g_Bahamut.dissolve = 1.0f;

}


//=============================================================================
// �������@�𔭓�
//=============================================================================
void SetSummon(void)
{
	g_Summon = TRUE;

	// �o�n���[�g�̍��W���v���C���[�ɍ��킹��
	PLAYER *player = GetPlayer();
	// �o�n���[�g���G���A�O���U�����Ȃ��悤�ɒ���
	if (player->pos.x > MAP_CIRCLE_LEFT)
	{
		g_Bahamut.pos.x = MAP_CIRCLE_LEFT;
	}
	else
	{
		g_Bahamut.pos.x = player->pos.x;
	}

	

	// BGM���t�F�[�h�A�E�g
	SetTitleSoundFade(SOUND_LABEL_BGM_game, SOUNDFADE_OUT, 0.0f, 1.5f);
}


//=============================================================================
// �u���X���`���[�W���Ă���Ƃ��Ƀ��[�t�B���O��ύX����
//=============================================================================
void SetChargeMorphing(void)
{
	if (g_Charge) return;
	g_Charge = TRUE;
	g_time = 0.0f;
	g_BlessCount = 0;
}


//=============================================================================
// �u���X�����s���Ă���Ƃ��Ƀ��[�t�B���O��ύX����
//=============================================================================
void SetBlessMorphing(void)
{
	if (g_Bless) return;
	g_Bless = TRUE;
	g_time = 0.0f;
	morphingNo = 3;
}


//=============================================================================
// �u���X���I�������Ƃ��Ƀ��[�t�B���O��ύX����
//=============================================================================
void SetBlessEndMorphing(void)
{
	if (!g_Bless) return;
	g_Charge = FALSE;
	g_Bless = FALSE;
	morphingNo = 2;
	g_time = 0.0f;
}

