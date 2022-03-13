//=============================================================================
//
// �E�H�[�^�[���� [water.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "water.h"
#include "player.h"
#include "enemy.h"
#include "math.h"
#include "collision.h"
#include "sound.h"
#include "tutorial.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_WATER0		"data/MODEL/polka_dots_0.obj"		// �ǂݍ��ރ��f����
#define	MODEL_WATER1		"data/MODEL/polka_dots_1.obj"		// �ǂݍ��ރ��f����
#define	MODEL_WATER2		"data/MODEL/polka_dots_2.obj"		// �ǂݍ��ރ��f����
#define	MODEL_WATER_DROP_0	"data/MODEL/polka_drop_0.obj"		// �ǂݍ��ރ��f����
#define	MODEL_WATER_DROP_1	"data/MODEL/polka_drop_1.obj"		// �ǂݍ��ރ��f����
#define	MODEL_WATER_DROP_2	"data/MODEL/polka_drop_2.obj"		// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(5.0f)						// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// ��]��

#define WATER_SHADOW_SIZE	(0.4f)						// �e�̑傫��
#define WATER_OFFSET_Y		(40.0f)						// ���[�t�B���O�̑��������킹��
#define WATER_OFFSET_X		(40.0f)						// �����蔻��̑傫��

#define WATER_HIGHT_DISTANCE	(80.0f)					// ���ʂ��v���C���[����ǂ̂��炢����Ă��邩
#define WATER_HIGHT			(120.0f)					// �����@�������������ɂǂ��܂ŃG�l�~�[���グ�邩
#define WATER_DISTANCE		(50.0f)						// ���ʂ��v���C���[����ǂ̂��炢����Ă��邩

#define WATER_LIFE			(400)						// �G�ɓ������Ă��������܂ł̎���
#define WATER_PENETRATE_MAX		(120)					// �n�ʂɐZ�����Ă��������܂ł̎���
#define WATER_PENETRATE_SPEED	(0.2f)					// �n�ʂɐZ�����鑬�x

#define WATER_SCALE_COUNT	(30)						// ���ʂ���������܂ł̃t���[����
#define WATER_SCALE_MAX		(2.3f)						// ���ʂ̍ő�̑傫��

#define WATER_DISSOLVE		(0.003f)						// �f�B�]���u�̃X�s�[�h

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static WATER		g_Water[MAX_WATER];				// ���[�t�B���O

static BOOL			g_Load = FALSE;

static MODEL		g_Water_Vertex[MAX_WATER_MOVE];	// ���[�t�B���O�p���f���̒��_��񂪓������f�[�^�z��
static VERTEX_3D	*g_Vertex = NULL;				// �r���o�߂��L�^����ꏊ

static WATER_MOVE	g_Linear[MAX_WATER_MOVE];

static ID3D11Buffer	*g_VertexBuffer = NULL;			// ���_�o�b�t�@

static BOOL			g_move = FALSE;

static XMFLOAT3		g_target;						// ���ː�
static int			g_targetnum;					// ���ː�G�l�~�[�ԍ�
static XMFLOAT3		g_ControlPoint[MAX_WATER];		// ����_
static XMFLOAT3		g_PosOld[MAX_WATER];			// 1�t���[���O�̍��W

static float		g_time;
static int			mof;
static float		g_rot;
static BOOL			g_drop = FALSE;				// �����郂�[�V�����Ɉڍs���邩�ǂ���
static int			g_PenetrationCount = 0;		// �n�ʂɐZ�����Ă������Ԃ��J�E���g

static float		g_Dissolve;					// �g��̃f�B�]���u

