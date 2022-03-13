//=============================================================================
//
// �����_�����O���� [renderer.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "screen.h"
#include "model.h"
#include "enemy.h"
#include "light.h"
#include "beam_orbit.h"

#define TEXTURE_MAX				(4)			// �e�N�X�`���̐�

//�f�o�b�O�p��ʃe�L�X�g�o�͂�L���ɂ���
#define DEBUG_DISP_TEXTOUT
//�V�F�[�_�[�f�o�b�O�ݒ��L���ɂ���
//#define DEBUG_SHADER


//*********************************************************
// �\����
//*********************************************************

// �}�e���A���p�萔�o�b�t�@�\����
struct MATERIAL_CBUFFER
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	int			noTexSampling;
	float		Dummy[2];				// 16byte���E�p
};

// ���C�g�p�t���O�\����
struct LIGHTFLAGS
{
	int			Type;		//���C�g�^�C�v�ienum LIGHT_TYPE�j
	int         OnOff;		//���C�g�̃I��or�I�t�X�C�b�`
	int			Dummy[2];
};

// ���C�g�p�萔�o�b�t�@�\����
struct LIGHT_CBUFFER
{
	XMFLOAT4	Direction[LIGHT_MAX];	// ���C�g�̕���
	XMFLOAT4	Position[LIGHT_MAX];	// ���C�g�̈ʒu
	XMFLOAT4	Diffuse[LIGHT_MAX];		// �g�U���̐F
	XMFLOAT4	Ambient[LIGHT_MAX];		// �����̐F
	XMFLOAT4	Attenuation[LIGHT_MAX];	// ������
	LIGHTFLAGS	Flags[LIGHT_MAX];		// ���C�g���
	int			Enable;					// ���C�e�B���O�L���E�����t���O
	int			Dummy[3];				// 16byte���E�p
};

// �t�H�O�p�萔�o�b�t�@�\����
struct FOG_CBUFFER
{
	XMFLOAT4	Fog;					// �t�H�O��
	XMFLOAT4	FogColor;				// �t�H�O�̐F
	int			Enable;					// �t�H�O�L���E�����t���O
	float		Dummy[3];				// 16byte���E�p
};

// �����p�o�b�t�@
struct FUCHI
{
	int			fuchi;
	int			fill[3];
};

// �f�B�]���u�p�o�b�t�@
struct DISSOLVE
{
	float		threshold;				// 臒l
	int			Dummy[3];				// 16byte���E�p
};

// �o�n���[�g�̖��@�w�p�o�b�t�@
struct BAHAMUT
{
	XMFLOAT3	pos;					// ���W
	bool		open;					// �����L�������ǂ���
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
static void SetLightBuffer(void);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static D3D_FEATURE_LEVEL       g_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

static ID3D11Device*			g_D3DDevice = NULL;
static ID3D11DeviceContext*		g_ImmediateContext = NULL;
static IDXGISwapChain*			g_SwapChain = NULL;
static ID3D11RenderTargetView*	g_RenderTargetView = NULL;
static ID3D11RenderTargetView*	g_RenderTargetViewPre = NULL;				// backrt�ɏ������ޑO�ɂ��ׂĂ��������ރ����_�[�^�[�Q�b�g�̃r���[
static ID3D11RenderTargetView*	g_RenderTargetViewLuminance = NULL;			// �P�x���ort�֏������ރ����_�[�^�[�Q�b�g�̃r���[
static ID3D11RenderTargetView*	g_RenderTargetViewBlurX = NULL;				// X�����ւ̃u���[���Hrt�֏������ރ����_�[�^�[�Q�b�g�̃r���[
static ID3D11RenderTargetView*	g_RenderTargetViewBlurXY = NULL;			// XY�����ւ̃u���[���Hrt�֏������ރ����_�[�^�[�Q�b�g�̃r���[
static ID3D11RenderTargetView*	g_RenderTargetViewShadowMap = NULL;			// �V���h�E�}�b�vrt�֏������ރ����_�[�^�[�Q�b�g�̃r���[
static ID3D11DepthStencilView*	g_DepthStencilView = NULL;
static ID3D11ShaderResourceView*	g_ShaderResourceView = NULL;			// g_PreBuffer���V�F�[�_�\���\�[�X�Ƃ��Ďg�p���邽�߂̃r���[
static ID3D11ShaderResourceView*	g_ShaderResourceViewLuminace = NULL;	// g_LuminanceBuffer���V�F�[�_�\���\�[�X�Ƃ��Ďg�p���邽�߂̃r���[
static ID3D11ShaderResourceView*	g_ShaderResourceViewBlurX = NULL;		// g_BlurBuffer���V�F�[�_�\���\�[�X�Ƃ��Ďg�p���邽�߂̃r���[
static ID3D11ShaderResourceView*	g_ShaderResourceViewBlurXY = NULL;		// g_BlurBuffer���V�F�[�_�\���\�[�X�Ƃ��Ďg�p���邽�߂̃r���[
static ID3D11ShaderResourceView*	g_ShaderResourceViewShadowMap = NULL;	// g_ShadowMapBuffer���V�F�[�_�\���\�[�X�Ƃ��Ďg�p���邽�߂̃r���[
static ID3D11ShaderResourceView*	g_NoiseTexture[TEXTURE_MAX] = { NULL };	// �m�C�Y�e�N�X�`�����

static ID3D11Texture2D*			g_PreBuffer;								// back�o�b�t�@�ɏ������ޑO�ɂ��ׂĂ��������ރ����_�[�^�[�Q�b�g
static ID3D11Texture2D*			g_LuminanceBuffer;							// �P�x���o�����f�[�^���������ރ����_�[�^�[�Q�b�g
static ID3D11Texture2D*			g_BlurBufferX;								// X�����ւ̃u���[���H���������ރ����_�[�^�[�Q�b�g
static ID3D11Texture2D*			g_BlurBufferXY;								// XY�����ւ̃u���[���H���������ރ����_�[�^�[�Q�b�g
static ID3D11Texture2D*			g_ShadowMapBuffer;							// �V���h�E�}�b�v���������ރ����_�[�^�[�Q�b�g


static ID3D11VertexShader*		g_VertexShader = NULL;						// �ʏ�`��p�̒��_�V�F�[�_�[
static ID3D11VertexShader*		g_VertexShaderBlurX = NULL;					// x�����u���[�p�̒��_�V�F�[�_�[
static ID3D11VertexShader*		g_VertexShaderBlurY = NULL;					// y�����u���[�p�̒��_�V�F�[�_�[
static ID3D11VertexShader*		g_VertexShaderShadowMap = NULL;				// �V���h�E�}�b�v�p�̒��_�V�F�[�_�[
static ID3D11VertexShader*		g_VertexShaderShadowPaste = NULL;			// �e�`��p�̒��_�V�F�[�_�[
//static ID3D11VertexShader*		g_VertexShaderParticle = NULL;				// �p�[�e�B�N���p�̒��_�V�F�[�_�[
static ID3D11PixelShader*		g_PixelShader = NULL;						// �ʏ�`��p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderBackBuffer = NULL;				// back�o�b�t�@�ɂ��̂܂܏������ރs�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderLuminance = NULL;				// Luminance�����_�[�^�[�Q�b�g�֋P�x���o�����F���������ރs�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderBlurX = NULL;					// Luminance�����_�[�^�[�Q�b�g��x�����̃u���[���H���������ރs�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderBlurY = NULL;					// Luminance�����_�[�^�[�Q�b�g��y�����̃u���[���H���������ރs�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderComposite = NULL;				// �����p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderLightning = NULL;				// ���C�g�j���O�p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderShadowMap = NULL;				// �V���h�E�}�b�v�p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderShadowPaste = NULL;			// �e�`��p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderMagicCircle = NULL;			// ���@�w�p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderEnemy = NULL;					// �G�l�~�[�p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderWand = NULL;					// ��p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderBahamut = NULL;				// �o�n���[�g�p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderBahamutCircle = NULL;			// �o�n���[�g�̖��@�w�p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderBahamutSurge = NULL;			// �o�n���[�g�̔g���p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderParticle = NULL;				// �p�[�e�B�N���p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderWater = NULL;					// �����@�p�̃s�N�Z���V�F�[�_�[
static ID3D11PixelShader*		g_PixelShaderBlast = NULL;					// �����p�̃s�N�Z���V�F�[�_�[
static ID3D11InputLayout*		g_VertexLayout = NULL;
static ID3D11Buffer*			g_WorldBuffer = NULL;
static ID3D11Buffer*			g_ViewBuffer = NULL;						// �J��������̎��_
static ID3D11Buffer*			g_LightViewBuffer = NULL;					// ���C�g����̎��_
static ID3D11Buffer*			g_ProjectionBuffer = NULL;					// �J��������̎ˉe
static ID3D11Buffer*			g_LightProjectionBuffer = NULL;				// ���C�g����̎ˉe
//static ID3D11Buffer*			g_WVPBuffer = NULL;							// �C���X�^���V���O�p�s��
static ID3D11Buffer*			g_MaterialBuffer = NULL;
static ID3D11Buffer*			g_LightBuffer = NULL;
static ID3D11Buffer*			g_FogBuffer = NULL;
static ID3D11Buffer*			g_FuchiBuffer = NULL;
static ID3D11Buffer*			g_CameraBuffer = NULL;
static ID3D11Buffer*			g_BlurParaBuffer = NULL;
static ID3D11Buffer*			g_DissolveBuffer = NULL;
static ID3D11Buffer*			g_DissolveEnemyBuffer = NULL;
static ID3D11Buffer*			g_DissolveWandBuffer = NULL;
static ID3D11Buffer*			g_BahamutBuffer = NULL;						// �o�n���[�g�̖��@�w�̈ʒu

static ID3D11DepthStencilState* g_DepthStateEnable;
static ID3D11DepthStencilState* g_DepthStateDisable;

static ID3D11BlendState*		g_BlendStateNone;
static ID3D11BlendState*		g_BlendStateAlphaBlend;
static ID3D11BlendState*		g_BlendStateAdd;
static ID3D11BlendState*		g_BlendStateSubtract;
static BLEND_MODE				g_BlendStateParam;


static ID3D11RasterizerState*	g_RasterStateCullOff;
static ID3D11RasterizerState*	g_RasterStateCullCW;
static ID3D11RasterizerState*	g_RasterStateCullCCW;


static MATERIAL_CBUFFER	g_Material;
static LIGHT_CBUFFER	g_Light;
static FOG_CBUFFER		g_Fog;

static FUCHI			g_Fuchi;

static DISSOLVE			g_Dissolve;						// �f�B�]���u�p
static BAHAMUT			g_Circle;						// �o�n���[�g�̖��@�w�p
static float			g_weights[8];

static XMMATRIX			WVP[MAX_BEAM_ORBIT];

static int				g_ShaderMode = 0;				// ���݂̕`�惂�[�h


static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/noise.png",
	"data/TEXTURE/noise_center.png",
	"data/TEXTURE/noise_surge.png",
	"data/TEXTURE/noise_water.png",
};


