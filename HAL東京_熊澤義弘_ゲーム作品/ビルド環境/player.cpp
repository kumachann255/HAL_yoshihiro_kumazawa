//=============================================================================
//
// ���f������ [player.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "model.h"
#include "player.h"
#include "light.h"
#include "meshfield.h"
#include "lightning.h"
#include "fire.h"
#include "holy.h"
#include "water.h"
#include "magic_circle.h"
#include "sound.h"
#include "tutorial.h"
#include "bahamut.h"
#include "chant_particle.h"
#include "bahamut.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/player_body.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PARTS_CLOAK	"data/MODEL/player_cloak.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PARTS_FEMUR	"data/MODEL/player_femur.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PARTS_KNEE	"data/MODEL/player_knee.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PARTS_LEG		"data/MODEL/player_leg.obj"				// �ǂݍ��ރ��f����
#define	MODEL_PARTS_CLOAK_MOVE_0	"data/MODEL/player_cloak_mof_0.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PARTS_CLOAK_MOVE_1	"data/MODEL/player_cloak_mof_1.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PARTS_CLOAK_MOVE_2	"data/MODEL/player_cloak_mof_2.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PARTS_CLOAK_MOVE_3	"data/MODEL/player_cloak_mof_3.obj"			// �ǂݍ��ރ��f����
#define	MODEL_PARTS_WAND_0	"data/MODEL/wand.obj"					// �ǂݍ��ރ��f����
#define	MODEL_PARTS_WAND_1	"data/MODEL/wand_fire.obj"					// �ǂݍ��ރ��f����
#define	MODEL_PARTS_WAND_2	"data/MODEL/wand_lightning.obj"					// �ǂݍ��ރ��f����
#define	MODEL_PARTS_WAND_3	"data/MODEL/wand_water.obj"					// �ǂݍ��ރ��f����
#define	MODEL_PARTS_WAND_4	"data/MODEL/wand_holy.obj"					// �ǂݍ��ރ��f����

#define MAX_PLAYER_HP		(10)							// �v���C���[��HP
#define MAX_INVINCIBLE		(120)							// �_���[�W��H������ۂ̖��G����

#define	VALUE_MOVE			(2.0f)							// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// ��]��

#define PLAYER_SHADOW_SIZE	(1.0f)							// �e�̑傫��
#define PLAYER_OFFSET_Y		(20.0f)							// �v���C���[�̑��������킹��

#define PLAYER_PARTS_MAX	(8)								// �v���C���[�̃p�[�c�̐�

#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)					// �v���C���[�̉�]��

#define MAX_PLAYER_MOVE		(5)								// ���[�t�B���O�̃��f����
#define MAX_WAND_MOVE		(5)								// ���[�t�B���O�̃��f����

#define WAND_OFFSET			(20)			// ����\���鎞��
#define WATER_CHANT_COUNT	(90)			// �����@�̉r������
#define WATER_FIRING_COUNT	(140)			// �����@�̔����܂ł̎���

#define LIGNHTNING_CHANT_COUNT	(90)		// �����@�̉r������
#define LIGNHTNING_FIRING_COUNT	(120)		// �����@�̔����܂ł̎���
#define LIGNHTNING_RESET_COUNT	(20)		// �����@�Ĕ����܂ł̎���

#define HOLY_CHANT_COUNT	(60)			// �����@�̉r������
#define HOLY_MORPHING_COUNT	(90)			// �����@�̃��[�t�B���O�J�n����
#define HOLY_FIRING_COUNT	(150)			// �����@�̔����܂ł̎���

#define WAND_DISSOLVE		(0.005f)		// ��̃f�B�]���u���x

#define SUMMON_CHANT_COUNT	(240)			// �������@�̉r������


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void RunAnimation(int i, BOOL data);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static PLAYER		g_Player;						// �v���C���[

static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// �v���C���[�̃p�[�c�p

static BOOL			g_Load = FALSE;

static MODEL		g_Player_Vertex[MAX_PLAYER_MOVE];	// ���[�t�B���O�p���f���̒��_��񂪓������f�[�^�z��
static VERTEX_3D	*g_Vertex = NULL;					// �r���o�߂��L�^����ꏊ

static MODEL		g_Wand_Vertex[MAX_WAND_MOVE];		// ���[�t�B���O�p���f���̒��_��񂪓������f�[�^�z��
static VERTEX_3D	*g_wandVertex = NULL;				// �r���o�߂��L�^����ꏊ
static COLOR		g_wandSubset;				// �F�̓r���o�߂��L�^����ꏊ

static float		g_time;					// ���[�t�B���O�̎��Ԃ��Ǘ�
static int			mof;					// ���[�t�B���O�̃��f���ԍ����Ǘ�
static BOOL			g_gait;					// �������[�V�����Ɉڍs���邩�ǂ���
static BOOL			g_gait_old;				// �O�̃t���[���ŕ����Ă�����
static BOOL			g_magic;				// ���@���[�V�����Ɉڍs���邩�ǂ���
static BOOL			g_magicChant;			// �r�����[�V�����Ɉڍs���邩�ǂ���
static BOOL			g_magic_old;			// �O�̃t���[���Ŗ��@���g�p���Ă�����
static int			g_chantCount;			// �r���̌o�ߎ���
static int			g_LightningCount;		// �����@�̃��Z�b�g�܂ł̎���
static float		g_Speed;				// ���s�X�s�[�h���Ǘ�
static float		g_Wand;					// ��̃f�B�]���u�Ǘ�
static BOOL			g_WandDissolve;			// ��̃f�B�]���u�`�惂�[�h�ɂȂ��Ă��邩
static float		g_Wnadtime;				// ��̃��[�t�B���O�̎��Ԃ��Ǘ�
static int			g_WandMorph;			// ��̃��[�t�B���O�̃��f���ԍ����Ǘ�
static int			g_MagicType;			// ���݉����r������(4:�r�����Ă��Ȃ�)

static float		g_DebugY;				// �f�o�b�O�p��y���W��ύX����

// �v���C���[�̊K�w�A�j���[�V�����f�[�^
static INTERPOLATION_DATA move_tbl_cloak[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(30), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },

};

static INTERPOLATION_DATA move_tbl_cloak_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};


static INTERPOLATION_DATA move_tbl_femur_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(45), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���㔼
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(30), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �׏d������
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-5), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 38 / 4 },	// ���r����
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���O��
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-50), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���㔼
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-20), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �O�V�r��
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(15), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 26 / 4 },	// �V�r����
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(35), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �V�r����
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���O��
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(45), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },		// �V�r�I���㔼
};

