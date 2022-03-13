//=============================================================================
//
// 矢印ガイドの処理 [arrow.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "arrow.h"
#include "lightning.h"
#include "player.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ARROW				"data/MODEL/arrow.obj"		// 読み込むモデル名

#define	VALUE_ROTATE			(XM_PI * 0.02f)				// 回転量

#define DESTINATION_X			(2200.0f)					// 目的地のx座標

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ARROW			g_Arrow;				// 木

static BOOL				g_Load = FALSE;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitArrow(void)
{
	LoadModel(MODEL_ARROW, &g_Arrow.model, 0);
	g_Arrow.load = TRUE;
	g_Arrow.pos = XMFLOAT3(0.0f, 70.0f, 0.0f);
	g_Arrow.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Arrow.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Arrow.use = TRUE;			// TRUE:生きてる
	// モデルのディフューズを保存しておく。色変え対応の為。
	GetModelDiffuse(&g_Arrow.model, &g_Arrow.diffuse[0]);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitArrow(void)
{
	if (g_Load == FALSE) return;

	UnloadModel(&g_Arrow.model);
	g_Arrow.load = FALSE;

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateArrow(void)
{
	// エネミーを動かす場合は、影も合わせて動かす事を忘れないようにね！
	if (g_Arrow.use == TRUE)			// このエネミーが使われている？
	{									// Yes
		PLAYER *player = GetPlayer();
		CAMERA *cam = GetCamera();

		// 画面の上に合わせる
		g_Arrow.pos.x = player->pos.x;
		g_Arrow.pos.z = player->pos.z;

		// 目的地の方向
		float disX, disZ;
		disX = DESTINATION_X - player->pos.x;
		disZ = -player->pos.z;

		// y軸回転の調整
		if (disZ > 0.0f)
		{
			g_Arrow.rot.y = (XM_PI) + atanf(disX / disZ);
		}
		else if(disX < 0.0f)
		{
			g_Arrow.rot.y = (XM_PI / 2) - atanf(disZ / disX);
		}
		else
		{
			g_Arrow.rot.y = (-XM_PI / 2) - atanf(disZ / disX);
		}


	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawArrow(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	if (g_Arrow.use == FALSE) return;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Arrow.scl.x, g_Arrow.scl.y, g_Arrow.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Arrow.rot.x, g_Arrow.rot.y + XM_PI, g_Arrow.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Arrow.pos.x, g_Arrow.pos.y, g_Arrow.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Arrow.mtxWorld, mtxWorld);

	// モデル描画
	DrawModel(&g_Arrow.model);
}

//=============================================================================
// エネミーの取得
//=============================================================================
ARROW *GetArrow()
{
	return &g_Arrow;
}


// 視点を消えり替えた時に木の表示を切り替える
// TPS視点では手前の木を非表示にする
void SetArrowSwitch(BOOL data)
{
	g_Arrow.use = data;
}