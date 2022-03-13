//=============================================================================
//
// バハムートのブレス処理 [bahamut_bless.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "bahamut_bless.h"
#include "bless_particle.h"
#include "player.h"
#include "enemy.h"
#include "math.h"
#include "collision.h"
#include "sound.h"
#include "tutorial.h"
#include "debugproc.h"
#include "fade_white.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_BLAST			"data/MODEL/bahamut_blast.obj"			// 読み込むモデル名

#define	MODEL_SURGE			"data/MODEL/bahamut_surge.obj"			// 読み込むモデル名

#define BLESS_SCALE_COUNT	(200)						// 水玉が完成するまでのフレーム数

#define BLESS_COLOR_MIN		(0.55f)						// 色の下限
#define BLESS_COLOR_SPEED	(0.001f)					// 色の変化する速度

#define BLAST_ROT_SPEED		(0.5f)						// 回転速度
#define BLAST_SCALE_SPEED	(0.18f)						// 拡大速度
#define BLAST_SCALE_MAX		(50.0f)						// 拡大の最大値
#define BLAST_SCALE_FADE_START	(20.0f)					// フェードアウトが始まる大きさ

#define BLESS_SCENE_WAIT_1	(25)						// ブレスシーン(1カメ)の時間
#define BLESS_SCENE_WAIT_2	(60)						// ブレスシーン(2カメ)の時間

#define SURGE_DISSOLVE_SPEED	(0.02f)					// ディゾルブの速さ
#define SURGE_ROT_SPEED			(0.05f)					// 回転の速さ
#define SURGE_POS_SPEED			(20)					// 移動の速さ(何フレームで目的地に到達するか)

#define BLAST_A_LINE			(1.0f)					// 透明にする基準ラインの下降速度

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static BLESS		g_Blast;						// 爆風
static BLESS		g_Surge;						// 波動

static BOOL			g_Load = FALSE;

static MODEL		g_Bless_Vertex[MAX_BLESS_MOVE];	// モーフィング用モデルの頂点情報が入ったデータ配列
static VERTEX_3D	*g_Vertex = NULL;				// 途中経過を記録する場所

static ID3D11Buffer	*g_VertexBuffer = NULL;			// 頂点バッファ

static BOOL			g_move = FALSE;
static int			morphing;

static int			g_SceneWait;					// 次のシーンに進むまでの待機時間

static float		g_SurgeDissolve;				// 波動の閾値
static BOOL			g_SurgeFiring;					// 波動が動くかどうか
static XMFLOAT3		g_SurgeMove;					// 波動の動く速度

static int			g_count = 0;

