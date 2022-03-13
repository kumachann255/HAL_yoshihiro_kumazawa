//=============================================================================
//
// バハムート処理 [bahamut.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************

// 召喚魔法のシーン(カメラワーク用)
enum {
	circle,
	pop,
	fold,
	open,
	charge,
	bless,
	bless_2camera,
	blast,
	end,
	sceneMAX,
};


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct BAHAMUT
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
	BAHAMUT				*parent;	// 自分が親ならNULL、自分が子供なら親のplayerアドレス

	BOOL				moveCan;			// 移動出来るかどうか
	BOOL				turn;				// 目標に到達して折り返したか
	float				radian;				// サインカーブ用のラジアン
	float				hight;				// エネミーのベースの高さ(ランダム)
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBahamut(void);
void UninitBahamut(void);
void UpdateBahamut(void);
void DrawBahamut(void);

BAHAMUT *GetBahamut(void);
BOOL GetSummon(void);
bool GetWingOpen(void);

void SetSummon(void);
void SetChargeMorphing(void);
void SetBlessMorphing(void);
void SetBlessEndMorphing(void);

void ResetBahamut(void);
