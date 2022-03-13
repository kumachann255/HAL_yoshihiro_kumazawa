//=============================================================================
//
// ���C�g���� [light.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "player.h"
#include "bahamut.h"
#include "beam_particle.h"
#include "input.h"
#include "camera.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// �r���[���ʂ̎���p
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_WIDTH)		// �r���[���ʂ̃A�X�y�N�g��	
#define	VIEW_NEAR_Z		(10.0f)											// �r���[���ʂ�NearZ�l
#define	VIEW_FAR_Z		(10000.0f)										// �r���[���ʂ�FarZ�l



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];

static FOG		g_Fog;

// �ړ�����
static float	vector = -1.0f;

static BOOL		g_FogSwitch;

//=============================================================================
// ����������
//=============================================================================
void InitLight(void)
{

	//���C�g������
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[0].dir       = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[i].Direction = XMFLOAT3( 0.0f, -1.0f, 0.0f );
		g_Light[i].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
		g_Light[i].Ambient   = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		g_Light[i].Attenuation = 100.0f;	// ��������
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ���C�g�̃^�C�v
		g_Light[i].Enable = FALSE;			// ON / OFF
		SetLight(i, &g_Light[i]);
	}

	// ���s�����̐ݒ�i���E���Ƃ炷���j
	g_Light[0].Position = XMFLOAT3(-200.0f, 800.0f, 0.0f);
	g_Light[0].dir = XMFLOAT3(200.0f, -800.0f, 0.0f);
	g_Light[0].At = XMFLOAT3( 0.0f, 0.0f, 0.0f);
	g_Light[0].up = { 0.0f, 1.0f, 0.0f };
	g_Light[0].Direction = XMFLOAT3( 2.0f, -8.0f, 0.0f );		// ���̌���
	g_Light[0].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );	// ���̐F
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// ���s����
	g_Light[0].Enable = TRUE;									// ���̃��C�g��ON
	SetLight(0, &g_Light[0]);									// ����Őݒ肵�Ă���

	g_Light[1].Position = XMFLOAT3(0.0f, 50.0f,  50.0f);
	g_Light[1].dir = XMFLOAT3(10.0f, 10.0f, 10.0f);
	g_Light[1].At = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Light[1].up = { 0.0f, 1.0f, 0.0f };
	g_Light[1].Direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);		// ���̌���
	g_Light[1].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);	// ���̐F
	g_Light[1].Type = LIGHT_TYPE_POINT;						// ���s����
	g_Light[1].Enable = FALSE;								// ���̃��C�g��ON
	g_Light[1].Attenuation = 130.0f;						// ��������

	SetLight(1, &g_Light[1]);								// ����Őݒ肵�Ă���

	g_Light[2].Position = XMFLOAT3(0.0f, 50.0f, -50.0f);
	g_Light[2].dir = XMFLOAT3(2.0f, 2.0f, 2.0f);
	g_Light[2].At = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Light[2].up = { 0.0f, 1.0f, 0.0f };
	g_Light[2].Direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);		// ���̌���
	g_Light[2].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);	// ���̐F
	g_Light[2].Type = LIGHT_TYPE_POINT;						// ���s����
	g_Light[2].Enable = FALSE;								// ���̃��C�g��ON
	g_Light[2].Attenuation = 130.0f;						// ��������

	SetLight(2, &g_Light[2]);								// ����Őݒ肵�Ă���

	g_Light[3].Position = XMFLOAT3(0.0f, 50.0f, 10.0f);
	g_Light[3].dir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	g_Light[3].At = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Light[3].up = { 0.0f, 1.0f, 0.0f };
	g_Light[3].Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);		// ���̌���
	g_Light[3].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);	// ���̐F
	g_Light[3].Type = LIGHT_TYPE_POINT;						// ���s����
	g_Light[3].Enable = FALSE;								// ���̃��C�g��ON
	SetLight(3, &g_Light[3]);								// ����Őݒ肵�Ă���

	// �t�H�O�̏������i���̌��ʁj
	g_Fog.FogStart = 300.0f;								// ���_���炱�̋��������ƃt�H�O��������n�߂�
	g_Fog.FogEnd   = 800.0f;								// �����܂ŗ����ƃt�H�O�̐F�Ō����Ȃ��Ȃ�
	g_Fog.FogColor = XMFLOAT4( 0.2f, 0.0f, 0.2f, 1.0f );	// �t�H�O�̐F
	SetFog(&g_Fog);
	SetFogEnable(TRUE);
	g_FogSwitch = TRUE;

}


