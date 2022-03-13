//=============================================================================
//
// サウンド処理 [sound.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "sound.h"
#include "model.h"
#include "bahamut.h"
#include "main.h"
#include <DirectXMath.h>

//*****************************************************************************
// パラメータ構造体定義
//*****************************************************************************
typedef struct
{
	char *pFilename;	// ファイル名
	int nCntLoop;		// ループカウント
} SOUNDPARAM;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
IXAudio2 *g_pXAudio2 = NULL;								// XAudio2オブジェクトへのインターフェイス
IXAudio2MasteringVoice *g_pMasteringVoice = NULL;			// マスターボイス
IXAudio2SubmixVoice *g_apSubmixVoice;						// サブミックスボイス

IXAudio2SourceVoice *g_apSourceVoice[SOUND_LABEL_MAX] = {};	// ソースボイス
BYTE *g_apDataAudio[SOUND_LABEL_MAX] = {};					// オーディオデータ
DWORD g_aSizeAudio[SOUND_LABEL_MAX] = {};					// オーディオデータサイズ


// fadeで使用する関数
static BOOL		g_Use;						// TURE:使用している
SOUND_FADE		g_Fade = SOUNDFADE_NONE;	// フェードの状態
int				g_label;					// 次のモード
float			g_NowVolume;
float			g_NextVolume;
float			g_FadeRate;


// 各音素材のパラメータ
SOUNDPARAM g_aParam[SOUND_LABEL_MAX] =
{
	{ (char*)"data/BGM/title_bgm.wav", -1 },	// BGM0
	{ (char*)"data/BGM/game_bgm.wav", -1 },	// BGM0
	{ (char*)"data/BGM/game_over_bgm.wav", -1 },	// BGM0
	{ (char*)"data/BGM/clear_bgm.wav", -1 },	// BGM0
	{ (char*)"data/BGM/end_bgm.wav", 0 },	// BGM0
	{ (char*)"data/BGM/bahamut_bgm.wav", 0 },	// BGM0

	{ (char*)"data/SE/PLAYER/gait.wav", -1 },			// プレイヤーの足音
	{ (char*)"data/SE/PLAYER/player_damage0.wav", 0 },	// プレイヤーのダメージ
	{ (char*)"data/SE/PLAYER/player_damage1.wav", 0 },	// プレイヤーのダメージ
	{ (char*)"data/SE/PLAYER/player_dead0.wav", 0 },	// プレイヤーのHPが0になった時の声
	{ (char*)"data/SE/PLAYER/player_dead1.wav", 0 },	// プレイヤーのHPが0になった時の声
	{ (char*)"data/SE/PLAYER/player_dead2.wav", 0 },	// プレイヤーのHPが0になった時の声
	{ (char*)"data/SE/PLAYER/player_start0.wav", 0 },	// プレイヤーの始まった時の掛け声
	{ (char*)"data/SE/PLAYER/player_start1.wav", 0 },	// プレイヤーの始まった時の掛け声
	{ (char*)"data/SE/PLAYER/player_start2.wav", 0 },	// プレイヤーの始まった時の掛け声

	{ (char*)"data/SE/ENEMY/fly.wav", 0 },				// エネミーの羽ばたき音
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// エネミーのHPが0になった時の声
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// エネミーのHPが0になった時の声
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// エネミーのHPが0になった時の声
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// エネミーのHPが0になった時の声
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// エネミーのHPが0になった時の声
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// エネミーのHPが0になった時の声
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// エネミーのHPが0になった時の声
	{ (char*)"data/SE/ENEMY/pop0.wav", 0 },				// エネミーポップしたときの声
	{ (char*)"data/SE/ENEMY/pop1.wav", 0 },				// エネミーポップしたときの声
	{ (char*)"data/SE/ENEMY/pop2.wav", 0 },				// エネミーポップしたときの声
	{ (char*)"data/SE/ENEMY/pop3.wav", 0 },				// エネミーポップしたときの声

	{ (char*)"data/SE/MAGIC/fire_01.wav", 0 },			// 炎魔法
	{ (char*)"data/SE/MAGIC/fire_01.wav", 0 },			// 炎魔法
	{ (char*)"data/SE/MAGIC/fire_01.wav", 0 },			// 炎魔法
	{ (char*)"data/SE/MAGIC/lightning0.wav", 0 },		// 雷魔法
	{ (char*)"data/SE/MAGIC/lightning0.wav", 0 },		// 雷魔法
	{ (char*)"data/SE/MAGIC/lightning0.wav", 0 },		// 雷魔法
	{ (char*)"data/SE/MAGIC/water0.wav", 0 },			// 水魔法
	{ (char*)"data/SE/MAGIC/water0.wav", 0 },			// 水魔法
	{ (char*)"data/SE/MAGIC/water1.wav", 0 },			// 水魔法
	{ (char*)"data/SE/MAGIC/holy_firing.wav", 0 },		// 光魔法
	{ (char*)"data/SE/MAGIC/holy_morphing.wav", 0 },	// 光魔法
	{ (char*)"data/SE/MAGIC/holy_set.wav", 0 },			// 光魔法

	{ (char*)"data/SE/BAHAMUT/circle.wav", 0 },			// バハムートの魔法陣
	{ (char*)"data/SE/BAHAMUT/pop.wav", 0 },			// バハムートの登場
	{ (char*)"data/SE/BAHAMUT/open.wav", 0 },			// バハムートの登場時の鳴き声
	{ (char*)"data/SE/BAHAMUT/charge.wav", 0 },			// バハムートのブレスをチャージ
	{ (char*)"data/SE/BAHAMUT/bless.wav", 0 },			// バハムートのブレス
	{ (char*)"data/SE/BAHAMUT/blast.wav", 0 },			// バハムートの爆風

	{ (char*)"data/SE/OPTION/option_set.wav", 0 },		// オプション
	{ (char*)"data/SE/OPTION/cursor_enter.wav", 0 },	// オプション
	{ (char*)"data/SE/OPTION/cursor_updown.wav", 0 },	// オプション
	{ (char*)"data/SE/OPTION/cursor_rightleft.wav", 0 },// オプション

	{ (char*)"data/SE/CIRCLE/magic_circle.wav", 0 },	// 魔法陣


};

