//=============================================================================
//
// �I�v�V�������� [option.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "renderer.h"
#include "option.h"
#include "sprite.h"
#include "sound.h"
#include "fade.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// �w�i�T�C�Y
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(9)		// �e�N�X�`���̐�

#define OPTION_SOUND_SPEED			(0.01f)			// ���ʒ����X�s�[�h
#define OPTION_CURSOR_SPEED			(1.2f)			// �J�[�\���̈ړ��X�s�[�h
#define MAX_VOLUME					(2.0f)			// ���ʂ̍ő�l
#define MIN_VOLUME					(0.0f)			// ���ʂ̍ŏ��l

#define MAX_SOUND_TYPE				(4)				// �ύX�ł���T�E���h�̎��
#define MAX_OPTION_TYPE				(6)				// �I�v�V�����̎��

#define CURSOR_OFFSET_X				(15.0f)			// �J�[�\���̐U�ꕝ
#define CURSOR_OFFSET_time			(0.05f)			// �J�[�\���̐U��鑬�x



//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct OPTION
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
	"data/TEXTURE/fade_black.png",
	"data/TEXTURE/option_master.png",
	"data/TEXTURE/option_bgm.png",
	"data/TEXTURE/option_se.png",
	"data/TEXTURE/option_voice.png",
	"data/TEXTURE/option_game.png",
	"data/TEXTURE/option_title.png",
	"data/TEXTURE/option_cursor_0.png",
	"data/TEXTURE/option_cursor_1.png",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static OPTION					g_Option[TEXTURE_MAX];

static BOOL						g_Load = FALSE;
static BOOL						g_Init = FALSE;				// �N�����̏����Init���𔻒f

static XMFLOAT3					g_CursorPos[MAX_SOUND_TYPE];// �J�[�\���̍��W 0:�}�X�^�[�{�����[�� 1:BGM 2:SE 3:voice
static int						g_CursorTarget;				// �J�[�\���������w���Ă��邩

static float					g_SoundPala[MAX_SOUND_TYPE];	// ���ʂ��Ǘ�  0:�}�X�^�[�{�����[�� 1:BGM 2:SE 3:voice

static float					g_time = 0.0f;				// �J�[�\����sin�J�[�u�����邽�߂̎��ԊǗ�

