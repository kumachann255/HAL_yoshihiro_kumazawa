//=============================================================================
//
// ホーリー処理 [holy.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_HOLY		(5)					// モーフィングの数
#define MAX_HOLY_MOVE	(2)					// モーフィングの最大種類

#define	HOLY_SIZE		(7.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct HOLY
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)
	XMFLOAT3			direct;				// 剣から敵までのベクトル

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号
	
	INTERPOLATION_DATA	*tbl_adr;			// アニメデータのテーブル先頭アドレス
	int					tbl_size;			// 登録したテーブルのレコード総数
	float				move_time;			// 実行時間

	float				time;				// ベジェ曲線処理の経過時間
	BOOL				firing;				// 発射

};

struct HOLY_MOVE
{
	INTERPOLATION_DATA	*tbl_adr;			// アニメデータのテーブル先頭アドレス
	int					tbl_size;			// 登録したテーブルのレコード総数
	float				move_time;			// 実行時間
};




//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitHoly(void);
void UninitHoly(void);
void UpdateHoly(void);
void DrawHoly(void);

HOLY *GetHoly(void);
void SetHoly(void);

void SetHolyVertex(DX11_MODEL *Model);
void SetMove(void);
BOOL GetMove(void);
void SetFiringSword(void);
XMFLOAT3 GetHolyMoveValue(int i);
