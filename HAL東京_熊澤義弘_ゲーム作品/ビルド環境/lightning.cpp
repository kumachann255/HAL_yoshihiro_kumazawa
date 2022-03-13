//=============================================================================
//
// ���C�g�j���O���� [lightning.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "debugproc.h"
#include "camera.h"
#include "model.h"
#include "lightning.h"
#include "player.h"
#include "sound.h"
#include "tutorial.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX				(1)			// �e�N�X�`���̐�

#define	LIGHTNING_SIZE_X		(2.0f)		// ���_�T�C�Y
#define	LIGHTNING_SIZE_Y		(2.0f)		// ���_�T�C�Y
#define	VALUE_MOVE_LIGHTNING	(3.0f)		// �ړ����x

#define	LIGHTNING_SET_Y			(150.0f)	// ���C�g�j���O�̊J�n���x
#define LIGHTNING_DISTANCE		(150.0f)	// ���C�g�j���O���ǂ̂��炢�����Ĕ��������邩

#define	LIGHTNING_SET_X			(90.0f)		// ���C�g�j���O�̓����蔻���X�����̕ӂ̒���
#define	LIGHTNING_SET_Z			(90.0f)		// ���C�g�j���O�̓����蔻���Z�����̕ӂ̒���

#define LIGHTNING_SPEED			(25)		// 1�t���[���Ői�ރX�s�[�h

#define BIFURCATION_COUNT		(25)		// ����܂ł̃J�E���g

#define LIGHTNING_LIFE			(30)		// ���C�g�j���O�̕\������
#define LIGHTNING_A_SUB			(0.1f)		// ���l�̌��Z���x

#define	DISP_SHADOW				// �e�̕\��
//#undef DISP_SHADOW


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexLightning(void);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// ���_�o�b�t�@

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static int							g_TexNo;					// �e�N�X�`���ԍ�

								// ���C�g�j���O���[�N x:����܂ł̃J�E���g y���򂵂��}
static LIGHTNING				g_Lightning[MAX_LIGHTNING][MAX_REFRACTION][BIFURCATION_COUNT];
static LIGHTNING_COLLI			g_LightningCollition[MAX_LIGHTNING];		// �����蔻��p�̓��ꕨ

static XMFLOAT3					g_posBase;						// �r���{�[�h�����ʒu
static float					g_fWidthBase = 5.0f;			// ��̕�
static float					g_fHeightBase = 10.0f;			// ��̍���
static float					g_roty = 0.0f;					// �ړ�����
static float					g_spd = 0.0f;					// �ړ��X�s�[�h

static XMFLOAT3					g_vector[MAX_LIGHTNING][MAX_REFRACTION];		// �ړ������̃x�N�g��
static XMFLOAT3					g_scale[MAX_LIGHTNING][MAX_REFRACTION];			// �X�P�[��
static int						g_vecType[MAX_LIGHTNING + 1];					// ���݂̃x�N�g���^�C�v�̍ő吔
static int						g_bifurcation[MAX_LIGHTNING];					// �����

static int						g_SE = 0;						// SE�̔ԍ����Ǘ�
static BOOL						g_LightningOnOff;				// �����@���g�p���Ă��邩�ǂ���

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/bloom.png",
};

