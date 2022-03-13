//=============================================================================
//
// �G�l�~�[���f������ [enemy.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "enemy.h"
#include "lightning.h"
#include "player.h"
#include "sound.h"
#include "bahamut.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/enemy0_body.obj"		// �ǂݍ��ރ��f����
#define	MODEL_PARTS_WING_0	"data/MODEL/enemy0_wing0.obj"		// �ǂݍ��ރ��f����
#define	MODEL_PARTS_WING_1	"data/MODEL/enemy0_wing1.obj"		// �ǂݍ��ރ��f����
#define	MODEL_TREE			"data/MODEL/tree.obj"		// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(1.0f)				// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)		// ��]��

#define ENEMY_SHADOW_SIZE	(0.4f)				// �e�̑傫��
#define ENEMY_OFFSET_Y		(40.0f)				// �G�l�~�[�̑��������킹��

#define ENEMY_PARTS_MAX		(4)					// �v���C���[�̃p�[�c�̐�

#define DISSOLVE_SPEED		(0.007f)			// �f�B�]���u�̑��x
#define MAX_WATER_COUNT		(200)				// �_�f�l

#define TURN_DISTANCE		(300.0f)			// �G�l�~�[���܂�Ԃ��|�C���g
#define TURN_OFFSET			(5.0f)				// �G�l�~�[���܂�Ԃ��|�C���g��傫������
#define TARGET_SPEED		(300)				// ���t���[���ŖڕW�܂œ��B���邩

#define ENEMY_BASE_HIGHT	(70.0f)				// �G�l�~�[�̍����̃x�[�X

#define MAX_POS_WAVE1		(300.0f)			// 1�E�F�C�u�ŉ���POP�����邩
#define MAX_POS_WAVE2		(700.0f)			// 2�E�F�C�u�ŉ���POP�����邩
#define MAX_POS_WAVE3		(1200.0f)			// 3�E�F�C�u�ŉ���POP�����邩
#define MAX_POS_WAVE4		(1950.0f)			// 4�E�F�C�u�ŉ���POP�����邩

#define POP_DISTANCE		(100)				// �G�l�~�[�̃|�b�v����Ԋu

#define ENEMY_VOICE_MAX		(4)					// �G�l�~�[�̃{�C�X�̎��

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void RunEnemyPartsAnimation(int i);
void ResetEnemy(int i);
void LoadParts(char *FileName, int i);
void SetPopEnemy(int i);
void SetEnemyVoice(void);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// �G�l�~�[

static ENEMY			g_Parts[MAX_ENEMY][ENEMY_PARTS_MAX];		// �v���C���[�̃p�[�c�p

static BOOL				g_Load = FALSE;

static XMFLOAT3			g_targetPos[MAX_ENEMY];

static int				g_WaveCount[MAX_WAVE];			// �E�F�[�u���n�܂��Ă���̌o�ߎ���(enemy�����Ԃɏo������)

static int				g_Wave;							// ���݂̃E�F�[�u���L�^