static INTERPOLATION_DATA move_tbl_femur_right_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���㔼
};


static INTERPOLATION_DATA move_tbl_femur_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-50), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���㔼
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-20), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �O�V�r��
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(15), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 26 / 4 },	// �V�r����
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(35), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �V�r����
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���O��
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(45), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���㔼
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(30), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �׏d������
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-5), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 38 / 4 },	// ���r����
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���O��
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-50), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },		// ���r�I���㔼
};

static INTERPOLATION_DATA move_tbl_femur_left_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���㔼
};


static INTERPOLATION_DATA move_tbl_knee_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���㔼
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �׏d������
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 38 / 4 },	// ���r����
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���O��
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���㔼
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �O�V�r��
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(90), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 26 / 4 },	// �V�r����
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(25), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �V�r����
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(15), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���O��
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },		// �V�r�I���㔼
};

static INTERPOLATION_DATA move_tbl_knee_left_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���㔼
};


static INTERPOLATION_DATA move_tbl_knee_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���㔼
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �O�V�r��
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(90), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 26 / 4 },	// �V�r����
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(25), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �V�r����
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(15), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���O��
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���㔼
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// �׏d������
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 38 / 4 },	// ���r����
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���O��
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },		// ���r�I���㔼
};

static INTERPOLATION_DATA move_tbl_knee_right_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���㔼
};


static INTERPOLATION_DATA move_tbl_leg_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

static INTERPOLATION_DATA move_tbl_leg_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

static INTERPOLATION_DATA move_tbl_wand[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, -5.0f, 9.0f),	XMFLOAT3(0.0f, 0.0f, RadianSum(-45)),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

// �r����
static INTERPOLATION_DATA move_tbl_wand_chant[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 10.0f, 25.0f),		XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-90)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(2.0f, 10.0f, 20.0f),		XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-85)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 40 },
	{ XMFLOAT3(-2.0f, 10.0f, 22.0f),	XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-90)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(0.0f, 9.0f, 20.0f),		XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-95)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(0.0f, 10.0f, 25.0f),		XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-90)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