static BOOL						g_Load = FALSE;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitLightning(void)
{
	// ���_���̍쐬
	MakeVertexLightning();

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

	// ���C�g�j���O���[�N�̏�����
	for (int z = 0; z < MAX_LIGHTNING; z++)
	{
		for (int y = 0; y < MAX_REFRACTION; y++)
		{
			for (int x = 0; x < BIFURCATION_COUNT; x++)
			{
				g_Lightning[z][y][x].pos = XMFLOAT3(0.0f, LIGHTNING_SET_Y, -10.0f);
				g_Lightning[z][y][x].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
				g_Lightning[z][y][x].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
				g_Lightning[z][y][x].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

				ZeroMemory(&g_Lightning[z][y][x].material, sizeof(g_Lightning[z][y][x].material));
				g_Lightning[z][y][x].material.Diffuse = XMFLOAT4(0.8f, 0.7f, 0.3f, 0.6f);

				g_Lightning[z][y][x].fSizeX = LIGHTNING_SIZE_X;
				g_Lightning[z][y][x].fSizeY = LIGHTNING_SIZE_Y;
				g_Lightning[z][y][x].nIdxShadow = -1;
				g_Lightning[z][y][x].nLife = LIGHTNING_LIFE;
				g_Lightning[z][y][x].bUse = FALSE;

				g_Lightning[z][y][x].vecType = 0;
				g_Lightning[z][y][x].count = 0;
			}

			g_scale[z][y] = { 1.0f, 1.0f,1.0f };
		}

		g_Lightning[z][0][0].bUse = FALSE;
		g_vecType[z] = 0;
		g_bifurcation[z] = 0;
		g_vector[z][0] = { 0.3f, -0.5f, 0.2f };
		g_scale[z][0] = { 1.0f, 1.0f,1.0f };

	}

	for (int i = 0; i < MAX_LIGHTNING; i++)
	{
		g_LightningCollition[i].pos = XMFLOAT3(0.0f, LIGHTNING_SET_Y, -10.0f);
		g_LightningCollition[i].bUse = FALSE;
		g_LightningCollition[i].fSizeY = LIGHTNING_SET_Y * 2;
		g_LightningCollition[i].fSizeX = LIGHTNING_SET_X;
		g_LightningCollition[i].fSizeZ = LIGHTNING_SET_Z;
	}

	// �ϐ��̏�����
	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;
	g_SE = 0;
	g_LightningOnOff = FALSE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitLightning(void)
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
void UpdateLightning(void)
{
	for (int z = 0; z < MAX_LIGHTNING; z++)
	{
		// �ŏ��̃��C�g�j���O��TRUE�Ȃ珈���J�n
		if (g_Lightning[z][0][0].bUse)
		{
			for (int i = 0; i < LIGHTNING_SPEED; i++)
			{
				for (int y = 0; y < g_vecType[z] + 1; y++)
				{
					SetUpdateLightning(z, y);
				}
			}
		}

		// �f�[�^�̍X�V
		g_LightningCollition[z].bUse = g_Lightning[z][0][0].bUse;
		g_LightningCollition[z].pos.x = g_Lightning[z][0][0].pos.x;
		g_LightningCollition[z].pos.z = g_Lightning[z][0][0].pos.z;
		g_LightningCollition[z].pos.y = 0.0f;

		// �\��������0�ɂȂ�����I�������J�n
		for (int y = 0; y < MAX_REFRACTION; y++)
		{
			for (int x = 0; x < BIFURCATION_COUNT; x++)
			{
				if (g_Lightning[z][y][x].bUse)
				{
					g_Lightning[z][y][x].nLife--;
				}
				if ((g_Lightning[z][y][x].bUse) && (g_Lightning[z][y][x].nLife <= 0))
				{
					g_Lightning[z][y][x].material.Diffuse.w -= LIGHTNING_A_SUB;

					// �����ɂȂ����疢�g�p��
					if (g_Lightning[z][y][x].material.Diffuse.w > 0.0f) continue;

					ResetLightning(z);

					// �`���[�g���A���N���A
					SetTutorialClear(tx_lightning);
				}
			}
		}


#ifdef _DEBUG	// �f�o�b�O����\������
		PrintDebugProc("g_vecType[%d]: %d\n", z, g_vecType[z]);
#endif

	}
	PrintDebugProc("g_vecType[%d]: %d\n", 4, g_vecType[4]);

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawLightning(void)
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

	for (int z = 0; z < MAX_LIGHTNING; z++)
	{
		for (int y = 0; y < MAX_REFRACTION; y++)
		{
			if (g_Lightning[z][y][0].bUse)
			{
				for (int x = 0; x < BIFURCATION_COUNT; x++)
				{
					if (g_Lightning[z][y][x].bUse)
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
						mtxScl = XMMatrixScaling(g_Lightning[z][y][x].scale.x, g_Lightning[z][y][x].scale.y, g_Lightning[z][y][x].scale.z);
						mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

						// �ړ��𔽉f
						mtxTranslate = XMMatrixTranslation(g_Lightning[z][y][x].pos.x, g_Lightning[z][y][x].pos.y, g_Lightning[z][y][x].pos.z);
						mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

						// ���[���h�}�g���b�N�X�̐ݒ�
						SetWorldMatrix(&mtxWorld);

						// �}�e���A���ݒ�
						SetMaterial(g_Lightning[z][y][x].material);

						// ���Z�����ɐݒ�
						//SetBlendState(BLEND_MODE_ADD);

						// �|���S���̕`��
						GetDeviceContext()->Draw(4, 0);
					}
				}
			}
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
HRESULT MakeVertexLightning(void)
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
		vertex[0].Position = XMFLOAT3(-LIGHTNING_SIZE_X / 2, LIGHTNING_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(LIGHTNING_SIZE_X / 2, LIGHTNING_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-LIGHTNING_SIZE_X / 2, -LIGHTNING_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(LIGHTNING_SIZE_X / 2, -LIGHTNING_SIZE_Y / 2, 0.0f);

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
// ���C�g�j���O�̏����擾
//=============================================================================
LIGHTNING_COLLI *GetLightningCollition(void)
{
	return &g_LightningCollition[0];
}

//=============================================================================
// ���C�g�j���O�̔�������
//=============================================================================
void SetLightning(void)
{
	PLAYER *player = GetPlayer();
	g_LightningOnOff = TRUE;

	// ���C�g�j���O�̂��ׂď�����
	for (int z = 0; z < MAX_LIGHTNING; z++)
	{
		if (g_Lightning[z][0][0].bUse == TRUE) continue;

		// �O�̂��߂�����x������
		ResetLightning(z);

		// �ŏ���1���Z�b�g
		g_Lightning[z][0][0].bUse = TRUE;

		// SE���Đ�
		g_SE++;

		switch (g_SE % 3)
		{
		case 0:
			PlaySound(SOUND_LABEL_SE_MAGIC_lightning01);
			break;

		case 1:
			PlaySound(SOUND_LABEL_SE_MAGIC_lightning02);
			break;

		case 2:
			PlaySound(SOUND_LABEL_SE_MAGIC_lightning03);
			break;
		}

		return;
	}
}

//=============================================================================
// ���C�g�j���O�̔�������
//=============================================================================
void SetUpdateLightning(int z, int vectype)
{
	vectype %= MAX_REFRACTION;

	for (int x = 0; x < BIFURCATION_COUNT; x++)
	{
		if (g_Lightning[z][vectype][x].bUse != TRUE)
		{
			XMFLOAT3 pos = g_Lightning[z][vectype][x - 1].pos;
			int vecType = g_Lightning[z][vectype][x - 1].vecType;
			int count = g_Lightning[z][vectype][x - 1].count;

			// �n�ʂ�˂��������Ȃ������m�F
			if (pos.y > 0.0f)
			{
				if (count < BIFURCATION_COUNT - 2)	// ���C�g�j���O��BIFURCATION_COUNT�A���ŕ`�悵����
				{
					// �i�s�����̂܂ܔ���
					count++;

					// �g�p���Ă���
					g_Lightning[z][vectype][x].bUse = TRUE;

					pos.x += g_vector[z][g_Lightning[z][vectype][0].vecType].x;
					pos.y += g_vector[z][g_Lightning[z][vectype][0].vecType].y;
					pos.z += g_vector[z][g_Lightning[z][vectype][0].vecType].z;

					// �V�������C�g�j���O�̃|�W�V�����Ƃ��Đݒ�
					g_Lightning[z][vectype][x].pos = pos;
					g_Lightning[z][vectype][x].vecType = vecType;
					g_Lightning[z][vectype][x].scale = g_Lightning[z][vectype][0].scale;

					// count���\���̂�count�֑��
					g_Lightning[z][vectype][x].count = count;
				}
				else
				{
					// ����񐔂��ő�܂ōs���Ă��Ȃ��ꍇ�A�����_���ŕ�����s��
					if (g_bifurcation[z] < MAX_BIFURCATION)
					{
						g_bifurcation[z]++;

						// ����(���ド���_��������)
						int random = (rand() % 4 + 1) / 2;
						if (random == 0) random = 1;
						for (int i = 0; i < random; i++)
						{
							// �x�N�g���^�C�v�𑝂₷
							if (g_vecType[z] < MAX_REFRACTION)
							{
								g_vecType[z]++;

								// �g�p���Ă���
								g_Lightning[z][vectype][x].bUse = TRUE;
								g_Lightning[z][g_vecType[z]][0].bUse = TRUE;

								// �x�N�g���̃��Z�b�g
								g_vector[z][g_vecType[z]].x = RamdomFloat(3, 0.7f, -0.7f);
								g_vector[z][g_vecType[z]].z = RamdomFloat(3, 0.7f, -0.7f);
								g_vector[z][g_vecType[z]].y = RamdomFloat(3, -0.1f, -0.8f);

								// ���̑����������_���Őݒ�
								g_scale[z][g_vecType[z]].x = g_scale[z][g_vecType[z]].y = RamdomFloat(3, 3.0f, 1.5f);

								pos.x += g_vector[z][g_vecType[z]].x;
								pos.y += g_vector[z][g_vecType[z]].y;
								pos.z += g_vector[z][g_vecType[z]].z;

								// �V�������C�g�j���O�̃|�W�V�����Ƃ��Đݒ�
								g_Lightning[z][g_vecType[z]][0].pos = pos;
								g_Lightning[z][g_vecType[z]][0].vecType = g_vecType[z];
								g_Lightning[z][g_vecType[z]][0].scale = g_scale[z][g_vecType[z]];
							}
						}
					}
					else
					{	// ������s��Ȃ�
						// �x�N�g���^�C�v�𑝂₷
						if (g_vecType[z] < MAX_REFRACTION)
						{
							g_vecType[z]++;

							// �g�p���Ă���
							g_Lightning[z][vectype][x].bUse = TRUE;
							g_Lightning[z][g_vecType[z]][0].bUse = TRUE;

							// �x�N�g���̃��Z�b�g
							g_vector[z][g_vecType[z]].x = RamdomFloat(3, 0.7f, -0.7f);
							g_vector[z][g_vecType[z]].z = RamdomFloat(3, 0.7f, -0.7f);
							g_vector[z][g_vecType[z]].y = RamdomFloat(3, -0.1f, -0.8f);

							// ���̑����������_���Őݒ�
							g_scale[z][g_vecType[z]].x = g_scale[z][g_vecType[z]].y = RamdomFloat(3, 3.0f, 1.5f);

							pos.x += g_vector[z][g_vecType[z]].x;
							pos.y += g_vector[z][g_vecType[z]].y;
							pos.z += g_vector[z][g_vecType[z]].z;

							// �V�������C�g�j���O�̃|�W�V�����Ƃ��Đݒ�
							g_Lightning[z][g_vecType[z]][0].pos = pos;
							g_Lightning[z][g_vecType[z]][0].vecType = g_vecType[z];
							g_Lightning[z][g_vecType[z]][0].scale = g_scale[z][g_vecType[z]];
						}
					}
				}
			}

			return;
		}
	}
}


//=============================================================================
// �����@�̃��Z�b�g����
//=============================================================================
void ResetLightning(int z)
{
	PLAYER *player = GetPlayer();

	for (int y = 0; y < MAX_REFRACTION; y++)
	{
		for (int x = 0; x < BIFURCATION_COUNT; x++)
		{
			g_Lightning[z][y][x].pos.x = player->pos.x;
			g_Lightning[z][y][x].pos.z = player->pos.z;
			g_Lightning[z][y][x].pos.y = LIGHTNING_SET_Y;

			g_Lightning[z][y][x].pos.x -= sinf(player->rot.y) * LIGHTNING_DISTANCE;
			g_Lightning[z][y][x].pos.z -= cosf(player->rot.y) * LIGHTNING_DISTANCE;

			g_Lightning[z][y][x].bUse = FALSE;
			g_Lightning[z][y][x].material.Diffuse.w = 1.0f;
			g_Lightning[z][y][x].vecType = 0;
			g_Lightning[z][y][x].count = 0;

			g_Lightning[z][y][x].nLife = LIGHTNING_LIFE;
		}
	}

	// �x�N�g����������
	for (int i = 1; i < MAX_REFRACTION; i++)
	{
		g_vector[z][i] = { 0.0f,0.0f,0.0f };
	}

	g_vecType[z] = 0;
	g_bifurcation[z] = 0;
}


//=============================================================================
// float�^�ŕ����ɂ��Ή����Ă��郉���_���̒l��Ԃ��֐�
// digits:�����_�ȉ��̌���(0.01f �� 2), max:�~�����ő�l, min:�~�����ŏ��l
//=============================================================================
float RamdomFloat(int digits, float max, float min)
{
	// �����_���������߂̕ϐ����쐬
	int l_digits = 1;	
	for (int i = 0; i < digits; i++)
	{
		l_digits *= 10;
	}

	// ��]�Z���邽�߂̒l���쐬
	float surplus = (max - min) * l_digits;

	int random = rand() % (int)surplus;

	float ans;

	// �����̃����_���̒l��float�^�փL���X�g�ƕ����̏���
	ans = (float)random / l_digits + min;

	return ans;
}


//=============================================================================
//	�����@�����s�����ǂ���
//=============================================================================
BOOL GetLightningOnOff(void)
{
	return g_LightningOnOff;
}


//=============================================================================
//	�����@��OnOff���Ǘ�
//=============================================================================
void SetLightningOnOff(BOOL data)
{
	g_LightningOnOff = data;
}