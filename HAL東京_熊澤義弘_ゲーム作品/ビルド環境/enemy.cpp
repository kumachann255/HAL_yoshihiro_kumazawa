//=============================================================================
//
// エネミーモデル処理 [enemy.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "enemy.h"
#include "lightning.h"
#include "player.h"
#include "sound.h"
#include "bahamut.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/enemy0_body.obj"		// 読み込むモデル名
#define	MODEL_PARTS_WING_0	"data/MODEL/enemy0_wing0.obj"		// 読み込むモデル名
#define	MODEL_PARTS_WING_1	"data/MODEL/enemy0_wing1.obj"		// 読み込むモデル名
#define	MODEL_TREE			"data/MODEL/tree.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(1.0f)				// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)		// 回転量

#define ENEMY_SHADOW_SIZE	(0.4f)				// 影の大きさ
#define ENEMY_OFFSET_Y		(40.0f)				// エネミーの足元をあわせる

#define ENEMY_PARTS_MAX		(4)					// プレイヤーのパーツの数

#define DISSOLVE_SPEED		(0.007f)			// ディゾルブの速度
#define MAX_WATER_COUNT		(200)				// 酸素値

#define TURN_DISTANCE		(300.0f)			// エネミーが折り返すポイント
#define TURN_OFFSET			(5.0f)				// エネミーが折り返すポイントを大きくする
#define TARGET_SPEED		(300)				// 何フレームで目標まで到達するか

#define ENEMY_BASE_HIGHT	(70.0f)				// エネミーの高さのベース

#define MAX_POS_WAVE1		(300.0f)			// 1ウェイブで何体POPさせるか
#define MAX_POS_WAVE2		(700.0f)			// 2ウェイブで何体POPさせるか
#define MAX_POS_WAVE3		(1200.0f)			// 3ウェイブで何体POPさせるか
#define MAX_POS_WAVE4		(1950.0f)			// 4ウェイブで何体POPさせるか

#define POP_DISTANCE		(100)				// エネミーのポップする間隔

