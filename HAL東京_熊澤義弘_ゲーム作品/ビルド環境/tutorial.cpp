//=============================================================================
//
// �`���[�g���A������ [tutorial.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "renderer.h"
#include "tutorial.h"
#include "sprite.h"
#include "sound.h"
#include "fade.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// �w�i�T�C�Y
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(9)				// �e�N�X�`���̐�

#define TEXT_WIDTH					(622.0f)	// �e�L�X�g�T�C�Y
#define TEXT_HEIGHT					(126.0f)	// 



//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct TUTORIAL
{
	XMFLOAT3			pos;		// �ʒu

	float				sizeX;		// �e�N�X�`���̃T�C�Y
	float				sizeY;		// �e�N�X�`���̃T�C�Y
	int					txetNo;		// �e�L�X�g�̔ԍ�
	BOOL				end;		// �`���[�g���A�����I��������
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
	"data/TEXTURE/fade_black.png",
	"data/TEXTURE/text_UI.png",
	"data/TEXTURE/text_start.png",
	"data/TEXTURE/text_fire.png",
	"data/TEXTURE/text_lightning.png",
	"data/TEXTURE/text_water.png",
	"data/TEXTURE/text_holy.png",
	"data/TEXTURE/text_clear.png",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static BOOL						g_TutorialClear;			// �`���[�g���A�����N���A���Ă��邩�ǂ���
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static TUTORIAL					g_Tutorial[tx_MAX - tx_start];

static BOOL						g_Load = FALSE;

static int						g_TutorialNo;
//=============================================================================
// ����������
//=============================================================================
HRESULT InitTutorial(void)
{
	ID3D11Device *pDevice = GetDevice();

	//�e�N�X�`������ + ������
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


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �`���[�g���A���̏�����
	g_Use   = FALSE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos = XMFLOAT3(g_w / 2, g_h / 2, 0.0f);
	g_TexNo = 0;
	g_TutorialClear = FALSE;
	g_TutorialNo = tx_start - 1;

	for (int i = 0; i < tx_MAX; i++)
	{
		g_Tutorial[i].pos = { TEXTURE_WIDTH / 2 , 120.0f, 0.0f };
		g_Tutorial[i].sizeX = TEXT_WIDTH;
		g_Tutorial[i].sizeY = TEXT_HEIGHT;
		g_Tutorial[i].txetNo = tx_start + 2 + i;
		g_Tutorial[i].end = FALSE;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTutorial(void)
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
void UpdateTutorial(void)
{
	// �`���[�g���A�����\������Ă���Ƃ��ɏ������s��
	if (g_Use)
	{
		if ((GetKeyboardTrigger(DIK_RETURN)) || (GetKeyboardTrigger(DIK_SPACE)) ||
			(IsButtonTriggered(0, BUTTON_A)))
		{
			// �`���[�g���A����ʂ����
			g_Use = FALSE;

			// �ŏ��̃`���[�g���A���̓{�^�������������ŃN���A
			if(g_Tutorial[tx_start].end == FALSE) SetTutorialClear(tx_start);

			// �����@�܂ŏI�������`���[�g���A���N���A
			if ((g_Tutorial[tx_holy].end) && (g_Tutorial[tx_clear].end == FALSE)) SetTutorialClear(tx_clear);

			// ���ʉ��Đ�
			PlaySound(SOUND_LABEL_SE_OPTION_cursor_enter);
		}
	}
}


//=============================================================================
// �`�揈��
//=============================================================================
void DrawTutorial(void)
{
	// �`���[�g���A�����\������Ă��Ȃ��ꍇ�̓X�L�b�v
	if (!g_Use) return;

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
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.4f);	// �s�����ɂ���
	SetMaterial(material);

	// �`���[�g���A���̔w�i��`��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// �s�����ɂ���
	SetMaterial(material);

	// �`���[�g���A����UI��\��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	// �`���[�g���A���̃e�L�X�g��\��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Tutorial[g_TutorialNo].txetNo]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSprite(g_VertexBuffer, 
			g_Tutorial[g_TutorialNo].pos.x, 
			g_Tutorial[g_TutorialNo].pos.y, 
			g_Tutorial[g_TutorialNo].sizeX, 
			g_Tutorial[g_TutorialNo].sizeY, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}
}


//=============================================================================
// �`���[�g���A�����\������Ă��邩�ǂ�����Ԃ�
//=============================================================================
BOOL GetTutorialUse(void)
{
	return g_Use;
}

//=============================================================================
// �`���[�g���A���I�����Ă��邩�ǂ�����Ԃ�
//=============================================================================
BOOL GetTutorialClear(int tutorial)
{
	return g_Tutorial[tutorial].end;
}


//=============================================================================
// �`���[�g���A���̊J�n
//=============================================================================
void SetTutorial(int tutorial)
{
	// �`���[�g���A�����N���A���Ă���ꍇ�͔�����
	if (tutorial >= tx_MAX) return;

	// ���̃`���[�g���A�����I�����Ă��Ȃ��ꍇ�ɏ���
	if (g_Tutorial[tutorial].end == FALSE)
	{
		g_Use = TRUE;
		g_TutorialNo = tutorial;
		PlaySound(SOUND_LABEL_SE_OPTION_option_set);
	}
}


//=============================================================================
// �`���[�g���A���̃N���A����
//=============================================================================
void SetTutorialClear(int tutorial)
{
	g_Tutorial[tutorial].end = TRUE;

	// ���̃`���[�g���A�����J�n
	SetTutorial(tutorial + 1);
}