// ���Ɛ����@
static INTERPOLATION_DATA move_tbl_wand_fire_water[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(5.0f, -0.0f, 5.0f),		XMFLOAT3(RadianSum(-30), RadianSum(90), RadianSum(80)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

// ���ƌ����@
static INTERPOLATION_DATA move_tbl_wand_lightning_holy[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(8.0f, 15.0f, 15.0f),		XMFLOAT3(RadianSum(10), RadianSum(30), RadianSum(30)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};


//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayer(void)
{
	LoadModel(MODEL_PLAYER, &g_Player.model, 0);
	g_Player.load = TRUE;

	g_Player.pos = { 0.0f, PLAYER_OFFSET_Y, 0.0f };
	g_Player.rot = { 0.0f, 0.0f, 0.0f };
	g_Player.scl = { 1.0f, 1.0f, 1.0f };

	g_Player.spd = 0.0f;			// �ړ��X�s�[�h�N���A
	g_Player.size = PLAYER_SIZE;	// �����蔻��̑傫��
	g_Player.hitCheck = TRUE;
	g_Player.HP = MAX_PLAYER_HP;
	g_Player.hitTime = 0;
	g_Player.use = TRUE;

	// �K�w�A�j���[�V�����p�̏���������
	g_Player.parent = NULL;			// �{�́i�e�j�Ȃ̂�NULL������

	// �p�[�c�̏�����
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		g_Parts[i].use = FALSE;

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

	g_Parts[0].use = TRUE;
	g_Parts[0].tbl_adr  = move_tbl_cloak_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[0].tbl_size = sizeof(move_tbl_cloak_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[0].load = 1;
	LoadModel(MODEL_PARTS_CLOAK, &g_Parts[0].model, 1);

	g_Parts[1].use = TRUE;
	g_Parts[1].tbl_adr  = move_tbl_femur_right_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[1].tbl_size = sizeof(move_tbl_femur_right_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[1].load = 1;
	LoadModel(MODEL_PARTS_FEMUR, &g_Parts[1].model, 0);

	g_Parts[2].use = TRUE;
	g_Parts[2].tbl_adr  = move_tbl_femur_left_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[2].tbl_size = sizeof(move_tbl_femur_left_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[2].load = 1;
	LoadModel(MODEL_PARTS_FEMUR, &g_Parts[2].model, 0);

	g_Parts[3].use = TRUE;
	g_Parts[3].tbl_adr  = move_tbl_knee_right_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[3].tbl_size = sizeof(move_tbl_knee_right_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[3].load = 1;
	LoadModel(MODEL_PARTS_KNEE, &g_Parts[3].model, 0);

	g_Parts[4].use = TRUE;
	g_Parts[4].tbl_adr  = move_tbl_knee_left_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[4].tbl_size = sizeof(move_tbl_knee_left_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[4].load = 1;
	LoadModel(MODEL_PARTS_KNEE, &g_Parts[4].model, 0);

	g_Parts[5].use = TRUE;
	g_Parts[5].tbl_adr  = move_tbl_leg_right;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[5].tbl_size = sizeof(move_tbl_leg_right) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[5].load = 1;
	LoadModel(MODEL_PARTS_LEG, &g_Parts[5].model, 0);

	g_Parts[6].use = TRUE;
	g_Parts[6].tbl_adr  = move_tbl_leg_left;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[6].tbl_size = sizeof(move_tbl_leg_left) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[6].load = 1;
	LoadModel(MODEL_PARTS_LEG, &g_Parts[6].model, 0);

	g_Parts[7].use = TRUE;
	g_Parts[7].tbl_adr = move_tbl_wand_chant;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[7].load = 1;
	LoadModel(MODEL_PARTS_WAND_0, &g_Parts[7].model, 1);


	// �e�q�֌W
	g_Parts[0].parent = &g_Player;		// �}���g�̐e�͖{��
	g_Parts[1].parent = &g_Player;		// �E��ڂ̐e�͖{��
	g_Parts[2].parent = &g_Player;		// ����ڂ̐e�͖{��
	g_Parts[3].parent = &g_Parts[1];	// �E�G�̐e�͉E���
	g_Parts[4].parent = &g_Parts[2];	// ���G�̐e�͍����
	g_Parts[5].parent = &g_Parts[3];	// �E���ڂ̐e�͉E�G
	g_Parts[6].parent = &g_Parts[4];	// �����ڂ̐e�͍��G
	g_Parts[7].parent = &g_Player;		// ��̐e�͖{��

	// �}���g
	// ���[�t�B���O����I�u�W�F�N�g�̓ǂݍ���
	LoadObj(MODEL_PARTS_CLOAK, &g_Player_Vertex[0]);
	LoadObj(MODEL_PARTS_CLOAK_MOVE_0, &g_Player_Vertex[1]);
	LoadObj(MODEL_PARTS_CLOAK_MOVE_1, &g_Player_Vertex[2]);
	LoadObj(MODEL_PARTS_CLOAK_MOVE_2, &g_Player_Vertex[3]);
	LoadObj(MODEL_PARTS_CLOAK_MOVE_3, &g_Player_Vertex[4]);

	// ���g��z��Ƃ��Ďg�p�ł���悤�Ɏd�l�ύX
	g_Vertex = new VERTEX_3D[g_Player_Vertex[0].VertexNum];

	// ����(�r���o��)�̏�����
	for (int i = 0; i < g_Player_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Player_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse = g_Player_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal = g_Player_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Player_Vertex[0].VertexArray[i].TexCoord;
	}

	// ��
	// ���[�t�B���O����I�u�W�F�N�g�̓ǂݍ���
	LoadObj(MODEL_PARTS_WAND_0, &g_Wand_Vertex[0]);
	LoadObj(MODEL_PARTS_WAND_1, &g_Wand_Vertex[1]);
	LoadObj(MODEL_PARTS_WAND_2, &g_Wand_Vertex[2]);
	LoadObj(MODEL_PARTS_WAND_3, &g_Wand_Vertex[3]);
	LoadObj(MODEL_PARTS_WAND_4, &g_Wand_Vertex[4]);

	// ���g��z��Ƃ��Ďg�p�ł���悤�Ɏd�l�ύX
	g_wandVertex = new VERTEX_3D[g_Wand_Vertex[0].VertexNum];


	// ����(�r���o��)�̏�����
	for (int i = 0; i < g_Wand_Vertex[0].VertexNum; i++)
	{
		g_wandVertex[i].Position = g_Wand_Vertex[0].VertexArray[i].Position;
		g_wandVertex[i].Diffuse  = g_Wand_Vertex[0].VertexArray[i].Diffuse;
		g_wandVertex[i].Normal   = g_Wand_Vertex[0].VertexArray[i].Normal;
		g_wandVertex[i].TexCoord = g_Wand_Vertex[0].VertexArray[i].TexCoord;
	}

	// ���g��z��Ƃ��Ďg�p�ł���悤�Ɏd�l�ύX
	g_wandSubset.SubsetArray = new SUBSET[g_Wand_Vertex[0].SubsetNum];

	// �����̏�����
	for (int i = 0; i < g_Wand_Vertex[0].SubsetNum; i++)
	{
		g_wandSubset.SubsetArray[i] = g_Wand_Vertex[0].SubsetArray[i];
	}
	g_wandSubset.SubsetNum = g_Wand_Vertex[0].SubsetNum;


	// �ϐ��̏�����
	g_time = 0.0f;
	mof = 0;
	g_gait = FALSE;
	g_gait_old = FALSE;
	g_magic = FALSE;
	g_magicChant = FALSE;
	g_magic_old = FALSE;
	g_chantCount = 0;
	g_LightningCount = 0;
	g_Speed = VALUE_MOVE;
	g_Wand = 0.0f;
	g_WandDissolve = FALSE;
	g_Wnadtime = 0.0f;
	g_WandMorph = 0;
	g_DebugY = 0.0f;
	g_MagicType = null;

	// �T�E���h�֘A
	PlaySound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�����
	PauseSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���~�����Ă���

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPlayer(void)
{
	if (g_Load == FALSE) return;

	// ���f���̉������
	if (g_Player.load)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdatePlayer(void)
{
	// �������@�����������瑀��s��
	if (GetSummon()) return;

	CAMERA *cam = GetCamera();

	// �Ƃ肠���������Ă��Ȃ�,���@���g�p���Ă��Ȃ��ݒ�
	SetGait(FALSE);
	g_magic = FALSE;
	g_magicChant = FALSE;
	SetCircle(TRUE, TRUE, null);
	SetChantParticleSwitch(FALSE);

	// ����SE����U��~
	// ���ۂ̑��̓�����SE�����킹�邽�߂ɒ�~�ƍĐ����g�p
	PauseSound(SOUND_LABEL_SE_PLAYER_gait);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_gait, 0.2f);			// ����SE�̉��ʒ���

	// �ŏ��Ƀ`���[�g���A�����J�n
	SetTutorial(tx_start);

	// �����@
	if (((GetKeyboardPress(DIK_UP)) || (IsButtonPressed(0, BUTTON_X))) &&
		GetTutorialClear(tx_start) && ((g_MagicType == fire) || (g_MagicType == null)))
	{
		g_chantCount++;
		g_magic = TRUE;
		SetCircle(TRUE, FALSE, fire);
		g_MagicType = fire;

		// ���@�����|�[�Y�ֈڍs(���`���)
		XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand_fire_water->pos);	// ���̏ꏊ
		XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// ���݂̏ꏊ
		XMVECTOR vec = p1 - p0;
		XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

		p1 = XMLoadFloat3(&move_tbl_wand_fire_water->rot);	// ���̏ꏊ
		p0 = XMLoadFloat3(&g_Parts[7].rot);			// ���݂̏ꏊ
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);

		// ���@�����|�[�Y���Ƃ�܂ł̑ҋ@����
		if (g_chantCount > WAND_OFFSET * 3)
		{
			SetFireSwitch(TRUE);

			// �`���[�g���A���N���A
			SetTutorialClear(tx_fire);
		}
	}
	else
	{
		SetFireSwitch(FALSE);	// ���g�p��
		ResetFireTime();		// �A���������Ԃ����Z�b�g
	}


	// �����@
	//if (((GetKeyboardPress(DIK_LEFT)) || (IsButtonPressed(0, BUTTON_C))))
	if (((GetKeyboardPress(DIK_LEFT)) || (IsButtonPressed(0, BUTTON_C))) &&
		GetTutorialClear(tx_lightning) && ((g_MagicType == water) || (g_MagicType == null)))
	{
		g_chantCount++;

		// �r���|�[�Y�ֈڍs(�K�w�A�j���[�V����)
		g_Parts[7].tbl_adr = move_tbl_wand_chant;
		g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);
		g_magicChant = TRUE;
		SetCircle(TRUE, FALSE, water);
		g_MagicType = water;

		// �r�����Ԃ����𒴂�����
		if (g_chantCount > WATER_CHANT_COUNT)
		{
			if (g_chantCount == WATER_CHANT_COUNT + 1) SetWater();

			// �r�����Ԃ�����Ɉ��𒴂�����
			if (g_chantCount > WATER_FIRING_COUNT)
			{
				if ((GetWaterUse(0) == TRUE) && (g_chantCount == WATER_FIRING_COUNT + WAND_OFFSET))
				{
					SetFiringWater();
				}

				// ���@�����|�[�Y�ֈڍs(���`���)
				g_magic = TRUE;
				g_magicChant = FALSE;

				XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand_fire_water->pos);	// ���̏ꏊ
				XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// ���݂̏ꏊ
				XMVECTOR vec = p1 - p0;
				XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

				p1 = XMLoadFloat3(&move_tbl_wand_fire_water->rot);	// ���̏ꏊ
				p0 = XMLoadFloat3(&g_Parts[7].rot);			// ���݂̏ꏊ
				vec = p1 - p0;
				XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);

			}
		}
	}


	// �����@
	if (((GetKeyboardPress(DIK_RIGHT)) || (IsButtonPressed(0, BUTTON_B))) &&
		GetTutorialClear(tx_fire) && ((g_MagicType == lightning) || (g_MagicType == null)))
	{
		g_chantCount++;
		g_LightningCount = 0;

		// �r���|�[�Y�ֈڍs(�K�w�A�j���[�V����)
		g_Parts[7].tbl_adr = move_tbl_wand_chant;
		g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);
		g_magicChant = TRUE;
		SetCircle(TRUE, FALSE, lightning);
		g_MagicType = lightning;

		// �r�����Ԃ����𒴂�����
		if (g_chantCount > LIGNHTNING_FIRING_COUNT)
		{
			if (g_chantCount == LIGNHTNING_FIRING_COUNT + WAND_OFFSET)
			{
				SetLightning();
			}

			// ��莞�Ԃ��Ƃɗ����@���Ĕ���
			if ((g_chantCount % LIGNHTNING_RESET_COUNT == 0) &&
				(g_chantCount > LIGNHTNING_FIRING_COUNT + WAND_OFFSET + LIGNHTNING_RESET_COUNT))
			{
				SetLightning();
			}
			// ���@�����|�[�Y�ֈڍs(���`���)
			g_magic = TRUE;
			g_magicChant = FALSE;

			XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand_lightning_holy->pos);	// ���̏ꏊ
			XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// ���݂̏ꏊ
			XMVECTOR vec = p1 - p0;
			XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

			p1 = XMLoadFloat3(&move_tbl_wand_lightning_holy->rot);	// ���̏ꏊ
			p0 = XMLoadFloat3(&g_Parts[7].rot);				// ���݂̏ꏊ
			vec = p1 - p0;
			XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);
		}
	}
	else
	{
		g_LightningCount++;

		if (g_LightningCount == LIGNHTNING_RESET_COUNT * 2)
		{
			for (int z = 0; z < MAX_LIGHTNING; z++)
			{
				ResetLightning(z);
			}
		}

		SetLightningOnOff(FALSE);
	}


	// �����@
	if (((GetKeyboardPress(DIK_DOWN)) || (IsButtonPressed(0, BUTTON_A))) &&
		GetTutorialClear(tx_water) && ((g_MagicType == holy) || (g_MagicType == null)))
	//if (((GetKeyboardPress(DIK_DOWN)) || (IsButtonPressed(0, BUTTON_A))))
		{
		g_chantCount++;

		// �r���|�[�Y�ֈڍs(�K�w�A�j���[�V����)
		g_Parts[7].tbl_adr = move_tbl_wand_chant;
		g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);
		g_magicChant = TRUE;
		SetCircle(TRUE, FALSE, holy);
		g_MagicType = holy;

		// �r�����Ԃ����𒴂�����
		if (g_chantCount > HOLY_CHANT_COUNT)
		{
			HOLY *holy = GetHoly();

			if (g_chantCount == HOLY_CHANT_COUNT + 1) SetHoly();
			if (g_chantCount == HOLY_MORPHING_COUNT) SetMove();

			// �r�����Ԃ�����Ɉ��𒴂�����
			if (g_chantCount > HOLY_FIRING_COUNT)
			{
				// �����@����
				if (g_chantCount == HOLY_FIRING_COUNT + WAND_OFFSET)
				{
					SetFiringSword();
				}

				// ���@�����|�[�Y�ֈڍs(���`���)
				g_magic = TRUE;
				g_magicChant = FALSE;

				XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand_lightning_holy->pos);	// ���̏ꏊ
				XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// ���݂̏ꏊ
				XMVECTOR vec = p1 - p0;
				XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

				p1 = XMLoadFloat3(&move_tbl_wand_lightning_holy->rot);	// ���̏ꏊ
				p0 = XMLoadFloat3(&g_Parts[7].rot);			// ���݂̏ꏊ
				vec = p1 - p0;
				XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);

			}
		}
	}


	// �������@
	if (((GetKeyboardPress(DIK_RIGHT) && GetKeyboardPress(DIK_LEFT)) || 
		(IsButtonPressed(0, BUTTON_C) && IsButtonPressed(0, BUTTON_B))))
	{
		g_chantCount++;

		// �p�[�e�B�N���𔭐�
		SetChantParticleSwitch(TRUE);

		// �r���|�[�Y�ֈڍs(�K�w�A�j���[�V����)
		g_Parts[7].tbl_adr = move_tbl_wand_chant;
		g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);
		g_magicChant = TRUE;
		g_MagicType = null;
		// SetCircle(TRUE, FALSE, holy);

		// �r�����Ԃ����𒴂�����
		if (g_chantCount > SUMMON_CHANT_COUNT)
		{
			SetSummon();
		}
	}


	// ���@���I��������̈ʒu��߂�
	if ((g_magicChant == FALSE) && (g_magic == FALSE))
	{
		g_chantCount = 0;

		XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand->pos);	// ���̏ꏊ
		XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// ���݂̏ꏊ
		XMVECTOR vec = p1 - p0;
		XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

		p1 = XMLoadFloat3(&move_tbl_wand->rot);	// ���̏ꏊ
		p0 = XMLoadFloat3(&g_Parts[7].rot);			// ���݂̏ꏊ
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);
	}

	// ���݂̃t���[���Ŗ��@���r�����Ă��邩���L�^���āA���̃t���[���Ŏg�p����
	g_magic_old = g_magic;



	// �����蔻��Ɩ��G���Ԃ̊Ǘ�
	if (g_Player.hitCheck == FALSE)
	{	// ���G���Ԃ̏ꍇ
		g_Player.hitTime++;		// ���Ԃ�i�߂�

		// �v���C���[�̐F�����������ɂ���
		for (int p = 0; p < g_Player_Vertex[0].VertexNum; p++)
		{
			if (g_Player.hitTime % 10 > 5)
			{
				g_Vertex[p].Diffuse.w = 0.5f;
				//g_Vertex[p].Diffuse.z = 0.5f;
			}
			else
			{
				g_Vertex[p].Diffuse.w = 1.0f;
				//g_Vertex[p].Diffuse.z = 1.0f;
			}
		}
		// �ړ����x�������x������
		g_Speed = VALUE_MOVE / 2;
		if (g_Player.hitTime >= MAX_INVINCIBLE)
		{	// ���G���Ԃ̌o�ߎ��Ԃ��߂����疳�G���������āA�����蔻���߂�
			g_Player.hitCheck = TRUE;
			g_Player.hitTime = 0;

			// �ړ����x��߂�
			g_Speed = VALUE_MOVE;

			// �v���C���[�̐F��߂�
			for (int p = 0; p < g_Player_Vertex[0].VertexNum; p++)
			{
				g_Vertex[p].Diffuse = g_Player_Vertex[0].VertexArray[p].Diffuse;
			}
		}
	}