#define ENEMY_VOICE_MAX		(4)					// エネミーのボイスの種類

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void RunEnemyPartsAnimation(int i);
void ResetEnemy(int i);
void LoadParts(char *FileName, int i);
void SetPopEnemy(int i);
void SetEnemyVoice(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// エネミー

static ENEMY			g_Parts[MAX_ENEMY][ENEMY_PARTS_MAX];		// プレイヤーのパーツ用

static BOOL				g_Load = FALSE;

static XMFLOAT3			g_targetPos[MAX_ENEMY];

static int				g_WaveCount[MAX_WAVE];			// ウェーブが始まってからの経過時間(enemyを順番に出すため)

static int				g_Wave;							// 現在のウェーブを記録


static INTERPOLATION_DATA move_tbl_body[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, ENEMY_OFFSET_Y * 10.0f - 10.0f,  21.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, ENEMY_OFFSET_Y * 10.0f		  ,   19.5f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(0.0f, ENEMY_OFFSET_Y * 10.0f - 10.0f,  21.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

static INTERPOLATION_DATA move_tbl_wing0_light[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-12.5f, -3.0f,  -20.0f), XMFLOAT3(RadianSum(70), RadianSum(90), RadianSum(70)), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(-16.0f, -4.0f,  -10.0f), XMFLOAT3(RadianSum(-10), RadianSum(170), RadianSum(40)), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(-12.5f, -3.0f,  -20.0f), XMFLOAT3(RadianSum(70), RadianSum(90), RadianSum(70)), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

static INTERPOLATION_DATA move_tbl_wing0_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(12.5f, -3.0f,  -20.0f), XMFLOAT3(RadianSum(70), RadianSum(-90), RadianSum(-70)), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(16.0f, -4.0f,  -10.0f), XMFLOAT3(RadianSum(-10), RadianSum(-170), RadianSum(-40)), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(12.5f, -3.0f,  -20.0f), XMFLOAT3(RadianSum(70), RadianSum(-90), RadianSum(-70)), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

static INTERPOLATION_DATA move_tbl_wing1_light[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(-30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(-30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(0.0f, 0.0f,  -10.0f), XMFLOAT3(0.0f, RadianSum(140), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(-30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

static INTERPOLATION_DATA move_tbl_wing1_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(0.0f, 0.0f,  -10.0f), XMFLOAT3(0.0f, RadianSum(-140), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(0.0f, 0.0f,  -8.0f), XMFLOAT3(0.0f, RadianSum(30), 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		//LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = TRUE;

		g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, ENEMY_OFFSET_Y + i * 30.0f, 20.0f);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Enemy[i].move = XMFLOAT3(0.0f, 0.0f, 0.0f);

		g_Enemy[i].spd = 0.0f;			// 移動スピードクリア
		g_Enemy[i].size = ENEMY_SIZE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		g_Enemy[i].move_time = 0.0f;	// 線形補間用のタイマーをクリア
		g_Enemy[i].tbl_adr = NULL;		// 再生するアニメデータの先頭アドレスをセット
		g_Enemy[i].tbl_size = 0;		// 再生するアニメデータのレコード数をセット

		g_Enemy[i].waterCount = MAX_WATER_COUNT;

		g_Enemy[i].use = FALSE;			// TRUE:生きてる
		g_Enemy[i].end = FALSE;			// FALSE:生きている

		// 階層アニメーション用の初期化処理
		g_Enemy[i].parent = NULL;			// 本体（親）なのでNULLを入れる

		// ディゾルブの閾値
		g_Enemy[i].dissolve = 1.0f;

		// 移動できるかのフラグ
		g_Enemy[i].moveCan = TRUE;

		// 目標に到達して折り返したか
		g_Enemy[i].turn = TRUE;

		// サインカーブ用のラジアン
		g_Enemy[i].radian = 0.0f;

		// エネミーのベースの高さ
		g_Enemy[i].hight = ENEMY_BASE_HIGHT;

		// エネミーが向かう目標地点をリセット
		g_targetPos[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);

		// ウェーブが始まってからポップするまでの時間
		g_Enemy[i].popCount = 60;


		// パーツの初期化
		for (int p = 0; p < ENEMY_PARTS_MAX; p++)
		{
			g_Parts[i][p].use = FALSE;

			// 位置・回転・スケールの初期設定
			g_Parts[i][p].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[i][p].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[i][p].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

			// 階層アニメーション用のメンバー変数の初期化
			g_Parts[i][p].tbl_adr = NULL;		// 再生するアニメデータの先頭アドレスをセット
			g_Parts[i][p].move_time = 0.0f;	// 実行時間をクリア
			g_Parts[i][p].tbl_size = 0;		// 再生するアニメデータのレコード数をセット

			// パーツの読み込みはまだしていない
			g_Parts[i][p].load = 0;
		}

		g_Parts[i][0].use = TRUE;
		g_Parts[i][0].tbl_adr = move_tbl_wing0_light;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[i][0].tbl_size = sizeof(move_tbl_wing0_light) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[i][0].load = 1;

		g_Parts[i][1].use = TRUE;
		g_Parts[i][1].tbl_adr = move_tbl_wing1_light;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[i][1].tbl_size = sizeof(move_tbl_wing1_light) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[i][1].load = 1;

		g_Parts[i][2].use = TRUE;
		g_Parts[i][2].tbl_adr = move_tbl_wing0_left;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[i][2].tbl_size = sizeof(move_tbl_wing0_left) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[i][2].load = 1;

		g_Parts[i][3].use = TRUE;
		g_Parts[i][3].tbl_adr = move_tbl_wing1_left;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[i][3].tbl_size = sizeof(move_tbl_wing1_left) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[i][3].load = 1;

		// 親子関係
		g_Parts[i][0].parent = &g_Enemy[i];			// 右翼前半の親は本体
		g_Parts[i][1].parent = &g_Parts[i][0];		// 右翼後半の親は右翼前半
		g_Parts[i][2].parent = &g_Enemy[i];			// 左翼前半の親は本体
		g_Parts[i][3].parent = &g_Parts[i][2];		// 左翼後半の親は左翼前半

	}

	LoadModel(MODEL_ENEMY, &g_Enemy[0].model, 0);

	// パーツのロード
	LoadModel(MODEL_PARTS_WING_0, &g_Parts[0][0].model, 0);
	LoadModel(MODEL_PARTS_WING_1, &g_Parts[0][1].model, 0);
	LoadModel(MODEL_PARTS_WING_0, &g_Parts[0][2].model, 0);
	LoadModel(MODEL_PARTS_WING_1, &g_Parts[0][3].model, 0);


	// ウェーブの初期化(wave1)
	for (int i = 0; i < MAX_POP_WAVE1; i++)
	{
		g_Enemy[i].popCount = (i + 1) * POP_DISTANCE;
		g_Enemy[i].wave = 1;
	}

	// ウェーブの初期化(wave2)
	for (int i = MAX_POP_WAVE1; i < MAX_POP_WAVE2 + MAX_POP_WAVE1; i++)
	{
		g_Enemy[i].popCount = i * POP_DISTANCE;
		g_Enemy[i].wave = 2;
	}

	// ウェーブの初期化(wave3)
	for (int i = MAX_POP_WAVE2 + MAX_POP_WAVE1; i < MAX_POP_WAVE3 + MAX_POP_WAVE2 + MAX_POP_WAVE1; i++)
	{
		g_Enemy[i].popCount = i * POP_DISTANCE;
		g_Enemy[i].wave = 3;
	}

	// ウェーブの初期化(wave4)
	for (int i = MAX_POP_WAVE3 + MAX_POP_WAVE2 + MAX_POP_WAVE1; i < MAX_POP_WAVE4 + MAX_POP_WAVE3 + MAX_POP_WAVE2 + MAX_POP_WAVE1; i++)
	{
		g_Enemy[i].popCount = i * POP_DISTANCE / 2;
		g_Enemy[i].wave = 4;
	}


	// 位置調整
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		switch (g_Enemy[i].wave)
		{
		case 1:
			g_Enemy[i].pos.x = MAX_POS_WAVE1 + 100.0f;
			break;

		case 2:
			g_Enemy[i].pos.x = MAX_POS_WAVE2 + 100.0f;
			break;

		case 3:
			g_Enemy[i].pos.x = MAX_POS_WAVE3 + 100.0f;
			break;

		case 4:
			g_Enemy[i].pos.x = MAX_POS_WAVE4 + 100.0f;
			break;
		}
	}


	// 0番だけ線形補間で動かしてみる
	g_Enemy[0].move_time = 0.0f;		// 線形補間用のタイマーをクリア
	g_Enemy[0].tbl_adr = move_tbl_body;		// 再生するアニメデータの先頭アドレスをセット
	g_Enemy[0].tbl_size = sizeof(move_tbl_body) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	// 現在のウェーブを初期化
	g_Wave = 0;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	// 召喚魔法が発動したら移動不可
	if (GetSummon()) return;

	PLAYER *player = GetPlayer();

	// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		// プレイヤーのポジションに応じてウェーブを変更
		if (player->pos.x > MAX_POS_WAVE4)
		{	// ウェイブ1
			g_Wave = 4;
		}
		else if (player->pos.x > MAX_POS_WAVE3)
		{	// ウェイブ1
			g_Wave = 3;
		}
		else if (player->pos.x > MAX_POS_WAVE2)
		{	// ウェイブ1
			g_Wave = 2;
		}
		else if (player->pos.x > MAX_POS_WAVE1)
		{	// ウェイブ1
			g_Wave = 1;
		}

		// ウェーブに応じて出現エネミーを変更
		if ((g_Wave >= g_Enemy[i].wave) && (g_Enemy[i].popCount > 0)) g_Enemy[i].popCount--;

		// popカウントが0になったらエネミーを出現させる
		if (g_Enemy[i].popCount == 0)
		{
			SetPopEnemy(i);
		}



		if (g_Enemy[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes
			if (g_Enemy[i].moveCan)			// 線形補間を実行する？
			{								// 線形補間の処理
				//// 移動処理



				// プレイヤーの方向を取得
				float disX, disZ, royY;

				disX = player->pos.x - g_Enemy[i].pos.x;
				disZ = player->pos.z - g_Enemy[i].pos.z;

				if (disZ > 0.0f)
				{
					royY = (XM_PI)+atanf(disX / disZ);
				}
				else if (disX < 0.0f)
				{
					royY = (XM_PI / 2) - atanf(disZ / disX);
				}
				else
				{
					royY = (-XM_PI / 2) - atanf(disZ / disX);
				}

				if (g_Enemy[i].turn)
				{
					// エネミーが向かう目標地点をリセット
					g_targetPos[i].x = sinf(royY + XM_PI) * TURN_DISTANCE + player->pos.x;
					g_targetPos[i].z = cosf(royY + XM_PI) * TURN_DISTANCE + player->pos.z;

					g_Enemy[i].turn = FALSE;
					g_Enemy[i].move.x = (g_targetPos[i].x - g_Enemy[i].pos.x) / TARGET_SPEED;
					g_Enemy[i].move.z = (g_targetPos[i].z - g_Enemy[i].pos.z) / TARGET_SPEED;
					g_Enemy[i].rot.y = royY;
				}

				// エネミーを移動
				g_Enemy[i].pos.x += g_Enemy[i].move.x;
				g_Enemy[i].pos.z += g_Enemy[i].move.z;

				// 目標地点近くにいるかどうか
				if ((g_Enemy[i].pos.x > g_targetPos[i].x - TURN_OFFSET) &&
					(g_Enemy[i].pos.x < g_targetPos[i].x + TURN_OFFSET) &&
					(g_Enemy[i].pos.z > g_targetPos[i].z - TURN_OFFSET) &&
					(g_Enemy[i].pos.z < g_targetPos[i].z + TURN_OFFSET))
				{	// 近くにいる場合に方向転換する
					g_Enemy[i].turn = TRUE;
				}

				// サインカーブ
				g_Enemy[i].radian -= XM_PI / TARGET_SPEED;
				if (g_Enemy[i].radian < -XM_PI) g_Enemy[i].radian = 0.0f;
				g_Enemy[i].pos.y = sinf(g_Enemy[i].radian) * g_Enemy[i].hight + g_Enemy[i].hight + ENEMY_OFFSET_Y;
			}


			// パーツの階層アニメーション
			RunEnemyPartsAnimation(i);

			// ディゾルブフラグが立ったら
			if (g_Enemy[i].end)
			{
				g_Enemy[i].dissolve -= DISSOLVE_SPEED;

				if (g_Enemy[i].dissolve < 0.0f)
				{	// ディゾルブ処理が終わったら
					ResetEnemy(i);
				}
			}

		}
	}

#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Enemy:%f \n", g_Enemy[0].dissolve);
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	//SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == FALSE) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// ディゾルブの設定
		SetDissolve(g_Enemy[i].dissolve);

		// モデル描画
		DrawModel(&g_Enemy[0].model);

		// パーツの階層アニメーション
		for (int p = 0; p < ENEMY_PARTS_MAX; p++)
		{
			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// スケールを反映
			mtxScl = XMMatrixScaling(g_Parts[i][p].scl.x, g_Parts[i][p].scl.y, g_Parts[i][p].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 回転を反映
			mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i][p].rot.x, g_Parts[i][p].rot.y, g_Parts[i][p].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_Parts[i][p].pos.x, g_Parts[i][p].pos.y, g_Parts[i][p].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			if (g_Parts[i][p].parent != NULL)	// 子供だったら親と結合する
			{
				mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i][p].parent->mtxWorld));
			}

			XMStoreFloat4x4(&g_Parts[i][p].mtxWorld, mtxWorld);

			// 使われているなら処理する。ここまで処理している理由は他のパーツがこのパーツを参照している可能性があるから。
			if (g_Parts[i][p].use == FALSE) continue;

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// モデル描画
			DrawModel(&g_Parts[0][p].model);
		}
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}


//　階層アニメーション
void RunEnemyPartsAnimation(int i)
{
	for (int p = 0; p < ENEMY_PARTS_MAX; p++)
	{
		// 使われているなら処理する
		if ((g_Parts[i][p].use == TRUE) && (g_Parts[i][p].tbl_adr != NULL))
		{
			// 移動処理
			int		index = (int)g_Parts[i][p].move_time;
			float	time = g_Parts[i][p].move_time - index;
			int		size = g_Parts[i][p].tbl_size;

			float dt = 1.0f / g_Parts[i][p].tbl_adr[index].frame;	// 1フレームで進める時間
			g_Parts[i][p].move_time += dt;					// アニメーションの合計時間に足す

			if (index > (size - 2))	// ゴールをオーバーしていたら、最初へ戻す
			{
				g_Parts[i][p].move_time = 0.0f;
				index = 0;
			}

			// 座標を求める	X = StartX + (EndX - StartX) * 今の時間
			XMVECTOR p1 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 1].pos);	// 次の場所
			XMVECTOR p0 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 0].pos);	// 現在の場所
			XMVECTOR vec = p1 - p0;
			XMStoreFloat3(&g_Parts[i][p].pos, p0 + vec * time);

			// 回転を求める	R = StartX + (EndX - StartX) * 今の時間
			XMVECTOR r1 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 1].rot);	// 次の角度
			XMVECTOR r0 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 0].rot);	// 現在の角度
			XMVECTOR rot = r1 - r0;
			XMStoreFloat3(&g_Parts[i][p].rot, r0 + rot * time);

			// scaleを求める S = StartX + (EndX - StartX) * 今の時間
			XMVECTOR s1 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 1].scl);	// 次のScale
			XMVECTOR s0 = XMLoadFloat3(&g_Parts[i][p].tbl_adr[index + 0].scl);	// 現在のScale
			XMVECTOR scl = s1 - s0;
			XMStoreFloat3(&g_Parts[i][p].scl, s0 + scl * time);

		}
	}
}


