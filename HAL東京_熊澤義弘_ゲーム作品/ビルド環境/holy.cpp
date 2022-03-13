//=============================================================================
//
// �z�[���[���� [holy.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "holy.h"
#include "player.h"
#include "enemy.h"
#include "math.h"
#include "collision.h"
#include "holy_particle.h"
#include "holy_orbit.h"
#include "sound.h"
#include "tutorial.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_HOLY0		"data/MODEL/circle.obj"		// �ǂݍ��ރ��f����
#define	MODEL_HOLY1		"data/MODEL/sword.obj"		// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(5.0f)					// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)			// ��]��

#define HOLY_SPEED			(0.018f)				// �z�[���[�̈ړ����x
#define HOLY_SHADOW_SIZE	(0.4f)					// �e�̑傫��
#define HOLY_OFFSET_Y		(25.0f)					// ���[�t�B���O�̑��������킹��

#define SWORD_DISTANCE		(50.0f)					// �����v���C���[����ǂ̂��炢����Ă��邩
#define SWORD_CONTROL_POINT	(600.0f)				// ����_���v���C���[����ǂ̂��炢����Ă��邩

#define HOLY_SCALE_COUNT	(20)					// �������̑傫���Ȃ鑬�x

#define HOLY_OFFSET			(60 * MAX_HOLY)			// �z�[���[���G�ɓ������Ă�������n�߂�܂ł̎���
#define HOLY_DELCOUNT		(60)					// �z�[���[��������܂ł̃t���[����

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void ResetHoly(int i);



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static HOLY				g_Holy[MAX_HOLY];				// ���[�t�B���O

static BOOL				g_Load = FALSE;

static MODEL			g_Holy_Vertex[MAX_HOLY_MOVE];	// ���[�t�B���O�p���f���̒��_��񂪓������f�[�^�z��

static HOLY_MOVE		g_Linear[MAX_HOLY_MOVE];

static ID3D11Buffer		*g_VertexBuffer = NULL;	// ���_�o�b�t�@

static VERTEX_3D		*g_Vertex = NULL;

static BOOL				g_move = FALSE;

static XMFLOAT3			g_target;				// ���ː�
static XMFLOAT3			g_ControlPoint[MAX_HOLY];			// ����_
static XMFLOAT3			g_PosOld[MAX_HOLY];					// 1�t���[���O�̍��W
static XMFLOAT3			g_distance[MAX_HOLY];			// 1�t���[���Ői�񂾋������L�^

static float g_time;	// ���[�t�B���O�̌o�ߎ���
static int mof;			// ���[�t�B���O�̃��f���ԍ�

static int g_offset;	// �z�[���[���G�ɓ������Ă�������n�߂�܂ł̎���


