//=============================================================================
//
// ウォーター処理 [water.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_WATER		(1)					// モーフィングの数
#define MAX_WATER_MOVE	(6)					// モーフィングの最大種類

#define	WATER_SIZE		(50.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct WATER
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号
	int					life;				// 敵に当たってから消えるまでの時間
	float				move_time;			// 実行時間

	float				time;				// ベジェ曲線処理の経過時間
	BOOL				firing;				// 発射

};

struct WATER_MOVE
{
	INTERPOLATION_DATA	*tbl_adr;			// アニメデータのテーブル先頭アドレス
	int					tbl_size;			// 登録したテーブルのレコード総数
	float				move_time;			// 実行時間
};




//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitWater(void);
void UninitWater(void);
void UpdateWater(void);
void DrawWater(void);

WATER *GetWater(void);

void SetWaterVertex(DX11_MODEL *Model);
void SetWaterMove(void);
BOOL GetWaterMove(void);
void SetWater(void);
BOOL GetWaterUse(int i);
void SetFiringWater(void);
void SetPolkaDrop(int i);
void ResetWater(int i);