ID3D11Device* GetDevice( void )
{
	return g_D3DDevice;
}


ID3D11DeviceContext* GetDeviceContext( void )
{
	return g_ImmediateContext;
}


void SetDepthEnable( BOOL Enable )
{
	if( Enable )
		g_ImmediateContext->OMSetDepthStencilState( g_DepthStateEnable, NULL );
	else
		g_ImmediateContext->OMSetDepthStencilState( g_DepthStateDisable, NULL );

}


void SetBlendState(BLEND_MODE bm)
{
	g_BlendStateParam = bm;

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	switch (g_BlendStateParam)
	{
	case BLEND_MODE_NONE:
		g_ImmediateContext->OMSetBlendState(g_BlendStateNone, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_ALPHABLEND:
		g_ImmediateContext->OMSetBlendState(g_BlendStateAlphaBlend, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_ADD:
		g_ImmediateContext->OMSetBlendState(g_BlendStateAdd, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_SUBTRACT:
		g_ImmediateContext->OMSetBlendState(g_BlendStateSubtract, blendFactor, 0xffffffff);
		break;
	}
}

void SetCullingMode(CULL_MODE cm)
{
	switch (cm)
	{
	case CULL_MODE_NONE:
		g_ImmediateContext->RSSetState(g_RasterStateCullOff);
		break;
	case CULL_MODE_FRONT:
		g_ImmediateContext->RSSetState(g_RasterStateCullCW);
		break;
	case CULL_MODE_BACK:
		g_ImmediateContext->RSSetState(g_RasterStateCullCCW);
		break;
	}
}

void SetAlphaTestEnable(BOOL flag)
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	if (flag)
		blendDesc.AlphaToCoverageEnable = TRUE;
	else
		blendDesc.AlphaToCoverageEnable = FALSE;

	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	switch (g_BlendStateParam)
	{
	case BLEND_MODE_NONE:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_ALPHABLEND:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_ADD:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_SUBTRACT:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	}

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState* blendState = NULL;
	g_D3DDevice->CreateBlendState(&blendDesc, &blendState);
	g_ImmediateContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

	if (blendState != NULL)
		blendState->Release();
}


void SetWorldViewProjection2D( void )
{
	XMMATRIX world;
	world = XMMatrixTranspose(XMMatrixIdentity());
	GetDeviceContext()->UpdateSubresource(g_WorldBuffer, 0, NULL, &world, 0, 0);

	XMMATRIX view;
	view = XMMatrixTranspose(XMMatrixIdentity());
	GetDeviceContext()->UpdateSubresource(g_ViewBuffer, 0, NULL, &view, 0, 0);

	XMMATRIX worldViewProjection;
	worldViewProjection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	worldViewProjection = XMMatrixTranspose(worldViewProjection);
	GetDeviceContext()->UpdateSubresource(g_ProjectionBuffer, 0, NULL, &worldViewProjection, 0, 0);
}


void SetWorldMatrix( XMMATRIX *WorldMatrix )
{
	XMMATRIX world;
	world = *WorldMatrix;
	world = XMMatrixTranspose(world);

	GetDeviceContext()->UpdateSubresource(g_WorldBuffer, 0, NULL, &world, 0, 0);
}

void SetViewMatrix(XMMATRIX *ViewMatrix )
{
	XMMATRIX view;
	view = *ViewMatrix;
	view = XMMatrixTranspose(view);

	GetDeviceContext()->UpdateSubresource(g_ViewBuffer, 0, NULL, &view, 0, 0);
}

// ���C�g���猩���r���[�̒萔�o�b�t�@���X�V
void SetLightViewMatrix(XMMATRIX *ViewMatrix)
{
	XMMATRIX view;
	view = *ViewMatrix;
	view = XMMatrixTranspose(view);

	GetDeviceContext()->UpdateSubresource(g_LightViewBuffer, 0, NULL, &view, 0, 0);
}

void SetProjectionMatrix( XMMATRIX *ProjectionMatrix )
{
	XMMATRIX projection;
	projection = *ProjectionMatrix;
	projection = XMMatrixTranspose(projection);

	GetDeviceContext()->UpdateSubresource(g_ProjectionBuffer, 0, NULL, &projection, 0, 0);
}

// ���C�g���猩���ˉe�ϊ��p�̒萔�o�b�t�@���X�V
void SetLightProjectionMatrix(XMMATRIX *ProjectionMatrix)
{
	XMMATRIX projection;
	projection = *ProjectionMatrix;
	projection = XMMatrixTranspose(projection);

	GetDeviceContext()->UpdateSubresource(g_LightProjectionBuffer, 0, NULL, &projection, 0, 0);
}
//
//// �C���X�^���V���O�p�̒萔�o�b�t�@���X�V
//void SetWVPMatrix(XMMATRIX *WVPMatrix)
//{
//	XMMATRIX WVP;
//	WVP = *WVPMatrix;
//	WVP = XMMatrixTranspose(WVP);
//
//	GetDeviceContext()->UpdateSubresource(g_WVPBuffer, 0, NULL, &WVP, 0, 0);
//}

void SetMaterial( MATERIAL material )
{
	g_Material.Diffuse = material.Diffuse;
	g_Material.Ambient = material.Ambient;
	g_Material.Specular = material.Specular;
	g_Material.Emission = material.Emission;
	g_Material.Shininess = material.Shininess;
	g_Material.noTexSampling = material.noTexSampling;

	GetDeviceContext()->UpdateSubresource( g_MaterialBuffer, 0, NULL, &g_Material, 0, 0 );
}

void SetLightBuffer(void)
{
	GetDeviceContext()->UpdateSubresource(g_LightBuffer, 0, NULL, &g_Light, 0, 0);
}

void SetLightEnable(BOOL flag)
{
	// �t���O���X�V����
	g_Light.Enable = flag;

	SetLightBuffer();
}

void SetLight(int index, LIGHT* pLight)
{
	g_Light.Position[index] = XMFLOAT4(pLight->Position.x, pLight->Position.y, pLight->Position.z, 0.0f);
	g_Light.Direction[index] = XMFLOAT4(pLight->Direction.x, pLight->Direction.y, pLight->Direction.z, 0.0f);
	g_Light.Diffuse[index] = pLight->Diffuse;
	g_Light.Ambient[index] = pLight->Ambient;
	g_Light.Flags[index].Type = pLight->Type;
	g_Light.Flags[index].OnOff = pLight->Enable;
	g_Light.Attenuation[index].x = pLight->Attenuation;

	SetLightBuffer();
}

void SetFogBuffer(void)
{
	GetDeviceContext()->UpdateSubresource(g_FogBuffer, 0, NULL, &g_Fog, 0, 0);
}

void SetFogEnable(BOOL flag)
{
	// �t���O���X�V����
	g_Fog.Enable = flag;

	SetFogBuffer();
}

void SetFog(FOG* pFog)
{
	g_Fog.Fog.x = pFog->FogStart;
	g_Fog.Fog.y = pFog->FogEnd;
	g_Fog.FogColor = pFog->FogColor;

	SetFogBuffer();
}

// �t�H�O�̈ꊇ�ύX�p
BOOL GetFogSwitch(void)
{
	return GetFogOnOff();
	//return g_Fog.Enable;
}


void SetFuchi(int flag)
{
	g_Fuchi.fuchi = flag;
	GetDeviceContext()->UpdateSubresource(g_FuchiBuffer, 0, NULL, &g_Fuchi, 0, 0);
}

// �f�B�]���u�o�b�t�@�̍X�V
void SetDissolve(float data)
{
	g_Dissolve.threshold = data;
	GetDeviceContext()->UpdateSubresource(g_DissolveBuffer, 0, NULL, &g_Dissolve, 0, 0);
}

void SetShaderCamera(XMFLOAT3 pos)
{
	XMFLOAT4 tmp = XMFLOAT4( pos.x, pos.y, pos.z, 0.0f );

	GetDeviceContext()->UpdateSubresource(g_CameraBuffer, 0, NULL, &tmp, 0, 0);
}

// �o�n���[�g�̖��@�w
void SetShaderBahamutCircle(XMFLOAT3 pos, bool open)
{
	g_Circle.pos = pos;
	g_Circle.open = open;

	GetDeviceContext()->UpdateSubresource(g_BahamutBuffer, 0, NULL, &g_Circle, 0, 0);
}



//=============================================================================
// ����������
//=============================================================================
HRESULT InitRenderer(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	HRESULT hr = S_OK;

	// �f�o�C�X�A�X���b�v�`�F�[���A�R���e�L�X�g����
	DWORD deviceFlags = 0;
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = SCREEN_WIDTH;
	sd.BufferDesc.Height = SCREEN_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = bWindow;

	//�f�o�b�O�����o�͗p�ݒ�
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	deviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

	hr = D3D11CreateDeviceAndSwapChain( NULL,
										D3D_DRIVER_TYPE_HARDWARE,
										NULL,
										deviceFlags,
										NULL,
										0,
										D3D11_SDK_VERSION,
										&sd,
										&g_SwapChain,
										&g_D3DDevice,
										&g_FeatureLevel,
										&g_ImmediateContext );
	if( FAILED( hr ) )
		return hr;

	//�f�o�b�O�����o�͗p�ݒ�
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	hr = g_SwapChain->ResizeBuffers(0, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE); // N.B. the GDI compatible flag
	if (FAILED(hr))
		return hr;
#endif

	// �K�E�X�u���[�̏�����
	const float dispersion_sq = 5.0f;
	GaussianDistribution(dispersion_sq * dispersion_sq);


	// �����_�[�^�[�Q�b�g�r���[�����A�ݒ�
	ID3D11Texture2D* pBackBuffer = NULL;
	g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	g_D3DDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_RenderTargetView );
	pBackBuffer->Release();


	// ���`��p�@PRE�����_�[�^�[�Q�b�g
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = sd.BufferDesc.Width;
		desc.Height = sd.BufferDesc.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.SampleDesc = sd.SampleDesc;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;


		// �����_�[�^�[�Q�b�g���쐬
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_PreBuffer);

		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// �����_�[�^�[�Q�b�g�r���[���쐬
		g_D3DDevice->CreateRenderTargetView(g_PreBuffer, nullptr, &g_RenderTargetViewPre);

		// �V�F�[�_���\�[�X�r���[�̐ݒ�
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// �V�F�[�_���\�[�X�r���[���쐬
		g_D3DDevice->CreateShaderResourceView(g_PreBuffer, &srvDesc, &g_ShaderResourceView);

	}

	// �P�x���o�p�@Luminance�����_�[�^�[�Q�b�g
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = sd.BufferDesc.Width;
		desc.Height = sd.BufferDesc.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.SampleDesc = sd.SampleDesc;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;


		// �����_�[�^�[�Q�b�g���쐬
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_LuminanceBuffer);

		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// �����_�[�^�[�Q�b�g�r���[���쐬
		g_D3DDevice->CreateRenderTargetView(g_LuminanceBuffer, nullptr, &g_RenderTargetViewLuminance);

		// �V�F�[�_���\�[�X�r���[�̐ݒ�
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// �V�F�[�_���\�[�X�r���[���쐬
		g_D3DDevice->CreateShaderResourceView(g_LuminanceBuffer, &srvDesc, &g_ShaderResourceViewLuminace);

	}

	//x�����u���[���H�p�@Blur�����_�[�^�[�Q�b�g
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = sd.BufferDesc.Width;
		desc.Height = sd.BufferDesc.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.SampleDesc = sd.SampleDesc;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;


		// �����_�[�^�[�Q�b�g���쐬
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_BlurBufferX);

		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// �����_�[�^�[�Q�b�g�r���[���쐬
		g_D3DDevice->CreateRenderTargetView(g_BlurBufferX, nullptr, &g_RenderTargetViewBlurX);

		// �V�F�[�_���\�[�X�r���[�̐ݒ�
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// �V�F�[�_���\�[�X�r���[���쐬
		g_D3DDevice->CreateShaderResourceView(g_BlurBufferX, &srvDesc, &g_ShaderResourceViewBlurX);

	}

	//xy�����u���[���H�p�@Blur�����_�[�^�[�Q�b�g
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = sd.BufferDesc.Width;
		desc.Height = sd.BufferDesc.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.SampleDesc = sd.SampleDesc;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;


		// �����_�[�^�[�Q�b�g���쐬
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_BlurBufferXY);

		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// �����_�[�^�[�Q�b�g�r���[���쐬
		g_D3DDevice->CreateRenderTargetView(g_BlurBufferXY, nullptr, &g_RenderTargetViewBlurXY);

		// �V�F�[�_���\�[�X�r���[�̐ݒ�
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// �V�F�[�_���\�[�X�r���[���쐬
		g_D3DDevice->CreateShaderResourceView(g_BlurBufferXY, &srvDesc, &g_ShaderResourceViewBlurXY);

	}

	// �V���h�E�}�b�v�p�@ShadowMap�����_�[�^�[�Q�b�g
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = sd.BufferDesc.Width;
		desc.Height = sd.BufferDesc.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.SampleDesc = sd.SampleDesc;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;


		// �����_�[�^�[�Q�b�g���쐬
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_ShadowMapBuffer);

		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// �����_�[�^�[�Q�b�g�r���[���쐬
		g_D3DDevice->CreateRenderTargetView(g_ShadowMapBuffer, nullptr, &g_RenderTargetViewShadowMap);

		// �V�F�[�_���\�[�X�r���[�̐ݒ�
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// �V�F�[�_���\�[�X�r���[���쐬
		g_D3DDevice->CreateShaderResourceView(g_ShadowMapBuffer, &srvDesc, &g_ShaderResourceViewShadowMap);

	}


	// �m�C�Y�e�N�X�`���̍쐬
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_NoiseTexture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_NoiseTexture[i],
			NULL);
	}


	//�X�e���V���p�e�N�X�`���[�쐬
	ID3D11Texture2D* depthTexture = NULL;
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory( &td, sizeof(td) );
	td.Width			= sd.BufferDesc.Width;
	td.Height			= sd.BufferDesc.Height;
	td.MipLevels		= 1;
	td.ArraySize		= 1;
	td.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;
	td.SampleDesc		= sd.SampleDesc;
	td.Usage			= D3D11_USAGE_DEFAULT;
	td.BindFlags		= D3D11_BIND_DEPTH_STENCIL;
    td.CPUAccessFlags	= 0;
    td.MiscFlags		= 0;
	g_D3DDevice->CreateTexture2D( &td, NULL, &depthTexture );

	//�X�e���V���^�[�Q�b�g�쐬
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory( &dsvd, sizeof(dsvd) );
	dsvd.Format			= td.Format;
	dsvd.ViewDimension	= D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Flags			= 0;
	g_D3DDevice->CreateDepthStencilView( depthTexture, &dsvd, &g_DepthStencilView );


	g_ImmediateContext->OMSetRenderTargets( 1, &g_RenderTargetView, g_DepthStencilView );


	// �r���[�|�[�g�ݒ�
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)SCREEN_WIDTH;
	vp.Height = (FLOAT)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_ImmediateContext->RSSetViewports( 1, &vp );



	// ���X�^���C�U�X�e�[�g�쐬
	D3D11_RASTERIZER_DESC rd; 
	ZeroMemory( &rd, sizeof( rd ) );
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE; 
	rd.DepthClipEnable = TRUE; 
	rd.MultisampleEnable = FALSE; 
	g_D3DDevice->CreateRasterizerState( &rd, &g_RasterStateCullOff);

	rd.CullMode = D3D11_CULL_FRONT;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterStateCullCW);

	rd.CullMode = D3D11_CULL_BACK;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterStateCullCCW);

	// �J�����O���[�h�ݒ�iCCW�j
	SetCullingMode(CULL_MODE_BACK);

	// �u�����h�X�e�[�g�̍쐬
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory( &blendDesc, sizeof( blendDesc ) );
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState( &blendDesc, &g_BlendStateAlphaBlend );

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateNone);

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateAdd);

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateSubtract);

	// �A���t�@�u�����h�ݒ�
	SetBlendState(BLEND_MODE_ALPHABLEND);

	// �[�x�X�e���V���X�e�[�g�쐬
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateEnable );//�[�x�L���X�e�[�g

	//depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateDisable );//�[�x�����X�e�[�g

	// �[�x�X�e���V���X�e�[�g�ݒ�
	SetDepthEnable(TRUE);

	// �T���v���[�X�e�[�g�ݒ�
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory( &samplerDesc, sizeof( samplerDesc ) );
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* samplerState = NULL;
	g_D3DDevice->CreateSamplerState( &samplerDesc, &samplerState );

	g_ImmediateContext->PSSetSamplers( 0, 1, &samplerState );

	// �k���o�b�t�@�p�T���v���[�X�e�[�g
	{
		// �T���v���[�X�e�[�g�ݒ�
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 1;
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;

		ID3D11SamplerState* samplerState = NULL;
		g_D3DDevice->CreateSamplerState(&samplerDesc, &samplerState);

		g_ImmediateContext->PSSetSamplers(1, 1, &samplerState);
	}

	// �V���h�E�}�b�v�p�T���v���[�X�e�[�g
	{
		// �T���v���[�X�e�[�g�ݒ�
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.MipLODBias = 1;
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;

		ID3D11SamplerState* samplerState = NULL;
		g_D3DDevice->CreateSamplerState(&samplerDesc, &samplerState);

		g_ImmediateContext->PSSetSamplers(2, 1, &samplerState);
	}

	// ���_�V�F�[�_�R���p�C���E����
	ID3DBlob* pErrorBlob;
	ID3DBlob* pVSBlob = NULL;
	DWORD shFlag = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG) && defined(DEBUG_SHADER)
	shFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// �f�t�H���g�̒��_�V�F�[�_�[�R���p�C���E����
	hr = D3DX11CompileFromFile( "shader.hlsl", NULL, NULL, "VertexShaderPolygon", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		MessageBox( NULL , (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR );
	}

	g_D3DDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShader );

	// x�����u���[�̒��_�V�F�[�_�[�R���p�C���E����
	pVSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "VertexShaderBlurPass1", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderBlurX);

	// y�����u���[�̒��_�V�F�[�_�[�R���p�C���E����
	pVSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "VertexShaderBlurPass2", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderBlurY);

	// �V���h�E�}�b�v�p�̒��_�V�F�[�_�[�R���p�C���E����
	pVSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "VertexShaderShadowMap", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderShadowMap);

	// �e�`��p�̒��_�V�F�[�_�[�R���p�C���E����
	pVSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "VertexShaderShadowPaste", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderShadowPaste);

	//// �p�[�e�B�N���̒��_�V�F�[�_�[�R���p�C���E����
	//pVSBlob = NULL;
	//hr = D3DX11CompileFromFile("shader2.hlsl", NULL, NULL, "VertexShaderParticle", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	//if (FAILED(hr))
	//{
	//	MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	//}

	//g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderParticle);



	// ���̓��C�A�E�g����
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 4, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 6, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 7, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

	g_D3DDevice->CreateInputLayout( layout,
		numElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_VertexLayout );

	pVSBlob->Release();


	// �s�N�Z���V�F�[�_�R���p�C���E����
	ID3DBlob* pPSBlob = NULL;
	hr = D3DX11CompileFromFile( "shader.hlsl", NULL, NULL, "PixelShaderPolygon", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		MessageBox( NULL , (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR );
	}

	g_D3DDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShader );
	
	pPSBlob->Release();

	// �e�X�g�p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderMap", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBackBuffer);

	pPSBlob->Release();

	// �P�x���o�p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderLuminance", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderLuminance);

	pPSBlob->Release();


	// x�����u���[�p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBlurPass1", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBlurX);

	pPSBlob->Release();

	// y�����u���[�p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBlurPass2", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBlurY);

	pPSBlob->Release();


	// �����p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderComposite", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderComposite);

	pPSBlob->Release();


	// �V���h�E�}�b�v�p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderShadowMap", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderShadowMap);

	pPSBlob->Release();

	// �e�`��p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderShadowPaste", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderShadowPaste);

	pPSBlob->Release();

	// ���@�w�p(�f�B�]���u)
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderMagicCircle", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderMagicCircle);

	pPSBlob->Release();

	// �G�l�~�[�p(�f�B�]���u)
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderEnemy", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderEnemy);

	pPSBlob->Release();

	// ��p(�f�B�]���u)
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderWand", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderWand);

	pPSBlob->Release();

	// �o�n���[�g�p(�f�B�]���u)
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBahamut", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBahamut);

	pPSBlob->Release();

	// �o�n���[�g�̖��@�w�p(�f�B�]���u)
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBahamutCircle", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBahamutCircle);

	pPSBlob->Release();

	// �o�n���[�g�̔g���p(�f�B�]���u)
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBahamutSurge", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBahamutSurge);

	pPSBlob->Release();

	// �p�[�e�B�N���p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderParticle", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderParticle);

	pPSBlob->Release();

	// �����@�p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderWater", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderWater);

	pPSBlob->Release();

	// �����p
	// �s�N�Z���V�F�[�_�R���p�C���E����
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBlast", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBlast);

	pPSBlob->Release();

	

	// �萔�o�b�t�@����
	D3D11_BUFFER_DESC hBufferDesc;
	hBufferDesc.ByteWidth = sizeof(XMMATRIX);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(float);

	//���[���h�}�g���N�X
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_WorldBuffer);
	g_ImmediateContext->VSSetConstantBuffers(0, 1, &g_WorldBuffer);
	g_ImmediateContext->PSSetConstantBuffers(0, 1, &g_WorldBuffer);

	//�r���[�}�g���N�X
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ViewBuffer);
	g_ImmediateContext->VSSetConstantBuffers(1, 1, &g_ViewBuffer);
	g_ImmediateContext->PSSetConstantBuffers(1, 1, &g_ViewBuffer);

	//���C�g�r���[�}�g���N�X
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightViewBuffer);
	g_ImmediateContext->VSSetConstantBuffers(9, 1, &g_LightViewBuffer);
	g_ImmediateContext->PSSetConstantBuffers(9, 1, &g_LightViewBuffer);

	//�v���W�F�N�V�����}�g���N�X
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ProjectionBuffer);
	g_ImmediateContext->VSSetConstantBuffers(2, 1, &g_ProjectionBuffer);
	g_ImmediateContext->PSSetConstantBuffers(2, 1, &g_ProjectionBuffer);

	//���C�g�v���W�F�N�V�����}�g���N�X
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightProjectionBuffer);
	g_ImmediateContext->VSSetConstantBuffers(10, 1, &g_LightProjectionBuffer);
	g_ImmediateContext->PSSetConstantBuffers(10, 1, &g_LightProjectionBuffer);

	//// ���[���h�E�r���[�E�ˉe�ϊ��s��
	//g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_WVPBuffer);
	//g_ImmediateContext->VSSetConstantBuffers(13, 1, &g_WVPBuffer);
	//g_ImmediateContext->PSSetConstantBuffers(13, 1, &g_WVPBuffer);



	//�}�e���A�����
	hBufferDesc.ByteWidth = sizeof(MATERIAL_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_MaterialBuffer);
	g_ImmediateContext->VSSetConstantBuffers(3, 1, &g_MaterialBuffer);
	g_ImmediateContext->PSSetConstantBuffers(3, 1, &g_MaterialBuffer);

	//���C�g���
	hBufferDesc.ByteWidth = sizeof(LIGHT_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightBuffer);
	g_ImmediateContext->VSSetConstantBuffers(4, 1, &g_LightBuffer);
	g_ImmediateContext->PSSetConstantBuffers(4, 1, &g_LightBuffer);

	//�t�H�O���
	hBufferDesc.ByteWidth = sizeof(FOG_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FogBuffer);
	g_ImmediateContext->VSSetConstantBuffers(5, 1, &g_FogBuffer);
	g_ImmediateContext->PSSetConstantBuffers(5, 1, &g_FogBuffer);

	//�����
	hBufferDesc.ByteWidth = sizeof(FUCHI);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FuchiBuffer);
	g_ImmediateContext->VSSetConstantBuffers(6, 1, &g_FuchiBuffer);
	g_ImmediateContext->PSSetConstantBuffers(6, 1, &g_FuchiBuffer);

	//�J����
	hBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_CameraBuffer);
	g_ImmediateContext->VSSetConstantBuffers(7, 1, &g_CameraBuffer);
	g_ImmediateContext->PSSetConstantBuffers(7, 1, &g_CameraBuffer);

	//�u���[
	hBufferDesc.ByteWidth = sizeof(GaussBlurParam);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_BlurParaBuffer);
	g_ImmediateContext->VSSetConstantBuffers(8, 1, &g_BlurParaBuffer);
	g_ImmediateContext->PSSetConstantBuffers(8, 1, &g_BlurParaBuffer);

	//�f�B�]���u
	hBufferDesc.ByteWidth = sizeof(DISSOLVE);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_DissolveBuffer);
	g_ImmediateContext->VSSetConstantBuffers(11, 1, &g_DissolveBuffer);
	g_ImmediateContext->PSSetConstantBuffers(11, 1, &g_DissolveBuffer);

	//�o�n���[�g�̖��@�w
	hBufferDesc.ByteWidth = sizeof(BAHAMUT);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_BahamutBuffer);
	g_ImmediateContext->VSSetConstantBuffers(12, 1, &g_BahamutBuffer);
	g_ImmediateContext->PSSetConstantBuffers(12, 1, &g_BahamutBuffer);



	// ���̓��C�A�E�g�ݒ�
	g_ImmediateContext->IASetInputLayout( g_VertexLayout );

	// �V�F�[�_�ݒ�
	g_ImmediateContext->VSSetShader( g_VertexShader, NULL, 0 );
	g_ImmediateContext->PSSetShader( g_PixelShader, NULL, 0 );

	//���C�g������
	ZeroMemory(&g_Light, sizeof(LIGHT_CBUFFER));
	g_Light.Direction[0] = XMFLOAT4(1.0f, -1.0f, 1.0f, 0.0f);
	g_Light.Diffuse[0] = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	g_Light.Ambient[0] = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	g_Light.Flags[0].Type = LIGHT_TYPE_DIRECTIONAL;
	SetLightBuffer();


	//�}�e���A��������
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	return S_OK;
}


