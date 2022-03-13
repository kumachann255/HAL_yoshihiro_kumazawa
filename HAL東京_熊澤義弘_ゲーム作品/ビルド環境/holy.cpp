//=============================================================================
//
// ホーリー処理 [holy.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "holy.h"
#include "player.h"
#include "enemy.h"
#include "math.h"
#include "collision.h"
#include "holy_particle.h"
#include "holy_orbit.h"
#include "sound.h"
#include "tutorial.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_HOLY0		"data/MODEL/circle.obj"		// 読み込むモデル名
#define	MODEL_HOLY1		"data/MODEL/sword.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(5.0f)					// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)			// 回転量

#define HOLY_SPEED			(0.018f)				// ホーリーの移動速度
#define HOLY_SHADOW_SIZE	(0.4f)					// 影の大きさ
#define HOLY_OFFSET_Y		(25.0f)					// モーフィングの足元をあわせる

#define SWORD_DISTANCE		(50.0f)					// 剣がプレイヤーからどのくらい離れているか
#define SWORD_CONTROL_POINT	(600.0f)				// 制御点がプレイヤーからどのくらい離れているか

#define HOLY_SCALE_COUNT	(20)					// 発生時の大きくなる速度

#define HOLY_OFFSET			(60 * MAX_HOLY)			// ホーリーが敵に当たってから消え始めるまでの時間
#define HOLY_DELCOUNT		(60)					// ホーリーが消えるまでのフレーム数

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void ResetHoly(int i);



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static HOLY				g_Holy[MAX_HOLY];				// モーフィング

static BOOL				g_Load = FALSE;

static MODEL			g_Holy_Vertex[MAX_HOLY_MOVE];	// モーフィング用モデルの頂点情報が入ったデータ配列

static HOLY_MOVE		g_Linear[MAX_HOLY_MOVE];

static ID3D11Buffer		*g_VertexBuffer = NULL;	// 頂点バッファ

static VERTEX_3D		*g_Vertex = NULL;

static BOOL				g_move = FALSE;

static XMFLOAT3			g_target;				// 発射先
static XMFLOAT3			g_ControlPoint[MAX_HOLY];			// 制御点
static XMFLOAT3			g_PosOld[MAX_HOLY];					// 1フレーム前の座標
static XMFLOAT3			g_distance[MAX_HOLY];			// 1フレームで進んだ距離を記録

static float g_time;	// モーフィングの経過時間
static int mof;			// モーフィングのモデル番号

static int g_offset;	// ホーリーが敵に当たってから消え始めるまでの時間


