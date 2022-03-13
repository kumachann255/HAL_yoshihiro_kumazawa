//=============================================================================
//
// ウォーター処理 [water.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "water.h"
#include "player.h"
#include "enemy.h"
#include "math.h"
#include "collision.h"
#include "sound.h"
#include "tutorial.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_WATER0		"data/MODEL/polka_dots_0.obj"		// 読み込むモデル名
#define	MODEL_WATER1		"data/MODEL/polka_dots_1.obj"		// 読み込むモデル名
#define	MODEL_WATER2		"data/MODEL/polka_dots_2.obj"		// 読み込むモデル名
#define	MODEL_WATER_DROP_0	"data/MODEL/polka_drop_0.obj"		// 読み込むモデル名
#define	MODEL_WATER_DROP_1	"data/MODEL/polka_drop_1.obj"		// 読み込むモデル名
#define	MODEL_WATER_DROP_2	"data/MODEL/polka_drop_2.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define WATER_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define WATER_OFFSET_Y		(40.0f)						// モーフィングの足元をあわせる
#define WATER_OFFSET_X		(40.0f)						// 当たり判定の大きさ

#define WATER_HIGHT_DISTANCE	(80.0f)					// 水玉がプレイヤーからどのくらい離れているか
#define WATER_HIGHT			(120.0f)					// 水魔法が当たった時にどこまでエネミーを上げるか
#define WATER_DISTANCE		(50.0f)						// 水玉がプレイヤーからどのくらい離れているか

#define WATER_LIFE			(400)						// 敵に当たってから消えるまでの時間
#define WATER_PENETRATE_MAX		(120)					// 地面に浸透してから消えるまでの時間
#define WATER_PENETRATE_SPEED	(0.2f)					// 地面に浸透する速度

#define WATER_SCALE_COUNT	(30)						// 水玉が完成するまでのフレーム数
#define WATER_SCALE_MAX		(2.3f)						// 水玉の最大の大きさ

#define WATER_DISSOLVE		(0.003f)						// ディゾルブのスピード

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static WATER		g_Water[MAX_WATER];				// モーフィング

static BOOL			g_Load = FALSE;

static MODEL		g_Water_Vertex[MAX_WATER_MOVE];	// モーフィング用モデルの頂点情報が入ったデータ配列
static VERTEX_3D	*g_Vertex = NULL;				// 途中経過を記録する場所

static WATER_MOVE	g_Linear[MAX_WATER_MOVE];

static ID3D11Buffer	*g_VertexBuffer = NULL;			// 頂点バッファ

static BOOL			g_move = FALSE;

static XMFLOAT3		g_target;						// 発射先
static int			g_targetnum;					// 発射先エネミー番号
static XMFLOAT3		g_ControlPoint[MAX_WATER];		// 制御点
static XMFLOAT3		g_PosOld[MAX_WATER];			// 1フレーム前の座標

static float		g_time;
static int			mof;
static float		g_rot;
static BOOL			g_drop = FALSE;				// 落ちるモーションに移行するかどうか
static int			g_PenetrationCount = 0;		// 地面に浸透していく時間をカウント

static float		g_Dissolve;					// 波紋のディゾルブ