//=============================================================================
// �I������
//=============================================================================
void UninitRenderer(void)
{
	// �I�u�W�F�N�g���
	if (g_DepthStateEnable)			g_DepthStateEnable->Release();
	if (g_DepthStateDisable)		g_DepthStateDisable->Release();
	if (g_BlendStateNone)			g_BlendStateNone->Release();
	if (g_BlendStateAlphaBlend)		g_BlendStateAlphaBlend->Release();
	if (g_BlendStateAdd)			g_BlendStateAdd->Release();
	if (g_BlendStateSubtract)		g_BlendStateSubtract->Release();
	if (g_RasterStateCullOff)		g_RasterStateCullOff->Release();
	if (g_RasterStateCullCW)		g_RasterStateCullCW->Release();
	if (g_RasterStateCullCCW)		g_RasterStateCullCCW->Release();

	if (g_WorldBuffer)				g_WorldBuffer->Release();
	if (g_ViewBuffer)				g_ViewBuffer->Release();
	if (g_ProjectionBuffer)			g_ProjectionBuffer->Release();
//	if (g_LightViewBuffer)			g_LightViewBuffer->Release;
//	if (g_LightProjectionBuffer)	g_LightProjectionBuffer->Release;
	if (g_MaterialBuffer)			g_MaterialBuffer->Release();
	if (g_LightBuffer)				g_LightBuffer->Release();
	if (g_FogBuffer)				g_FogBuffer->Release();

	if (g_ImmediateContext)			g_ImmediateContext->ClearState();
	if (g_RenderTargetView)			g_RenderTargetView->Release();
	if (g_SwapChain)				g_SwapChain->Release();
	if (g_ImmediateContext)			g_ImmediateContext->Release();
	if (g_D3DDevice)				g_D3DDevice->Release();

	if (g_VertexLayout)				g_VertexLayout->Release();
	if (g_VertexShader)				g_VertexShader->Release();
//	if (g_VertexShaderBlurX)		g_VertexShaderBlurX->Release;
//	if (g_VertexShaderBlurY)		g_VertexShaderBlurY->Release;
//	if (g_VertexShaderShadowMap)	g_VertexShaderShadowMap->Release;
//	if (g_VertexShaderShadowPaste)	g_VertexShaderShadowPaste->Release;

	if (g_PixelShader)				g_PixelShader->Release();
//	if (g_PixelShaderBackBuffer)	g_PixelShaderBackBuffer->Release;
//	if (g_PixelShaderLuminance)		g_PixelShaderLuminance->Release;
//	if (g_PixelShaderBlurX)			g_PixelShaderBlurX->Release;
//	if (g_PixelShaderBlurY)			g_PixelShaderBlurY->Release;
//	if (g_PixelShaderComposite)		g_PixelShaderComposite->Release;
//	if (g_PixelShaderLightning)		g_PixelShaderLightning->Release;
//	if (g_PixelShaderShadowMap)		g_PixelShaderShadowMap->Release;
//	if (g_PixelShaderShadowPaste)	g_PixelShaderShadowPaste->Release;
//	if (g_PixelShaderMagicCircle)	g_PixelShaderMagicCircle->Release;
//	if (g_PixelShaderEnemy)			g_PixelShaderEnemy->Release;
//	if (g_PixelShaderWand)			g_PixelShaderWand->Release;

//	if (g_PreBuffer)				g_PreBuffer->Release;
//	if (g_ShadowMapBuffer)			g_ShadowMapBuffer->Release;
//	if (g_LuminanceBuffer)			g_LuminanceBuffer->Release;
//	if (g_BlurBufferX)				g_BlurBufferX->Release;
//	if (g_BlurBufferXY)				g_BlurBufferXY->Release;
//	if (g_FuchiBuffer)				g_FuchiBuffer->Release;
//	if (g_BlurParaBuffer)			g_BlurParaBuffer->Release;
//	if (g_DissolveBuffer)			g_DissolveBuffer->Release;
//	if (g_DissolveEnemyBuffer)		g_DissolveEnemyBuffer->Release;
//	if (g_DissolveWandBuffer)		g_DissolveWandBuffer->Release;
}