static INTERPOLATION_DATA move_tbl_body[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, ENEMY_OFFSET_Y * 10.0f - 10.0f,  21.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, ENEMY_OFFSET_Y * 10.0f		  ,   19.5f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(0.0f, ENEMY_OFFSET_Y * 10.0f - 10.0f,  21.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

static INTERPOLATION_DATA move_tbl_wing0_light[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-12.5f, -3.0f,  -20.0f), XMFLOAT3(RadianSum(70), RadianSum(90), RadianSum(70)), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(-16.0f, -4.0f,  -10.0f), XMFLOAT3(RadianSum(-10), RadianSum(170), RadianSum(40)), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(-12.5f, -3.0f,  -20.0f), XMFLOAT3(RadianSum(70), RadianSum(90), RadianSum(70)), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

static INTERPOLATION_DATA move_tbl_wing0_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(12.5f, -3.0f,  -20.0f), XMFLOAT3(RadianSum(70), RadianSum(-90), RadianSum(-70)), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(16.0f, -4.0f,  -10.0f), XMFLOAT3(RadianSum(-10), RadianSum(-170), RadianSum(-40)), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(12.5f, -3.0f,  -20.0f), XMFLOAT3(RadianSum(70), RadianSum(-90), RadianSum(-70)), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

static INTERPOLATION_DATA move_tbl_wing1_light[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(-30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(-30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(0.0f, 0.0f,  -10.0f), XMFLOAT3(0.0f, RadianSum(140), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(-30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

static INTERPOLATION_DATA move_tbl_wing1_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(0.0f, 0.0f,  -10.0f), XMFLOAT3(0.0f, RadianSum(-140), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};


//=============================================================================
// ����������
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		//LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = TRUE;

		g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, ENEMY_OFFSET_Y + i * 30.0f, 20.0f);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Enemy[i].move = XMFLOAT3(0.0f, 0.0f, 0.0f);

		g_Enemy[i].spd = 0.0f;			// �ړ��X�s�[�h�N���A
		g_Enemy[i].size = ENEMY_SIZE;	// �����蔻��̑傫��

		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		g_Enemy[i].move_time = 0.0f;	// ���`��ԗp�̃^�C�}�[���N���A
		g_Enemy[i].tbl_adr = NULL;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Enemy[i].tbl_size = 0;		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

		g_Enemy[i].waterCount = MAX_WATER_COUNT;

		g_Enemy[i].use = FALSE;			// TRUE:�����Ă�
		g_Enemy[i].end = FALSE;			// FALSE:�����Ă���

		// �K�w�A�j���[�V�����p�̏���������
		g_Enemy[i].parent = NULL;			// �{�́i�e�j�Ȃ̂�NULL������

		// �f�B�]���u��臒l
		g_Enemy[i].dissolve = 1.0f;

		// �ړ��ł��邩�̃t���O
		g_Enemy[i].moveCan = TRUE;

		// �ڕW�ɓ��B���Đ܂�Ԃ�����
		g_Enemy[i].turn = TRUE;

		// �T�C���J�[�u�p�̃��W�A��
		g_Enemy[i].radian = 0.0f;

		// �G�l�~�[�̃x�[�X�̍���
		g_Enemy[i].hight = ENEMY_BASE_HIGHT;

		// �G�l�~�[���������ڕW�n�_�����Z�b�g
		g_targetPos[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);

		// �E�F�[�u���n�܂��Ă���|�b�v����܂ł̎���
		g_Enemy[i].popCount = 60;


		// �p�[�c�̏�����
		for (int p = 0; p < ENEMY_PARTS_MAX; p++)
		{
			g_Parts[i][p].use = FALSE;

			// �ʒu�E��]�E�X�P�[���̏����ݒ�
			g_Parts[i][p].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[i][p].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[i][p].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

			// �K�w�A�j���[�V�����p�̃����o�[�ϐ��̏�����
			g_Parts[i][p].tbl_adr = NULL;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
			g_Parts[i][p].move_time = 0.0f;	// ���s���Ԃ��N���A
			g_Parts[i][p].tbl_size = 0;		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

			// �p�[�c�̓ǂݍ��݂͂܂����Ă��Ȃ�
			g_Parts[i][p].load = 0;
		}

		g_Parts[i][0].use = TRUE;
		g_Parts[i][0].tbl_adr = move_tbl_wing0_light;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[i][0].tbl_size = sizeof(move_tbl_wing0_light) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[i][0].load = 1;

		g_Parts[i][1].use = TRUE;
		g_Parts[i][1].tbl_adr = move_tbl_wing1_light;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[i][1].tbl_size = sizeof(move_tbl_wing1_light) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[i][1].load = 1;

		g_Parts[i][2].use = TRUE;
		g_Parts[i][2].tbl_adr = move_tbl_wing0_left;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[i][2].tbl_size = sizeof(move_tbl_wing0_left) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[i][2].load = 1;

		g_Parts[i][3].use = TRUE;
		g_Parts[i][3].tbl_adr = move_tbl_wing1_left;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[i][3].tbl_size = sizeof(move_tbl_wing1_left) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[i][3].load = 1;

		// �e�q�֌W
		g_Parts[i][0].parent = &g_Enemy[i];			// �E���O���̐e�͖{��
		g_Parts[i][1].parent = &g_Parts[i][0];		// �E���㔼�̐e�͉E���O��
		g_Parts[i][2].parent = &g_Enemy[i];			// �����O���̐e�͖{��
		g_Parts[i][3].parent = &g_Parts[i][2];		// �����㔼�̐e�͍����O��

	}

	LoadModel(MODEL_ENEMY, &g_Enemy[0].model, 0);

	// �p�[�c�̃��[�h
	LoadModel(MODEL_PARTS_WING_0, &g_Parts[0][0].model, 0);
	LoadModel(MODEL_PARTS_WING_1, &g_Parts[0][1].model, 0);
	LoadModel(MODEL_PARTS_WING_0, &g_Parts[0][2].model, 0);
	LoadModel(MODEL_PARTS_WING_1, &g_Parts[0][3].model, 0);


	// �E�F�[�u�̏�����(wave1)
	for (int i = 0; i < MAX_POP_WAVE1; i++)
	{
		g_Enemy[i].popCount = (i + 1) * POP_DISTANCE;
		g_Enemy[i].wave = 1;
	}

	// �E�F�[�u�̏�����(wave2)
	for (int i = MAX_POP_WAVE1; i < MAX_POP_WAVE2 + MAX_POP_WAVE1; i++)
	{
		g_Enemy[i].popCount = i * POP_DISTANCE;
		g_Enemy[i].wave = 2;
	}

	// �E�F�[�u�̏�����(wave3)
	for (int i = MAX_POP_WAVE2 + MAX_POP_WAVE1; i < MAX_POP_WAVE3 + MAX_POP_WAVE2 + MAX_POP_WAVE1; i++)
	{
		g_Enemy[i].popCount = i * POP_DISTANCE;
		g_Enemy[i].wave = 3;
	}

	// �E�F�[�u�̏�����(wave4)
	for (int i = MAX_POP_WAVE3 + MAX_POP_WAVE2 + MAX_POP_WAVE1; i < MAX_POP_WAVE4 + MAX_POP_WAVE3 + MAX_POP_WAVE2 + MAX_POP_WAVE1; i++)
	{
		g_Enemy[i].popCount = i * POP_DISTANCE / 2;
		g_Enemy[i].wave = 4;
	}


	// �ʒu����
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		switch (g_Enemy[i].wave)
		{
		case 1:
			g_Enemy[i].pos.x = MAX_POS_WAVE1 + 100.0f;
			break;

		case 2:
			g_Enemy[i].pos.x = MAX_POS_WAVE2 + 100.0f;
			break;

		case 3:
			g_Enemy[i].pos.x = MAX_POS_WAVE3 + 100.0f;
			break;

		case 4:
			g_Enemy[i].pos.x = MAX_POS_WAVE4 + 100.0f;
			break;
		}
	}


	// 0�Ԃ������`��Ԃœ������Ă݂�
	g_Enemy[0].move_time = 0.0f;		// ���`��ԗp�̃^�C�}�[���N���A
	g_Enemy[0].tbl_adr = move_tbl_body;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Enemy[0].tbl_size = sizeof(move_tbl_body) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

	// ���݂̃E�F�[�u��������
	g_Wave = 0;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
	// �������@������������ړ��s��
	if (GetSummon()) return;

	PLAYER *player = GetPlayer();

	// �G�l�~�[�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		// �v���C���[�̃|�W�V�����ɉ����ăE�F�[�u��ύX
		if (player->pos.x > MAX_POS_WAVE4)
		{	// �E�F�C�u1
			g_Wave = 4;
		}
		else if (player->pos.x > MAX_POS_WAVE3)
		{	// �E�F�C�u1
			g_Wave = 3;
		}
		else if (player->pos.x > MAX_POS_WAVE2)
		{	// �E�F�C�u1
			g_Wave = 2;
		}
		else if (player->pos.x > MAX_POS_WAVE1)
		{	// �E�F�C�u1
			g_Wave = 1;
		}

		// �E�F�[�u�ɉ����ďo���G�l�~�[��ύX
		if ((g_Wave >= g_Enemy[i].wave) && (g_Enemy[i].popCount > 0)) g_Enemy[i].popCount--;

		// pop�J�E���g��0�ɂȂ�����G�l�~�[���o��������
		if (g_Enemy[i].popCount == 0)
		{
			SetPopEnemy(i);
		}



		if (g_Enemy[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			if (g_Enemy[i].moveCan)			// ���`��Ԃ����s����H
			{								// ���`��Ԃ̏���
				//// �ړ�����



				// �v���C���[�̕������擾
				float disX, disZ, royY;

				disX = player->pos.x - g_Enemy[i].pos.x;
				disZ = player->pos.z - g_Enemy[i].pos.z;

				if (disZ > 0.0f)
				{
					royY = (XM_PI)+atanf(disX / disZ);
				}
				else if (disX < 0.0f)
				{
					royY = (XM_PI / 2) - atanf(disZ / disX);
				}
				else
				{
					royY = (-XM_PI / 2) - atanf(disZ / disX);
				}

				if (g_Enemy[i].turn)
				{
					// �G�l�~�[���������ڕW�n�_�����Z�b�g
					g_targetPos[i].x = sinf(royY + XM_PI) * TURN_DISTANCE + player->pos.x;
					g_targetPos[i].z = cosf(royY + XM_PI) * TURN_DISTANCE + player->pos.z;

					g_Enemy[i].turn = FALSE;
					g_Enemy[i].move.x = (g_targetPos[i].x - g_Enemy[i].pos.x) / TARGET_SPEED;
					g_Enemy[i].move.z = (g_targetPos[i].z - g_Enemy[i].pos.z) / TARGET_SPEED;
					g_Enemy[i].rot.y = royY;
				}

				// �G�l�~�[���ړ�
				g_Enemy[i].pos.x += g_Enemy[i].move.x;
				g_Enemy[i].pos.z += g_Enemy[i].move.z;

				// �ڕW�n�_�߂��ɂ��邩�ǂ���
				if ((g_Enemy[i].pos.x > g_targetPos[i].x - TURN_OFFSET) &&
					(g_Enemy[i].pos.x < g_targetPos[i].x + TURN_OFFSET) &&
					(g_Enemy[i].pos.z > g_targetPos[i].z - TURN_OFFSET) &&
					(g_Enemy[i].pos.z < g_targetPos[i].z + TURN_OFFSET))
				{	// �߂��ɂ���ꍇ�ɕ����]������
					g_Enemy[i].turn = TRUE;
				}

				// �T�C���J�[�u
				g_Enemy[i].radian -= XM_PI / TARGET_SPEED;
				if (g_Enemy[i].radian < -XM_PI) g_Enemy[i].radian = 0.0f;
				g_Enemy[i].pos.y = sinf(g_Enemy[i].radian) * g_Enemy[i].hight + g_Enemy[i].hight + ENEMY_OFFSET_Y;
			}


			// �p�[�c�̊K�w�A�j���[�V����
			RunEnemyPartsAnimation(i);

			// �f�B�]���u�t���O����������
			if (g_Enemy[i].end)
			{
				g_Enemy[i].dissolve -= DISSOLVE_SPEED;

				if (g_Enemy[i].dissolve < 0.0f)
				{	// �f�B�]���u�������I�������
					ResetEnemy(i);
				}
			}

		}
	}

#ifdef _DEBUG	// �f�o�b�O����\������
	PrintDebugProc("Enemy:%f \n", g_Enemy[0].dissolve);
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// �J�����O����
	//SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == FALSE) continue;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// �f�B�]���u�̐ݒ�
		SetDissolve(g_Enemy[i].dissolve);

		// ���f���`��
		DrawModel(&g_Enemy[0].model);

		// �p�[�c�̊K�w�A�j���[�V����
		for (int p = 0; p < ENEMY_PARTS_MAX; p++)
		{
			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_Parts[i][p].scl.x, g_Parts[i][p].scl.y, g_Parts[i][p].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// ��]�𔽉f
			mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i][p].rot.x, g_Parts[i][p].rot.y, g_Parts[i][p].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_Parts[i][p].pos.x, g_Parts[i][p].pos.y, g_Parts[i][p].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			if (g_Parts[i][p].parent != NULL)	// �q����������e�ƌ�������
			{
				mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i][p].parent->mtxWorld));
			}

			XMStoreFloat4x4(&g_Parts[i][p].mtxWorld, mtxWorld);

			// �g���Ă���Ȃ珈������B�����܂ŏ������Ă��闝�R�͑��̃p�[�c�����̃p�[�c���Q�Ƃ��Ă���\�������邩��B
			if (g_Parts[i][p].use == FALSE) continue;

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// ���f���`��
			DrawModel(&g_Parts[0][p].model);
		}
	}

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// �G�l�~�[�̎擾
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}


