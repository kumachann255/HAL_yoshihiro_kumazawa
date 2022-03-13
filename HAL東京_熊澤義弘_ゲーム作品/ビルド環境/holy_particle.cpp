//=============================================================================
//
// �z�[���[�̃p�[�e�B�N������ [holy_particle.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "holy_particle.h"
#include "player.h"
#include "holy.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX			(1)			// �e�N�X�`���̐�

#define	HOLY_PARTICLE_SIZE_X		(5.0f)		// ���_�T�C�Y
#define	HOLY_PARTICLE_SIZE_Y		(5.0f)		// ���_�T�C�Y
#define	VALUE_MOVE_HOLY_PARTICLE	(0.2f)		// �ړ����x

#define	MAX_HOLY_PARTICLE		(512)		// �p�[�e�B�N���ő吔

#define HOLY_PARTICLE_OFFSET_X		(40)		// ���̒��S����ǂ͈̔͂̒��Ŕ��������邩
#define HOLY_PARTICLE_OFFSET_Z		(40)		// ���̒��S����ǂ͈̔͂̒��Ŕ��������邩

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
	XMFLOAT3		move;			// �ړ���
	MATERIAL		material;		// �}�e���A��
	float			fSizeX;			// ��
	float			fSizeY;			// ����
	int				nIdxShadow;		// �eID
	int				nLife;			// ����
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���

} PARTICLE;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexHolyParticle(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// ���_�o�b�t�@

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static int							g_TexNo;					// �e�N�X�`���ԍ�

static PARTICLE					g_aHolyParticle[MAX_HOLY_PARTICLE];		// �p�[�e�B�N�����[�N
static XMFLOAT3					g_posBase;						// �r���{�[�h�����ʒu
static float					g_fWidthBase = 1.0f;			// ��̕�
static float					g_fHeightBase = 1.0f;			// ��̍���
static float					g_roty = 0.0f;					// �ړ�����
static float					g_spd = 0.0f;					// �ړ��X�s�[�h

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/effect000.jpg",
};

static BOOL						g_Load = FALSE;
static BOOL						g_HolyOn = FALSE;		// ������p�[�e�B�N���𔭐������邩
static int						g_HolyNum = 0;			// �ǂ̌�����p�[�e�B�N���𔭐������邩