//=============================================================================
// ����������
//=============================================================================
HRESULT InitOption(void)
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

		g_Option[i].pos = { TEXTURE_WIDTH / 2, 20.0f + 80.0f * i, 0.0f };
		g_Option[i].sizeX = 550.0f;
		g_Option[i].sizeY = 55.0f;
	}

	g_Option[5].sizeX = 275.0f;
	g_Option[5].sizeY = 55.0f;

	g_Option[6].sizeX = 275.0f;
	g_Option[6].sizeY = 55.0f;

	g_Option[8].pos = { TEXTURE_WIDTH / 2 - 320.0f, 105.0f, 0.0f };
	g_Option[8].sizeX = 50.0f;
	g_Option[8].sizeY = 35.0f;

	// �N�����̏��񂾂�����������
	if (g_Init == FALSE)
	{
		for (int i = 0; i < MAX_SOUND_TYPE; i++)
		{
			g_CursorPos[i] = { TEXTURE_WIDTH / 2 + 105.0f, 110.0f + 80.0f * i, 0.0f };
			g_SoundPala[i] = 1.0f;
		}

		g_Init = TRUE;
	}

	// �\�[�X�{�C�X�̏�����
	SetSourceVolume(g_SoundPala[bgm], g_SoundPala[se], g_SoundPala[voice]);

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �I�v�V�����̏�����
	g_Use   = FALSE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos = XMFLOAT3(g_w / 2, g_h / 2, 0.0f);
	g_TexNo = 0;
	g_CursorTarget = 120;
	g_time = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitOption(void)
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
void UpdateOption(void)
{
	if ((GetKeyboardTrigger(DIK_X)) || (IsButtonTriggered(0, BUTTON_Z)))
	{
		// �I���I�t��switch
		if (g_Use) g_Use = FALSE;
		else g_Use = TRUE;
		PlaySound(SOUND_LABEL_SE_OPTION_option_set);
	}

	// �I�v�V�������\������Ă���Ƃ��ɏ������s��
	if (g_Use)
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
		optionType = g_CursorTarget % MAX_OPTION_TYPE;

		// �I���J�[�\���̈ʒu����
		g_Option[8].pos.y = 105.0f + (optionType * 80.0f);
		if (optionType >= MAX_SOUND_TYPE) g_Option[8].pos.x = TEXTURE_WIDTH / 2 - 170.0f;
		else g_Option[8].pos.x = TEXTURE_WIDTH / 2 - 310.0f;

		// �I���J�[�\����sin�J�[�u������
		g_time += CURSOR_OFFSET_time;
		g_Option[8].pos.x += sinf(g_time) * CURSOR_OFFSET_X;

		// �I���J�[�\���ɉ����Ăǂ̃{�����[���𒲐����邩��ύX
		if ((GetKeyboardPress(DIK_RIGHT)) || (IsButtonPressed(0, BUTTON_RIGHT)))
		{
			if ((MAX_VOLUME > g_SoundPala[optionType]) && (optionType < MAX_SOUND_TYPE))
			{	// �ő�l�ɓ��B���Ă��Ȃ��ꍇ
				g_CursorPos[optionType].x += OPTION_CURSOR_SPEED;
				g_SoundPala[optionType] += OPTION_SOUND_SPEED;

				// ���ʉ��Đ�
				PlaySound(SOUND_LABEL_SE_OPTION_cursor_rightleft);
			}
		}
		if ((GetKeyboardPress(DIK_LEFT)) || (IsButtonPressed(0, BUTTON_LEFT)))
		{
			if ((MIN_VOLUME < g_SoundPala[optionType]) && (optionType < MAX_SOUND_TYPE))
			{	// �ŏ��l�ɓ��B���Ă��Ȃ��ꍇ
				g_CursorPos[optionType].x -= OPTION_CURSOR_SPEED;
				g_SoundPala[optionType] -= OPTION_SOUND_SPEED;

				// ���ʉ��Đ�
				PlaySound(SOUND_LABEL_SE_OPTION_cursor_rightleft);
			}
		}

		// �}�X�^�[�{�����[���ƃ\�[�X�{�C�X�̒���
		SetMasterVolume(g_SoundPala[master]);
		SetSourceVolume(g_SoundPala[bgm], g_SoundPala[se], g_SoundPala[voice]);


		// �I���J�[�\���ɉ����ăQ�[���ɖ߂邩�A�^�C�g���ɖ߂邩
		if ((GetKeyboardTrigger(DIK_RETURN)) || (GetKeyboardTrigger(DIK_SPACE)) ||
			(IsButtonTriggered(0, BUTTON_A)))
		{
			switch (optionType)
			{
			case 4:		// �Q�[���ɖ߂�
				g_Use = FALSE;
				break;

			case 5:		// �^�C�g���ɖ߂�
				SetFade(FADE_OUT, MODE_TITLE);
				SetSourceVolume(SOUND_LABEL_SE_PLAYER_gait, 0.0f);		// ����SE����U�������Ȃ��悤�ɒ���
				break;
			}

			// ���ʉ��Đ�
			PlaySound(SOUND_LABEL_SE_OPTION_cursor_enter);
		}
	}
}


//=============================================================================
// �`�揈��
//=============================================================================
void DrawOption(void)
{
	// �I�v�V�������\������Ă��Ȃ��ꍇ�̓X�L�b�v
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

	// �Q�[�����[�h�Ŕw�i�̐F��ς���
	switch (GetMode())
	{
	case MODE_TITLE:
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// �s�����ɂ���
		break;

	case MODE_TUTORIAL:
	case MODE_GAME:
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.4f);	// �������ɂ���
		break;
	}
	SetMaterial(material);

	// �I�v�V�����̔w�i��`��
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

	// �I�v�V������UI��\��
	for (int i = 1; i < TEXTURE_MAX - 2; i++)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSprite(g_VertexBuffer, g_Option[i].pos.x, g_Option[i].pos.y, g_Option[i].sizeX, g_Option[i].sizeY, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	// ���ʃp�����[�^�[�̃J�[�\����\��
	for (int i = 0; i < 4; i++)
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[7]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSprite(g_VertexBuffer, g_CursorPos[i].x, g_CursorPos[i].y, 10.0f, 17.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	// �I���J�[�\����\��
	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[8]);

	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	SetSprite(g_VertexBuffer, g_Option[8].pos.x, g_Option[8].pos.y, g_Option[8].sizeX, g_Option[8].sizeY, 0.0f, 0.0f, 1.0f, 1.0f);

	// �|���S���`��
	GetDeviceContext()->Draw(4, 0);
}


//=============================================================================
// �I�v�V�������\������Ă��邩�ǂ�����Ԃ�
//=============================================================================
BOOL GetOptionUse(void)
{
	return g_Use;
}


//=============================================================================
// �I�v�V������switch
//=============================================================================
void SetOption(BOOL data)
{
	g_Use = data;
	g_CursorTarget = 120; // �J�[�\���̈ʒu�𒲐�
}

//=============================================================================
// �T�E���h�̒��������p�����[�^�[��Ԃ�
//=============================================================================
float SetSoundPala(int soundType)
{
	return g_SoundPala[soundType];
}