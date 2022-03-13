//=============================================================================
//
// モデル処理 [player.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "model.h"
#include "player.h"
#include "title_player.h"
#include "light.h"
#include "meshfield.h"
#include "lightning.h"
#include "fire.h"
#include "holy.h"
#include "water.h"
#include "magic_circle.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/player_body.obj"			// 読み込むモデル名
#define	MODEL_PARTS_CLOAK	"data/MODEL/player_cloak.obj"			// 読み込むモデル名
#define	MODEL_PARTS_FEMUR	"data/MODEL/player_femur.obj"			// 読み込むモデル名
#define	MODEL_PARTS_KNEE	"data/MODEL/player_knee.obj"			// 読み込むモデル名
#define	MODEL_PARTS_LEG		"data/MODEL/player_leg.obj"				// 読み込むモデル名
#define	MODEL_PARTS_CLOAK_MOVE_0	"data/MODEL/player_cloak_mof_0.obj"			// 読み込むモデル名
#define	MODEL_PARTS_CLOAK_MOVE_1	"data/MODEL/player_cloak_mof_1.obj"			// 読み込むモデル名
#define	MODEL_PARTS_CLOAK_MOVE_2	"data/MODEL/player_cloak_mof_2.obj"			// 読み込むモデル名
#define	MODEL_PARTS_CLOAK_MOVE_3	"data/MODEL/player_cloak_mof_3.obj"			// 読み込むモデル名
#define	MODEL_PARTS_WAND	"data/MODEL/wand.obj"					// 読み込むモデル名

#define MAX_PLAYER_HP		(1000)				// プレイヤーのHP
#define MAX_INVINCIBLE		(120)				// ダメージを食らった際の無敵時間

#define	VALUE_MOVE			(2.0f)				// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)		// 回転量

#define PLAYER_SHADOW_SIZE	(1.0f)				// 影の大きさ
#define PLAYER_OFFSET_Y		(20.0f)				// プレイヤーの足元をあわせる

#define PLAYER_PARTS_MAX	(8)					// プレイヤーのパーツの数

#define MAX_PLAYER_MOVE		(5)					// モーフィングのモデル数

#define WAND_DISSOLVE		(0.005f)			// 杖のディゾルブ速度

#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(10000.0f)										// ビュー平面のFarZ値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void RunTitleAnimation(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER		g_Player;						// プレイヤー

static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// プレイヤーのパーツ用

static BOOL			g_Load = FALSE;

static MODEL		g_Player_Vertex[MAX_PLAYER_MOVE];	// モーフィング用モデルの頂点情報が入ったデータ配列
static VERTEX_3D	*g_Vertex = NULL;				// 途中経過を記録する場所

static float		g_time;
static int			mof;
static BOOL			g_gait = TRUE;					// 歩くモーションに移行するかどうか

static float		g_Wand = 0.0f;					// 杖のディゾルブ管理
static BOOL			g_WandDissolve = FALSE;			// 杖のディゾルブ描画モードになっているか

static CAMERA		g_Camera;						// タイトル用のカメラ



// プレイヤーの階層アニメーションデータ
static INTERPOLATION_DATA move_tbl_cloak[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },

};

static INTERPOLATION_DATA move_tbl_cloak_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

static INTERPOLATION_DATA move_tbl_femur_right_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期後半
};

static INTERPOLATION_DATA move_tbl_femur_left_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期後半
};

static INTERPOLATION_DATA move_tbl_knee_left_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期後半
};

static INTERPOLATION_DATA move_tbl_knee_right_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期後半
};

static INTERPOLATION_DATA move_tbl_leg_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

static INTERPOLATION_DATA move_tbl_leg_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

