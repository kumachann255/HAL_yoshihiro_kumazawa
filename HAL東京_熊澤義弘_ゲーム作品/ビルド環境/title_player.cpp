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
#include "title_player.h"
#include "light.h"
#include "meshfield.h"
#include "lightning.h"
#include "fire.h"
#include "holy.h"
#include "water.h"
#include "magic_circle.h"
#include "sound.h"

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
#define	MODEL_PARTS_WAND	"data/MODEL/wand.obj"					// �ǂݍ��ރ��f����

#define MAX_PLAYER_HP		(1000)				// �v���C���[��HP
#define MAX_INVINCIBLE		(120)				// �_���[�W��H������ۂ̖��G����

#define	VALUE_MOVE			(2.0f)				// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)		// ��]��

#define PLAYER_SHADOW_SIZE	(1.0f)				// �e�̑傫��
#define PLAYER_OFFSET_Y		(20.0f)				// �v���C���[�̑��������킹��

#define PLAYER_PARTS_MAX	(8)					// �v���C���[�̃p�[�c�̐�

#define MAX_PLAYER_MOVE		(5)					// ���[�t�B���O�̃��f����

#define WAND_DISSOLVE		(0.005f)			// ��̃f�B�]���u���x

#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// �r���[���ʂ̎���p
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// �r���[���ʂ̃A�X�y�N�g��	
#define	VIEW_NEAR_Z		(10.0f)											// �r���[���ʂ�NearZ�l
#define	VIEW_FAR_Z		(10000.0f)										// �r���[���ʂ�FarZ�l


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void RunTitleAnimation(void);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static PLAYER		g_Player;						// �v���C���[

static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// �v���C���[�̃p�[�c�p

static BOOL			g_Load = FALSE;

static MODEL		g_Player_Vertex[MAX_PLAYER_MOVE];	// ���[�t�B���O�p���f���̒��_��񂪓������f�[�^�z��
static VERTEX_3D	*g_Vertex = NULL;				// �r���o�߂��L�^����ꏊ

static float		g_time;
static int			mof;
static BOOL			g_gait = TRUE;					// �������[�V�����Ɉڍs���邩�ǂ���

static float		g_Wand = 0.0f;					// ��̃f�B�]���u�Ǘ�
static BOOL			g_WandDissolve = FALSE;			// ��̃f�B�]���u�`�惂�[�h�ɂȂ��Ă��邩

static CAMERA		g_Camera;						// �^�C�g���p�̃J����



// �v���C���[�̊K�w�A�j���[�V�����f�[�^
static INTERPOLATION_DATA move_tbl_cloak[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },

};

static INTERPOLATION_DATA move_tbl_cloak_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

static INTERPOLATION_DATA move_tbl_femur_right_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���㔼
};

static INTERPOLATION_DATA move_tbl_femur_left_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// ���r�I���㔼
};

static INTERPOLATION_DATA move_tbl_knee_left_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// �V�r�I���㔼
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


//=============================================================================
// ����������
//=============================================================================
HRESULT InitTitlePlayer(void)
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
	g_Parts[7].tbl_adr = move_tbl_wand;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	g_Parts[7].tbl_size = sizeof(move_tbl_wand) / sizeof(INTERPOLATION_DATA);		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g
	g_Parts[7].load = 1;
	LoadModel(MODEL_PARTS_WAND, &g_Parts[7].model, 0);


	// �e�q�֌W
	g_Parts[0].parent = &g_Player;		// �}���g�̐e�͖{��
	g_Parts[1].parent = &g_Player;		// �E��ڂ̐e�͖{��
	g_Parts[2].parent = &g_Player;		// ����ڂ̐e�͖{��
	g_Parts[3].parent = &g_Parts[1];	// �E�G�̐e�͉E���
	g_Parts[4].parent = &g_Parts[2];	// ���G�̐e�͍����
	g_Parts[5].parent = &g_Parts[3];	// �E���ڂ̐e�͉E�G
	g_Parts[6].parent = &g_Parts[4];	// �����ڂ̐e�͍��G
	g_Parts[7].parent = &g_Player;		// ��̐e�͖{��

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

	g_time = 0.0f;
	mof = 0;

	// �J�����̏�����
	g_Camera.pos = { 10.0f, 40.0f, 80.0f };
	g_Camera.up = { 0.0f, 1.0f, 0.0f };
	g_Camera.at = { 25.0f, 30.0f, 0.0f };


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTitlePlayer(void)
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
void UpdateTitlePlayer(void)
{
	// ���[�t�B���O�̏���
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
		g_time += 0.03f;
		if (g_time > 1.0f)
		{
			mof++;
			g_time = 0.0f;
		}

		// ���_�����㏑��
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * g_Player_Vertex[0].VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_Vertex;

		UnloadModelVertex(&g_Parts[0].model);
		GetDevice()->CreateBuffer(&bd, &sd, &g_Parts[0].model.VertexBuffer);


		//// ���_�o�b�t�@�ɒl���Z�b�g����
		//D3D11_MAPPED_SUBRESOURCE msr;
		//GetDeviceContext()->Map(g_Parts[0].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		//VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

		//// �S���_���𖈉�㏑�����Ă���̂�DX11�ł͂��̕�����������ł�
		//memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D));

		//GetDeviceContext()->Unmap(g_Parts[0].model.VertexBuffer, 0);

	}


	// ��̃f�B�]���u����
	g_Wand += WAND_DISSOLVE;
	if (g_Wand > 1.0f) g_Wand -= 1.0f;

	RunTitleAnimation();


	// �J�����̐ݒ���ꏏ�ɍs���Ă��܂�
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





#ifdef _DEBUG	// �f�o�b�O����\������
	PrintDebugProc("Player:�� �� �� ���@Space\n");
	PrintDebugProc("Player:X:%f Y:%f Z:%f\n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
#endif
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTitlePlayer(void)
{
	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���C�e�B���O�𖳌���
	//SetLightEnable(FALSE);

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
		if (i == 7)
		{
			SetShaderMode(SHADER_MODE_WAND);
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

		// �f�B�]���u�̐ݒ�
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
PLAYER *GetTitlePlayer(void)
{
	return &g_Player;
}


//=============================================================================
// �����Ă��邩���Ǘ�����֐�
//=============================================================================
void SetTitleGait(BOOL data)
{
	g_gait = data;
}


//=============================================================================
//�@�K�w�A�j���[�V����
//=============================================================================
void RunTitleAnimation(void)
{
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
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