static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(   0.0f, WATER_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
	{ XMFLOAT3(-200.0f, WATER_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*1 },
	{ XMFLOAT3(-200.0f, WATER_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*0.5f },
	{ XMFLOAT3(   0.0f, WATER_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
};


//=============================================================================
// ����������
//=============================================================================
HRESULT InitWater(void)
{
	for (int i = 0; i < MAX_WATER; i++)
	{
		LoadModel(MODEL_WATER0, &g_Water[i].model, 1);
		g_Water[i].load = TRUE;

		g_Water[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Water[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Water[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Water[i].diffuse[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_Water[i].spd = 0.0f;			// �ړ��X�s�[�h�N���A
		g_Water[i].size = WATER_SIZE;	// �����蔻��̑傫��
		g_Water[i].life = WATER_LIFE;
		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		GetModelDiffuse(&g_Water[0].model, &g_Water[0].diffuse[0]);

		XMFLOAT3 pos = g_Water[i].pos;
		pos.y -= (WATER_OFFSET_Y - 0.1f);

		g_Water[i].move_time = 0.0f;	// ���`��ԗp�̃^�C�}�[���N���A

		g_Water[i].use = FALSE;			// TRUE:�����Ă�
		g_Water[i].firing = FALSE;		// FALSE:���˂��Ă��Ȃ�
		g_Water[i].time = 0.0f;
		g_ControlPoint[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);		// ����_��������
	}


	// ���[�t�B���O����I�u�W�F�N�g�̓ǂݍ���
	LoadObj(MODEL_WATER0, &g_Water_Vertex[0]);
	LoadObj(MODEL_WATER1, &g_Water_Vertex[1]);
	LoadObj(MODEL_WATER2, &g_Water_Vertex[2]);

	LoadObj(MODEL_WATER_DROP_0, &g_Water_Vertex[3]);
	LoadObj(MODEL_WATER_DROP_1, &g_Water_Vertex[4]);
	LoadObj(MODEL_WATER_DROP_2, &g_Water_Vertex[5]);

	// ���g��z��Ƃ��Ďg�p�ł���悤�Ɏd�l�ύX
	g_Vertex = new VERTEX_3D[g_Water_Vertex[0].VertexNum];

	// ����(�r���o��)�̏�����
	for (int i = 0; i < g_Water_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Water_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse  = g_Water_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal   = g_Water_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Water_Vertex[0].VertexArray[i].TexCoord;

		g_Vertex[i].Diffuse.x *= 1.2f;
		g_Vertex[i].Diffuse.y *= 1.2f;
		g_Vertex[i].Diffuse.z *= 1.2f;
		g_Vertex[i].Diffuse.w = 0.7f;
	}

	g_time = 0.0f;
	g_rot = 0.0f;
	mof = 0;
	g_Dissolve = 0.0f;

	// �ڕW�n�_��������
	g_target = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Load = TRUE;
	return S_OK;
}


//=============================================================================
// �I������
//=============================================================================
void UninitWater(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_WATER; i++)
	{
		if (g_Water[i].load)
		{
			UnloadModel(&g_Water[i].model);
			g_Water[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}


//=============================================================================
// �X�V����
//=============================================================================
void UpdateWater(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();

	// ���[�t�B���O�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	for (int i = 0; i < MAX_WATER; i++)
	{
		if (g_Water[i].use == TRUE)			// ���̃��[�t�B���O���g���Ă���H
		{									// Yes
			// �������傫���Ȃ�
			if (g_Water[i].scl.x < WATER_SCALE_MAX)
			{
				g_Water[i].scl.x += WATER_SCALE_MAX / WATER_SCALE_COUNT;
				g_Water[i].scl.y += WATER_SCALE_MAX / WATER_SCALE_COUNT;
				g_Water[i].scl.z += WATER_SCALE_MAX / WATER_SCALE_COUNT;
			}

			// �f�B�]���u����
			g_Dissolve += WATER_DISSOLVE;
			if (g_Dissolve > 1.0f) g_Dissolve -= 1.0f;

			// ���ˌ�̏���
			if (g_Water[i].firing == TRUE)
			{	// ���˃t���O���I���̎�
				XMFLOAT3 dis;
				dis.x = enemy[g_targetnum].pos.x - g_Water[i].pos.x;
				dis.y = enemy[g_targetnum].pos.y - g_Water[i].pos.y;
				dis.z = enemy[g_targetnum].pos.z - g_Water[i].pos.z;

				g_Water[i].pos.x += dis.x * 0.1f;
				g_Water[i].pos.y += dis.y * 0.1f;
				g_Water[i].pos.z += dis.z * 0.1f;


				// �����@����苗���܂ŃG�l�~�[�ɋ߂Â�����
				if (g_Water[i].life > 0)
				{
					if ((fabsf(dis.x) < WATER_DISTANCE) && (fabsf(dis.y) < WATER_DISTANCE) && (fabsf(dis.z) < WATER_DISTANCE))
					{	// �G�l�~�[�𕂂����āA��]������
						float hight;
						hight = WATER_HIGHT - enemy[g_targetnum].pos.y;

						enemy[g_targetnum].pos.y += hight * 0.04f;

						// ���ʂ̎��������炷
						g_Water[i].life--;

						// ���C�t������n�߂���SE���Đ�
						if (g_Water[i].life == WATER_LIFE - 1)
						{
							// SE�̍Đ�
							PlaySound(SOUND_LABEL_SE_MAGIC_water01);
						}
						else if (g_Water[i].life == WATER_LIFE / 2)
						{
							// SE�̍Đ�
							PlaySound(SOUND_LABEL_SE_MAGIC_water01);
						}
					}
				}
				else
				{
					// �������s�����痎���郂�[�V�����ֈڍs
					SetPolkaDrop(i);
				}

			}
			else if (g_drop == FALSE)
			{
				// �v���C���[�̓���Ɉʒu����
				g_Water[i].pos = player->pos;
				g_Water[i].pos.y += WATER_HIGHT_DISTANCE;
			}


			g_PosOld[i].x = g_Water[i].pos.x;
			g_PosOld[i].y = g_Water[i].pos.y;
			g_PosOld[i].z = g_Water[i].pos.z;


			// ���[�t�B���O�̏���
			{
				int after, brfore;

				// ���[�t�B���O���f���̔ԍ�����
				if (g_drop == FALSE)
				{
					after = (mof + 1) % (MAX_WATER_MOVE / 2);
					brfore = mof % (MAX_WATER_MOVE / 2);
				}
				else
				{
					after = (mof + 1) % (MAX_WATER_MOVE / 2) + (MAX_WATER_MOVE / 2);
					brfore = mof % (MAX_WATER_MOVE / 2) + (MAX_WATER_MOVE / 2);
				}

				// ���[�t�B���O����
				for (int p = 0; p < g_Water_Vertex[0].VertexNum; p++)
				{
					g_Vertex[p].Position.x = g_Water_Vertex[after].VertexArray[p].Position.x - g_Water_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y = g_Water_Vertex[after].VertexArray[p].Position.y - g_Water_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z = g_Water_Vertex[after].VertexArray[p].Position.z - g_Water_Vertex[brfore].VertexArray[p].Position.z;

					g_Vertex[p].Position.x *= g_time;
					g_Vertex[p].Position.y *= g_time;
					g_Vertex[p].Position.z *= g_time;

					g_Vertex[p].Position.x += g_Water_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y += g_Water_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z += g_Water_Vertex[brfore].VertexArray[p].Position.z;
				}

				// ���Ԃ�i�߂�
				if (g_drop == FALSE)
				{	// ����
					g_time += 0.03f;
					if (g_time > 1.0f)
					{
						mof++;
						g_time = 0.0f;
					}
				}
				else
				{	// �����郂�[�V����
					if ((g_time < 1.0f) && (mof < 2)) g_time += 0.06f;
					if (g_time > 1.0f)
					{
						mof++;
						g_time = 0.0f;
					}
				}

				// �����郂�[�V�������I����Ă���Ƃ�
				if ((g_drop) && (mof >= 2))
				{
					// �������n�ʂɐZ��������
					g_Water[i].pos.y -= WATER_PENETRATE_SPEED;
					g_PenetrationCount++;
					
					// �n�ʂɐZ��������I������
					if (g_PenetrationCount >= WATER_PENETRATE_MAX)
					{
						ResetWater(i);

						// �`���[�g���A���N���A
						SetTutorialClear(tx_water);
					}
				}

				// ���ʂ���]������
				g_rot += 0.02f;
				g_Water[i].rot.y = g_rot;

				D3D11_SUBRESOURCE_DATA sd;
				ZeroMemory(&sd, sizeof(sd));
				sd.pSysMem = g_Vertex;

				// ���_�o�b�t�@�ɒl���Z�b�g����
				D3D11_MAPPED_SUBRESOURCE msr;
				GetDeviceContext()->Map(g_Water[i].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
				VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

				// �S���_���𖈉�㏑�����Ă���̂�DX11�ł͂��̕�����������ł�
				memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Water_Vertex[0].VertexNum);

				GetDeviceContext()->Unmap(g_Water[i].model.VertexBuffer, 0);


				//// ���_�����㏑��
				//D3D11_BUFFER_DESC bd;
				//ZeroMemory(&bd, sizeof(bd));
				//bd.Usage = D3D11_USAGE_DEFAULT;
				//bd.ByteWidth = sizeof(VERTEX_3D) * g_Water_Vertex[0].VertexNum;
				//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				//bd.CPUAccessFlags = 0;

				//D3D11_SUBRESOURCE_DATA sd;
				//ZeroMemory(&sd, sizeof(sd));
				//sd.pSysMem = g_Vertex;

				//UnloadModelVertex(&g_Water[i].model);
				//GetDevice()->CreateBuffer(&bd, &sd, &g_Water[i].model.VertexBuffer);
			}
		}
	}
}


//=============================================================================
// �`�揈��
//=============================================================================
void DrawWater(void)
{
	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ���C�e�B���O�𖳌���
	SetLightEnable(FALSE);

	// �t�H�O����
	SetFogEnable(FALSE);

	// �������C�gOn
	SetFuchi(1);

	for (int i = 0; i < MAX_WATER; i++)
	{
		if (g_Water[i].use == FALSE) continue;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Water[i].scl.x, g_Water[i].scl.y, g_Water[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Water[i].rot.x, g_Water[i].rot.y + XM_PI, g_Water[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Water[i].pos.x, g_Water[i].pos.y, g_Water[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Water[i].mtxWorld, mtxWorld);

		// �f�B�]���u�̐ݒ�
		SetDissolve(g_Dissolve);

		// ���f���`��
		DrawModel(&g_Water[i].model);
	}

	// �������C�gOff
	SetFuchi(0);

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// �t�H�O�L��
	SetFogEnable(GetFogSwitch());

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// ���[�t�B���O�̎擾
//=============================================================================
WATER *GetWater()
{
	return &g_Water[0];
}


//=============================================================================
// ���_���̏�������
//=============================================================================
void SetWaterVertex(DX11_MODEL *Model)
{
	// ���_�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * g_Water_Vertex[0].VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_Water_Vertex[0].VertexArray;

		GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer);
	}
}


//=============================================================================
// ���[�t�B���O�J�n
//=============================================================================
void SetWaterMove(void)
{
	g_move = TRUE;
}


//=============================================================================
// ���[�t�B���O���������Ă��邩
//=============================================================================
BOOL GetWaterMove(void)
{
	return g_move;
}


//=============================================================================
// �����@�̉r��
//=============================================================================
void SetWater(void)
{
	for (int i = 0; i < MAX_WATER; i++)
	{
		if (g_Water[i].use == FALSE)
		{
			g_Water[i].use = TRUE;

			// SE�̍Đ�
			PlaySound(SOUND_LABEL_SE_MAGIC_water00);
			return;
		}
	}
}


//=============================================================================
// �����@���g�p���Ă��邩�ǂ�����Ԃ�
//=============================================================================
BOOL GetWaterUse(int i)
{
	return g_Water[i].use;
}


//=============================================================================
// �����@�̔���
//=============================================================================
void SetFiringWater(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();

	float distance = 4000000.0f, x, z;	// �G�l�~�[�܂ł̋����v�Z�Ɏg�p
	int num = 0;					// ��ԋ߂��G�l�~�[�̔ԍ�(-1�͔͈͊O)

	for (int i = 0; i < MAX_WATER; i++)
	{
		// ���g�p�ł���Δ���
		if (g_Water[i].firing == FALSE)
		{
			g_Water[i].firing = TRUE;

			// ��񂾂�����������
			if (i == 0)
			{
				for (int e = 0; e < MAX_ENEMY; e++)
				{
					// �G�l�~�[�Ƃ̋������v�Z
					x = g_Water[0].pos.x - enemy[e].pos.x;
					z = g_Water[0].pos.z - enemy[e].pos.z;

					// ��ԋ߂��G�l�~�[�̏����L�^
					if (distance > x * x + z * z)
					{
						distance = x * x + z * z;
						num = e;
					}

				}

				// �ڕW�n�_�Ɛ���_��������
				g_target = XMFLOAT3(0.0f, 0.0f, 0.0f);
				g_ControlPoint[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);

				// �ڕW�n�_���L�^
				g_target = enemy[num].pos;
				g_targetnum = num;
			}
		}
	}
}


//=============================================================================
// ���ʂ������郂�[�V�����ւ̐؂�ւ�
//=============================================================================
void SetPolkaDrop(int i)
{
	g_Water[i].firing = FALSE;		// FALSE:���˂��Ă��Ȃ�
	if (g_drop == FALSE)
	{
		g_drop = TRUE;
		mof = 0;
		g_time = 0.0f;

		// SE�̍Đ�
		PlaySound(SOUND_LABEL_SE_MAGIC_water1);
	}
}


//=============================================================================
// �����@�̃��Z�b�g����
//=============================================================================
void ResetWater(int i)
{
	g_Water[i].use = FALSE;
	g_Water[i].time = 0.0f;
	g_Water[i].life = WATER_LIFE;
	g_Water[i].scl = { 0.0f,0.0f,0.0f };
	g_drop = FALSE;
	mof = 0;
	g_time = 0.0f;
	g_PenetrationCount = 0;

	// ���f���̏�����
	for (int i = 0; i < g_Water_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Water_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse = g_Water_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal = g_Water_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Water_Vertex[0].VertexArray[i].TexCoord;

		g_Vertex[i].Diffuse.x *= 1.2f;
		g_Vertex[i].Diffuse.y *= 1.2f;
		g_Vertex[i].Diffuse.z *= 1.2f;
		g_Vertex[i].Diffuse.w = 0.7f;
	}

	//D3D11_BUFFER_DESC bd;
	//ZeroMemory(&bd, sizeof(bd));
	//bd.Usage = D3D11_USAGE_DEFAULT;
	//bd.ByteWidth = sizeof(VERTEX_3D) * g_Water_Vertex[0].VertexNum;
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//bd.CPUAccessFlags = 0;

	//D3D11_SUBRESOURCE_DATA sd;
	//ZeroMemory(&sd, sizeof(sd));
	//sd.pSysMem = g_Vertex;

	//UnloadModelVertex(&g_Water[i].model);
	//GetDevice()->CreateBuffer(&bd, &sd, &g_Water[i].model.VertexBuffer);
}