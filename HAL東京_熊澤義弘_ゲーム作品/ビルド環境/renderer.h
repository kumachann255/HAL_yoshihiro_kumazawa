//=============================================================================
//
// �����_�����O���� [renderer.h]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once


//*********************************************************
// �}�N����`
//*********************************************************
#define LIGHT_MAX		(5)

enum LIGHT_TYPE
{
	LIGHT_TYPE_NONE,		//���C�g����
	LIGHT_TYPE_DIRECTIONAL,	//�f�B���N�V���i�����C�g
	LIGHT_TYPE_POINT,		//�|�C���g���C�g

	LIGHT_TYPE_NUM
};

enum BLEND_MODE
{
	BLEND_MODE_NONE,		//�u�����h����
	BLEND_MODE_ALPHABLEND,	//���u�����h
	BLEND_MODE_ADD,			//���Z�u�����h
	BLEND_MODE_SUBTRACT,	//���Z�u�����h

	BLEDD_MODE_NUM
};

enum CULL_MODE
{
	CULL_MODE_NONE,			//�J�����O����
	CULL_MODE_FRONT,		//�\�̃|���S����`�悵�Ȃ�(CW)
	CULL_MODE_BACK,			//���̃|���S����`�悵�Ȃ�(CCW)

	CULL_MODE_NUM
};

// �V�F�[�_�[�̃��[�h
enum SHADER_MODE
{
	SHADER_MODE_DEFAULT,
	SHADER_MODE_LUMINACE,
	SHADER_MODE_LUMINACE_FIRE,
	SHADER_MODE_BLUR_X,
	SHADER_MODE_BLUR_Y,
	SHADER_MODE_COMPOSITE,
	SHADER_MODE_BACKBUFFER,
	SHADER_MODE_LIGHTNING,
	SHADER_MODE_SHADOWMAP,
	SHADER_MODE_SHADOWPASTE,
	SHADER_MODE_CIRCLE,
	SHADER_MODE_ENEMY,
	SHADER_MODE_WAND,
	SHADER_MODE_BAHAMUT,
	SHADER_MODE_BAHAMUT_CIRCLE,
	SHADER_MODE_BAHAMUT_SURGE,
	SHADER_MODE_PARTICLE,
	SHADER_MODE_WATER,
	SHADER_MODE_BLAST,
};

// �����_�[�r���[�̃��[�h
enum RENDER_MODE
{
	VIEW_MODE_BACKBUFFER,
	VIEW_MODE_PREVIOUS,
	VIEW_MODE_LUMINACE,
	VIEW_MODE_BLUR_X,
	VIEW_MODE_BLUR_XY,
	VIEW_MODE_COMPOSITE,
	VIEW_MODE_SHADOWMAP,
};

// �V�F�[�_�\���\�[�X�r���[�̃��[�h
enum SHADER_RESOURCE_MODE
{
	SHADER_RESOURCE_MODE_PRE,
	SHADER_RESOURCE_MODE_LUMINACE,
	SHADER_RESOURCE_MODE_BLUR_X,
	SHADER_RESOURCE_MODE_BLUR_XY,
	SHADER_RESOURCE_MODE_COMPOSITE,
	SHADER_RESOURCE_MODE_SHADOWMAP,
	SHADER_RESOURCE_MODE_NOISE,
	SHADER_RESOURCE_MODE_NOISE_CENTER,
	SHADER_RESOURCE_MODE_NOISE_WATER,
	SHADER_RESOURCE_MODE_CLEAR,
};

// �V�F�[�_�[�̐ݒ�ꊇ�ϊ����[�h
enum SHADER_OPTION
{
	// rt�֕`��
	// �I�u�W�F�N�g��PRErt�փf�t�H���g�V�F�[�_�\�ŏ�������
	SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ,

	// �P�x���o
	// PRErt���P�x���ort��LUMINACE�V�F�[�_�\�ŏ�������
	SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS,

	// �P�x���o(fire�p)
	// fire���P�x���ort��LUMINACE_FIRE�V�F�[�_�\�ŏ�������
	SHADER_LUMINACE_FIRE__VIEW_LUMINACE__RESOUCE_FIRE,

	// x�����ւ̃u���[���H
	// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE,

	// y�����ւ̃u���[���H
	// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR,

	// ����
	// �P�x���ort��PRErt��back�o�b�t�@��Composite�V�F�[�_�\�ŏ�������
	SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS,

	// back�o�b�t�@��
	// PRErt��back�o�b�t�@��BACK�V�F�[�_�\�ŏ�������
	SHADER_BACK__VIEW_BACK__RESOURCE_PREVIOUS,

	// �u���[���H�e�X�g
	// �P�x���ort��back�o�b�t�@��Blur�V�F�[�_�\�ŏ�������
	SHADER_BLUR_X__VIEW_BACK__RESOURCE_LUMINACE,
	SHADER_BLUR_Y__VIEW_BACK__RESOURCE_LUMINACE,

	// �V���h�E�}�b�v��`��
	// �I�u�W�F�N�g���V���h�E�}�b�vrt��ShadowMap�V�F�[�_�\�ŏ�������
	SHADER_SHADOWMAP__VIEW_SHADOWMAP__RESOUCE_OBJ,

