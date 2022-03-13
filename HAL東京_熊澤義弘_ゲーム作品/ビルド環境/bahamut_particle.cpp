//=============================================================================
//
// バハムートのパーティクル処理 [Bahamut_particle.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "bahamut_particle.h"
#include "player.h"
#include "holy.h"
#include "bahamut.h"
#include "lightning.h"
#include "debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX			(1)			// テクスチャの数

#define	BAHAMUT_PARTICLE_SIZE_X		(5.0f)		// 頂点サイズ
#define	BAHAMUT_PARTICLE_SIZE_Y		(5.0f)		// 頂点サイズ
#define	VALUE_MOVE_BAHAMUT_PARTICLE	(0.2f)		// 移動速度

#define	MAX_BAHAMUT_PARTICLE		(512 * 15)		// パーティクル最大数

#define BAHAMUT_PARTICLE_OFFSET		(100)		// バハムート中心からどのくらい離して発生させるか

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
	int				nIdxShadow;		// 影ID
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか

} PARTICLE;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexBahamutParticle(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static PARTICLE					g_aBahamutParticle[MAX_BAHAMUT_PARTICLE];		// パーティクルワーク
static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 1.0f;			// 基準の幅
static float					g_fHeightBase = 1.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/effect000.jpg",
};

static BOOL						g_Load = FALSE;
static BOOL						g_On = FALSE;				// パーティクルを発生さているか
static BOOL						g_End = TRUE;				// パーティクルを発生させたか
static int						g_BahamutNum = 0;			// どの剣からパーティクルを発生させるか

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBahamutParticle(void)
{
	// 頂点情報の作成
	MakeVertexBahamutParticle();

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
	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		g_aBahamutParticle[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBahamutParticle[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBahamutParticle[nCntParticle].scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
		g_aBahamutParticle[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_aBahamutParticle[nCntParticle].material, sizeof(g_aBahamutParticle[nCntParticle].material));
		g_aBahamutParticle[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aBahamutParticle[nCntParticle].fSizeX = BAHAMUT_PARTICLE_SIZE_X;
		g_aBahamutParticle[nCntParticle].fSizeY = BAHAMUT_PARTICLE_SIZE_Y;
		g_aBahamutParticle[nCntParticle].nIdxShadow = -1;
		g_aBahamutParticle[nCntParticle].nLife = 0;
		g_aBahamutParticle[nCntParticle].bUse = FALSE;
	}

	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBahamutParticle(void)
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
void UpdateBahamutParticle(void)
{
	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		if (g_aBahamutParticle[nCntParticle].bUse)
		{// 使用中
			g_aBahamutParticle[nCntParticle].pos.x += g_aBahamutParticle[nCntParticle].move.x;
			g_aBahamutParticle[nCntParticle].pos.z += g_aBahamutParticle[nCntParticle].move.z;

			g_aBahamutParticle[nCntParticle].pos.y += g_aBahamutParticle[nCntParticle].move.y;

			g_aBahamutParticle[nCntParticle].move.x += (0.0f - g_aBahamutParticle[nCntParticle].move.x) * 0.015f;
			g_aBahamutParticle[nCntParticle].move.z += (0.0f - g_aBahamutParticle[nCntParticle].move.z) * 0.015f;


			g_aBahamutParticle[nCntParticle].nLife--;
			if ((g_aBahamutParticle[nCntParticle].nLife <= 0))
			{
				g_aBahamutParticle[nCntParticle].bUse = FALSE;
				g_aBahamutParticle[nCntParticle].nIdxShadow = -1;
			}
			else
			{
				if (g_aBahamutParticle[nCntParticle].nLife <= 20)
				{
					g_aBahamutParticle[nCntParticle].material.Diffuse.x = 0.5f - (float)(80 - g_aBahamutParticle[nCntParticle].nLife) / 80 * 0.5f;
					g_aBahamutParticle[nCntParticle].material.Diffuse.y = 0.5f - (float)(80 - g_aBahamutParticle[nCntParticle].nLife) / 80 * 0.5f;
					g_aBahamutParticle[nCntParticle].material.Diffuse.z = 0.5f - (float)(80 - g_aBahamutParticle[nCntParticle].nLife) / 80 * 0.5f;
				}

				if (g_aBahamutParticle[nCntParticle].nLife <= 10)
				{
					// α値設定
					g_aBahamutParticle[nCntParticle].material.Diffuse.w -= 0.05f;
					if (g_aBahamutParticle[nCntParticle].material.Diffuse.w < 0.0f)
					{
						g_aBahamutParticle[nCntParticle].material.Diffuse.w = 0.0f;
					}
				}
			}
		}
	}

	// 翼が開いたタイミングでパーティクルを発生させる
	if (GetWingOpen())
	{
		if (g_On == FALSE)
		{
			g_On = TRUE;
			g_End = FALSE;
		}
	}


	// パーティクル発生
	if((g_On) && (!g_End))
	{
		for (int i = 0; i < MAX_BAHAMUT_PARTICLE; i++)
		{
			XMFLOAT3 pos;
			BAHAMUT *bahamut = GetBahamut();

			pos = bahamut->pos;
			pos.y += 100.0f;


			g_BahamutNum++;

			XMFLOAT3 move;
			float fAngle, fLength;
			int nLife;
			float fSize;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			fLength = RamdomFloat(2, 2.0f, 0.2f);
			pos.x += sinf(fAngle) * BAHAMUT_PARTICLE_OFFSET;
			move.x = -sinf(fAngle) * fLength * 20.0f;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			//fLength = rand() % (int)(g_fWidthBase * 10) / 100.0f - g_fWidthBase;
			pos.y += cosf(fAngle + (XM_PI / 2)) * BAHAMUT_PARTICLE_OFFSET;
			move.y = -cosf(fAngle) * fLength * 20.0f;

			fAngle = (float)(rand() % 628 - 314) / 100.0f;
			//fLength = rand() % (int)(g_fWidthBase * 10) / 100.0f - g_fWidthBase;
			pos.z += cosf(fAngle) * BAHAMUT_PARTICLE_OFFSET;
			move.z = -cosf(fAngle + (XM_PI / 2)) * fLength * 20.0f;


			nLife = rand() % 40 + 120;

			fSize = RamdomFloat(2, 1.0f, 0.5f);

			// ビルボードの設定
			SetBahamutParticle(pos, move, XMFLOAT4(0.5f, 0.5f, 0.5f, 0.85f), fSize, fSize, nLife);
		}

		g_End = TRUE;
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("life:%d \n", g_aBahamutParticle[100].nLife);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBahamutParticle(void)
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

	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		if (g_aBahamutParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aBahamutParticle[nCntParticle].scale.x, g_aBahamutParticle[nCntParticle].scale.y, g_aBahamutParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aBahamutParticle[nCntParticle].pos.x, g_aBahamutParticle[nCntParticle].pos.y, g_aBahamutParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_aBahamutParticle[nCntParticle].material);

			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		if (g_aBahamutParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aBahamutParticle[nCntParticle].scale.x, g_aBahamutParticle[nCntParticle].scale.y, g_aBahamutParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aBahamutParticle[nCntParticle].pos.x, g_aBahamutParticle[nCntParticle].pos.y, g_aBahamutParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_aBahamutParticle[nCntParticle].material);

			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

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
HRESULT MakeVertexBahamutParticle(void)
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
		vertex[0].Position = XMFLOAT3(-BAHAMUT_PARTICLE_SIZE_X / 2,  BAHAMUT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3( BAHAMUT_PARTICLE_SIZE_X / 2,  BAHAMUT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-BAHAMUT_PARTICLE_SIZE_X / 2, -BAHAMUT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3( BAHAMUT_PARTICLE_SIZE_X / 2, -BAHAMUT_PARTICLE_SIZE_Y / 2, 0.0f);

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
void SetColorBahamutParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aBahamutParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// パーティクルの発生処理
//=============================================================================
int SetBahamutParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_BAHAMUT_PARTICLE; nCntParticle++)
	{
		if (!g_aBahamutParticle[nCntParticle].bUse)
		{
			g_aBahamutParticle[nCntParticle].pos = pos;
			g_aBahamutParticle[nCntParticle].rot = { fSizeX, fSizeX, fSizeX };
			g_aBahamutParticle[nCntParticle].scale = { 1.0f, 1.0f, 1.0f };
			g_aBahamutParticle[nCntParticle].move = move;
			g_aBahamutParticle[nCntParticle].material.Diffuse = col;
			g_aBahamutParticle[nCntParticle].fSizeX = fSizeX;
			g_aBahamutParticle[nCntParticle].fSizeY = fSizeY;
			g_aBahamutParticle[nCntParticle].nLife = nLife;
			g_aBahamutParticle[nCntParticle].bUse = TRUE;

			nIdxParticle = nCntParticle;

			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// パーティクルの発生スイッチ
//=============================================================================
void SetBahamutParticleSwitch(BOOL data)
{
	g_On = data;
}