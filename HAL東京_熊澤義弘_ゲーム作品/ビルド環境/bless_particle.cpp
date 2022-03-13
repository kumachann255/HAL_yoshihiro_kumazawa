//=============================================================================
//
// �u���X�̃p�[�e�B�N������ [bless_particle.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "bless_particle.h"
#include "bahamut_bless.h"
#include "player.h"
#include "holy.h"
#include "bahamut.h"
#include "lightning.h"
#include "debugproc.h"
#include "fade_white.h"
#include "sound.h"
#include "beam_particle.h"
#include "beam_particle_2.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX			(1)			// �e�N�X�`���̐�

#define	BLESS_PARTICLE_SIZE_X		(5.0f)		// ���_�T�C�Y
#define	BLESS_PARTICLE_SIZE_Y		(5.0f)		// ���_�T�C�Y
#define	VALUE_MOVE_BLESS_PARTICLE	(0.2f)		// �ړ����x

#define	MAX_BLESS_PARTICLE		(512 * 4)		// �p�[�e�B�N���ő吔

#define BLESS_PARTICLE_BASE			(30)		// �o�n���[�g���S����ǂ̂��炢�����Ĕ��������邩
#define BLESS_PARTICLE_OFFSET		(60)		// ��L�̗���

#define BLESS_SPEED					(10)		// �u���X�p�[�e�B�N���̍Œᑬ�x(�ڕW�n�_�܂ł̓��B�t���[����)
#define BLESS_1F_MAX				(40)		// 1�t���[���ŏo���p�[�e�B�N���̐�

#define BLESS_TIME_MAX				(280)		// �p�[�e�B�N���𔭐������Ă��鎞��
#define BLESS_TIME_START			(60)		// �`���[�W�p�[�e�B�N�𔭐������n�߂鎞��

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
	int				nLife;			// ����
	BOOL			bUse;			// �g�p���Ă��邩�ǂ���

} PARTICLE;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexBlessParticle(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// ���_�o�b�t�@

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static int							g_TexNo;					// �e�N�X�`���ԍ�

static PARTICLE					g_aBlessParticle[MAX_BLESS_PARTICLE];		// �p�[�e�B�N�����[�N
static XMFLOAT3					g_posBase;						// �r���{�[�h�����ʒu
static float					g_fWidthBase = 1.0f;			// ��̕�
static float					g_fHeightBase = 1.0f;			// ��̍���
static float					g_roty = 0.0f;					// �ړ�����
static float					g_spd = 0.0f;					// �ړ��X�s�[�h
static int						g_time;							// �p�[�e�B�N���𔭐������Ă��鎞�Ԃ��Ǘ�

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/effect000.jpg",
};

static BOOL						g_Load = FALSE;
static BOOL						g_On = FALSE;				// �p�[�e�B�N���𔭐����Ă��邩
static BOOL						g_End = TRUE;				// �p�[�e�B�N���𔭐���������


//=============================================================================
// ����������
//=============================================================================
HRESULT InitBlessParticle(void)
{
	// ���_���̍쐬
	MakeVertexBlessParticle();

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
	for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	{
		g_aBlessParticle[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBlessParticle[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBlessParticle[nCntParticle].scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
		g_aBlessParticle[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_aBlessParticle[nCntParticle].material, sizeof(g_aBlessParticle[nCntParticle].material));
		g_aBlessParticle[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aBlessParticle[nCntParticle].fSizeX = BLESS_PARTICLE_SIZE_X;
		g_aBlessParticle[nCntParticle].fSizeY = BLESS_PARTICLE_SIZE_Y;
		g_aBlessParticle[nCntParticle].nLife = 0;
		g_aBlessParticle[nCntParticle].bUse = FALSE;
	}

	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;
	g_time = 0;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitBlessParticle(void)
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
void UpdateBlessParticle(void)
{
	BAHAMUT *bahamut = GetBahamut();

	for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	{
		if (g_aBlessParticle[nCntParticle].bUse)
		{// �g�p��
			g_aBlessParticle[nCntParticle].pos.x += g_aBlessParticle[nCntParticle].move.x;
			g_aBlessParticle[nCntParticle].pos.z += g_aBlessParticle[nCntParticle].move.z;
			g_aBlessParticle[nCntParticle].pos.y += g_aBlessParticle[nCntParticle].move.y;

			g_aBlessParticle[nCntParticle].nLife--;
			if ((g_aBlessParticle[nCntParticle].nLife <= 0))
			{
				g_aBlessParticle[nCntParticle].bUse = FALSE;
			}
		}
	}


	// �p�[�e�B�N������
	if((g_On) && (!g_End))
	{
		for (int i = 0; i < BLESS_1F_MAX; i++)
		{
			XMFLOAT3 pos, pos_old;

			// �u���X�̐ݒ�
			pos = bahamut->pos;
			pos.y += 125.0f;
			pos.x += 85.0f;

			pos_old = pos;

			XMFLOAT3 move;
			XMFLOAT3 color;
			float fAngle, fLength;
			int nLife;
			float fSize;
			int speed;


			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			fLength = (float)(rand() % BLESS_PARTICLE_OFFSET + BLESS_PARTICLE_BASE);
			pos.x += sinf(fAngle) * fLength;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			pos.y += cosf(fAngle + (XM_PI / 2)) * fLength;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			pos.z += cosf(fAngle) * fLength;

			speed = rand() % BLESS_SPEED + BLESS_SPEED;
			nLife = speed;
			move.x = (pos_old.x - pos.x) / (float)speed;
			move.y = (pos_old.y - pos.y) / (float)speed;
			move.z = (pos_old.z - pos.z) / (float)speed;

			color.x = RamdomFloat(3, 1.0f, 0.2f);
			color.y = RamdomFloat(3, 1.0f, 0.2f);
			color.z = RamdomFloat(3, 1.0f, 0.9f);

			fSize = RamdomFloat(4, 0.3f, 0.05f);

			// �r���{�[�h�̐ݒ�
			SetBlessParticle(pos, move, XMFLOAT4(color.x, color.y, color.z, 0.85f), fSize, fSize, speed);
		}

		// �ŏ���1�񂾂�SE���Đ�
		if (g_time == 0)
		{
			// SE���Đ�
			PlaySound(SOUND_LABEL_SE_BAHAMUT_charge);
		}


		// ���Ԃ�i�߂�
		g_time++;

		// ��莞�Ԍo�߂Ń`���[�W�p�[�e�B�N���𔭐�
		if (g_time > BLESS_TIME_START)
		{
			SetBeamParticleSwitch(TRUE);
			//SetBeamParticleSwitch2(TRUE);
		}

		// ��莞�ԂɂȂ�����~�߂�
		if (g_time > BLESS_TIME_MAX)
		{
			g_End = TRUE;

			SetChargeRadius(0.2f);


			// SE���t�F�[�h�A�E�g
			StopSound(SOUND_LABEL_SE_BAHAMUT_charge);
		}

		// �t�F�[�h�J�n
//		SetFadeWhite(WHITE_OUT_charge);
	}


#ifdef _DEBUG	// �f�o�b�O����\������
	if ((GetKeyboardPress(DIK_N)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_On = FALSE;
	}


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawBlessParticle(void)
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

	for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	{
		if (g_aBlessParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aBlessParticle[nCntParticle].scale.x, g_aBlessParticle[nCntParticle].scale.y, g_aBlessParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aBlessParticle[nCntParticle].pos.x, g_aBlessParticle[nCntParticle].pos.y, g_aBlessParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			// �}�e���A���ݒ�
			SetMaterial(g_aBlessParticle[nCntParticle].material);

			// �|���S���̕`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	//for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	//{
	//	if (g_aBlessParticle[nCntParticle].bUse)
	//	{
	//		// ���[���h�}�g���b�N�X�̏�����
	//		mtxWorld = XMMatrixIdentity();

	//		// �r���[�}�g���b�N�X���擾
	//		mtxView = XMLoadFloat4x4(&cam->mtxView);

	//		// �]�u�s�񏈗�
	//		mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
	//		mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
	//		mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

	//		mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
	//		mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
	//		mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

	//		mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
	//		mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
	//		mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

	//		// �X�P�[���𔽉f
	//		mtxScl = XMMatrixScaling(g_aBlessParticle[nCntParticle].scale.x, g_aBlessParticle[nCntParticle].scale.y, g_aBlessParticle[nCntParticle].scale.z);
	//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	//		// �ړ��𔽉f
	//		mtxTranslate = XMMatrixTranslation(g_aBlessParticle[nCntParticle].pos.x, g_aBlessParticle[nCntParticle].pos.y, g_aBlessParticle[nCntParticle].pos.z);
	//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	//		// ���[���h�}�g���b�N�X�̐ݒ�
	//		SetWorldMatrix(&mtxWorld);

	//		// �}�e���A���ݒ�
	//		SetMaterial(g_aBlessParticle[nCntParticle].material);

	//		// �|���S���̕`��
	//		GetDeviceContext()->Draw(4, 0);
	//	}
	//}

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
HRESULT MakeVertexBlessParticle(void)
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
		vertex[0].Position = XMFLOAT3(-BLESS_PARTICLE_SIZE_X / 2,  BLESS_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3( BLESS_PARTICLE_SIZE_X / 2,  BLESS_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-BLESS_PARTICLE_SIZE_X / 2, -BLESS_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3( BLESS_PARTICLE_SIZE_X / 2, -BLESS_PARTICLE_SIZE_Y / 2, 0.0f);

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
void SetColorBlessParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aBlessParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// �p�[�e�B�N���̔�������
//=============================================================================
int SetBlessParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int life)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	{
		if (!g_aBlessParticle[nCntParticle].bUse)
		{
			g_aBlessParticle[nCntParticle].pos = pos;
			g_aBlessParticle[nCntParticle].rot = { 0.0f, 0.0f, 0.0f };
			g_aBlessParticle[nCntParticle].scale = { fSizeX, fSizeY, fSizeY };
			g_aBlessParticle[nCntParticle].move = move;
			g_aBlessParticle[nCntParticle].material.Diffuse = col;
			g_aBlessParticle[nCntParticle].fSizeX = fSizeX;
			g_aBlessParticle[nCntParticle].fSizeY = fSizeY;
			g_aBlessParticle[nCntParticle].nLife = life;
			g_aBlessParticle[nCntParticle].bUse = TRUE;

			nIdxParticle = nCntParticle;

			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// �p�[�e�B�N���̔����X�C�b�`
//=============================================================================
void SetBlessParticleSwitch(BOOL data)
{
	if (g_On == FALSE)
	{
		g_On = data;
		g_End = FALSE;
		g_time = 0;
	}
	else
	{
		g_On = data;
	}
}