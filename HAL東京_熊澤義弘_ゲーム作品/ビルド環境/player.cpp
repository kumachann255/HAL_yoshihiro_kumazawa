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
#include "light.h"
#include "meshfield.h"
#include "lightning.h"
#include "fire.h"
#include "holy.h"
#include "water.h"
#include "magic_circle.h"
#include "sound.h"
#include "tutorial.h"
#include "bahamut.h"
#include "chant_particle.h"
#include "bahamut.h"

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
#define	MODEL_PARTS_WAND_0	"data/MODEL/wand.obj"					// 読み込むモデル名
#define	MODEL_PARTS_WAND_1	"data/MODEL/wand_fire.obj"					// 読み込むモデル名
#define	MODEL_PARTS_WAND_2	"data/MODEL/wand_lightning.obj"					// 読み込むモデル名
#define	MODEL_PARTS_WAND_3	"data/MODEL/wand_water.obj"					// 読み込むモデル名
#define	MODEL_PARTS_WAND_4	"data/MODEL/wand_holy.obj"					// 読み込むモデル名

#define MAX_PLAYER_HP		(10)							// プレイヤーのHP
#define MAX_INVINCIBLE		(120)							// ダメージを食らった際の無敵時間

#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// 回転量

#define PLAYER_SHADOW_SIZE	(1.0f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(20.0f)							// プレイヤーの足元をあわせる

#define PLAYER_PARTS_MAX	(8)								// プレイヤーのパーツの数

#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)					// プレイヤーの回転量

#define MAX_PLAYER_MOVE		(5)								// モーフィングのモデル数
#define MAX_WAND_MOVE		(5)								// モーフィングのモデル数

#define WAND_OFFSET			(20)			// 杖を構える時間
#define WATER_CHANT_COUNT	(90)			// 水魔法の詠唱時間
#define WATER_FIRING_COUNT	(140)			// 水魔法の発動までの時間

#define LIGNHTNING_CHANT_COUNT	(90)		// 雷魔法の詠唱時間
#define LIGNHTNING_FIRING_COUNT	(120)		// 雷魔法の発動までの時間
#define LIGNHTNING_RESET_COUNT	(20)		// 雷魔法再発動までの時間

#define HOLY_CHANT_COUNT	(60)			// 光魔法の詠唱時間
#define HOLY_MORPHING_COUNT	(90)			// 光魔法のモーフィング開始時間
#define HOLY_FIRING_COUNT	(150)			// 光魔法の発動までの時間

#define WAND_DISSOLVE		(0.005f)		// 杖のディゾルブ速度

#define SUMMON_CHANT_COUNT	(240)			// 召喚魔法の詠唱時間


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void RunAnimation(int i, BOOL data);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER		g_Player;						// プレイヤー

static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// プレイヤーのパーツ用

static BOOL			g_Load = FALSE;

static MODEL		g_Player_Vertex[MAX_PLAYER_MOVE];	// モーフィング用モデルの頂点情報が入ったデータ配列
static VERTEX_3D	*g_Vertex = NULL;					// 途中経過を記録する場所

static MODEL		g_Wand_Vertex[MAX_WAND_MOVE];		// モーフィング用モデルの頂点情報が入ったデータ配列
static VERTEX_3D	*g_wandVertex = NULL;				// 途中経過を記録する場所
static COLOR		g_wandSubset;				// 色の途中経過を記録する場所

static float		g_time;					// モーフィングの時間を管理
static int			mof;					// モーフィングのモデル番号を管理
static BOOL			g_gait;					// 歩くモーションに移行するかどうか
static BOOL			g_gait_old;				// 前のフレームで歩いていたか
static BOOL			g_magic;				// 魔法モーションに移行するかどうか
static BOOL			g_magicChant;			// 詠唱モーションに移行するかどうか
static BOOL			g_magic_old;			// 前のフレームで魔法を使用していたか
static int			g_chantCount;			// 詠唱の経過時間
static int			g_LightningCount;		// 雷魔法のリセットまでの時間
static float		g_Speed;				// 歩行スピードを管理
static float		g_Wand;					// 杖のディゾルブ管理
static BOOL			g_WandDissolve;			// 杖のディゾルブ描画モードになっているか
static float		g_Wnadtime;				// 杖のモーフィングの時間を管理
static int			g_WandMorph;			// 杖のモーフィングのモデル番号を管理
static int			g_MagicType;			// 現在何を詠唱中か(4:詠唱していない)

static float		g_DebugY;				// デバッグ用にy座標を変更する

// プレイヤーの階層アニメーションデータ
static INTERPOLATION_DATA move_tbl_cloak[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(30), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },

};

static INTERPOLATION_DATA move_tbl_cloak_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};


static INTERPOLATION_DATA move_tbl_femur_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(45), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期後半
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(30), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 荷重応答期
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-5), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 38 / 4 },	// 立脚中期
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期前半
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-50), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期後半
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-20), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 前遊脚期
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(15), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 26 / 4 },	// 遊脚初期
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(35), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 遊脚中期
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期前半
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(45), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },		// 遊脚終期後半
};

static INTERPOLATION_DATA move_tbl_femur_right_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(2.25f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期後半
};