static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(   0.0f, HOLY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
	{ XMFLOAT3(-200.0f, HOLY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*1 },
	{ XMFLOAT3(-200.0f, HOLY_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*0.5f },
	{ XMFLOAT3(   0.0f, HOLY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitHoly(void)
{
	for (int i = 0; i < MAX_HOLY; i++)
	{
		LoadModel(MODEL_HOLY0, &g_Holy[i].model, 1);
		g_Holy[i].load = TRUE;

		g_Holy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, HOLY_OFFSET_Y, 20.0f);
		g_Holy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Holy[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Holy[i].diffuse[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_Holy[i].spd = 0.0f;		// 移動スピードクリア
		g_Holy[i].size = HOLY_SIZE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Holy[0].model, &g_Holy[0].diffuse[0]);

		XMFLOAT3 pos = g_Holy[i].pos;
		pos.y -= (HOLY_OFFSET_Y - 0.1f);

		g_Holy[i].move_time = 0.0f;	// 線形補間用のタイマーをクリア
		g_Holy[i].tbl_adr = NULL;	// 再生するアニメデータの先頭アドレスをセット
		g_Holy[i].tbl_size = 0;		// 再生するアニメデータのレコード数をセット

		g_Holy[i].use = FALSE;			// TRUE:生きてる
		g_Holy[i].firing = FALSE;		// FALSE:発射していない
		g_Holy[i].time = 0.0f;

		g_ControlPoint[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);		// 制御点を初期化
	}


	// モーフィングするオブジェクトの読み込み
	LoadObj(MODEL_HOLY0, &g_Holy_Vertex[0]);
	LoadObj(MODEL_HOLY1, &g_Holy_Vertex[1]);

	// 中身を配列として使用できるように仕様変更
	g_Vertex = new VERTEX_3D[g_Holy_Vertex[0].VertexNum];

	// 差分(途中経過)の初期化
	for (int i = 0; i < g_Holy_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Holy_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse  = g_Holy_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal   = g_Holy_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Holy_Vertex[0].VertexArray[i].TexCoord;
	}

	g_time = 0.0f;
	mof = 0;

	// 目標地点を初期化
	g_target = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitHoly(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_HOLY; i++)
	{
		if (g_Holy[i].load)
		{
			UnloadModel(&g_Holy[i].model);
			g_Holy[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateHoly(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();

	// モーフィングを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_HOLY; i++)
	{
		if (g_Holy[i].use == TRUE)			// このモーフィングが使われている？
		{									// Yes
			// 少しずつ大きくなる
			if (g_Holy[i].scl.x < 1.0f)
			{
				g_Holy[i].scl.x += 1.0f / HOLY_SCALE_COUNT;
				g_Holy[i].scl.y += 1.0f / HOLY_SCALE_COUNT;
				g_Holy[i].scl.z += 1.0f / HOLY_SCALE_COUNT;
			}

			if (g_Holy[i].firing)
			{
				// 計算式が長くならないよう調整するため
				float t = g_Holy[i].time;
				float t1 = 1.0f - g_Holy[i].time;

				// ベジェ曲線の処理
				g_Holy[i].pos.x = ((t1 * t1) * g_Holy[i].pos.x) + (2 * t1 * t * g_ControlPoint[i].x) + ((t * t) * g_target.x);
				g_Holy[i].pos.z = ((t1 * t1) * g_Holy[i].pos.z) + (2 * t1 * t * g_ControlPoint[i].z) + ((t * t) * g_target.z);
				g_Holy[i].pos.y = ((t1 * t1) * g_Holy[i].pos.y) + (2 * t1 * t * g_ControlPoint[i].y) + ((t * t) * g_target.y);

				if (g_Holy[i].time < 1.0f) g_Holy[i].time += HOLY_SPEED;
				if (g_Holy[i].time >= 1.0f) g_offset++;		// 移動が終わってからカウント開始

				// 目標まで到達した場合に終了処理
				if ((g_move) && (g_Holy[i].time >= 1.0f) && (i == 0) && (g_offset >= HOLY_OFFSET))
				{
					for (int p = 0;p < g_Holy_Vertex[0].VertexNum;p++)
					{
						g_Vertex[p].Diffuse.w -= 1.0f / HOLY_DELCOUNT;
					}

					SetParticleSwitch(TRUE);
				}


				// 目標物に剣先が向くように
				{
					// 制御点までは制御点に剣先が向くように
					if (g_Holy[i].time < 0.12f)
					{
						g_Holy[i].direct.x = g_ControlPoint[i].x - g_Holy[i].pos.x;
						g_Holy[i].direct.y = g_ControlPoint[i].y - g_Holy[i].pos.y;
						g_Holy[i].direct.z = g_ControlPoint[i].z - g_Holy[i].pos.z;
					}
					else if (g_Holy[i].time < 0.8f)
					{
						g_Holy[i].direct.x = g_target.x - g_Holy[i].pos.x;
						g_Holy[i].direct.y = g_target.y - g_Holy[i].pos.y;
						g_Holy[i].direct.z = g_target.z - g_Holy[i].pos.z;
					}

					// エネミーまでの距離が2割以下になると剣先は固定
					if (g_Holy[i].time < 0.9f)
					{
						float x, z;
						x = g_target.x - g_Holy[i].pos.x;
						z = g_target.z - g_Holy[i].pos.z;

						// プレイヤーの位置で処理を変更
						if (fabsf(x) > fabsf(z))
						{
							// y軸回転の調整
							if ((g_Holy[i].direct.z > 0.0f) && (g_Holy[i].direct.x > 0.0f))
							{	// 左下
								g_Holy[i].rot.y = (XM_PI / 2) + atanf(g_Holy[i].direct.x / g_Holy[i].direct.z);
							}
							else if((g_Holy[i].direct.x < 0.0f) && (g_Holy[i].direct.z > 0.0f))
							{	// 右下
								g_Holy[i].rot.y = (XM_PI / 2) + atanf(g_Holy[i].direct.x / g_Holy[i].direct.z);
							}
							else if (g_Holy[i].direct.x < 0.0f)
							{	// 右上
								g_Holy[i].rot.y = (-XM_PI / 2) + atanf(g_Holy[i].direct.x / g_Holy[i].direct.z);
							}
							else // if (g_Holy[i].direct.x > 0.0f)
							{	// 左上
								g_Holy[i].rot.y = (-XM_PI / 2) + atanf(g_Holy[i].direct.x / g_Holy[i].direct.z);
							}


							// z軸回転の調整
							if (g_Holy[i].direct.x > 0.0f)
							{
								g_Holy[i].rot.z = (-XM_PI / 2) + atanf(g_Holy[i].direct.y / g_Holy[i].direct.x);
							}
							else
							{
								g_Holy[i].rot.z = (-XM_PI / 2) - atanf(g_Holy[i].direct.y / g_Holy[i].direct.x);
							}
						}
						else
						{
							// y軸回転の調整
							if ((g_Holy[i].direct.z > 0.0f) && (g_Holy[i].direct.x > 0.0f))
							{	// 左下
								g_Holy[i].rot.y = (-XM_PI / 2) - atanf(g_Holy[i].direct.z / g_Holy[i].direct.x);
							}
							else if ((g_Holy[i].direct.x < 0.0f) && (g_Holy[i].direct.z > 0.0f))
							{	// 右下
								g_Holy[i].rot.y = (XM_PI / 2) - atanf(g_Holy[i].direct.z / g_Holy[i].direct.x);
							}
							else if (g_Holy[i].direct.x < 0.0f)
							{	// 右上
								g_Holy[i].rot.y = (XM_PI / 2) - atanf(g_Holy[i].direct.z / g_Holy[i].direct.x);
							}
							else // if (g_Holy[i].direct.x > 0.0f)
							{	// 左上
								g_Holy[i].rot.y = (-XM_PI / 2) - atanf(g_Holy[i].direct.z / g_Holy[i].direct.x);
							}

							// x軸回転の調整
							if (g_Holy[i].direct.z > 0.0f)
							{
								g_Holy[i].rot.x = (XM_PI / 2) - atanf(g_Holy[i].direct.y / g_Holy[i].direct.z);
							}
							else
							{
								g_Holy[i].rot.x = (XM_PI / 2) + atanf(g_Holy[i].direct.y / g_Holy[i].direct.z);
							}
						}
					}
				}

				// 完全に透明になったら未使用へ
				if (g_Vertex[0].Diffuse.w <= 0.0f)
				{
					ResetHoly(i);
					SetParticleSwitch(FALSE);
				}
			}
			else 
			{
				// プレイヤーの背後に位置調整
				g_Holy[i].pos.x = player->pos.x + sinf(player->rot.y + XM_PI * (-0.3f + 0.15f * i)) * SWORD_DISTANCE;
				g_Holy[i].pos.z = player->pos.z + cosf(player->rot.y + XM_PI * (-0.3f + 0.15f * i)) * SWORD_DISTANCE;

				// プレイヤーの向きに応じてモデルの向きも変更
				g_Holy[i].rot.y = player->rot.y + XM_PI * (-0.8f + 0.15f * i);

				// 制御点を記録
				g_ControlPoint[i].x = player->pos.x + sinf(player->rot.y + XM_PI / 2) * SWORD_CONTROL_POINT * (-0.3f + 0.15f * i);
				g_ControlPoint[i].z = player->pos.z + cosf(player->rot.y + XM_PI / 2) * SWORD_CONTROL_POINT * (-0.3f + 0.15f * i);
				g_ControlPoint[i].y = player->pos.y + cosf(player->rot.z + XM_PI * (-0.5f + 0.25f * i + 0.01f)) * SWORD_CONTROL_POINT / 4;
			}

			// 差分を記録
			g_distance[i].x = g_Holy[i].pos.x - g_PosOld[i].x;
			g_distance[i].y = g_Holy[i].pos.y - g_PosOld[i].y;
			g_distance[i].z = g_Holy[i].pos.z - g_PosOld[i].z;


			g_PosOld[i].x = g_Holy[i].pos.x;
			g_PosOld[i].y = g_Holy[i].pos.y;
			g_PosOld[i].z = g_Holy[i].pos.z;



			// モーフィングの処理
			{
				int after, brfore;

				after = (mof + 1) % MAX_HOLY_MOVE;
				brfore = mof % MAX_HOLY_MOVE;

				for (int p = 0; p < g_Holy_Vertex[0].VertexNum; p++)
				{
					g_Vertex[p].Position.x = g_Holy_Vertex[after].VertexArray[p].Position.x - g_Holy_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y = g_Holy_Vertex[after].VertexArray[p].Position.y - g_Holy_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z = g_Holy_Vertex[after].VertexArray[p].Position.z - g_Holy_Vertex[brfore].VertexArray[p].Position.z;

					g_Vertex[p].Position.x *= g_time;
					g_Vertex[p].Position.y *= g_time;
					g_Vertex[p].Position.z *= g_time;

					g_Vertex[p].Position.x += g_Holy_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y += g_Holy_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z += g_Holy_Vertex[brfore].VertexArray[p].Position.z;
				}

				// 時間を進める
				if((g_move) && (g_time < 1.0f)) g_time += 0.005f;

				// 目標まで到達したら軌道を消す
				if(g_time < 1.0f) SetOrbitSwitch(FALSE);

				D3D11_SUBRESOURCE_DATA sd;
				ZeroMemory(&sd, sizeof(sd));
				sd.pSysMem = g_Vertex;

				// 頂点バッファに値をセットする
				D3D11_MAPPED_SUBRESOURCE msr;
				GetDeviceContext()->Map(g_Holy[i].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
				VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

				// 全頂点情報を毎回上書きしているのはDX11ではこの方が早いからです
				memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Holy_Vertex[0].VertexNum);

				GetDeviceContext()->Unmap(g_Holy[i].model.VertexBuffer, 0);
			}
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawHoly(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ライティングを無効に
	SetLightEnable(FALSE);

	// フォグ無効
	SetFogEnable(FALSE);

	for (int i = 0; i < MAX_HOLY; i++)
	{
		if (g_Holy[i].use == FALSE) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Holy[i].scl.x, g_Holy[i].scl.y, g_Holy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Holy[i].rot.x, g_Holy[i].rot.y + XM_PI, g_Holy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Holy[i].pos.x, g_Holy[i].pos.y, g_Holy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Holy[i].mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_Holy[i].model);
	}

	// ライティングを有効に
	SetLightEnable(TRUE);

	// フォグ有効
	SetFogEnable(GetFogSwitch());
}

//=============================================================================
// ホーリーの取得
//=============================================================================
HOLY *GetHoly()
{
	return &g_Holy[0];
}


//=============================================================================
// ホーリーのセット
//=============================================================================
void SetHoly(void)
{
	// モデルの初期化
	for (int i = 0; i < g_Holy_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Holy_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse = g_Holy_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal = g_Holy_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Holy_Vertex[0].VertexArray[i].TexCoord;
	}

	D3D11_SUBRESOURCE_DATA sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.pSysMem = g_Vertex;

	for (int i = 0; i < MAX_HOLY; i++)
	{
		g_Holy[i].use = TRUE;

		// 頂点バッファに値をセットする
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_Holy[i].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

		// 全頂点情報を毎回上書きしているのはDX11ではこの方が早いからです
		memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Holy_Vertex[0].VertexNum);

		GetDeviceContext()->Unmap(g_Holy[i].model.VertexBuffer, 0);
	}

	// 効果音再生
	PlaySound(SOUND_LABEL_SE_MAGIC_holy_set);
}


// 頂点情報の書き込み
void SetHolyVertex(DX11_MODEL *Model)
{
	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * g_Holy_Vertex[0].VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_Holy_Vertex[0].VertexArray;

		GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer);
	}
}


// モーフィング開始
void SetMove(void)
{
	if (g_move) return;
	g_move = TRUE;

	// 効果音再生
	PlaySound(SOUND_LABEL_SE_MAGIC_holy_morphing);
}

// モーフィングが完了しているか
BOOL GetMove(void)
{
	return g_move;
}


// 光魔法の発射
void SetFiringSword(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();

	float distance = 15000.0f, x, z;	// エネミーまでの距離計算に使用
	int num = 0;						// 一番近いエネミーの番号(-1は範囲外)

	for (int i = 0; i < MAX_HOLY; i++)
	{
		// 未使用であれば発射
		if (g_Holy[i].firing == FALSE)
		{
			g_Holy[i].firing = TRUE;

			// 一回だけ処理をする
			if (i == 0)
			{
				for (int e = 0; e < MAX_ENEMY; e++)
				{
					if (enemy[e].use)
					{
						// エネミーとの距離を計算
						x = g_Holy[2].pos.x - enemy[e].pos.x;
						z = g_Holy[2].pos.z - enemy[e].pos.z;

						// 一番遠いエネミーの情報を記録
						if (distance < x * x + z * z)
						{
							distance = x * x + z * z;
							num = e;
						}
					}
				}

				// 目標地点と制御点を初期化
				g_target = XMFLOAT3(0.0f, 0.0f, 0.0f);

				// 目標地点を記録
				g_target = enemy[num].pos;

				// 効果音再生
				PlaySound(SOUND_LABEL_SE_MAGIC_holy_firing);

				// 軌道のパーティクルを発生させる
				SetOrbitSwitch(TRUE);
			}
		}
	}
}


// ホーリーのリセット処理
void ResetHoly(int i)
{
	g_Holy[i].use = FALSE;
	g_Holy[i].firing = FALSE;
	g_Holy[i].time = 0.0f;
	g_Holy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, HOLY_OFFSET_Y, 20.0f);
	g_Holy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Holy[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Holy[i].direct = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_move = FALSE;
	g_time = 0.0f;
	g_offset = 0;

	// チュートリアルクリア
	SetTutorialClear(tx_holy);
}


// 1フレームでどれくらい進んだかを返す
XMFLOAT3 GetHolyMoveValue(int i)
{
	return g_distance[i];
}