//=============================================================================
// 初期化処理
//=============================================================================
BOOL InitSound(HWND hWnd)
{
	HRESULT hr;

	// COMライブラリの初期化
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// XAudio2オブジェクトの作成
	hr = XAudio2Create(&g_pXAudio2, 0);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "XAudio2オブジェクトの作成に失敗！", "警告！", MB_ICONWARNING);

		// COMライブラリの終了処理
		CoUninitialize();

		return FALSE;
	}
	
	// マスターボイスの生成
	hr = g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "マスターボイスの生成に失敗！", "警告！", MB_ICONWARNING);

		if(g_pXAudio2)
		{
			// XAudio2オブジェクトの開放
			g_pXAudio2->Release();
			g_pXAudio2 = NULL;
		}

		// COMライブラリの終了処理
		CoUninitialize();

		return FALSE;
	}

	// サブミックスボイスの生成
	hr = g_pXAudio2->CreateSubmixVoice(&g_apSubmixVoice, 2, 44800);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "サブミックスの生成に失敗！", "警告！", MB_ICONWARNING);
		return FALSE;
	}

	// サウンドデータの初期化
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		HANDLE hFile;
		DWORD dwChunkSize = 0;
		DWORD dwChunkPosition = 0;
		DWORD dwFiletype;
		WAVEFORMATEXTENSIBLE wfx;
		XAUDIO2_BUFFER buffer;

		// バッファのクリア
		memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

		// サウンドデータファイルの生成
		hFile = CreateFile(g_aParam[nCntSound].pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(hWnd, "サウンドデータファイルの生成に失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{// ファイルポインタを先頭に移動
			MessageBox(hWnd, "サウンドデータファイルの生成に失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// WAVEファイルのチェック
		hr = CheckChunk(hFile, 'FFIR', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &dwFiletype, sizeof(DWORD), dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		if(dwFiletype != 'EVAW')
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(3)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// フォーマットチェック
		hr = CheckChunk(hFile, ' tmf', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "フォーマットチェックに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "フォーマットチェックに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}

		// オーディオデータ読み込み
		hr = CheckChunk(hFile, 'atad', &g_aSizeAudio[nCntSound], &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "オーディオデータ読み込みに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		g_apDataAudio[nCntSound] = (BYTE*)malloc(g_aSizeAudio[nCntSound]);
		hr = ReadChunkData(hFile, g_apDataAudio[nCntSound], g_aSizeAudio[nCntSound], dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "オーディオデータ読み込みに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// ソースボイスの生成
		hr = g_pXAudio2->CreateSourceVoice(&g_apSourceVoice[nCntSound], &(wfx.Format), 0, 10.0f);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "ソースボイスの生成に失敗！", "警告！", MB_ICONWARNING);
			return FALSE;
		}

		// バッファの値設定
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
		buffer.AudioBytes = g_aSizeAudio[nCntSound];
		buffer.pAudioData = g_apDataAudio[nCntSound];
		buffer.Flags      = XAUDIO2_END_OF_STREAM;
		buffer.LoopCount  = g_aParam[nCntSound].nCntLoop;

		// オーディオバッファの登録
		g_apSourceVoice[nCntSound]->SubmitSourceBuffer(&buffer);
	}

	// 各ソースボイスの調整
	SetFrequencyRatio(SOUND_LABEL_SE_PLAYER_gait, 0.793f);		// 走るSEの速度調整
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_gait, 0.0f);			// 走るSEを一旦聞こえないように調整

	// ソースボイスの一括調整
	SetSourceVolume(1.0f, 1.0f, 1.0f);

	return TRUE;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSound(void)
{
	// 一時停止
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// 一時停止
			g_apSourceVoice[nCntSound]->Stop(0);
	
			// ソースボイスの破棄
			g_apSourceVoice[nCntSound]->DestroyVoice();
			g_apSourceVoice[nCntSound] = NULL;
	
			// オーディオデータの開放
			free(g_apDataAudio[nCntSound]);
			g_apDataAudio[nCntSound] = NULL;
		}
	}
	
	// マスターボイスの破棄
	g_pMasteringVoice->DestroyVoice();
	g_pMasteringVoice = NULL;
	
	if(g_pXAudio2)
	{
		// XAudio2オブジェクトの開放
		g_pXAudio2->Release();
		g_pXAudio2 = NULL;
	}
	
	// COMライブラリの終了処理
	CoUninitialize();
}