//�@�K�w�A�j���[�V����
void RunEnemyPartsAnimation(int i)
{
	for (int p = 0; p < ENEMY_PARTS_MAX; p++)
	{
		// �g���Ă���Ȃ珈������
		if ((g_Parts[i][p].use == TRUE) && (g_Parts[i][p].tbl_adr != NULL))
		{
			// �ړ�����
			int		index = (int)g_Parts[i][p].move_time;
			float	time = g_Parts[i][p].move_time - index;
			int		size = g_Parts[i][p].tbl_size;

			float dt = 1.0f / g_Parts[i][p].tbl_adr[index].frame;	// 1�t���[���Ői�߂鎞��
			g_Parts[i][p].move_time += dt;					// �A�j���[�V�����̍��v���Ԃɑ���

			if (index > (size - 2))	// �S�[�����I�[�o�[���Ă�����A�ŏ��֖߂�
			{
				g_Parts[i][p].move_time = 0.0f;
				index = 0;
			}

			// ���W�����߂�	X = StartX + (EndX - StartX) * ���̎���
			XMVECTOR p1 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 1].pos);	// ���̏ꏊ
			XMVECTOR p0 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 0].pos);	// ���݂̏ꏊ
			XMVECTOR vec = p1 - p0;
			XMStoreFloat3(&g_Parts[i][p].pos, p0 + vec * time);

			// ��]�����߂�	R = StartX + (EndX - StartX) * ���̎���
			XMVECTOR r1 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 1].rot);	// ���̊p�x
			XMVECTOR r0 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 0].rot);	// ���݂̊p�x
			XMVECTOR rot = r1 - r0;
			XMStoreFloat3(&g_Parts[i][p].rot, r0 + rot * time);

			// scale�����߂� S = StartX + (EndX - StartX) * ���̎���
			XMVECTOR s1 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 1].scl);	// ����Scale
			XMVECTOR s0 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 0].scl);	// ���݂�Scale
			XMVECTOR scl = s1 - s0;
			XMStoreFloat3(&g_Parts[i][p].scl, s0 + scl * time);

		}
	}
}