//=============================================================================
// �o�b�N�o�b�t�@�N���A
//=============================================================================
void Clear(void)
{
	// �o�b�N�o�b�t�@�N���A
	float ClearColor[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
//	float ClearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	g_ImmediateContext->ClearRenderTargetView(g_RenderTargetView, ClearColor);
	g_ImmediateContext->ClearRenderTargetView(g_RenderTargetViewPre, ClearColor);
	g_ImmediateContext->ClearRenderTargetView(g_RenderTargetViewLuminance, ClearColor);
	g_ImmediateContext->ClearRenderTargetView(g_RenderTargetViewBlurX, ClearColor);
	g_ImmediateContext->ClearRenderTargetView(g_RenderTargetViewBlurXY, ClearColor);
	g_ImmediateContext->ClearDepthStencilView(g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


//=============================================================================
// �v���[���g
//=============================================================================
void Present(void)
{
	g_SwapChain->Present( 0, 0 );
}


//=============================================================================
// �f�o�b�O�p�e�L�X�g�o��
//=============================================================================
void DebugTextOut(char* text, int x, int y)
{
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	HRESULT hr;

	//�o�b�N�o�b�t�@����T�[�t�F�X���擾����
	IDXGISurface1* pBackSurface = NULL;
	hr = g_SwapChain->GetBuffer(0, __uuidof(IDXGISurface1), (void**)&pBackSurface);

	if (SUCCEEDED(hr))
	{
		//�擾�����T�[�t�F�X����f�o�C�X�R���e�L�X�g���擾����
		HDC hdc;
		hr = pBackSurface->GetDC(FALSE, &hdc);

		if (SUCCEEDED(hr))
		{
			//�����F�𔒂ɕύX
			SetTextColor(hdc, RGB(255, 255, 255));
			//�w�i�𓧖��ɕύX
			SetBkMode(hdc, TRANSPARENT);

			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = SCREEN_WIDTH;
			rect.bottom = SCREEN_HEIGHT;

			//�e�L�X�g�o��
			DrawText(hdc, text, (int)strlen(text), &rect, DT_LEFT);

			//�f�o�C�X�R���e�L�X�g���������
			pBackSurface->ReleaseDC(NULL);
		}
		//�T�[�t�F�X���������
		pBackSurface->Release();

		//�����_�����O�^�[�Q�b�g�����Z�b�g�����̂ŃZ�b�g���Ȃ���
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);
	}
#endif
}



//=============================================================================
// �V�F�[�_�\�̐؂�ւ��֐�
//=============================================================================
void SetShaderMode(int mode)
{
	switch (mode)
	{
	case SHADER_MODE_DEFAULT:		// �ʏ�`��p�̃V�F�[�_�\
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShader, NULL, 0);

		break;

	case SHADER_MODE_LUMINACE:		// �P�x���o�p�̃V�F�[�_�\
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderLuminance, NULL, 0);

		break;

	case SHADER_MODE_BLUR_X:		// x�����̃u���[���H�p�̃V�F�[�_�\
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShaderBlurX, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBlurX, NULL, 0);

		break;

	case SHADER_MODE_BLUR_Y:		// xy�����̃u���[���H�p�̃V�F�[�_�\
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShaderBlurY, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBlurY, NULL, 0);

		break;

	case SHADER_MODE_COMPOSITE:		// back�o�b�t�@�֏������ޗp�̃V�F�[�_�\
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderComposite, NULL, 0);

		break;

	case SHADER_MODE_BACKBUFFER:	// back�o�b�t�@�֏������ޗp�̃V�F�[�_�\
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBackBuffer, NULL, 0);

		break;

	case SHADER_MODE_LIGHTNING:	// ���C�g�j���O�p�̃V�F�[�_�\
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderLightning, NULL, 0);

		break;

	case SHADER_MODE_SHADOWMAP:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowMap, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderShadowMap, NULL, 0);

		break;

	case SHADER_MODE_SHADOWPASTE:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowPaste, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderShadowPaste, NULL, 0);

		break;

	case SHADER_MODE_CIRCLE:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderMagicCircle, NULL, 0);

		break;

	case SHADER_MODE_ENEMY:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowPaste, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderEnemy, NULL, 0);

		break;

	case SHADER_MODE_WAND:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowPaste, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderWand, NULL, 0);

		break;

	case SHADER_MODE_BAHAMUT:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowPaste, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBahamut, NULL, 0);

		break;

	case SHADER_MODE_BAHAMUT_CIRCLE:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBahamutCircle, NULL, 0);

		break;

	case SHADER_MODE_BAHAMUT_SURGE:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBahamutSurge, NULL, 0);

		break;

	case SHADER_MODE_PARTICLE:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderParticle, NULL, 0);

		break;

	case SHADER_MODE_WATER:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderWater, NULL, 0);

		break;

	case SHADER_MODE_BLAST:
		// ���̓��C�A�E�g�ݒ�
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// �V�F�[�_�ݒ�
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBlast, NULL, 0);

		break;
	}
}


