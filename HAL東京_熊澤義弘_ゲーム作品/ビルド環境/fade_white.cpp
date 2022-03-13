//=============================================================================
//
// ホワイトフェード処理 [fade_white.cpp]
// Author : GP11A132 10 熊澤義弘
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
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(1)				// テクスチャの数

#define	FADE_RATE_CHARGE			(0.005f)		// フェード係数
#define	FADE_RATE_BLESS				(0.1f)			// フェード係数
#define	FADE_RATE_BLAST				(0.007f)		// フェード係数
#define	FADE_RATE_IN				(0.02f)			// フェード係数

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/fade_white.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

WHITE							g_FadeWhite = WHITE_NONE;	// フェードの状態
static XMFLOAT4					g_Color;					// フェードのカラー（α値）
static BOOL						g_InOut;					// フェードが始まったか

static BOOL						g_Load = FALSE;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitFadeWhite(void)
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


	// プレイヤーの初期化
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
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateFadeWhite(void)
{
	if (g_FadeWhite != WHITE_NONE)
	{// フェード処理中
		if (g_FadeWhite == WHITE_OUT_charge)
		{// フェードアウト処理
			// 最初に透明にする
			if (!g_InOut)
			{
				g_Color.w = 0.0f;
				g_InOut = TRUE;
			}

			g_Color.w += FADE_RATE_CHARGE;		// α値を加算して画面を消していく
			if (g_Color.w >= 1.0f)
			{
				// フェードイン処理に切り替え
				g_Color.w = 1.0f;
				SetFadeWhite(WHITE_IN_bless);
			}

		}
		else if (g_FadeWhite == WHITE_OUT_blast)
		{// フェードアウト処理
			g_Color.w += FADE_RATE_BLAST;		// α値を加算して画面を消していく
			if (g_Color.w >= 1.0f)
			{
				// フェードイン処理に切り替え
				g_Color.w = 1.0f;
				SetFadeWhite(WHITE_IN_bless);

				// 召喚魔法を終了させる
				ResetBahamut();
			}

		}
		else if (g_FadeWhite == WHITE_IN_bless)
		{// フェードイン処理
			// 最初に真っ白にする
			if (!g_InOut)
			{
				g_Color.w = 1.0f;
				g_InOut = TRUE;
			}

			g_Color.w -= FADE_RATE_IN;			// α値を減算して画面を浮き上がらせる
			if (g_Color.w <= 0.0f)
			{
				// フェード処理終了
				g_Color.w = 0.0f;
				SetFadeWhite(WHITE_NONE);
				g_InOut = FALSE;
			}
		}
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	// PrintDebugProc("\n");

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawFadeWhite(void)
{
	if (g_FadeWhite == WHITE_NONE) return;	// フェードしないのなら描画しない

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);


	// タイトルの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		//SetVertex(0.0f, 0.0f, SCREEN_WIDTH, TEXTURE_WIDTH, 0.0f, 0.0f, 1.0f, 1.0f);
		SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH/2, TEXTURE_WIDTH/2, SCREEN_WIDTH, TEXTURE_WIDTH, 0.0f, 0.0f, 1.0f, 1.0f,
			g_Color);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}



}


//=============================================================================
// フェードの状態設定
//=============================================================================
void SetFadeWhite(WHITE fade)
{
	g_FadeWhite = fade;
}

//=============================================================================
// フェードの状態取得
//=============================================================================
WHITE GetFadeWhite(void)
{
	return g_FadeWhite;
}


//=============================================================================
// フェードのリセット
//=============================================================================
void ResetInOut(void)
{
	g_InOut = FALSE;
}