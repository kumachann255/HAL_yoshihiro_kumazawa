//=============================================================================
//
// �^�C�g����ʏ��� [title.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "title.h"
#include "option.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// �w�i�T�C�Y
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(5)				// �e�N�X�`���̐�

#define MAX_MENU					(4)				// ���j���[�̐��{�J�[�\��

#define TEXTURE_WIDTH_LOGO			(480)			// ���S�T�C�Y
#define TEXTURE_HEIGHT_LOGO			(240)			// 

#define CURSOR_OFFSET_X				(0.5f)			// �J�[�\���̐U�ꕝ
#define CURSOR_OFFSET_time			(0.05f)			// �J�[�\���̐U��鑬�x

// ���j���[�̎��
enum {
	start,
	option,
	exit0,
	cursor,
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
	"data/TEXTURE/title_logo.png",
	"data/TEXTURE/title_start.png",
	"data/TEXTURE/title_option.png",
	"data/TEXTURE/title_exit.png",
	"data/TEXTURE/arrowD_red_r.png",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static MENU						g_Menu[MAX_MENU];			// ���j���[

static float	alpha;
static BOOL	flag_alpha;

static BOOL						g_Load = FALSE;

static int						g_CursorTarget;				// �J�[�\���������w���Ă��邩
static float					g_time = 0.0f;				// �J�[�\����sin�J�[�u�����邽�߂̎��ԊǗ�


//=============================================================================
// ����������
//=============================================================================
HRESULT InitTitle(void)
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

	for (int i = 0; i < cursor; i++)
	{
		g_Menu[i].pos = { TEXTURE_WIDTH / 3 , 350.0f + (i * 50.0f), 0.0f };
		g_Menu[i].sizeX = 250.0f;
		g_Menu[i].sizeY = 50.0f;
	}

	// �J�[�\���̏�����
	g_Menu[cursor].pos = { TEXTURE_WIDTH / 3 - 170.0f , 350.0f, 0.0f };
	g_Menu[cursor].sizeX = 40.0f;
	g_Menu[cursor].sizeY = 28.0f;



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

	// BGM�Đ�
	PlaySound(SOUND_LABEL_BGM_title);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTitle(void)
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
void UpdateTitle(void)
{
	// �I���J�[�\���̈ړ�
	if ((GetKeyboardTrigger(DIK_UP)) || (IsButtonTriggered(0, BUTTON_UP)))
	{
		g_CursorTarget--;

		// ���ʉ��Đ�
		PlaySound(SOUND_LABEL_SE_OPTION_cursol_updown);
	}
	if ((GetKeyboardTrigger(DIK_DOWN)) || (IsButtonTriggered(0, BUTTON_DOWN)))
	{
		g_CursorTarget++;

		// ���ʉ��Đ�
		PlaySound(SOUND_LABEL_SE_OPTION_cursol_updown);
	}

	// �����I������Ă��邩���v�Z
	int optionType;
	optionType = g_CursorTarget % (cursor);

	// �I���J�[�\���̈ʒu����
	g_Menu[cursor].pos.y = 345.0f + (optionType * 50.0f);

	// �I���J�[�\����sin�J�[�u������
	g_time += CURSOR_OFFSET_time;
	g_Menu[cursor].pos.x += sinf(g_time) * CURSOR_OFFSET_X;


	// �I���J�[�\���ɉ����ăQ�[���ɖ߂邩�A�^�C�g���ɖ߂邩
	if ((GetKeyboardTrigger(DIK_RETURN)) || (GetKeyboardTrigger(DIK_SPACE)) || 
		(IsButtonTriggered(0, BUTTON_A))   || (IsButtonTriggered(0, BUTTON_START)))
	{
		switch (optionType)
		{
		case start:		// �Q�[���X�^�[�g
			SetFade(FADE_OUT, MODE_GAME);
			SetTitleSoundFade(SOUND_LABEL_BGM_title, SOUNDFADE_OUT, 0.0f, 1.4f);
			break;

		case option:	// �I�v�V����
			SetOption(TRUE);
			break;

		case exit0:		// �Q�[���I��
			exit(-1);
			break;
		}

		// ���ʉ��Đ�
		PlaySound(SOUND_LABEL_SE_OPTION_cursor_enter);
	}


#ifdef _DEBUG	// �f�o�b�O����\������
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTitle(void)
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

	// �^�C�g���̃��S��`��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, g_Pos.x, g_Pos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f,
						XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	// �^�C�g���̃��j���[��`��
	for (int i = 0; i < cursor; i++)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i + 1]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteColor(g_VertexBuffer, g_Menu[i].pos.x, g_Menu[i].pos.y, g_Menu[i].sizeX, g_Menu[i].sizeY, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	// �J�[�\����`��
	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[MAX_MENU]);

	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	SetSpriteColor(g_VertexBuffer, g_Menu[cursor].pos.x, g_Menu[cursor].pos.y, g_Menu[cursor].sizeX, g_Menu[cursor].sizeY, 0.0f, 0.0f, 1.0f, 1.0f,
		XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

	// �|���S���`��
	GetDeviceContext()->Draw(4, 0);
}





