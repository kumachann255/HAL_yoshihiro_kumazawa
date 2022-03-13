//=============================================================================
//
// チュートリアル処理 [tutorial.cpp]
// Author : GP11A132 10 熊澤義弘
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
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(9)				// テクスチャの数

#define TEXT_WIDTH					(622.0f)	// テキストサイズ
#define TEXT_HEIGHT					(126.0f)	// 



//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct TUTORIAL
{
	XMFLOAT3			pos;		// 位置

	float				sizeX;		// テクスチャのサイズ
	float				sizeY;		// テクスチャのサイズ
	int					txetNo;		// テキストの番号
	BOOL				end;		// チュートリアルが終了したか
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
	"data/TEXTURE/fade_black.png",
	"data/TEXTURE/text_UI.png",
	"data/TEXTURE/text_start.png",
	"data/TEXTURE/text_fire.png",
	"data/TEXTURE/text_lightning.png",
	"data/TEXTURE/text_water.png",
	"data/TEXTURE/text_holy.png",
	"data/TEXTURE/text_clear.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static BOOL						g_TutorialClear;			// チュートリアルをクリアしているかどうか
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static TUTORIAL					g_Tutorial[tx_MAX - tx_start];

static BOOL						g_Load = FALSE;

static int						g_TutorialNo;
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTutorial(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成 + 初期化
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


	// チュートリアルの初期化
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
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateTutorial(void)
{
	// チュートリアルが表示されているときに処理を行う
	if (g_Use)
	{
		if ((GetKeyboardTrigger(DIK_RETURN)) || (GetKeyboardTrigger(DIK_SPACE)) ||
			(IsButtonTriggered(0, BUTTON_A)))
		{
			// チュートリアル画面を閉じる
			g_Use = FALSE;

			// 最初のチュートリアルはボタンを押すだけでクリア
			if(g_Tutorial[tx_start].end == FALSE) SetTutorialClear(tx_start);

			// 光魔法まで終わったらチュートリアルクリア
			if ((g_Tutorial[tx_holy].end) && (g_Tutorial[tx_clear].end == FALSE)) SetTutorialClear(tx_clear);

			// 効果音再生
			PlaySound(SOUND_LABEL_SE_OPTION_cursor_enter);
		}
	}
}


//=============================================================================
// 描画処理
//=============================================================================
void DrawTutorial(void)
{
	// チュートリアルが表示されていない場合はスキップ
	if (!g_Use) return;

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
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.4f);	// 不透明にする
	SetMaterial(material);

	// チュートリアルの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// 不透明にする
	SetMaterial(material);

	// チュートリアルのUIを表示
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// チュートリアルのテキストを表示
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Tutorial[g_TutorialNo].txetNo]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, 
			g_Tutorial[g_TutorialNo].pos.x, 
			g_Tutorial[g_TutorialNo].pos.y, 
			g_Tutorial[g_TutorialNo].sizeX, 
			g_Tutorial[g_TutorialNo].sizeY, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}
}


//=============================================================================
// チュートリアルが表示されているかどうかを返す
//=============================================================================
BOOL GetTutorialUse(void)
{
	return g_Use;
}

//=============================================================================
// チュートリアル終了しているかどうかを返す
//=============================================================================
BOOL GetTutorialClear(int tutorial)
{
	return g_Tutorial[tutorial].end;
}


//=============================================================================
// チュートリアルの開始
//=============================================================================
void SetTutorial(int tutorial)
{
	// チュートリアルをクリアしている場合は抜ける
	if (tutorial >= tx_MAX) return;

	// そのチュートリアルが終了していない場合に処理
	if (g_Tutorial[tutorial].end == FALSE)
	{
		g_Use = TRUE;
		g_TutorialNo = tutorial;
		PlaySound(SOUND_LABEL_SE_OPTION_option_set);
	}
}


//=============================================================================
// チュートリアルのクリア処理
//=============================================================================
void SetTutorialClear(int tutorial)
{
	g_Tutorial[tutorial].end = TRUE;

	// 次のチュートリアルを開始
	SetTutorial(tutorial + 1);
}