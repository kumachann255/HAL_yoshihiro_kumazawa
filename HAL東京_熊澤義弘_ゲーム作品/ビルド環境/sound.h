//=============================================================================
//
// サウンド処理 [sound.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// サウンド処理で必要

//*****************************************************************************
// サウンドファイル
//*****************************************************************************
enum 
{
	// BGM
	SOUND_LABEL_BGM_title,
	SOUND_LABEL_BGM_game,
	SOUND_LABEL_BGM_game_over,
	SOUND_LABEL_BGM_clear,
	SOUND_LABEL_BGM_end,
	SOUND_LABEL_BGM_bahamut,

	// プレイヤーのSE
	SOUND_LABEL_SE_PLAYER_gait,
	SOUND_LABEL_SE_PLAYER_damage0,
	SOUND_LABEL_SE_PLAYER_damage1,
	SOUND_LABEL_SE_PLAYER_dead0,
	SOUND_LABEL_SE_PLAYER_dead1,
	SOUND_LABEL_SE_PLAYER_dead2,
	SOUND_LABEL_SE_PLAYER_start0,
	SOUND_LABEL_SE_PLAYER_start1,
	SOUND_LABEL_SE_PLAYER_start2,

	// エネミーのSE
	SOUND_LABEL_SE_ENEMY_fly,
	SOUND_LABEL_SE_ENEMY_dead0,
	SOUND_LABEL_SE_ENEMY_dead1,
	SOUND_LABEL_SE_ENEMY_dead2,
	SOUND_LABEL_SE_ENEMY_dead3,
	SOUND_LABEL_SE_ENEMY_dead4,
	SOUND_LABEL_SE_ENEMY_dead5,
	SOUND_LABEL_SE_ENEMY_dead6,
	SOUND_LABEL_SE_ENEMY_pop0,
	SOUND_LABEL_SE_ENEMY_pop1,
	SOUND_LABEL_SE_ENEMY_pop2,
	SOUND_LABEL_SE_ENEMY_pop3,

	// 魔法のSE
	SOUND_LABEL_SE_MAGIC_fire0,
	SOUND_LABEL_SE_MAGIC_fire1,
	SOUND_LABEL_SE_MAGIC_fire2,
	SOUND_LABEL_SE_MAGIC_lightning01,
	SOUND_LABEL_SE_MAGIC_lightning02,
	SOUND_LABEL_SE_MAGIC_lightning03,
	SOUND_LABEL_SE_MAGIC_water00,
	SOUND_LABEL_SE_MAGIC_water01,
	SOUND_LABEL_SE_MAGIC_water1,
	SOUND_LABEL_SE_MAGIC_holy_firing,
	SOUND_LABEL_SE_MAGIC_holy_morphing,
	SOUND_LABEL_SE_MAGIC_holy_set,

	// バハムートのSE
	SOUND_LABEL_SE_BAHAMUT_circle,
	SOUND_LABEL_SE_BAHAMUT_pop,
	SOUND_LABEL_SE_BAHAMUT_open,
	SOUND_LABEL_SE_BAHAMUT_charge,
	SOUND_LABEL_SE_BAHAMUT_bless,
	SOUND_LABEL_SE_BAHAMUT_blast,


	// オプション
	SOUND_LABEL_SE_OPTION_option_set,
	SOUND_LABEL_SE_OPTION_cursor_enter,
	SOUND_LABEL_SE_OPTION_cursol_updown,
	SOUND_LABEL_SE_OPTION_cursor_rightleft,

	// 魔法陣のSE
	SOUND_LABEL_SE_CIRCLE_circle,

	SOUND_LABEL_MAX,
};

typedef enum
{
	SOUNDFADE_NONE = 0,		// 何もしない
	SOUNDFADE_IN,			// フェードイン処理
	SOUNDFADE_OUT,			// フェードアウト
}SOUND_FADE;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
BOOL InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);
void SetMasterVolume(float volume);
void SetSourceVolume(int label, float volume);
void SetSoundFade(int label, SOUND_FADE fadetype, float TargetVolume, float TargetTime);
void SetTitleSoundFade(int label, SOUND_FADE fadetype, float TargetVolume, float TargetTime);
void UpdateSoundFade(void);
void SetFrequencyRatio(int label, float Pitch);
void PauseSound(int label);
void ReStartSound(int label);
void SetSourceVolume(float bgm, float se, float voice);

