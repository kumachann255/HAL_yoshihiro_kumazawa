//=============================================================================
//
// ���@�w���� [magic_circle.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "debugproc.h"
#include "camera.h"
#include "model.h"
#include "magic_circle.h"
#include "player.h"
#include "holy.h"
#include "lightning.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX				(4)			// �e�N�X�`���̐�

#define	CIRCLE_SIZE_X			(5.0f)		// ���_�T�C�Y
#define	CIRCLE_SIZE_Y			(5.0f)		// ���_�T�C�Y
#define	VALUE_MOVE_CIRCLE		(0.2f)		// �ړ����x

#define CIRCLE_OFFSET_X			(40)		// ���̒��S����ǂ͈̔͂̒��Ŕ��������邩
#define CIRCLE_OFFSET_Z			(40)		// ���̒��S����ǂ͈̔͂̒��Ŕ��������邩

#define CIRCLE_DISTANCE			(40)		// �ǂ̂��炢�v���C���[����b�����Ƃ��납�牊�𔭐������邩

#define CIRCLE_DISSOLVE			(0.01f)		// �f�B�]���u�̑��x

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexCircleParticle(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// ���_�o�b�t�@

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static int							g_TexNo;					// �e�N�X�`���ԍ�

static CIRCLE					g_Circle;						// ���@�w�̍\����

static float					g_fWidthBase = 1.0f;			// ��̕�
static float					g_fHeightBase = 1.0f;			// ��̍���
static float					g_roty = 0.0f;					// �ړ�����
static float					g_spd = 0.0f;					// �ړ��X�s�[�h

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/circle0.png",
	"data/TEXTURE/circle1.png",
	"data/TEXTURE/circle2.png",
	"data/TEXTURE/circle3.png",
};

static BOOL						g_Load = FALSE;
static BOOL						g_CircleOn = FALSE;		// ������p�[�e�B�N���𔭐������邩
static int						g_CircleNum = 0;			// �ǂ̌�����p�[�e�B�N���𔭐������邩

//=============================================================================
// ����������
//=============================================================================
HRESULT InitCircle(void)
{
	// ���_���̍쐬
	MakeVertexCircleParticle();

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
	for (int i = 0; i < MAX_CIRCLE; i++)
	{
		g_Circle.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Circle.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Circle.scale = XMFLOAT3(12.0f, 12.0f, 1.0f);

		ZeroMemory(&g_Circle.material, sizeof(g_Circle.material));
		g_Circle.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_Circle.bUse = FALSE;
		g_Circle.dissolve = 0.0f;
	}

	g_roty = 0.0f;
	g_spd = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitCircle(void)
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
void UpdateCircle(void)
{
	PLAYER *player = GetPlayer();

	for (int i = 0; i < MAX_CIRCLE; i++)
	{
		if (g_Circle.bUse)
		{// �g�p��
			g_Circle.pos = player->pos;

			// �ʒu����
			g_Circle.pos.x -= sinf(player->rot.y) * CIRCLE_DISTANCE;
			g_Circle.pos.z -= cosf(player->rot.y) * CIRCLE_DISTANCE;
			g_Circle.pos.y += 6.0f;


			// �v���C���[�̐��ʂɌ����𒲐�
			g_Circle.rot.y = player->rot.y + XM_PI;

			// �f�B�]���u�Ō���n�߂�
			if ((g_Circle.dissolve < 1.0f) && (!g_Circle.end))
			{
				g_Circle.dissolve += CIRCLE_DISSOLVE;
			}
		}

	
		if (g_Circle.end)
		{	// �����n�߂鏈�����n�܂��Ă���
			if (g_Circle.dissolve > 0.0f) g_Circle.dissolve -= CIRCLE_DISSOLVE;

			// �f�B�]���u���I��������
			if (g_Circle.dissolve <= -CIRCLE_DISSOLVE)
			{
				ResetCircle();
			}
		}
	}

	PrintDebugProc("%f\n", g_Circle.dissolve);
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawCircle(void)
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


	for (int i = 0; i < MAX_CIRCLE; i++)
	{
		if (g_Circle.bUse)
		{
			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �r���[�}�g���b�N�X���擾
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_Circle.scale.x, g_Circle.scale.y, g_Circle.scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// ��]�𔽉f
			mtxRot = XMMatrixRotationRollPitchYaw(g_Circle.rot.x, g_Circle.rot.y, g_Circle.rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_Circle.pos.x, g_Circle.pos.y, g_Circle.pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �f�B�]���u�̐ݒ�
			SetDissolve(g_Circle.dissolve);

			// �}�e���A���ݒ�
			SetMaterial(g_Circle.material);

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
HRESULT MakeVertexCircleParticle(void)
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
		vertex[0].Position = XMFLOAT3(-CIRCLE_SIZE_X / 2, CIRCLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(CIRCLE_SIZE_X / 2, CIRCLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-CIRCLE_SIZE_X / 2, -CIRCLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(CIRCLE_SIZE_X / 2, -CIRCLE_SIZE_Y / 2, 0.0f);

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
void SetColorCircle(XMFLOAT4 col)
{
	g_Circle.material.Diffuse = col;
}

//=============================================================================
// ���@�w�̔�������
//=============================================================================
void SetCircle(BOOL use, BOOL end, int magic)
{
	g_Circle.bUse = use;
	g_Circle.end = end;
	if(magic != null) g_TexNo = magic - 1;

	// ���@�̎�ނɂ���ĐF��ς���
	switch (magic)
	{
	case fire:
		g_Circle.material.Diffuse = { 0.8f, 0.3f, 0.2f, 1.0f };
		break;

	case lightning:
		g_Circle.material.Diffuse = { 0.8f, 0.8f, 0.0f, 1.0f };
		break;

	case water:
		g_Circle.material.Diffuse = { 0.25f, 0.6f, 1.0f, 1.0f };
		break;

	case holy:
		g_Circle.material.Diffuse = { 2.0f, 2.0f, 2.0f, 1.0f };
		break;
	}
}


//=============================================================================
// �t�@�C�A�̏I������
//=============================================================================
void ResetCircle(void)
{
	g_Circle.pos = { 0.0f, 0.0f, 0.0f };
	g_Circle.rot = { 0.0f, 0.0f, 0.0f };
	g_Circle.scale = { 1.0f, 1.0f, 1.0f };
	g_Circle.material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f};
	g_Circle.bUse = FALSE;
	g_Circle.dissolve = 0.0f;
}


//=============================================================================
// �t�@�C�A�̏����擾
//=============================================================================
CIRCLE *GetCircle(void)
{
	return &g_Circle;
}