//=============================================================================
//
// スカイドームの処理 [skydome.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "skydome.h"
#include "lightning.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_SKY				"data/MODEL/sky.obj"		// 読み込むモデル名

#define	VALUE_ROTATE			(XM_PI * 0.02f)				// 回転量

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static SKY				g_Sky;				// 木

static BOOL				g_Load = FALSE;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitSky(void)
{
	LoadModel(MODEL_SKY, &g_Sky.model, 0);
	g_Sky.load = TRUE;
	g_Sky.pos = XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f);
	g_Sky.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Sky.scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
	g_Sky.use = TRUE;			// TRUE:生きてる
	// モデルのディフューズを保存しておく。色変え対応の為。
	GetModelDiffuse(&g_Sky.model, &g_Sky.diffuse[0]);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSky(void)
{
	if (g_Load == FALSE) return;

	UnloadModel(&g_Sky.model);
	g_Sky.load = FALSE;

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateSky(void)
{

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawSky(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_FRONT);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Sky.scl.x, g_Sky.scl.y, g_Sky.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Sky.rot.x, g_Sky.rot.y + XM_PI, g_Sky.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Sky.pos.x, g_Sky.pos.y, g_Sky.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Sky.mtxWorld, mtxWorld);

	// モデル描画
	DrawModel(&g_Sky.model);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
SKY *GetSky()
{
	return &g_Sky;
}