static INTERPOLATION_DATA move_tbl_femur_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-50), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期後半
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-20), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 前遊脚期
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(15), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 26 / 4 },	// 遊脚初期
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(35), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 遊脚中期
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期前半
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(45), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期後半
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(30), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 荷重応答期
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-5), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 38 / 4 },	// 立脚中期
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期前半
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(RadianSum(-50), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },		// 立脚終期後半
};

static INTERPOLATION_DATA move_tbl_femur_left_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-2.25f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期後半
};


static INTERPOLATION_DATA move_tbl_knee_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期後半
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 荷重応答期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 38 / 4 },	// 立脚中期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期前半
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期後半
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 前遊脚期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(90), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 26 / 4 },	// 遊脚初期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(25), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 遊脚中期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(15), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期前半
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },		// 遊脚終期後半
};

static INTERPOLATION_DATA move_tbl_knee_left_stop[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期後半
};


static INTERPOLATION_DATA move_tbl_knee_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期後半
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(40), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 前遊脚期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(90), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 26 / 4 },	// 遊脚初期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(25), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 遊脚中期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(15), 0.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期前半
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 12 / 6 },	// 遊脚終期後半
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 24 / 4 },	// 荷重応答期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 38 / 4 },	// 立脚中期
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(5), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 19 / 4 },	// 立脚終期前半
	{ XMFLOAT3(0.0f,-11.0f, 0.0f),	XMFLOAT3(RadianSum(0), 0.0f, 0.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },		// 立脚終期後半
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

// 詠唱中
static INTERPOLATION_DATA move_tbl_wand_chant[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 10.0f, 25.0f),		XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-90)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(2.0f, 10.0f, 20.0f),		XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-85)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 40 },
	{ XMFLOAT3(-2.0f, 10.0f, 22.0f),	XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-90)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(0.0f, 9.0f, 20.0f),		XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-95)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(0.0f, 10.0f, 25.0f),		XMFLOAT3(RadianSum(0), RadianSum(0), RadianSum(-90)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 1 },
};

