//=============================================================================
//
// ���C������ [main.cpp]
// Author : GP11A132 10 �F�V�`�O
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
// �}�N����`
//*****************************************************************************
#define CLASS_NAME		"AppClass"			// �E�C���h�E�̃N���X��
#define WINDOW_NAME		"MAGIC TIME"		// �E�C���h�E�̃L���v�V������

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);
void DrawGameTitle(void);


//*****************************************************************************
// �O���[�o���ϐ�:
//*****************************************************************************
long g_MouseX = 0;
long g_MouseY = 0;


#ifdef _DEBUG
int		g_CountFPS;							// FPS�J�E���^
char	g_DebugStr[2048] = WINDOW_NAME;		// �f�o�b�O�����\���p

#endif

int	g_Mode = MODE_TITLE;					// �N�����̉�ʂ�ݒ�


//=============================================================================
// ���C���֐�
//=============================================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);	// �����Ă��ǂ����ǁA�x�����o��i���g�p�錾�j
	UNREFERENCED_PARAMETER(lpCmdLine);		// �����Ă��ǂ����ǁA�x�����o��i���g�p�錾�j

	// ���Ԍv���p
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
	
	// �E�B���h�E�N���X�̓o�^
	RegisterClassEx(&wcex);

	// �E�B���h�E�̍쐬
	hWnd = CreateWindow(CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,																		// �E�B���h�E�̍����W
		CW_USEDEFAULT,																		// �E�B���h�E�̏���W
		SCREEN_WIDTH + GetSystemMetrics(SM_CXDLGFRAME) * 2,									// �E�B���h�E����
		SCREEN_HEIGHT + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),	// �E�B���h�E�c��
		NULL,
		NULL,
		hInstance,
		NULL);

	// �E�B���h�E���[�h���t���X�N���[�����[�h���̏���
	bool mode = true;

	int id = MessageBox(NULL, "Window���[�h�Ńv���C���܂����H", "�N�����[�h", MB_YESNOCANCEL | MB_ICONQUESTION);
	switch (id)
	{
	case IDYES:		// Yes�Ȃ�Window���[�h�ŋN��
		mode = true;
		break;
	case IDNO:		// No�Ȃ�t���X�N���[�����[�h�ŋN��
		mode = false;
		break;
	case IDCANCEL:	// CANCEL�Ȃ�I��
	default:
		return -1;
		break;
	}

	// ����������(�E�B���h�E���쐬���Ă���s��)
	if(FAILED(Init(hInstance, hWnd, mode)))
	{
		return -1;
	}

	// �t���[���J�E���g������
	timeBeginPeriod(1);	// ����\��ݒ�
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// �V�X�e���������~���b�P�ʂŎ擾
	dwCurrentTime = dwFrameCount = 0;

	// �E�C���h�E�̕\��(�����������̌�ɌĂ΂Ȃ��Ƒʖ�)
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	// ���b�Z�[�W���[�v
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{// PostQuitMessage()���Ă΂ꂽ�烋�[�v�I��
				break;
			}
			else
			{
				// ���b�Z�[�W�̖|��Ƒ��o
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)	// 1�b���ƂɎ��s
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;				// FPS�𑪒肵��������ۑ�
				dwFrameCount = 0;							// �J�E���g���N���A
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))	// 1/60�b���ƂɎ��s
			{
				dwExecLastTime = dwCurrentTime;	// ��������������ۑ�

#ifdef _DEBUG	// �f�o�b�O�ł̎�����FPS��\������
				wsprintf(g_DebugStr, WINDOW_NAME);
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " FPS:%d", g_CountFPS);
#endif

				Update();			// �X�V����
				Draw();				// �`�揈��

#ifdef _DEBUG	// �f�o�b�O�ł̎������\������
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " MX:%d MY:%d", GetMousePosX(), GetMousePosY());
				SetWindowText(hWnd, g_DebugStr);
#endif

				dwFrameCount++;
			}
		}
	}

	timeEndPeriod(1);				// ����\��߂�

	// �E�B���h�E�N���X�̓o�^������
	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// �I������
	Uninit();

	return (int)msg.wParam;
}