static INTERPOLATION_DATA move_tbl_wand[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, -5.0f, 9.0f),	XMFLOAT3(0.0f, 0.0f, RadianSum(-45)),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTitlePlayer(void)
{
	LoadModel(MODEL_PLAYER, &g_Player.model, 0);
	g_Player.load = TRUE;

	g_Player.pos = { 0.0f, PLAYER_OFFSET_Y, 0.0f };
	g_Player.rot = { 0.0f, 0.0f, 0.0f };
	g_Player.scl = { 1.0f, 1.0f, 1.0f };

	g_Player.spd = 0.0f;			// 移動スピードクリア
	g_Player.size = PLAYER_SIZE;	// 当たり判定の大きさ
	g_Player.hitCheck = TRUE;
	g_Player.HP = MAX_PLAYER_HP;
	g_Player.hitTime = 0;
	g_Player.use = TRUE;

	// 階層アニメーション用の初期化処理
	g_Player.parent = NULL;			// 本体（親）なのでNULLを入れる

	// パーツの初期化
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		g_Parts[i].use = FALSE;

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

	g_Parts[0].use = TRUE;
	g_Parts[0].tbl_adr  = move_tbl_cloak_stop;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[0].tbl_size = sizeof(move_tbl_cloak_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[0].load = 1;
	LoadModel(MODEL_PARTS_CLOAK, &g_Parts[0].model, 1);

	g_Parts[1].use = TRUE;
	g_Parts[1].tbl_adr  = move_tbl_femur_right_stop;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[1].tbl_size = sizeof(move_tbl_femur_right_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[1].load = 1;
	LoadModel(MODEL_PARTS_FEMUR, &g_Parts[1].model, 0);

	g_Parts[2].use = TRUE;
	g_Parts[2].tbl_adr  = move_tbl_femur_left_stop;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[2].tbl_size = sizeof(move_tbl_femur_left_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[2].load = 1;
	LoadModel(MODEL_PARTS_FEMUR, &g_Parts[2].model, 0);

	g_Parts[3].use = TRUE;
	g_Parts[3].tbl_adr  = move_tbl_knee_right_stop;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[3].tbl_size = sizeof(move_tbl_knee_right_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[3].load = 1;
	LoadModel(MODEL_PARTS_KNEE, &g_Parts[3].model, 0);

	g_Parts[4].use = TRUE;
	g_Parts[4].tbl_adr  = move_tbl_knee_left_stop;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[4].tbl_size = sizeof(move_tbl_knee_left_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[4].load = 1;
	LoadModel(MODEL_PARTS_KNEE, &g_Parts[4].model, 0);

	g_Parts[5].use = TRUE;
	g_Parts[5].tbl_adr  = move_tbl_leg_right;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[5].tbl_size = sizeof(move_tbl_leg_right) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[5].load = 1;
	LoadModel(MODEL_PARTS_LEG, &g_Parts[5].model, 0);

	g_Parts[6].use = TRUE;
	g_Parts[6].tbl_adr  = move_tbl_leg_left;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[6].tbl_size = sizeof(move_tbl_leg_left) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[6].load = 1;
	LoadModel(MODEL_PARTS_LEG, &g_Parts[6].model, 0);

	g_Parts[7].use = TRUE;
	g_Parts[7].tbl_adr = move_tbl_wand;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[7].tbl_size = sizeof(move_tbl_wand) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[7].load = 1;
	LoadModel(MODEL_PARTS_WAND, &g_Parts[7].model, 0);


	// 親子関係
	g_Parts[0].parent = &g_Player;		// マントの親は本体
	g_Parts[1].parent = &g_Player;		// 右大腿の親は本体
	g_Parts[2].parent = &g_Player;		// 左大腿の親は本体
	g_Parts[3].parent = &g_Parts[1];	// 右膝の親は右大腿
	g_Parts[4].parent = &g_Parts[2];	// 左膝の親は左大腿
	g_Parts[5].parent = &g_Parts[3];	// 右下腿の親は右膝
	g_Parts[6].parent = &g_Parts[4];	// 左下腿の親は左膝
	g_Parts[7].parent = &g_Player;		// 杖の親は本体

	// モーフィングするオブジェクトの読み込み
	LoadObj(MODEL_PARTS_CLOAK, &g_Player_Vertex[0]);
	LoadObj(MODEL_PARTS_CLOAK_MOVE_0, &g_Player_Vertex[1]);
	LoadObj(MODEL_PARTS_CLOAK_MOVE_1, &g_Player_Vertex[2]);
	LoadObj(MODEL_PARTS_CLOAK_MOVE_2, &g_Player_Vertex[3]);
	LoadObj(MODEL_PARTS_CLOAK_MOVE_3, &g_Player_Vertex[4]);

	// 中身を配列として使用できるように仕様変更
	g_Vertex = new VERTEX_3D[g_Player_Vertex[0].VertexNum];

	// 差分(途中経過)の初期化
	for (int i = 0; i < g_Player_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Player_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse = g_Player_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal = g_Player_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Player_Vertex[0].VertexArray[i].TexCoord;
	}

	g_time = 0.0f;
	mof = 0;

	// カメラの初期化
	g_Camera.pos = { 10.0f, 40.0f, 80.0f };
	g_Camera.up = { 0.0f, 1.0f, 0.0f };
	g_Camera.at = { 25.0f, 30.0f, 0.0f };


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTitlePlayer(void)
{
	if (g_Load == FALSE) return;

	// モデルの解放処理
	if (g_Player.load)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTitlePlayer(void)
{
	// モーフィングの処理
	{
		int after, brfore;

		// モーフィングモデルの番号調整
		if (g_gait == FALSE)
		{	// 基本のモデルへモーフィングさせる
			after = 0;
			brfore = 1;
		}
		else
		{
			after = (mof + 1) % (MAX_PLAYER_MOVE -1) + 1;
			brfore = mof % (MAX_PLAYER_MOVE - 1) + 1;
		}

		// モーフィング処理
		for (int p = 0; p < g_Player_Vertex[0].VertexNum; p++)
		{
			g_Vertex[p].Position.x = g_Player_Vertex[after].VertexArray[p].Position.x - g_Player_Vertex[brfore].VertexArray[p].Position.x;
			g_Vertex[p].Position.y = g_Player_Vertex[after].VertexArray[p].Position.y - g_Player_Vertex[brfore].VertexArray[p].Position.y;
			g_Vertex[p].Position.z = g_Player_Vertex[after].VertexArray[p].Position.z - g_Player_Vertex[brfore].VertexArray[p].Position.z;

			g_Vertex[p].Position.x *= g_time;
			g_Vertex[p].Position.y *= g_time;
			g_Vertex[p].Position.z *= g_time;

			g_Vertex[p].Position.x += g_Player_Vertex[brfore].VertexArray[p].Position.x;
			g_Vertex[p].Position.y += g_Player_Vertex[brfore].VertexArray[p].Position.y;
			g_Vertex[p].Position.z += g_Player_Vertex[brfore].VertexArray[p].Position.z;
		}

		// 時間を進める
		g_time += 0.03f;
		if (g_time > 1.0f)
		{
			mof++;
			g_time = 0.0f;
		}

		// 頂点情報を上書き
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * g_Player_Vertex[0].VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_Vertex;

		UnloadModelVertex(&g_Parts[0].model);
		GetDevice()->CreateBuffer(&bd, &sd, &g_Parts[0].model.VertexBuffer);


		//// 頂点バッファに値をセットする
		//D3D11_MAPPED_SUBRESOURCE msr;
		//GetDeviceContext()->Map(g_Parts[0].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		//VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

		//// 全頂点情報を毎回上書きしているのはDX11ではこの方が早いからです
		//memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D));

		//GetDeviceContext()->Unmap(g_Parts[0].model.VertexBuffer, 0);

	}


	// 杖のディゾルブ処理
	g_Wand += WAND_DISSOLVE;
	if (g_Wand > 1.0f) g_Wand -= 1.0f;

	RunTitleAnimation();


	// カメラの設定も一緒に行ってしまう
	{
		// ビューマトリックス設定
		XMMATRIX mtxView;
		mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Camera.pos), XMLoadFloat3(&g_Camera.at), XMLoadFloat3(&g_Camera.up));
		SetViewMatrix(&mtxView);
		XMStoreFloat4x4(&g_Camera.mtxView, mtxView);

		XMMATRIX mtxInvView;
		mtxInvView = XMMatrixInverse(nullptr, mtxView);
		XMStoreFloat4x4(&g_Camera.mtxInvView, mtxInvView);


		// プロジェクションマトリックス設定
		XMMATRIX mtxProjection;
		mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

		SetProjectionMatrix(&mtxProjection);
		XMStoreFloat4x4(&g_Camera.mtxProjection, mtxProjection);

		SetShaderCamera(g_Camera.pos);
	}





#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Player:↑ → ↓ ←　Space\n");
	PrintDebugProc("Player:X:%f Y:%f Z:%f\n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
#endif
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTitlePlayer(void)
{
	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ライティングを無効に
	//SetLightEnable(FALSE);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);

	// モデル描画
	DrawModel(&g_Player.model);


	// パーツの階層アニメーション
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// 杖にディゾルブを付ける
		if (i == 7)
		{
			SetShaderMode(SHADER_MODE_WAND);
			g_WandDissolve = TRUE;
		}

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

		// ディゾルブの設定
		SetDissolve(g_Wand);

		// モデル描画
		DrawModel(&g_Parts[i].model);

		// シェーダーを元に戻す
		if (g_WandDissolve)
		{
			SetShaderType(SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ);
		}
	}

	// ライティングを有効に
	SetLightEnable(TRUE);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
PLAYER *GetTitlePlayer(void)
{
	return &g_Player;
}


//=============================================================================
// 歩いているかを管理する関数
//=============================================================================
void SetTitleGait(BOOL data)
{
	g_gait = data;
}


//=============================================================================
//　階層アニメーション
//=============================================================================
void RunTitleAnimation(void)
{
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// 移動処理
		int		index = (int)g_Parts[i].move_time;
		float	time = g_Parts[i].move_time - index;
		int		size = g_Parts[i].tbl_size;

		float dt = 1.0f / g_Parts[i].tbl_adr[index].frame;	// 1フレームで進める時間
		g_Parts[i].move_time += dt;					// アニメーションの合計時間に足す

		if (index > (size - 2))	// ゴールをオーバーしていたら、最初へ戻す
		{
			g_Parts[i].move_time = 0.0f;
			index = 0;
		}

		// 座標を求める	X = StartX + (EndX - StartX) * 今の時間
		XMVECTOR p1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].pos);	// 次の場所
		XMVECTOR p0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].pos);	// 現在の場所
		XMVECTOR vec = p1 - p0;
		XMStoreFloat3(&g_Parts[i].pos, p0 + vec * time);

		// 回転を求める	R = StartX + (EndX - StartX) * 今の時間
		XMVECTOR r1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].rot);	// 次の角度
		XMVECTOR r0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].rot);	// 現在の角度
		XMVECTOR rot = r1 - r0;
		XMStoreFloat3(&g_Parts[i].rot, r0 + rot * time);

		// scaleを求める S = StartX + (EndX - StartX) * 今の時間
		XMVECTOR s1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].scl);	// 次のScale
		XMVECTOR s0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].scl);	// 現在のScale
		XMVECTOR scl = s1 - s0;
		XMStoreFloat3(&g_Parts[i].scl, s0 + scl * time);
	}
}