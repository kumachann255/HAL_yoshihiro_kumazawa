//=============================================================================
//
// バハムート処理 [bahamut.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "model.h"
#include "bahamut.h"
#include "bahamut_particle.h"
#include "bahamut_bless.h"
#include "bless_particle.h"
#include "beam_orbit.h"
#include "beam_particle.h"
#include "beam_particle_2.h"
#include "lightning.h"
#include "player.h"
#include "sound.h"
#include "option.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_BAHAMUT		"data/MODEL/bahamut_body.obj"		// 読み込むモデル名
#define	MODEL_BAHAMUT_WING_0		"data/MODEL/bahamut_wing_fold.obj"		// 読み込むモデル名
#define	MODEL_BAHAMUT_WING_1		"data/MODEL/bahamut_wing_open_mid.obj"	// 読み込むモデル名
#define	MODEL_BAHAMUT_WING_2		"data/MODEL/bahamut_wing_open.obj"		// 読み込むモデル名
#define	MODEL_BAHAMUT_WING_3		"data/MODEL/bahamut_wing_base.obj"		// 読み込むモデル名
#define MODEL_BAHAMUT_CIRCLE_0		"data/MODEL/bahamut_circle_0.obj"		// 読み込むモデル名
#define MODEL_BAHAMUT_CIRCLE_1		"data/MODEL/bahamut_circle_1.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(5.0f)				// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)		// 回転量

#define BAHAMUT_PARTS_MAX	(3)					// プレイヤーのパーツの数
#define MAX_BAHAMUT_MOVE	(4)					// バハムートの翼のモーフィングモデル数

#define ROT_SPEED			(0.01f)				// 魔法陣を回転させる速度

#define POP_SPEED			(0.7f)				// バハムートが出現し始める速度(上昇速度)

#define BAHAMUT_BASE_HIGHT	(170.0f)			// バハムートの高さのベース

#define BAHAMUT_DISSOLVE	(0.004f)			// 杖のディゾルブ速度

#define BLESS_WAIT_TIME		(150)				// 召喚し終わってからブレスを開始するまでの時間
#define OPEN_SCENE_WAIT		(60)				// ブレスシーンに行くまでの待機時間

#define CIRCLE_ROT_SPEED	(0.001f)				// 魔法陣の回転速度

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
//void RunBahamutPartsAnimation(int i);
//void LoadParts(char *FileName, int i);
//void SetPopBahamut(int i);
//void SetBahamutVoice(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static BAHAMUT			g_Bahamut;							// バハムート

static BAHAMUT			g_Parts[BAHAMUT_PARTS_MAX];			// バハムートのパーツ用

static MODEL			g_Bahamut_Vertex[MAX_BAHAMUT_MOVE];	// モーフィング用モデルの頂点情報が入ったデータ配列
static VERTEX_3D		*g_Vertex = NULL;					// 途中経過を記録する場所

static BOOL				g_Summon;				// 召喚魔法が発動しているかどうか
static float			g_time;					// モーフィングの時間を管理
static int				morphingNo;				// モーフィングのモデル番号を管理
static BOOL				g_Entry;				// 登場モーションに移行するかどうか
static int				g_StopTime;				// 翼を広げた時に止める時間を管理
static bool				g_WingOpen;				// 翼を広げたかどうか

static int				g_BlessCount;			// ブレスを発射するまでの待機時間
static BOOL				g_POP;					// 出現し始めるかどうか
static BOOL				g_Charge;				// ブレスをチャージしているタイミングかどうか
static BOOL				g_Bless;				// ブレスを実行しているかどうか

static BOOL				g_Load = FALSE;

static float			g_Dissolve;					// 杖のディゾルブ管理