	// �V���h�E�}�b�v��back�o�b�t�@��
	// �V���h�E�}�b�vrt��back�o�b�t�@�փf�t�H���g�V�F�[�_�\�ŏ�������
	SHADER_DEFAULT__VIEW_BACK__RESOUCE_SHADOWMAP,

	// �e�̕t�����I�u�W�F�N�g��PRErt��
	// �V���h�E�}�b�vrt������PRErt�փV���h�E�y�[�X�g�V�F�[�_�\�ŏ�������
	SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ,

	// �f�B�]���u���������ĉe�̂Ȃ��I�u�W�F�N�g��PRErt��
	// �m�C�Y�e�N�X�`��������PRErt�֖��@�w�p�V�F�[�_�\�ŏ�������
	SHADER_CIRCLR__VIEW_PRE__RESOUCE_OBJ_NOISE,

	// �f�B�]���u���������ĉe�̕t�����I�u�W�F�N�g��PRErt��
	// �m�C�Y�e�N�X�`��������PRErt�փG�l�~�[�p�V�F�[�_�\�ŏ�������
	SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE,

	// �f�B�]���u�̏��������ĉe�̎g���Ȃ��I�u�W�F�N�g��PRErt��
	// �m�C�Y�e�N�X�`�������ɐ����@�ɉ��H������PRErt�֐����@�p�V�F�[�_�\�ŏ�������
	SHADER_WATER__VIEW_PRE__RESOUCE_OBJ_NOISE,

	// ���ȏ�̍������瓧���ɂ��鏈��
	// �萔�o�b�t�@�����ɔ�����PRErt�֔����p�V�F�[�_�[�ŏ�������
	SHADER_BLAST__VIEW_PRE__RESOUCE_OBJ,


	// �ʏ�`��
	// back�o�b�t�@�֒��ڏ�������
	DEFAULT,
};


//*********************************************************
// �\����
//*********************************************************

// ���_�\����
struct VERTEX_3D
{
    XMFLOAT3	Position;
    XMFLOAT3	Normal;
    XMFLOAT4	Diffuse;
    XMFLOAT2	TexCoord;
};

// �}�e���A���\����
struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	int			noTexSampling;
};

// ���C�g�\����
struct LIGHT {
	XMFLOAT4X4	mtxView;		// �r���[�}�g���b�N�X
	XMFLOAT4X4	mtxInvView;		// �r���[�}�g���b�N�X
	XMFLOAT4X4	mtxProjection;	// �v���W�F�N�V�����}�g���b�N�X

	XMFLOAT3	Direction;	// ���C�g�̕���
	XMFLOAT3	Position;	// ���C�g�̈ʒu
	XMFLOAT3	dir;		// ���C�g�̌���
	XMFLOAT3	At;			// ���C�g�̒����_
	XMFLOAT3	up;			// ���C�g�̏�����x�N�g��
	XMFLOAT4	Diffuse;	// �g�U���̐F
	XMFLOAT4	Ambient;	// �����̐F
	float		Attenuation;// ������
	int			Type;		// ���C�g��ʁE�L���t���O
	int			Enable;		// ���C�g��ʁE�L���t���O

};

// �t�H�O�\����
struct FOG {
	float		FogStart;	// �t�H�O�̊J�n����
	float		FogEnd;		// �t�H�O�̍ő勗��
	XMFLOAT4	FogColor;	// �t�H�O�̐F
};

// �u���[�\����
struct GaussBlurParam
{
	float		weights[8];
	XMFLOAT2	offset;
	XMFLOAT2	size;
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitRenderer(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void UninitRenderer(void);

void Clear(void);
void Present(void);

ID3D11Device *GetDevice( void );
ID3D11DeviceContext *GetDeviceContext( void );

void SetDepthEnable( BOOL Enable );
void SetBlendState(BLEND_MODE bm);
void SetCullingMode(CULL_MODE cm);
void SetAlphaTestEnable(BOOL flag);

void SetWorldViewProjection2D( void );
void SetWorldMatrix( XMMATRIX *WorldMatrix );
void SetViewMatrix( XMMATRIX *ViewMatrix );
void SetLightViewMatrix(XMMATRIX *ViewMatrix);
void SetProjectionMatrix( XMMATRIX *ProjectionMatrix );
void SetLightProjectionMatrix(XMMATRIX *ProjectionMatrix);

void SetMaterial( MATERIAL material );

void SetLightEnable(BOOL flag);
void SetLight(int index, LIGHT* light);

void SetFogEnable(BOOL flag);
void SetFog(FOG* fog);
BOOL GetFogSwitch(void);

void SetDissolve(float data);

void SetShaderBahamutCircle(XMFLOAT3 pos, bool open);

void DebugTextOut(char* text, int x, int y);

void SetFuchi(int flag);
void SetShaderCamera(XMFLOAT3 pos);

void SetShaderMode(int mode);
void SetRenderTargetView(int mode);
void SetShaderResouces(int mode);
void SetShaderType(int mode);

void GaussianDistribution(float dispersion);
void RunBlur(void);

int GetShaderMode(void);
