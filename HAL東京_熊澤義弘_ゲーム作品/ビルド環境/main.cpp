//=============================================================================
//
// メイン処理 [main.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "model.h"
#include "player.h"
#include "title_player.h"
#include "enemy.h"
#include "light.h"
#include "meshfield.h"
#include "tree.h"
#include "collision.h"
#include "score.h"
#include "sound.h"
#include "particle.h"
#include "Holy_particle.h"
#include "light.h"
#include "option.h"
#include "screen.h"
#include "endroll.h"

#include "title.h"
#include "game.h"
#include "result.h"
#include "fade.h"
#include "time.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define CLASS_NAME		"AppClass"			// ウインドウのクラス名
#define WINDOW_NAME		"MAGIC TIME"		// ウインドウのキャプション名

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);
void DrawGameTitle(void);


//*****************************************************************************
// グローバル変数:
//*****************************************************************************
long g_MouseX = 0;
long g_MouseY = 0;


#ifdef _DEBUG
int		g_CountFPS;							// FPSカウンタ
char	g_DebugStr[2048] = WINDOW_NAME;		// デバッグ文字表示用

#endif

int	g_Mode = MODE_TITLE;					// 起動時の画面を設定


//=============================================================================
// メイン関数
//=============================================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);	// 無くても良いけど、警告が出る（未使用宣言）
	UNREFERENCED_PARAMETER(lpCmdLine);		// 無くても良いけど、警告が出る（未使用宣言）

	// 時間計測用
	DWORD dwExecLastTime;
	DWORD dwFPSLastTime;
	DWORD dwCurrentTime;
	DWORD dwFrameCount;

	WNDCLASSEX	wcex = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		WndProc,
		0,
		0,
		hInstance,
		NULL,
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		CLASS_NAME,
		NULL
	};
	HWND		hWnd;
	MSG			msg;
	
	// ウィンドウクラスの登録
	RegisterClassEx(&wcex);

	// ウィンドウの作成
	hWnd = CreateWindow(CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,																		// ウィンドウの左座標
		CW_USEDEFAULT,																		// ウィンドウの上座標
		SCREEN_WIDTH + GetSystemMetrics(SM_CXDLGFRAME) * 2,									// ウィンドウ横幅
		SCREEN_HEIGHT + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),	// ウィンドウ縦幅
		NULL,
		NULL,
		hInstance,
		NULL);

	// ウィンドウモードかフルスクリーンモードかの処理
	bool mode = true;

	int id = MessageBox(NULL, "Windowモードでプレイしますか？", "起動モード", MB_YESNOCANCEL | MB_ICONQUESTION);
	switch (id)
	{
	case IDYES:		// YesならWindowモードで起動
		mode = true;
		break;
	case IDNO:		// Noならフルスクリーンモードで起動
		mode = false;
		break;
	case IDCANCEL:	// CANCELなら終了
	default:
		return -1;
		break;
	}

	// 初期化処理(ウィンドウを作成してから行う)
	if(FAILED(Init(hInstance, hWnd, mode)))
	{
		return -1;
	}

	// フレームカウント初期化
	timeBeginPeriod(1);	// 分解能を設定
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// システム時刻をミリ秒単位で取得
	dwCurrentTime = dwFrameCount = 0;

	// ウインドウの表示(初期化処理の後に呼ばないと駄目)
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	// メッセージループ
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{// PostQuitMessage()が呼ばれたらループ終了
				break;
			}
			else
			{
				// メッセージの翻訳と送出
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)	// 1秒ごとに実行
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;				// FPSを測定した時刻を保存
				dwFrameCount = 0;							// カウントをクリア
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))	// 1/60秒ごとに実行
			{
				dwExecLastTime = dwCurrentTime;	// 処理した時刻を保存

#ifdef _DEBUG	// デバッグ版の時だけFPSを表示する
				wsprintf(g_DebugStr, WINDOW_NAME);
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " FPS:%d", g_CountFPS);
#endif

				Update();			// 更新処理
				Draw();				// 描画処理

#ifdef _DEBUG	// デバッグ版の時だけ表示する
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " MX:%d MY:%d", GetMousePosX(), GetMousePosY());
				SetWindowText(hWnd, g_DebugStr);
#endif

				dwFrameCount++;
			}
		}
	}

	timeEndPeriod(1);				// 分解能を戻す

	// ウィンドウクラスの登録を解除
	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// 終了処理
	Uninit();

	return (int)msg.wParam;
}

//=============================================================================
// プロシージャ
//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		break;

	case WM_MOUSEMOVE:
		g_MouseX = LOWORD(lParam);
		g_MouseY = HIWORD(lParam);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	// ランダムの初期化処理
	srand((unsigned)time(NULL));

	InitRenderer(hInstance, hWnd, bWindow);

	InitLight();

	InitCamera();

	// 入力処理の初期化
	InitInput(hInstance, hWnd);

	// サウンドの初期化
	InitSound(hWnd);


	// ライトを有効化
	SetLightEnable(TRUE);

	// 背面ポリゴンをカリング
	SetCullingMode(CULL_MODE_BACK);


	// フェードの初期化
	InitFade();

	// 最初のモードをセット
	SetMode(g_Mode);	// ここはSetModeのままで！

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void Uninit(void)
{
	// 終了のモードをセット
	SetMode(MODE_MAX);

	// サウンド終了処理
	UninitSound();

	// カメラの終了処理
	UninitCamera();

	//入力の終了処理
	UninitInput();

	// レンダラーの終了処理
	UninitRenderer();
}