//=============================================================================
// �����_�[�^�[�Q�b�g�r���[�̐؂�ւ��֐�
//=============================================================================
void SetRenderTargetView(int mode)
{
	switch (mode)
	{
	case VIEW_MODE_BACKBUFFER:	// �`����back�o�b�t�@�֕ύX
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);

		break;

	case VIEW_MODE_PREVIOUS:	// �`�����V�F�[�_�\���\�[�X�Ɏg�p���郌���_�[�^�[�Q�b�g�֕ύX
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewPre, g_DepthStencilView);

		break;

	case VIEW_MODE_LUMINACE:	// �`�����P�x���o�o�b�t�@�֕ύX
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewLuminance, g_DepthStencilView);

		break;

	case VIEW_MODE_BLUR_X:		// �`����x�����u���[���H�o�b�t�@�֕ύX
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewBlurX, g_DepthStencilView);

		break;

	case VIEW_MODE_BLUR_XY:		// �`����xy�����u���[���H�o�b�t�@�֕ύX
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewBlurXY, g_DepthStencilView);

		break;

	case VIEW_MODE_SHADOWMAP:	// �`�����V���h�E�}�b�v�o�b�t�@�֕ύX
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewShadowMap, g_DepthStencilView);

		break;
	}

}


//=============================================================================
// �V�F�[�_�\���\�[�X���u���[�p�ɐ؂�ւ�
//=============================================================================
void SetShaderResouces(int mode)
{
	switch (mode)
	{
	case SHADER_RESOURCE_MODE_PRE:			// PRErt���V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceView);

		break;

	case SHADER_RESOURCE_MODE_LUMINACE:		// �P�x���ort���V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewLuminace);

		break;

	case SHADER_RESOURCE_MODE_BLUR_X:		// x�����u���[���Hrt���V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewBlurX);

		break;

	case SHADER_RESOURCE_MODE_BLUR_XY:		// y�����u���[���Hrt���V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewBlurXY);

		break;

	case SHADER_RESOURCE_MODE_COMPOSITE:	// PRErt�ƃu���[���Hrt���V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(0, 1, &g_ShaderResourceView);
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewBlurXY);

		break;

	case SHADER_RESOURCE_MODE_SHADOWMAP:	// �V���h�E�}�b�vrt���V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewShadowMap);

		break;

	case SHADER_RESOURCE_MODE_NOISE:		// �m�C�Y�e�N�X�`�����V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(2, 1, &g_NoiseTexture[0]);

		break;

	case SHADER_RESOURCE_MODE_NOISE_CENTER:		// �m�C�Y�e�N�X�`�����V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(3, 1, &g_NoiseTexture[1]);
		g_ImmediateContext->PSSetShaderResources(4, 1, &g_NoiseTexture[2]);

		break;

	case SHADER_RESOURCE_MODE_NOISE_WATER:		// �m�C�Y�e�N�X�`�����V�F�[�_�\���\�[�X�Ɏg�p
		g_ImmediateContext->PSSetShaderResources(5, 1, &g_NoiseTexture[3]);

		break;

	case SHADER_RESOURCE_MODE_CLEAR:		// �V�F�[�_�\���\�[�X�p�̃����_�[�^�[�Q�b�g���N���A
		ID3D11ShaderResourceView* clear = NULL;
		g_ImmediateContext->PSSetShaderResources(0, 1, &clear);
		g_ImmediateContext->PSSetShaderResources(1, 1, &clear);
		g_ImmediateContext->PSSetShaderResources(2, 1, &clear);
		g_ImmediateContext->PSSetShaderResources(3, 1, &clear);
		g_ImmediateContext->PSSetShaderResources(4, 1, &clear);
		break;
	}
}


