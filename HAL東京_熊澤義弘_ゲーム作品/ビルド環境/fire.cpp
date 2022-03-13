//=============================================================================
//
// �t�@�C�A���� [fire.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "fire.h"
#include "player.h"
#include "holy.h"
#include "lightning.h"
#include "sound.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX				(1)			// �e�N�X�`���̐�

#define	FIRE_SIZE_X				(10.0f)		// ���_�T�C�Y
#define	FIRE_SIZE_Y				(10.0f)		// ���_�T�C�Y
#define	VALUE_MOVE_FIRE			(0.2f)		// �ړ����x

#define FIRE_OFFSET_X			(40)		// ���̒��S����ǂ͈̔͂̒��Ŕ��������邩
#define FIRE_OFFSET_Z			(40)		// ���̒��S����ǂ͈̔͂̒��Ŕ��������邩

#define FIRE_DISTANCE			(40)		// �ǂ̂��炢�v���C���[����b�����Ƃ��납�牊�𔭐������邩

#define FIRE_SPEED_BACE			(3.0f)		// ���̊�{�̑��x
#define FIRE_WIDTH_X			(0.8f)		// ���̐U�ꕝ
#define FIRE_WIDTH_Z			(0.8f)		// ���̐U�ꕝ
#define FIRE_WIDTH_Y			(0.4f)		// ���̐U�ꕝ

#define FIRE_START_SCALE_MAX	(1.0f)		// ���ˎ��̑傫���̍ő�l
#define FIRE_START_SCALE_MIN	(0.5f)		// ���ˎ��̑傫���̍ŏ��l

#define FIRE_SE_TIME			(40)		// SE���čĐ�����t���[����

#define	DISP_SHADOW							// �e�̕\��
//#undef DISP_SHADOW

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexFireParticle(void);
void SetFireParameter(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// ���_�o�b�t�@

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static int							g_TexNo;					// �e�N�X�`���ԍ�

static FIRE						g_Fire[MAX_FIRE];		// �p�[�e�B�N�����[�N
static XMFLOAT3					g_posBase;						// �r���{�[�h�����ʒu
static float					g_fWidthBase = 1.0f;			// ��̕�
static float					g_fHeightBase = 1.0f;			// ��̍���
static float					g_roty = 0.0f;					// �ړ�����
static float					g_spd = 0.0f;					// �ړ��X�s�[�h

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/fire1.png",
};

static BOOL						g_Load = FALSE;
static BOOL						g_FireOn = FALSE;		// ������p�[�e�B�N���𔭐������邩
static int						g_FireNum = 0;			// �ǂ̌�����p�[�e�B�N���𔭐������邩
static int						g_time = 0;				// �ǂ̂��炢�A�����Ĕ������Ă��邩
static int						g_SE = 0;				// SE���Ǘ�

//=============================================================================
// ����������
//=============================================================================
HRESULT InitFire(void)
{
	// ���_���̍쐬
	MakeVertexFireParticle();

	// �e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	g_TexNo = 0;

	// �p�[�e�B�N�����[�N�̏�����
	for (int nCntParticle = 0; nCntParticle < MAX_FIRE; nCntParticle++)
	{
		g_Fire[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Fire[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Fire[nCntParticle].scale = XMFLOAT3(0.3f, 0.3f, 0.3f);
		g_Fire[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_Fire[nCntParticle].material, sizeof(g_Fire[nCntParticle].material));
		g_Fire[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_Fire[nCntParticle].fSizeX = FIRE_SIZE_X;
		g_Fire[nCntParticle].fSizeY = FIRE_SIZE_Y;
		g_Fire[nCntParticle].nIdxShadow = -1;
		g_Fire[nCntParticle].nLife = 0;
		g_Fire[nCntParticle].bUse = FALSE;
		g_Fire[nCntParticle].Up = FALSE;
		g_Fire[nCntParticle].atk = FALSE;
	}

	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitFire(void)
{
	if (g_Load == FALSE) return;

	//�e�N�X�`���̉��
	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	// ���_�o�b�t�@�̉��
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateFire(void)
{
	PLAYER *player = GetPlayer();
	//g_posBase = pPlayer->pos;
	CAMERA *cam = GetCamera();

	for (int nCntParticle = 0; nCntParticle < MAX_FIRE; nCntParticle++)
	{
		if (g_Fire[nCntParticle].bUse)
		{// �g�p��
			g_Fire[nCntParticle].pos.x += g_Fire[nCntParticle].move.x;
			g_Fire[nCntParticle].pos.z += g_Fire[nCntParticle].move.z;

			g_Fire[nCntParticle].pos.y += g_Fire[nCntParticle].move.y;


			g_Fire[nCntParticle].move.x += (0.0f - g_Fire[nCntParticle].move.x) * 0.015f;
			g_Fire[nCntParticle].move.z += (0.0f - g_Fire[nCntParticle].move.z) * 0.015f;

			// ��ɉ�]������
			g_Fire[nCntParticle].rot.x -= sinf(cam->rot.y) * RamdomFloat(4, 0.2f, -0.2f);
			g_Fire[nCntParticle].rot.z -= -cosf(cam->rot.y) * RamdomFloat(4, 0.2f, -0.2f);
			//	�㏸���n�܂��Ă��邩�ǂ����ŏ�����ύX
			if (g_Fire[nCntParticle].Up)
			{
				// �㏸����
				g_Fire[nCntParticle].move.y += 0.12f;

				// �㏸�ɔ�������������
				g_Fire[nCntParticle].scale.x -= 0.15f;
				g_Fire[nCntParticle].scale.y -= 0.15f;
				g_Fire[nCntParticle].scale.z -= 0.15f;
			}
			else
			{	// ���X�ɑ傫�����āA�傫�������ɂȂ�����㏸����
				g_Fire[nCntParticle].scale.x += 0.015f;
				g_Fire[nCntParticle].scale.y += 0.015f;

				// �㏸�����J�n
				if (g_Fire[nCntParticle].scale.x > 2.0f) g_Fire[nCntParticle].Up = TRUE;
			}


			// �傫����0.0f�ȉ��ɂȂ�����I������
			if (g_Fire[nCntParticle].scale.x < 0.0f)
			{
				ResetFire(nCntParticle);
			}
		}
	}

	// �A���������Ԃ����ɂȂ�����SE���d�˂čĐ�
	if (g_time % FIRE_SE_TIME == 1)
	{
		g_SE++;

		switch (g_SE % 3)
		{
		case 0:
			PlaySound(SOUND_LABEL_SE_MAGIC_fire0);
			break;

		case 1:
			PlaySound(SOUND_LABEL_SE_MAGIC_fire1);
			break;

		case 2:
			PlaySound(SOUND_LABEL_SE_MAGIC_fire2);
			break;
		}
	}

	for (int p = 0; p < 5; p++)
	{
		SetFireParameter();
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawFire(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���C�e�B���O�𖳌���
	SetLightEnable(FALSE);

	// Z��r����
	SetDepthEnable(FALSE);

	// �t�H�O����
	SetFogEnable(FALSE);

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);


	for (int nCntParticle = 0; nCntParticle < MAX_FIRE; nCntParticle++)
	{
		if (g_Fire[nCntParticle].bUse)
		{
			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �r���[�}�g���b�N�X���擾
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			// �]�u�s�񏈗�
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_Fire[nCntParticle].scale.x, g_Fire[nCntParticle].scale.y, g_Fire[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// ��]�𔽉f
			mtxRot = XMMatrixRotationRollPitchYaw(g_Fire[nCntParticle].rot.x, g_Fire[nCntParticle].rot.y, g_Fire[nCntParticle].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_Fire[nCntParticle].pos.x, g_Fire[nCntParticle].pos.y, g_Fire[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �}�e���A���ݒ�
			SetMaterial(g_Fire[nCntParticle].material);

			// �|���S���̕`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// Z��r�L��
	SetDepthEnable(TRUE);

	// �t�H�O�L��
	SetFogEnable(GetFogSwitch());

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexFireParticle(void)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	{//���_�o�b�t�@�̒��g�𖄂߂�
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

		VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

		// ���_���W�̐ݒ�
		vertex[0].Position = XMFLOAT3(-FIRE_SIZE_X / 2, FIRE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(FIRE_SIZE_X / 2, FIRE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-FIRE_SIZE_X / 2, -FIRE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(FIRE_SIZE_X / 2, -FIRE_SIZE_Y / 2, 0.0f);

		// �@���̐ݒ�
		vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

		// ���ˌ��̐ݒ�
		vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		// �e�N�X�`�����W�̐ݒ�
		vertex[0].TexCoord = { 0.0f, 0.0f };
		vertex[1].TexCoord = { 1.0f, 0.0f };
		vertex[2].TexCoord = { 0.0f, 1.0f };
		vertex[3].TexCoord = { 1.0f, 1.0f };

		GetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}

	return S_OK;
}

//=============================================================================
// �}�e���A���J���[�̐ݒ�
//=============================================================================
void SetColorFire(int nIdxParticle, XMFLOAT4 col)
{
	g_Fire[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// �p�[�e�B�N���̔�������
//=============================================================================
int SetFireParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, int nLife, float scaleX, float scaleY)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_FIRE; nCntParticle++)
	{
		if (!g_Fire[nCntParticle].bUse)
		{
			g_Fire[nCntParticle].pos = pos;
			g_Fire[nCntParticle].rot = { 0.0f, 0.0f, 0.0f };
			g_Fire[nCntParticle].scale = { scaleX, scaleY, 1.0f };
			g_Fire[nCntParticle].move = move;
			g_Fire[nCntParticle].material.Diffuse = col;
			g_Fire[nCntParticle].nLife = nLife;
			g_Fire[nCntParticle].bUse = TRUE;
			g_Fire[nCntParticle].Up = FALSE;

			nIdxParticle = nCntParticle;
			break;
		}
	}

	return nIdxParticle;
}


// �p�[�e�B�N���̔����X�C�b�`
void SetFireSwitch(BOOL data)
{
	g_FireOn = data;
	if (data) g_time++;	// �g�p���Ă����玞�Ԃ�i�߂�
}


// �t�@�C�A�̏I������
void ResetFire(int i)
{
	g_Fire[i].pos = { 0.0f, 0.0f, 0.0f };
	g_Fire[i].rot = { 0.0f, 0.0f, 0.0f };
	g_Fire[i].scale = { 0.3f, 0.3f, 1.0f };
	g_Fire[i].move = { 0.0f, 0.0f, 0.0f };
	g_Fire[i].material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f};
	g_Fire[i].fSizeX = FIRE_SIZE_X;
	g_Fire[i].fSizeY = FIRE_SIZE_Y;
	g_Fire[i].nLife = 0;
	g_Fire[i].bUse = FALSE;
	g_Fire[i].Up = FALSE;
	g_Fire[i].atk = FALSE;
}


// �t�@�C�A�̃p�����[�^�[�ݒ�
void SetFireParameter(void)
{
	if (g_FireOn)
	{
		XMFLOAT3 pos;		// �����ʒu
		XMFLOAT3 dis;		// �v���C���[�̌����Ă�������x�N�g��
		XMFLOAT2 scale;		// ���ˎ��̑傫��

		PLAYER *player = GetPlayer();

		pos = player->pos;

		// �����ʒu�̐ݒ�
		pos.x -= sinf(player->rot.y) * FIRE_DISTANCE;
		pos.z -= cosf(player->rot.y) * FIRE_DISTANCE;
		pos.y += 6.0f;

		// �t�@�C�A���΂��������Z�o
		dis.x = -sinf(player->rot.y) * FIRE_SPEED_BACE;
		dis.z = -cosf(player->rot.y) * FIRE_SPEED_BACE;
		dis.y = 0.3f;

		// ��΂������Ƀ����_������^����
		dis.x += RamdomFloat(4, FIRE_WIDTH_X, -FIRE_WIDTH_X);
		dis.z += RamdomFloat(4, FIRE_WIDTH_Z, -FIRE_WIDTH_Z);
		dis.y += RamdomFloat(4, FIRE_WIDTH_Y, -FIRE_WIDTH_Y);

		// ���ˎ��̑傫���������_���Őݒ�
		scale.x = scale.y = RamdomFloat(4, FIRE_START_SCALE_MAX, FIRE_START_SCALE_MIN);

		int nLife = 0;

		// �r���{�[�h�̐ݒ�
		SetFireParticle(pos, dis, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),  nLife, scale.x, scale.y);
	}
}


// �t�@�C�A�̏����擾
FIRE *GetFire(void)
{
	return &g_Fire[0];
}


// �����@�̘A���������Ԃ����Z�b�g
void ResetFireTime(void)
{
	g_time = 0;
}