//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBahamut(void)
{
	LoadModel(MODEL_BAHAMUT, &g_Bahamut.model, 0);
	g_Bahamut.load = TRUE;

	g_Bahamut.pos = XMFLOAT3(0.0f, BAHAMUT_BASE_HIGHT, 0.0f);
	g_Bahamut.rot = XMFLOAT3(0.0f, -XM_PI / 2, 0.0f);
	//ag_Bahamut.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Bahamut.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Bahamut.move = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Bahamut.spd = 0.0f;			// 移動スピードクリア

	// モデルのディフューズを保存しておく。色変え対応の為。
	GetModelDiffuse(&g_Bahamut.model, &g_Bahamut.diffuse[0]);

	g_Bahamut.move_time = 0.0f;	// 線形補間用のタイマーをクリア
	g_Bahamut.tbl_adr = NULL;	// 再生するアニメデータの先頭アドレスをセット
	g_Bahamut.tbl_size = 0;		// 再生するアニメデータのレコード数をセット

	g_Bahamut.use = TRUE;			// TRUE:生きてる
	g_Bahamut.end = FALSE;			// FALSE:生きている

	// 階層アニメーション用の初期化処理
	g_Bahamut.parent = NULL;			// 本体（親）なのでNULLを入れる

	// ディゾルブの閾値
	g_Bahamut.dissolve = 1.0f;

	// 移動できるかのフラグ
	g_Bahamut.moveCan = TRUE;

	// 目標に到達して折り返したか
	g_Bahamut.turn = TRUE;

	// サインカーブ用のラジアン
	g_Bahamut.radian = 0.0f;


	// パーツの初期化
	for (int i = 0; i < BAHAMUT_PARTS_MAX; i++)
	{
		g_Parts[i].use = TRUE;

		// 位置・回転・スケールの初期設定
		g_Parts[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		// 階層アニメーション用のメンバー変数の初期化
		g_Parts[i].tbl_adr = NULL;		// 再生するアニメデータの先頭アドレスをセット
		g_Parts[i].move_time = 0.0f;	// 実行時間をクリア
		g_Parts[i].tbl_size = 0;		// 再生するアニメデータのレコード数をセット

		// パーツの読み込みはまだしていない
		g_Parts[i].load = 0;
	}


	// 親へのペアリング
	g_Parts[0].parent = &g_Bahamut;		// 右翼前半の親は本体
	//g_Parts[1].parent = &g_Bahamut;	// 右翼前半の親は本体
	//g_Parts[2].parent = &g_Bahamut;	// 右翼前半の親は本体

	g_Parts[2].pos = XMFLOAT3(0.0f, -BAHAMUT_BASE_HIGHT - 50.0f, 0.0f);

	// パーツモデルの読み込み
	LoadModel(MODEL_BAHAMUT_WING_0, &g_Parts[0].model, 1);
	LoadModel(MODEL_BAHAMUT_CIRCLE_0, &g_Parts[1].model, 0);
	LoadModel(MODEL_BAHAMUT_CIRCLE_1, &g_Parts[2].model, 0);


	// モーフィングするオブジェクトの読み込み
	LoadObj(MODEL_BAHAMUT_WING_0, &g_Bahamut_Vertex[0]);
	LoadObj(MODEL_BAHAMUT_WING_1, &g_Bahamut_Vertex[1]);
	LoadObj(MODEL_BAHAMUT_WING_2, &g_Bahamut_Vertex[2]);
	LoadObj(MODEL_BAHAMUT_WING_3, &g_Bahamut_Vertex[3]);

	// 中身を配列として使用できるように仕様変更
	g_Vertex = new VERTEX_3D[g_Bahamut_Vertex[0].VertexNum];

	// 差分(途中経過)の初期化
	for (int i = 0; i < g_Bahamut_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Bahamut_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse  = g_Bahamut_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal   = g_Bahamut_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Bahamut_Vertex[0].VertexArray[i].TexCoord;
	}

	// 変数の初期化
	g_Summon = FALSE;
	g_time = 0.0f;
	morphingNo = 0;
	g_Entry = FALSE;
	g_StopTime = 0;
	g_POP = FALSE;
	g_Dissolve = 0.0f;
	g_WingOpen = false;
	g_Charge = FALSE;
	g_Bless = FALSE;

	//g_Parts[1].use = FALSE;
	//g_Parts[2].scl = XMFLOAT3(1.1f, 1.1f, 1.1f);
	//g_Parts[1].pos.y = 50.0f;
	//g_Parts[2].pos.y = 50.0f;

	//g_Parts[0].use = TRUE;
	//g_Parts[0].tbl_adr = move_tbl_wing0_light;	// 再生するアニメデータの先頭アドレスをセット
	//g_Parts[0].tbl_size = sizeof(move_tbl_wing0_light) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	//g_Parts[0].load = 1;


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBahamut(void)
{
	if (g_Load == FALSE) return;

	if (g_Bahamut.load)
	{
		UnloadModel(&g_Bahamut.model);
		g_Bahamut.load = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateBahamut(void)
{
	// 召喚魔法が発動している場合
	if (g_Summon)
	{
		PLAYER *player = GetPlayer();

		// 封印している魔法陣の処理
		if (g_Parts[1].use)
		{
			g_Parts[1].pos.x = g_Bahamut.pos.x;
			g_Parts[1].pos.z = g_Bahamut.pos.z;

			g_Parts[2].pos.x = g_Bahamut.pos.x;
			g_Parts[2].pos.z = g_Bahamut.pos.z;

			// 少し回転させる
			g_Parts[1].rot.y += CIRCLE_ROT_SPEED;
			g_Parts[2].rot.y -= CIRCLE_ROT_SPEED;
		}

		// 魔法陣のディゾルブ処理
		if (g_Dissolve < 1.0f)
		{
			g_Dissolve += BAHAMUT_DISSOLVE;
		}
		else
		{	// 魔法陣が完成したらバハムートを出現させる
			g_POP = TRUE;
			SetSoundFade(SOUND_LABEL_SE_BAHAMUT_circle, SOUNDFADE_OUT, 0.0f, 0.5f);
		}

		if ((g_POP) && (g_Parts[2].pos.y < BAHAMUT_BASE_HIGHT + 100.0f))
		{
			g_Parts[2].pos.y += POP_SPEED;

			// 魔法陣が地面から出てきたらシーン変更
			if (g_Parts[2].pos.y > -BAHAMUT_BASE_HIGHT - 18.0f) SetSummonScene(pop);

			// バハムートの全身が出現したらシーン変更
			if (g_Parts[2].pos.y > BAHAMUT_BASE_HIGHT + 70.0f) SetSummonScene(fold);
		}
		else if (g_POP)
		{
			g_Entry = TRUE;
		}

		if (g_Bahamut.use == TRUE)			// このバハムートが使われている？
		{									// Yes



				// モーフィングの処理
			{
				int after, brfore;

				// モーフィングモデルの番号調整
				if (g_Bless)
				{
					after = 2;
					brfore = 1;
				}
				else if (g_Charge)
				{
					after = 1;
					brfore = 3;
				}
				else
				{
					after = (morphingNo + 1) % MAX_BAHAMUT_MOVE;
					brfore = morphingNo % MAX_BAHAMUT_MOVE;
				}

				// モーフィング処理
				for (int p = 0; p < g_Bahamut_Vertex[0].VertexNum; p++)
				{
					g_Vertex[p].Position.x = g_Bahamut_Vertex[after].VertexArray[p].Position.x - g_Bahamut_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y = g_Bahamut_Vertex[after].VertexArray[p].Position.y - g_Bahamut_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z = g_Bahamut_Vertex[after].VertexArray[p].Position.z - g_Bahamut_Vertex[brfore].VertexArray[p].Position.z;

					g_Vertex[p].Position.x *= g_time;
					g_Vertex[p].Position.y *= g_time;
					g_Vertex[p].Position.z *= g_time;

					g_Vertex[p].Position.x += g_Bahamut_Vertex[brfore].VertexArray[p].Position.x;
					g_Vertex[p].Position.y += g_Bahamut_Vertex[brfore].VertexArray[p].Position.y;
					g_Vertex[p].Position.z += g_Bahamut_Vertex[brfore].VertexArray[p].Position.z;
				}

				//// 時間を進める
				if (g_Entry)
				{	// 1回のモーフィングだけ行う
					if (g_time < 1.0f)
					{
						// 翼を徐々に広げる
						if (morphingNo == 0)	g_time += 0.012f;

						// 翼を一気に広げる
						if (morphingNo == 1)
						{
							g_time += 0.15f;
							g_WingOpen = true;
							SetSummonScene(open);
						}

						// ベースの位置に翼を戻す際に時間によって速度を変える
						if (g_StopTime <= 40)	g_time += 0.001f;
						if (g_StopTime > 40)	g_time += 0.018f;

						// ブレス時は一気に翼を開く
						if (morphingNo == 3)	g_time += 0.15f;
					}
					else
					{
						// 次のモーフィングへ以降
						if (morphingNo < 2)
						{
							morphingNo++;
							g_time = 0.0f;
						}
					}

					// 翼を広げ終わってからカウント開始
					if (morphingNo == 2) g_StopTime++;

					// モーフィングが終わってからカウント開始
					if ((morphingNo == 2) && (g_time >= 1.0f))
					{
						g_BlessCount++;

						// ブレスを開始
						if (g_BlessCount == BLESS_WAIT_TIME) SetBlessMove();

						// シーンを変更
						if(g_BlessCount > OPEN_SCENE_WAIT) SetSummonScene(charge);
					}
				}

				D3D11_SUBRESOURCE_DATA sd;
				ZeroMemory(&sd, sizeof(sd));
				sd.pSysMem = g_Vertex;

				// 頂点バッファに値をセットする
				D3D11_MAPPED_SUBRESOURCE msr;
				GetDeviceContext()->Map(g_Parts[0].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
				VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

				// 全頂点情報を毎回上書きしているのはDX11ではこの方が早いからです
				memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Bahamut_Vertex[0].VertexNum);

				GetDeviceContext()->Unmap(g_Parts[0].model.VertexBuffer, 0);
			}
		}
	}

	if ((GetKeyboardPress(DIK_J)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		SetSummon();
	}

#ifdef _DEBUG	// デバッグ情報を表示する

	if ((GetKeyboardPress(DIK_J)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		SetSummon();
	}

	if ((GetKeyboardPress(DIK_K)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_Summon = FALSE;
		g_Entry = FALSE;
		g_POP = FALSE;
		g_Charge = FALSE;
		g_Bless = FALSE;
		SetBlessParticleSwitch(FALSE);

		g_BlessCount = 0;
		morphingNo = 0;
		g_time = 0.0f;
		g_StopTime = 0;
		g_Dissolve = 0.0f;
		g_WingOpen = false;
		g_Bahamut.pos = XMFLOAT3(0.0f, BAHAMUT_BASE_HIGHT, 0.0f);
		g_Parts[2].pos = XMFLOAT3(0.0f, -BAHAMUT_BASE_HIGHT - 30.0f, 0.0f);
		SetBahamutParticleSwitch(FALSE);

		ResetSummonScene();

	}

	if ((GetKeyboardPress(DIK_H)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_POP = TRUE;
	}

	if ((GetKeyboardPress(DIK_I)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		morphingNo = 0;
		g_time = 0.0f;
		g_StopTime = 0;
		g_WingOpen = false;
		SetBahamutParticleSwitch(FALSE);
	}

	if ((GetKeyboardPress(DIK_N)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		ResetBless();
	}
#endif
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBahamut(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ライティングを無効に
	SetLightEnable(TRUE);

	// αテストを有効に
	SetAlphaTestEnable(TRUE);

	// リムライトOn
	SetFuchi(1);

	// シェーダーモードを変更
	SetShaderMode(SHADER_MODE_BAHAMUT);

	// 定数バッファを更新
	SetShaderBahamutCircle(g_Parts[2].pos, g_WingOpen);

	//if (g_Bahamut.use == FALSE) return;

	if (g_Bahamut.use)
	{
		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Bahamut.scl.x, g_Bahamut.scl.y, g_Bahamut.scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Bahamut.rot.x, g_Bahamut.rot.y + XM_PI, g_Bahamut.rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Bahamut.pos.x, g_Bahamut.pos.y, g_Bahamut.pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Bahamut.mtxWorld, mtxWorld);

		// モデル描画
		DrawModel(&g_Bahamut.model);

		// パーツの階層アニメーション
		for (int i = 0; i < BAHAMUT_PARTS_MAX; i++)
		{
			if ((i == 1) || (i == 2)) continue;

			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// スケールを反映
			mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 回転を反映
			mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			if (g_Parts[i].parent != NULL)	// 子供だったら親と結合する
			{
				mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
			}

			XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

			// 使われているなら処理する。ここまで処理している理由は他のパーツがこのパーツを参照している可能性があるから。
			if (g_Parts[i].use == FALSE) continue;

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// モデル描画
			DrawModel(&g_Parts[i].model);
		}
	}


	// リムライトOff
	SetFuchi(0);

	// シェーダーモードを変更
	SetShaderMode(SHADER_MODE_BAHAMUT_CIRCLE);

	for (int i = 1; i < 3; i++)
	{
		// バハムートを封印している魔法陣を描画
		if (g_Parts[i].use)
		{
			// 加算合成に設定
			SetBlendState(BLEND_MODE_ADD);

			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// スケールを反映
			mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 回転を反映
			mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			if (g_Parts[i].parent != NULL)	// 子供だったら親と結合する
			{
				mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
			}

			XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// ディゾルブの設定
			SetDissolve(g_Dissolve);

			// モデル描画
			DrawModel(&g_Parts[i].model);

			// 通常ブレンドに戻す
			SetBlendState(BLEND_MODE_ALPHABLEND);
		}
	}

	// αテストを無効に
	SetAlphaTestEnable(FALSE);

	// ライティングを有効に
	SetLightEnable(TRUE);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// バハムートの取得
//=============================================================================
BAHAMUT *GetBahamut()
{
	return &g_Bahamut;
}


//=============================================================================
// 召喚魔法が発動しているかを取得
//=============================================================================
BOOL GetSummon(void)
{
	return g_Summon;
}


//=============================================================================
// バハムートの翼が開いているかどうかを取得
//=============================================================================
bool GetWingOpen(void)
{
	return g_WingOpen;
}


//=============================================================================
// バハムートの終了処理
//=============================================================================
void ResetBahamut(void)
{
	g_Summon = FALSE;
	g_Entry = FALSE;
	g_POP = FALSE;
	g_Charge = FALSE;
	g_Bless = FALSE;
	SetBlessParticleSwitch(FALSE);

	g_BlessCount = 0;
	morphingNo = 0;
	g_time = 0.0f;
	g_StopTime = 0;
	g_Dissolve = 0.0f;
	g_WingOpen = false;
	g_Bahamut.pos = XMFLOAT3(0.0f, BAHAMUT_BASE_HIGHT, 0.0f);
	g_Parts[2].pos = XMFLOAT3(0.0f, -BAHAMUT_BASE_HIGHT - 50.0f, 0.0f);
	SetBahamutParticleSwitch(FALSE);

	ResetSummonScene();
	ResetBless();
	SetBeamOrbitDelete();
	SetBeamDelete();
	SetBeam2Delete();
	SetBeamOrbitSwitch(FALSE);


	// 音声のリセット
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_circle,	1.0f * SetSoundPala(se));
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_pop,		1.0f * SetSoundPala(se));
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_open,	1.5f * SetSoundPala(se));
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_charge,	0.7f * SetSoundPala(se));
	//SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_bless,	1.0f * SetSoundPala(se));

	//g_Bahamut.use = FALSE;			// TRUE:生きてる
	//g_Bahamut.end = TRUE;			// TRUE:生きてる

	//// ディゾルブの閾値
	//g_Bahamut.dissolve = 1.0f;

}


//=============================================================================
// 召喚魔法を発動
//=============================================================================
void SetSummon(void)
{
	g_Summon = TRUE;

	// バハムートの座標をプレイヤーに合わせる
	PLAYER *player = GetPlayer();
	// バハムートがエリア外を攻撃しないように調整
	if (player->pos.x > MAP_CIRCLE_LEFT)
	{
		g_Bahamut.pos.x = MAP_CIRCLE_LEFT;
	}
	else
	{
		g_Bahamut.pos.x = player->pos.x;
	}

	

	// BGMをフェードアウト
	SetTitleSoundFade(SOUND_LABEL_BGM_game, SOUNDFADE_OUT, 0.0f, 1.5f);
}


//=============================================================================
// ブレスをチャージしているときにモーフィングを変更する
//=============================================================================
void SetChargeMorphing(void)
{
	if (g_Charge) return;
	g_Charge = TRUE;
	g_time = 0.0f;
	g_BlessCount = 0;
}


//=============================================================================
// ブレスを実行しているときにモーフィングを変更する
//=============================================================================
void SetBlessMorphing(void)
{
	if (g_Bless) return;
	g_Bless = TRUE;
	g_time = 0.0f;
	morphingNo = 3;
}


//=============================================================================
// ブレスが終了したときにモーフィングを変更する
//=============================================================================
void SetBlessEndMorphing(void)
{
	if (!g_Bless) return;
	g_Charge = FALSE;
	g_Bless = FALSE;
	morphingNo = 2;
	g_time = 0.0f;
}

