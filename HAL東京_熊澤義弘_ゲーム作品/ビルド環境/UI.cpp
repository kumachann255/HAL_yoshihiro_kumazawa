//=============================================================================
//
// UI処理 [UI.cpp]
// Author : GP11A132 10 熊澤義弘
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
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(3)				// テクスチャの数

#define MAX_UI						(3)				// メニューの数 + カーソル + HP

#define TEXTURE_HP_SIZE				(20)			// ハートのサイズ
#define TEXTURE_HP_DISTANCE			(30)			// 隣のハートとの幅
#define HP_MAX_X					(5)				// 横に並べる個数

#define CURSOR_OFFSET_X				(0.5f)			// カーソルの振れ幅
#define CURSOR_OFFSET_time			(0.05f)			// カーソルの振れる速度

enum {
	magic,
	option,
	HP,
};


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct MENU
{
	XMFLOAT3			pos;		// 位置

	float				sizeX;		// テクスチャのサイズ
	float				sizeY;		// テクスチャのサイズ
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
	"data/TEXTURE/UI_2.png",
	"data/TEXTURE/UI_3.png",
	"data/TEXTURE/HP.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static MENU						g_UI[MAX_UI];			// メニュー

static float	alpha;
static BOOL	flag_alpha;

static BOOL						g_Load = FALSE;

static int						g_CursorTarget;				// カーソルが何を指しているか
static float					g_time = 0.0f;				// カーソルをsinカーブさせるための時間管理

static int						g_HP;						// HPを保存する

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitUI(void)
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


	// 魔法表の初期化
	g_UI[magic].pos = { 836.0f , 426.0f, 0.0f };
	g_UI[magic].sizeX = 21.5f * 11.0f;
	g_UI[magic].sizeY = 19.8f * 11.0f;

	// オプション表の初期化
	g_UI[option].pos = { 130.0f , 490.0f, 0.0f };
	g_UI[option].sizeX = 18.0f * 12.0f;
	g_UI[option].sizeY = 7.0f * 12.0f;

	// HPの初期化
	g_UI[HP].pos = { 40.0f , 30.0f, 0.0f };
	g_UI[HP].sizeX = TEXTURE_HP_SIZE;
	g_UI[HP].sizeY = TEXTURE_HP_SIZE;



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
	g_Pos   = XMFLOAT3(g_w/3, g_h/4 + 20.0f, 0.0f);
	g_TexNo = 0;
	g_CursorTarget = 90;

	alpha = 1.0f;
	flag_alpha = TRUE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateUI(void)
{
	g_HP = GetHP();


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawUI(void)
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

	// 操作のUIを描画
	for (int i = 0; i < MAX_UI - 1; i++)
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, g_UI[i].pos.x, g_UI[i].pos.y, g_UI[i].sizeX, g_UI[i].sizeY, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(0.8f, 0.8f, 0.8f, alpha));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// HPを描画
	for (int i = 0; i < g_HP; i++)
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[HP]);

		int x = i % HP_MAX_X;
		int y = i / HP_MAX_X;

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, 
			g_UI[HP].pos.x + TEXTURE_HP_DISTANCE * x, 
			g_UI[HP].pos.y + TEXTURE_HP_DISTANCE * y, 
			g_UI[HP].sizeX, g_UI[HP].sizeY, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}
}





