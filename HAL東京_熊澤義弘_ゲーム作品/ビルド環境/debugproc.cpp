//=============================================================================
//
// デバッグ表示処理 [debugproc.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include <stdio.h>
#include "debugproc.h"
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

//*****************************************************************************
// グローバル変数
//*****************************************************************************
char		g_aStrDebug[1024] = {"\0"};	// デバッグ情報


//=============================================================================
// デバッグ表示処理の描画処理
//=============================================================================
void DrawDebugProc(void)
{
	RECT rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

	DebugTextOut(g_aStrDebug, 0, 0);

	// 情報クリア
	memset(g_aStrDebug, 0, sizeof g_aStrDebug);
}

//=============================================================================
// デバッグ表示の登録
//=============================================================================
void PrintDebugProc(char *fmt,...)
{
#if 0
	long *pParam;
	static char aBuf[256];

	pParam = (long*)&fmt;
	sprintf(aBuf, fmt, pParam[1], pParam[2], pParam[3], pParam[4],
									pParam[5], pParam[6], pParam[7], pParam[8],
									pParam[9], pParam[10], pParam[11], pParam[12]);
#else
	va_list list;			// 可変引数を処理する為に使用する変数
	char *pCur;
	char aBuf[256]={"\0"};
	char aWk[32];

	// 可変引数にアクセスする前の初期処理
	va_start(list, fmt);

	pCur = fmt;
	for( ; *pCur; ++pCur)
	{
		if(*pCur != '%')
		{
			sprintf_s(aWk, "%c", *pCur);
		}
		else
		{
			pCur++;

			switch(*pCur)
			{
			case 'd':
				// 可変引数にアクセスしてその変数を取り出す処理
				sprintf_s(aWk, "%d", va_arg(list, int));
				break;

			case 'f':
				// 可変引数にアクセスしてその変数を取り出す処理
				sprintf_s(aWk, "%.2f", va_arg(list, double));		// double型で指定
				break;

			case 's':
				// 可変引数にアクセスしてその変数を取り出す処理
				sprintf_s(aWk, "%s", va_arg(list, char*));
				break;

			case 'c':
				// 可変引数にアクセスしてその変数を取り出す処理
				sprintf_s(aWk, "%c", va_arg(list, char));
				break;

			default:
				sprintf_s(aWk, "%c", *pCur);
				break;
			}
		}
		strcat_s(aBuf, aWk);
	}

	// 可変引数にアクセスした後の終了処理
	va_end(list);

	// 連結
	if((strlen(g_aStrDebug) + strlen(aBuf)) < ((sizeof g_aStrDebug) - 1))
	{
		strcat_s(g_aStrDebug, aBuf);
	}
#endif
}