//=============================================================================
// セグメント再生(再生中なら停止)
//=============================================================================
void PlaySound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;
	XAUDIO2_BUFFER buffer;

	// バッファの値設定
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
	buffer.AudioBytes = g_aSizeAudio[label];
	buffer.pAudioData = g_apDataAudio[label];
	buffer.Flags      = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount  = g_aParam[label].nCntLoop;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();
	}

	// ソースボイスの出力をサブミックスボイスに切り替え
	XAUDIO2_SEND_DESCRIPTOR send = { 0, g_apSubmixVoice };
	XAUDIO2_VOICE_SENDS sendlist = { 1, &send };
	g_apSourceVoice[label]->SetOutputVoices(&sendlist);

	// オーディオバッファの登録
	g_apSourceVoice[label]->SubmitSourceBuffer(&buffer);

	// 再生
	g_apSourceVoice[label]->Start(0);

}

//=============================================================================
// セグメント停止(ラベル指定)
//=============================================================================
void StopSound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();
	}
}

//=============================================================================
// セグメント停止(全て)
//=============================================================================
void StopSound(void)
{
	// 一時停止
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// 一時停止
			g_apSourceVoice[nCntSound]->Stop(0);
		}
	}
}

//=============================================================================
// チャンクのチェック
//=============================================================================
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition)
{
	HRESULT hr = S_OK;
	DWORD dwRead;
	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD dwBytesRead = 0;
	DWORD dwOffset = 0;
	
	if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを先頭に移動
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	while(hr == S_OK)
	{
		if(ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if(ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクデータの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch(dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize  = dwChunkDataSize;
			dwChunkDataSize = 4;
			if(ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) == 0)
			{// ファイルタイプの読み込み
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if(SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{// ファイルポインタをチャンクデータ分移動
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		dwOffset += sizeof(DWORD) * 2;
		if(dwChunkType == format)
		{
			*pChunkSize         = dwChunkDataSize;
			*pChunkDataPosition = dwOffset;

			return S_OK;
		}

		dwOffset += dwChunkDataSize;
		if(dwBytesRead >= dwRIFFDataSize)
		{
			return S_FALSE;
		}
	}
	
	return S_OK;
}

//=============================================================================
// チャンクデータの読み込み
//=============================================================================
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset)
{
	DWORD dwRead;
	
	if(SetFilePointer(hFile, dwBufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを指定位置まで移動
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if(ReadFile(hFile, pBuffer, dwBuffersize, &dwRead, NULL) == 0)
	{// データの読み込み
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	return S_OK;
}



// マスターボイスの音量調節(0.0f ~ 1.0fで調整)
void SetMasterVolume(float volume)
{
	volume *= volume;
	g_pMasteringVoice->SetVolume(volume);
	return;
}


// ソースボイスの音量調整(0.0f ~ 1.0fで調整)
void SetSourceVolume(int label, float volume)
{
	g_apSourceVoice[label]->SetVolume(volume);
	return;
}


// サウンドのフェード処理(TargetVolume:最終的な音量、TargetTime:何秒でフェード処理したいか)
void SetSoundFade(int label, SOUND_FADE fadetype, float TargetVolume, float TargetTime)
{
	g_Fade = fadetype;
	g_label = label;
	g_NextVolume = TargetVolume;
	g_NowVolume = 1.0f;
	g_FadeRate = fabsf(g_NowVolume - g_NextVolume) / (TargetTime * 60.0f);
	return;
}


// タイトル専用
// サウンドのフェード処理(TargetVolume:最終的な音量、TargetTime:何秒でフェード処理したいか)
void SetTitleSoundFade(int label, SOUND_FADE fadetype, float TargetVolume, float TargetTime)
{
	g_Fade = fadetype;
	g_label = label;
	g_NextVolume = TargetVolume;
	g_NowVolume = 0.2f;
	g_FadeRate = fabsf(g_NowVolume - g_NextVolume) / (TargetTime * 60.0f);
	return;
}


//サウンドのフェード更新処理
void UpdateSoundFade(void)
{
	if (g_Fade != SOUNDFADE_NONE)
	{// フェード処理中
		if (g_Fade == SOUNDFADE_OUT)
		{// フェードアウト処理
			g_NowVolume -= g_FadeRate;		// 減算
			if (g_NowVolume > g_NextVolume)
			{
				g_apSourceVoice[g_label]->SetVolume(g_NowVolume);
			}
			else
			{
				// フェードアウト処理終了
				g_apSourceVoice[g_label]->SetVolume(0.0f);
				g_apSourceVoice[g_label]->Stop();
				g_Fade = SOUNDFADE_NONE;

				// 召喚中だったら曲を再生
				if (GetSummon())
				{
					// BGMを再生
					PlaySound(SOUND_LABEL_BGM_bahamut);
					SetSoundFade(SOUND_LABEL_BGM_bahamut, SOUNDFADE_IN, 0.1f, 1.0f);
				}
				else if (GetMode() == MODE_GAME)
				{
					// BGMを再生
					PlaySound(SOUND_LABEL_BGM_game);
					SetTitleSoundFade(SOUND_LABEL_BGM_game, SOUNDFADE_IN, 0.08f, 1.0f);
				}
			}
		}

		else if (g_Fade == SOUNDFADE_IN)
		{// フェードイン処理
			g_NowVolume += g_FadeRate;		// 加算
			g_apSourceVoice[g_label]->SetVolume(g_NowVolume);

			if (g_NowVolume >= g_NextVolume)
			{
				// フェードイン処理終了
				g_Fade = SOUNDFADE_NONE;
			}
		}
	}
}


//=============================================================================
// ソースボイスのボイスの再生ピッチ調整
//=============================================================================
void SetFrequencyRatio(int label, float Pitch)
{
	g_apSourceVoice[label]->SetFrequencyRatio(Pitch);
	return;
}


//=============================================================================
// ソースボイスの一時停止
//=============================================================================
void PauseSound(int label)
{
	g_apSourceVoice[label]->Stop(XAUDIO2_PLAY_TAILS);
	return;
}

//=============================================================================
// ソースボイスの再開
//=============================================================================
void ReStartSound(int label)
{
	g_apSourceVoice[label]->Start();
	return;
}

//=============================================================================
// ソースボイスの音量調整
//=============================================================================
void SetSourceVolume(float bgm, float se, float voice)
{
	// BGM
	SetSourceVolume(SOUND_LABEL_BGM_title,		0.2f * bgm);
	SetSourceVolume(SOUND_LABEL_BGM_game,		0.05f * bgm);
	SetSourceVolume(SOUND_LABEL_BGM_game_over,	0.1f * bgm);
	SetSourceVolume(SOUND_LABEL_BGM_clear,		0.1f * bgm);
	SetSourceVolume(SOUND_LABEL_BGM_end,		0.1f * bgm);
	SetSourceVolume(SOUND_LABEL_BGM_bahamut,	0.3f * bgm);


	// プレイヤー
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_damage0, 1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_damage1, 1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_dead0,   1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_dead1,   1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_start0,  1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_start1,  1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_start2,  1.0f * voice);


	// エネミー
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_dead0, 0.1f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_dead1, 0.1f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_dead2, 0.1f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_dead3, 0.1f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_dead4, 0.1f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_dead5, 0.1f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_dead6, 0.1f * voice);

	SetSourceVolume(SOUND_LABEL_SE_ENEMY_pop0, 1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_pop1, 1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_pop2, 1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_ENEMY_pop3, 1.0f * voice);


	// 炎魔法
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_fire0, 0.2f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_fire1, 0.2f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_fire2, 0.2f * se);


	// 雷魔法
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_lightning01, 0.4f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_lightning02, 0.4f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_lightning03, 0.4f * se);


	// 水魔法
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_water00, 0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_water01, 0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_water1,  1.2f * se);


	// 光魔法
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_holy_firing,	0.2f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_holy_morphing, 0.4f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_holy_set,		0.4f * se);


	// 魔法陣
	SetSourceVolume(SOUND_LABEL_SE_CIRCLE_circle, 1.0f * se);


	// バハムートのSE
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_circle,	1.0f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_pop,		1.0f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_open,	4.0f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_charge,	0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_bless,	1.0f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_blast,	2.0f * se);



	// オプションの効果音
	SetSourceVolume(SOUND_LABEL_SE_OPTION_option_set,		0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_OPTION_cursor_enter,		0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_OPTION_cursol_updown,	0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_OPTION_cursor_rightleft, 0.7f * se);



}