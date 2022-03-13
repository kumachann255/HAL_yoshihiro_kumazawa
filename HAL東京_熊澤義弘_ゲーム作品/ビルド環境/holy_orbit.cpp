//=============================================================================
//
// ホーリーの軌道処理 [holy_orbit.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "holy_orbit.h"
#include "player.h"
#include "holy.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					(1)			// テクスチャの数

#define	HOLY_PARTICLE_SIZE_X		(2.5f)		// 頂点サイズ
#define	HOLY_PARTICLE_SIZE_Y		(2.5f)		// 頂点サイズ
#define	VALUE_MOVE_HOLY_PARTICLE	(0.2f)		// 移動速度

#define	MAX_HOLY_ORBIT				(1024)		// パーティクル最大数

#define ORBIT_LIFE					(25)		// 軌道の寿命

#define HOLY_PARTICLE_OFFSET_X		(40)		// 剣の中心からどの範囲の中で発生させるか
#define HOLY_PARTICLE_OFFSET_Z		(40)		// 剣の中心からどの範囲の中で発生させるか

#define ORBIT_1F_NUM				(8)		// 1フレームで何個表示するか

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
	MATERIAL		material;		// マテリアル
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか

} PARTICLE;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexHolyOrbit(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static PARTICLE					g_aHolyOrbit[MAX_HOLY_ORBIT];		// パーティクルワーク
static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 1.0f;			// 基準の幅
static float					g_fHeightBase = 1.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/holy_orbit.png",
};

static BOOL						g_Load = FALSE;
static BOOL						g_OrbitOn = FALSE;		// 剣から軌道を発生させるか
static int						g_HolyNum = 0;			// どの剣からパーティクルを発生させるか

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitHolyOrbit(void)
{
	// 頂点情報の作成
	MakeVertexHolyOrbit();

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
	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_ORBIT; nCntParticle++)
	{
		g_aHolyOrbit[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aHolyOrbit[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aHolyOrbit[nCntParticle].scale = XMFLOAT3(HOLY_PARTICLE_SIZE_X, HOLY_PARTICLE_SIZE_Y, 1.0f);

		ZeroMemory(&g_aHolyOrbit[nCntParticle].material, sizeof(g_aHolyOrbit[nCntParticle].material));
		g_aHolyOrbit[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aHolyOrbit[nCntParticle].nLife = 0;
		g_aHolyOrbit[nCntParticle].bUse = FALSE;
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
void UninitHolyOrbit(void)
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
void UpdateHolyOrbit(void)
{
	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_ORBIT; nCntParticle++)
	{
		if (g_aHolyOrbit[nCntParticle].bUse)
		{// 使用中

			g_aHolyOrbit[nCntParticle].nLife--;
			if (g_aHolyOrbit[nCntParticle].nLife <= 0)
			{
				g_aHolyOrbit[nCntParticle].bUse = FALSE;
			}
			else
			{
				g_aHolyOrbit[nCntParticle].material.Diffuse.w -= 0.04f;
				g_aHolyOrbit[nCntParticle].scale.x -= 0.06f;
				g_aHolyOrbit[nCntParticle].scale.y -= 0.06f;
				if (g_aHolyOrbit[nCntParticle].material.Diffuse.w < 0.0f)
				{
					g_aHolyOrbit[nCntParticle].material.Diffuse.w = 0.0f;
				}
			}
		}
	}

	// 軌道発生
	if(g_OrbitOn)
	{
		XMFLOAT3 pos,dis;
		HOLY *holy = GetHoly();

		for (int h = 0; h < MAX_HOLY; h++)
		{
			pos = holy[h].pos;			// 現在の位置を取得
			dis = GetHolyMoveValue(h);	// 移動量を取得

			for (int p = 0; p < ORBIT_1F_NUM; p++)
			{
				// 差をパーティクル数で割って、少しずつ移動させながら発生させる
				pos.x -= dis.x / (float)ORBIT_1F_NUM;
				pos.y -= dis.y / (float)ORBIT_1F_NUM;
				pos.z -= dis.z / (float)ORBIT_1F_NUM;

				// ビルボードの設定
				SetHolyOrbit(pos);
			}
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawHolyOrbit(void)
{
	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// ライティングを無効に
	SetLightEnable(FALSE);

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

	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_ORBIT; nCntParticle++)
	{
		if (g_aHolyOrbit[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aHolyOrbit[nCntParticle].scale.x, g_aHolyOrbit[nCntParticle].scale.y, g_aHolyOrbit[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aHolyOrbit[nCntParticle].pos.x, g_aHolyOrbit[nCntParticle].pos.y, g_aHolyOrbit[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_aHolyOrbit[nCntParticle].material);

			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ライティングを有効に
	SetLightEnable(TRUE);

	// Z比較有効
	SetDepthEnable(TRUE);

	// フォグ有効
	SetFogEnable(GetFogSwitch());

}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexHolyOrbit(void)
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
		vertex[0].Position = XMFLOAT3(-HOLY_PARTICLE_SIZE_X / 2, HOLY_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(HOLY_PARTICLE_SIZE_X / 2, HOLY_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-HOLY_PARTICLE_SIZE_X / 2, -HOLY_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(HOLY_PARTICLE_SIZE_X / 2, -HOLY_PARTICLE_SIZE_Y / 2, 0.0f);

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
void SetColorHolyOrbit(int nIdxParticle, XMFLOAT4 col)
{
	g_aHolyOrbit[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// パーティクルの発生処理
//=============================================================================
int SetHolyOrbit(XMFLOAT3 pos)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_HOLY_ORBIT; nCntParticle++)
	{
		if (!g_aHolyOrbit[nCntParticle].bUse)
		{
			g_aHolyOrbit[nCntParticle].pos = pos;
			g_aHolyOrbit[nCntParticle].rot = { 0.0f, 0.0f, 0.0f };
			g_aHolyOrbit[nCntParticle].scale = XMFLOAT3(HOLY_PARTICLE_SIZE_X, HOLY_PARTICLE_SIZE_Y, 1.0f);
			g_aHolyOrbit[nCntParticle].material.Diffuse.w = 1.0f;
			g_aHolyOrbit[nCntParticle].nLife = ORBIT_LIFE;
			g_aHolyOrbit[nCntParticle].bUse = TRUE;

			nIdxParticle = nCntParticle;

			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// 軌道の発生スイッチ
//=============================================================================
void SetOrbitSwitch(BOOL data)
{
	g_OrbitOn = data;
}

