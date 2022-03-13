//=============================================================================
//
// �T�E���h���� [sound.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "sound.h"
#include "model.h"
#include "bahamut.h"
#include "main.h"
#include <DirectXMath.h>

//*****************************************************************************
// �p�����[�^�\���̒�`
//*****************************************************************************
typedef struct
{
	char *pFilename;	// �t�@�C����
	int nCntLoop;		// ���[�v�J�E���g
} SOUNDPARAM;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
IXAudio2 *g_pXAudio2 = NULL;								// XAudio2�I�u�W�F�N�g�ւ̃C���^�[�t�F�C�X
IXAudio2MasteringVoice *g_pMasteringVoice = NULL;			// �}�X�^�[�{�C�X
IXAudio2SubmixVoice *g_apSubmixVoice;						// �T�u�~�b�N�X�{�C�X

IXAudio2SourceVoice *g_apSourceVoice[SOUND_LABEL_MAX] = {};	// �\�[�X�{�C�X
BYTE *g_apDataAudio[SOUND_LABEL_MAX] = {};					// �I�[�f�B�I�f�[�^
DWORD g_aSizeAudio[SOUND_LABEL_MAX] = {};					// �I�[�f�B�I�f�[�^�T�C�Y


// fade�Ŏg�p����֐�
static BOOL		g_Use;						// TURE:�g�p���Ă���
SOUND_FADE		g_Fade = SOUNDFADE_NONE;	// �t�F�[�h�̏��
int				g_label;					// ���̃��[�h
float			g_NowVolume;
float			g_NextVolume;
float			g_FadeRate;


// �e���f�ނ̃p�����[�^
SOUNDPARAM g_aParam[SOUND_LABEL_MAX] =
{
	{ (char*)"data/BGM/title_bgm.wav", -1 },	// BGM0
	{ (char*)"data/BGM/game_bgm.wav", -1 },	// BGM0
	{ (char*)"data/BGM/game_over_bgm.wav", -1 },	// BGM0
	{ (char*)"data/BGM/clear_bgm.wav", -1 },	// BGM0
	{ (char*)"data/BGM/end_bgm.wav", 0 },	// BGM0
	{ (char*)"data/BGM/bahamut_bgm.wav", 0 },	// BGM0

	{ (char*)"data/SE/PLAYER/gait.wav", -1 },			// �v���C���[�̑���
	{ (char*)"data/SE/PLAYER/player_damage0.wav", 0 },	// �v���C���[�̃_���[�W
	{ (char*)"data/SE/PLAYER/player_damage1.wav", 0 },	// �v���C���[�̃_���[�W
	{ (char*)"data/SE/PLAYER/player_dead0.wav", 0 },	// �v���C���[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/PLAYER/player_dead1.wav", 0 },	// �v���C���[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/PLAYER/player_dead2.wav", 0 },	// �v���C���[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/PLAYER/player_start0.wav", 0 },	// �v���C���[�̎n�܂������̊|����
	{ (char*)"data/SE/PLAYER/player_start1.wav", 0 },	// �v���C���[�̎n�܂������̊|����
	{ (char*)"data/SE/PLAYER/player_start2.wav", 0 },	// �v���C���[�̎n�܂������̊|����

	{ (char*)"data/SE/ENEMY/fly.wav", 0 },				// �G�l�~�[�̉H�΂�����
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// �G�l�~�[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// �G�l�~�[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// �G�l�~�[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// �G�l�~�[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// �G�l�~�[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// �G�l�~�[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/ENEMY/dead.wav", 0 },				// �G�l�~�[��HP��0�ɂȂ������̐�
	{ (char*)"data/SE/ENEMY/pop0.wav", 0 },				// �G�l�~�[�|�b�v�����Ƃ��̐�
	{ (char*)"data/SE/ENEMY/pop1.wav", 0 },				// �G�l�~�[�|�b�v�����Ƃ��̐�
	{ (char*)"data/SE/ENEMY/pop2.wav", 0 },				// �G�l�~�[�|�b�v�����Ƃ��̐�
	{ (char*)"data/SE/ENEMY/pop3.wav", 0 },				// �G�l�~�[�|�b�v�����Ƃ��̐�

	{ (char*)"data/SE/MAGIC/fire_01.wav", 0 },			// �����@
	{ (char*)"data/SE/MAGIC/fire_01.wav", 0 },			// �����@
	{ (char*)"data/SE/MAGIC/fire_01.wav", 0 },			// �����@
	{ (char*)"data/SE/MAGIC/lightning0.wav", 0 },		// �����@
	{ (char*)"data/SE/MAGIC/lightning0.wav", 0 },		// �����@
	{ (char*)"data/SE/MAGIC/lightning0.wav", 0 },		// �����@
	{ (char*)"data/SE/MAGIC/water0.wav", 0 },			// �����@
	{ (char*)"data/SE/MAGIC/water0.wav", 0 },			// �����@
	{ (char*)"data/SE/MAGIC/water1.wav", 0 },			// �����@
	{ (char*)"data/SE/MAGIC/holy_firing.wav", 0 },		// �����@
	{ (char*)"data/SE/MAGIC/holy_morphing.wav", 0 },	// �����@
	{ (char*)"data/SE/MAGIC/holy_set.wav", 0 },			// �����@

	{ (char*)"data/SE/BAHAMUT/circle.wav", 0 },			// �o�n���[�g�̖��@�w
	{ (char*)"data/SE/BAHAMUT/pop.wav", 0 },			// �o�n���[�g�̓o��
	{ (char*)"data/SE/BAHAMUT/open.wav", 0 },			// �o�n���[�g�̓o�ꎞ�̖���
	{ (char*)"data/SE/BAHAMUT/charge.wav", 0 },			// �o�n���[�g�̃u���X���`���[�W
	{ (char*)"data/SE/BAHAMUT/bless.wav", 0 },			// �o�n���[�g�̃u���X
	{ (char*)"data/SE/BAHAMUT/blast.wav", 0 },			// �o�n���[�g�̔���

	{ (char*)"data/SE/OPTION/option_set.wav", 0 },		// �I�v�V����
	{ (char*)"data/SE/OPTION/cursor_enter.wav", 0 },	// �I�v�V����
	{ (char*)"data/SE/OPTION/cursor_updown.wav", 0 },	// �I�v�V����
	{ (char*)"data/SE/OPTION/cursor_rightleft.wav", 0 },// �I�v�V����

	{ (char*)"data/SE/CIRCLE/magic_circle.wav", 0 },	// ���@�w


};