// 炎と水魔法
static INTERPOLATION_DATA move_tbl_wand_fire_water[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(5.0f, -0.0f, 5.0f),		XMFLOAT3(RadianSum(-30), RadianSum(90), RadianSum(80)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};

// 雷と光魔法
static INTERPOLATION_DATA move_tbl_wand_lightning_holy[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(8.0f, 15.0f, 15.0f),		XMFLOAT3(RadianSum(10), RadianSum(30), RadianSum(30)),	XMFLOAT3(1.0f, 1.0f, 1.0f), 60 },
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
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
	g_Parts[7].tbl_adr = move_tbl_wand_chant;	// 再生するアニメデータの先頭アドレスをセット
	g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	g_Parts[7].load = 1;
	LoadModel(MODEL_PARTS_WAND_0, &g_Parts[7].model, 1);


	// 親子関係
	g_Parts[0].parent = &g_Player;		// マントの親は本体
	g_Parts[1].parent = &g_Player;		// 右大腿の親は本体
	g_Parts[2].parent = &g_Player;		// 左大腿の親は本体
	g_Parts[3].parent = &g_Parts[1];	// 右膝の親は右大腿
	g_Parts[4].parent = &g_Parts[2];	// 左膝の親は左大腿
	g_Parts[5].parent = &g_Parts[3];	// 右下腿の親は右膝
	g_Parts[6].parent = &g_Parts[4];	// 左下腿の親は左膝
	g_Parts[7].parent = &g_Player;		// 杖の親は本体

	// マント
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

	// 杖
	// モーフィングするオブジェクトの読み込み
	LoadObj(MODEL_PARTS_WAND_0, &g_Wand_Vertex[0]);
	LoadObj(MODEL_PARTS_WAND_1, &g_Wand_Vertex[1]);
	LoadObj(MODEL_PARTS_WAND_2, &g_Wand_Vertex[2]);
	LoadObj(MODEL_PARTS_WAND_3, &g_Wand_Vertex[3]);
	LoadObj(MODEL_PARTS_WAND_4, &g_Wand_Vertex[4]);

	// 中身を配列として使用できるように仕様変更
	g_wandVertex = new VERTEX_3D[g_Wand_Vertex[0].VertexNum];


	// 差分(途中経過)の初期化
	for (int i = 0; i < g_Wand_Vertex[0].VertexNum; i++)
	{
		g_wandVertex[i].Position = g_Wand_Vertex[0].VertexArray[i].Position;
		g_wandVertex[i].Diffuse  = g_Wand_Vertex[0].VertexArray[i].Diffuse;
		g_wandVertex[i].Normal   = g_Wand_Vertex[0].VertexArray[i].Normal;
		g_wandVertex[i].TexCoord = g_Wand_Vertex[0].VertexArray[i].TexCoord;
	}

	// 中身を配列として使用できるように仕様変更
	g_wandSubset.SubsetArray = new SUBSET[g_Wand_Vertex[0].SubsetNum];

	// 差分の初期化
	for (int i = 0; i < g_Wand_Vertex[0].SubsetNum; i++)
	{
		g_wandSubset.SubsetArray[i] = g_Wand_Vertex[0].SubsetArray[i];
	}
	g_wandSubset.SubsetNum = g_Wand_Vertex[0].SubsetNum;


	// 変数の初期化
	g_time = 0.0f;
	mof = 0;
	g_gait = FALSE;
	g_gait_old = FALSE;
	g_magic = FALSE;
	g_magicChant = FALSE;
	g_magic_old = FALSE;
	g_chantCount = 0;
	g_LightningCount = 0;
	g_Speed = VALUE_MOVE;
	g_Wand = 0.0f;
	g_WandDissolve = FALSE;
	g_Wnadtime = 0.0f;
	g_WandMorph = 0;
	g_DebugY = 0.0f;
	g_MagicType = null;

	// サウンド関連
	PlaySound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生する
	PauseSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを停止させておく

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
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
void UpdatePlayer(void)
{
	// 召喚魔法が発動したら操作不可
	if (GetSummon()) return;

	CAMERA *cam = GetCamera();

	// とりあえず歩いていない,魔法を使用していない設定
	SetGait(FALSE);
	g_magic = FALSE;
	g_magicChant = FALSE;
	SetCircle(TRUE, TRUE, null);
	SetChantParticleSwitch(FALSE);

	// 走るSEも一旦停止
	// 実際の足の動きとSEを合わせるために停止と再生を使用
	PauseSound(SOUND_LABEL_SE_PLAYER_gait);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_gait, 0.2f);			// 走るSEの音量調整

	// 最初にチュートリアルを開始
	SetTutorial(tx_start);

	// 炎魔法
	if (((GetKeyboardPress(DIK_UP)) || (IsButtonPressed(0, BUTTON_X))) &&
		GetTutorialClear(tx_start) && ((g_MagicType == fire) || (g_MagicType == null)))
	{
		g_chantCount++;
		g_magic = TRUE;
		SetCircle(TRUE, FALSE, fire);
		g_MagicType = fire;

		// 魔法発動ポーズへ移行(線形補間)
		XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand_fire_water->pos);	// 次の場所
		XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// 現在の場所
		XMVECTOR vec = p1 - p0;
		XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

		p1 = XMLoadFloat3(&move_tbl_wand_fire_water->rot);	// 次の場所
		p0 = XMLoadFloat3(&g_Parts[7].rot);			// 現在の場所
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);

		// 魔法発動ポーズをとるまでの待機時間
		if (g_chantCount > WAND_OFFSET * 3)
		{
			SetFireSwitch(TRUE);

			// チュートリアルクリア
			SetTutorialClear(tx_fire);
		}
	}
	else
	{
		SetFireSwitch(FALSE);	// 未使用に
		ResetFireTime();		// 連続発動時間をリセット
	}


	// 水魔法
	//if (((GetKeyboardPress(DIK_LEFT)) || (IsButtonPressed(0, BUTTON_C))))
	if (((GetKeyboardPress(DIK_LEFT)) || (IsButtonPressed(0, BUTTON_C))) &&
		GetTutorialClear(tx_lightning) && ((g_MagicType == water) || (g_MagicType == null)))
	{
		g_chantCount++;

		// 詠唱ポーズへ移行(階層アニメーション)
		g_Parts[7].tbl_adr = move_tbl_wand_chant;
		g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);
		g_magicChant = TRUE;
		SetCircle(TRUE, FALSE, water);
		g_MagicType = water;

		// 詠唱時間が一定を超えた時
		if (g_chantCount > WATER_CHANT_COUNT)
		{
			if (g_chantCount == WATER_CHANT_COUNT + 1) SetWater();

			// 詠唱時間がさらに一定を超えた時
			if (g_chantCount > WATER_FIRING_COUNT)
			{
				if ((GetWaterUse(0) == TRUE) && (g_chantCount == WATER_FIRING_COUNT + WAND_OFFSET))
				{
					SetFiringWater();
				}

				// 魔法発動ポーズへ移行(線形補間)
				g_magic = TRUE;
				g_magicChant = FALSE;

				XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand_fire_water->pos);	// 次の場所
				XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// 現在の場所
				XMVECTOR vec = p1 - p0;
				XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

				p1 = XMLoadFloat3(&move_tbl_wand_fire_water->rot);	// 次の場所
				p0 = XMLoadFloat3(&g_Parts[7].rot);			// 現在の場所
				vec = p1 - p0;
				XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);

			}
		}
	}


	// 雷魔法
	if (((GetKeyboardPress(DIK_RIGHT)) || (IsButtonPressed(0, BUTTON_B))) &&
		GetTutorialClear(tx_fire) && ((g_MagicType == lightning) || (g_MagicType == null)))
	{
		g_chantCount++;
		g_LightningCount = 0;

		// 詠唱ポーズへ移行(階層アニメーション)
		g_Parts[7].tbl_adr = move_tbl_wand_chant;
		g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);
		g_magicChant = TRUE;
		SetCircle(TRUE, FALSE, lightning);
		g_MagicType = lightning;

		// 詠唱時間が一定を超えた時
		if (g_chantCount > LIGNHTNING_FIRING_COUNT)
		{
			if (g_chantCount == LIGNHTNING_FIRING_COUNT + WAND_OFFSET)
			{
				SetLightning();
			}

			// 一定時間ごとに雷魔法を再発動
			if ((g_chantCount % LIGNHTNING_RESET_COUNT == 0) &&
				(g_chantCount > LIGNHTNING_FIRING_COUNT + WAND_OFFSET + LIGNHTNING_RESET_COUNT))
			{
				SetLightning();
			}
			// 魔法発動ポーズへ移行(線形補間)
			g_magic = TRUE;
			g_magicChant = FALSE;

			XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand_lightning_holy->pos);	// 次の場所
			XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// 現在の場所
			XMVECTOR vec = p1 - p0;
			XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

			p1 = XMLoadFloat3(&move_tbl_wand_lightning_holy->rot);	// 次の場所
			p0 = XMLoadFloat3(&g_Parts[7].rot);				// 現在の場所
			vec = p1 - p0;
			XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);
		}
	}
	else
	{
		g_LightningCount++;

		if (g_LightningCount == LIGNHTNING_RESET_COUNT * 2)
		{
			for (int z = 0; z < MAX_LIGHTNING; z++)
			{
				ResetLightning(z);
			}
		}

		SetLightningOnOff(FALSE);
	}


	// 光魔法
	if (((GetKeyboardPress(DIK_DOWN)) || (IsButtonPressed(0, BUTTON_A))) &&
		GetTutorialClear(tx_water) && ((g_MagicType == holy) || (g_MagicType == null)))
	//if (((GetKeyboardPress(DIK_DOWN)) || (IsButtonPressed(0, BUTTON_A))))
		{
		g_chantCount++;

		// 詠唱ポーズへ移行(階層アニメーション)
		g_Parts[7].tbl_adr = move_tbl_wand_chant;
		g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);
		g_magicChant = TRUE;
		SetCircle(TRUE, FALSE, holy);
		g_MagicType = holy;

		// 詠唱時間が一定を超えた時
		if (g_chantCount > HOLY_CHANT_COUNT)
		{
			HOLY *holy = GetHoly();

			if (g_chantCount == HOLY_CHANT_COUNT + 1) SetHoly();
			if (g_chantCount == HOLY_MORPHING_COUNT) SetMove();

			// 詠唱時間がさらに一定を超えた時
			if (g_chantCount > HOLY_FIRING_COUNT)
			{
				// 光魔法発射
				if (g_chantCount == HOLY_FIRING_COUNT + WAND_OFFSET)
				{
					SetFiringSword();
				}

				// 魔法発動ポーズへ移行(線形補間)
				g_magic = TRUE;
				g_magicChant = FALSE;

				XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand_lightning_holy->pos);	// 次の場所
				XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// 現在の場所
				XMVECTOR vec = p1 - p0;
				XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

				p1 = XMLoadFloat3(&move_tbl_wand_lightning_holy->rot);	// 次の場所
				p0 = XMLoadFloat3(&g_Parts[7].rot);			// 現在の場所
				vec = p1 - p0;
				XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);

			}
		}
	}


	// 召喚魔法
	if (((GetKeyboardPress(DIK_RIGHT) && GetKeyboardPress(DIK_LEFT)) || 
		(IsButtonPressed(0, BUTTON_C) && IsButtonPressed(0, BUTTON_B))))
	{
		g_chantCount++;

		// パーティクルを発生
		SetChantParticleSwitch(TRUE);

		// 詠唱ポーズへ移行(階層アニメーション)
		g_Parts[7].tbl_adr = move_tbl_wand_chant;
		g_Parts[7].tbl_size = sizeof(move_tbl_wand_chant) / sizeof(INTERPOLATION_DATA);
		g_magicChant = TRUE;
		g_MagicType = null;
		// SetCircle(TRUE, FALSE, holy);

		// 詠唱時間が一定を超えた時
		if (g_chantCount > SUMMON_CHANT_COUNT)
		{
			SetSummon();
		}
	}


	// 魔法が終わったら杖の位置を戻す
	if ((g_magicChant == FALSE) && (g_magic == FALSE))
	{
		g_chantCount = 0;

		XMVECTOR p1 = XMLoadFloat3(&move_tbl_wand->pos);	// 次の場所
		XMVECTOR p0 = XMLoadFloat3(&g_Parts[7].pos);			// 現在の場所
		XMVECTOR vec = p1 - p0;
		XMStoreFloat3(&g_Parts[7].pos, p0 + vec / 10);

		p1 = XMLoadFloat3(&move_tbl_wand->rot);	// 次の場所
		p0 = XMLoadFloat3(&g_Parts[7].rot);			// 現在の場所
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[7].rot, p0 + vec / 10);
	}

	// 現在のフレームで魔法を詠唱しているかを記録して、次のフレームで使用する
	g_magic_old = g_magic;



	// 当たり判定と無敵時間の管理
	if (g_Player.hitCheck == FALSE)
	{	// 無敵時間の場合
		g_Player.hitTime++;		// 時間を進める

		// プレイヤーの色を少し透明にする
		for (int p = 0; p < g_Player_Vertex[0].VertexNum; p++)
		{
			if (g_Player.hitTime % 10 > 5)
			{
				g_Vertex[p].Diffuse.w = 0.5f;
				//g_Vertex[p].Diffuse.z = 0.5f;
			}
			else
			{
				g_Vertex[p].Diffuse.w = 1.0f;
				//g_Vertex[p].Diffuse.z = 1.0f;
			}
		}
		// 移動速度を少し遅くする
		g_Speed = VALUE_MOVE / 2;
		if (g_Player.hitTime >= MAX_INVINCIBLE)
		{	// 無敵時間の経過時間が過ぎたら無敵を解除して、当たり判定を戻す
			g_Player.hitCheck = TRUE;
			g_Player.hitTime = 0;

			// 移動速度を戻す
			g_Speed = VALUE_MOVE;

			// プレイヤーの色を戻す
			for (int p = 0; p < g_Player_Vertex[0].VertexNum; p++)
			{
				g_Vertex[p].Diffuse = g_Player_Vertex[0].VertexArray[p].Diffuse;
			}
		}
	}