// エネミーの終了処理
void ResetEnemy(int i)
{
	g_Enemy[i].use = FALSE;			// TRUE:生きてる
	g_Enemy[i].end = TRUE;			// TRUE:生きてる

	// ディゾルブの閾値
	g_Enemy[i].dissolve = 1.0f;

}


// パーツのロード
void LoadParts(char *FileName, int i)
{
	MODEL model;

	LoadObj(FileName, &model);

	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * model.VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = model.VertexArray;

		for (int p = 0 ; p < MAX_ENEMY; p++)
		{
			GetDevice()->CreateBuffer(&bd, &sd, &g_Parts[p][i].model.VertexBuffer);
		}
	}


	// インデックスバッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned short) * model.IndexNum;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = model.IndexArray;

		for (int p = 0; p < MAX_ENEMY; p++)
		{
			GetDevice()->CreateBuffer(&bd, &sd, &g_Parts[p][i].model.IndexBuffer);
		}
	}

	// サブセット設定
	for (int p = 0; p < MAX_ENEMY; p++)
	{
		g_Parts[p][i].model.SubsetArray = new DX11_SUBSET[model.SubsetNum];
		g_Parts[p][i].model.SubsetNum = model.SubsetNum;

		for (unsigned short v = 0; v < model.SubsetNum; v++)
		{
			g_Parts[p][i].model.SubsetArray[v].StartIndex = model.SubsetArray[v].StartIndex;
			g_Parts[p][i].model.SubsetArray[v].IndexNum = model.SubsetArray[v].IndexNum;

			g_Parts[p][i].model.SubsetArray[v].Material.Material = model.SubsetArray[v].Material.Material;

			D3DX11CreateShaderResourceViewFromFile(GetDevice(),
				model.SubsetArray[v].Material.TextureName,
				NULL,
				NULL,
				&g_Parts[p][i].model.SubsetArray[v].Material.Texture,
				NULL);
		}
	}

	delete[] model.VertexArray;
	delete[] model.IndexArray;
	delete[] model.SubsetArray;
}


