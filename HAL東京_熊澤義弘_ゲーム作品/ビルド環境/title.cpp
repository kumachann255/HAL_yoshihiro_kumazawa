//=============================================================================
//
// タイトル画面処理 [title.cpp]
// Author : GP11A132 10 熊澤義弘
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
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(5)				// テクスチャの数

#define MAX_MENU					(4)				// メニューの数＋カーソル

#define TEXTURE_WIDTH_LOGO			(480)			// ロゴサイズ
#define TEXTURE_HEIGHT_LOGO			(240)			// 

#define CURSOR_OFFSET_X				(0.5f)			// カーソルの振れ幅
#define CURSOR_OFFSET_time			(0.05f)			// カーソルの振れる速度

// メニューの種類
enum {
	start,
	option,
	exit0,
	cursor,
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
	"data/TEXTURE/title_logo.png",
	"data/TEXTURE/title_start.png",
	"data/TEXTURE/title_option.png",
	"data/TEXTURE/title_exit.png",
	"data/TEXTURE/arrowD_red_r.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static MENU						g_Menu[MAX_MENU];			// メニュー

static float	alpha;
static BOOL	flag_alpha;

static BOOL						g_Load = FALSE;

static int						g_CursorTarget;				// カーソルが何を指しているか
static float					g_time = 0.0f;				// カーソルをsinカーブさせるための時間管理


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTitle(void)
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

	for (int i = 0; i < cursor; i++)
	{
		g_Menu[i].pos = { TEXTURE_WIDTH / 3 , 350.0f + (i * 50.0f), 0.0f };
		g_Menu[i].sizeX = 250.0f;
		g_Menu[i].sizeY = 50.0f;
	}

	// カーソルの初期化
	g_Menu[cursor].pos = { TEXTURE_WIDTH / 3 - 170.0f , 350.0f, 0.0f };
	g_Menu[cursor].sizeX = 40.0f;
	g_Menu[cursor].sizeY = 28.0f;



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

	// BGM再生
	PlaySound(SOUND_LABEL_BGM_title);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateTitle(void)
{
	// 選択カーソルの移動
	if ((GetKeyboardTrigger(DIK_UP)) || (IsButtonTriggered(0, BUTTON_UP)))
	{
		g_CursorTarget--;

		// 効果音再生
		PlaySound(SOUND_LABEL_SE_OPTION_cursol_updown);
	}
	if ((GetKeyboardTrigger(DIK_DOWN)) || (IsButtonTriggered(0, BUTTON_DOWN)))
	{
		g_CursorTarget++;

		// 効果音再生
		PlaySound(SOUND_LABEL_SE_OPTION_cursol_updown);
	}

	// 何が選択されているかを計算
	int optionType;
	optionType = g_CursorTarget % (cursor);

	// 選択カーソルの位置調整
	g_Menu[cursor].pos.y = 345.0f + (optionType * 50.0f);

	// 選択カーソルをsinカーブさせる
	g_time += CURSOR_OFFSET_time;
	g_Menu[cursor].pos.x += sinf(g_time) * CURSOR_OFFSET_X;


	// 選択カーソルに応じてゲームに戻るか、タイトルに戻るか
	if ((GetKeyboardTrigger(DIK_RETURN)) || (GetKeyboardTrigger(DIK_SPACE)) || 
		(IsButtonTriggered(0, BUTTON_A))   || (IsButtonTriggered(0, BUTTON_START)))
	{
		switch (optionType)
		{
		case start:		// ゲームスタート
			SetFade(FADE_OUT, MODE_GAME);
			SetTitleSoundFade(SOUND_LABEL_BGM_title, SOUNDFADE_OUT, 0.0f, 1.4f);
			break;

		case option:	// オプション
			SetOption(TRUE);
			break;

		case exit0:		// ゲーム終了
			exit(-1);
			break;
		}

		// 効果音再生
		PlaySound(SOUND_LABEL_SE_OPTION_cursor_enter);
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTitle(void)
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

	// タイトルのロゴを描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, g_Pos.x, g_Pos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f,
						XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// タイトルのメニューを描画
	for (int i = 0; i < cursor; i++)
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i + 1]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, g_Menu[i].pos.x, g_Menu[i].pos.y, g_Menu[i].sizeX, g_Menu[i].sizeY, 0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// カーソルを描画
	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[MAX_MENU]);

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSpriteColor(g_VertexBuffer, g_Menu[cursor].pos.x, g_Menu[cursor].pos.y, g_Menu[cursor].sizeX, g_Menu[cursor].sizeY, 0.0f, 0.0f, 1.0f, 1.0f,
		XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);
}