//=============================================================================
// �V�F�[�_�[�̐ݒ����������C�ɕύX����
//=============================================================================
void SetShaderType(int mode)
{
	switch (mode)
	{
		// �ʏ�`��
		// �I�u�W�F�N�g��PRErt�փf�t�H���g�V�F�[�_�\�ŏ�������
	case SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ:
		SetShaderMode(SHADER_MODE_DEFAULT);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);

		g_ShaderMode = SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ;
		break;

		// �P�x���o
		// PRErt���P�x���ort��LUMINACE�V�F�[�_�\�ŏ�������
	case SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS:
		SetShaderMode(SHADER_MODE_LUMINACE);
		SetRenderTargetView(VIEW_MODE_LUMINACE);
		SetShaderResouces(SHADER_RESOURCE_MODE_PRE);

		g_ShaderMode = SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS;
		break;

		// x�����ւ̃u���[���H
		// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	case SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE:
		SetShaderMode(SHADER_MODE_BLUR_X);
		SetRenderTargetView(VIEW_MODE_BLUR_X);
		SetShaderResouces(SHADER_RESOURCE_MODE_LUMINACE);

		g_ShaderMode = SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE;
		break;

		// y�����ւ̃u���[���H
		// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	case SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR:
		SetShaderMode(SHADER_MODE_BLUR_Y);
		SetRenderTargetView(VIEW_MODE_BLUR_XY);
		SetShaderResouces(SHADER_RESOURCE_MODE_BLUR_X);

		g_ShaderMode = SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR;
		break;

		// ����
		// �P�x���ort��PRErt��back�o�b�t�@��Composite�V�F�[�_�\�ŏ�������
	case SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS:
		SetShaderMode(SHADER_MODE_COMPOSITE);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);
		SetShaderResouces(SHADER_RESOURCE_MODE_COMPOSITE);

		g_ShaderMode = SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS;
		break;

		// back�o�b�t�@��
		// PRErt��back�o�b�t�@��BACK�V�F�[�_�\�ŏ�������
	case SHADER_BACK__VIEW_BACK__RESOURCE_PREVIOUS:
		SetShaderMode(SHADER_MODE_BACKBUFFER);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);
		SetShaderResouces(SHADER_RESOURCE_MODE_PRE);

		g_ShaderMode = SHADER_BACK__VIEW_BACK__RESOURCE_PREVIOUS;
		break;

		// �u���[���H�e�X�g
		// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	case SHADER_BLUR_X__VIEW_BACK__RESOURCE_LUMINACE:
		SetShaderMode(SHADER_MODE_BLUR_X);
		SetRenderTargetView(VIEW_MODE_BLUR_X);
		SetShaderResouces(SHADER_RESOURCE_MODE_LUMINACE);

		g_ShaderMode = SHADER_BLUR_X__VIEW_BACK__RESOURCE_LUMINACE;
		break;
		
		// �u���[���H�e�X�g
		// �P�x���ort���P�x���ort��Blur�V�F�[�_�\�ŏ�������
	case SHADER_BLUR_Y__VIEW_BACK__RESOURCE_LUMINACE:
		SetShaderMode(SHADER_MODE_BLUR_Y);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);
		SetShaderResouces(SHADER_RESOURCE_MODE_BLUR_XY);

		g_ShaderMode = SHADER_BLUR_Y__VIEW_BACK__RESOURCE_LUMINACE;
		break;

		// �V���h�E�}�b�v��`��
		// �I�u�W�F�N�g���V���h�E�}�b�vrt��ShadowMap�V�F�[�_�\�ŏ�������
	case SHADER_SHADOWMAP__VIEW_SHADOWMAP__RESOUCE_OBJ:
		SetShaderMode(SHADER_MODE_SHADOWMAP);
		SetRenderTargetView(VIEW_MODE_SHADOWMAP);

		g_ShaderMode = SHADER_SHADOWMAP__VIEW_SHADOWMAP__RESOUCE_OBJ;
		break;

		// �V���h�E�}�b�v��back�o�b�t�@��
		// �V���h�E�}�b�vrt��back�o�b�t�@�փf�t�H���g�V�F�[�_�\�ŏ�������
	case SHADER_DEFAULT__VIEW_BACK__RESOUCE_SHADOWMAP:
		SetShaderMode(SHADER_MODE_DEFAULT);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);
		SetShaderResouces(SHADER_RESOURCE_MODE_SHADOWMAP);

		g_ShaderMode = SHADER_DEFAULT__VIEW_BACK__RESOUCE_SHADOWMAP;
		break;

		// �e�̕t�����I�u�W�F�N�g��PRErt��
		// �V���h�E�}�b�vrt������PRErt�փV���h�E�y�[�X�g�V�F�[�_�\�ŏ�������
	case SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ:
		SetShaderMode(SHADER_MODE_SHADOWPASTE);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);
		SetShaderResouces(SHADER_RESOURCE_MODE_SHADOWMAP);

		g_ShaderMode = SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ;
		break;

		// �f�B�]���u���������ĉe�̂Ȃ��I�u�W�F�N�g��PRErt��
		// �m�C�Y�e�N�X�`��������PRErt�֖��@�w�p�V�F�[�_�\�ŏ�������
	case SHADER_CIRCLR__VIEW_PRE__RESOUCE_OBJ_NOISE:
		SetShaderMode(SHADER_MODE_CIRCLE);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);
		SetShaderResouces(SHADER_RESOURCE_MODE_NOISE);

		g_ShaderMode = SHADER_CIRCLR__VIEW_PRE__RESOUCE_OBJ_NOISE;
		break;

		// �f�B�]���u���������ĉe�̕t�����I�u�W�F�N�g��PRErt��
		// �m�C�Y�e�N�X�`��������PRErt�փG�l�~�[�p�V�F�[�_�\�ŏ�������
	case SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE:
		SetShaderMode(SHADER_MODE_ENEMY);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);
		SetShaderResouces(SHADER_RESOURCE_MODE_NOISE);

		g_ShaderMode = SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE;
		break;

		// �f�B�]���u�̏��������ĉe�̎g���Ȃ��I�u�W�F�N�g��PRErt��
		// �m�C�Y�e�N�X�`�������ɐ����@�ɉ��H������PRErt�֐����@�p�V�F�[�_�\�ŏ�������
	case SHADER_WATER__VIEW_PRE__RESOUCE_OBJ_NOISE:
		SetShaderMode(SHADER_MODE_WATER);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);
		SetShaderResouces(SHADER_RESOURCE_MODE_NOISE_WATER);

		g_ShaderMode = SHADER_WATER__VIEW_PRE__RESOUCE_OBJ_NOISE;
		break;

		// ���ȏ�̍������瓧���ɂ��鏈��
		// �萔�o�b�t�@�����ɔ�����PRErt�֔����p�V�F�[�_�[�ŏ�������
	case SHADER_BLAST__VIEW_PRE__RESOUCE_OBJ:
		SetShaderMode(SHADER_MODE_BLAST);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);

		g_ShaderMode = SHADER_BLAST__VIEW_PRE__RESOUCE_OBJ;
		break;

	case DEFAULT:
		SetShaderMode(SHADER_MODE_DEFAULT);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);

		g_ShaderMode = DEFAULT;
		break;
	}
}