//=============================================================================
// �v���V�[�W��
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
// ����������
//=============================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	// �����_���̏���������
	srand((unsigned)time(NULL));

	InitRenderer(hInstance, hWnd, bWindow);

	InitLight();

	InitCamera();

	// ���͏����̏�����
	InitInput(hInstance, hWnd);

	// �T�E���h�̏�����
	InitSound(hWnd);


	// ���C�g��L����
	SetLightEnable(TRUE);

	// �w�ʃ|���S�����J�����O
	SetCullingMode(CULL_MODE_BACK);


	// �t�F�[�h�̏�����
	InitFade();

	// �ŏ��̃��[�h���Z�b�g
	SetMode(g_Mode);	// ������SetMode�̂܂܂ŁI

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void Uninit(void)
{
	// �I���̃��[�h���Z�b�g
	SetMode(MODE_MAX);

	// �T�E���h�I������
	UninitSound();

	// �J�����̏I������
	UninitCamera();

	//���͂̏I������
	UninitInput();

	// �����_���[�̏I������
	UninitRenderer();
}

//=============================================================================
// �X�V����
//=============================================================================
void Update(void)
{
	// ���͂̍X�V����
	UpdateInput();

	// ���C�g�̍X�V����
	UpdateLight();

	// �J�����X�V
	UpdateCamera();

	// �T�E���h�̃t�F�[�h�X�V
	UpdateSoundFade();


	// ���[�h�ɂ���ď����𕪂���
	switch (g_Mode)
	{
	case MODE_TITLE:		
		// �I�v�V������ʎ��̓X�L�b�v
		if (GetOptionUse() == FALSE)
		{
			// �^�C�g����ʂ̍X�V
			UpdateTitle();

			// �^�C�g���Ŏg�p����v���C���[�̍X�V����
			UpdateTitlePlayer();

			// �X�N���[���̍X�V����
			UpdateScreen();
		}

		// �I�v�V�����̍X�V����
		UpdateOption();

		break;

	case MODE_GAME:			// �Q�[����ʂ̍X�V
		UpdateGame();
		break;

	case MODE_CLEAR:		// ���U���g��ʂ̍X�V
	case MODE_OVER:			// ���U���g��ʂ̍X�V
		UpdateResult();
		break;

	case MODE_ENDROLL:		// �G���h���[����ʂ̍X�V
		UpdateEndroll();
		break;
	}

	// �t�F�[�h�����̍X�V
	UpdateFade();


}

