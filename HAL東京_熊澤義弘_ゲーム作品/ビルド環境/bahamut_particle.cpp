//=============================================================================
//
// �o�n���[�g�̃p�[�e�B�N������ [Bahamut_particle.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "bahamut_particle.h"
#include "player.h"
#include "holy.h"
#include "bahamut.h"
#include "lightning.h"
#include "debugproc.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX			(1)			// �e�N�X�`���̐�

#define	BAHAMUT_PARTICLE_SIZE_X		(5.0f)		// ���_�T�C�Y
#define	BAHAMUT_PARTICLE_SIZE_Y		(5.0f)		// ���_�T�C�Y
#define	VALUE_MOVE_BAHAMUT_PARTICLE	(0.2f)		// �ړ����x

#define	MAX_BAHAMUT_PARTICLE		(512 * 15)		// �p�[�e�B�N���ő吔

#define BAHAMUT_PARTICLE_OFFSET		(100)		// �o�n���[�g���S����ǂ̂��炢�����Ĕ��������邩

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
HRESULT MakeVertexBahamutParticle(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// ���_�o�b�t�@

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static int							g_TexNo;					// �e�N�X�`���ԍ�

static PARTICLE					g_aBahamutParticle[MAX_BAHAMUT_PARTICLE];		// �p�[�e�B�N�����[�N
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
static BOOL						g_On = FALSE;				// �p�[�e�B�N���𔭐����Ă��邩
static BOOL						g_End = TRUE;				// �p�[�e�B�N���𔭐���������
static int						g_BahamutNum = 0;			// �ǂ̌�����p�[�e�B�N���𔭐������邩

//=============================================================================
// ����������
//=============================================================================
HRESULT InitBahamutParticle(void)
{
	// ���_���̍쐬
	MakeVertexBahamutParticle();

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
	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		g_aBahamutParticle[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBahamutParticle[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBahamutParticle[nCntParticle].scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
		g_aBahamutParticle[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_aBahamutParticle[nCntParticle].material, sizeof(g_aBahamutParticle[nCntParticle].material));
		g_aBahamutParticle[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aBahamutParticle[nCntParticle].fSizeX = BAHAMUT_PARTICLE_SIZE_X;
		g_aBahamutParticle[nCntParticle].fSizeY = BAHAMUT_PARTICLE_SIZE_Y;
		g_aBahamutParticle[nCntParticle].nIdxShadow = -1;
		g_aBahamutParticle[nCntParticle].nLife = 0;
		g_aBahamutParticle[nCntParticle].bUse = FALSE;
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
void UninitBahamutParticle(void)
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
void UpdateBahamutParticle(void)
{
	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		if (g_aBahamutParticle[nCntParticle].bUse)
		{// �g�p��
			g_aBahamutParticle[nCntParticle].pos.x += g_aBahamutParticle[nCntParticle].move.x;
			g_aBahamutParticle[nCntParticle].pos.z += g_aBahamutParticle[nCntParticle].move.z;

			g_aBahamutParticle[nCntParticle].pos.y += g_aBahamutParticle[nCntParticle].move.y;

			g_aBahamutParticle[nCntParticle].move.x += (0.0f - g_aBahamutParticle[nCntParticle].move.x) * 0.015f;
			g_aBahamutParticle[nCntParticle].move.z += (0.0f - g_aBahamutParticle[nCntParticle].move.z) * 0.015f;


			g_aBahamutParticle[nCntParticle].nLife--;
			if ((g_aBahamutParticle[nCntParticle].nLife <= 0))
			{
				g_aBahamutParticle[nCntParticle].bUse = FALSE;
				g_aBahamutParticle[nCntParticle].nIdxShadow = -1;
			}
			else
			{
				if (g_aBahamutParticle[nCntParticle].nLife <= 20)
				{
					g_aBahamutParticle[nCntParticle].material.Diffuse.x = 0.5f - (float)(80 - g_aBahamutParticle[nCntParticle].nLife) / 80 * 0.5f;
					g_aBahamutParticle[nCntParticle].material.Diffuse.y = 0.5f - (float)(80 - g_aBahamutParticle[nCntParticle].nLife) / 80 * 0.5f;
					g_aBahamutParticle[nCntParticle].material.Diffuse.z = 0.5f - (float)(80 - g_aBahamutParticle[nCntParticle].nLife) / 80 * 0.5f;
				}

				if (g_aBahamutParticle[nCntParticle].nLife <= 10)
				{
					// ���l�ݒ�
					g_aBahamutParticle[nCntParticle].material.Diffuse.w -= 0.05f;
					if (g_aBahamutParticle[nCntParticle].material.Diffuse.w < 0.0f)
					{
						g_aBahamutParticle[nCntParticle].material.Diffuse.w = 0.0f;
					}
				}
			}
		}
	}

	// �����J�����^�C�~���O�Ńp�[�e�B�N���𔭐�������
	if (GetWingOpen())
	{
		if (g_On == FALSE)
		{
			g_On = TRUE;
			g_End = FALSE;
		}
	}


	// �p�[�e�B�N������
	if((g_On) && (!g_End))
	{
		for (int i = 0; i < MAX_BAHAMUT_PARTICLE; i++)
		{
			XMFLOAT3 pos;
			BAHAMUT *bahamut = GetBahamut();

			pos = bahamut->pos;
			pos.y += 100.0f;


			g_BahamutNum++;

			XMFLOAT3 move;
			float fAngle, fLength;
			int nLife;
			float fSize;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			fLength = RamdomFloat(2, 2.0f, 0.2f);
			pos.x += sinf(fAngle) * BAHAMUT_PARTICLE_OFFSET;
			move.x = -sinf(fAngle) * fLength * 20.0f;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			//fLength = rand() % (int)(g_fWidthBase * 10) / 100.0f - g_fWidthBase;
			pos.y += cosf(fAngle + (XM_PI / 2)) * BAHAMUT_PARTICLE_OFFSET;
			move.y = -cosf(fAngle) * fLength * 20.0f;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			//fLength = rand() % (int)(g_fWidthBase * 10) / 100.0f - g_fWidthBase;
			pos.z += cosf(fAngle) * BAHAMUT_PARTICLE_OFFSET;
			move.z = -cosf(fAngle + (XM_PI / 2)) * fLength * 20.0f;


			nLife = rand() % 40 + 120;

			fSize = RamdomFloat(2, 1.0f, 0.5f);

			// �r���{�[�h�̐ݒ�
			SetBahamutParticle(pos, move, XMFLOAT4(0.5f, 0.5f, 0.5f, 0.85f), fSize, fSize, nLife);
		}

		g_End = TRUE;
	}


#ifdef _DEBUG	// �f�o�b�O����\������
	PrintDebugProc("life:%d \n", g_aBahamutParticle[100].nLife);

#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawBahamutParticle(void)
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

	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		if (g_aBahamutParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aBahamutParticle[nCntParticle].scale.x, g_aBahamutParticle[nCntParticle].scale.y, g_aBahamutParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aBahamutParticle[nCntParticle].pos.x, g_aBahamutParticle[nCntParticle].pos.y, g_aBahamutParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �}�e���A���ݒ�
			SetMaterial(g_aBahamutParticle[nCntParticle].material);

			// �|���S���̕`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		if (g_aBahamutParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aBahamutParticle[nCntParticle].scale.x, g_aBahamutParticle[nCntParticle].scale.y, g_aBahamutParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aBahamutParticle[nCntParticle].pos.x, g_aBahamutParticle[nCntParticle].pos.y, g_aBahamutParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �}�e���A���ݒ�
			SetMaterial(g_aBahamutParticle[nCntParticle].material);

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
HRESULT MakeVertexBahamutParticle(void)
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
		vertex[0].Position = XMFLOAT3(-BAHAMUT_PARTICLE_SIZE_X / 2,  BAHAMUT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3( BAHAMUT_PARTICLE_SIZE_X / 2,  BAHAMUT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-BAHAMUT_PARTICLE_SIZE_X / 2, -BAHAMUT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3( BAHAMUT_PARTICLE_SIZE_X / 2, -BAHAMUT_PARTICLE_SIZE_Y / 2, 0.0f);

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
void SetColorBahamutParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aBahamutParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// �p�[�e�B�N���̔�������
//=============================================================================
int SetBahamutParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		if (!g_aBahamutParticle[nCntParticle].bUse)
		{
			g_aBahamutParticle[nCntParticle].pos = pos;
			g_aBahamutParticle[nCntParticle].rot = { fSizeX, fSizeX, fSizeX };
			g_aBahamutParticle[nCntParticle].scale = { 1.0f, 1.0f, 1.0f };
			g_aBahamutParticle[nCntParticle].move = move;
			g_aBahamutParticle[nCntParticle].material.Diffuse = col;
			g_aBahamutParticle[nCntParticle].fSizeX = fSizeX;
			g_aBahamutParticle[nCntParticle].fSizeY = fSizeY;
			g_aBahamutParticle[nCntParticle].nLife = nLife;
			g_aBahamutParticle[nCntParticle].bUse = TRUE;

			nIdxParticle = nCntParticle;

			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// �p�[�e�B�N���̔����X�C�b�`
//=============================================================================
void SetBahamutParticleSwitch(BOOL data)
{
	g_On = data;
}