#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Player.pos.z = g_Player.pos.x = 0.0f;
		g_Player.rot.y = g_Player.dir = 0.0f;
		g_Player.spd = 0.0f;
	}
#endif

	// 俯瞰視点:上  TPS視点:前進
	if ((GetKeyboardPress(DIK_W)) || ((IsButtonPressed(0, BUTTON_UP))))
	{
		g_Player.spd = g_Speed;
		g_Player.dir = XM_PI;

		SetGait(TRUE);
		ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
	}


	// 俯瞰視点かどうかで移動処理を変更
	if (GetOverLook())
	{	// 俯瞰視点

		// 俯瞰視点時の操作
		// 横への移動はそのまま可能
		if ((GetKeyboardPress(DIK_A)) || (IsButtonPressed(0, BUTTON_LEFT)))
		{	// 左へ移動
			g_Player.spd = g_Speed;
			g_Player.dir = XM_PI / 2;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}
		if ((GetKeyboardPress(DIK_D)) || (IsButtonPressed(0, BUTTON_RIGHT)))
		{	// 右へ移動
			g_Player.spd = g_Speed;
			g_Player.dir = -XM_PI / 2;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}

		// 下への移動もそのまま可能
		if ((GetKeyboardPress(DIK_S)) || (IsButtonPressed(0, BUTTON_DOWN)))
		{	// 下へ移動
			g_Player.spd = g_Speed;
			g_Player.dir = 0.0f;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}

		// 斜め移動
		if ((GetKeyboardPress(DIK_A)) && (GetKeyboardPress(DIK_W)) ||
			(IsButtonPressed(0, BUTTON_LEFT)) && (IsButtonPressed(0, BUTTON_UP)))
		{	// 左上へ移動
			g_Player.spd = g_Speed;
			g_Player.dir = -XM_PI * 3 / 4;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}
		if ((GetKeyboardPress(DIK_D)) && (GetKeyboardPress(DIK_W)) ||
			(IsButtonPressed(0, BUTTON_RIGHT)) && (IsButtonPressed(0, BUTTON_UP)))
		{	// 右上へ移動
			g_Player.spd = g_Speed;
			g_Player.dir = XM_PI * 3 / 4;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}
		if ((GetKeyboardPress(DIK_A)) && (GetKeyboardPress(DIK_S)) ||
			(IsButtonPressed(0, BUTTON_LEFT)) && (IsButtonPressed(0, BUTTON_DOWN)))
		{	// 左下へ移動
			g_Player.spd = g_Speed;
			g_Player.dir = -XM_PI / 4;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}
		if ((GetKeyboardPress(DIK_D)) && (GetKeyboardPress(DIK_S)) ||
			(IsButtonPressed(0, BUTTON_RIGHT)) && (IsButtonPressed(0, BUTTON_DOWN)))
		{	// 右下へ移動
			g_Player.spd = g_Speed;
			g_Player.dir = XM_PI / 4;

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}


		// z軸方向の調整
		if ((g_Player.spd == g_Speed) && (g_Player.dir == XM_PI / 2)) g_Player.dir = -XM_PI / 2;
		else if ((g_Player.spd == g_Speed) && (g_Player.dir == -XM_PI / 2)) g_Player.dir = XM_PI / 2;
		
		// 移動処理する
		if (g_Player.spd > 0.0f)
		{

			g_Player.rot.y = -g_Player.dir;

			// 入力のあった方向へプレイヤーを向かせて移動させる
			g_Player.pos.x -= sinf(-g_Player.dir) * g_Player.spd;
			g_Player.pos.z -= cosf(g_Player.dir) * g_Player.spd;
		}
		
	}
	else
	{	// TPS視点

		// TPS視点時の操作
		// 横を向く
		if ((GetKeyboardPress(DIK_A)) || (IsButtonPressed(0, BUTTON_LEFT)))
		{	// 左へ向く
			cam->rot.y -= VALUE_ROTATE_CAMERA;
			if (cam->rot.y > XM_PI)
			{
				cam->rot.y -= XM_PI * 2.0f;
			}

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}

		if ((GetKeyboardPress(DIK_D)) || (IsButtonPressed(0, BUTTON_RIGHT)))
		{	// 右へ向く
			cam->rot.y += VALUE_ROTATE_CAMERA;
			if (cam->rot.y > XM_PI)
			{
				cam->rot.y -= XM_PI * 2.0f;
			}

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}

		// 振り向く動作を実施
		if ((GetKeyboardPress(DIK_S)) || (IsButtonPressed(0, BUTTON_DOWN)))
		{
			g_Player.spd = g_Speed;
			//g_Player.dir = 0.0f;

			if((GetKeyboardTrigger(DIK_S)) || (IsButtonPressed(0, BUTTON_DOWN)))
			{
				// 振り向くまでの補間
				SetTurnAround();
			}

			SetGait(TRUE);
			ReStartSound(SOUND_LABEL_SE_PLAYER_gait);	// 走るSEを再生
		}



		// 移動処理する
		{
			g_Player.rot.y = g_Player.dir + cam->rot.y;

			// 入力のあった方向へプレイヤーを向かせて移動させる
			g_Player.pos.x -= sinf(g_Player.rot.y) * g_Player.spd;
			g_Player.pos.z -= cosf(g_Player.rot.y) * g_Player.spd;
		}
	}


	// 歩いているときは歩行テーブルをセット
	if (g_gait)
	{
		g_Parts[0].tbl_adr = move_tbl_cloak;		// 再生するアニメデータの先頭アドレスをセット
		g_Parts[1].tbl_adr = move_tbl_femur_right;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[2].tbl_adr = move_tbl_femur_left;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[3].tbl_adr = move_tbl_knee_right;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[4].tbl_adr = move_tbl_knee_left;	// 再生するアニメデータの先頭アドレスをセット

		g_Parts[0].tbl_size = sizeof(move_tbl_cloak) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[1].tbl_size = sizeof(move_tbl_femur_right) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[2].tbl_size = sizeof(move_tbl_femur_left) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[3].tbl_size = sizeof(move_tbl_knee_right) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[4].tbl_size = sizeof(move_tbl_knee_left) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
	}
	else
	{
		g_Parts[0].tbl_adr = move_tbl_cloak_stop;		// 再生するアニメデータの先頭アドレスをセット
		g_Parts[1].tbl_adr = move_tbl_femur_right_stop;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[2].tbl_adr = move_tbl_femur_left_stop;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[3].tbl_adr = move_tbl_knee_right_stop;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[4].tbl_adr = move_tbl_knee_left_stop;	// 再生するアニメデータの先頭アドレスをセット

		g_Parts[0].tbl_size = sizeof(move_tbl_cloak_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[1].tbl_size = sizeof(move_tbl_femur_right_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[2].tbl_size = sizeof(move_tbl_femur_left_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[3].tbl_size = sizeof(move_tbl_knee_right_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット
		g_Parts[4].tbl_size = sizeof(move_tbl_knee_left_stop) / sizeof(INTERPOLATION_DATA);		// 再生するアニメデータのレコード数をセット

		// ニュートラルポジションにゆっくり戻す処理
		// マントの処理
		XMVECTOR p1 = XMLoadFloat3(&g_Parts[0].tbl_adr[0].rot);	// 次の場所
		XMVECTOR p0 = XMLoadFloat3(&g_Parts[0].rot);			// 現在の場所
		XMVECTOR vec = p1 - p0;
		XMStoreFloat3(&g_Parts[0].rot, p0 + vec / 10);

		// 右大腿の処理
		p1 = XMLoadFloat3(&g_Parts[1].tbl_adr[0].rot);	// 次の場所
		p0 = XMLoadFloat3(&g_Parts[1].rot);				// 現在の場所
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[1].rot, p0 + vec / 10);

		// 左大腿の処理
		p1 = XMLoadFloat3(&g_Parts[2].tbl_adr[0].rot);	// 次の場所
		p0 = XMLoadFloat3(&g_Parts[2].rot);				// 現在の場所
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[2].rot, p0 + vec / 10);

		// 右膝の処理
		p1 = XMLoadFloat3(&g_Parts[3].tbl_adr[0].rot);	// 次の場所
		p0 = XMLoadFloat3(&g_Parts[3].rot);				// 現在の場所
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[3].rot, p0 + vec / 10);

		// 左膝の処理
		p1 = XMLoadFloat3(&g_Parts[4].tbl_adr[0].rot);	// 次の場所
		p0 = XMLoadFloat3(&g_Parts[4].rot);				// 現在の場所
		vec = p1 - p0;
		XMStoreFloat3(&g_Parts[4].rot, p0 + vec / 10);
	}


	// マップ外かを判定して、外の場合は戻す
	{
		// x座標
		if (g_Player.pos.x > MAP_LIMIT_X_MAX)
		{
			g_Player.pos.x = MAP_LIMIT_X_MAX;
		}
		if (g_Player.pos.x < MAP_LIMIT_X_MIN)
		{
			g_Player.pos.x = MAP_LIMIT_X_MIN;
		}

		// z座標
		// 奥は円形の当たり判定になっているのでその処理を追加
		float radian = 0;
		if (g_Player.pos.x > MAP_CIRCLE_LEFT - MAP_OFFSTT_X)
		{
			radian = (g_Player.pos.x - MAP_CIRCLE_LEFT + MAP_OFFSTT_X) / (MAP_CIRCLE_R * 2) * XM_PI;
		}

		if ((g_Player.pos.x > MAP_CIRCLE_LEFT + MAP_OFFSTT_X) && (g_Player.pos.z > (sinf(radian) * MAP_CIRCLE_R)))
		{
			g_Player.pos.z = sinf(radian) * MAP_CIRCLE_R;
		}
		else if ((g_Player.pos.x <= MAP_CIRCLE_LEFT + MAP_OFFSTT_X) && (g_Player.pos.z > MAP_LIMIT_Z_MAX))
		{
			g_Player.pos.z = MAP_LIMIT_Z_MAX;
		}
		if ((g_Player.pos.x > MAP_CIRCLE_LEFT + MAP_OFFSTT_X) && (g_Player.pos.z < (sinf(-radian) * MAP_CIRCLE_R)))
		{
			g_Player.pos.z = sinf(-radian) * MAP_CIRCLE_R;
		}
		else if ((g_Player.pos.x <= MAP_CIRCLE_LEFT + MAP_OFFSTT_X) && (g_Player.pos.z < MAP_LIMIT_Z_MIN))
		{
			g_Player.pos.z = MAP_LIMIT_Z_MIN;
		}

		// チュートリアル中は動ける範囲が狭くなっている
		if (GetTutorialClear(tx_clear) == FALSE)
		{
			if (g_Player.pos.x > TUTORIAL_X)
			{
				g_Player.pos.x = TUTORIAL_X;
			}
		}
	}


	// レイキャストして足元の高さを求める
	XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };				// ぶつかったポリゴンの法線ベクトル（向き）
	XMFLOAT3 hitPosition;								// 交点
	hitPosition.y = g_Player.pos.y - PLAYER_OFFSET_Y;	// 外れた時用に初期化しておく
	bool ans = RayHitField(g_Player.pos, &hitPosition, &normal);
	g_Player.pos.y = hitPosition.y + PLAYER_OFFSET_Y;
	g_Player.pos.y = PLAYER_OFFSET_Y;

	g_Player.spd *= 0.3f;


	// 歩行の階層アニメーション(杖以外)
	// 階層アニメーション
	for (int i = 0; i < PLAYER_PARTS_MAX - 1; i++)
	{
		RunAnimation(i, g_gait);
	}

	// 杖の階層アニメーション
	RunAnimation(7, g_magicChant);

	// 前のフレームで歩いていて現在歩いていない場合に時間をリセット
	if ((g_gait_old == TRUE) && (g_gait == FALSE))	g_time = 0.0f;

	// 現在のフレームが歩いているかを記録して、次のフレームで使用する
	g_gait_old = g_gait;


	// マントのモーフィングの処理
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
		if (g_gait == FALSE)
		{	// 1回のモーフィングだけ行う
			if (g_time < 1.0f) g_time += 0.05f;
		}
		else
		{	// 歩くモーション
			g_time += 0.08f;
			if (g_time > 1.0f)
			{
				mof++;
				g_time = 0.0f;
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
		memcpy(pVtx, g_Vertex, sizeof(VERTEX_3D)*g_Player_Vertex[0].VertexNum);

		GetDeviceContext()->Unmap(g_Parts[0].model.VertexBuffer, 0);
	}


	// 杖のモーフィングの処理
	{
		int after, brfore;
		float time;

		// モーフィングモデルの番号調整
		{
			after = g_MagicType;
			brfore = null;
			time = g_Wnadtime;
		}

		// モーフィング処理
		for (int p = 0; p < g_Wand_Vertex[0].VertexNum; p++)
		{
			g_wandVertex[p].Position.x = g_Wand_Vertex[after].VertexArray[p].Position.x - g_Wand_Vertex[brfore].VertexArray[p].Position.x;
			g_wandVertex[p].Position.y = g_Wand_Vertex[after].VertexArray[p].Position.y - g_Wand_Vertex[brfore].VertexArray[p].Position.y;
			g_wandVertex[p].Position.z = g_Wand_Vertex[after].VertexArray[p].Position.z - g_Wand_Vertex[brfore].VertexArray[p].Position.z;

			g_wandVertex[p].Position.x *= g_Wnadtime;
			g_wandVertex[p].Position.y *= g_Wnadtime;
			g_wandVertex[p].Position.z *= g_Wnadtime;

			g_wandVertex[p].Position.x += g_Wand_Vertex[brfore].VertexArray[p].Position.x;
			g_wandVertex[p].Position.y += g_Wand_Vertex[brfore].VertexArray[p].Position.y;
			g_wandVertex[p].Position.z += g_Wand_Vertex[brfore].VertexArray[p].Position.z;


			// 法線
			g_wandVertex[p].Normal.x = g_Wand_Vertex[after].VertexArray[p].Normal.x - g_Wand_Vertex[brfore].VertexArray[p].Normal.x;
			g_wandVertex[p].Normal.y = g_Wand_Vertex[after].VertexArray[p].Normal.y - g_Wand_Vertex[brfore].VertexArray[p].Normal.y;
			g_wandVertex[p].Normal.z = g_Wand_Vertex[after].VertexArray[p].Normal.z - g_Wand_Vertex[brfore].VertexArray[p].Normal.z;

			g_wandVertex[p].Normal.x *= g_Wnadtime;
			g_wandVertex[p].Normal.y *= g_Wnadtime;
			g_wandVertex[p].Normal.z *= g_Wnadtime;

			g_wandVertex[p].Normal.x += g_Wand_Vertex[brfore].VertexArray[p].Normal.x;
			g_wandVertex[p].Normal.y += g_Wand_Vertex[brfore].VertexArray[p].Normal.y;
			g_wandVertex[p].Normal.z += g_Wand_Vertex[brfore].VertexArray[p].Normal.z;

			// 法線
			g_wandVertex[p].Normal.x = g_Wand_Vertex[after].SubsetArray->Material.Material.Diffuse.x - g_Wand_Vertex[brfore].VertexArray[p].Normal.x;
			g_wandVertex[p].Normal.y = g_Wand_Vertex[after].VertexArray[p].Normal.y - g_Wand_Vertex[brfore].VertexArray[p].Normal.y;
			g_wandVertex[p].Normal.z = g_Wand_Vertex[after].VertexArray[p].Normal.z - g_Wand_Vertex[brfore].VertexArray[p].Normal.z;

			g_wandVertex[p].Normal.x *= g_Wnadtime;
			g_wandVertex[p].Normal.y *= g_Wnadtime;
			g_wandVertex[p].Normal.z *= g_Wnadtime;

			g_wandVertex[p].Normal.x += g_Wand_Vertex[brfore].VertexArray[p].Normal.x;
			g_wandVertex[p].Normal.y += g_Wand_Vertex[brfore].VertexArray[p].Normal.y;
			g_wandVertex[p].Normal.z += g_Wand_Vertex[brfore].VertexArray[p].Normal.z;
		}

		// 色のモーフィング処理
		for (int p = 0; p < g_wandSubset.SubsetNum; p++)
		{
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.x = g_Wand_Vertex[after].SubsetArray[p].Material.Material.Diffuse.x - g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.x;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.y = g_Wand_Vertex[after].SubsetArray[p].Material.Material.Diffuse.y - g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.y;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.z = g_Wand_Vertex[after].SubsetArray[p].Material.Material.Diffuse.z - g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.z;

			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.x *= g_Wnadtime;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.y *= g_Wnadtime;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.z *= g_Wnadtime;

			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.x += g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.x;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.y += g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.y;
			g_wandSubset.SubsetArray[p].Material.Material.Diffuse.z += g_Wand_Vertex[brfore].SubsetArray[p].Material.Material.Diffuse.z;
		}


		SetModelColor(&g_Parts[7].model, &g_wandSubset);

		// 時間を進める
		if ((g_magicChant == FALSE) && (g_magic == FALSE))
		{	// 1回のモーフィングだけ行う
			if (g_Wnadtime > 0.0f) g_Wnadtime -= 0.01f;
			if (g_Wnadtime <= 0.0f) g_MagicType = null;
		}
		else
		{	// 歩くモーション
			if (g_Wnadtime < 1.0f)
			{
				g_Wnadtime += 0.015f;
			}
		}

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = g_wandVertex;

		// 頂点バッファに値をセットする
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_Parts[7].model.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		VERTEX_3D* pVtx = (VERTEX_3D*)msr.pData;

		// 全頂点情報を毎回上書きしているのはDX11ではこの方が早いからです
		memcpy(pVtx, g_wandVertex, sizeof(VERTEX_3D)*g_Wand_Vertex[0].VertexNum);

		GetDeviceContext()->Unmap(g_Parts[7].model.VertexBuffer, 0);
	}


	//////////////////////////////////////////////////////////////////////
	// 姿勢制御
	//////////////////////////////////////////////////////////////////////

	XMVECTOR vx, nvx, up;
	XMVECTOR quat;
	float len, angle;

	// ２つのベクトルの外積を取って任意の回転軸を求める
	g_Player.upVector = normal;
	up = { 0.0f, 1.0f, 0.0f, 0.0f };
	vx = XMVector3Cross(up, XMLoadFloat3(&g_Player.upVector));

	// 求めた回転軸からクォータニオンを作り出す
	nvx = XMVector3Length(vx);
	XMStoreFloat(&len, nvx);
	nvx = XMVector3Normalize(vx);
	angle = asinf(len);
	quat = XMQuaternionRotationNormal(nvx, angle);

	// 前回のクォータニオンから今回のクォータニオンまでの回転を滑らかにする
	quat = XMQuaternionSlerp(XMLoadFloat4(&g_Player.quaternion), quat, 0.05f);

	// 今回のクォータニオンの結果を保存する
	XMStoreFloat4(&g_Player.quaternion, quat);


	// 杖のディゾルブ処理
	g_Wand += WAND_DISSOLVE;
	if (g_Wand > 1.0f) g_Wand -= 1.0f;



#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Player:↑ → ↓ ←　Space\n");
	PrintDebugProc("Player:X:%f Y:%f Z:%f \n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);

	if ((GetKeyboardPress(DIK_1)))
	{
		cam->pos.y += 2.0f;
	}
	if ((GetKeyboardPress(DIK_2)))
	{
		cam->pos.y -= 2.0f;
	}

#endif
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
{
	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

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
		if ((i == 7) && (GetShaderMode() == SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ))
		{
			SetShaderMode(SHADER_MODE_WAND);
			SetShaderResouces(SHADER_RESOURCE_MODE_NOISE);
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

		// 杖のディゾルブ処理
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
PLAYER *GetPlayer(void)
{
	return &g_Player;
}


//=============================================================================
// 歩いているかを管理する関数
//=============================================================================
void SetGait(BOOL data)
{
	g_gait = data;
}


//=============================================================================
//　階層アニメーション
//=============================================================================
void RunAnimation(int i,BOOL data)
{
	// 使われているなら処理する
	if ((g_Parts[i].use == TRUE) && (g_Parts[i].tbl_adr != NULL) && (data))
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


//=============================================================================
// HPを返す
//=============================================================================
int GetHP(void)
{
	return g_Player.HP;
}