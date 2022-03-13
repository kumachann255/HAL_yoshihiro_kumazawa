//=============================================================================
//
// モデル処理 [player.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_PLAYER		(1)					// プレイヤーの数

#define	PLAYER_SIZE		(10.0f)				// 当たり判定の大きさ

// 魔法の番号
enum {
	null,
	fire,
	lightning,
	water,
	holy,
	summon,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct PLAYER
{
	XMFLOAT3			pos;		// ポリゴンの位置
	XMFLOAT3			rot;		// ポリゴンの向き(回転)
	XMFLOAT3			scl;		// ポリゴンの大きさ(スケール)

	XMFLOAT4X4			mtxWorld;	// ワールドマトリックス

	BOOL				load;
	DX11_MODEL			model;		// モデル情報

	float				spd;		// 移動スピード
	float				dir;		// 向き
	float				size;		// 当たり判定の大きさ
	int					HP;			// ヒットポイント
	BOOL				hitCheck;	// 当たり判定が出来る状態か(無敵時間ではないかどうか)
	int					hitTime;	// 無敵時間の管理
	BOOL				use;


	// 階層アニメーション用のメンバー変数
	int					tbl_size;	// 登録したテーブルのレコード総数
	float				move_time;	// 実行時間
	INTERPOLATION_DATA	*tbl_adr;	// アニメデータのテーブル先頭アドレス

	// 親は、NULL、子供は親のアドレスを入れる
	PLAYER				*parent;	// 自分が親ならNULL、自分が子供なら親のplayerアドレス

	// クォータニオン
	XMFLOAT4			quaternion;	// クォータニオン
	XMFLOAT3			upVector;	// 自分が立っている所


};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER *GetPlayer(void);
void SetGait(BOOL data);
int GetHP(void);