static XMFLOAT3		g_ALine;						// 爆風の透明化する基準値

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBless(void)
{
	// 爆風の初期化
	LoadModel(MODEL_BLAST, &g_Blast.model, 0);
	g_Blast.load = TRUE;

	g_Blast.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Blast.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Blast.scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Blast.diffuse[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_Blast.move_time = 0.0f;

	g_Blast.use = FALSE;			// TRUE:生きてる


	// 波動の初期化
	LoadModel(MODEL_SURGE, &g_Surge.model, 0);
	g_Surge.load = TRUE;

	g_Surge.pos = XMFLOAT3(0.0f, 100.0f, 0.0f);
	g_Surge.rot = XMFLOAT3(XM_PI / 5, -XM_PI / 2, 0.0f);
	g_Surge.scl = XMFLOAT3(1.4f, 1.4f, 1.4f);
	g_Surge.diffuse[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_Surge.move_time = 0.0f;

	g_Surge.use = FALSE;			// TRUE:生きてる

	morphing = 0;
	g_move = FALSE;
	g_SurgeDissolve = 1.0f;
	g_SurgeFiring = FALSE;
	g_SurgeMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_ALine = XMFLOAT3(0.0f, 400.0f, 0.0f);

	g_Load = TRUE;


	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBless(void)
{
	if (g_Load == FALSE) return;

	if (g_Blast.load)
	{
		UnloadModel(&g_Blast.model);
		g_Blast.load = FALSE;
	}

	if (g_Surge.load)
	{
		UnloadModel(&g_Surge.model);
		g_Surge.load = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateBless(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();
	BAHAMUT *bahamut = GetBahamut();

	if (g_move)
	{
		// 時間を進める
		g_count++;

		if (g_count > 410)
		{
			// シーンを変更
			SetSummonScene(bless);
		}
		
		if (g_count > 460)
		{
			SetSummonScene(bless_2camera);
		}

		if (g_count > 505)
		{
			SetSummonScene(blast);
		}

		if (g_count > 555)
		{
			// 爆風を描画
			if (!g_Blast.use)
			{
				g_Blast.use = TRUE;

				// ブレスと地面が設置している場所に爆風を配置
				g_Blast.pos.x = bahamut->pos.x + BLAST_X_OFFSET + 150.0f;
			}
		}
	}


	if (g_Blast.use)
	{
		if (g_Blast.scl.x < BLAST_SCALE_MAX)
		{
			// 回転しながら大きくなる
			g_Blast.scl.x += BLAST_SCALE_SPEED;
			g_Blast.scl.y += BLAST_SCALE_SPEED;
			g_Blast.scl.z += BLAST_SCALE_SPEED;

			g_Blast.rot.y += BLAST_ROT_SPEED;

			if (g_Blast.scl.x > BLAST_SCALE_FADE_START)
			{
				SetFadeWhite(WHITE_OUT_blast);
			}

			// 基準値を少しずつ下げる
			g_ALine.y -= BLAST_A_LINE;

		}
		else
		{
			ResetBless();
		}
	}

	if (g_Surge.use)
	{
		// 波動発射前
		if (!g_SurgeFiring)
		{
			// ディゾルブの閾値を降らす
			g_SurgeDissolve += SURGE_DISSOLVE_SPEED;
			if (g_SurgeDissolve > 1.0f) g_SurgeDissolve -= 1.0f;
		}
		else
		{	// 波動発射後
			g_Surge.rot.x += SURGE_ROT_SPEED;

			// 移動
			g_Surge.pos.x -= g_SurgeMove.x;
			g_Surge.pos.y -= g_SurgeMove.y;
		}
	}

#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Dissolve :%f  x:%f z:%f\n", g_SurgeDissolve, g_Surge.pos.x, g_Surge.pos.z);


#endif

}


//=============================================================================
// 描画処理
//=============================================================================
void DrawBless(void)
{
	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// αテストを有効に
	SetAlphaTestEnable(TRUE);

	// ライティングを無効に
	SetLightEnable(FALSE);

	// フォグ無効
	SetFogEnable(FALSE);

	// 爆風の描画処理
	if(g_Blast.use)
	{
		// リムライトOn
		SetFuchi(1);

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Blast.scl.x, g_Blast.scl.y, g_Blast.scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Blast.rot.x, g_Blast.rot.y + XM_PI, g_Blast.rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Blast.pos.x, g_Blast.pos.y, g_Blast.pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		XMStoreFloat4x4(&g_Blast.mtxWorld, mtxWorld);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		// 透明にする基準値を更新
		SetShaderBahamutCircle(g_ALine, TRUE);

		// モデル描画
		DrawModel(&g_Blast.model);

		// リムライトOff
		SetFuchi(0);
	}

	// 波動の描画処理
	//if (g_Surge.use)
	//{
	//	// シェーダーの変更
	//	if(!g_SurgeFiring) SetShaderMode(SHADER_MODE_BAHAMUT_SURGE);

	//	// 加算合成に設定
	//	SetBlendState(BLEND_MODE_ADD);

	//	// ワールドマトリックスの初期化
	//	mtxWorld = XMMatrixIdentity();

	//	// スケールを反映
	//	mtxScl = XMMatrixScaling(g_Surge.scl.x, g_Surge.scl.y, g_Surge.scl.z);
	//	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	//	// 回転を反映
	//	mtxRot = XMMatrixRotationRollPitchYaw(g_Surge.rot.x, g_Surge.rot.y + XM_PI, g_Surge.rot.z);
	//	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	//	// 移動を反映
	//	mtxTranslate = XMMatrixTranslation(g_Surge.pos.x, g_Surge.pos.y, g_Surge.pos.z);
	//	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	//	XMStoreFloat4x4(&g_Surge.mtxWorld, mtxWorld);

	//	// ワールドマトリックスの設定
	//	SetWorldMatrix(&mtxWorld);

	//	// ディゾルブの設定
	//	SetDissolve(g_SurgeDissolve);

	//	// モデル描画
	//	DrawModel(&g_Surge.model);

	//	// 通常ブレンドに戻す
	//	SetBlendState(BLEND_MODE_ALPHABLEND);
	//}

	// αテストを無効に
	SetAlphaTestEnable(FALSE);

	// ライティングを有効に
	SetLightEnable(TRUE);

	// フォグ有効
	SetFogEnable(GetFogSwitch());

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// 爆風の取得
//=============================================================================
BLESS *GetBlast(void)
{
	return &g_Blast;
}


//=============================================================================
// 頂点情報の書き込み
//=============================================================================
void SetBlessVertex(DX11_MODEL *Model)
{
	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * g_Bless_Vertex[0].VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_Bless_Vertex[0].VertexArray;

		GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer);
	}
}


//=============================================================================
// チャージ開始
//=============================================================================
void SetBlessMove(void)
{
	g_move = TRUE;
	SetBlessParticleSwitch(TRUE);
}

//=============================================================================
// モーフィングが完了しているか
//=============================================================================
BOOL GetBlessMove(void)
{
	return g_move;
}


//=============================================================================
// ブレスのリセット処理
//=============================================================================
void ResetBless(void)
{
	// 時間をリセット
	g_SceneWait = 0;
	g_count = 0;
	g_move = FALSE;

	// 爆風をリセット
	g_Blast.use = FALSE;
	g_Blast.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Blast.scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_ALine = XMFLOAT3(0.0f, 400.0f, 0.0f);

	// 波動をリセット
	g_Surge.use = FALSE;
	g_Surge.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Surge.rot = XMFLOAT3(-XM_PI / 5, -XM_PI / 2, 0.0f);

	g_SurgeDissolve = 0.0f;
	g_SurgeFiring = FALSE;
	g_SurgeMove = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// BGMをフェードアウト
	SetSoundFade(SOUND_LABEL_BGM_bahamut, SOUNDFADE_OUT, 0.0f, 1.0f);
}


//=============================================================================
// 波動を飛ばす
//=============================================================================
void SetSurge(void)
{
	g_SurgeFiring = TRUE;

	// 座標調整
	BAHAMUT *bahamut = GetBahamut();
	g_Surge.pos = bahamut->pos;
	g_Surge.pos.y += 125.0f;
	g_Surge.pos.x += 85.0f;
}
