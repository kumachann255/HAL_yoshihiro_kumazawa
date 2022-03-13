//=============================================================================
//
// リザルト画面処理 [result.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "result.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "score.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(2)				// テクスチャの数


enum {
	clear,
	gameover,
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/clear.png",
	"data/TEXTURE/game_over.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static BOOL						g_Load = FALSE;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitResult(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
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


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// 変数の初期化
	g_Use   = TRUE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos   = { g_w / 2, 50.0f, 0.0f };

	switch (GetMode())
	{
	case MODE_CLEAR:
		g_TexNo = clear;

		// BGM再生
		PlaySound(SOUND_LABEL_BGM_clear);
		break;

	case MODE_OVER:
		g_TexNo = gameover;

		// BGM再生
		PlaySound(SOUND_LABEL_BGM_game_over);
		break;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitResult(void)
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
// 更新処理
//=============================================================================
void UpdateResult(void)
{
	// 現在のモードに応じて切り換え先のモードを変更
	if (GetKeyboardTrigger(DIK_RETURN))
	{// Enter押したら、ステージを切り替える
		if (GetMode() == MODE_OVER)
		{
			SetFade(FADE_OUT, MODE_TITLE);
			SetTitleSoundFade(SOUND_LABEL_BGM_game_over, SOUNDFADE_OUT, 0.0f, 1.4f);
		}
		else
		{
			SetFade(FADE_OUT, MODE_ENDROLL);
			SetTitleSoundFade(SOUND_LABEL_BGM_clear, SOUNDFADE_OUT, 0.0f, 1.4f);
		}
	}
	// ゲームパッドで入力処理
	else if (IsButtonTriggered(0, BUTTON_START))
	{
		if (GetMode() == MODE_OVER)
		{
			SetFade(FADE_OUT, MODE_TITLE);
			SetTitleSoundFade(SOUND_LABEL_BGM_game_over, SOUNDFADE_OUT, 0.0f, 1.4f);
		}
		else
		{
			SetFade(FADE_OUT, MODE_ENDROLL);
			SetTitleSoundFade(SOUND_LABEL_BGM_clear, SOUNDFADE_OUT, 0.0f, 1.4f);
		}
	}
	else if (IsButtonTriggered(0, BUTTON_A))
	{
		if (GetMode() == MODE_OVER)
		{
			SetFade(FADE_OUT, MODE_TITLE);
			SetTitleSoundFade(SOUND_LABEL_BGM_game_over, SOUNDFADE_OUT, 0.0f, 1.4f);
		}
		else
		{
			SetFade(FADE_OUT, MODE_ENDROLL);
			SetTitleSoundFade(SOUND_LABEL_BGM_clear, SOUNDFADE_OUT, 0.0f, 1.4f);
		}
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawResult(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// リザルトの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLeftTop(g_VertexBuffer, 0.0f, 0.0f, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}
}