//=============================================================================
// �X�V����
//=============================================================================
void UpdateLight(void)
{
	// �������v���C���[�̃|�W�V������ɂ���
	PLAYER *player = GetPlayer();

	g_Light[0].Position.x = player->pos.x - 200.0f;
	g_Light[0].Position.z = player->pos.z;
	g_Light[0].Position.y = player->pos.y + 800.0f;
	g_Light[0].At.x = player->pos.x;
	g_Light[0].At.z = player->pos.z;

	// �u���X���g�p���Ă����ꍇ
	if (GetScene() >= charge)
	{
		BAHAMUT *bahamut = GetBahamut();

		for (int i = 1; i < 4; i++)
		{
			g_Light[i].Enable = TRUE;	// ���̃��C�g��ON

		// ���C�g�̈ʒu���o�n���[�g�̌����̂ɒ���
			if (i == 3)
			{	// �ڂ̃|�C���g���C�g
				g_Light[i].Position.x = bahamut->pos.x + 79.0f;
				g_Light[i].Position.y = bahamut->pos.y + 142.7f;

				if (g_Light[i].Diffuse.x < 30.0f)
				{
					g_Light[i].Diffuse.x += 0.1f;
					g_Light[i].Diffuse.y += 0.1f;
					g_Light[i].Diffuse.z += 0.1f;
				}
			}
			else
			{	// �����̃|�C���g���C�g
				//g_Light[i].Position.x = bahamut->pos.x + 85.0f;
				g_Light[i].Position.x = bahamut->pos.x + 100.0f;
				g_Light[i].Position.y = bahamut->pos.y + 125.0f;

				if (g_Light[i].Diffuse.x < 30.0f)
				{
					g_Light[i].Diffuse.x += 0.1f;
					g_Light[i].Diffuse.y += 0.1f;
					g_Light[i].Diffuse.z += 0.1f;
				}
			}


			// �X�V
			SetLight(i, &g_Light[i]);
		}
	}
	else
	{
		for (int i = 1; i < 4; i++)
		{
			g_Light[i].Enable = FALSE;
			g_Light[i].Diffuse.x = 0.0f;
			g_Light[i].Diffuse.y = 0.0f;
			g_Light[i].Diffuse.z = 0.0f;

			// �X�V
			SetLight(i, &g_Light[i]);
		}
	}
}


//=============================================================================
// ���C�g�̐ݒ�
// Type�ɂ���ăZ�b�g���郁���o�[�ϐ����ς���Ă���
//=============================================================================
void SetLightData(int index, LIGHT *light)
{
	SetLight(index, light);
}


LIGHT *GetLightData(int index)
{
	return(&g_Light[index]);
}


//=============================================================================
// �t�H�O�̐ݒ�
//=============================================================================
void SetFogData(FOG *fog)
{
	SetFog(fog);
}


//=============================================================================
// ���C�g�̍X�V
//=============================================================================
void SetDirectionalLight(void)
{
	// �r���[�}�g���b�N�X�ݒ�
	XMMATRIX mtxView;
	mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Light[0].Position), XMLoadFloat3(&g_Light[0].At), XMLoadFloat3(&g_Light[0].up));
	SetLightViewMatrix(&mtxView);
	XMStoreFloat4x4(&g_Light[0].mtxView, mtxView);

	XMMATRIX mtxInvView;
	mtxInvView = XMMatrixInverse(nullptr, mtxView);
	XMStoreFloat4x4(&g_Light[0].mtxInvView, mtxInvView);


	// �v���W�F�N�V�����}�g���b�N�X�ݒ�
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE * 1.2f, VIEW_ASPECT, VIEW_NEAR_Z * 20.0f, VIEW_FAR_Z);

	SetLightProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&g_Light[0].mtxProjection, mtxProjection);
}


BOOL GetFogOnOff(void)
{
	return g_FogSwitch;
}