static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(   0.0f, WATER_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
	{ XMFLOAT3(-200.0f, WATER_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*1 },
	{ XMFLOAT3(-200.0f, WATER_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*0.5f },
	{ XMFLOAT3(   0.0f, WATER_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitWater(void)
{
	for (int i = 0; i < MAX_WATER; i++)
	{
		LoadModel(MODEL_WATER0, &g_Water[i].model, 1);
		g_Water[i].load = TRUE;

		g_Water[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Water[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Water[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Water[i].diffuse[0] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_Water[i].spd = 0.0f;			// 移動スピードクリア
		g_Water[i].size = WATER_SIZE;	// 当たり判定の大きさ
		g_Water[i].life = WATER_LIFE;
		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Water[0].model, &g_Water[0].diffuse[0]);

		XMFLOAT3 pos = g_Water[i].pos;
		pos.y -= (WATER_OFFSET_Y - 0.1f);

		g_Water[i].move_time = 0.0f;	// 線形補間用のタイマーをクリア

		g_Water[i].use = FALSE;			// TRUE:生きてる
		g_Water[i].firing = FALSE;		// FALSE:発射していない
		g_Water[i].time = 0.0f;
		g_ControlPoint[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);		// 制御点を初期化
	}


	// モーフィングするオブジェクトの読み込み
	LoadObj(MODEL_WATER0, &g_Water_Vertex[0]);
	LoadObj(MODEL_WATER1, &g_Water_Vertex[1]);
	LoadObj(MODEL_WATER2, &g_Water_Vertex[2]);

	LoadObj(MODEL_WATER_DROP_0, &g_Water_Vertex[3]);
	LoadObj(MODEL_WATER_DROP_1, &g_Water_Vertex[4]);
	LoadObj(MODEL_WATER_DROP_2, &g_Water_Vertex[5]);

	// 中身を配列として使用できるように仕様変更
	g_Vertex = new VERTEX_3D[g_Water_Vertex[0].VertexNum];

	// 差分(途中経過)の初期化
	for (int i = 0; i < g_Water_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Water_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse  = g_Water_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal   = g_Water_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Water_Vertex[0].VertexArray[i].TexCoord;

		g_Vertex[i].Diffuse.x *= 1.2f;
		g_Vertex[i].Diffuse.y *= 1.2f;
		g_Vertex[i].Diffuse.z *= 1.2f;
		g_Vertex[i].Diffuse.w = 0.7f;
	}

	g_time = 0.0f;
	g_rot = 0.0f;
	mof = 0;
	g_Dissolve = 0.0f;

	// 目標地点を初期化
	g_target = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Load = TRUE;
	return S_OK;
}


//=============================================================================
// 終了処理
//=============================================================================
void UninitWater(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_WATER; i++)
	{
		if (g_Water[i].load)
		{
			UnloadModel(&g_Water[i].model);
			g_Water[i].load = FALSE;
		}
	}

	g_Load = FALSE;
}


//=============================================================================
// 更新処理
//=============================================================================
void UpdateWater(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();

	// モーフィングを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_WATER; i++)
	{
		if (g_Water[i].use == TRUE)			// このモーフィングが使われている？
		{									// Yes
			// 少しずつ大きくなる
			if (g_Water[i].scl.x < WATER_SCALE_MAX)
			{
				g_Water[i].scl.x += WATER_SCALE_MAX / WATER_SCALE_COUNT;
				g_Water[i].scl.y += WATER_SCALE_MAX / WATER_SCALE_COUNT;
				g_Water[i].scl.z += WATER_SCALE_MAX / WATER_SCALE_COUNT;
			}

			// ディゾルブ処理
			g_Dissolve += WATER_DISSOLVE;
			if (g_Dissolve > 1.0f) g_Dissolve -= 1.0f;

			// 発射後の処理
			if (g_Water[i].firing == TRUE)
			{	// 発射フラグがオンの時
				XMFLOAT3 dis;
				dis.x = enemy[g_targetnum].pos.x - g_Water[i].pos.x;
				dis.y = enemy[g_targetnum].pos.y - g_Water[i].pos.y;
				dis.z = enemy[g_targetnum].pos.z - g_Water[i].pos.z;

				g_Water[i].pos.x += dis.x * 0.1f;
				g_Water[i].pos.y += dis.y * 0.1f;
				g_Water[i].pos.z += dis.z * 0.1f;


				// 水魔法が一定距離までエネミーに近づいたら
				if (g_Water[i].life > 0)
				{
					if ((fabsf(dis.x) < WATER_DISTANCE) && (fabsf(dis.y) < WATER_DISTANCE) && (fabsf(dis.z) < WATER_DISTANCE))
					{	// エネミーを浮かせて、回転させる
						float hight;
						hight = WATER_HIGHT - enemy[g_targetnum].pos.y;

						enemy[g_targetnum].pos.y += hight * 0.04f;

						// 水玉の寿命を減らす
						g_Water[i].life--;

						// ライフが減り始めたらSEを再生
						if (g_Water[i].life == WATER_LIFE - 1)
						{
							// SEの再生
							PlaySound(SOUND_LABEL_SE_MAGIC_water01);
						}
						else if (g_Water[i].life == WATER_LIFE / 2)
						{
							// SEの再生
							PlaySound(SOUND_LABEL_SE_MAGIC_water01);
						}
					}
				}
				else
				{
					// 寿命が尽きたら落ちるモーションへ移行
					SetPolkaDrop(i);
				}

			}
			else if (g_drop == FALSE)
			{
				// プレイヤーの頭上に位置調整
				g_Water[i].pos = player->pos;
				g_Water[i].pos.y += WATER_HIGHT_DISTANCE;
			}


			g_PosOld[i].x = g_Water[i].pos.x;
			g_PosOld[i].y = g_Water[i].pos.y;
			g_PosOld[i].z = g_Water[i].pos.z;


			// モーフィングの処理
			{
				int after, brfore;

				// モーフィングモデルの番号調整
				if (g_drop == FALSE)
				{
					after = (mof + 1) % (MAX_WATER_MOVE / 2);
					brfore = mof % (MAX_WATER_MOVE / 2);
				}
				else
				{
					after = (mof + 1) % (MAX_WATER_MOVE / 2) + (MAX_WATER_MOVE / 2);
					brfore = mof % (MAX_WATER_MOVE / 2) + (MAX_WATER_MOVE / 2);
				}

				// モーフィング処理
				for (int p = 0; p < g_Water_Vertex[0].VertexNum; p++)
				{
					g_Vertex[p].Position.x = g_Water_Vertex[after].VertexArray[p].Position.x - g_Water_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y = g_Water_Vertex[after].VertexArray[p].Position.y - g_Water_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z = g_Water_Vertex[after].VertexArray[p].Position.z - g_Water_Vertex[brfore].VertexArray[p].Position.z;

					g_Vertex[p].Position.x *= g_time;
					g_Vertex[p].Position.y *= g_time;
					g_Vertex[p].Position.z *= g_time;

					g_Vertex[p].Position.x += g_Water_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y += g_Water_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z += g_Water_Vertex[brfore].VertexArray[p].Position.z;
				}

				// 時間を進める
				if (g_drop == FALSE)
				{	// 水玉
					g_time += 0.03f;
					if (g_time > 1.0f)
					{
						mof++;
						g_time = 0.0f;
					}
				}
				else
				{	// 落ちるモーション
					if ((g_time < 1.0f) && (mof < 2)) g_time += 0.06f;
					if (g_time > 1.0f)
					{
						mof++;
						g_time = 0.0f;
					}
				}

				// 落ちるモーションが終わっているとき
				if ((g_drop) && (mof >= 2))
				{
					// 少しずつ地面に浸透させる
					g_Water[i].pos.y -= WATER_PENETRATE_SPEED;
					g_PenetrationCount++;
					
					// 地面に浸透したら終了処理
					if (g_PenetrationCount >= WATER_PENETRATE_MAX)
					{
						ResetWater(i);

						// チュートリアルクリア
						SetTutorialClear(tx_water);
					}
				}

				// 水玉を回転させる
				g_rot += 0.02f;
				g_Water[i].rot.y = g_rot;

				D3D11_SUBRESOURCE_DATA sd;
				ZeroMemory(&sd, sizeof(sd));
				sd.pSysMem = g_Vertex;

				// 頂点バッファに値をセットする
				D3D11_MAPPED_SUBRESOURCE msr;
				GetDeviceContext()->Map(g_Water[i].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
				VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

				// 全頂点情報を毎回上書きしているのはDX11ではこの方が早いからです
				memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Water_Vertex[0].VertexNum);

				GetDeviceContext()->Unmap(g_Water[i].model.VertexBuffer, 0);


				//// 頂点情報を上書き
				//D3D11_BUFFER_DESC bd;
				//ZeroMemory(&bd, sizeof(bd));
				//bd.Usage = D3D11_USAGE_DEFAULT;
				//bd.ByteWidth = sizeof(VERTEX_3D) * g_Water_Vertex[0].VertexNum;
				//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				//bd.CPUAccessFlags = 0;

				//D3D11_SUBRESOURCE_DATA sd;
				//ZeroMemory(&sd, sizeof(sd));
				//sd.pSysMem = g_Vertex;

				//UnloadModelVertex(&g_Water[i].model);
				//GetDevice()->CreateBuffer(&bd, &sd, &g_Water[i].model.VertexBuffer);
			}
		}
	}
}


//=============================================================================
// 描画処理
//=============================================================================
void DrawWater(void)
{
	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ライティングを無効に
	SetLightEnable(FALSE);

	// フォグ無効
	SetFogEnable(FALSE);

	// リムライトOn
	SetFuchi(1);

	for (int i = 0; i < MAX_WATER; i++)
	{
		if (g_Water[i].use == FALSE) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Water[i].scl.x, g_Water[i].scl.y, g_Water[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Water[i].rot.x, g_Water[i].rot.y + XM_PI, g_Water[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Water[i].pos.x, g_Water[i].pos.y, g_Water[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Water[i].mtxWorld, mtxWorld);

		// ディゾルブの設定
		SetDissolve(g_Dissolve);

		// モデル描画
		DrawModel(&g_Water[i].model);
	}

	// リムライトOff
	SetFuchi(0);

	// ライティングを有効に
	SetLightEnable(TRUE);

	// フォグ有効
	SetFogEnable(GetFogSwitch());

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// モーフィングの取得
//=============================================================================
WATER *GetWater()
{
	return &g_Water[0];
}


//=============================================================================
// 頂点情報の書き込み
//=============================================================================
void SetWaterVertex(DX11_MODEL *Model)
{
	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * g_Water_Vertex[0].VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_Water_Vertex[0].VertexArray;

		GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer);
	}
}


//=============================================================================
// モーフィング開始
//=============================================================================
void SetWaterMove(void)
{
	g_move = TRUE;
}


//=============================================================================
// モーフィングが完了しているか
//=============================================================================
BOOL GetWaterMove(void)
{
	return g_move;
}


//=============================================================================
// 水魔法の詠唱
//=============================================================================
void SetWater(void)
{
	for (int i = 0; i < MAX_WATER; i++)
	{
		if (g_Water[i].use == FALSE)
		{
			g_Water[i].use = TRUE;

			// SEの再生
			PlaySound(SOUND_LABEL_SE_MAGIC_water00);
			return;
		}
	}
}


//=============================================================================
// 水魔法が使用しているかどうかを返す
//=============================================================================
BOOL GetWaterUse(int i)
{
	return g_Water[i].use;
}


//=============================================================================
// 水魔法の発射
//=============================================================================
void SetFiringWater(void)
{
	PLAYER *player = GetPlayer();
	ENEMY *enemy = GetEnemy();

	float distance = 4000000.0f, x, z;	// エネミーまでの距離計算に使用
	int num = 0;					// 一番近いエネミーの番号(-1は範囲外)

	for (int i = 0; i < MAX_WATER; i++)
	{
		// 未使用であれば発射
		if (g_Water[i].firing == FALSE)
		{
			g_Water[i].firing = TRUE;

			// 一回だけ処理をする
			if (i == 0)
			{
				for (int e = 0; e < MAX_ENEMY; e++)
				{
					// エネミーとの距離を計算
					x = g_Water[0].pos.x - enemy[e].pos.x;
					z = g_Water[0].pos.z - enemy[e].pos.z;

					// 一番近いエネミーの情報を記録
					if (distance > x * x + z * z)
					{
						distance = x * x + z * z;
						num = e;
					}

				}

				// 目標地点と制御点を初期化
				g_target = XMFLOAT3(0.0f, 0.0f, 0.0f);
				g_ControlPoint[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);

				// 目標地点を記録
				g_target = enemy[num].pos;
				g_targetnum = num;
			}
		}
	}
}


//=============================================================================
// 水玉が落ちるモーションへの切り替え
//=============================================================================
void SetPolkaDrop(int i)
{
	g_Water[i].firing = FALSE;		// FALSE:発射していない
	if (g_drop == FALSE)
	{
		g_drop = TRUE;
		mof = 0;
		g_time = 0.0f;

		// SEの再生
		PlaySound(SOUND_LABEL_SE_MAGIC_water1);
	}
}


//=============================================================================
// 水魔法のリセット処理
//=============================================================================
void ResetWater(int i)
{
	g_Water[i].use = FALSE;
	g_Water[i].time = 0.0f;
	g_Water[i].life = WATER_LIFE;
	g_Water[i].scl = { 0.0f,0.0f,0.0f };
	g_drop = FALSE;
	mof = 0;
	g_time = 0.0f;
	g_PenetrationCount = 0;

	// モデルの初期化
	for (int i = 0; i < g_Water_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Water_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse = g_Water_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal = g_Water_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Water_Vertex[0].VertexArray[i].TexCoord;

		g_Vertex[i].Diffuse.x *= 1.2f;
		g_Vertex[i].Diffuse.y *= 1.2f;
		g_Vertex[i].Diffuse.z *= 1.2f;
		g_Vertex[i].Diffuse.w = 0.7f;
	}

	//D3D11_BUFFER_DESC bd;
	//ZeroMemory(&bd, sizeof(bd));
	//bd.Usage = D3D11_USAGE_DEFAULT;
	//bd.ByteWidth = sizeof(VERTEX_3D) * g_Water_Vertex[0].VertexNum;
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//bd.CPUAccessFlags = 0;

	//D3D11_SUBRESOURCE_DATA sd;
	//ZeroMemory(&sd, sizeof(sd));
	//sd.pSysMem = g_Vertex;

	//UnloadModelVertex(&g_Water[i].model);
	//GetDevice()->CreateBuffer(&bd, &sd, &g_Water[i].model.VertexBuffer);
}