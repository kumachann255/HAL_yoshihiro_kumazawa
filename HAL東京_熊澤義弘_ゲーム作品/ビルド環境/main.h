//=============================================================================
//
// ���C������ [main.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


#pragma warning(push)
#pragma warning(disable:4005)

#define _CRT_SECURE_NO_WARNINGS			// scanf ��warning�h�~
#include <stdio.h>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>


// �{���̓w�b�_�ɏ����Ȃ������ǂ�
using namespace DirectX;


#define DIRECTINPUT_VERSION 0x0800		// �x���Ώ�
#include "dinput.h"
#include "mmsystem.h"

#pragma warning(pop)


//*****************************************************************************
// ���C�u�����̃����N
//*****************************************************************************
#pragma comment (lib, "d3d11.lib")		
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "d3dx11.lib")	
//#pragma comment (lib, "d3dx9.lib")	
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "dxerr.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dinput8.lib")


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define SCREEN_WIDTH	(960)			// �E�C���h�E�̕�
#define SCREEN_HEIGHT	(540)			// �E�C���h�E�̍���
#define SCREEN_CENTER_X	(SCREEN_WIDTH / 2)	// �E�C���h�E�̒��S�w���W
#define SCREEN_CENTER_Y	(SCREEN_HEIGHT / 2)	// �E�C���h�E�̒��S�x���W

#define	MAP_W			(2000.0f)
#define	MAP_H			(1000.0f)
#define	MAP_TOP			(MAP_H/2)
#define	MAP_DOWN		(-MAP_H/2)
#define	MAP_LEFT		(-MAP_W * 2/3)
#define	MAP_RIGHT		(MAP_W * 2/3)

#define MAP_OFFSET_X	(1150.0f)		// �}�b�v�����炵�āAx���W0.0f���X�^�[�g�n�_�ɂ���

// �}�b�v
#define MAP_LIMIT_X_MAX	(2600.0f)		// �E�[
#define MAP_LIMIT_X_MIN	(-200.0f)		// ���[
#define MAP_LIMIT_Z_MAX	( 140.0f)		// ��[
#define MAP_LIMIT_Z_MIN	(-140.0f)		// ���[

#define MAP_CIRCLE_LEFT	(1800.0f)		// �~�^�t�B�[���h��x���W�̎n�܂�
#define MAP_CIRCLE_R	(500.0f)		// �~�^�t�B�[���h�̔��a
#define MAP_OFFSTT_X	(100.0f)		// ������

// ��ԗp�̃f�[�^�\���̂��`
struct INTERPOLATION_DATA
{
	XMFLOAT3	pos;		// ���_���W
	XMFLOAT3	rot;		// ��]
	XMFLOAT3	scl;		// �g��k��
	float		frame;		// ���s�t���[���� ( dt = 1.0f/frame )
};

enum
{
	MODE_TITLE = 0,			// �^�C�g�����
	MODE_TUTORIAL,			// �Q�[���������
	MODE_GAME,				// �Q�[�����
	MODE_CLEAR,				// �N���A���
	MODE_OVER,				// �Q�[���I�[�o�[���
	MODE_ENDROLL,			// �G���h���[�����
	MODE_MAX
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
long GetMousePosX(void);
long GetMousePosY(void);
char* GetDebugStr(void);

void SetMode(int mode);
int GetMode(void);
float RadianSum(float angle);

