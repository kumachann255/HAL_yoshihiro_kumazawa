//=============================================================================
//
// �z�[���[�̋O������ [holy_orbit.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "holy_orbit.h"
#include "player.h"
#include "holy.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX					(1)			// �e�N�X�`���̐�

#define	HOLY_PARTICLE_SIZE_X		(2.5f)		// ���_�T�C�Y
#define	HOLY_PARTICLE_SIZE_Y		(2.5f)		// ���_�T�C�Y
#define	VALUE_MOVE_HOLY_PARTICLE	(0.2f)		// �ړ����x

#define	MAX_HOLY_ORBIT				(1024)		// �p�[�e�B�N���ő吔

#define ORBIT_LIFE					(25)		// �O���̎���

#define HOLY_PARTICLE_OFFSET_X		(40)		// ���̒��S����ǂ͈̔͂̒��Ŕ��������邩
#define HOLY_PARTICLE_OFFSET_Z		(40)		// ���̒��S����ǂ͈̔͂̒��Ŕ��������邩

#define ORBIT_1F_NUM				(8)		// 1�t���[���ŉ��\�����邩

#define	DISP_SHADOW						// �e�̕\��
//#undef DISP_SHADOW

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
typedef struct
{
	XMFLOAT3		pos;			// �ʒu
	XMFLOAT3		rot;			// ��]
	XMFLOAT3		scale;			// �X�P�[��
	MATERIAL		material;		// �}�e���A��
	int				nLife;			// ����
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���

} PARTICLE;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexHolyOrbit(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// ���_�o�b�t�@

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static int							g_TexNo;					// �e�N�X�`���ԍ�

static PARTICLE					g_aHolyOrbit[MAX_HOLY_ORBIT];		// �p�[�e�B�N�����[�N
static XMFLOAT3					g_posBase;						// �r���{�[�h�����ʒu
static float					g_fWidthBase = 1.0f;			// ��̕�
static float					g_fHeightBase = 1.0f;			// ��̍���
static float					g_roty = 0.0f;					// �ړ�����
static float					g_spd = 0.0f;					// �ړ��X�s�[�h

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/holy_orbit.png",
};

static BOOL						g_Load = FALSE;
static BOOL						g_OrbitOn = FALSE;		// ������O���𔭐������邩
static int						g_HolyNum = 0;			// �ǂ̌�����p�[�e�B�N���𔭐������邩

//=============================================================================
// ����������
//=============================================================================
HRESULT InitHolyOrbit(void)
{
	// ���_���̍쐬
	MakeVertexHolyOrbit();

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
	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_ORBIT; nCntParticle++)
	{
		g_aHolyOrbit[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aHolyOrbit[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aHolyOrbit[nCntParticle].scale = XMFLOAT3(HOLY_PARTICLE_SIZE_X, HOLY_PARTICLE_SIZE_Y, 1.0f);

		ZeroMemory(&g_aHolyOrbit[nCntParticle].material, sizeof(g_aHolyOrbit[nCntParticle].material));
		g_aHolyOrbit[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aHolyOrbit[nCntParticle].nLife = 0;
		g_aHolyOrbit[nCntParticle].bUse = FALSE;
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
void UninitHolyOrbit(void)
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
void UpdateHolyOrbit(void)
{
	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_ORBIT; nCntParticle++)
	{
		if (g_aHolyOrbit[nCntParticle].bUse)
		{// �g�p��

			g_aHolyOrbit[nCntParticle].nLife--;
			if (g_aHolyOrbit[nCntParticle].nLife <= 0)
			{
				g_aHolyOrbit[nCntParticle].bUse = FALSE;
			}
			else
			{
				g_aHolyOrbit[nCntParticle].material.Diffuse.w -= 0.04f;
				g_aHolyOrbit[nCntParticle].scale.x -= 0.06f;
				g_aHolyOrbit[nCntParticle].scale.y -= 0.06f;
				if (g_aHolyOrbit[nCntParticle].material.Diffuse.w < 0.0f)
				{
					g_aHolyOrbit[nCntParticle].material.Diffuse.w = 0.0f;
				}
			}
		}
	}

	// �O������
	if(g_OrbitOn)
	{
		XMFLOAT3 pos,dis;
		HOLY *holy = GetHoly();

		for (int h = 0; h < MAX_HOLY; h++)
		{
			pos = holy[h].pos;			// ���݂̈ʒu���擾
			dis = GetHolyMoveValue(h);	// �ړ��ʂ��擾

			for (int p = 0; p < ORBIT_1F_NUM; p++)
			{
				// �����p�[�e�B�N�����Ŋ����āA�������ړ������Ȃ��甭��������
				pos.x -= dis.x / (float)ORBIT_1F_NUM;
				pos.y -= dis.y / (float)ORBIT_1F_NUM;
				pos.z -= dis.z / (float)ORBIT_1F_NUM;

				// �r���{�[�h�̐ݒ�
				SetHolyOrbit(pos);
			}
		}
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawHolyOrbit(void)
{
	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

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

	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_ORBIT; nCntParticle++)
	{
		if (g_aHolyOrbit[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aHolyOrbit[nCntParticle].scale.x, g_aHolyOrbit[nCntParticle].scale.y, g_aHolyOrbit[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aHolyOrbit[nCntParticle].pos.x, g_aHolyOrbit[nCntParticle].pos.y, g_aHolyOrbit[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �}�e���A���ݒ�
			SetMaterial(g_aHolyOrbit[nCntParticle].material);

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

}

//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexHolyOrbit(void)
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
		vertex[0].Position = XMFLOAT3(-HOLY_PARTICLE_SIZE_X / 2, HOLY_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(HOLY_PARTICLE_SIZE_X / 2, HOLY_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-HOLY_PARTICLE_SIZE_X / 2, -HOLY_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(HOLY_PARTICLE_SIZE_X / 2, -HOLY_PARTICLE_SIZE_Y / 2, 0.0f);

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
void SetColorHolyOrbit(int nIdxParticle, XMFLOAT4 col)
{
	g_aHolyOrbit[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// �p�[�e�B�N���̔�������
//=============================================================================
int SetHolyOrbit(XMFLOAT3 pos)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_ORBIT; nCntParticle++)
	{
		if (!g_aHolyOrbit[nCntParticle].bUse)
		{
			g_aHolyOrbit[nCntParticle].pos = pos;
			g_aHolyOrbit[nCntParticle].rot = { 0.0f, 0.0f, 0.0f };
			g_aHolyOrbit[nCntParticle].scale = XMFLOAT3(HOLY_PARTICLE_SIZE_X, HOLY_PARTICLE_SIZE_Y, 1.0f);
			g_aHolyOrbit[nCntParticle].material.Diffuse.w = 1.0f;
			g_aHolyOrbit[nCntParticle].nLife = ORBIT_LIFE;
			g_aHolyOrbit[nCntParticle].bUse = TRUE;

			nIdxParticle = nCntParticle;

			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// �O���̔����X�C�b�`
//=============================================================================
void SetOrbitSwitch(BOOL data)
{
	g_OrbitOn = data;
}

