//=============================================================================
//
// エネミーモデル処理 [enemy.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_WAVE		(4)					// ウェイブ数
#define MAX_POP_WAVE1	(3)					// 1ウェイブで何体POPさせるか
#define MAX_POP_WAVE2	(8)					// 2ウェイブで何体POPさせるか
#define MAX_POP_WAVE3	(10)				// 3ウェイブで何体POPさせるか
#define MAX_POP_WAVE4	(15)				// 4ウェイブで何体POPさせるか

// デバック用
//#define MAX_POP_WAVE1	(1)					// 1ウェイブで何体POPさせるか
//#define MAX_POP_WAVE2	(1)					// 2ウェイブで何体POPさせるか
//#define MAX_POP_WAVE3	(1)				// 3ウェイブで何体POPさせるか
//#define MAX_POP_WAVE4	(1)				// 4ウェイブで何体POPさせるか

#define MAX_ENEMY		(MAX_POP_WAVE1 + MAX_POP_WAVE2 + MAX_POP_WAVE3 + MAX_POP_WAVE4)		// エネミーの数

#define	ENEMY_SIZE		(30.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct ENEMY
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)
	XMFLOAT3			move;				// 移動方向

	BOOL				use;
	BOOL				end;				// ディゾルブ開始フラグ
	BOOL				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ
	float				dissolve;			// ディゾルブの閾値

	int					waterCount;			// 酸素値(waterで使用)

	INTERPOLATION_DATA	*tbl_adr;			// アニメデータのテーブル先頭アドレス
	int					tbl_size;			// 登録したテーブルのレコード総数
	float				move_time;			// 実行時間

	// 親は、NULL、子供は親のアドレスを入れる
	ENEMY				*parent;	// 自分が親ならNULL、自分が子供なら親のplayerアドレス

	BOOL				moveCan;			// 移動出来るかどうか
	BOOL				turn;				// 目標に到達して折り返したか
	float				radian;				// サインカーブ用のラジアン
	float				hight;				// エネミーのベースの高さ(ランダム)

	int					wave;				// どのウェーブで出現させるか
	int					popCount;			// ウェーブが開始されてからどのタイミングで発生させるか
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY *GetEnemy(void);

