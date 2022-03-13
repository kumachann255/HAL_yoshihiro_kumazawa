//=============================================================================
//
// �؏��� [tree.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBarrier(void);
void UninitBarrier(void);
void UpdateBarrier(void);
void DrawBarrier(void);

int SetBarrier(XMFLOAT3 pos, float fWidth, float fHeight, XMFLOAT4 col);