#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Player.pos.z = g_Player.pos.x = 0.0f;
		g_Player.rot.y = g_Player.dir = 0.0f;
		g_Player.spd = 0.0f;
	}
#endif

	// ���Վ��_:��  TPS���_:�O�i
	if ((GetKeyboardPress(DIK_W)) || ((IsButtonPressed(0, BUTTON_UP))))
	{
		g_Player.spd = g_Speed;
		g_Player.dir = XM_PI;

		SetGait(TRUE);
		ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
	}


	// ���Վ��_���ǂ����ňړ�������ύX
	if (GetOverLook())
	{	// ���Վ��_

		// ���Վ��_���̑���
		// ���ւ̈ړ��͂��̂܂܉\
		if ((GetKeyboardPress(DIK_A)) || (IsButtonPressed(0, BUTTON_LEFT)))
		{	// ���ֈړ�
			g_Player.spd = g_Speed;
			g_Player.dir = XM_PI / 2;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}
		if ((GetKeyboardPress(DIK_D)) || (IsButtonPressed(0, BUTTON_RIGHT)))
		{	// �E�ֈړ�
			g_Player.spd = g_Speed;
			g_Player.dir = -XM_PI / 2;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}

		// ���ւ̈ړ������̂܂܉\
		if ((GetKeyboardPress(DIK_S)) || (IsButtonPressed(0, BUTTON_DOWN)))
		{	// ���ֈړ�
			g_Player.spd = g_Speed;
			g_Player.dir = 0.0f;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}

		// �΂߈ړ�
		if ((GetKeyboardPress(DIK_A)) && (GetKeyboardPress(DIK_W)) ||
			(IsButtonPressed(0, BUTTON_LEFT)) && (IsButtonPressed(0, BUTTON_UP)))
		{	// ����ֈړ�
			g_Player.spd = g_Speed;
			g_Player.dir = -XM_PI * 3 / 4;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}
		if ((GetKeyboardPress(DIK_D)) && (GetKeyboardPress(DIK_W)) ||
			(IsButtonPressed(0, BUTTON_RIGHT)) && (IsButtonPressed(0, BUTTON_UP)))
		{	// �E��ֈړ�
			g_Player.spd = g_Speed;
			g_Player.dir = XM_PI * 3 / 4;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}
		if ((GetKeyboardPress(DIK_A)) && (GetKeyboardPress(DIK_S)) ||
			(IsButtonPressed(0, BUTTON_LEFT)) && (IsButtonPressed(0, BUTTON_DOWN)))
		{	// �����ֈړ�
			g_Player.spd = g_Speed;
			g_Player.dir = -XM_PI / 4;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}
		if ((GetKeyboardPress(DIK_D)) && (GetKeyboardPress(DIK_S)) ||
			(IsButtonPressed(0, BUTTON_RIGHT)) && (IsButtonPressed(0, BUTTON_DOWN)))
		{	// �E���ֈړ�
			g_Player.spd = g_Speed;
			g_Player.dir = XM_PI / 4;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}


		// z�������̒���
		if ((g_Player.spd == g_Speed) && (g_Player.dir == XM_PI / 2)) g_Player.dir = -XM_PI / 2;
		else if ((g_Player.spd == g_Speed) && (g_Player.dir == -XM_PI / 2)) g_Player.dir = XM_PI / 2;
		
		// �ړ���������
		if (g_Player.spd > 0.0f)
		{

			g_Player.rot.y = -g_Player.dir;

			// ���͂̂����������փv���C���[���������Ĉړ�������
			g_Player.pos.x -= sinf(-g_Player.dir) * g_Player.spd;
			g_Player.pos.z -= cosf(g_Player.dir) * g_Player.spd;
		}
		
	}
	else
	{	// TPS���_

		// TPS���_���̑���
		// ��������
		if ((GetKeyboardPress(DIK_A)) || (IsButtonPressed(0, BUTTON_LEFT)))
		{	// ���֌���
			cam->rot.y -= VALUE_ROTATE_CAMERA;
			if (cam->rot.y > XM_PI)
			{
				cam->rot.y -= XM_PI * 2.0f;
			}

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}

		if ((GetKeyboardPress(DIK_D)) || (IsButtonPressed(0, BUTTON_RIGHT)))
		{	// �E�֌���
			cam->rot.y += VALUE_ROTATE_CAMERA;
			if (cam->rot.y > XM_PI)
			{
				cam->rot.y -= XM_PI * 2.0f;
			}

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}

		// �U�������������{
		if ((GetKeyboardPress(DIK_S)) || (IsButtonPressed(0, BUTTON_DOWN)))
		{
			g_Player.spd = g_Speed;
			//g_Player.dir = 0.0f;

			if((GetKeyboardTrigger(DIK_S)) || (IsButtonPressed(0, BUTTON_DOWN)))
			{
				// �U������܂ł̕��
				SetTurnAround();
			}

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// ����SE���Đ�
		}



		// �ړ���������
		{
			g_Player.rot.y = g_Player.dir + cam->rot.y;

			// ���͂̂����������փv���C���[���������Ĉړ�������
			g_Player.pos.x -= sinf(g_Player.rot.y) * g_Player.spd;
			g_Player.pos.z -= cosf(g_Player.rot.y) * g_Player.spd;
		}
	}


	// �����Ă���Ƃ��͕��s�e�[�u�����Z�b�g
	if (g_gait)
	{
		g_Parts[0].tbl_adr = move_tbl_cloak;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[1].tbl_adr = move_tbl_femur_right;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[2].tbl_adr = move_tbl_femur_left;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[3].tbl_adr = move_tbl_knee_right;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[4].tbl_adr = move_tbl_knee_left;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g

		g_Parts[0].tbl_size = sizeof(move_tbl_cloak) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[1].tbl_size = sizeof(move_tbl_femur_right) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[2].tbl_size = sizeof(move_tbl_femur_left) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[3].tbl_size = sizeof(move_tbl_knee_right) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[4].tbl_size = sizeof(move_tbl_knee_left) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	}
	else
	{
		g_Parts[0].tbl_adr = move_tbl_cloak_stop;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[1].tbl_adr = move_tbl_femur_right_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[2].tbl_adr = move_tbl_femur_left_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[3].tbl_adr = move_tbl_knee_right_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[4].tbl_adr = move_tbl_knee_left_stop;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g

		g_Parts[0].tbl_size = sizeof(move_tbl_cloak_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[1].tbl_size = sizeof(move_tbl_femur_right_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[2].tbl_size = sizeof(move_tbl_femur_left_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[3].tbl_size = sizeof(move_tbl_knee_right_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
		g_Parts[4].tbl_size = sizeof(move_tbl_knee_left_stop) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

		// �j���[�g�����|�W�V�����ɂ������߂�����
		// �}���g�̏���
		XMVECTOR p1 = XMLoadFloat3(&g_Parts[0].tbl_adr[0].rot);	// ���̏ꏊ
		XMVECTOR p0 = XMLoadFloat3(&g_Parts[0].rot);			// ���݂̏ꏊ
		XMVECTOR vec = p1 - p0;
		XMStoreFloat3(&g_Parts[0].rot, p0 + vec / 10);

		// �E��ڂ̏���
		p1 = XMLoadFloat3(&g_Parts[1].tbl_adr[0].rot);	// ���̏ꏊ
		p0 = XMLoadFloat3(&g_Parts[1].rot);				// ���݂̏ꏊ
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[1].rot, p0 + vec / 10);

		// ����ڂ̏���
		p1 = XMLoadFloat3(&g_Parts[2].tbl_adr[0].rot);	// ���̏ꏊ
		p0 = XMLoadFloat3(&g_Parts[2].rot);				// ���݂̏ꏊ
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[2].rot, p0 + vec / 10);

		// �E�G�̏���
		p1 = XMLoadFloat3(&g_Parts[3].tbl_adr[0].rot);	// ���̏ꏊ
		p0 = XMLoadFloat3(&g_Parts[3].rot);				// ���݂̏ꏊ
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[3].rot, p0 + vec / 10);

		// ���G�̏���
		p1 = XMLoadFloat3(&g_Parts[4].tbl_adr[0].rot);	// ���̏ꏊ
		p0 = XMLoadFloat3(&g_Parts[4].rot);				// ���݂̏ꏊ
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[4].rot, p0 + vec / 10);
	}


	// �}�b�v�O���𔻒肵�āA�O�̏ꍇ�͖߂�
	{
		// x���W
		if (g_Player.pos.x > MAP_LIMIT_X_MAX)
		{
			g_Player.pos.x = MAP_LIMIT_X_MAX;
		}
		if (g_Player.pos.x < MAP_LIMIT_X_MIN)
		{
			g_Player.pos.x = MAP_LIMIT_X_MIN;
		}

		// z���W
		// ���͉~�`�̓����蔻��ɂȂ��Ă���̂ł��̏�����ǉ�
		float radian = 0;
		if (g_Player.pos.x > MAP_CIRCLE_LEFT - MAP_OFFSTT_X)
		{
			radian = (g_Player.pos.x - MAP_CIRCLE_LEFT + MAP_OFFSTT_X) / (MAP_CIRCLE_R * 2) * XM_PI;
		}

		if ((g_Player.pos.x > MAP_CIRCLE_LEFT + MAP_OFFSTT_X) && (g_Player.pos.z > (sinf(radian) * MAP_CIRCLE_R)))
		{
			g_Player.pos.z = sinf(radian) * MAP_CIRCLE_R;
		}
		else if ((g_Player.pos.x <= MAP_CIRCLE_LEFT + MAP_OFFSTT_X) && (g_Player.pos.z > MAP_LIMIT_Z_MAX))
		{
			g_Player.pos.z = MAP_LIMIT_Z_MAX;
		}
		if ((g_Player.pos.x > MAP_CIRCLE_LEFT + MAP_OFFSTT_X) && (g_Player.pos.z < (sinf(-radian) * MAP_CIRCLE_R)))
		{
			g_Player.pos.z = sinf(-radian) * MAP_CIRCLE_R;
		}
		else if ((g_Player.pos.x <= MAP_CIRCLE_LEFT + MAP_OFFSTT_X) && (g_Player.pos.z < MAP_LIMIT_Z_MIN))
		{
			g_Player.pos.z = MAP_LIMIT_Z_MIN;
		}

		// �`���[�g���A�����͓�����͈͂������Ȃ��Ă���
		if (GetTutorialClear(tx_clear) == FALSE)
		{
			if (g_Player.pos.x > TUTORIAL_X)
			{
				g_Player.pos.x = TUTORIAL_X;
			}
		}
	}


	// ���C�L���X�g���đ����̍��������߂�
	XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };				// �Ԃ������|���S���̖@���x�N�g���i�����j
	XMFLOAT3 hitPosition;								// ��_
	hitPosition.y = g_Player.pos.y - PLAYER_OFFSET_Y;	// �O�ꂽ���p�ɏ��������Ă���
	bool ans = RayHitField(g_Player.pos, &hitPosition, &normal);
	g_Player.pos.y = hitPosition.y + PLAYER_OFFSET_Y;
	g_Player.pos.y = PLAYER_OFFSET_Y;

	g_Player.spd *= 0.3f;


	// ���s�̊K�w�A�j���[�V����(��ȊO)
	// �K�w�A�j���[�V����
	for (int i = 0; i < PLAYER_PARTS_MAX - 1; i++)
	{
		RunAnimation(i, g_gait);
	}

	// ��̊K�w�A�j���[�V����
	RunAnimation(7, g_magicChant);

	// �O�̃t���[���ŕ����Ă��Č��ݕ����Ă��Ȃ��ꍇ�Ɏ��Ԃ����Z�b�g
	if ((g_gait_old == TRUE) && (g_gait == FALSE))	g_time = 0.0f;

	// ���݂̃t���[���������Ă��邩���L�^���āA���̃t���[���Ŏg�p����
	g_gait_old = g_gait;


	// �}���g�̃��[�t�B���O�̏���
	{
		int after, brfore;

		// ���[�t�B���O���f���̔ԍ�����
		if (g_gait == FALSE)
		{	// ��{�̃��f���փ��[�t�B���O������
			after = 0;
			brfore = 1;
		}
		else
		{
			after = (mof + 1) % (MAX_PLAYER_MOVE -1) + 1;
			brfore = mof % (MAX_PLAYER_MOVE - 1) + 1;
		}

		// ���[�t�B���O����
		for (int p = 0; p < g_Player_Vertex[0].VertexNum; p++)
		{
			g_Vertex[p].Position.x = g_Player_Vertex[after].VertexArray[p].Position.x - g_Player_Vertex[brfore].VertexArray[p].Position.x;
			g_Vertex[p].Position.y = g_Player_Vertex[after].VertexArray[p].Position.y - g_Player_Vertex[brfore].VertexArray[p].Position.y;
			g_Vertex[p].Position.z = g_Player_Vertex[after].VertexArray[p].Position.z - g_Player_Vertex[brfore].VertexArray[p].Position.z;

			g_Vertex[p].Position.x *= g_time;
			g_Vertex[p].Position.y *= g_time;
			g_Vertex[p].Position.z *= g_time;

			g_Vertex[p].Position.x += g_Player_Vertex[brfore].VertexArray[p].Position.x;
			g_Vertex[p].Position.y += g_Player_Vertex[brfore].VertexArray[p].Position.y;
			g_Vertex[p].Position.z += g_Player_Vertex[brfore].VertexArray[p].Position.z;
		}

		// ���Ԃ�i�߂�
		if (g_gait == FALSE)
		{	// 1��̃��[�t�B���O�����s��
			if (g_time < 1.0f) g_time += 0.05f;
		}
		else
		{	// �������[�V����
			g_time += 0.08f;
			if (g_time > 1.0f)
			{
				mof++;
				g_time = 0.0f;
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
		memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Player_Vertex[0].VertexNum);

		GetDeviceContext()->Unmap(g_Parts[0].model.VertexBuffer, 0);
	}


	// ��̃��[�t�B���O�̏���
	{
		int after, brfore;
		float time;

		// ���[�t�B���O���f���̔ԍ�����
		{
			after = g_MagicType;
			brfore = null;
			time = g_Wnadtime;
		}

		// ���[�t�B���O����
		for (int p = 0; p < g_Wand_Vertex[0].VertexNum; p++)
		{
			g_wandVertex[p].Position.x = g_Wand_Vertex[after].VertexArray[p].Position.x - g_Wand_Vertex[brfore].VertexArray[p].Position.x;
			g_wandVertex[p].Position.y = g_Wand_Vertex[after].VertexArray[p].Position.y - g_Wand_Vertex[brfore].VertexArray[p].Position.y;
			g_wandVertex[p].Position.z = g_Wand_Vertex[after].VertexArray[p].Position.z - g_Wand_Vertex[brfore].VertexArray[p].Position.z;

			g_wandVertex[p].Position.x *= g_Wnadtime;
			g_wandVertex[p].Position.y *= g_Wnadtime;
			g_wandVertex[p].Position.z *= g_Wnadtime;

			g_wandVertex[p].Position.x += g_Wand_Vertex[brfore].VertexArray[p].Position.x;
			g_wandVertex[p].Position.y += g_Wand_Vertex[brfore].VertexArray[p].Position.y;
			g_wandVertex[p].Position.z += g_Wand_Vertex[brfore].VertexArray[p].Position.z;


			// �@��
			g_wandVertex[p].Normal.x = g_Wand_Vertex[after].VertexArray[p].Normal.x - g_Wand_Vertex[brfore].VertexArray[p].Normal.x;
			g_wandVertex[p].Normal.y = g_Wand_Vertex[after].VertexArray[p].Normal.y - g_Wand_Vertex[brfore].VertexArray[p].Normal.y;
			g_wandVertex[p].Normal.z = g_Wand_Vertex[after].VertexArray[p].Normal.z - g_Wand_Vertex[brfore].VertexArray[p].Normal.z;

			g_wandVertex[p].Normal.x *= g_Wnadtime;
			g_wandVertex[p].Normal.y *= g_Wnadtime;
			g_wandVertex[p].Normal.z *= g_Wnadtime;

			g_wandVertex[p].Normal.x += g_Wand_Vertex[brfore].VertexArray[p].Normal.x;
			g_wandVertex[p].Normal.y += g_Wand_Vertex[brfore].VertexArray[p].Normal.y;
			g_wandVertex[p].Normal.z += g_Wand_Vertex[brfore].VertexArray[p].Normal.z;

			// �@��
			g_wandVertex[p].Normal.x = g_Wand_Vertex[after].SubsetArray->Material.Material.Diffuse.x - g_Wand_Vertex[brfore].VertexArray[p].Normal.x;
			g_wandVertex[p].Normal.y = g_Wand_Vertex[after].VertexArray[p].Normal.y - g_Wand_Vertex[brfore].VertexArray[p].Normal.y;
			g_wandVertex[p].Normal.z = g_Wand_Vertex[after].VertexArray[p].Normal.z - g_Wand_Vertex[brfore].VertexArray[p].Normal.z;

			g_wandVertex[p].Normal.x *= g_Wnadtime;
			g_wandVertex[p].Normal.y *= g_Wnadtime;
			g_wandVertex[p].Normal.z *= g_Wnadtime;

			g_wandVertex[p].Normal.x += g_Wand_Vertex[brfore].VertexArray[p].Normal.x;
			g_wandVertex[p].Normal.y += g_Wand_Vertex[brfore].VertexArray[p].Normal.y;
			g_wandVertex[p].Normal.z += g_Wand_Vertex[brfore].VertexArray[p].Normal.z;
		}

		// �F�̃��[�t�B���O����
		for (int p = 0; p < g_wandSubset.SubsetNum; p++)
		{
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.x = g_Wand_Vertex[after].SubsetArray[p].Material.Material.Diffuse.x - g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.x;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.y = g_Wand_Vertex[after].SubsetArray[p].Material.Material.Diffuse.y - g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.y;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.z = g_Wand_Vertex[after].SubsetArray[p].Material.Material.Diffuse.z - g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.z;

			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.x *= g_Wnadtime;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.y *= g_Wnadtime;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.z *= g_Wnadtime;

			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.x += g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.x;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.y += g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.y;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.z += g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.z;
		}


		SetModelColor(&g_Parts[7].model, &g_wandSubset);

		// ���Ԃ�i�߂�
		if ((g_magicChant == FALSE) && (g_magic == FALSE))
		{	// 1��̃��[�t�B���O�����s��
			if (g_Wnadtime > 0.0f) g_Wnadtime -= 0.01f;
			if (g_Wnadtime <= 0.0f) g_MagicType = null;
		}
		else
		{	// �������[�V����
			if (g_Wnadtime < 1.0f)
			{
				g_Wnadtime += 0.015f;
			}
		}

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_wandVertex;

		// ���_�o�b�t�@�ɒl���Z�b�g����
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_Parts[7].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

		// �S���_���𖈉�㏑�����Ă���̂�DX11�ł͂��̕�����������ł�
		memcpy(pVtx, g_wandVertex, sizeof(VERTEX_3D)*g_Wand_Vertex[0].VertexNum);

		GetDeviceContext()->Unmap(g_Parts[7].model.VertexBuffer, 0);
	}


	//////////////////////////////////////////////////////////////////////
	// �p������
	//////////////////////////////////////////////////////////////////////

	XMVECTOR vx, nvx, up;
	XMVECTOR quat;
	float len, angle;

	// �Q�̃x�N�g���̊O�ς�����ĔC�ӂ̉�]�������߂�
	g_Player.upVector = normal;
	up = { 0.0f, 1.0f, 0.0f, 0.0f };
	vx = XMVector3Cross(up, XMLoadFloat3(&g_Player.upVector));

	// ���߂���]������N�H�[�^�j�I�������o��
	nvx = XMVector3Length(vx);
	XMStoreFloat(&len, nvx);
	nvx = XMVector3Normalize(vx);
	angle = asinf(len);
	quat = XMQuaternionRotationNormal(nvx, angle);

	// �O��̃N�H�[�^�j�I�����獡��̃N�H�[�^�j�I���܂ł̉�]�����炩�ɂ���
	quat = XMQuaternionSlerp(XMLoadFloat4(&g_Player.quaternion), quat, 0.05f);

	// ����̃N�H�[�^�j�I���̌��ʂ�ۑ�����
	XMStoreFloat4(&g_Player.quaternion, quat);


	// ��̃f�B�]���u����
	g_Wand += WAND_DISSOLVE;
	if (g_Wand > 1.0f) g_Wand -= 1.0f;



#ifdef _DEBUG	// �f�o�b�O����\������
	PrintDebugProc("Player:�� �� �� ���@Space\n");
	PrintDebugProc("Player:X:%f Y:%f Z:%f \n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);

	if ((GetKeyboardPress(DIK_1)))
	{
		cam->pos.y += 2.0f;
	}
	if ((GetKeyboardPress(DIK_2)))
	{
		cam->pos.y -= 2.0f;
	}

#endif
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawPlayer(void)
{
	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);
	
	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);

	// ���f���`��
	DrawModel(&g_Player.model);


	// �p�[�c�̊K�w�A�j���[�V����
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// ��Ƀf�B�]���u��t����
		if ((i == 7) && (GetShaderMode() == SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ))
		{
			SetShaderMode(SHADER_MODE_WAND);
			SetShaderResouces(SHADER_RESOURCE_MODE_NOISE);
			g_WandDissolve = TRUE;
		}

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

		// ��̃f�B�]���u����
		SetDissolve(g_Wand);

		// ���f���`��
		DrawModel(&g_Parts[i].model);

		// �V�F�[�_�[�����ɖ߂�
		if (g_WandDissolve)
		{
			SetShaderType(SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ);
		}
	}

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
//=============================================================================
PLAYER *GetPlayer(void)
{
	return &g_Player;
}


