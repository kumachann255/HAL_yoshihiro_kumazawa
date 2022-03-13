//=============================================================================
//
// ゲーム画面処理 [game.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "camera.h"
#include "input.h"
#include "sound.h"
#include "fade.h"
#include "game.h"

#include "player.h"
#include "enemy.h"
#include "bahamut.h"
#include "bahamut_particle.h"
#include "bahamut_bless.h"
#include "bless_particle.h"
#include "beam_particle.h"
#include "beam_particle_2.h"
#include "beam_orbit.h"
#include "chant_particle.h"
#include "holy.h"
#include "water.h"
#include "meshfield.h"
#include "tree.h"
#include "skydome.h"
#include "magic_circle.h"
#include "score.h"
#include "particle.h"
#include "holy_particle.h"
#include "holy_orbit.h"
#include "lightning.h"
#include "fire.h"
#include "collision.h"
#include "debugproc.h"
#include "screen.h"
#include "option.h"
#include "arrow.h"
#include "UI.h"
#include "light.h"
#include "barrier.h"
#include "tutorial.h"
#include "fade_white.h"
#include "wand_lightning.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define DAMAGE		(1)		// ダメージ量


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void CheckHit(void);
void SetEnemyDeadSound(void);



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// ポーズON/OFF

static int g_SE = 0;			// SEの管理
static BOOL g_EnemySE = FALSE;	// 現在のフレームでSEを再生したか

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGame(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// ライトの初期化
	InitLight();

	// オプションの初期化
	InitOption();

	// チュートリアルの初期化
	InitTutorial();

	// フィールドの初期化
	InitMeshField(XMFLOAT3(MAP_OFFSET_X, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 150, 65, 30.0f, 20.0f);

	// プレイヤーの初期化
	InitPlayer();

	// エネミーの初期化
	InitEnemy();

	// バハムートの初期化
	InitBahamut();

	// バハムートのパーティクルの初期化
	InitBahamutParticle();

	// バハムートのブレスの初期化
	InitBless();

	// ブレスのパーティクルの初期化
	InitBlessParticle();

	// ビームのパーティクルの初期化
	InitBeamParticle();
	InitBeamParticle2();

	// 光魔法の初期化
	InitHoly();

	// 水魔法の初期化
	InitWater();

	// 木を生やす
	InitTree();

	// スカイドームの初期化
	InitSky();

	// 魔法陣の初期化
	InitCircle();

	// スコアの初期化
	InitScore();

	// UIの初期化
	InitUI();

	// パーティクルの初期化
	InitHolyParticle();
	InitHolyOrbit();
	InitBeamOrbit();
	InitChantParticle();

	// ライトニングの初期化
	InitLightning();

	// 杖の雷の初期化
	InitWandLightning();

	// ファイアの初期化
	InitFire();

	// バリアの初期化
	InitBarrier();

	// 矢印ガイドの初期化
	InitArrow();

	// スクリーンの初期化
	InitScreen();

	// ホワイトフェードの初期化
	InitFadeWhite();

	// BGM再生
	PlaySound(SOUND_LABEL_BGM_game);
	
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGame(void)
{
	// ホワイトフェードの終了処理
	UninitFadeWhite();

	// スクリーンの終了処理
	UninitScreen();

	// 矢印ガイドの終了処理
	UninitArrow();

	// バリアの終了処理
	UninitBarrier();

	// ファイアの終了処理
	UninitFire();

	// 杖の雷の終了処理
	UninitWandLightning();

	// ライトニングの終了処理
	UninitLightning();

	// パーティクルの終了処理
	UninitChantParticle();
	UninitBeamOrbit();
	UninitHolyOrbit();
	UninitHolyParticle();

	// UIの終了処理
	UninitUI();

	// スコアの終了処理
	UninitScore();

	// 魔法陣の終了処理
	UninitCircle();

	// スカイドームの終了処理
	UninitSky();

	// 木の終了処理
	UninitTree();

	// 地面の終了処理
	UninitMeshField();

	// 光魔法の終了処理
	UninitHoly();

	// 水魔法の終了処理
	UninitWater();

	// ビームのパーティクルの終了処理
	UninitBeamParticle2();
	UninitBeamParticle();

	// ブレスのパーティクルの終了処理
	UninitBlessParticle();

	// バハムートのブレスの終了処理
	UninitBless();

	// バハムートのパーティクルの終了処理
	UninitBahamutParticle();

	// バハムートの終了処理
	UninitBahamut();

	// エネミーの終了処理
	UninitEnemy();

	// プレイヤーの終了処理
	UninitPlayer();

	// チュートリアルの終了処理
	UninitTutorial();

	// オプションの終了処理
	UninitOption();

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateGame(void)
{
#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_V))
	{
		g_ViewPortType_Game = (g_ViewPortType_Game + 1) % TYPE_NONE;
		SetViewPort(g_ViewPortType_Game);
	}

	if (GetKeyboardTrigger(DIK_P))
	{
		g_bPause = g_bPause ? FALSE : TRUE;
	}


#endif

	if (g_bPause == FALSE)
		return;

	// チュートリアルの更新処理
	UpdateTutorial();

	// オプションの更新処理
	UpdateOption();

	// チュートリアル画面中は画面を止める
	if (GetTutorialUse()) return;

	// オプションが表示されているときは、画面を止める
	if (GetOptionUse()) return;

	// 地面処理の更新
	UpdateMeshField();

	// プレイヤーの更新処理
	UpdatePlayer();

	// エネミーの更新処理
	UpdateEnemy();

	// バハムートの更新処理
	UpdateBahamut();

	// バハムートのパーティクルの更新処理
	UpdateBahamutParticle();

	// バハムートのブレスの更新処理
	UpdateBless();

	// ブレスのパーティクルの更新処理
	UpdateBlessParticle();

	// ビームのパーティクルの更新処理
	UpdateBeamParticle();
	UpdateBeamParticle2();

	// 光魔法の更新処理
	UpdateHoly();

	// 水魔法の更新処理
	UpdateWater();

	// 木の更新処理
	UpdateTree();

	// スカイドームの更新処理
	UpdateSky();

	// 魔法陣の更新処理
	UpdateCircle();

	// パーティクルの更新処理
	UpdateHolyParticle();
	UpdateHolyOrbit();
	UpdateBeamOrbit();
	UpdateChantParticle();

	// ライトニングの更新処理
	UpdateLightning();

	// 杖の雷の更新処理
	UpdateWandLightning();

	// ファイアの更新処理
	UpdateFire();

	// 矢印ガイドの更新処理
	UpdateArrow();

	// バリアの更新処理
	UpdateBarrier();

	// スクリーンの更新処理
	UpdateScreen();

	// ホワイトフェードの更新処理
	UpdateFadeWhite();

	// 当たり判定処理
	CheckHit();

	// スコアの更新処理
	UpdateScore();

	// UIの更新処理
	UpdateUI();

	// サウンドの更新処理
	UpdateSoundFade();
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGame0(void)
{
	// オブジェクトをPREレンダーターゲットへ描画
	SetShaderType(SHADER_SHADOWMAP__VIEW_SHADOWMAP__RESOUCE_OBJ);
	{
		// 3Dの物を描画する処理
		// 地面の描画処理
		DrawMeshField();

		// 木の描画処理
		DrawTree();

		// エネミーの描画処理
		DrawEnemy();

		// プレイヤーの描画処理
		DrawPlayer();

		// バハムートの描画処理
		DrawBahamut();

		// スカイドームの描画処理
		//DrawSky();
	}


	SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	Clear();

	// 影を付けたいオブジェクト
	// オブジェクトをPREレンダーターゲットへ描画
	SetShaderType(SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ);
	{
		// 3Dの物を描画する処理
		// 地面の描画処理
		DrawMeshField();

		// 木の描画処理
		DrawTree();

		// プレイヤーの描画処理
		DrawPlayer();

		// スカイドームの描画処理
		DrawSky();
	}

	// ディゾルブ処理(エネミー)
	SetShaderType(SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE);
	{
		// エネミーの描画処理
		DrawEnemy();

		// ノイズテクスチャを変更
		SetShaderResouces(SHADER_RESOURCE_MODE_NOISE_CENTER);
		//SetShaderType(SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE);

		// バハムートの描画処理
		DrawBahamut();
	}

	// ディゾルブ処理(魔法陣)
	SetShaderType(SHADER_CIRCLR__VIEW_PRE__RESOUCE_OBJ_NOISE);
	{
		// 魔法陣の描画処理
		DrawCircle();

		// バリアの描画処理
		DrawBarrier();
	}

	// 影を付けたくないオブジェクト
	SetShaderType(SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ);
	{
		// 光魔法の描画処理
		DrawHoly();

		// ライトニングの描画処理
		DrawLightning();

		// ファイアの描画処理
		DrawFire();

		// 矢印ガイドの描画処理
		DrawArrow();


		//DrawHolyParticle();
		//DrawHolyOrbit();
		//DrawBahamutParticle();
		//DrawBlessParticle();
		//DrawBeamParticle();
		//DrawBeamParticle2();
		//DrawBeamOrbit();
		//DrawChantParticle();
		//DrawWandLightning();
	}

	// 爆風の描画
	SetShaderType(SHADER_BLAST__VIEW_PRE__RESOUCE_OBJ);
	{
		// バハムートの爆風の描画処理
		DrawBless();
	}


	// 水魔法の描画
	SetShaderType(SHADER_WATER__VIEW_PRE__RESOUCE_OBJ_NOISE);
	{
		// 水魔法の描画処理
		DrawWater();
	}

	// パーティクル描画
	SetShaderMode(SHADER_MODE_PARTICLE);
	{
		DrawHolyParticle();
		DrawHolyOrbit();
		DrawBahamutParticle();
		DrawBlessParticle();
		DrawBeamParticle();
		DrawBeamParticle2();
		DrawBeamOrbit();
		DrawChantParticle();
		DrawWandLightning();
	}


	// 2Dの物を描画する処理
	// Z比較なし
	SetDepthEnable(FALSE);

	// ライティングを無効
	SetLightEnable(FALSE);

	RunBlur();

	// PRErtを輝度抽出rtへLUMINACEシェーダ―で書き込む
	SetShaderType(SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS);
	{
		// スクリーンの描画処理
		DrawScreen();

		// シェーダ―リソース用のレンダーターゲットをクリア
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// x方向へのブラー加工
	// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	SetShaderType(SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE);
	{
		// スクリーンの描画処理
		DrawScreen();

		// シェーダ―リソース用のレンダーターゲットをクリア
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// y方向へのブラー加工
	// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	SetShaderType(SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR);
	{
		// スクリーンの描画処理
		DrawScreen();

		// シェーダ―リソース用のレンダーターゲットをクリア
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// y方向へのブラー加工
	// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	SetShaderType(SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS);
	{
		// スクリーンの描画処理
		DrawScreen();

		// シェーダ―リソース用のレンダーターゲットをクリア
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// シェーダ―リソース用のレンダーターゲットをクリア
	SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);


	SetShaderType(DEFAULT);

	// ホワイトフェードの描画処理
	DrawFadeWhite();

	// スコアの描画処理
	DrawScore();

	// UIの描画
	DrawUI();

	// チュートリアルの描画処理
	DrawTutorial();

	// オプションの描画処理
	DrawOption();


	// ライティングを有効に
	SetLightEnable(TRUE);

	// Z比較あり
	SetDepthEnable(TRUE);
}


void DrawGame(void)
{
	XMFLOAT3 pos;


#ifdef _DEBUG
	// デバッグ表示
	PrintDebugProc("ViewPortType:%d\n", g_ViewPortType_Game);

#endif

	// プレイヤー視点
	pos = GetPlayer()->pos;
	pos.y = 0.0f;			// カメラ酔いを防ぐためにクリアしている

	// 召喚魔法が発動していない場合は注視点はプレイヤー付近
	if(!GetSummon()) SetCameraAT(pos);
	SetCamera();

	switch(g_ViewPortType_Game)
	{
	case TYPE_FULL_SCREEN:
		SetViewPort(TYPE_FULL_SCREEN);
		DrawGame0();
		break;

	case TYPE_LEFT_HALF_SCREEN:
	case TYPE_RIGHT_HALF_SCREEN:
		SetViewPort(TYPE_LEFT_HALF_SCREEN);
		DrawGame0();

		// エネミー視点
		pos = GetEnemy()->pos;
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_RIGHT_HALF_SCREEN);
		DrawGame0();
		break;

	case TYPE_UP_HALF_SCREEN:
	case TYPE_DOWN_HALF_SCREEN:
		SetViewPort(TYPE_UP_HALF_SCREEN);
		DrawGame0();

		// エネミー視点
		pos = GetEnemy()->pos;
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_DOWN_HALF_SCREEN);
		DrawGame0();
		break;

	}
}


//=============================================================================
// 当たり判定処理
//=============================================================================
void CheckHit(void)
{
	ENEMY *enemy = GetEnemy();		// エネミーのポインターを初期化
	PLAYER *player = GetPlayer();	// プレイヤーのポインターを初期化
	FIRE *fire = GetFire();
	LIGHTNING_COLLI *lightning = GetLightningCollition();
	HOLY *holy = GetHoly();
	WATER *water = GetWater();


	// 初期化
	g_EnemySE = FALSE;

	// 敵とプレイヤーキャラ
	if (player->hitCheck)
	{
		for (int i = 0; i < MAX_ENEMY; i++)
		{
			//敵の有効フラグをチェックする
			if ((enemy[i].use == FALSE) || (enemy[i].end))
				continue;

			//BCの当たり判定
			if (CollisionBC(player->pos, enemy[i].pos, player->size, enemy[i].size))
			{
				// プレイヤーのHPを減らす
				player->HP -= DAMAGE;

				// プレイヤーを無敵時間へ
				player->hitCheck = FALSE;

				// 画面を少し揺らす
				SetCameraShake(20);

				// HPが0の時は別ボイスを再生
				if (player->HP > 0)
				{
					// ダメージを受けた時のSEを再生
					// 2種類からランダム
					char se = rand() % 2;

					switch (se)
					{
					case 0:
						PlaySound(SOUND_LABEL_SE_PLAYER_damage0);
						break;

					case 1:
						PlaySound(SOUND_LABEL_SE_PLAYER_damage1);
						break;
					}
				}
			}
		}
	}


	// プレイヤーの炎魔法と敵
	for (int i = 0; i < MAX_FIRE; i++)
	{
		//弾の有効フラグをチェックする
		if (fire[i].bUse == FALSE)
			continue;

		// 当たった判定を行ったものは再度判定を行わない
		if (fire[i].atk == TRUE)
			continue;

		// 敵と当たってるか調べる
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//敵の有効フラグをチェックする
			if (enemy[j].use == FALSE)
				continue;

			//BCの当たり判定
			if (CollisionBC(fire[i].pos, enemy[j].pos, fire[i].fSizeY, enemy[j].size))
			{
				// 当たったから当たった判定にする
				fire[i].atk = TRUE;

				// 敵の死ぬときのSEを再生
				if (enemy[j].end == FALSE)
				{
					SetEnemyDeadSound();
				}

				// 敵キャラクターは倒される
				enemy[j].end = TRUE;

				// スコアを足す
				AddScore(10);
			}
		}
	}


	// プレイヤーの雷魔法と敵
	for (int i = 0; i < MAX_LIGHTNING; i++)
	{
		//弾の有効フラグをチェックする
		if (lightning[i].bUse == FALSE)
			continue;

		// 敵と当たってるか調べる
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//敵の有効フラグをチェックする
			if (enemy[j].use == FALSE)
				continue;

			//BCの当たり判定
			if (CollisionBB(lightning[i].pos, lightning[i].fSizeX, lightning[i].fSizeY, lightning[i].fSizeZ, enemy[j].pos, enemy[j].size, enemy[j].size, enemy[j].size))
			{
				// 敵の死ぬときのSEを再生
				if (enemy[j].end == FALSE)
				{
					SetEnemyDeadSound();
				}

				// 敵キャラクターは倒される
				enemy[j].end = TRUE;

				// スコアを足す
				AddScore(10);
			}
		}
	}


	// プレイヤーの光魔法と敵
	for (int i = 0; i < MAX_HOLY; i++)
	{
		//弾の有効フラグをチェックする
		if (holy[i].use == FALSE)
			continue;

		// 敵と当たってるか調べる
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//敵の有効フラグをチェックする
			if (enemy[j].use == FALSE)
				continue;

			//BCの当たり判定
			if (CollisionBC(holy[i].pos, enemy[j].pos, holy[i].size, enemy[j].size))
			{
				// 敵の死ぬときのSEを再生
				if (enemy[j].end == FALSE)
				{
					SetEnemyDeadSound();
				}

				// 敵キャラクターは倒される
				enemy[j].end = TRUE;

				// スコアを足す
				AddScore(10);
			}
		}
	}


	// プレイヤーの水魔法と敵
	for (int i = 0; i < MAX_WATER; i++)
	{
		//弾の有効フラグをチェックする
		if (water[i].use == FALSE)
			continue;

		// 敵と当たってるか調べる
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//敵の有効フラグをチェックする
			if (enemy[j].use == FALSE)
				continue;

			//BCの当たり判定
			if (CollisionBC(water[i].pos, enemy[j].pos, water[i].size, enemy[j].size))
			{
				// エネミーを動けなくする
				enemy[j].moveCan = FALSE;

				// エネミーを回転させる
				enemy[j].rot.x += 0.01f;
				enemy[j].rot.y += 0.01f;
				enemy[j].rot.z += 0.01f;

				// エネミーを水玉の中心に近づける
				float dis;
				// x
				dis = water[i].pos.x - enemy[j].pos.x;
				enemy[j].pos.x += dis / 10.0f;
				// y
				dis = water[i].pos.y - enemy[j].pos.y;
				enemy[j].pos.y += dis / 10.0f;
				// z
				dis = water[i].pos.z - enemy[j].pos.z;
				enemy[j].pos.z += dis / 10.0f;


				// 酸素値を減らす
				enemy[j].waterCount--;
				if (enemy[j].waterCount < 0)
				{
					// 敵の死ぬときのSEを再生
					if (enemy[j].end == FALSE)
					{
						SetEnemyDeadSound();
					}

					// 敵キャラクターは倒される
					enemy[j].end = TRUE;
				}

				// スコアを足す
				AddScore(10);
			}
		}
	}


	// 召喚魔法が発動したら登場しているエネミーはすべて倒される
	if (GetSummon())
	{
		for (int j = 0; j < MAX_ENEMY; j++)
		{
			//敵の有効フラグをチェックする
			if (enemy[j].use == FALSE) continue;

			enemy[j].end = TRUE;

			// スコアを足す
			AddScore(10);
		}
	}


	// エネミーが全部死亡したら状態遷移
	int enemy_count = 0;
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if ((enemy[i].use == FALSE) && (enemy[i].popCount == -1)) continue;
		enemy_count++;
	}

	// エネミーが０匹？
	if (enemy_count == 0)
	{
		SetFade(FADE_OUT, MODE_CLEAR);
	}

	// プレイヤーのHPが0になったらシーン遷移
	if ((player->HP <= 0) && (player->HP != -100))
	{
		SetFade(FADE_OUT, MODE_OVER);
		
		// ボイスが重ならないように調整
		player->HP = -100;

		// ランダムで死亡時のボイスを再生
		int voice = rand() % 3;
		switch (voice)
		{
		case 0:
			PlaySound(SOUND_LABEL_SE_PLAYER_dead0);
			SetSoundFade(SOUND_LABEL_SE_PLAYER_dead0, SOUNDFADE_OUT, 0.0f, 1.0f);
			break;

		case 1:
			PlaySound(SOUND_LABEL_SE_PLAYER_dead1);
			SetSoundFade(SOUND_LABEL_SE_PLAYER_dead1, SOUNDFADE_OUT, 0.0f, 1.0f);
			break;

		case 2:
			PlaySound(SOUND_LABEL_SE_PLAYER_dead2);
			SetSoundFade(SOUND_LABEL_SE_PLAYER_dead2, SOUNDFADE_OUT, 0.0f, 1.0f);
			break;
		}

		// BGMをフェードアウト
		SetTitleSoundFade(SOUND_LABEL_BGM_game, SOUNDFADE_OUT, 0.0f, 1.4f);
	}
}


// SEが重なるように別音源で再生する処理
void SetEnemyDeadSound(void)
{
	if(g_EnemySE) return;

	g_SE++;
	g_EnemySE = TRUE;

	switch (g_SE % 7)
	{
	case 0:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead0);
		break;

	case 1:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead1);
		break;

	case 2:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead2);
		break;

	case 3:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead3);
		break;

	case 4:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead4);
		break;

	case 5:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead5);
		break;

	case 6:
		PlaySound(SOUND_LABEL_SE_ENEMY_dead6);
		break;
	}
}