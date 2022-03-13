//=============================================================================
//
// ファイア処理 [fire.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "fire.h"
#include "player.h"
#include "holy.h"
#include "lightning.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX				(1)			// テクスチャの数

#define	FIRE_SIZE_X				(10.0f)		// 頂点サイズ
#define	FIRE_SIZE_Y				(10.0f)		// 頂点サイズ
#define	VALUE_MOVE_FIRE			(0.2f)		// 移動速度

#define FIRE_OFFSET_X			(40)		// 剣の中心からどの範囲の中で発生させるか
#define FIRE_OFFSET_Z			(40)		// 剣の中心からどの範囲の中で発生させるか

#define FIRE_DISTANCE			(40)		// どのくらいプレイヤーから話したところから炎を発生させるか

#define FIRE_SPEED_BACE			(3.0f)		// 炎の基本の速度
#define FIRE_WIDTH_X			(0.8f)		// 炎の振れ幅
#define FIRE_WIDTH_Z			(0.8f)		// 炎の振れ幅
#define FIRE_WIDTH_Y			(0.4f)		// 炎の振れ幅

#define FIRE_START_SCALE_MAX	(1.0f)		// 発射時の大きさの最大値
#define FIRE_START_SCALE_MIN	(0.5f)		// 発射時の大きさの最小値

#define FIRE_SE_TIME			(40)		// SEを再再生するフレーム数

#define	DISP_SHADOW							// 影の表示
//#undef DISP_SHADOW

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexFireParticle(void);
void SetFireParameter(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static FIRE						g_Fire[MAX_FIRE];		// パーティクルワーク
static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 1.0f;			// 基準の幅
static float					g_fHeightBase = 1.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/fire1.png",
};