//=============================================================================
// ����������
//=============================================================================
HRESULT InitHolyParticle(void)
{
	// ���_���̍쐬
	MakeVertexHolyParticle();

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
	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_PARTICLE; nCntParticle++)
	{
		g_aHolyParticle[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aHolyParticle[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aHolyParticle[nCntParticle].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_aHolyParticle[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_aHolyParticle[nCntParticle].material, sizeof(g_aHolyParticle[nCntParticle].material));
		g_aHolyParticle[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aHolyParticle[nCntParticle].fSizeX = HOLY_PARTICLE_SIZE_X;
		g_aHolyParticle[nCntParticle].fSizeY = HOLY_PARTICLE_SIZE_Y;
		g_aHolyParticle[nCntParticle].nIdxShadow = -1;
		g_aHolyParticle[nCntParticle].nLife = 0;
		g_aHolyParticle[nCntParticle].bUse = FALSE;
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
void UninitHolyParticle(void)
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
void UpdateHolyParticle(void)
{
	{
		for (int nCntParticle = 0; nCntParticle < MAX_HOLY_PARTICLE; nCntParticle++)
		{
			if (g_aHolyParticle[nCntParticle].bUse)
			{// �g�p��
				g_aHolyParticle[nCntParticle].pos.x += g_aHolyParticle[nCntParticle].move.x;
				g_aHolyParticle[nCntParticle].pos.z += g_aHolyParticle[nCntParticle].move.z;

				g_aHolyParticle[nCntParticle].pos.y += g_aHolyParticle[nCntParticle].move.y;
				if (g_aHolyParticle[nCntParticle].pos.y <= g_aHolyParticle[nCntParticle].fSizeY / 2)
				{// ���n����
					g_aHolyParticle[nCntParticle].pos.y = g_aHolyParticle[nCntParticle].fSizeY / 2;
					g_aHolyParticle[nCntParticle].move.y = -g_aHolyParticle[nCntParticle].move.y * 0.75f;
				}

				g_aHolyParticle[nCntParticle].move.x += (0.0f - g_aHolyParticle[nCntParticle].move.x) * 0.015f;
				g_aHolyParticle[nCntParticle].move.z += (0.0f - g_aHolyParticle[nCntParticle].move.z) * 0.015f;


				g_aHolyParticle[nCntParticle].nLife--;
				if (g_aHolyParticle[nCntParticle].nLife <= 0)
				{
					g_aHolyParticle[nCntParticle].bUse = FALSE;
					g_aHolyParticle[nCntParticle].nIdxShadow = -1;
				}
				else
				{
					if (g_aHolyParticle[nCntParticle].nLife <= 20)
					{
						g_aHolyParticle[nCntParticle].material.Diffuse.x = 0.5f - (float)(80 - g_aHolyParticle[nCntParticle].nLife) / 80 * 0.5f;
						g_aHolyParticle[nCntParticle].material.Diffuse.y = 0.5f - (float)(80 - g_aHolyParticle[nCntParticle].nLife) / 80 * 0.5f;
						g_aHolyParticle[nCntParticle].material.Diffuse.z = 0.5f - (float)(80 - g_aHolyParticle[nCntParticle].nLife) / 80 * 0.5f;
					}

					if (g_aHolyParticle[nCntParticle].nLife <= 10)
					{
						// ���l�ݒ�
						g_aHolyParticle[nCntParticle].material.Diffuse.w -= 0.05f;
						if (g_aHolyParticle[nCntParticle].material.Diffuse.w < 0.0f)
						{
							g_aHolyParticle[nCntParticle].material.Diffuse.w = 0.0f;
						}
					}
				}
			}
		}

		// �p�[�e�B�N������
		if(g_HolyOn)
		{
			XMFLOAT3 pos;
			HOLY *holy = GetHoly();

			pos = holy[g_HolyNum % MAX_HOLY].pos;

			pos.x += rand() % HOLY_PARTICLE_OFFSET_X - (HOLY_PARTICLE_OFFSET_X / 2);
			pos.z += rand() % HOLY_PARTICLE_OFFSET_Z - (HOLY_PARTICLE_OFFSET_Z / 2);

			g_HolyNum++;

			XMFLOAT3 move;
			float fAngle, fLength;
			int nLife;
			float fSize;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			fLength = rand() % (int)(g_fWidthBase * 10) / 100.0f - g_fWidthBase;
			move.x = sinf(fAngle) * fLength;
			move.y = rand() % 50 / 100.0f + g_fHeightBase;
			move.z = cosf(fAngle) * fLength;

			nLife = rand() % 40 + 20;

			fSize = (float)(rand() % 30 + 20);

			// �r���{�[�h�̐ݒ�
			SetHolyParticle(pos, move, XMFLOAT4(0.5f, 0.5f, 0.5f, 0.85f), fSize, fSize, nLife);
		}
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawHolyParticle(void)
{
	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// ���C�e�B���O�𖳌���
	SetLightEnable(FALSE);

	// ���Z�����ɐݒ�
	SetBlendState(BLEND_MODE_ADD);

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

	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_PARTICLE; nCntParticle++)
	{
		if (g_aHolyParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aHolyParticle[nCntParticle].scale.x, g_aHolyParticle[nCntParticle].scale.y, g_aHolyParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aHolyParticle[nCntParticle].pos.x, g_aHolyParticle[nCntParticle].pos.y, g_aHolyParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �}�e���A���ݒ�
			SetMaterial(g_aHolyParticle[nCntParticle].material);

			// �|���S���̕`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_PARTICLE; nCntParticle++)
	{
		if (g_aHolyParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aHolyParticle[nCntParticle].scale.x, g_aHolyParticle[nCntParticle].scale.y, g_aHolyParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aHolyParticle[nCntParticle].pos.x, g_aHolyParticle[nCntParticle].pos.y, g_aHolyParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �}�e���A���ݒ�
			SetMaterial(g_aHolyParticle[nCntParticle].material);

			// �|���S���̕`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// �ʏ�u�����h�ɖ߂�
	SetBlendState(BLEND_MODE_ALPHABLEND);

	// Z��r�L��
	SetDepthEnable(TRUE);

	// �t�H�O�L��
	SetFogEnable(GetFogSwitch());

}

//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexHolyParticle(void)
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
void SetColorHolyParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aHolyParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// �p�[�e�B�N���̔�������
//=============================================================================
int SetHolyParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_PARTICLE; nCntParticle++)
	{
		if (!g_aHolyParticle[nCntParticle].bUse)
		{
			g_aHolyParticle[nCntParticle].pos = pos;
			g_aHolyParticle[nCntParticle].rot = { 0.0f, 0.0f, 0.0f };
			g_aHolyParticle[nCntParticle].scale = { 1.0f, 1.0f, 1.0f };
			g_aHolyParticle[nCntParticle].move = move;
			g_aHolyParticle[nCntParticle].material.Diffuse = col;
			g_aHolyParticle[nCntParticle].fSizeX = fSizeX;
			g_aHolyParticle[nCntParticle].fSizeY = fSizeY;
			g_aHolyParticle[nCntParticle].nLife = nLife;
			g_aHolyParticle[nCntParticle].bUse = TRUE;

			nIdxParticle = nCntParticle;

			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// �p�[�e�B�N���̔����X�C�b�`
//=============================================================================
void SetParticleSwitch(BOOL data)
{
	g_HolyOn = data;
}