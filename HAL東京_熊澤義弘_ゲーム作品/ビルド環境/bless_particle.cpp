//=============================================================================
//
// ブレスのパーティクル処理 [bless_particle.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "bless_particle.h"
#include "bahamut_bless.h"
#include "player.h"
#include "holy.h"
#include "bahamut.h"
#include "lightning.h"
#include "debugproc.h"
#include "fade_white.h"
#include "sound.h"
#include "beam_particle.h"
#include "beam_particle_2.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX			(1)			// テクスチャの数

#define	BLESS_PARTICLE_SIZE_X		(5.0f)		// 頂点サイズ
#define	BLESS_PARTICLE_SIZE_Y		(5.0f)		// 頂点サイズ
#define	VALUE_MOVE_BLESS_PARTICLE	(0.2f)		// 移動速度

#define	MAX_BLESS_PARTICLE		(512 * 4)		// パーティクル最大数

#define BLESS_PARTICLE_BASE			(30)		// バハムート中心からどのくらい離して発生させるか
#define BLESS_PARTICLE_OFFSET		(60)		// 上記の乱数

#define BLESS_SPEED					(10)		// ブレスパーティクルの最低速度(目標地点までの到達フレーム数)
#define BLESS_1F_MAX				(40)		// 1フレームで出すパーティクルの数

#define BLESS_TIME_MAX				(280)		// パーティクルを発生させている時間
#define BLESS_TIME_START			(60)		// チャージパーティクを発生させ始める時間

#define	DISP_SHADOW						// 影の表示
//#undef DISP_SHADOW

//*****************************************************************************
// 構造体定義
//*****************************************************************************
typedef struct
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	XMFLOAT3		move;			// 移動量
	MATERIAL		material;		// マテリアル
	float			fSizeX;			// 幅
	float			fSizeY;			// 高さ
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか

} PARTICLE;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexBlessParticle(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static PARTICLE					g_aBlessParticle[MAX_BLESS_PARTICLE];		// パーティクルワーク
static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 1.0f;			// 基準の幅
static float					g_fHeightBase = 1.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード
static int						g_time;							// パーティクルを発生させている時間を管理

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/effect000.jpg",
};