// �G�l�~�[�̏I������
void ResetEnemy(int i)
{
	g_Enemy[i].use = FALSE;			// TRUE:�����Ă�
	g_Enemy[i].end = TRUE;			// TRUE:�����Ă�

	// �f�B�]���u��臒l
	g_Enemy[i].dissolve = 1.0f;

}


// �p�[�c�̃��[�h
void LoadParts(char *FileName, int i)
{
	MODEL model;

	LoadObj(FileName, &model);

	// ���_�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * model.VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = model.VertexArray;

		for (int p = 0 ; p < MAX_ENEMY; p++)
		{
			GetDevice()->CreateBuffer(&bd, &sd, &g_Parts[p][i].model.VertexBuffer);
		}
	}


	// �C���f�b�N�X�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned short) * model.IndexNum;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = model.IndexArray;

		for (int p = 0; p < MAX_ENEMY; p++)
		{
			GetDevice()->CreateBuffer(&bd, &sd, &g_Parts[p][i].model.IndexBuffer);
		}
	}

	// �T�u�Z�b�g�ݒ�
	for (int p = 0; p < MAX_ENEMY; p++)
	{
		g_Parts[p][i].model.SubsetArray = new DX11_SUBSET[model.SubsetNum];
		g_Parts[p][i].model.SubsetNum = model.SubsetNum;

		for (unsigned short v = 0; v < model.SubsetNum; v++)
		{
			g_Parts[p][i].model.SubsetArray[v].StartIndex = model.SubsetArray[v].StartIndex;
			g_Parts[p][i].model.SubsetArray[v].IndexNum = model.SubsetArray[v].IndexNum;

			g_Parts[p][i].model.SubsetArray[v].Material.Material = model.SubsetArray[v].Material.Material;

			D3DX11CreateShaderResourceViewFromFile(GetDevice(),
				model.SubsetArray[v].Material.TextureName,
				NULL,
				NULL,
				&g_Parts[p][i].model.SubsetArray[v].Material.Texture,
				NULL);
		}
	}

	delete[] model.VertexArray;
	delete[] model.IndexArray;
	delete[] model.SubsetArray;
}