// エネミーを出現させる
void SetPopEnemy(int i)
{
	g_Enemy[i].use = TRUE;
	g_Enemy[i].popCount = -1;	// 出現したというフラグ(-1)

	// 出現位置をランダムに
	float radian = RamdomFloat(3, 3.14f, 0.0f);
	g_Enemy[i].pos.z = cosf(radian) * TURN_DISTANCE;

	switch (g_Enemy[i].wave)
	{
	case 1:
		g_Enemy[i].pos.x = sinf(radian) * TURN_DISTANCE * 2 + MAX_POS_WAVE1 + TURN_DISTANCE;
		break;

	case 2:
		g_Enemy[i].pos.x = sinf(radian) * TURN_DISTANCE * 2 + MAX_POS_WAVE2 + TURN_DISTANCE;
		break;

	case 3:
		g_Enemy[i].pos.x = sinf(radian) * TURN_DISTANCE * 2 + MAX_POS_WAVE3 + TURN_DISTANCE;
		break;

	case 4:
		g_Enemy[i].pos.x = sinf(radian) * TURN_DISTANCE * 2 + MAX_POS_WAVE4 + TURN_DISTANCE;
		break;
	}

	SetEnemyVoice();
}


// エネミーが出現するときのボイスをランダムで再生
void SetEnemyVoice(void)
{
	int type = rand() % ENEMY_VOICE_MAX;

	switch (type)
	{
	case 0:
		PlaySound(SOUND_LABEL_SE_ENEMY_pop0);
		break;

	case 1:
		PlaySound(SOUND_LABEL_SE_ENEMY_pop1);
		break;

	case 2:
		PlaySound(SOUND_LABEL_SE_ENEMY_pop2);
		break;

	case 3:
		PlaySound(SOUND_LABEL_SE_ENEMY_pop3);
		break;
	}
}