static BOOL						g_Load = FALSE;
static BOOL						g_On = FALSE;				// パーティクルを発生さているか
static BOOL						g_End = TRUE;				// パーティクルを発生させたか


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBlessParticle(void)
{
	// 頂点情報の作成
	MakeVertexBlessParticle();

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	g_TexNo = 0;

	// パーティクルワークの初期化
	for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	{
		g_aBlessParticle[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBlessParticle[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBlessParticle[nCntParticle].scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
		g_aBlessParticle[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_aBlessParticle[nCntParticle].material, sizeof(g_aBlessParticle[nCntParticle].material));
		g_aBlessParticle[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aBlessParticle[nCntParticle].fSizeX = BLESS_PARTICLE_SIZE_X;
		g_aBlessParticle[nCntParticle].fSizeY = BLESS_PARTICLE_SIZE_Y;
		g_aBlessParticle[nCntParticle].nLife = 0;
		g_aBlessParticle[nCntParticle].bUse = FALSE;
	}

	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;
	g_time = 0;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBlessParticle(void)
{
	if (g_Load == FALSE) return;

	//テクスチャの解放
	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	// 頂点バッファの解放
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateBlessParticle(void)
{
	BAHAMUT *bahamut = GetBahamut();

	for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	{
		if (g_aBlessParticle[nCntParticle].bUse)
		{// 使用中
			g_aBlessParticle[nCntParticle].pos.x += g_aBlessParticle[nCntParticle].move.x;
			g_aBlessParticle[nCntParticle].pos.z += g_aBlessParticle[nCntParticle].move.z;
			g_aBlessParticle[nCntParticle].pos.y += g_aBlessParticle[nCntParticle].move.y;

			g_aBlessParticle[nCntParticle].nLife--;
			if ((g_aBlessParticle[nCntParticle].nLife <= 0))
			{
				g_aBlessParticle[nCntParticle].bUse = FALSE;
			}
		}
	}


	// パーティクル発生
	if((g_On) && (!g_End))
	{
		for (int i = 0; i < BLESS_1F_MAX; i++)
		{
			XMFLOAT3 pos, pos_old;

			// ブレスの設定
			pos = bahamut->pos;
			pos.y += 125.0f;
			pos.x += 85.0f;

			pos_old = pos;

			XMFLOAT3 move;
			XMFLOAT3 color;
			float fAngle, fLength;
			int nLife;
			float fSize;
			int speed;


			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			fLength = (float)(rand() % BLESS_PARTICLE_OFFSET + BLESS_PARTICLE_BASE);
			pos.x += sinf(fAngle) * fLength;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			pos.y += cosf(fAngle + (XM_PI / 2)) * fLength;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			pos.z += cosf(fAngle) * fLength;

			speed = rand() % BLESS_SPEED + BLESS_SPEED;
			nLife = speed;
			move.x = (pos_old.x - pos.x) / (float)speed;
			move.y = (pos_old.y - pos.y) / (float)speed;
			move.z = (pos_old.z - pos.z) / (float)speed;

			color.x = RamdomFloat(3, 1.0f, 0.2f);
			color.y = RamdomFloat(3, 1.0f, 0.2f);
			color.z = RamdomFloat(3, 1.0f, 0.9f);

			fSize = RamdomFloat(4, 0.3f, 0.05f);

			// ビルボードの設定
			SetBlessParticle(pos, move, XMFLOAT4(color.x, color.y, color.z, 0.85f), fSize, fSize, speed);
		}

		// 最初に1回だけSEを再生
		if (g_time == 0)
		{
			// SEを再生
			PlaySound(SOUND_LABEL_SE_BAHAMUT_charge);
		}


		// 時間を進める
		g_time++;

		// 一定時間経過でチャージパーティクルを発生
		if (g_time > BLESS_TIME_START)
		{
			SetBeamParticleSwitch(TRUE);
			//SetBeamParticleSwitch2(TRUE);
		}

		// 一定時間になったら止める
		if (g_time > BLESS_TIME_MAX)
		{
			g_End = TRUE;

			SetChargeRadius(0.2f);


			// SEをフェードアウト
			StopSound(SOUND_LABEL_SE_BAHAMUT_charge);
		}

		// フェード開始
//		SetFadeWhite(WHITE_OUT_charge);
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	if ((GetKeyboardPress(DIK_N)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_On = FALSE;
	}


#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBlessParticle(void)
{
	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// ライティングを無効に
	SetLightEnable(FALSE);

	// 加算合成に設定
	SetBlendState(BLEND_MODE_ADD);

	// Z比較無し
	SetDepthEnable(FALSE);

	// フォグ無効
	SetFogEnable(FALSE);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	{
		if (g_aBlessParticle[nCntParticle].bUse)
		{
			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// ビューマトリックスを取得
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			// 転置行列処理
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

			// スケールを反映
			mtxScl = XMMatrixScaling(g_aBlessParticle[nCntParticle].scale.x, g_aBlessParticle[nCntParticle].scale.y, g_aBlessParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aBlessParticle[nCntParticle].pos.x, g_aBlessParticle[nCntParticle].pos.y, g_aBlessParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_aBlessParticle[nCntParticle].material);

			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	//for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	//{
	//	if (g_aBlessParticle[nCntParticle].bUse)
	//	{
	//		// ワールドマトリックスの初期化
	//		mtxWorld = XMMatrixIdentity();

	//		// ビューマトリックスを取得
	//		mtxView = XMLoadFloat4x4(&cam->mtxView);

	//		// 転置行列処理
	//		mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
	//		mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
	//		mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

	//		mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
	//		mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
	//		mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

	//		mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
	//		mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
	//		mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

	//		// スケールを反映
	//		mtxScl = XMMatrixScaling(g_aBlessParticle[nCntParticle].scale.x, g_aBlessParticle[nCntParticle].scale.y, g_aBlessParticle[nCntParticle].scale.z);
	//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	//		// 移動を反映
	//		mtxTranslate = XMMatrixTranslation(g_aBlessParticle[nCntParticle].pos.x, g_aBlessParticle[nCntParticle].pos.y, g_aBlessParticle[nCntParticle].pos.z);
	//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	//		// ワールドマトリックスの設定
	//		SetWorldMatrix(&mtxWorld);

	//		// マテリアル設定
	//		SetMaterial(g_aBlessParticle[nCntParticle].material);

	//		// ポリゴンの描画
	//		GetDeviceContext()->Draw(4, 0);
	//	}
	//}

	// ライティングを有効に
	SetLightEnable(TRUE);

	// 通常ブレンドに戻す
	SetBlendState(BLEND_MODE_ALPHABLEND);

	// Z比較有効
	SetDepthEnable(TRUE);

	// フォグ有効
	SetFogEnable(GetFogSwitch());

}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexBlessParticle(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	{//頂点バッファの中身を埋める
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

		VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

		// 頂点座標の設定
		vertex[0].Position = XMFLOAT3(-BLESS_PARTICLE_SIZE_X / 2,  BLESS_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3( BLESS_PARTICLE_SIZE_X / 2,  BLESS_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-BLESS_PARTICLE_SIZE_X / 2, -BLESS_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3( BLESS_PARTICLE_SIZE_X / 2, -BLESS_PARTICLE_SIZE_Y / 2, 0.0f);

		// 法線の設定
		vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

		// 反射光の設定
		vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		// テクスチャ座標の設定
		vertex[0].TexCoord = { 0.0f, 0.0f };
		vertex[1].TexCoord = { 1.0f, 0.0f };
		vertex[2].TexCoord = { 0.0f, 1.0f };
		vertex[3].TexCoord = { 1.0f, 1.0f };

		GetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}

	return S_OK;
}

//=============================================================================
// マテリアルカラーの設定
//=============================================================================
void SetColorBlessParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aBlessParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// パーティクルの発生処理
//=============================================================================
int SetBlessParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int life)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_BLESS_PARTICLE; nCntParticle++)
	{
		if (!g_aBlessParticle[nCntParticle].bUse)
		{
			g_aBlessParticle[nCntParticle].pos = pos;
			g_aBlessParticle[nCntParticle].rot = { 0.0f, 0.0f, 0.0f };
			g_aBlessParticle[nCntParticle].scale = { fSizeX, fSizeY, fSizeY };
			g_aBlessParticle[nCntParticle].move = move;
			g_aBlessParticle[nCntParticle].material.Diffuse = col;
			g_aBlessParticle[nCntParticle].fSizeX = fSizeX;
			g_aBlessParticle[nCntParticle].fSizeY = fSizeY;
			g_aBlessParticle[nCntParticle].nLife = life;
			g_aBlessParticle[nCntParticle].bUse = TRUE;

			nIdxParticle = nCntParticle;

			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// パーティクルの発生スイッチ
//=============================================================================
void SetBlessParticleSwitch(BOOL data)
{
	if (g_On == FALSE)
	{
		g_On = data;
		g_End = FALSE;
		g_time = 0;
	}
	else
	{
		g_On = data;
	}
}