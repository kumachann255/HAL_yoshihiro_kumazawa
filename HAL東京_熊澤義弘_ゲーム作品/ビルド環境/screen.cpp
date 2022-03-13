//=============================================================================
//
// スクリーン処理 [screen.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "screen.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報

static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標

static float	alpha;
static BOOL	flag_alpha;

static BOOL						g_Load = FALSE;

static BOOL						g_Bloom = TRUE;
static float					Gauss = -0.05f;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitScreen(void)
{
	ID3D11Device *pDevice = GetDevice();

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
	g_Pos   = XMFLOAT3(g_w/2, g_h/2, 0.0f);

	alpha = 1.0f;
	flag_alpha = TRUE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitScreen(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	g_Load = FALSE;
}


//=============================================================================
// 描画処理
//=============================================================================
void UpdateScreen(void)
{
	// ガウスの重み調整
	if (GetKeyboardPress(DIK_W))
	{	// 大きく
		Gauss *= 0.5f;
	}
	if (GetKeyboardPress(DIK_S))
	{	// 小さく
		Gauss *= 1.5f;
	}


	// ブルームのOn/Off
	if (GetKeyboardTrigger(DIK_L))
	{
		if (g_Bloom)
		{
			g_Bloom = FALSE;
		}
		else
		{
			g_Bloom = TRUE;
		}
	}
}



//=============================================================================
// 描画処理
//=============================================================================
void DrawScreen(void)
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

	// スクリーンの背景を描画
	{
		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}
}


//=============================================================================
// ガウスの重みを返す
//=============================================================================
float GetGausee(void)
{
	return Gauss;
}


//=============================================================================
// ブルームのOn/Off
//=============================================================================
BOOL GetBloom(void)
{
	return g_Bloom;
}