static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(   0.0f, HOLY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
	{ XMFLOAT3(-200.0f, HOLY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*1 },
	{ XMFLOAT3(-200.0f, HOLY_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*0.5f },
	{ XMFLOAT3(   0.0f, HOLY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
};


//=============================================================================
// ����������
//=============================================================================
HRESULT InitHoly(void)
{
	for (int i = 0; i < MAX_HOLY; i++)
	{
		LoadModel(MODEL_HOLY0, &g_Holy[i].model, 1);
		g_Holy[i].load = TRUE;

		g_Holy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, HOLY_OFFSET_Y, 20.0f);
		g_Holy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Holy[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Holy[i].diffuse[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_Holy[i].spd = 0.0f;		// �ړ��X�s�[�h�N���A
		g_Holy[i].size = HOLY_SIZE;	// �����蔻��̑傫��

		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		GetModelDiffuse(&g_Holy[0].model, &g_Holy[0].diffuse[0]);

		XMFLOAT3 pos = g_Holy[i].pos;
		pos.y -= (HOLY_OFFSET_Y - 0.1f);

		g_Holy[i].move_time = 0.0f;	// ���`��ԗp�̃^�C�}�[���N���A
		g_Holy[i].tbl_adr = NULL;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Holy[i].tbl_size = 0;		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

		g_Holy[i].use = FALSE;			// TRUE:�����Ă�
		g_Holy[i].firing = FALSE;		// FALSE:���˂��Ă��Ȃ�
		g_Holy[i].time = 0.0f;

		g_ControlPoint[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);		// ����_��������
	}


	// ���[�t�B���O����I�u�W�F�N�g�̓ǂݍ���
	LoadObj(MODEL_HOLY0, &g_Holy_Vertex[0]);
	LoadObj(MODEL_HOLY1, &g_Holy_Vertex[1]);

	// ���g��z��Ƃ��Ďg�p�ł���悤�Ɏd�l�ύX
	g_Vertex = new VERTEX_3D[g_Holy_Vertex[0].VertexNum];

	// ����(�r���o��)�̏�����
	for (int i = 0; i < g_Holy_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Holy_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse  = g_Holy_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal   = g_Holy_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Holy_Vertex[0].VertexArray[i].TexCoord;
	}

	g_time = 0.0f;
	mof = 0;

	// �ڕW�n�_��������
	g_target = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitHoly(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_HOLY; i++)
	{
		if (g_Holy[i].load)
		{
			UnloadModel(&g_Holy[i].model);
			g_Holy[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateHoly(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();

	// ���[�t�B���O�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	for (int i = 0; i < MAX_HOLY; i++)
	{
		if (g_Holy[i].use == TRUE)			// ���̃��[�t�B���O���g���Ă���H
		{									// Yes
			// �������傫���Ȃ�
			if (g_Holy[i].scl.x < 1.0f)
			{
				g_Holy[i].scl.x += 1.0f / HOLY_SCALE_COUNT;
				g_Holy[i].scl.y += 1.0f / HOLY_SCALE_COUNT;
				g_Holy[i].scl.z += 1.0f / HOLY_SCALE_COUNT;
			}

			if (g_Holy[i].firing)
			{
				// �v�Z���������Ȃ�Ȃ��悤�������邽��
				float t = g_Holy[i].time;
				float t1 = 1.0f - g_Holy[i].time;

				// �x�W�F�Ȑ��̏���
				g_Holy[i].pos.x = ((t1 * t1) * g_Holy[i].pos.x) + (2 * t1 * t * g_ControlPoint[i].x) + ((t * t) * g_target.x);
				g_Holy[i].pos.z = ((t1 * t1) * g_Holy[i].pos.z) + (2 * t1 * t * g_ControlPoint[i].z) + ((t * t) * g_target.z);
				g_Holy[i].pos.y = ((t1 * t1) * g_Holy[i].pos.y) + (2 * t1 * t * g_ControlPoint[i].y) + ((t * t) * g_target.y);

				if (g_Holy[i].time < 1.0f) g_Holy[i].time += HOLY_SPEED;
				if (g_Holy[i].time >= 1.0f) g_offset++;		// �ړ����I����Ă���J�E���g�J�n

				// �ڕW�܂œ��B�����ꍇ�ɏI������
				if ((g_move) && (g_Holy[i].time >= 1.0f) && (i == 0) && (g_offset >= HOLY_OFFSET))
				{
					for (int p = 0;p < g_Holy_Vertex[0].VertexNum;p++)
					{
						g_Vertex[p].Diffuse.w -= 1.0f / HOLY_DELCOUNT;
					}

					SetParticleSwitch(TRUE);
				}


				// �ڕW���Ɍ��悪�����悤��
				{
					// ����_�܂ł͐���_�Ɍ��悪�����悤��
					if (g_Holy[i].time < 0.12f)
					{
						g_Holy[i].direct.x = g_ControlPoint[i].x - g_Holy[i].pos.x;
						g_Holy[i].direct.y = g_ControlPoint[i].y - g_Holy[i].pos.y;
						g_Holy[i].direct.z = g_ControlPoint[i].z - g_Holy[i].pos.z;
					}
					else if (g_Holy[i].time < 0.8f)
					{
						g_Holy[i].direct.x = g_target.x - g_Holy[i].pos.x;
						g_Holy[i].direct.y = g_target.y - g_Holy[i].pos.y;
						g_Holy[i].direct.z = g_target.z - g_Holy[i].pos.z;
					}

					// �G�l�~�[�܂ł̋�����2���ȉ��ɂȂ�ƌ���͌Œ�
					if (g_Holy[i].time < 0.9f)
					{
						float x, z;
						x = g_target.x - g_Holy[i].pos.x;
						z = g_target.z - g_Holy[i].pos.z;

						// �v���C���[�̈ʒu�ŏ�����ύX
						if (fabsf(x) > fabsf(z))
						{
							// y����]�̒���
							if ((g_Holy[i].direct.z > 0.0f) && (g_Holy[i].direct.x > 0.0f))
							{	// ����
								g_Holy[i].rot.y = (XM_PI / 2) + atanf(g_Holy[i].direct.x / g_Holy[i].direct.z);
							}
							else if((g_Holy[i].direct.x < 0.0f) && (g_Holy[i].direct.z > 0.0f))
							{	// �E��
								g_Holy[i].rot.y = (XM_PI / 2) + atanf(g_Holy[i].direct.x / g_Holy[i].direct.z);
							}
							else if (g_Holy[i].direct.x < 0.0f)
							{	// �E��
								g_Holy[i].rot.y = (-XM_PI / 2) + atanf(g_Holy[i].direct.x / g_Holy[i].direct.z);
							}
							else // if (g_Holy[i].direct.x > 0.0f)
							{	// ����
								g_Holy[i].rot.y = (-XM_PI / 2) + atanf(g_Holy[i].direct.x / g_Holy[i].direct.z);
							}


							// z����]�̒���
							if (g_Holy[i].direct.x > 0.0f)
							{
								g_Holy[i].rot.z = (-XM_PI / 2) + atanf(g_Holy[i].direct.y / g_Holy[i].direct.x);
							}
							else
							{
								g_Holy[i].rot.z = (-XM_PI / 2) - atanf(g_Holy[i].direct.y / g_Holy[i].direct.x);
							}
						}
						else
						{
							// y����]�̒���
							if ((g_Holy[i].direct.z > 0.0f) && (g_Holy[i].direct.x > 0.0f))
							{	// ����
								g_Holy[i].rot.y = (-XM_PI / 2) - atanf(g_Holy[i].direct.z / g_Holy[i].direct.x);
							}
							else if ((g_Holy[i].direct.x < 0.0f) && (g_Holy[i].direct.z > 0.0f))
							{	// �E��
								g_Holy[i].rot.y = (XM_PI / 2) - atanf(g_Holy[i].direct.z / g_Holy[i].direct.x);
							}
							else if (g_Holy[i].direct.x < 0.0f)
							{	// �E��
								g_Holy[i].rot.y = (XM_PI / 2) - atanf(g_Holy[i].direct.z / g_Holy[i].direct.x);
							}
							else // if (g_Holy[i].direct.x > 0.0f)
							{	// ����
								g_Holy[i].rot.y = (-XM_PI / 2) - atanf(g_Holy[i].direct.z / g_Holy[i].direct.x);
							}

							// x����]�̒���
							if (g_Holy[i].direct.z > 0.0f)
							{
								g_Holy[i].rot.x = (XM_PI / 2) - atanf(g_Holy[i].direct.y / g_Holy[i].direct.z);
							}
							else
							{
								g_Holy[i].rot.x = (XM_PI / 2) + atanf(g_Holy[i].direct.y / g_Holy[i].direct.z);
							}
						}
					}
				}

				// ���S�ɓ����ɂȂ����疢�g�p��
				if (g_Vertex[0].Diffuse.w <= 0.0f)
				{
					ResetHoly(i);
					SetParticleSwitch(FALSE);
				}
			}
			else 
			{
				// �v���C���[�̔w��Ɉʒu����
				g_Holy[i].pos.x = player->pos.x + sinf(player->rot.y + XM_PI * (-0.3f + 0.15f * i)) * SWORD_DISTANCE;
				g_Holy[i].pos.z = player->pos.z + cosf(player->rot.y + XM_PI * (-0.3f + 0.15f * i)) * SWORD_DISTANCE;

				// �v���C���[�̌����ɉ����ă��f���̌������ύX
				g_Holy[i].rot.y = player->rot.y + XM_PI * (-0.8f + 0.15f * i);

				// ����_���L�^
				g_ControlPoint[i].x = player->pos.x + sinf(player->rot.y + XM_PI / 2) * SWORD_CONTROL_POINT * (-0.3f + 0.15f * i);
				g_ControlPoint[i].z = player->pos.z + cosf(player->rot.y + XM_PI / 2) * SWORD_CONTROL_POINT * (-0.3f + 0.15f * i);
				g_ControlPoint[i].y = player->pos.y + cosf(player->rot.z + XM_PI * (-0.5f + 0.25f * i + 0.01f)) * SWORD_CONTROL_POINT / 4;
			}

			// �������L�^
			g_distance[i].x = g_Holy[i].pos.x - g_PosOld[i].x;
			g_distance[i].y = g_Holy[i].pos.y - g_PosOld[i].y;
			g_distance[i].z = g_Holy[i].pos.z - g_PosOld[i].z;


			g_PosOld[i].x = g_Holy[i].pos.x;
			g_PosOld[i].y = g_Holy[i].pos.y;
			g_PosOld[i].z = g_Holy[i].pos.z;



			// ���[�t�B���O�̏���
			{
				int after, brfore;

				after = (mof + 1) % MAX_HOLY_MOVE;
				brfore = mof % MAX_HOLY_MOVE;

				for (int p = 0; p < g_Holy_Vertex[0].VertexNum; p++)
				{
					g_Vertex[p].Position.x = g_Holy_Vertex[after].VertexArray[p].Position.x - g_Holy_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y = g_Holy_Vertex[after].VertexArray[p].Position.y - g_Holy_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z = g_Holy_Vertex[after].VertexArray[p].Position.z - g_Holy_Vertex[brfore].VertexArray[p].Position.z;

					g_Vertex[p].Position.x *= g_time;
					g_Vertex[p].Position.y *= g_time;
					g_Vertex[p].Position.z *= g_time;

					g_Vertex[p].Position.x += g_Holy_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y += g_Holy_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z += g_Holy_Vertex[brfore].VertexArray[p].Position.z;
				}

				// ���Ԃ�i�߂�
				if((g_move) && (g_time < 1.0f)) g_time += 0.005f;

				// �ڕW�܂œ��B������O��������
				if(g_time < 1.0f) SetOrbitSwitch(FALSE);

				D3D11_SUBRESOURCE_DATA sd;
				ZeroMemory(&sd, sizeof(sd));
				sd.pSysMem = g_Vertex;

				// ���_�o�b�t�@�ɒl���Z�b�g����
				D3D11_MAPPED_SUBRESOURCE msr;
				GetDeviceContext()->Map(g_Holy[i].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
				VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

				// �S���_���𖈉�㏑�����Ă���̂�DX11�ł͂��̕�����������ł�
				memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Holy_Vertex[0].VertexNum);

				GetDeviceContext()->Unmap(g_Holy[i].model.VertexBuffer, 0);
			}
		}
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawHoly(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ���C�e�B���O�𖳌���
	SetLightEnable(FALSE);

	// �t�H�O����
	SetFogEnable(FALSE);

	for (int i = 0; i < MAX_HOLY; i++)
	{
		if (g_Holy[i].use == FALSE) continue;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Holy[i].scl.x, g_Holy[i].scl.y, g_Holy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Holy[i].rot.x, g_Holy[i].rot.y + XM_PI, g_Holy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Holy[i].pos.x, g_Holy[i].pos.y, g_Holy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Holy[i].mtxWorld, mtxWorld);


		// ���f���`��
		DrawModel(&g_Holy[i].model);
	}

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// �t�H�O�L��
	SetFogEnable(GetFogSwitch());
}

//=============================================================================
// �z�[���[�̎擾
//=============================================================================
HOLY *GetHoly()
{
	return &g_Holy[0];
}


//=============================================================================
// �z�[���[�̃Z�b�g
//=============================================================================
void SetHoly(void)
{
	// ���f���̏�����
	for (int i = 0; i < g_Holy_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Holy_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse = g_Holy_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal = g_Holy_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Holy_Vertex[0].VertexArray[i].TexCoord;
	}

	D3D11_SUBRESOURCE_DATA sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.pSysMem = g_Vertex;

	for (int i = 0; i < MAX_HOLY; i++)
	{
		g_Holy[i].use = TRUE;

		// ���_�o�b�t�@�ɒl���Z�b�g����
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_Holy[i].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

		// �S���_���𖈉�㏑�����Ă���̂�DX11�ł͂��̕�����������ł�
		memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Holy_Vertex[0].VertexNum);

		GetDeviceContext()->Unmap(g_Holy[i].model.VertexBuffer, 0);
	}

	// ���ʉ��Đ�
	PlaySound(SOUND_LABEL_SE_MAGIC_holy_set);
}


// ���_���̏�������
void SetHolyVertex(DX11_MODEL *Model)
{
	// ���_�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * g_Holy_Vertex[0].VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_Holy_Vertex[0].VertexArray;

		GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer);
	}
}


// ���[�t�B���O�J�n
void SetMove(void)
{
	if (g_move) return;
	g_move = TRUE;

	// ���ʉ��Đ�
	PlaySound(SOUND_LABEL_SE_MAGIC_holy_morphing);
}

// ���[�t�B���O���������Ă��邩
BOOL GetMove(void)
{
	return g_move;
}


// �����@�̔���
void SetFiringSword(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();

	float distance = 15000.0f, x, z;	// �G�l�~�[�܂ł̋����v�Z�Ɏg�p
	int num = 0;						// ��ԋ߂��G�l�~�[�̔ԍ�(-1�͔͈͊O)

	for (int i = 0; i < MAX_HOLY; i++)
	{
		// ���g�p�ł���Δ���
		if (g_Holy[i].firing == FALSE)
		{
			g_Holy[i].firing = TRUE;

			// ��񂾂�����������
			if (i == 0)
			{
				for (int e = 0; e < MAX_ENEMY; e++)
				{
					if (enemy[e].use)
					{
						// �G�l�~�[�Ƃ̋������v�Z
						x = g_Holy[2].pos.x - enemy[e].pos.x;
						z = g_Holy[2].pos.z - enemy[e].pos.z;

						// ��ԉ����G�l�~�[�̏����L�^
						if (distance < x * x + z * z)
						{
							distance = x * x + z * z;
							num = e;
						}
					}
				}

				// �ڕW�n�_�Ɛ���_��������
				g_target = XMFLOAT3(0.0f, 0.0f, 0.0f);

				// �ڕW�n�_���L�^
				g_target = enemy[num].pos;

				// ���ʉ��Đ�
				PlaySound(SOUND_LABEL_SE_MAGIC_holy_firing);

				// �O���̃p�[�e�B�N���𔭐�������
				SetOrbitSwitch(TRUE);
			}
		}
	}
}


// �z�[���[�̃��Z�b�g����
void ResetHoly(int i)
{
	g_Holy[i].use = FALSE;
	g_Holy[i].firing = FALSE;
	g_Holy[i].time = 0.0f;
	g_Holy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, HOLY_OFFSET_Y, 20.0f);
	g_Holy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Holy[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Holy[i].direct = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_move = FALSE;
	g_time = 0.0f;
	g_offset = 0;

	// �`���[�g���A���N���A
	SetTutorialClear(tx_holy);
}


// 1�t���[���łǂꂭ�炢�i�񂾂���Ԃ�
XMFLOAT3 GetHolyMoveValue(int i)
{
	return g_distance[i];
}