static BOOL						g_Load = FALSE;
static BOOL						g_FireOn = FALSE;		// 剣からパーティクルを発生させるか
static int						g_FireNum = 0;			// どの剣からパーティクルを発生させるか
static int						g_time = 0;				// どのくらい連続して発動しているか
static int						g_SE = 0;				// SEを管理

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitFire(void)
{
	// 頂点情報の作成
	MakeVertexFireParticle();

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
	for (int nCntParticle = 0; nCntParticle < MAX_FIRE; nCntParticle++)
	{
		g_Fire[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Fire[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Fire[nCntParticle].scale = XMFLOAT3(0.3f, 0.3f, 0.3f);
		g_Fire[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_Fire[nCntParticle].material, sizeof(g_Fire[nCntParticle].material));
		g_Fire[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_Fire[nCntParticle].fSizeX = FIRE_SIZE_X;
		g_Fire[nCntParticle].fSizeY = FIRE_SIZE_Y;
		g_Fire[nCntParticle].nIdxShadow = -1;
		g_Fire[nCntParticle].nLife = 0;
		g_Fire[nCntParticle].bUse = FALSE;
		g_Fire[nCntParticle].Up = FALSE;
		g_Fire[nCntParticle].atk = FALSE;
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
void UninitFire(void)
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
void UpdateFire(void)
{
	PLAYER *player = GetPlayer();
	//g_posBase = pPlayer->pos;
	CAMERA *cam = GetCamera();

	for (int nCntParticle = 0; nCntParticle < MAX_FIRE; nCntParticle++)
	{
		if (g_Fire[nCntParticle].bUse)
		{// 使用中
			g_Fire[nCntParticle].pos.x += g_Fire[nCntParticle].move.x;
			g_Fire[nCntParticle].pos.z += g_Fire[nCntParticle].move.z;

			g_Fire[nCntParticle].pos.y += g_Fire[nCntParticle].move.y;


			g_Fire[nCntParticle].move.x += (0.0f - g_Fire[nCntParticle].move.x) * 0.015f;
			g_Fire[nCntParticle].move.z += (0.0f - g_Fire[nCntParticle].move.z) * 0.015f;

			// 常に回転させる
			g_Fire[nCntParticle].rot.x -= sinf(cam->rot.y) * RamdomFloat(4, 0.2f, -0.2f);
			g_Fire[nCntParticle].rot.z -= -cosf(cam->rot.y) * RamdomFloat(4, 0.2f, -0.2f);
			//	上昇が始まっているかどうかで処理を変更
			if (g_Fire[nCntParticle].Up)
			{
				// 上昇処理
				g_Fire[nCntParticle].move.y += 0.12f;

				// 上昇に伴い小さくする
				g_Fire[nCntParticle].scale.x -= 0.15f;
				g_Fire[nCntParticle].scale.y -= 0.15f;
				g_Fire[nCntParticle].scale.z -= 0.15f;
			}
			else
			{	// 徐々に大きくして、大きさが一定になったら上昇する
				g_Fire[nCntParticle].scale.x += 0.015f;
				g_Fire[nCntParticle].scale.y += 0.015f;

				// 上昇処理開始
				if (g_Fire[nCntParticle].scale.x > 2.0f) g_Fire[nCntParticle].Up = TRUE;
			}


			// 大きさが0.0f以下になったら終了処理
			if (g_Fire[nCntParticle].scale.x < 0.0f)
			{
				ResetFire(nCntParticle);
			}
		}
	}

	// 連続発動時間が一定になったらSEを重ねて再生
	if (g_time % FIRE_SE_TIME == 1)
	{
		g_SE++;

		switch (g_SE % 3)
		{
		case 0:
			PlaySound(SOUND_LABEL_SE_MAGIC_fire0);
			break;

		case 1:
			PlaySound(SOUND_LABEL_SE_MAGIC_fire1);
			break;

		case 2:
			PlaySound(SOUND_LABEL_SE_MAGIC_fire2);
			break;
		}
	}

	for (int p = 0; p < 5; p++)
	{
		SetFireParameter();
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawFire(void)
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


	for (int nCntParticle = 0; nCntParticle < MAX_FIRE; nCntParticle++)
	{
		if (g_Fire[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_Fire[nCntParticle].scale.x, g_Fire[nCntParticle].scale.y, g_Fire[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 回転を反映
			mtxRot = XMMatrixRotationRollPitchYaw(g_Fire[nCntParticle].rot.x, g_Fire[nCntParticle].rot.y, g_Fire[nCntParticle].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_Fire[nCntParticle].pos.x, g_Fire[nCntParticle].pos.y, g_Fire[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_Fire[nCntParticle].material);

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
HRESULT MakeVertexFireParticle(void)
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
		vertex[0].Position = XMFLOAT3(-FIRE_SIZE_X / 2, FIRE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(FIRE_SIZE_X / 2, FIRE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-FIRE_SIZE_X / 2, -FIRE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(FIRE_SIZE_X / 2, -FIRE_SIZE_Y / 2, 0.0f);

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
void SetColorFire(int nIdxParticle, XMFLOAT4 col)
{
	g_Fire[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// パーティクルの発生処理
//=============================================================================
int SetFireParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, int nLife, float scaleX, float scaleY)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_FIRE; nCntParticle++)
	{
		if (!g_Fire[nCntParticle].bUse)
		{
			g_Fire[nCntParticle].pos = pos;
			g_Fire[nCntParticle].rot = { 0.0f, 0.0f, 0.0f };
			g_Fire[nCntParticle].scale = { scaleX, scaleY, 1.0f };
			g_Fire[nCntParticle].move = move;
			g_Fire[nCntParticle].material.Diffuse = col;
			g_Fire[nCntParticle].nLife = nLife;
			g_Fire[nCntParticle].bUse = TRUE;
			g_Fire[nCntParticle].Up = FALSE;

			nIdxParticle = nCntParticle;
			break;
		}
	}

	return nIdxParticle;
}


// パーティクルの発生スイッチ
void SetFireSwitch(BOOL data)
{
	g_FireOn = data;
	if (data) g_time++;	// 使用していたら時間を進める
}


// ファイアの終了処理
void ResetFire(int i)
{
	g_Fire[i].pos = { 0.0f, 0.0f, 0.0f };
	g_Fire[i].rot = { 0.0f, 0.0f, 0.0f };
	g_Fire[i].scale = { 0.3f, 0.3f, 1.0f };
	g_Fire[i].move = { 0.0f, 0.0f, 0.0f };
	g_Fire[i].material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f};
	g_Fire[i].fSizeX = FIRE_SIZE_X;
	g_Fire[i].fSizeY = FIRE_SIZE_Y;
	g_Fire[i].nLife = 0;
	g_Fire[i].bUse = FALSE;
	g_Fire[i].Up = FALSE;
	g_Fire[i].atk = FALSE;
}


// ファイアのパラメーター設定
void SetFireParameter(void)
{
	if (g_FireOn)
	{
		XMFLOAT3 pos;		// 発生位置
		XMFLOAT3 dis;		// プレイヤーの向いている方向ベクトル
		XMFLOAT2 scale;		// 発射時の大きさ

		PLAYER *player = GetPlayer();

		pos = player->pos;

		// 発生位置の設定
		pos.x -= sinf(player->rot.y) * FIRE_DISTANCE;
		pos.z -= cosf(player->rot.y) * FIRE_DISTANCE;
		pos.y += 6.0f;

		// ファイアを飛ばす方向を算出
		dis.x = -sinf(player->rot.y) * FIRE_SPEED_BACE;
		dis.z = -cosf(player->rot.y) * FIRE_SPEED_BACE;
		dis.y = 0.3f;

		// 飛ばす方向にランダム性を与える
		dis.x += RamdomFloat(4, FIRE_WIDTH_X, -FIRE_WIDTH_X);
		dis.z += RamdomFloat(4, FIRE_WIDTH_Z, -FIRE_WIDTH_Z);
		dis.y += RamdomFloat(4, FIRE_WIDTH_Y, -FIRE_WIDTH_Y);

		// 発射時の大きさをランダムで設定
		scale.x = scale.y = RamdomFloat(4, FIRE_START_SCALE_MAX, FIRE_START_SCALE_MIN);

		int nLife = 0;

		// ビルボードの設定
		SetFireParticle(pos, dis, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),  nLife, scale.x, scale.y);
	}
}


// ファイアの情報を取得
FIRE *GetFire(void)
{
	return &g_Fire[0];
}


// 炎魔法の連続発動時間をリセット
void ResetFireTime(void)
{
	g_time = 0;
}