//=============================================================================
//
// カメラ処理 [camera.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "model.h"
#include "player.h"
#include "enemy.h"
#include "bahamut.h"
#include "bahamut_bless.h"
#include "beam_particle.h"
#include "beam_particle_2.h"
#include "beam_orbit.h"
#include "lightning.h"
#include "tree.h"
#include "fade_white.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	POS_X_CAM			(0.0f)			// カメラの初期位置(X座標)
#define	POS_Y_CAM			(30.0f)			// カメラの初期位置(Y座標)
#define	POS_Z_CAM			(-80.0f)		// カメラの初期位置(Z座標)

#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(10000.0f)										// ビュー平面のFarZ値

#define	VALUE_MOVE_CAMERA	(2.0f)										// カメラの移動量
#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)								// カメラの回転量

#define OVERLOOKING_Y	(150.0f)			// 横からの俯瞰視点のY座標
#define OVERLOOKING_Z	(-250.0f)			// 横からの俯瞰視点のZ座標

#define OFFSET_X		(60)
#define OFFSET_Y		(50.0f)				// 通常視点の時の注視点調整
#define OFFSET_Z		(100.0f)			// 注視点の調整
#define CAMERA_OFFSET_X	(50.0f)				// カメラの視点調整

#define CAMERA_TRACKING	(20.0f)				// カメラの追跡速度(少なければ早い)

#define TURNCOUNT_MAX	(20)				// 振り向く動作を何フレームで行うか

#define SUMMON_SCENE_MAX		(sceneMAX)		// 召喚魔法の最大シーン数
#define SCENE_POP_UP			(0.5f)			// popシーン時のATの上昇速度
#define SCENE_POP_DISTANCE		(250.0f)		// popシーン時のATの上昇速度
#define SCENE_OPEN_AT_Y			(220.0f)		// openシーンの注視点調整
#define SCENE_BLESS_AT			(1.0f)			// blessシーンの注視点調整
#define SCENE_BLESS_COUNT_MAX	(10)			// blessシーンの注視点を移動させる時間

#define CAMERA_SHAKE_MAX		(7)				// 画面が揺れる最大値
#define CAMERA_SHAKE_INTERVAL	(2)				// 画面が揺れる間隔


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static CAMERA			g_Camera;		// カメラデータ

static int				g_ViewPortType = TYPE_FULL_SCREEN;

static float			g_CameraX[OFFSET_X];

static BOOL				OverLook = TRUE;

static int				g_turn;			// 振り向く動作を実行しているか
static int				g_turnCount;	// 振り向く動作が始まって何フレーム経過したかを記録

static XMFLOAT3			g_DebugAT;

static float			g_DebugY;		// デバッグ用にy座標を変更する

static int				g_NowScene;		// 現在のシーン

static XMFLOAT3			g_OffsetAT;		// 注視点調整用

static int				g_BlessCount;	// 注視点を移動させる時間を管理

static BOOL				g_Shake;		// 画面揺れをするかどうか
static int				g_ShakeCount;	// 画面が揺れる残り時間
static XMFLOAT3			g_ShakePos;		// どのくらい揺らすかの距離を保存


static BOOL				g_SummonScene[2][SUMMON_SCENE_MAX];		// 召喚魔法のどのシーンを再生しているかを管理(シーン進行度管理)
																// y軸 0:どのシーンまで進んだか  1:そのシーンを再生したか

