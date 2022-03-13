//=============================================================================
//
// UI���� [UI.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "UI.h"
#include "option.h"
#include "model.h"
#include "player.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// �w�i�T�C�Y
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(3)				// �e�N�X�`���̐�

#define MAX_UI						(3)				// ���j���[�̐� + �J�[�\�� + HP

#define TEXTURE_HP_SIZE				(20)			// �n�[�g�̃T�C�Y
#define TEXTURE_HP_DISTANCE			(30)			// �ׂ̃n�[�g�Ƃ̕�
#define HP_MAX_X					(5)				// ���ɕ��ׂ��

#define CURSOR_OFFSET_X				(0.5f)			// �J�[�\���̐U�ꕝ
#define CURSOR_OFFSET_time			(0.05f)			// �J�[�\���̐U��鑬�x

enum {
	magic,
	option,
	HP,
};


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct MENU
{
	XMFLOAT3			pos;		// �ʒu

	float				sizeX;		// �e�N�X�`���̃T�C�Y
	float				sizeY;		// �e�N�X�`���̃T�C�Y
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/UI_2.png",
	"data/TEXTURE/UI_3.png",
	"data/TEXTURE/HP.png",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static MENU						g_UI[MAX_UI];			// ���j���[

static float	alpha;
static BOOL	flag_alpha;

static BOOL						g_Load = FALSE;

static int						g_CursorTarget;				// �J�[�\���������w���Ă��邩
static float					g_time = 0.0f;				// �J�[�\����sin�J�[�u�����邽�߂̎��ԊǗ�

static int						g_HP;						// HP��ۑ�����

//=============================================================================
// ����������
//=============================================================================
HRESULT InitUI(void)
{
	ID3D11Device *pDevice = GetDevice();

	//�e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// ���@�\�̏�����
	g_UI[magic].pos = { 836.0f , 426.0f, 0.0f };
	g_UI[magic].sizeX = 21.5f * 11.0f;
	g_UI[magic].sizeY = 19.8f * 11.0f;

	// �I�v�V�����\�̏�����
	g_UI[option].pos = { 130.0f , 490.0f, 0.0f };
	g_UI[option].sizeX = 18.0f * 12.0f;
	g_UI[option].sizeY = 7.0f * 12.0f;

	// HP�̏�����
	g_UI[HP].pos = { 40.0f , 30.0f, 0.0f };
	g_UI[HP].sizeX = TEXTURE_HP_SIZE;
	g_UI[HP].sizeY = TEXTURE_HP_SIZE;



	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �ϐ��̏�����
	g_Use   = TRUE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = XMFLOAT3(g_w/3, g_h/4 + 20.0f, 0.0f);
	g_TexNo = 0;
	g_CursorTarget = 90;

	alpha = 1.0f;
	flag_alpha = TRUE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitUI(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateUI(void)
{
	g_HP = GetHP();


#ifdef _DEBUG	// �f�o�b�O����\������
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawUI(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// �����UI��`��
	for (int i = 0; i < MAX_UI - 1; i++)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, g_UI[i].pos.x, g_UI[i].pos.y, g_UI[i].sizeX, g_UI[i].sizeY, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(0.8f, 0.8f, 0.8f, alpha));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	// HP��`��
	for (int i = 0; i < g_HP; i++)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[HP]);

		int x = i % HP_MAX_X;
		int y = i / HP_MAX_X;

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, 
			g_UI[HP].pos.x + TEXTURE_HP_DISTANCE * x, 
			g_UI[HP].pos.y + TEXTURE_HP_DISTANCE * y, 
			g_UI[HP].sizeX, g_UI[HP].sizeY, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}
}