//=============================================================================
// �K�E�X�̏d�݌v�Z
//=============================================================================
void GaussianDistribution(float dispersion)
{
	float total = 0.0f;
	const int num_weight = 8;

	for (int i = 0; i < num_weight; ++i)
	{
		float pos = 1.0f + 3.0f * (float)i;
		g_weights[i] = expf(GetGausee() * (pos * pos) / dispersion);
		total += 2.0f * g_weights[i];
	}

	float invTotal = 1.0f / total;
	for (int i = 0; i < num_weight; ++i)
	{
		g_weights[i] *= invTotal;
	}
}



//=============================================================================
// �u���[�̌v�Z���s���X�V�������s��
//=============================================================================
void RunBlur(void)
{
	// �K�E�X�t�B���^�p�p�����[�^��ݒ�
	GaussBlurParam buffer;
	memcpy(buffer.weights, g_weights, sizeof(float) * 8);
	buffer.offset.x = 16.0f / SCREEN_WIDTH;
	buffer.offset.y = 16.0f / SCREEN_HEIGHT;
	buffer.size.x = (float)SCREEN_WIDTH;
	buffer.size.y = (float)SCREEN_HEIGHT;

	g_ImmediateContext->UpdateSubresource(g_BlurParaBuffer, 0, NULL, &buffer, 0, 0);
}


//=============================================================================
// ���݂̕`�惂�[�h���擾
//=============================================================================
int GetShaderMode(void)
{
	return g_ShaderMode;
}