// 魔法陣シーン
static INTERPOLATION_DATA g_MoveTbl_circle[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3(0.0f, 300.0f, 10.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.008f },
	{ XMFLOAT3(0.0f, 900.0f, 10.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// 召喚シーン
static INTERPOLATION_DATA g_MoveTbl_pop[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3(0.0f, 150.0f, 10.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0015f },
	{ XMFLOAT3(0.0f, 350.0f, 10.0f),		XMFLOAT3(0.0f, XM_PI, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// 翼を徐々に広げるシーン
static INTERPOLATION_DATA g_MoveTbl_fold[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3(250.0f, SCENE_OPEN_AT_Y, -250.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.02f },
	{ XMFLOAT3(270.0f, SCENE_OPEN_AT_Y, -270.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// 翼を一気に広げるシーン
static INTERPOLATION_DATA g_MoveTbl_open[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3(270.0f, SCENE_OPEN_AT_Y, -270.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.18f },
	{ XMFLOAT3(400.0f, SCENE_OPEN_AT_Y, -400.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// ブレスをチャージするシーン
static INTERPOLATION_DATA g_MoveTbl_charge[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3(50.0f, 0.0f, 60.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.001f },
	{ XMFLOAT3(80.0f, 0.0f, 100.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// ブレスシーン
static INTERPOLATION_DATA g_MoveTbl_bless[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3(80.0f, 0.0f, 100.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// ブレスシーン(2カメ)
static INTERPOLATION_DATA g_MoveTbl_bless_2camera[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3(300.0f, 100.0f, 0.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.0001f },
};

// 爆風シーン
static INTERPOLATION_DATA g_MoveTbl_blast[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3( 0.0f, 450.0f, 350.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.005f },
};

// 消えるシーン
static INTERPOLATION_DATA g_MoveTbl_end[] = {
	//座標									回転率					Z		拡大率		X		Y			時間
	{ XMFLOAT3(0.0f, 300.0f, -450.0f),		XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),	0.005f },
};


// シーンごとの移動テーブルアドレスを配列で管理
static INTERPOLATION_DATA *g_MoveTblAdr[] =
{
	g_MoveTbl_circle,
	g_MoveTbl_pop,
	g_MoveTbl_fold,
	g_MoveTbl_open,
	g_MoveTbl_charge,
	g_MoveTbl_bless,
	g_MoveTbl_bless_2camera,
	g_MoveTbl_blast,
	g_MoveTbl_end,
};







//=============================================================================
// 初期化処理
//=============================================================================
void InitCamera(void)
{
	g_Camera.pos = { POS_X_CAM, POS_Y_CAM, POS_Z_CAM };
	g_Camera.pos.z = OVERLOOKING_Z;
	g_Camera.at  = { 0.0f, 0.0f, 0.0f };
	g_Camera.up  = { 0.0f, 1.0f, 0.0f };
	g_Camera.rot = { 0.0f, 0.0f, 0.0f };
	g_Camera.time = 0.0f;
	g_Camera.moveTblNo = 0;
	g_Camera.tblMax = (sizeof(g_MoveTbl_circle) / sizeof(INTERPOLATION_DATA));

	// シーン管理配列の初期化
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < SUMMON_SCENE_MAX; x++)
		{
			g_SummonScene[y][x] = FALSE;
		}
	}
	// 魔法陣シーンにセット
	g_SummonScene[0][0] = TRUE;


	// 視点と注視点の距離を計算
	float vx, vz;
	vx = g_Camera.pos.x - g_Camera.at.x;
	vz = g_Camera.pos.z - g_Camera.at.z;
	g_Camera.len = sqrtf(vx * vx + vz * vz);
	
	// デバッグ用のAT調整変数
	g_DebugAT = { 0.0f, 0.0f, 0.0f };

	// ビューポートタイプの初期化
	g_ViewPortType = TYPE_FULL_SCREEN;

	// 変数の初期化
	g_NowScene = circle;
	g_OffsetAT = { 0.0f, 0.0f, 0.0f };
	g_BlessCount = 0;
}


//=============================================================================
// カメラの終了処理
//=============================================================================
void UninitCamera(void)
{

}


//=============================================================================
// カメラの更新処理
//=============================================================================
void UpdateCamera(void)
{
	BAHAMUT *bahamut = GetBahamut();
	BLESS *BahamutBlast = GetBlast();
	// 召喚魔法が発動しているかで処理を変更
	if (GetSummon())
	{
		int nowNo = (int)g_Camera.time;			// 整数分であるテーブル番号を取り出している
		int maxNo = g_Camera.tblMax;			// 登録テーブル数を数えている
		int nextNo = (nowNo + 1) % maxNo;		// 移動先テーブルの番号を求めている

		INTERPOLATION_DATA *tbl = g_MoveTblAdr[g_Camera.moveTblNo];	// 行動テーブルのアドレスを取得

		// XMFloat3型をXMVECTOR型へコンバート
		XMVECTOR pos1 = XMLoadFloat3(&tbl[nextNo].pos);
		XMVECTOR pos2 = XMLoadFloat3(&tbl[nowNo].pos);
		pos1 -= pos2;	// XYZ移動量を計算している

		// XMFloat3型をXMVECTOR型へコンバート
		XMVECTOR rot1 = XMLoadFloat3(&tbl[nextNo].rot);
		XMVECTOR rot2 = XMLoadFloat3(&tbl[nowNo].rot);
		rot1 -= rot2;	// XYZ回転量を計算している

		float nowTime = g_Camera.time - nowNo;	// 時間部分である少数を取り出している

		pos1 *= nowTime;	// 現在の移動量を計算している
		rot1 *= nowTime;	// 現在の回転量を計算している

		// 計算して求めた移動量、回転量、拡大率を現在の移動テーブルXYZに足している＝表示座標を求めている
		pos2 += pos1;	// 移動量
		rot2 += rot1;	// 回転量

		// XMVECTOR型をXMFloat3型へコンバート
		XMStoreFloat3(&g_Camera.pos, pos2);
		XMStoreFloat3(&g_Camera.rot, rot2);

		//// frameを使て時間経過処理をする
		//if ((int)g_Camera.time < maxNo)	// 登録テーブル最後までで移動を止める
		//{
		//	g_Camera.time += tbl[nowNo].frame;	// 時間を進めている
		//}

		// frameを使て時間経過処理をする
		g_Camera.time += tbl[nowNo].frame;	// 時間を進めている
		if ((int)g_Camera.time >= maxNo)			// 登録テーブル最後まで移動したか？
		{
			g_Camera.time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
		}


		// X座標をバハムートに合わせる
		g_Camera.pos.x += bahamut->pos.x;

		// シーン進行度の検索
		for (int i = 0; i < SUMMON_SCENE_MAX; i++)
		{
			// 進んだシーンのカメラワークが再生されていなければ処理
			if ((g_SummonScene[0][i] == TRUE) && (g_SummonScene[1][i] == FALSE))
			{
				// 再生済みに変更
				g_SummonScene[1][i] = TRUE;

				// 線形補間の進行度をリセット
				g_Camera.time = 0.0f;

				// 色々リセット
				g_OffsetAT = { 0.0f, 0.0f, 0.0f };
				g_BlessCount = 0;
				ResetInOut();

				// 進行度に合ったシーンをセット
				// 一緒にサウンドも再生
				switch (i)
				{
				case circle:
					g_Camera.moveTblNo = circle;
					g_Camera.tblMax = (sizeof(g_MoveTbl_circle) / sizeof(INTERPOLATION_DATA));
					g_NowScene = circle;

					PlaySound(SOUND_LABEL_SE_BAHAMUT_circle);
					break;

				case pop:
					g_Camera.moveTblNo = pop;
					g_Camera.tblMax = (sizeof(g_MoveTbl_pop) / sizeof(INTERPOLATION_DATA));
					g_NowScene = pop;

					PlaySound(SOUND_LABEL_SE_BAHAMUT_pop);
					break;

				case fold:
					g_Camera.moveTblNo = fold;
					g_Camera.tblMax = (sizeof(g_MoveTbl_fold) / sizeof(INTERPOLATION_DATA));
					g_NowScene = fold;
					break;

				case open:
					g_Camera.moveTblNo = open;
					g_Camera.tblMax = (sizeof(g_MoveTbl_open) / sizeof(INTERPOLATION_DATA));
					g_NowScene = open;

					PlaySound(SOUND_LABEL_SE_BAHAMUT_open);
					break;

				case charge:
					g_Camera.moveTblNo = charge;
					g_Camera.tblMax = (sizeof(g_MoveTbl_charge) / sizeof(INTERPOLATION_DATA));
					g_NowScene = charge;

					SetBeamParticleCharge(TRUE);
					break;

				case bless:
					g_Camera.moveTblNo = bless;
					g_Camera.tblMax = (sizeof(g_MoveTbl_bless) / sizeof(INTERPOLATION_DATA));
					g_NowScene = bless;

					SetFadeWhite(WHITE_IN_bless);
					SetSurge();

					SetBeamDelete();
					SetBeam2Delete();
					SetBeamParticleCharge(FALSE);
					SetBeamParticleSwitch(TRUE);
					SetBeamParticleSwitch2(TRUE);
					SetBeamOrbitSwitch(TRUE);

					PlaySound(SOUND_LABEL_SE_BAHAMUT_bless);
					break;

				case bless_2camera:
					g_Camera.moveTblNo = bless_2camera;
					g_Camera.tblMax = (sizeof(g_MoveTbl_bless_2camera) / sizeof(INTERPOLATION_DATA));
					g_NowScene = bless_2camera;

					SetFadeWhite(WHITE_IN_bless);
					SetSurge();

					SetBeamDelete();
					SetBeam2Delete();
					SetBeamOrbitDelete();
					SetBeamParticleSwitch(TRUE);
					SetBeamParticleSwitch2(TRUE);
					SetBeamOrbitSwitch(TRUE);

					PlaySound(SOUND_LABEL_SE_BAHAMUT_bless);
					break;

				case blast:
					g_Camera.moveTblNo = blast;
					g_Camera.tblMax = (sizeof(g_MoveTbl_blast) / sizeof(INTERPOLATION_DATA));
					g_NowScene = blast;

					SetCameraShake(270);
					SetFadeWhite(WHITE_IN_bless);
					SetSurge();

					SetBeamDelete();
					SetBeam2Delete();
					SetBeamOrbitDelete();
					SetBeamParticleSwitch(TRUE);
					SetBeamParticleSwitch2(TRUE);
					SetBeamOrbitSwitch(TRUE);

					PlaySound(SOUND_LABEL_SE_BAHAMUT_bless);
					PlaySound(SOUND_LABEL_SE_BAHAMUT_blast);
					break;

				case end:
					g_Camera.moveTblNo = end;
					g_Camera.tblMax = (sizeof(g_MoveTbl_end) / sizeof(INTERPOLATION_DATA));
					g_NowScene = end;
					break;
				}
			}
		}

		// 注視点(AT)と視点の管理
		{
			// 進行度に合った注視点をセット
			switch (g_NowScene)
			{
			case circle:
				// 注視点
				g_Camera.at = bahamut->pos;
				g_Camera.at.y = 0.0f;

				break;

			case pop:
				// 注視点
				g_Camera.at = bahamut->pos;
				g_OffsetAT.y += SCENE_POP_UP;
				g_Camera.at.y = g_OffsetAT.y;

				// 視点
				g_Camera.pos.x = bahamut->pos.x + sinf(g_Camera.rot.y) * SCENE_POP_DISTANCE;
				g_Camera.pos.z = bahamut->pos.z + cosf(g_Camera.rot.y) * SCENE_POP_DISTANCE;

				break;

			case fold:
				// 注視点
				g_Camera.at = bahamut->pos;
				g_Camera.at.y = SCENE_OPEN_AT_Y + 10.0f;

				break;

			case open:
				// 注視点
				g_Camera.at = bahamut->pos;
				g_Camera.at.y = SCENE_OPEN_AT_Y + 10.0f;

				break;

			case charge:
				// 注視点
				g_Camera.at = bahamut->pos;
				g_Camera.at.y += 135.0f;
				g_Camera.at.x += 85.0f;

				// 視点
				g_Camera.pos.x = g_Camera.at.x;
				g_Camera.pos.y = g_Camera.at.y + 20.0f;

				SetBeamParticleCharge(TRUE);
				break;

			case bless:
				// 注視点
				g_Camera.at = bahamut->pos;
				g_Camera.at.y += 135.0f;
				g_Camera.at.x += 85.0f;
				g_Camera.at.x += g_OffsetAT.x;
				g_Camera.at.y -= g_OffsetAT.y + 10.0f;
				g_Camera.at.z += g_OffsetAT.z;
				
				g_BlessCount++;
				if (g_BlessCount < SCENE_BLESS_COUNT_MAX)
				{
					g_OffsetAT.x += SCENE_BLESS_AT;
					g_OffsetAT.y += SCENE_BLESS_AT;
					g_OffsetAT.z += SCENE_BLESS_AT;
				}

				// 視点
				//g_Camera.pos.y = BahamutBless->pos.y + 20.0f;
				//g_Camera.pos = bahamut->pos;
				g_Camera.pos.y = bahamut->pos.y + 145.0f;
				//g_Camera.pos.x += 85.0f;

				break;

			case bless_2camera:
				// 注視点
				g_Camera.at = bahamut->pos;
				g_Camera.at.y += 80.0f;
				g_Camera.at.x += 85.0f;
				g_Camera.at.x += g_OffsetAT.x;
				g_Camera.at.y -= g_OffsetAT.y;

				g_BlessCount++;
				if (g_BlessCount < SCENE_BLESS_COUNT_MAX)
				{
					g_OffsetAT.x += SCENE_BLESS_AT;
					g_OffsetAT.y += SCENE_BLESS_AT;
				}

				// 視点
				g_Camera.pos.y = bahamut->pos.y + 145.0f;

				break;

			case blast:
				// 注視点
				g_Camera.at = bahamut->pos;
				g_Camera.at.x += BLAST_X_OFFSET - 100.0f;
				g_Camera.at.z -= 170.0f;
				g_Camera.at.y -= 60.0f;

				break;

			case end:

				break;
			}
		}

		SetCamera();
	}
	else
	{
		PLAYER *player = GetPlayer();

		// 俯瞰視点の切り替え
		if ((GetKeyboardTrigger(DIK_Z)) || (IsButtonTriggered(0, BUTTON_Y)))
		{
			if (OverLook)
			{
				OverLook = FALSE;
				g_Camera.len = 150.0f;
				g_Camera.pos.y = 60.0f;

				// 手前の木を表示
				SetTreeSwitch(TRUE);
			}
			else
			{
				OverLook = TRUE;

				// 手前の木を非表示
				SetTreeSwitch(FALSE);
			}
		}

		// 俯瞰視点かどうかで操作を変更
		if (OverLook)
		{	// 横からの俯瞰視点処理
			g_Camera.pos.y = OVERLOOKING_Y;

			float dis;	// x座標のカメラとプレイヤーの距離を保存

			PLAYER *player = GetPlayer();

			dis = player->pos.x - g_Camera.pos.x;
			g_Camera.pos.x += dis / CAMERA_TRACKING;

			dis = player->pos.z - g_Camera.pos.z + OVERLOOKING_Z;
			g_Camera.pos.z += dis / CAMERA_TRACKING;
		}
		else
		{	// TPS視点処理
			// 振り向く動作の処理
			if ((g_turn) && (g_turnCount < TURNCOUNT_MAX))
			{
				// 少しずつ振り向く
				g_Camera.rot.y += XM_PI / TURNCOUNT_MAX;

				g_turnCount++;

				// 振り向き終わったら終了処理
				if (g_turnCount >= TURNCOUNT_MAX)
				{
					g_turn = FALSE;
					g_turnCount = 0;
				}
			}


			// TPS視点へ調整
			float dis;
			XMFLOAT3 ppos;

			ppos = player->pos;
			ppos.x -= sinf(player->rot.y + XM_PI / 2) * CAMERA_OFFSET_X;
			ppos.z -= cosf(player->rot.y + XM_PI / 2) * CAMERA_OFFSET_X;

			dis = (ppos.x + -sinf(player->rot.y + XM_PI) * OFFSET_Z) - g_Camera.pos.x;
			g_Camera.pos.x = dis / CAMERA_TRACKING + g_Camera.pos.x;

			dis = (ppos.z + -cosf(player->rot.y + XM_PI) * OFFSET_Z) - g_Camera.pos.z;
			g_Camera.pos.z = dis / CAMERA_TRACKING + g_Camera.pos.z;


			// カメラの視点をカメラのY軸回転に対応させている
			float disX, disZ;

			disX = (g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len) - g_Camera.pos.x;
			disZ = (g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len) - g_Camera.pos.z;

			g_Camera.pos.x += disX / CAMERA_TRACKING;
			g_Camera.pos.z += disZ / CAMERA_TRACKING;
		}
	}


	// 画面が揺れる処理
	if (g_Shake)
	{
		// 画面揺れの残り時間を減らす
		g_ShakeCount--;

		// 残り時間が残っている場合
		if (g_ShakeCount > 0)
		{
			g_Camera.pos.x += g_ShakePos.x;
			g_Camera.pos.y += g_ShakePos.y;
			g_Camera.pos.z += g_ShakePos.z;

			// 画面揺れ幅をリセット
			if (g_ShakeCount % CAMERA_SHAKE_INTERVAL == 0)
			{
				g_ShakePos.x = RamdomFloat(3, CAMERA_SHAKE_MAX, -CAMERA_SHAKE_MAX);
				g_ShakePos.y = RamdomFloat(3, CAMERA_SHAKE_MAX, -CAMERA_SHAKE_MAX);
				g_ShakePos.z = RamdomFloat(3, CAMERA_SHAKE_MAX, -CAMERA_SHAKE_MAX);
			}
		}
		else
		{
			g_Shake = FALSE;
		}
	}
	else if (!OverLook)
	{
		// 召喚魔法が発動していなかった場合
		if (!GetSummon())
		{
			// TPS視点時に画面揺れしたカメラを修正
			g_Camera.pos.y = 60.0f;
		}
	}




#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Camera:ZC QE TB YN UM R\n");
	PrintDebugProc("CameraPos:x:%f  y;%f  z:%f  \nscene:%d\n", g_Camera.pos.x, g_Camera.pos.y, g_Camera.pos.z, g_NowScene);
	PrintDebugProc("CameraPosAT:x:%f  y;%f  z:%f  \nscene:%d\n", g_Camera.at.x, g_Camera.at.y, g_Camera.at.z, g_NowScene);

	if ((GetKeyboardPress(DIK_M)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_Camera.len += 5.0f;
	}

	if ((GetKeyboardPress(DIK_N)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_Camera.len -= 5.0f;
	}

	if ((GetKeyboardPress(DIK_U)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_DebugAT.y += 1.0f;
	}

	if ((GetKeyboardPress(DIK_Y)) || (IsButtonTriggered(0, BUTTON_Y)))
	{
		g_DebugAT.y -= 1.0f;
	}




#endif
}


//=============================================================================
// カメラの更新
//=============================================================================
void SetCamera(void) 
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


//=============================================================================
// カメラの取得
//=============================================================================
CAMERA *GetCamera(void) 
{
	return &g_Camera;
}

//=============================================================================
// ビューポートの設定
//=============================================================================
void SetViewPort(int type)
{
	ID3D11DeviceContext *g_ImmediateContext = GetDeviceContext();
	D3D11_VIEWPORT vp;

	g_ViewPortType = type;

	// ビューポート設定
	switch (g_ViewPortType)
	{
	case TYPE_FULL_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_LEFT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_RIGHT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = (FLOAT)SCREEN_WIDTH / 2;
		vp.TopLeftY = 0;
		break;

	case TYPE_UP_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_DOWN_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = (FLOAT)SCREEN_HEIGHT / 2;
		break;


	}
	g_ImmediateContext->RSSetViewports(1, &vp);

}


int GetViewPortType(void)
{
	return g_ViewPortType;
}



//=============================================================================
// カメラの視点と注視点をセット
//=============================================================================
void SetCameraAT(XMFLOAT3 pos)
{
	PLAYER *player = GetPlayer();

	// 俯瞰視点かどうかで注視点を変更
	if (GetOverLook())
	{	// 俯瞰視点あり
		// カメラの注視点をプレイヤーの座標にしてみる
		g_Camera.at = pos;
		g_Camera.at.y = pos.y + OFFSET_Y;
	}
	else
	{
		// カメラの注視点をプレイヤーの座標にしてみる
		g_Camera.at = pos;
		g_Camera.at.y = pos.y + OFFSET_Y;
		g_Camera.at.x -= sinf(player->rot.y + XM_PI / 2) * CAMERA_OFFSET_X/ 2;
		g_Camera.at.z -= cosf(player->rot.y + XM_PI / 2) * CAMERA_OFFSET_X/ 2;

	}

#ifdef _DEBUG	// デバッグ情報を表示する

	g_Camera.at.y += g_DebugAT.y;

#endif

}


//=============================================================================
// 俯瞰視点かどうかを返す
//=============================================================================
BOOL GetOverLook(void)
{
	return OverLook;
}


//=============================================================================
// 振り向く動作
//=============================================================================
void SetTurnAround(void)
{
	g_turn = TRUE;
}


//=============================================================================
// シーンを進める
//=============================================================================
void SetSummonScene(int scene)
{
	g_SummonScene[0][scene] = TRUE;
}


//=============================================================================
// 現在どこのシーンかを取得する
//=============================================================================
int GetScene(void)
{
	return g_NowScene;
}


//=============================================================================
// シーン管理配列の初期化
//=============================================================================
void ResetSummonScene(void)
{
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < SUMMON_SCENE_MAX; x++)
		{
			g_SummonScene[y][x] = FALSE;
		}
	}

	// 魔法陣シーンにセット
	g_SummonScene[0][0] = TRUE;
	g_NowScene = circle;
	g_Camera.time = 0.0f;
}


//=============================================================================
// 画面を揺らす処理を開始
//=============================================================================
void SetCameraShake(int time)
{
	g_Shake = TRUE;
	g_ShakeCount = time;
	g_ShakePos = { 0.0f, 0.0f ,0.0f };
}