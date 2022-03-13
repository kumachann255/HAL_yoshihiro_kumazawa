//=============================================================================
//
// 木処理 [tree.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBarrier(void);
void UninitBarrier(void);
void UpdateBarrier(void);
void DrawBarrier(void);

int SetBarrier(XMFLOAT3 pos, float fWidth, float fHeight, XMFLOAT4 col);


