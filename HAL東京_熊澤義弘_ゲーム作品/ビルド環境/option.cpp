//=============================================================================
//
// オプション処理 [option.cpp]
// Author : GP11A132 10 熊澤義弘
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
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(9)		// テクスチャの数

#define OPTION_SOUND_SPEED			(0.01f)			// 音量調整スピード
#define OPTION_CURSOR_SPEED			(1.2f)			// カーソルの移動スピード
#define MAX_VOLUME					(2.0f)			// 音量の最大値
#define MIN_VOLUME					(0.0f)			// 音量の最小値

#define MAX_SOUND_TYPE				(4)				// 変更できるサウンドの種類
#define MAX_OPTION_TYPE				(6)				// オプションの種類

#define CURSOR_OFFSET_X				(15.0f)			// カーソルの振れ幅
#define CURSOR_OFFSET_time			(0.05f)			// カーソルの振れる速度



//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct OPTION
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


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static OPTION					g_Option[TEXTURE_MAX];

static BOOL						g_Load = FALSE;
static BOOL						g_Init = FALSE;				// 起動時の初回のInitかを判断

static XMFLOAT3					g_CursorPos[MAX_SOUND_TYPE];// カーソルの座標 0:マスターボリューム 1:BGM 2:SE 3:voice
static int						g_CursorTarget;				// カーソルが何を指しているか

static float					g_SoundPala[MAX_SOUND_TYPE];	// 音量を管理  0:マスターボリューム 1:BGM 2:SE 3:voice

static float					g_time = 0.0f;				// カーソルをsinカーブさせるための時間管理

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitOption(void)
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

	// 起動時の初回だけ初期化する
	if (g_Init == FALSE)
	{
		for (int i = 0; i < MAX_SOUND_TYPE; i++)
		{
			g_CursorPos[i] = { TEXTURE_WIDTH / 2 + 105.0f, 110.0f + 80.0f * i, 0.0f };
			g_SoundPala[i] = 1.0f;
		}

		g_Init = TRUE;
	}

	// ソースボイスの初期化
	SetSourceVolume(g_SoundPala[bgm], g_SoundPala[se], g_SoundPala[voice]);

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// オプションの初期化
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
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateOption(void)
{
	if ((GetKeyboardTrigger(DIK_X)) || (IsButtonTriggered(0, BUTTON_Z)))
	{
		// オンオフのswitch
		if (g_Use) g_Use = FALSE;
		else g_Use = TRUE;
		PlaySound(SOUND_LABEL_SE_OPTION_option_set);
	}

	// オプションが表示されているときに処理を行う
	if (g_Use)
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
		optionType = g_CursorTarget % MAX_OPTION_TYPE;

		// 選択カーソルの位置調整
		g_Option[8].pos.y = 105.0f + (optionType * 80.0f);
		if (optionType >= MAX_SOUND_TYPE) g_Option[8].pos.x = TEXTURE_WIDTH / 2 - 170.0f;
		else g_Option[8].pos.x = TEXTURE_WIDTH / 2 - 310.0f;

		// 選択カーソルをsinカーブさせる
		g_time += CURSOR_OFFSET_time;
		g_Option[8].pos.x += sinf(g_time) * CURSOR_OFFSET_X;

		// 選択カーソルに応じてどのボリュームを調整するかを変更
		if ((GetKeyboardPress(DIK_RIGHT)) || (IsButtonPressed(0, BUTTON_RIGHT)))
		{
			if ((MAX_VOLUME > g_SoundPala[optionType]) && (optionType < MAX_SOUND_TYPE))
			{	// 最大値に到達していない場合
				g_CursorPos[optionType].x += OPTION_CURSOR_SPEED;
				g_SoundPala[optionType] += OPTION_SOUND_SPEED;

				// 効果音再生
				PlaySound(SOUND_LABEL_SE_OPTION_cursor_rightleft);
			}
		}
		if ((GetKeyboardPress(DIK_LEFT)) || (IsButtonPressed(0, BUTTON_LEFT)))
		{
			if ((MIN_VOLUME < g_SoundPala[optionType]) && (optionType < MAX_SOUND_TYPE))
			{	// 最小値に到達していない場合
				g_CursorPos[optionType].x -= OPTION_CURSOR_SPEED;
				g_SoundPala[optionType] -= OPTION_SOUND_SPEED;

				// 効果音再生
				PlaySound(SOUND_LABEL_SE_OPTION_cursor_rightleft);
			}
		}

		// マスターボリュームとソースボイスの調整
		SetMasterVolume(g_SoundPala[master]);
		SetSourceVolume(g_SoundPala[bgm], g_SoundPala[se], g_SoundPala[voice]);


		// 選択カーソルに応じてゲームに戻るか、タイトルに戻るか
		if ((GetKeyboardTrigger(DIK_RETURN)) || (GetKeyboardTrigger(DIK_SPACE)) ||
			(IsButtonTriggered(0, BUTTON_A)))
		{
			switch (optionType)
			{
			case 4:		// ゲームに戻る
				g_Use = FALSE;
				break;

			case 5:		// タイトルに戻る
				SetFade(FADE_OUT, MODE_TITLE);
				SetSourceVolume(SOUND_LABEL_SE_PLAYER_gait, 0.0f);		// 走るSEを一旦聞こえないように調整
				break;
			}

			// 効果音再生
			PlaySound(SOUND_LABEL_SE_OPTION_cursor_enter);
		}
	}
}


//=============================================================================
// 描画処理
//=============================================================================
void DrawOption(void)
{
	// オプションが表示されていない場合はスキップ
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

	// ゲームモードで背景の色を変える
	switch (GetMode())
	{
	case MODE_TITLE:
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// 不透明にする
		break;

	case MODE_TUTORIAL:
	case MODE_GAME:
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.4f);	// 半透明にする
		break;
	}
	SetMaterial(material);

	// オプションの背景を描画
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

	// オプションのUIを表示
	for (int i = 1; i < TEXTURE_MAX - 2; i++)
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_Option[i].pos.x, g_Option[i].pos.y, g_Option[i].sizeX, g_Option[i].sizeY, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// 音量パラメーターのカーソルを表示
	for (int i = 0; i < 4; i++)
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[7]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_CursorPos[i].x, g_CursorPos[i].y, 10.0f, 17.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// 選択カーソルを表示
	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[8]);

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSprite(g_VertexBuffer, g_Option[8].pos.x, g_Option[8].pos.y, g_Option[8].sizeX, g_Option[8].sizeY, 0.0f, 0.0f, 1.0f, 1.0f);

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);
}


//=============================================================================
// オプションが表示されているかどうかを返す
//=============================================================================
BOOL GetOptionUse(void)
{
	return g_Use;
}


//=============================================================================
// オプションのswitch
//=============================================================================
void SetOption(BOOL data)
{
	g_Use = data;
	g_CursorTarget = 120; // カーソルの位置を調整
}

//=============================================================================
// サウンドの調整したパラメーターを返す
//=============================================================================
float SetSoundPala(int soundType)
{
	return g_SoundPala[soundType];
}