//=============================================================================
//
// 木の処理 [tree.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "tree.h"
#include "lightning.h"
#include "bahamut.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_TREE_0			"data/MODEL/tree_0.obj"		// 読み込むモデル名
#define	MODEL_TREE_1			"data/MODEL/tree_1.obj"		// 読み込むモデル名
#define	MODEL_TREE_2			"data/MODEL/tree_2.obj"		// 読み込むモデル名

#define	VALUE_ROTATE			(XM_PI * 0.02f)				// 回転量


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static TREE				g_Tree[MAX_TREE];				// 木

static BOOL				g_Load = FALSE;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTree(void)
{
	LoadModel(MODEL_TREE_0, &g_Tree[0].model, 0);
	g_Tree[0].load = TRUE;
	g_Tree[0].pos = XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f);
	g_Tree[0].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Tree[0].scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
	g_Tree[0].use = TRUE;			// TRUE:生きてる
	// モデルのディフューズを保存しておく。色変え対応の為。
	GetModelDiffuse(&g_Tree[0].model, &g_Tree[0].diffuse[0]);

	LoadModel(MODEL_TREE_1, &g_Tree[1].model, 0);
	g_Tree[1].load = TRUE;
	g_Tree[1].pos = XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f);
	g_Tree[1].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Tree[1].scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
	g_Tree[1].use = FALSE;			// 最初は俯瞰視点の為非表示
	// モデルのディフューズを保存しておく。色変え対応の為。
	GetModelDiffuse(&g_Tree[1].model, &g_Tree[1].diffuse[0]);

	LoadModel(MODEL_TREE_2, &g_Tree[2].model, 0);
	g_Tree[2].load = TRUE;
	g_Tree[2].pos = XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f);
	g_Tree[2].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Tree[2].scl = XMFLOAT3(3.0f, 3.0f, 3.0f);
	g_Tree[2].use = TRUE;			// TRUE:生きてる
	// モデルのディフューズを保存しておく。色変え対応の為。
	GetModelDiffuse(&g_Tree[2].model, &g_Tree[2].diffuse[0]);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTree(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_TREE; i++)
	{
		if (g_Tree[i].load)
		{
			UnloadModel(&g_Tree[i].model);
			g_Tree[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTree(void)
{
	// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_TREE; i++)
	{
		if (g_Tree[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes

		}
	}

	if (GetSummon())
	{
		g_Tree[1].use = TRUE;
	}
	else if(GetOverLook())
	{
		g_Tree[1].use = FALSE;
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTree(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	for (int i = 0; i < MAX_TREE; i++)
	{
		if (g_Tree[i].use == FALSE) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Tree[i].scl.x, g_Tree[i].scl.y, g_Tree[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Tree[i].rot.x, g_Tree[i].rot.y + XM_PI, g_Tree[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Tree[i].pos.x, g_Tree[i].pos.y, g_Tree[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Tree[i].mtxWorld, mtxWorld);

		// モデル描画
		DrawModel(&g_Tree[i].model);

	}
}

//=============================================================================
// エネミーの取得
//=============================================================================
TREE *GetTree()
{
	return &g_Tree[0];
}


//=============================================================================
// 視点を消えり替えた時に木の表示を切り替える
// TPS視点では手前の木を非表示にする
//=============================================================================
void SetTreeSwitch(BOOL data)
{
	g_Tree[1].use = data;
}