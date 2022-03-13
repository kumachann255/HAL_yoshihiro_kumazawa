//=============================================================================
//
// �J�������� [camera.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "model.h"
#include "player.h"
#include "enemy.h"
#include "bahamut.h"
#include "bahamut_bless.h"
#include "beam_particle.h"
#include "beam_particle_2.h"
#include "beam_orbit.h"
#include "lightning.h"
#include "tree.h"
#include "fade_white.h"
#include "sound.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	POS_X_CAM			(0.0f)			// �J�����̏����ʒu(X���W)
#define	POS_Y_CAM			(30.0f)			// �J�����̏����ʒu(Y���W)
#define	POS_Z_CAM			(-80.0f)		// �J�����̏����ʒu(Z���W)

#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// �r���[���ʂ̎���p
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// �r���[���ʂ̃A�X�y�N�g��	
#define	VIEW_NEAR_Z		(10.0f)											// �r���[���ʂ�NearZ�l
#define	VIEW_FAR_Z		(10000.0f)										// �r���[���ʂ�FarZ�l

#define	VALUE_MOVE_CAMERA	(2.0f)										// �J�����̈ړ���
#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)								// �J�����̉�]��

#define OVERLOOKING_Y	(150.0f)			// ������̘��Վ��_��Y���W
#define OVERLOOKING_Z	(-250.0f)			// ������̘��Վ��_��Z���W

#define OFFSET_X		(60)
#define OFFSET_Y		(50.0f)				// �ʏ펋�_�̎��̒����_����
#define OFFSET_Z		(100.0f)			// �����_�̒���
#define CAMERA_OFFSET_X	(50.0f)				// �J�����̎��_����

#define CAMERA_TRACKING	(20.0f)				// �J�����̒ǐՑ��x(���Ȃ���Α���)

#define TURNCOUNT_MAX	(20)				// �U�������������t���[���ōs����

#define SUMMON_SCENE_MAX		(sceneMAX)		// �������@�̍ő�V�[����
#define SCENE_POP_UP			(0.5f)			// pop�V�[������AT�̏㏸���x
#define SCENE_POP_DISTANCE		(250.0f)		// pop�V�[������AT�̏㏸���x
#define SCENE_OPEN_AT_Y			(220.0f)		// open�V�[���̒����_����
#define SCENE_BLESS_AT			(1.0f)			// bless�V�[���̒����_����
#define SCENE_BLESS_COUNT_MAX	(10)			// bless�V�[���̒����_���ړ������鎞��

#define CAMERA_SHAKE_MAX		(7)				// ��ʂ��h���ő�l
#define CAMERA_SHAKE_INTERVAL	(2)				// ��ʂ��h���Ԋu


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static CAMERA			g_Camera;		// �J�����f�[�^

static int				g_ViewPortType = TYPE_FULL_SCREEN;

static float			g_CameraX[OFFSET_X];

static BOOL				OverLook = TRUE;

static int				g_turn;			// �U�������������s���Ă��邩
static int				g_turnCount;	// �U��������삪�n�܂��ĉ��t���[���o�߂��������L�^

static XMFLOAT3			g_DebugAT;

static float			g_DebugY;		// �f�o�b�O�p��y���W��ύX����

static int				g_NowScene;		// ���݂̃V�[��

static XMFLOAT3			g_OffsetAT;		// �����_�����p

static int				g_BlessCount;	// �����_���ړ������鎞�Ԃ��Ǘ�

static BOOL				g_Shake;		// ��ʗh������邩�ǂ���
static int				g_ShakeCount;	// ��ʂ��h���c�莞��
static XMFLOAT3			g_ShakePos;		// �ǂ̂��炢�h�炷���̋�����ۑ�


static BOOL				g_SummonScene[2][SUMMON_SCENE_MAX];		// �������@�̂ǂ̃V�[�����Đ����Ă��邩���Ǘ�(�V�[���i�s�x�Ǘ�)
																// y�� 0:�ǂ̃V�[���܂Ői�񂾂�  1:���̃V�[�����Đ�������

// ���@�w�V�[��
static INTERPOLATION_DATA g_MoveTbl_circle[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3(0.0f, 300.0f, 10.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.008f },
	{ XMFLOAT3(0.0f, 900.0f, 10.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// �����V�[��
static INTERPOLATION_DATA g_MoveTbl_pop[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3(0.0f, 150.0f, 10.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0015f },
	{ XMFLOAT3(0.0f, 350.0f, 10.0f),		XMFLOAT3(0.0f, XM_PI, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// �������X�ɍL����V�[��
static INTERPOLATION_DATA g_MoveTbl_fold[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3(250.0f, SCENE_OPEN_AT_Y, -250.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.02f },
	{ XMFLOAT3(270.0f, SCENE_OPEN_AT_Y, -270.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// ������C�ɍL����V�[��
static INTERPOLATION_DATA g_MoveTbl_open[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3(270.0f, SCENE_OPEN_AT_Y, -270.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.18f },
	{ XMFLOAT3(400.0f, SCENE_OPEN_AT_Y, -400.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// �u���X���`���[�W����V�[��
static INTERPOLATION_DATA g_MoveTbl_charge[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3(50.0f, 0.0f, 60.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.001f },
	{ XMFLOAT3(80.0f, 0.0f, 100.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// �u���X�V�[��
static INTERPOLATION_DATA g_MoveTbl_bless[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3(80.0f, 0.0f, 100.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// �u���X�V�[��(2�J��)
static INTERPOLATION_DATA g_MoveTbl_bless_2camera[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3(300.0f, 100.0f, 0.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// �����V�[��
static INTERPOLATION_DATA g_MoveTbl_blast[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3( 0.0f, 450.0f, 350.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.005f },
};

// ������V�[��
static INTERPOLATION_DATA g_MoveTbl_end[] = {
	//���W									��]��					Z		�g�嗦		X		Y			����
	{ XMFLOAT3(0.0f, 300.0f, -450.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.005f },
};


// �V�[�����Ƃ̈ړ��e�[�u���A�h���X��z��ŊǗ�
static INTERPOLATION_DATA *g_MoveTblAdr[] =
{
	g_MoveTbl_circle,
	g_MoveTbl_pop,
	g_MoveTbl_fold,
	g_MoveTbl_open,
	g_MoveTbl_charge,
	g_MoveTbl_bless,
	g_MoveTbl_bless_2camera,
	g_MoveTbl_blast,
	g_MoveTbl_end,
};







//=============================================================================
// ����������
//=============================================================================
void InitCamera(void)
{
	g_Camera.pos = { POS_X_CAM, POS_Y_CAM, POS_Z_CAM };
	g_Camera.pos.z = OVERLOOKING_Z;
	g_Camera.at  = { 0.0f, 0.0f, 0.0f };
	g_Camera.up  = { 0.0f, 1.0f, 0.0f };
	g_Camera.rot = { 0.0f, 0.0f, 0.0f };
	g_Camera.time = 0.0f;
	g_Camera.moveTblNo = 0;
	g_Camera.tblMax = (sizeof(g_MoveTbl_circle) / sizeof(INTERPOLATION_DATA));

	// �V�[���Ǘ��z��̏�����
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < SUMMON_SCENE_MAX; x++)
		{
			g_SummonScene[y][x] = FALSE;
		}
	}
	// ���@�w�V�[���ɃZ�b�g
	g_SummonScene[0][0] = TRUE;


	// ���_�ƒ����_�̋������v�Z
	float vx, vz;
	vx = g_Camera.pos.x - g_Camera.at.x;
	vz = g_Camera.pos.z - g_Camera.at.z;
	g_Camera.len = sqrtf(vx * vx + vz * vz);
	
	// �f�o�b�O�p��AT�����ϐ�
	g_DebugAT = { 0.0f, 0.0f, 0.0f };

	// �r���[�|�[�g�^�C�v�̏�����
	g_ViewPortType = TYPE_FULL_SCREEN;

	// �ϐ��̏�����
	g_NowScene = circle;
	g_OffsetAT = { 0.0f, 0.0f, 0.0f };
	g_BlessCount = 0;
}


//=============================================================================
// �J�����̏I������
//=============================================================================
void UninitCamera(void)
{

}


//=============================================================================
// �J�����̍X�V����
//=============================================================================
void UpdateCamera(void)
{
	BAHAMUT *bahamut = GetBahamut();
	BLESS *BahamutBlast = GetBlast();
	// �������@���������Ă��邩�ŏ�����ύX
	if (GetSummon())
	{
		int nowNo = (int)g_Camera.time;			// �������ł���e�[�u���ԍ������o���Ă���
		int maxNo = g_Camera.tblMax;			// �o�^�e�[�u�����𐔂��Ă���
		int nextNo = (nowNo + 1) % maxNo;		// �ړ���e�[�u���̔ԍ������߂Ă���

		INTERPOLATION_DATA *tbl = g_MoveTblAdr[g_Camera.moveTblNo];	// �s���e�[�u���̃A�h���X���擾

		// XMFloat3�^��XMVECTOR�^�փR���o�[�g
		XMVECTOR pos1 = XMLoadFloat3(&tbl[nextNo].pos);
		XMVECTOR pos2 = XMLoadFloat3(&tbl[nowNo].pos);
		pos1 -= pos2;	// XYZ�ړ��ʂ��v�Z���Ă���

		// XMFloat3�^��XMVECTOR�^�փR���o�[�g
		XMVECTOR rot1 = XMLoadFloat3(&tbl[nextNo].rot);
		XMVECTOR rot2 = XMLoadFloat3(&tbl[nowNo].rot);
		rot1 -= rot2;	// XYZ��]�ʂ��v�Z���Ă���

		float nowTime = g_Camera.time - nowNo;	// ���ԕ����ł��鏭�������o���Ă���

		pos1 *= nowTime;	// ���݂̈ړ��ʂ��v�Z���Ă���
		rot1 *= nowTime;	// ���݂̉�]�ʂ��v�Z���Ă���

		// �v�Z���ċ��߂��ړ��ʁA��]�ʁA�g�嗦�����݂̈ړ��e�[�u��XYZ�ɑ����Ă��遁�\�����W�����߂Ă���
		pos2 += pos1;	// �ړ���
		rot2 += rot1;	// ��]��

		// XMVECTOR�^��XMFloat3�^�փR���o�[�g
		XMStoreFloat3(&g_Camera.pos, pos2);
		XMStoreFloat3(&g_Camera.rot, rot2);

		//// frame���g�Ď��Ԍo�ߏ���������
		//if ((int)g_Camera.time < maxNo)	// �o�^�e�[�u���Ō�܂łňړ����~�߂�
		//{
		//	g_Camera.time += tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
		//}

		// frame���g�Ď��Ԍo�ߏ���������
		g_Camera.time += tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
		if ((int)g_Camera.time >= maxNo)			// �o�^�e�[�u���Ō�܂ňړ��������H
		{
			g_Camera.time -= maxNo;				// �O�ԖڂɃ��Z�b�g�������������������p���ł���
		}


		// X���W���o�n���[�g�ɍ��킹��
		g_Camera.pos.x += bahamut->pos.x;

		// �V�[���i�s�x�̌���
		for (int i = 0; i < SUMMON_SCENE_MAX; i++)
		{
			// �i�񂾃V�[���̃J�������[�N���Đ�����Ă��Ȃ���Ώ���
			if ((g_SummonScene[0][i] == TRUE) && (g_SummonScene[1][i] == FALSE))
			{
				// �Đ��ς݂ɕύX
				g_SummonScene[1][i] = TRUE;

				// ���`��Ԃ̐i�s�x�����Z�b�g
				g_Camera.time = 0.0f;

				// �F�X���Z�b�g
				g_OffsetAT = { 0.0f, 0.0f, 0.0f };
				g_BlessCount = 0;
				ResetInOut();

				// �i�s�x�ɍ������V�[�����Z�b�g
				// �ꏏ�ɃT�E���h���Đ�
				switch (i)
				{
				case circle:
					g_Camera.moveTblNo = circle;
					g_Camera.tblMax = (sizeof(g_MoveTbl_circle) / sizeof(INTERPOLATION_DATA));
					g_NowScene = circle;

					PlaySound(SOUND_LABEL_SE_BAHAMUT_circle);
					break;

				case pop:
					g_Camera.moveTblNo = pop;
					g_Camera.tblMax = (sizeof(g_MoveTbl_pop) / sizeof(INTERPOLATION_DATA));
					g_NowScene = pop;

					PlaySound(SOUND_LABEL_SE_BAHAMUT_pop);
					break;

				case fold:
					g_Camera.moveTblNo = fold;
					g_Camera.tblMax = (sizeof(g_MoveTbl_fold) / sizeof(INTERPOLATION_DATA));
					g_NowScene = fold;
					break;

				case open:
					g_Camera.moveTblNo = open;
					g_Camera.tblMax = (sizeof(g_MoveTbl_open) / sizeof(INTERPOLATION_DATA));
					g_NowScene = open;

					PlaySound(SOUND_LABEL_SE_BAHAMUT_open);
					break;

				case charge:
					g_Camera.moveTblNo = charge;
					g_Camera.tblMax = (sizeof(g_MoveTbl_charge) / sizeof(INTERPOLATION_DATA));
					g_NowScene = charge;

					SetBeamParticleCharge(TRUE);
					break;

				case bless:
					g_Camera.moveTblNo = bless;
					g_Camera.tblMax = (sizeof(g_MoveTbl_bless) / sizeof(INTERPOLATION_DATA));
					g_NowScene = bless;

					SetFadeWhite(WHITE_IN_bless);
					SetSurge();

					SetBeamDelete();
					SetBeam2Delete();
					SetBeamParticleCharge(FALSE);
					SetBeamParticleSwitch(TRUE);
					SetBeamParticleSwitch2(TRUE);
					SetBeamOrbitSwitch(TRUE);

					PlaySound(SOUND_LABEL_SE_BAHAMUT_bless);
					break;

				case bless_2camera:
					g_Camera.moveTblNo = bless_2camera;
					g_Camera.tblMax = (sizeof(g_MoveTbl_bless_2camera) / sizeof(INTERPOLATION_DATA));
					g_NowScene = bless_2camera;

					SetFadeWhite(WHITE_IN_bless);
					SetSurge();

					SetBeamDelete();
					SetBeam2Delete();
					SetBeamOrbitDelete();
					SetBeamParticleSwitch(TRUE);
					SetBeamParticleSwitch2(TRUE);
					SetBeamOrbitSwitch(TRUE);

					PlaySound(SOUND_LABEL_SE_BAHAMUT_bless);
					break;

				case blast:
					g_Camera.moveTblNo = blast;
					g_Camera.tblMax = (sizeof(g_MoveTbl_blast) / sizeof(INTERPOLATION_DATA));
					g_NowScene = blast;

					SetCameraShake(270);
					SetFadeWhite(WHITE_IN_bless);
					SetSurge();

					SetBeamDelete();
					SetBeam2Delete();
					SetBeamOrbitDelete();
					SetBeamParticleSwitch(TRUE);
					SetBeamParticleSwitch2(TRUE);
					SetBeamOrbitSwitch(TRUE);

					PlaySound(SOUND_LABEL_SE_BAHAMUT_bless);
					PlaySound(SOUND_LABEL_SE_BAHAMUT_blast);
					break;

				case end:
					g_Camera.moveTblNo = end;
					g_Camera.tblMax = (sizeof(g_MoveTbl_end) / sizeof(INTERPOLATION_DATA));
					g_NowScene = end;
					break;
				}
			}
		}

		// �����_(AT)�Ǝ��_�̊Ǘ�
		{
			// �i�s�x�ɍ����������_���Z�b�g
			switch (g_NowScene)
			{
			case circle:
				// �����_
				g_Camera.at = bahamut->pos;
				g_Camera.at.y = 0.0f;

				break;

			case pop:
				// �����_
				g_Camera.at = bahamut->pos;
				g_OffsetAT.y += SCENE_POP_UP;
				g_Camera.at.y = g_OffsetAT.y;

				// ���_
				g_Camera.pos.x = bahamut->pos.x + sinf(g_Camera.rot.y) * SCENE_POP_DISTANCE;
				g_Camera.pos.z = bahamut->pos.z + cosf(g_Camera.rot.y) * SCENE_POP_DISTANCE;

				break;

			case fold:
				// �����_
				g_Camera.at = bahamut->pos;
				g_Camera.at.y = SCENE_OPEN_AT_Y + 10.0f;

				break;

			case open:
				// �����_
				g_Camera.at = bahamut->pos;
				g_Camera.at.y = SCENE_OPEN_AT_Y + 10.0f;

				break;

			case charge:
				// �����_
				g_Camera.at = bahamut->pos;
				g_Camera.at.y += 135.0f;
				g_Camera.at.x += 85.0f;

				// ���_
				g_Camera.pos.x = g_Camera.at.x;
				g_Camera.pos.y = g_Camera.at.y + 20.0f;

				SetBeamParticleCharge(TRUE);
				break;

			case bless:
				// �����_
				g_Camera.at = bahamut->pos;
				g_Camera.at.y += 135.0f;
				g_Camera.at.x += 85.0f;
				g_Camera.at.x += g_OffsetAT.x;
				g_Camera.at.y -= g_OffsetAT.y + 10.0f;
				g_Camera.at.z += g_OffsetAT.z;
				
				g_BlessCount++;
				if (g_BlessCount < SCENE_BLESS_COUNT_MAX)
				{
					g_OffsetAT.x += SCENE_BLESS_AT;
					g_OffsetAT.y += SCENE_BLESS_AT;
					g_OffsetAT.z += SCENE_BLESS_AT;
				}

				// ���_
				//g_Camera.pos.y = BahamutBless->pos.y + 20.0f;
				//g_Camera.pos = bahamut->pos;
				g_Camera.pos.y = bahamut->pos.y + 145.0f;
				//g_Camera.pos.x += 85.0f;

				break;

			case bless_2camera:
				// �����_
				g_Camera.at = bahamut->pos;
				g_Camera.at.y += 80.0f;
				g_Camera.at.x += 85.0f;
				g_Camera.at.x += g_OffsetAT.x;
				g_Camera.at.y -= g_OffsetAT.y;

				g_BlessCount++;
				if (g_BlessCount < SCENE_BLESS_COUNT_MAX)
				{
					g_OffsetAT.x += SCENE_BLESS_AT;
					g_OffsetAT.y += SCENE_BLESS_AT;
				}

				// ���_
				g_Camera.pos.y = bahamut->pos.y + 145.0f;

				break;

			case blast:
				// �����_
				g_Camera.at = bahamut->pos;
				g_Camera.at.x += BLAST_X_OFFSET - 100.0f;
				g_Camera.at.z -= 170.0f;
				g_Camera.at.y -= 60.0f;

				break;

			case end:

				break;
			}
		}

		SetCamera();
	}
	else
	{
		PLAYER *player = GetPlayer();

		// ���Վ��_�̐؂�ւ�
		if ((GetKeyboardTrigger(DIK_Z)) || (IsButtonTriggered(0, BUTTON_Y)))
		{
			if (OverLook)
			{
				OverLook = FALSE;
				g_Camera.len = 150.0f;
				g_Camera.pos.y = 60.0f;

				// ��O�̖؂�\��
				SetTreeSwitch(TRUE);
			}
			else
			{
				OverLook = TRUE;

				// ��O�̖؂��\��
				SetTreeSwitch(FALSE);
			}
		}

		// ���Վ��_���ǂ����ő����ύX
		if (OverLook)
		{	// ������̘��Վ��_����
			g_Camera.pos.y = OVERLOOKING_Y;

			float dis;	// x���W�̃J�����ƃv���C���[�̋�����ۑ�

			PLAYER *player = GetPlayer();

			dis = player->pos.x - g_Camera.pos.x;
			g_Camera.pos.x += dis / CAMERA_TRACKING;

			dis = player->pos.z - g_Camera.pos.z + OVERLOOKING_Z;
			g_Camera.pos.z += dis / CAMERA_TRACKING;
		}
		else
		{	// TPS���_����
			// �U���������̏���
			if ((g_turn) && (g_turnCount < TURNCOUNT_MAX))
			{
				// �������U�����
				g_Camera.rot.y += XM_PI / TURNCOUNT_MAX;

				g_turnCount++;

				// �U������I�������I������
				if (g_turnCount >= TURNCOUNT_MAX)
				{
					g_turn = FALSE;
					g_turnCount = 0;
				}
			}


			// TPS���_�֒���
			float dis;
			XMFLOAT3 ppos;

			ppos = player->pos;
			ppos.x -= sinf(player->rot.y + XM_PI / 2) * CAMERA_OFFSET_X;
			ppos.z -= cosf(player->rot.y + XM_PI / 2) * CAMERA_OFFSET_X;

			dis = (ppos.x + -sinf(player->rot.y + XM_PI) * OFFSET_Z) - g_Camera.pos.x;
			g_Camera.pos.x = dis / CAMERA_TRACKING + g_Camera.pos.x;

			dis = (ppos.z + -cosf(player->rot.y + XM_PI) * OFFSET_Z) - g_Camera.pos.z;
			g_Camera.pos.z = dis / CAMERA_TRACKING + g_Camera.pos.z;


			// �J�����̎��_���J������Y����]�ɑΉ������Ă���
			float disX, disZ;

			disX = (g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len) - g_Camera.pos.x;
			disZ = (g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len) - g_Camera.pos.z;

			g_Camera.pos.x += disX / CAMERA_TRACKING;
			g_Camera.pos.z += disZ / CAMERA_TRACKING;
		}
	}


	// ��ʂ��h��鏈��
	if (g_Shake)
	{
		// ��ʗh��̎c�莞�Ԃ����炷
		g_ShakeCount--;

		// �c�莞�Ԃ��c���Ă���ꍇ
		if (g_ShakeCount > 0)
		{
			g_Camera.pos.x += g_ShakePos.x;
			g_Camera.pos.y += g_ShakePos.y;
			g_Camera.pos.z += g_ShakePos.z;

			// ��ʗh�ꕝ�����Z�b�g
			if (g_ShakeCount % CAMERA_SHAKE_INTERVAL == 0)
			{
				g_ShakePos.x = RamdomFloat(3, CAMERA_SHAKE_MAX, -CAMERA_SHAKE_MAX);
				g_ShakePos.y = RamdomFloat(3, CAMERA_SHAKE_MAX, -CAMERA_SHAKE_MAX);
				g_ShakePos.z = RamdomFloat(3, CAMERA_SHAKE_MAX, -CAMERA_SHAKE_MAX);
			}
		}
		else
		{
			g_Shake = FALSE;
		}
	}
	else if (!OverLook)
	{
		// �������@���������Ă��Ȃ������ꍇ
		if (!GetSummon())
		{
			// TPS���_���ɉ�ʗh�ꂵ���J�������C��
			g_Camera.pos.y = 60.0f;
		}
	}




#ifdef _DEBUG	// �f�o�b�O����\������
	PrintDebugProc("Camera:ZC QE TB YN UM R\n");
	PrintDebugProc("CameraPos:x:%f  y;%f  z:%f  \nscene:%d\n", g_Camera.pos.x, g_Camera.pos.y, g_Camera.pos.z, g_NowScene);
	PrintDebugProc("CameraPosAT:x:%f  y;%f  z:%f  \nscene:%d\n", g_Camera.at.x, g_Camera.at.y, g_Camera.at.z, g_NowScene);

	if ((GetKeyboardPress(DIK_M)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_Camera.len += 5.0f;
	}

	if ((GetKeyboardPress(DIK_N)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_Camera.len -= 5.0f;
	}

	if ((GetKeyboardPress(DIK_U)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_DebugAT.y += 1.0f;
	}

	if ((GetKeyboardPress(DIK_Y)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_DebugAT.y -= 1.0f;
	}




#endif
}


//=============================================================================
// �J�����̍X�V
//=============================================================================
void SetCamera(void) 
{
	// �r���[�}�g���b�N�X�ݒ�
	XMMATRIX mtxView;
	mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Camera.pos), XMLoadFloat3(&g_Camera.at), XMLoadFloat3(&g_Camera.up));
	SetViewMatrix(&mtxView);
	XMStoreFloat4x4(&g_Camera.mtxView, mtxView);

	XMMATRIX mtxInvView;
	mtxInvView = XMMatrixInverse(nullptr, mtxView);
	XMStoreFloat4x4(&g_Camera.mtxInvView, mtxInvView);


	// �v���W�F�N�V�����}�g���b�N�X�ݒ�
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

	SetProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&g_Camera.mtxProjection, mtxProjection);

	SetShaderCamera(g_Camera.pos);
}


//=============================================================================
// �J�����̎擾
//=============================================================================
CAMERA *GetCamera(void) 
{
	return &g_Camera;
}

//=============================================================================
// �r���[�|�[�g�̐ݒ�
//=============================================================================
void SetViewPort(int type)
{
	ID3D11DeviceContext *g_ImmediateContext = GetDeviceContext();
	D3D11_VIEWPORT vp;

	g_ViewPortType = type;

	// �r���[�|�[�g�ݒ�
	switch (g_ViewPortType)
	{
	case TYPE_FULL_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_LEFT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_RIGHT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = (FLOAT)SCREEN_WIDTH / 2;
		vp.TopLeftY = 0;
		break;

	case TYPE_UP_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_DOWN_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = (FLOAT)SCREEN_HEIGHT / 2;
		break;


	}
	g_ImmediateContext->RSSetViewports(1, &vp);

}


int GetViewPortType(void)
{
	return g_ViewPortType;
}



//=============================================================================
// �J�����̎��_�ƒ����_���Z�b�g
//=============================================================================
void SetCameraAT(XMFLOAT3 pos)
{
	PLAYER *player = GetPlayer();

	// ���Վ��_���ǂ����Œ����_��ύX
	if (GetOverLook())
	{	// ���Վ��_����
		// �J�����̒����_���v���C���[�̍��W�ɂ��Ă݂�
		g_Camera.at = pos;
		g_Camera.at.y = pos.y + OFFSET_Y;
	}
	else
	{
		// �J�����̒����_���v���C���[�̍��W�ɂ��Ă݂�
		g_Camera.at = pos;
		g_Camera.at.y = pos.y + OFFSET_Y;
		g_Camera.at.x -= sinf(player->rot.y + XM_PI / 2) * CAMERA_OFFSET_X/ 2;
		g_Camera.at.z -= cosf(player->rot.y + XM_PI / 2) * CAMERA_OFFSET_X/ 2;

	}

#ifdef _DEBUG	// �f�o�b�O����\������

	g_Camera.at.y += g_DebugAT.y;

#endif

}


//=============================================================================
// ���Վ��_���ǂ�����Ԃ�
//=============================================================================
BOOL GetOverLook(void)
{
	return OverLook;
}


//=============================================================================
// �U���������
//=============================================================================
void SetTurnAround(void)
{
	g_turn = TRUE;
}


//=============================================================================
// �V�[����i�߂�
//=============================================================================
void SetSummonScene(int scene)
{
	g_SummonScene[0][scene] = TRUE;
}


//=============================================================================
// ���݂ǂ��̃V�[�������擾����
//=============================================================================
int GetScene(void)
{
	return g_NowScene;
}


//=============================================================================
// �V�[���Ǘ��z��̏�����
//=============================================================================
void ResetSummonScene(void)
{
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < SUMMON_SCENE_MAX; x++)
		{
			g_SummonScene[y][x] = FALSE;
		}
	}

	// ���@�w�V�[���ɃZ�b�g
	g_SummonScene[0][0] = TRUE;
	g_NowScene = circle;
	g_Camera.time = 0.0f;
}


//=============================================================================
// ��ʂ�h�炷�������J�n
//=============================================================================
void SetCameraShake(int time)
{
	g_Shake = TRUE;
	g_ShakeCount = time;
	g_ShakePos = { 0.0f, 0.0f ,0.0f };
}