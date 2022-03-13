//=============================================================================
//
// 魔法陣処理 [magic_circle.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "debugproc.h"
#include "camera.h"
#include "model.h"
#include "magic_circle.h"
#include "player.h"
#include "holy.h"
#include "lightning.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX				(4)			// テクスチャの数

#define	CIRCLE_SIZE_X			(5.0f)		// 頂点サイズ
#define	CIRCLE_SIZE_Y			(5.0f)		// 頂点サイズ
#define	VALUE_MOVE_CIRCLE		(0.2f)		// 移動速度

#define CIRCLE_OFFSET_X			(40)		// 剣の中心からどの範囲の中で発生させるか
#define CIRCLE_OFFSET_Z			(40)		// 剣の中心からどの範囲の中で発生させるか

#define CIRCLE_DISTANCE			(40)		// どのくらいプレイヤーから話したところから炎を発生させるか

#define CIRCLE_DISSOLVE			(0.01f)		// ディゾルブの速度

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexCircleParticle(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static CIRCLE					g_Circle;						// 魔法陣の構造体

static float					g_fWidthBase = 1.0f;			// 基準の幅
static float					g_fHeightBase = 1.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/circle0.png",
	"data/TEXTURE/circle1.png",
	"data/TEXTURE/circle2.png",
	"data/TEXTURE/circle3.png",
};

static BOOL						g_Load = FALSE;
static BOOL						g_CircleOn = FALSE;		// 剣からパーティクルを発生させるか
static int						g_CircleNum = 0;			// どの剣からパーティクルを発生させるか

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitCircle(void)
{
	// 頂点情報の作成
	MakeVertexCircleParticle();

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
	for (int i = 0; i < MAX_CIRCLE; i++)
	{
		g_Circle.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Circle.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Circle.scale = XMFLOAT3(12.0f, 12.0f, 1.0f);

		ZeroMemory(&g_Circle.material, sizeof(g_Circle.material));
		g_Circle.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_Circle.bUse = FALSE;
		g_Circle.dissolve = 0.0f;
	}

	g_roty = 0.0f;
	g_spd = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitCircle(void)
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
void UpdateCircle(void)
{
	PLAYER *player = GetPlayer();

	for (int i = 0; i < MAX_CIRCLE; i++)
	{
		if (g_Circle.bUse)
		{// 使用中
			g_Circle.pos = player->pos;

			// 位置調整
			g_Circle.pos.x -= sinf(player->rot.y) * CIRCLE_DISTANCE;
			g_Circle.pos.z -= cosf(player->rot.y) * CIRCLE_DISTANCE;
			g_Circle.pos.y += 6.0f;


			// プレイヤーの正面に向きを調整
			g_Circle.rot.y = player->rot.y + XM_PI;

			// ディゾルブで現れ始める
			if ((g_Circle.dissolve < 1.0f) && (!g_Circle.end))
			{
				g_Circle.dissolve += CIRCLE_DISSOLVE;
			}
		}

	
		if (g_Circle.end)
		{	// 消え始める処理が始まっている
			if (g_Circle.dissolve > 0.0f) g_Circle.dissolve -= CIRCLE_DISSOLVE;

			// ディゾルブが終了したら
			if (g_Circle.dissolve <= -CIRCLE_DISSOLVE)
			{
				ResetCircle();
			}
		}
	}

	PrintDebugProc("%f\n", g_Circle.dissolve);
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawCircle(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

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


	for (int i = 0; i < MAX_CIRCLE; i++)
	{
		if (g_Circle.bUse)
		{
			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// ビューマトリックスを取得
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			// スケールを反映
			mtxScl = XMMatrixScaling(g_Circle.scale.x, g_Circle.scale.y, g_Circle.scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 回転を反映
			mtxRot = XMMatrixRotationRollPitchYaw(g_Circle.rot.x, g_Circle.rot.y, g_Circle.rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_Circle.pos.x, g_Circle.pos.y, g_Circle.pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// ディゾルブの設定
			SetDissolve(g_Circle.dissolve);

			// マテリアル設定
			SetMaterial(g_Circle.material);

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

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexCircleParticle(void)
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
		vertex[0].Position = XMFLOAT3(-CIRCLE_SIZE_X / 2, CIRCLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(CIRCLE_SIZE_X / 2, CIRCLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-CIRCLE_SIZE_X / 2, -CIRCLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(CIRCLE_SIZE_X / 2, -CIRCLE_SIZE_Y / 2, 0.0f);

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
void SetColorCircle(XMFLOAT4 col)
{
	g_Circle.material.Diffuse = col;
}

//=============================================================================
// 魔法陣の発生処理
//=============================================================================
void SetCircle(BOOL use, BOOL end, int magic)
{
	g_Circle.bUse = use;
	g_Circle.end = end;
	if(magic != null) g_TexNo = magic - 1;

	// 魔法の種類によって色を変える
	switch (magic)
	{
	case fire:
		g_Circle.material.Diffuse = { 0.8f, 0.3f, 0.2f, 1.0f };
		break;

	case lightning:
		g_Circle.material.Diffuse = { 0.8f, 0.8f, 0.0f, 1.0f };
		break;

	case water:
		g_Circle.material.Diffuse = { 0.25f, 0.6f, 1.0f, 1.0f };
		break;

	case holy:
		g_Circle.material.Diffuse = { 2.0f, 2.0f, 2.0f, 1.0f };
		break;
	}
}


//=============================================================================
// ファイアの終了処理
//=============================================================================
void ResetCircle(void)
{
	g_Circle.pos = { 0.0f, 0.0f, 0.0f };
	g_Circle.rot = { 0.0f, 0.0f, 0.0f };
	g_Circle.scale = { 1.0f, 1.0f, 1.0f };
	g_Circle.material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f};
	g_Circle.bUse = FALSE;
	g_Circle.dissolve = 0.0f;
}


//=============================================================================
// ファイアの情報を取得
//=============================================================================
CIRCLE *GetCircle(void)
{
	return &g_Circle;
}