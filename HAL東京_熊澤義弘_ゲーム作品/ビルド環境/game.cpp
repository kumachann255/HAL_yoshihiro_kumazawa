//=============================================================================
//
// �Q�[����ʏ��� [game.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "camera.h"
#include "input.h"
#include "sound.h"
#include "fade.h"
#include "game.h"

#include "player.h"
#include "enemy.h"
#include "bahamut.h"
#include "bahamut_particle.h"
#include "bahamut_bless.h"
#include "bless_particle.h"
#include "beam_particle.h"
#include "beam_particle_2.h"
#include "beam_orbit.h"
#include "chant_particle.h"
#include "holy.h"
#include "water.h"
#include "meshfield.h"
#include "tree.h"
#include "skydome.h"
#include "magic_circle.h"
#include "score.h"
#include "particle.h"
#include "holy_particle.h"
#include "holy_orbit.h"
#include "lightning.h"
#include "fire.h"
#include "collision.h"
#include "debugproc.h"
#include "screen.h"
#include "option.h"
#include "arrow.h"
#include "UI.h"
#include "light.h"
#include "barrier.h"
#include "tutorial.h"
#include "fade_white.h"
#include "wand_lightning.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define DAMAGE		(1)		// �_���[�W��


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void CheckHit(void);
void SetEnemyDeadSound(void);



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// �|�[�YON/OFF

static int g_SE = 0;			// SE�̊Ǘ�
static BOOL g_EnemySE = FALSE;	// ���݂̃t���[����SE���Đ�������

//=============================================================================
// ����������
//=============================================================================
HRESULT InitGame(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// ���C�g�̏�����
	InitLight();

	// �I�v�V�����̏�����
	InitOption();

	// �`���[�g���A���̏�����
	InitTutorial();

	// �t�B�[���h�̏�����
	InitMeshField(XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 150, 65, 30.0f, 20.0f);

	// �v���C���[�̏�����
	InitPlayer();

	// �G�l�~�[�̏�����
	InitEnemy();

	// �o�n���[�g�̏�����
	InitBahamut();

	// �o�n���[�g�̃p�[�e�B�N���̏�����
	InitBahamutParticle();

	// �o�n���[�g�̃u���X�̏�����
	InitBless();

	// �u���X�̃p�[�e�B�N���̏�����
	InitBlessParticle();

	// �r�[���̃p�[�e�B�N���̏�����
	InitBeamParticle();
	InitBeamParticle2();

	// �����@�̏�����
	InitHoly();

	// �����@�̏�����
	InitWater();

	// �؂𐶂₷
	InitTree();

	// �X�J�C�h�[���̏�����
	InitSky();

	// ���@�w�̏�����
	InitCircle();

	// �X�R�A�̏�����
	InitScore();

	// UI�̏�����
	InitUI();

	// �p�[�e�B�N���̏�����
	InitHolyParticle();
	InitHolyOrbit();
	InitBeamOrbit();
	InitChantParticle();

	// ���C�g�j���O�̏�����
	InitLightning();

	// ��̗��̏�����
	InitWandLightning();

	// �t�@�C�A�̏�����
	InitFire();

	// �o���A�̏�����
	InitBarrier();

	// ���K�C�h�̏�����
	InitArrow();

	// �X�N���[���̏�����
	InitScreen();

	// �z���C�g�t�F�[�h�̏�����
	InitFadeWhite();

	// BGM�Đ�
	PlaySound(SOUND_LABEL_BGM_game);
	
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitGame(void)
{
	// �z���C�g�t�F�[�h�̏I������
	UninitFadeWhite();

	// �X�N���[���̏I������
	UninitScreen();

	// ���K�C�h�̏I������
	UninitArrow();

	// �o���A�̏I������
	UninitBarrier();

	// �t�@�C�A�̏I������
	UninitFire();

	// ��̗��̏I������
	UninitWandLightning();

	// ���C�g�j���O�̏I������
	UninitLightning();

	// �p�[�e�B�N���̏I������
	UninitChantParticle();
	UninitBeamOrbit();
	UninitHolyOrbit();
	UninitHolyParticle();

	// UI�̏I������
	UninitUI();

	// �X�R�A�̏I������
	UninitScore();

	// ���@�w�̏I������
	UninitCircle();

	// �X�J�C�h�[���̏I������
	UninitSky();

	// �؂̏I������
	UninitTree();

	// �n�ʂ̏I������
	UninitMeshField();

	// �����@�̏I������
	UninitHoly();

	// �����@�̏I������
	UninitWater();

	// �r�[���̃p�[�e�B�N���̏I������
	UninitBeamParticle2();
	UninitBeamParticle();

	// �u���X�̃p�[�e�B�N���̏I������
	UninitBlessParticle();

	// �o�n���[�g�̃u���X�̏I������
	UninitBless();

	// �o�n���[�g�̃p�[�e�B�N���̏I������
	UninitBahamutParticle();

	// �o�n���[�g�̏I������
	UninitBahamut();

	// �G�l�~�[�̏I������
	UninitEnemy();

	// �v���C���[�̏I������
	UninitPlayer();

	// �`���[�g���A���̏I������
	UninitTutorial();

	// �I�v�V�����̏I������
	UninitOption();

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateGame(void)
{
#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_V))
	{
		g_ViewPortType_Game = (g_ViewPortType_Game + 1) % TYPE_NONE;
		SetViewPort(g_ViewPortType_Game);
	}

	if (GetKeyboardTrigger(DIK_P))
	{
		g_bPause = g_bPause ? FALSE : TRUE;
	}


#endif

	if (g_bPause == FALSE)
		return;

	// �`���[�g���A���̍X�V����
	UpdateTutorial();

	// �I�v�V�����̍X�V����
	UpdateOption();

	// �`���[�g���A����ʒ��͉�ʂ��~�߂�
	if (GetTutorialUse()) return;

	// �I�v�V�������\������Ă���Ƃ��́A��ʂ��~�߂�
	if (GetOptionUse()) return;

	// �n�ʏ����̍X�V
	UpdateMeshField();

	// �v���C���[�̍X�V����
	UpdatePlayer();

	// �G�l�~�[�̍X�V����
	UpdateEnemy();

	// �o�n���[�g�̍X�V����
	UpdateBahamut();

	// �o�n���[�g�̃p�[�e�B�N���̍X�V����
	UpdateBahamutParticle();

	// �o�n���[�g�̃u���X�̍X�V����
	UpdateBless();

	// �u���X�̃p�[�e�B�N���̍X�V����
	UpdateBlessParticle();

	// �r�[���̃p�[�e�B�N���̍X�V����
	UpdateBeamParticle();
	UpdateBeamParticle2();

	// �����@�̍X�V����
	UpdateHoly();

	// �����@�̍X�V����
	UpdateWater();

	// �؂̍X�V����
	UpdateTree();

	// �X�J�C�h�[���̍X�V����
	UpdateSky();

	// ���@�w�̍X�V����
	UpdateCircle();

	// �p�[�e�B�N���̍X�V����
	UpdateHolyParticle();
	UpdateHolyOrbit();
	UpdateBeamOrbit();
	UpdateChantParticle();

	// ���C�g�j���O�̍X�V����
	UpdateLightning();

	// ��̗��̍X�V����
	UpdateWandLightning();

	// �t�@�C�A�̍X�V����
	UpdateFire();

	// ���K�C�h�̍X�V����
	UpdateArrow();

	// �o���A�̍X�V����
	UpdateBarrier();

	// �X�N���[���̍X�V����
	UpdateScreen();

	// �z���C�g�t�F�[�h�̍X�V����
	UpdateFadeWhite();

	// �����蔻�菈��
	CheckHit();

	// �X�R�A�̍X�V����
	UpdateScore();

	// UI�̍X�V����
	UpdateUI();

	// �T�E���h�̍X�V����
	UpdateSoundFade();
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawGame0(void)
{
	// �I�u�W�F�N�g��PRE�����_�[�^�[�Q�b�g�֕`��
	SetShaderType(SHADER_SHADOWMAP__VIEW_SHADOWMAP__RESOUCE_OBJ);
	{
		// 3D�̕���`�悷�鏈��
		// �n�ʂ̕`�揈��
		DrawMeshField();

		// �؂̕`�揈��
		DrawTree();

		// �G�l�~�[�̕`�揈��
		DrawEnemy();

		// �v���C���[�̕`�揈��
		DrawPlayer();

		// �o�n���[�g�̕`�揈��
		DrawBahamut();

		// �X�J�C�h�[���̕`�揈��
		//DrawSky();
	}


	SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	Clear();

	// �e��t�������I�u�W�F�N�g
	// �I�u�W�F�N�g��PRE�����_�[�^�[�Q�b�g�֕`��
	SetShaderType(SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ);
	{
		// 3D�̕���`�悷�鏈��
		// �n�ʂ̕`�揈��
		DrawMeshField();

		// �؂̕`�揈��
		DrawTree();

		// �v���C���[�̕`�揈��
		DrawPlayer();

		// �X�J�C�h�[���̕`�揈��
		DrawSky();
	}

	// �f�B�]���u����(�G�l�~�[)
	SetShaderType(SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE);
	{
		// �G�l�~�[�̕`�揈��
		DrawEnemy();

		// �m�C�Y�e�N�X�`����ύX
		SetShaderResouces(SHADER_RESOURCE_MODE_NOISE_CENTER);
		//SetShaderType(SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE);

		// �o�n���[�g�̕`�揈��
		DrawBahamut();
	}

	// �f�B�]���u����(���@�w)
	SetShaderType(SHADER_CIRCLR__VIEW_PRE__RESOUCE_OBJ_NOISE);
	{
		// ���@�w�̕`�揈��
		DrawCircle();

		// �o���A�̕`�揈��
		DrawBarrier();
	}

	// �e��t�������Ȃ��I�u�W�F�N�g
	SetShaderType(SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ);
	{
		// �����@�̕`�揈��
		DrawHoly();

		// ���C�g�j���O�̕`�揈��
		DrawLightning();

		// �t�@�C�A�̕`�揈��
		DrawFire();

		// ���K�C�h�̕`�揈��
		DrawArrow();


		//DrawHolyParticle();
		//DrawHolyOrbit();
		//DrawBahamutParticle();
		//DrawBlessParticle();
		//DrawBeamParticle();
		//DrawBeamParticle2();
		//DrawBeamOrbit();
		//DrawChantParticle();
		//DrawWandLightning();
	}

	// �����̕`��
	SetShaderType(SHADER_BLAST__VIEW_PRE__RESOUCE_OBJ);
	{
		// �o�n���[�g�̔����̕`�揈��
		DrawBless();
	}


	// �����@�̕`��
	SetShaderType(SHADER_WATER__VIEW_PRE__RESOUCE_OBJ_NOISE);
	{
		// �����@�̕`�揈��
		DrawWater();
	}

	// �p�[�e�B�N���`��
	SetShaderMode(SHADER_MODE_PARTICLE);
	{
		DrawHolyParticle();
		DrawHolyOrbit();
		DrawBahamutParticle();
		DrawBlessParticle();
		DrawBeamParticle();
		DrawBeamParticle2();
		DrawBeamOrbit();
		DrawChantParticle();
		DrawWandLightning();
	}


	// 2D�̕���`�悷�鏈��
	// Z��r�Ȃ�
	SetDepthEnable(FALSE);

	// ���C�e�B���O�𖳌�
	SetLightEnable(FALSE);

	RunBlur();

	// PRErt���P�x���ort��LUMINACE�V�F�[�_�\�ŏ�������
	SetShaderType(SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS);
	{
		// �X�N���[���̕`�揈��
		DrawScreen();

		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// x�����ւ̃u���[���H
	// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	SetShaderType(SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE);
	{
		// �X�N���[���̕`�揈��
		DrawScreen();

		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// y�����ւ̃u���[���H
	// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	SetShaderType(SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR);
	{
		// �X�N���[���̕`�揈��
		DrawScreen();

		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// y�����ւ̃u���[���H
	// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	SetShaderType(SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS);
	{
		// �X�N���[���̕`�揈��
		DrawScreen();

		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
	SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);


	SetShaderType(DEFAULT);

	// �z���C�g�t�F�[�h�̕`�揈��
	DrawFadeWhite();

	// �X�R�A�̕`�揈��
	DrawScore();

	// UI�̕`��
	DrawUI();

	// �`���[�g���A���̕`�揈��
	DrawTutorial();

	// �I�v�V�����̕`�揈��
	DrawOption();


	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// Z��r����
	SetDepthEnable(TRUE);
}


void DrawGame(void)
{
	XMFLOAT3 pos;


#ifdef _DEBUG
	// �f�o�b�O�\��
	PrintDebugProc("ViewPortType:%d\n", g_ViewPortType_Game);

#endif

	// �v���C���[���_
	pos = GetPlayer()->pos;
	pos.y = 0.0f;			// �J����������h�����߂ɃN���A���Ă���

	// �������@���������Ă��Ȃ��ꍇ�͒����_�̓v���C���[�t��
	if(!GetSummon()) SetCameraAT(pos);
	SetCamera();

	switch(g_ViewPortType_Game)
	{
	case TYPE_FULL_SCREEN:
		SetViewPort(TYPE_FULL_SCREEN);
		DrawGame0();
		break;

	case TYPE_LEFT_HALF_SCREEN:
	case TYPE_RIGHT_HALF_SCREEN:
		SetViewPort(TYPE_LEFT_HALF_SCREEN);
		DrawGame0();

		// �G�l�~�[���_
		pos = GetEnemy()->pos;
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_RIGHT_HALF_SCREEN);
		DrawGame0();
		break;

	case TYPE_UP_HALF_SCREEN:
	case TYPE_DOWN_HALF_SCREEN:
		SetViewPort(TYPE_UP_HALF_SCREEN);
		DrawGame0();

		// �G�l�~�[���_
		pos = GetEnemy()->pos;
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_DOWN_HALF_SCREEN);
		DrawGame0();
		break;

	}
}


//=============================================================================
// �����蔻�菈��
//=============================================================================
void CheckHit(void)
{
	ENEMY *enemy = GetEnemy();		// �G�l�~�[�̃|�C���^�[��������
	PLAYER *player = GetPlayer();	// �v���C���[�̃|�C���^�[��������
	FIRE *fire = GetFire();
	LIGHTNING_COLLI *lightning = GetLightningCollition();
	HOLY *holy = GetHoly();
	WATER *water = GetWater();


	// ������
	g_EnemySE = FALSE;

	// �G�ƃv���C���[�L����
	if (player->hitCheck)
	{
		for (int i = 0; i < MAX_ENEMY; i++)
		{
			//�G�̗L���t���O���`�F�b�N����
			if ((enemy[i].use == FALSE) || (enemy[i].end))
				continue;

			//BC�̓����蔻��
			if (CollisionBC(player->pos, enemy[i].pos, player->size, enemy[i].size))
			{
				// �v���C���[��HP�����炷
				player->HP -= DAMAGE;

				// �v���C���[�𖳓G���Ԃ�
				player->hitCheck = FALSE;

				// ��ʂ������h�炷
				SetCameraShake(20);

				// HP��0�̎��͕ʃ{�C�X���Đ�
				if (player->HP > 0)
				{
					// �_���[�W���󂯂�����SE���Đ�
					// 2��ނ��烉���_��
					char se = rand() % 2;

					switch (se)
					{
					case 0:
						PlaySound(SOUND_LABEL_SE_PLAYER_damage0);
						break;

					case 1:
						PlaySound(SOUND_LABEL_SE_PLAYER_damage1);
						break;
					}
				}
			}
		}
	}


	// �v���C���[�̉����@�ƓG
	for (int i = 0; i < MAX_FIRE; i++)
	{
		//�e�̗L���t���O���`�F�b�N����
		if (fire[i].bUse == FALSE)
			continue;

		// ��������������s�������͍̂ēx������s��Ȃ�
		if (fire[i].atk == TRUE)
			continue;

		// �G�Ɠ������Ă邩���ׂ�
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//�G�̗L���t���O���`�F�b�N����
			if (enemy[j].use == FALSE)
				continue;

			//BC�̓����蔻��
			if (CollisionBC(fire[i].pos, enemy[j].pos, fire[i].fSizeY, enemy[j].size))
			{
				// �����������瓖����������ɂ���
				fire[i].atk = TRUE;

				// �G�̎��ʂƂ���SE���Đ�
				if (enemy[j].end == FALSE)
				{
					SetEnemyDeadSound();
				}

				// �G�L�����N�^�[�͓|�����
				enemy[j].end = TRUE;

				// �X�R�A�𑫂�
				AddScore(10);
			}
		}
	}


	// �v���C���[�̗����@�ƓG
	for (int i = 0; i < MAX_LIGHTNING; i++)
	{
		//�e�̗L���t���O���`�F�b�N����
		if (lightning[i].bUse == FALSE)
			continue;

		// �G�Ɠ������Ă邩���ׂ�
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//�G�̗L���t���O���`�F�b�N����
			if (enemy[j].use == FALSE)
				continue;

			//BC�̓����蔻��
			if (CollisionBB(lightning[i].pos, lightning[i].fSizeX, lightning[i].fSizeY, lightning[i].fSizeZ, enemy[j].pos, enemy[j].size, enemy[j].size, enemy[j].size))
			{
				// �G�̎��ʂƂ���SE���Đ�
				if (enemy[j].end == FALSE)
				{
					SetEnemyDeadSound();
				}

				// �G�L�����N�^�[�͓|�����
				enemy[j].end = TRUE;

				// �X�R�A�𑫂�
				AddScore(10);
			}
		}
	}


	// �v���C���[�̌����@�ƓG
	for (int i = 0; i < MAX_HOLY; i++)
	{
		//�e�̗L���t���O���`�F�b�N����
		if (holy[i].use == FALSE)
			continue;

		// �G�Ɠ������Ă邩���ׂ�
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//�G�̗L���t���O���`�F�b�N����
			if (enemy[j].use == FALSE)
				continue;

			//BC�̓����蔻��
			if (CollisionBC(holy[i].pos, enemy[j].pos, holy[i].size, enemy[j].size))
			{
				// �G�̎��ʂƂ���SE���Đ�
				if (enemy[j].end == FALSE)
				{
					SetEnemyDeadSound();
				}

				// �G�L�����N�^�[�͓|�����
				enemy[j].end = TRUE;

				// �X�R�A�𑫂�
				AddScore(10);
			}
		}
	}


	// �v���C���[�̐����@�ƓG
	for (int i = 0; i < MAX_WATER; i++)
	{
		//�e�̗L���t���O���`�F�b�N����
		if (water[i].use == FALSE)
			continue;

		// �G�Ɠ������Ă邩���ׂ�
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//�G�̗L���t���O���`�F�b�N����
			if (enemy[j].use == FALSE)
				continue;

			//BC�̓����蔻��
			if (CollisionBC(water[i].pos, enemy[j].pos, water[i].size, enemy[j].size))
			{
				// �G�l�~�[�𓮂��Ȃ�����
				enemy[j].moveCan = FALSE;

				// �G�l�~�[����]������
				enemy[j].rot.x += 0.01f;
				enemy[j].rot.y += 0.01f;
				enemy[j].rot.z += 0.01f;

				// �G�l�~�[�𐅋ʂ̒��S�ɋ߂Â���
				float dis;
				// x
				dis = water[i].pos.x - enemy[j].pos.x;
				enemy[j].pos.x += dis / 10.0f;
				// y
				dis = water[i].pos.y - enemy[j].pos.y;
				enemy[j].pos.y += dis / 10.0f;
				// z
				dis = water[i].pos.z - enemy[j].pos.z;
				enemy[j].pos.z += dis / 10.0f;


				// �_�f�l�����炷
				enemy[j].waterCount--;
				if (enemy[j].waterCount < 0)
				{
					// �G�̎��ʂƂ���SE���Đ�
					if (enemy[j].end == FALSE)
					{
						SetEnemyDeadSound();
					}

					// �G�L�����N�^�[�͓|�����
					enemy[j].end = TRUE;
				}

				// �X�R�A�𑫂�
				AddScore(10);
			}
		}
	}


	// �������@������������o�ꂵ�Ă���G�l�~�[�͂��ׂē|�����
	if (GetSummon())
	{
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//�G�̗L���t���O���`�F�b�N����
			if (enemy[j].use == FALSE) continue;

			enemy[j].end = TRUE;

			// �X�R�A�𑫂�
			AddScore(10);
		}
	}


	// �G�l�~�[���S�����S�������ԑJ��
	int enemy_count = 0;
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if ((enemy[i].use == FALSE) && (enemy[i].popCount == -1)) continue;
		enemy_count++;
	}

	// �G�l�~�[���O�C�H
	if (enemy_count == 0)
	{
		SetFade(FADE_OUT, MODE_CLEAR);
	}

	// �v���C���[��HP��0�ɂȂ�����V�[���J��
	if ((player->HP <= 0) && (player->HP != -100))
	{
		SetFade(FADE_OUT, MODE_OVER);
		
		// �{�C�X���d�Ȃ�Ȃ��悤�ɒ���
		player->HP = -100;

		// �����_���Ŏ��S���̃{�C�X���Đ�
		int voice = rand() % 3;
		switch (voice)
		{
		case 0:
			PlaySound(SOUND_LABEL_SE_PLAYER_dead0);
			SetSoundFade(SOUND_LABEL_SE_PLAYER_dead0, SOUNDFADE_OUT, 0.0f, 1.0f);
			break;

		case 1:
			PlaySound(SOUND_LABEL_SE_PLAYER_dead1);
			SetSoundFade(SOUND_LABEL_SE_PLAYER_dead1, SOUNDFADE_OUT, 0.0f, 1.0f);
			break;

		case 2:
			PlaySound(SOUND_LABEL_SE_PLAYER_dead2);
			SetSoundFade(SOUND_LABEL_SE_PLAYER_dead2, SOUNDFADE_OUT, 0.0f, 1.0f);
			break;
		}

		// BGM���t�F�[�h�A�E�g
		SetTitleSoundFade(SOUND_LABEL_BGM_game, SOUNDFADE_OUT, 0.0f, 1.4f);
	}
}


// SE���d�Ȃ�悤�ɕʉ����ōĐ����鏈��
void SetEnemyDeadSound(void)
{
	if(g_EnemySE) return;

	g_SE++;
	g_EnemySE = TRUE;

	switch (g_SE % 7)
	{
	case 0:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead0);
		break;

	case 1:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead1);
		break;

	case 2:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead2);
		break;

	case 3:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead3);
		break;

	case 4:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead4);
		break;

	case 5:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead5);
		break;

	case 6:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead6);
		break;
	}
}