// �G�l�~�[���o��������
void SetPopEnemy(int i)
{
	g_Enemy[i].use = TRUE;
	g_Enemy[i].popCount = -1;	// �o�������Ƃ����t���O(-1)

	// �o���ʒu�������_����
	float radian = RamdomFloat(3, 3.14f, 0.0f);
	g_Enemy[i].pos.z = cosf(radian) * TURN_DISTANCE;

	switch (g_Enemy[i].wave)
	{
	case 1:
		g_Enemy[i].pos.x = sinf(radian) * TURN_DISTANCE * 2 + MAX_POS_WAVE1 + TURN_DISTANCE;
		break;

	case 2:
		g_Enemy[i].pos.x = sinf(radian) * TURN_DISTANCE * 2 + MAX_POS_WAVE2 + TURN_DISTANCE;
		break;

	case 3:
		g_Enemy[i].pos.x = sinf(radian) * TURN_DISTANCE * 2 + MAX_POS_WAVE3 + TURN_DISTANCE;
		break;

	case 4:
		g_Enemy[i].pos.x = sinf(radian) * TURN_DISTANCE * 2 + MAX_POS_WAVE4 + TURN_DISTANCE;
		break;
	}

	SetEnemyVoice();
}


// �G�l�~�[���o������Ƃ��̃{�C�X�������_���ōĐ�
void SetEnemyVoice(void)
{
	int type = rand() % ENEMY_VOICE_MAX;

	switch (type)
	{
	case 0:
		PlaySound(SOUND_LABEL_SE_ENEMY_pop0);
		break;

	case 1:
		PlaySound(SOUND_LABEL_SE_ENEMY_pop1);
		break;

	case 2:
		PlaySound(SOUND_LABEL_SE_ENEMY_pop2);
		break;

	case 3:
		PlaySound(SOUND_LABEL_SE_ENEMY_pop3);
		break;
	}
}