//=============================================================================
// 更新処理
//=============================================================================
void Update(void)
{
	// 入力の更新処理
	UpdateInput();

	// ライトの更新処理
	UpdateLight();

	// カメラ更新
	UpdateCamera();

	// サウンドのフェード更新
	UpdateSoundFade();


	// モードによって処理を分ける
	switch (g_Mode)
	{
	case MODE_TITLE:		
		// オプション画面時はスキップ
		if (GetOptionUse() == FALSE)
		{
			// タイトル画面の更新
			UpdateTitle();

			// タイトルで使用するプレイヤーの更新処理
			UpdateTitlePlayer();

			// スクリーンの更新処理
			UpdateScreen();
		}

		// オプションの更新処理
		UpdateOption();

		break;

	case MODE_GAME:			// ゲーム画面の更新
		UpdateGame();
		break;

	case MODE_CLEAR:		// リザルト画面の更新
	case MODE_OVER:			// リザルト画面の更新
		UpdateResult();
		break;

	case MODE_ENDROLL:		// エンドロール画面の更新
		UpdateEndroll();
		break;
	}

	// フェード処理の更新
	UpdateFade();


}

//=============================================================================
// 描画処理
//=============================================================================
void Draw(void)
{
	// バックバッファクリア
	Clear();

	SetDirectionalLight();

	//SetCamera();

	// モードによって処理を分ける
	switch (g_Mode)
	{
	case MODE_TITLE:		// タイトル画面の描画
		DrawGameTitle();
		break;

	case MODE_GAME:			// ゲーム画面の描画
		DrawGame();
		break;

	case MODE_CLEAR:		// リザルト画面の描画
	case MODE_OVER:			// リザルト画面の描画
		SetViewPort(TYPE_FULL_SCREEN);

		// 2Dの物を描画する処理
		// Z比較なし
		SetDepthEnable(FALSE);

		// ライティングを無効
		SetLightEnable(FALSE);

		DrawResult();

		// ライティングを有効に
		SetLightEnable(TRUE);

		// Z比較あり
		SetDepthEnable(TRUE);
		break;

	case MODE_ENDROLL:		// エンドロール画面の描画
		SetViewPort(TYPE_FULL_SCREEN);

		// 2Dの物を描画する処理
		// Z比較なし
		SetDepthEnable(FALSE);

		// ライティングを無効
		SetLightEnable(FALSE);

		DrawEndroll();

		// ライティングを有効に
		SetLightEnable(TRUE);

		// Z比較あり
		SetDepthEnable(TRUE);
		break;
	}

	// フェード描画
	DrawFade();

#ifdef _DEBUG
	// デバッグ表示
	DrawDebugProc();
#endif

	// バックバッファ、フロントバッファ入れ替え
	Present();
}


long GetMousePosX(void)
{
	return g_MouseX;
}


long GetMousePosY(void)
{
	return g_MouseY;
}


#ifdef _DEBUG
char* GetDebugStr(void)
{
	return g_DebugStr;
}
#endif



//=============================================================================
// モードの設定
//=============================================================================
void SetMode(int mode)
{
	// モードを変える前に全部メモリを解放する

	// スクリーンの終了処理
	UninitScreen();

	// オプションの終了処理
	UninitOption();

	// タイトル画面の終了処理
	UninitTitle();

	// タイトルで使用するプレイヤーの終了処理
	UninitTitlePlayer();

	// ゲーム画面の終了処理
	UninitGame();

	// リザルト画面の終了処理
	UninitResult();

	// エンドロール画面の終了処理
	UninitEndroll();


	g_Mode = mode;	// 次のモードをセットしている

	switch (g_Mode)
	{
	case MODE_TITLE:
		// タイトル画面の初期化
		InitTitle();

		// タイトルで使用するプレイヤーの初期化
		InitTitlePlayer();

		// オプションの初期化
		InitOption();

		// スクリーンの初期化
		InitScreen();

		break;

	case MODE_GAME:
		// ゲーム画面の初期化
		InitGame();
		break;

	case MODE_CLEAR:
	case MODE_OVER:
		// リザルト画面の初期化
		InitResult();
		break;

	case MODE_ENDROLL:
		// エンドロール画面の初期化
		InitEndroll();
		break;

		// ゲーム終了時の処理
	case MODE_MAX:
		// エネミーの終了処理
		UninitEnemy();

		// プレイヤーの終了処理
		UninitPlayer();
		break;
	}
}

//=============================================================================
// モードの取得
//=============================================================================
int GetMode(void)
{
	return g_Mode;
}


//=============================================================================
// 角度をラジアンに変換する関数
//=============================================================================
float RadianSum(float angle)
{
	float radian = angle * XM_PI / 180.0f;

	return radian;
}


//=============================================================================
// タイトル画面の一括表示
//=============================================================================
void DrawGameTitle(void)
{
	SetViewPort(TYPE_FULL_SCREEN);

	// ノイズテクスチャを登録
	SetShaderResouces(SHADER_RESOURCE_MODE_NOISE);

	// オブジェクトをPREレンダーターゲットへ描画
	SetShaderType(SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ);
	{
		// タイトルで使用するプレイヤーの更新処理
		DrawTitlePlayer();
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

	// タイトルの描画
	DrawTitle();

	// オプションの描画処理
	DrawOption();


	// ライティングを有効に
	SetLightEnable(TRUE);

	// Z比較あり
	SetDepthEnable(TRUE);
}