//=============================================================================
// �`�揈��
//=============================================================================
void Draw(void)
{
	// �o�b�N�o�b�t�@�N���A
	Clear();

	SetDirectionalLight();

	//SetCamera();

	// ���[�h�ɂ���ď����𕪂���
	switch (g_Mode)
	{
	case MODE_TITLE:		// �^�C�g����ʂ̕`��
		DrawGameTitle();
		break;

	case MODE_GAME:			// �Q�[����ʂ̕`��
		DrawGame();
		break;

	case MODE_CLEAR:		// ���U���g��ʂ̕`��
	case MODE_OVER:			// ���U���g��ʂ̕`��
		SetViewPort(TYPE_FULL_SCREEN);

		// 2D�̕���`�悷�鏈��
		// Z��r�Ȃ�
		SetDepthEnable(FALSE);

		// ���C�e�B���O�𖳌�
		SetLightEnable(FALSE);

		DrawResult();

		// ���C�e�B���O��L����
		SetLightEnable(TRUE);

		// Z��r����
		SetDepthEnable(TRUE);
		break;

	case MODE_ENDROLL:		// �G���h���[����ʂ̕`��
		SetViewPort(TYPE_FULL_SCREEN);

		// 2D�̕���`�悷�鏈��
		// Z��r�Ȃ�
		SetDepthEnable(FALSE);

		// ���C�e�B���O�𖳌�
		SetLightEnable(FALSE);

		DrawEndroll();

		// ���C�e�B���O��L����
		SetLightEnable(TRUE);

		// Z��r����
		SetDepthEnable(TRUE);
		break;
	}

	// �t�F�[�h�`��
	DrawFade();

#ifdef _DEBUG
	// �f�o�b�O�\��
	DrawDebugProc();
#endif

	// �o�b�N�o�b�t�@�A�t�����g�o�b�t�@����ւ�
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
// ���[�h�̐ݒ�
//=============================================================================
void SetMode(int mode)
{
	// ���[�h��ς���O�ɑS�����������������

	// �X�N���[���̏I������
	UninitScreen();

	// �I�v�V�����̏I������
	UninitOption();

	// �^�C�g����ʂ̏I������
	UninitTitle();

	// �^�C�g���Ŏg�p����v���C���[�̏I������
	UninitTitlePlayer();

	// �Q�[����ʂ̏I������
	UninitGame();

	// ���U���g��ʂ̏I������
	UninitResult();

	// �G���h���[����ʂ̏I������
	UninitEndroll();


	g_Mode = mode;	// ���̃��[�h���Z�b�g���Ă���

	switch (g_Mode)
	{
	case MODE_TITLE:
		// �^�C�g����ʂ̏�����
		InitTitle();

		// �^�C�g���Ŏg�p����v���C���[�̏�����
		InitTitlePlayer();

		// �I�v�V�����̏�����
		InitOption();

		// �X�N���[���̏�����
		InitScreen();

		break;

	case MODE_GAME:
		// �Q�[����ʂ̏�����
		InitGame();
		break;

	case MODE_CLEAR:
	case MODE_OVER:
		// ���U���g��ʂ̏�����
		InitResult();
		break;

	case MODE_ENDROLL:
		// �G���h���[����ʂ̏�����
		InitEndroll();
		break;

		// �Q�[���I�����̏���
	case MODE_MAX:
		// �G�l�~�[�̏I������
		UninitEnemy();

		// �v���C���[�̏I������
		UninitPlayer();
		break;
	}
}

//=============================================================================
// ���[�h�̎擾
//=============================================================================
int GetMode(void)
{
	return g_Mode;
}


//=============================================================================
// �p�x�����W�A���ɕϊ�����֐�
//=============================================================================
float RadianSum(float angle)
{
	float radian = angle * XM_PI / 180.0f;

	return radian;
}


//=============================================================================
// �^�C�g����ʂ̈ꊇ�\��
//=============================================================================
void DrawGameTitle(void)
{
	SetViewPort(TYPE_FULL_SCREEN);

	// �m�C�Y�e�N�X�`����o�^
	SetShaderResouces(SHADER_RESOURCE_MODE_NOISE);

	// �I�u�W�F�N�g��PRE�����_�[�^�[�Q�b�g�֕`��
	SetShaderType(SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ);
	{
		// �^�C�g���Ŏg�p����v���C���[�̍X�V����
		DrawTitlePlayer();
	}


	// 2D�̕���`�悷�鏈��
	// Z��r�Ȃ�
	SetDepthEnable(FALSE);

	// ���C�e�B���O�𖳌�
	SetLightEnable(FALSE);

	RunBlur();


	// PRErt���P�x���ort��LUMINACE�V�F�[�_�\�ŏ�������
	SetShaderType(SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS);
	{
		// �X�N���[���̕`�揈��
		DrawScreen();

		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// x�����ւ̃u���[���H
	// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	SetShaderType(SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE);
	{
		// �X�N���[���̕`�揈��
		DrawScreen();

		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// y�����ւ̃u���[���H
	// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	SetShaderType(SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR);
	{
		// �X�N���[���̕`�揈��
		DrawScreen();

		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// y�����ւ̃u���[���H
	// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	SetShaderType(SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS);
	{
		// �X�N���[���̕`�揈��
		DrawScreen();

		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);
	}

	// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
	SetShaderResouces(SHADER_RESOURCE_MODE_CLEAR);

	// �^�C�g���̕`��
	DrawTitle();

	// �I�v�V�����̕`�揈��
	DrawOption();


	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// Z��r����
	SetDepthEnable(TRUE);
}