//=============================================================================
// �����Ă��邩���Ǘ�����֐�
//=============================================================================
void SetGait(BOOL data)
{
	g_gait = data;
}


//=============================================================================
//�@�K�w�A�j���[�V����
//=============================================================================
void RunAnimation(int i,BOOL data)
{
	// �g���Ă���Ȃ珈������
	if ((g_Parts[i].use == TRUE) && (g_Parts[i].tbl_adr != NULL) && (data))
	{
		// �ړ�����
		int		index = (int)g_Parts[i].move_time;
		float	time = g_Parts[i].move_time - index;
		int		size = g_Parts[i].tbl_size;

		float dt = 1.0f / g_Parts[i].tbl_adr[index].frame;	// 1�t���[���Ői�߂鎞��
		g_Parts[i].move_time += dt;					// �A�j���[�V�����̍��v���Ԃɑ���

		if (index > (size - 2))	// �S�[�����I�[�o�[���Ă�����A�ŏ��֖߂�
		{
			g_Parts[i].move_time = 0.0f;
			index = 0;
		}

		// ���W�����߂�	X = StartX + (EndX - StartX) * ���̎���
		XMVECTOR p1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].pos);	// ���̏ꏊ
		XMVECTOR p0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].pos);	// ���݂̏ꏊ
		XMVECTOR vec = p1 - p0;
		XMStoreFloat3(&g_Parts[i].pos, p0 + vec * time);

		// ��]�����߂�	R = StartX + (EndX - StartX) * ���̎���
		XMVECTOR r1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].rot);	// ���̊p�x
		XMVECTOR r0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].rot);	// ���݂̊p�x
		XMVECTOR rot = r1 - r0;
		XMStoreFloat3(&g_Parts[i].rot, r0 + rot * time);

		// scale�����߂� S = StartX + (EndX - StartX) * ���̎���
		XMVECTOR s1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].scl);	// ����Scale
		XMVECTOR s0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].scl);	// ���݂�Scale
		XMVECTOR scl = s1 - s0;
		XMStoreFloat3(&g_Parts[i].scl, s0 + scl * time);

	}
}


//=============================================================================
// HP��Ԃ�
//=============================================================================
int GetHP(void)
{
	return g_Player.HP;
}