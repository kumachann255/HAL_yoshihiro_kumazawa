//=============================================================================
//
// �z���C�g�t�F�[�h���� [fade_white.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "fade_white.h"
#include "sound.h"
#include "sprite.h"
#include "model.h"
#include "bahamut.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// �w�i�T�C�Y
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(1)				// �e�N�X�`���̐�

#define	FADE_RATE_CHARGE			(0.005f)		// �t�F�[�h�W��
#define	FADE_RATE_BLESS				(0.1f)			// �t�F�[�h�W��
#define	FADE_RATE_BLAST				(0.007f)		// �t�F�[�h�W��
#define	FADE_RATE_IN				(0.02f)			// �t�F�[�h�W��

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/fade_white.png",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

WHITE							g_FadeWhite = WHITE_NONE;	// �t�F�[�h�̏��
static XMFLOAT4					g_Color;					// �t�F�[�h�̃J���[�i���l�j
static BOOL						g_InOut;					// �t�F�[�h���n�܂�����

static BOOL						g_Load = FALSE;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitFadeWhite(void)
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


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �v���C���[�̏�����
	g_Use   = TRUE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = { 0.0f, 0.0f, 0.0f };
	g_TexNo = 0;

	g_FadeWhite  = WHITE_NONE;
	g_Color = { 1.0, 1.0, 1.0, 0.0 };
	g_InOut = FALSE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitFadeWhite(void)
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
void UpdateFadeWhite(void)
{
	if (g_FadeWhite != WHITE_NONE)
	{// �t�F�[�h������
		if (g_FadeWhite == WHITE_OUT_charge)
		{// �t�F�[�h�A�E�g����
			// �ŏ��ɓ����ɂ���
			if (!g_InOut)
			{
				g_Color.w = 0.0f;
				g_InOut = TRUE;
			}

			g_Color.w += FADE_RATE_CHARGE;		// ���l�����Z���ĉ�ʂ������Ă���
			if (g_Color.w >= 1.0f)
			{
				// �t�F�[�h�C�������ɐ؂�ւ�
				g_Color.w = 1.0f;
				SetFadeWhite(WHITE_IN_bless);
			}

		}
		else if (g_FadeWhite == WHITE_OUT_blast)
		{// �t�F�[�h�A�E�g����
			g_Color.w += FADE_RATE_BLAST;		// ���l�����Z���ĉ�ʂ������Ă���
			if (g_Color.w >= 1.0f)
			{
				// �t�F�[�h�C�������ɐ؂�ւ�
				g_Color.w = 1.0f;
				SetFadeWhite(WHITE_IN_bless);

				// �������@���I��������
				ResetBahamut();
			}

		}
		else if (g_FadeWhite == WHITE_IN_bless)
		{// �t�F�[�h�C������
			// �ŏ��ɐ^�����ɂ���
			if (!g_InOut)
			{
				g_Color.w = 1.0f;
				g_InOut = TRUE;
			}

			g_Color.w -= FADE_RATE_IN;			// ���l�����Z���ĉ�ʂ𕂂��オ�点��
			if (g_Color.w <= 0.0f)
			{
				// �t�F�[�h�����I��
				g_Color.w = 0.0f;
				SetFadeWhite(WHITE_NONE);
				g_InOut = FALSE;
			}
		}
	}


#ifdef _DEBUG	// �f�o�b�O����\������
	// PrintDebugProc("\n");

#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawFadeWhite(void)
{
	if (g_FadeWhite == WHITE_NONE) return;	// �t�F�[�h���Ȃ��̂Ȃ�`�悵�Ȃ�

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);


	// �^�C�g���̔w�i��`��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		//SetVertex(0.0f, 0.0f, SCREEN_WIDTH, TEXTURE_WIDTH, 0.0f, 0.0f, 1.0f, 1.0f);
		SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH/2, TEXTURE_WIDTH/2, SCREEN_WIDTH, TEXTURE_WIDTH, 0.0f, 0.0f, 1.0f, 1.0f,
			g_Color);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}



}


//=============================================================================
// �t�F�[�h�̏�Ԑݒ�
//=============================================================================
void SetFadeWhite(WHITE fade)
{
	g_FadeWhite = fade;
}

//=============================================================================
// �t�F�[�h�̏�Ԏ擾
//=============================================================================
WHITE GetFadeWhite(void)
{
	return g_FadeWhite;
}


//=============================================================================
// �t�F�[�h�̃��Z�b�g
//=============================================================================
void ResetInOut(void)
{
	g_InOut = FALSE;
}