//=============================================================================
// ����������
//=============================================================================
BOOL InitSound(HWND hWnd)
{
	HRESULT hr;

	// COM���C�u�����̏�����
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// XAudio2�I�u�W�F�N�g�̍쐬
	hr = XAudio2Create(&g_pXAudio2, 0);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "XAudio2�I�u�W�F�N�g�̍쐬�Ɏ��s�I", "�x���I", MB_ICONWARNING);

		// COM���C�u�����̏I������
		CoUninitialize();

		return FALSE;
	}
	
	// �}�X�^�[�{�C�X�̐���
	hr = g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "�}�X�^�[�{�C�X�̐����Ɏ��s�I", "�x���I", MB_ICONWARNING);

		if(g_pXAudio2)
		{
			// XAudio2�I�u�W�F�N�g�̊J��
			g_pXAudio2->Release();
			g_pXAudio2 = NULL;
		}

		// COM���C�u�����̏I������
		CoUninitialize();

		return FALSE;
	}

	// �T�u�~�b�N�X�{�C�X�̐���
	hr = g_pXAudio2->CreateSubmixVoice(&g_apSubmixVoice, 2, 44800);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "�T�u�~�b�N�X�̐����Ɏ��s�I", "�x���I", MB_ICONWARNING);
		return FALSE;
	}

	// �T�E���h�f�[�^�̏�����
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		HANDLE hFile;
		DWORD dwChunkSize = 0;
		DWORD dwChunkPosition = 0;
		DWORD dwFiletype;
		WAVEFORMATEXTENSIBLE wfx;
		XAUDIO2_BUFFER buffer;

		// �o�b�t�@�̃N���A
		memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

		// �T�E���h�f�[�^�t�@�C���̐���
		hFile = CreateFile(g_aParam[nCntSound].pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(hWnd, "�T�E���h�f�[�^�t�@�C���̐����Ɏ��s�I(1)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{// �t�@�C���|�C���^��擪�Ɉړ�
			MessageBox(hWnd, "�T�E���h�f�[�^�t�@�C���̐����Ɏ��s�I(2)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
	
		// WAVE�t�@�C���̃`�F�b�N
		hr = CheckChunk(hFile, 'FFIR', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVE�t�@�C���̃`�F�b�N�Ɏ��s�I(1)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &dwFiletype, sizeof(DWORD), dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVE�t�@�C���̃`�F�b�N�Ɏ��s�I(2)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		if(dwFiletype != 'EVAW')
		{
			MessageBox(hWnd, "WAVE�t�@�C���̃`�F�b�N�Ɏ��s�I(3)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
	
		// �t�H�[�}�b�g�`�F�b�N
		hr = CheckChunk(hFile, ' tmf', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�t�H�[�}�b�g�`�F�b�N�Ɏ��s�I(1)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�t�H�[�}�b�g�`�F�b�N�Ɏ��s�I(2)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}

		// �I�[�f�B�I�f�[�^�ǂݍ���
		hr = CheckChunk(hFile, 'atad', &g_aSizeAudio[nCntSound], &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�I�[�f�B�I�f�[�^�ǂݍ��݂Ɏ��s�I(1)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		g_apDataAudio[nCntSound] = (BYTE*)malloc(g_aSizeAudio[nCntSound]);
		hr = ReadChunkData(hFile, g_apDataAudio[nCntSound], g_aSizeAudio[nCntSound], dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�I�[�f�B�I�f�[�^�ǂݍ��݂Ɏ��s�I(2)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
	
		// �\�[�X�{�C�X�̐���
		hr = g_pXAudio2->CreateSourceVoice(&g_apSourceVoice[nCntSound], &(wfx.Format), 0, 10.0f);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�\�[�X�{�C�X�̐����Ɏ��s�I", "�x���I", MB_ICONWARNING);
			return FALSE;
		}

		// �o�b�t�@�̒l�ݒ�
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
		buffer.AudioBytes = g_aSizeAudio[nCntSound];
		buffer.pAudioData = g_apDataAudio[nCntSound];
		buffer.Flags      = XAUDIO2_END_OF_STREAM;
		buffer.LoopCount  = g_aParam[nCntSound].nCntLoop;

		// �I�[�f�B�I�o�b�t�@�̓o�^
		g_apSourceVoice[nCntSound]->SubmitSourceBuffer(&buffer);
	}

	// �e�\�[�X�{�C�X�̒���
	SetFrequencyRatio(SOUND_LABEL_SE_PLAYER_gait, 0.793f);		// ����SE�̑��x����
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_gait, 0.0f);			// ����SE����U�������Ȃ��悤�ɒ���

	// �\�[�X�{�C�X�̈ꊇ����
	SetSourceVolume(1.0f, 1.0f, 1.0f);

	return TRUE;
}

//=============================================================================
// �I������
//=============================================================================
void UninitSound(void)
{
	// �ꎞ��~
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// �ꎞ��~
			g_apSourceVoice[nCntSound]->Stop(0);
	
			// �\�[�X�{�C�X�̔j��
			g_apSourceVoice[nCntSound]->DestroyVoice();
			g_apSourceVoice[nCntSound] = NULL;
	
			// �I�[�f�B�I�f�[�^�̊J��
			free(g_apDataAudio[nCntSound]);
			g_apDataAudio[nCntSound] = NULL;
		}
	}
	
	// �}�X�^�[�{�C�X�̔j��
	g_pMasteringVoice->DestroyVoice();
	g_pMasteringVoice = NULL;
	
	if(g_pXAudio2)
	{
		// XAudio2�I�u�W�F�N�g�̊J��
		g_pXAudio2->Release();
		g_pXAudio2 = NULL;
	}
	
	// COM���C�u�����̏I������
	CoUninitialize();
}


//=============================================================================
// �Z�O�����g�Đ�(�Đ����Ȃ��~)
//=============================================================================
void PlaySound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;
	XAUDIO2_BUFFER buffer;

	// �o�b�t�@�̒l�ݒ�
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
	buffer.AudioBytes = g_aSizeAudio[label];
	buffer.pAudioData = g_apDataAudio[label];
	buffer.Flags      = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount  = g_aParam[label].nCntLoop;

	// ��Ԏ擾
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// �Đ���
		// �ꎞ��~
		g_apSourceVoice[label]->Stop(0);

		// �I�[�f�B�I�o�b�t�@�̍폜
		g_apSourceVoice[label]->FlushSourceBuffers();
	}

	// �\�[�X�{�C�X�̏o�͂��T�u�~�b�N�X�{�C�X�ɐ؂�ւ�
	XAUDIO2_SEND_DESCRIPTOR send = { 0, g_apSubmixVoice };
	XAUDIO2_VOICE_SENDS sendlist = { 1, &send };
	g_apSourceVoice[label]->SetOutputVoices(&sendlist);

	// �I�[�f�B�I�o�b�t�@�̓o�^
	g_apSourceVoice[label]->SubmitSourceBuffer(&buffer);

	// �Đ�
	g_apSourceVoice[label]->Start(0);

}

//=============================================================================
// �Z�O�����g��~(���x���w��)
//=============================================================================
void StopSound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;

	// ��Ԏ擾
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// �Đ���
		// �ꎞ��~
		g_apSourceVoice[label]->Stop(0);

		// �I�[�f�B�I�o�b�t�@�̍폜
		g_apSourceVoice[label]->FlushSourceBuffers();
	}
}

//=============================================================================
// �Z�O�����g��~(�S��)
//=============================================================================
void StopSound(void)
{
	// �ꎞ��~
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// �ꎞ��~
			g_apSourceVoice[nCntSound]->Stop(0);
		}
	}
}

//=============================================================================
// �`�����N�̃`�F�b�N
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
	{// �t�@�C���|�C���^��擪�Ɉړ�
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	while(hr == S_OK)
	{
		if(ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) == 0)
		{// �`�����N�̓ǂݍ���
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if(ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0)
		{// �`�����N�f�[�^�̓ǂݍ���
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch(dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize  = dwChunkDataSize;
			dwChunkDataSize = 4;
			if(ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) == 0)
			{// �t�@�C���^�C�v�̓ǂݍ���
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if(SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{// �t�@�C���|�C���^���`�����N�f�[�^���ړ�
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
// �`�����N�f�[�^�̓ǂݍ���
//=============================================================================
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset)
{
	DWORD dwRead;
	
	if(SetFilePointer(hFile, dwBufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// �t�@�C���|�C���^���w��ʒu�܂ňړ�
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if(ReadFile(hFile, pBuffer, dwBuffersize, &dwRead, NULL) == 0)
	{// �f�[�^�̓ǂݍ���
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	return S_OK;
}



// �}�X�^�[�{�C�X�̉��ʒ���(0.0f ~ 1.0f�Œ���)
void SetMasterVolume(float volume)
{
	volume *= volume;
	g_pMasteringVoice->SetVolume(volume);
	return;
}


// �\�[�X�{�C�X�̉��ʒ���(0.0f ~ 1.0f�Œ���)
void SetSourceVolume(int label, float volume)
{
	g_apSourceVoice[label]->SetVolume(volume);
	return;
}


// �T�E���h�̃t�F�[�h����(TargetVolume:�ŏI�I�ȉ��ʁATargetTime:���b�Ńt�F�[�h������������)
void SetSoundFade(int label, SOUND_FADE fadetype, float TargetVolume, float TargetTime)
{
	g_Fade = fadetype;
	g_label = label;
	g_NextVolume = TargetVolume;
	g_NowVolume = 1.0f;
	g_FadeRate = fabsf(g_NowVolume - g_NextVolume) / (TargetTime * 60.0f);
	return;
}


// �^�C�g����p
// �T�E���h�̃t�F�[�h����(TargetVolume:�ŏI�I�ȉ��ʁATargetTime:���b�Ńt�F�[�h������������)
void SetTitleSoundFade(int label, SOUND_FADE fadetype, float TargetVolume, float TargetTime)
{
	g_Fade = fadetype;
	g_label = label;
	g_NextVolume = TargetVolume;
	g_NowVolume = 0.2f;
	g_FadeRate = fabsf(g_NowVolume - g_NextVolume) / (TargetTime * 60.0f);
	return;
}


//�T�E���h�̃t�F�[�h�X�V����
void UpdateSoundFade(void)
{
	if (g_Fade != SOUNDFADE_NONE)
	{// �t�F�[�h������
		if (g_Fade == SOUNDFADE_OUT)
		{// �t�F�[�h�A�E�g����
			g_NowVolume -= g_FadeRate;		// ���Z
			if (g_NowVolume > g_NextVolume)
			{
				g_apSourceVoice[g_label]->SetVolume(g_NowVolume);
			}
			else
			{
				// �t�F�[�h�A�E�g�����I��
				g_apSourceVoice[g_label]->SetVolume(0.0f);
				g_apSourceVoice[g_label]->Stop();
				g_Fade = SOUNDFADE_NONE;

				// ��������������Ȃ��Đ�
				if (GetSummon())
				{
					// BGM���Đ�
					PlaySound(SOUND_LABEL_BGM_bahamut);
					SetSoundFade(SOUND_LABEL_BGM_bahamut, SOUNDFADE_IN, 0.1f, 1.0f);
				}
				else if (GetMode() == MODE_GAME)
				{
					// BGM���Đ�
					PlaySound(SOUND_LABEL_BGM_game);
					SetTitleSoundFade(SOUND_LABEL_BGM_game, SOUNDFADE_IN, 0.08f, 1.0f);
				}
			}
		}

		else if (g_Fade == SOUNDFADE_IN)
		{// �t�F�[�h�C������
			g_NowVolume += g_FadeRate;		// ���Z
			g_apSourceVoice[g_label]->SetVolume(g_NowVolume);

			if (g_NowVolume >= g_NextVolume)
			{
				// �t�F�[�h�C�������I��
				g_Fade = SOUNDFADE_NONE;
			}
		}
	}
}


//=============================================================================
// �\�[�X�{�C�X�̃{�C�X�̍Đ��s�b�`����
//=============================================================================
void SetFrequencyRatio(int label, float Pitch)
{
	g_apSourceVoice[label]->SetFrequencyRatio(Pitch);
	return;
}


//=============================================================================
// �\�[�X�{�C�X�̈ꎞ��~
//=============================================================================
void PauseSound(int label)
{
	g_apSourceVoice[label]->Stop(XAUDIO2_PLAY_TAILS);
	return;
}

//=============================================================================
// �\�[�X�{�C�X�̍ĊJ
//=============================================================================
void ReStartSound(int label)
{
	g_apSourceVoice[label]->Start();
	return;
}

//=============================================================================
// �\�[�X�{�C�X�̉��ʒ���
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


	// �v���C���[
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_damage0, 1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_damage1, 1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_dead0,   1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_dead1,   1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_start0,  1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_start1,  1.0f * voice);
	SetSourceVolume(SOUND_LABEL_SE_PLAYER_start2,  1.0f * voice);


	// �G�l�~�[
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


	// �����@
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_fire0, 0.2f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_fire1, 0.2f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_fire2, 0.2f * se);


	// �����@
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_lightning01, 0.4f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_lightning02, 0.4f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_lightning03, 0.4f * se);


	// �����@
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_water00, 0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_water01, 0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_water1,  1.2f * se);


	// �����@
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_holy_firing,	0.2f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_holy_morphing, 0.4f * se);
	SetSourceVolume(SOUND_LABEL_SE_MAGIC_holy_set,		0.4f * se);


	// ���@�w
	SetSourceVolume(SOUND_LABEL_SE_CIRCLE_circle, 1.0f * se);


	// �o�n���[�g��SE
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_circle,	1.0f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_pop,		1.0f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_open,	4.0f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_charge,	0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_bless,	1.0f * se);
	SetSourceVolume(SOUND_LABEL_SE_BAHAMUT_blast,	2.0f * se);



	// �I�v�V�����̌��ʉ�
	SetSourceVolume(SOUND_LABEL_SE_OPTION_option_set,		0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_OPTION_cursor_enter,		0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_OPTION_cursol_updown,	0.7f * se);
	SetSourceVolume(SOUND_LABEL_SE_OPTION_cursor_rightleft, 0.7f * se);



}