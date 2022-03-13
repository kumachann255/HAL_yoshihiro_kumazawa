//=============================================================================
//
// レンダリング処理 [renderer.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "screen.h"
#include "model.h"
#include "enemy.h"
#include "light.h"
#include "beam_orbit.h"

#define TEXTURE_MAX				(4)			// テクスチャの数

//デバッグ用画面テキスト出力を有効にする
#define DEBUG_DISP_TEXTOUT
//シェーダーデバッグ設定を有効にする
//#define DEBUG_SHADER


//*********************************************************
// 構造体
//*********************************************************

// マテリアル用定数バッファ構造体
struct MATERIAL_CBUFFER
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	int			noTexSampling;
	float		Dummy[2];				// 16byte境界用
};

// ライト用フラグ構造体
struct LIGHTFLAGS
{
	int			Type;		//ライトタイプ（enum LIGHT_TYPE）
	int         OnOff;		//ライトのオンorオフスイッチ
	int			Dummy[2];
};

// ライト用定数バッファ構造体
struct LIGHT_CBUFFER
{
	XMFLOAT4	Direction[LIGHT_MAX];	// ライトの方向
	XMFLOAT4	Position[LIGHT_MAX];	// ライトの位置
	XMFLOAT4	Diffuse[LIGHT_MAX];		// 拡散光の色
	XMFLOAT4	Ambient[LIGHT_MAX];		// 環境光の色
	XMFLOAT4	Attenuation[LIGHT_MAX];	// 減衰率
	LIGHTFLAGS	Flags[LIGHT_MAX];		// ライト種別
	int			Enable;					// ライティング有効・無効フラグ
	int			Dummy[3];				// 16byte境界用
};

// フォグ用定数バッファ構造体
struct FOG_CBUFFER
{
	XMFLOAT4	Fog;					// フォグ量
	XMFLOAT4	FogColor;				// フォグの色
	int			Enable;					// フォグ有効・無効フラグ
	float		Dummy[3];				// 16byte境界用
};

// 縁取り用バッファ
struct FUCHI
{
	int			fuchi;
	int			fill[3];
};

// ディゾルブ用バッファ
struct DISSOLVE
{
	float		threshold;				// 閾値
	int			Dummy[3];				// 16byte境界用
};

// バハムートの魔法陣用バッファ
struct BAHAMUT
{
	XMFLOAT3	pos;					// 座標
	bool		open;					// 翼を広げたかどうか
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
static void SetLightBuffer(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static D3D_FEATURE_LEVEL       g_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

static ID3D11Device*			g_D3DDevice = NULL;
static ID3D11DeviceContext*		g_ImmediateContext = NULL;
static IDXGISwapChain*			g_SwapChain = NULL;
static ID3D11RenderTargetView*	g_RenderTargetView = NULL;
static ID3D11RenderTargetView*	g_RenderTargetViewPre = NULL;				// backrtに書き込む前にすべてを書き込むレンダーターゲットのビュー
static ID3D11RenderTargetView*	g_RenderTargetViewLuminance = NULL;			// 輝度抽出rtへ書き込むレンダーターゲットのビュー
static ID3D11RenderTargetView*	g_RenderTargetViewBlurX = NULL;				// X方向へのブラー加工rtへ書き込むレンダーターゲットのビュー
static ID3D11RenderTargetView*	g_RenderTargetViewBlurXY = NULL;			// XY方向へのブラー加工rtへ書き込むレンダーターゲットのビュー
static ID3D11RenderTargetView*	g_RenderTargetViewShadowMap = NULL;			// シャドウマップrtへ書き込むレンダーターゲットのビュー
static ID3D11DepthStencilView*	g_DepthStencilView = NULL;
static ID3D11ShaderResourceView*	g_ShaderResourceView = NULL;			// g_PreBufferをシェーダ―リソースとして使用するためのビュー
static ID3D11ShaderResourceView*	g_ShaderResourceViewLuminace = NULL;	// g_LuminanceBufferをシェーダ―リソースとして使用するためのビュー
static ID3D11ShaderResourceView*	g_ShaderResourceViewBlurX = NULL;		// g_BlurBufferをシェーダ―リソースとして使用するためのビュー
static ID3D11ShaderResourceView*	g_ShaderResourceViewBlurXY = NULL;		// g_BlurBufferをシェーダ―リソースとして使用するためのビュー
static ID3D11ShaderResourceView*	g_ShaderResourceViewShadowMap = NULL;	// g_ShadowMapBufferをシェーダ―リソースとして使用するためのビュー
static ID3D11ShaderResourceView*	g_NoiseTexture[TEXTURE_MAX] = { NULL };	// ノイズテクスチャ情報

static ID3D11Texture2D*			g_PreBuffer;								// backバッファに書き込む前にすべてを書き込むレンダーターゲット
static ID3D11Texture2D*			g_LuminanceBuffer;							// 輝度抽出したデータを書き込むレンダーターゲット
static ID3D11Texture2D*			g_BlurBufferX;								// X方向へのブラー加工を書き込むレンダーターゲット
static ID3D11Texture2D*			g_BlurBufferXY;								// XY方向へのブラー加工を書き込むレンダーターゲット
static ID3D11Texture2D*			g_ShadowMapBuffer;							// シャドウマップを書き込むレンダーターゲット


static ID3D11VertexShader*		g_VertexShader = NULL;						// 通常描画用の頂点シェーダー
static ID3D11VertexShader*		g_VertexShaderBlurX = NULL;					// x方向ブラー用の頂点シェーダー
static ID3D11VertexShader*		g_VertexShaderBlurY = NULL;					// y方向ブラー用の頂点シェーダー
static ID3D11VertexShader*		g_VertexShaderShadowMap = NULL;				// シャドウマップ用の頂点シェーダー
static ID3D11VertexShader*		g_VertexShaderShadowPaste = NULL;			// 影描画用の頂点シェーダー
//static ID3D11VertexShader*		g_VertexShaderParticle = NULL;				// パーティクル用の頂点シェーダー
static ID3D11PixelShader*		g_PixelShader = NULL;						// 通常描画用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderBackBuffer = NULL;				// backバッファにそのまま書き込むピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderLuminance = NULL;				// Luminanceレンダーターゲットへ輝度抽出した色を書き込むピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderBlurX = NULL;					// Luminanceレンダーターゲットにx方向のブラー加工を書き込むピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderBlurY = NULL;					// Luminanceレンダーターゲットにy方向のブラー加工を書き込むピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderComposite = NULL;				// 合成用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderLightning = NULL;				// ライトニング用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderShadowMap = NULL;				// シャドウマップ用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderShadowPaste = NULL;			// 影描画用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderMagicCircle = NULL;			// 魔法陣用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderEnemy = NULL;					// エネミー用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderWand = NULL;					// 杖用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderBahamut = NULL;				// バハムート用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderBahamutCircle = NULL;			// バハムートの魔法陣用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderBahamutSurge = NULL;			// バハムートの波動用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderParticle = NULL;				// パーティクル用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderWater = NULL;					// 水魔法用のピクセルシェーダー
static ID3D11PixelShader*		g_PixelShaderBlast = NULL;					// 爆風用のピクセルシェーダー
static ID3D11InputLayout*		g_VertexLayout = NULL;
static ID3D11Buffer*			g_WorldBuffer = NULL;
static ID3D11Buffer*			g_ViewBuffer = NULL;						// カメラからの視点
static ID3D11Buffer*			g_LightViewBuffer = NULL;					// ライトからの視点
static ID3D11Buffer*			g_ProjectionBuffer = NULL;					// カメラからの射影
static ID3D11Buffer*			g_LightProjectionBuffer = NULL;				// ライトからの射影
//static ID3D11Buffer*			g_WVPBuffer = NULL;							// インスタンシング用行列
static ID3D11Buffer*			g_MaterialBuffer = NULL;
static ID3D11Buffer*			g_LightBuffer = NULL;
static ID3D11Buffer*			g_FogBuffer = NULL;
static ID3D11Buffer*			g_FuchiBuffer = NULL;
static ID3D11Buffer*			g_CameraBuffer = NULL;
static ID3D11Buffer*			g_BlurParaBuffer = NULL;
static ID3D11Buffer*			g_DissolveBuffer = NULL;
static ID3D11Buffer*			g_DissolveEnemyBuffer = NULL;
static ID3D11Buffer*			g_DissolveWandBuffer = NULL;
static ID3D11Buffer*			g_BahamutBuffer = NULL;						// バハムートの魔法陣の位置

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

static DISSOLVE			g_Dissolve;						// ディゾルブ用
static BAHAMUT			g_Circle;						// バハムートの魔法陣用
static float			g_weights[8];

static XMMATRIX			WVP[MAX_BEAM_ORBIT];

static int				g_ShaderMode = 0;				// 現在の描画モード


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

// ライトから見たビューの定数バッファを更新
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

// ライトから見た射影変換用の定数バッファを更新
void SetLightProjectionMatrix(XMMATRIX *ProjectionMatrix)
{
	XMMATRIX projection;
	projection = *ProjectionMatrix;
	projection = XMMatrixTranspose(projection);

	GetDeviceContext()->UpdateSubresource(g_LightProjectionBuffer, 0, NULL, &projection, 0, 0);
}
//
//// インスタンシング用の定数バッファを更新
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
	// フラグを更新する
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
	// フラグを更新する
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

// フォグの一括変更用
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

// ディゾルブバッファの更新
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

// バハムートの魔法陣
void SetShaderBahamutCircle(XMFLOAT3 pos, bool open)
{
	g_Circle.pos = pos;
	g_Circle.open = open;

	GetDeviceContext()->UpdateSubresource(g_BahamutBuffer, 0, NULL, &g_Circle, 0, 0);
}



//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitRenderer(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	HRESULT hr = S_OK;

	// デバイス、スワップチェーン、コンテキスト生成
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

	//デバッグ文字出力用設定
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

	//デバッグ文字出力用設定
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	hr = g_SwapChain->ResizeBuffers(0, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE); // N.B. the GDI compatible flag
	if (FAILED(hr))
		return hr;
#endif

	// ガウスブラーの初期化
	const float dispersion_sq = 5.0f;
	GaussianDistribution(dispersion_sq * dispersion_sq);


	// レンダーターゲットビュー生成、設定
	ID3D11Texture2D* pBackBuffer = NULL;
	g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	g_D3DDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_RenderTargetView );
	pBackBuffer->Release();


	// 仮描画用　PREレンダーターゲット
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


		// レンダーターゲットを作成
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_PreBuffer);

		// レンダーターゲットビューの設定
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// レンダーターゲットビューを作成
		g_D3DDevice->CreateRenderTargetView(g_PreBuffer, nullptr, &g_RenderTargetViewPre);

		// シェーダリソースビューの設定
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// シェーダリソースビューを作成
		g_D3DDevice->CreateShaderResourceView(g_PreBuffer, &srvDesc, &g_ShaderResourceView);

	}

	// 輝度抽出用　Luminanceレンダーターゲット
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


		// レンダーターゲットを作成
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_LuminanceBuffer);

		// レンダーターゲットビューの設定
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// レンダーターゲットビューを作成
		g_D3DDevice->CreateRenderTargetView(g_LuminanceBuffer, nullptr, &g_RenderTargetViewLuminance);

		// シェーダリソースビューの設定
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// シェーダリソースビューを作成
		g_D3DDevice->CreateShaderResourceView(g_LuminanceBuffer, &srvDesc, &g_ShaderResourceViewLuminace);

	}

	//x方向ブラー加工用　Blurレンダーターゲット
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


		// レンダーターゲットを作成
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_BlurBufferX);

		// レンダーターゲットビューの設定
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// レンダーターゲットビューを作成
		g_D3DDevice->CreateRenderTargetView(g_BlurBufferX, nullptr, &g_RenderTargetViewBlurX);

		// シェーダリソースビューの設定
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// シェーダリソースビューを作成
		g_D3DDevice->CreateShaderResourceView(g_BlurBufferX, &srvDesc, &g_ShaderResourceViewBlurX);

	}

	//xy方向ブラー加工用　Blurレンダーターゲット
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


		// レンダーターゲットを作成
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_BlurBufferXY);

		// レンダーターゲットビューの設定
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// レンダーターゲットビューを作成
		g_D3DDevice->CreateRenderTargetView(g_BlurBufferXY, nullptr, &g_RenderTargetViewBlurXY);

		// シェーダリソースビューの設定
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// シェーダリソースビューを作成
		g_D3DDevice->CreateShaderResourceView(g_BlurBufferXY, &srvDesc, &g_ShaderResourceViewBlurXY);

	}

	// シャドウマップ用　ShadowMapレンダーターゲット
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


		// レンダーターゲットを作成
		g_D3DDevice->CreateTexture2D(&desc, nullptr, &g_ShadowMapBuffer);

		// レンダーターゲットビューの設定
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(rtvDesc));
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// レンダーターゲットビューを作成
		g_D3DDevice->CreateRenderTargetView(g_ShadowMapBuffer, nullptr, &g_RenderTargetViewShadowMap);

		// シェーダリソースビューの設定
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// シェーダリソースビューを作成
		g_D3DDevice->CreateShaderResourceView(g_ShadowMapBuffer, &srvDesc, &g_ShaderResourceViewShadowMap);

	}


	// ノイズテクスチャの作成
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


	//ステンシル用テクスチャー作成
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

	//ステンシルターゲット作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory( &dsvd, sizeof(dsvd) );
	dsvd.Format			= td.Format;
	dsvd.ViewDimension	= D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Flags			= 0;
	g_D3DDevice->CreateDepthStencilView( depthTexture, &dsvd, &g_DepthStencilView );


	g_ImmediateContext->OMSetRenderTargets( 1, &g_RenderTargetView, g_DepthStencilView );


	// ビューポート設定
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)SCREEN_WIDTH;
	vp.Height = (FLOAT)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_ImmediateContext->RSSetViewports( 1, &vp );



	// ラスタライザステート作成
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

	// カリングモード設定（CCW）
	SetCullingMode(CULL_MODE_BACK);

	// ブレンドステートの作成
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

	// アルファブレンド設定
	SetBlendState(BLEND_MODE_ALPHABLEND);

	// 深度ステンシルステート作成
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateEnable );//深度有効ステート

	//depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateDisable );//深度無効ステート

	// 深度ステンシルステート設定
	SetDepthEnable(TRUE);

	// サンプラーステート設定
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

	// 縮小バッファ用サンプラーステート
	{
		// サンプラーステート設定
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

	// シャドウマップ用サンプラーステート
	{
		// サンプラーステート設定
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

	// 頂点シェーダコンパイル・生成
	ID3DBlob* pErrorBlob;
	ID3DBlob* pVSBlob = NULL;
	DWORD shFlag = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG) && defined(DEBUG_SHADER)
	shFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// デフォルトの頂点シェーダーコンパイル・生成
	hr = D3DX11CompileFromFile( "shader.hlsl", NULL, NULL, "VertexShaderPolygon", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		MessageBox( NULL , (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR );
	}

	g_D3DDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShader );

	// x方向ブラーの頂点シェーダーコンパイル・生成
	pVSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "VertexShaderBlurPass1", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderBlurX);

	// y方向ブラーの頂点シェーダーコンパイル・生成
	pVSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "VertexShaderBlurPass2", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderBlurY);

	// シャドウマップ用の頂点シェーダーコンパイル・生成
	pVSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "VertexShaderShadowMap", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderShadowMap);

	// 影描画用の頂点シェーダーコンパイル・生成
	pVSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "VertexShaderShadowPaste", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderShadowPaste);

	//// パーティクルの頂点シェーダーコンパイル・生成
	//pVSBlob = NULL;
	//hr = D3DX11CompileFromFile("shader2.hlsl", NULL, NULL, "VertexShaderParticle", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	//if (FAILED(hr))
	//{
	//	MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	//}

	//g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderParticle);



	// 入力レイアウト生成
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


	// ピクセルシェーダコンパイル・生成
	ID3DBlob* pPSBlob = NULL;
	hr = D3DX11CompileFromFile( "shader.hlsl", NULL, NULL, "PixelShaderPolygon", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		MessageBox( NULL , (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR );
	}

	g_D3DDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShader );
	
	pPSBlob->Release();

	// テスト用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderMap", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBackBuffer);

	pPSBlob->Release();

	// 輝度抽出用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderLuminance", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderLuminance);

	pPSBlob->Release();


	// x方向ブラー用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBlurPass1", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBlurX);

	pPSBlob->Release();

	// y方向ブラー用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBlurPass2", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBlurY);

	pPSBlob->Release();


	// 合成用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderComposite", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderComposite);

	pPSBlob->Release();


	// シャドウマップ用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderShadowMap", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderShadowMap);

	pPSBlob->Release();

	// 影描画用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderShadowPaste", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderShadowPaste);

	pPSBlob->Release();

	// 魔法陣用(ディゾルブ)
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderMagicCircle", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderMagicCircle);

	pPSBlob->Release();

	// エネミー用(ディゾルブ)
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderEnemy", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderEnemy);

	pPSBlob->Release();

	// 杖用(ディゾルブ)
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderWand", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderWand);

	pPSBlob->Release();

	// バハムート用(ディゾルブ)
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBahamut", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBahamut);

	pPSBlob->Release();

	// バハムートの魔法陣用(ディゾルブ)
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBahamutCircle", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBahamutCircle);

	pPSBlob->Release();

	// バハムートの波動用(ディゾルブ)
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBahamutSurge", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBahamutSurge);

	pPSBlob->Release();

	// パーティクル用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderParticle", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderParticle);

	pPSBlob->Release();

	// 水魔法用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderWater", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderWater);

	pPSBlob->Release();

	// 爆風用
	// ピクセルシェーダコンパイル・生成
	pPSBlob = NULL;
	hr = D3DX11CompileFromFile("shader.hlsl", NULL, NULL, "PixelShaderBlast", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderBlast);

	pPSBlob->Release();

	

	// 定数バッファ生成
	D3D11_BUFFER_DESC hBufferDesc;
	hBufferDesc.ByteWidth = sizeof(XMMATRIX);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(float);

	//ワールドマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_WorldBuffer);
	g_ImmediateContext->VSSetConstantBuffers(0, 1, &g_WorldBuffer);
	g_ImmediateContext->PSSetConstantBuffers(0, 1, &g_WorldBuffer);

	//ビューマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ViewBuffer);
	g_ImmediateContext->VSSetConstantBuffers(1, 1, &g_ViewBuffer);
	g_ImmediateContext->PSSetConstantBuffers(1, 1, &g_ViewBuffer);

	//ライトビューマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightViewBuffer);
	g_ImmediateContext->VSSetConstantBuffers(9, 1, &g_LightViewBuffer);
	g_ImmediateContext->PSSetConstantBuffers(9, 1, &g_LightViewBuffer);

	//プロジェクションマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ProjectionBuffer);
	g_ImmediateContext->VSSetConstantBuffers(2, 1, &g_ProjectionBuffer);
	g_ImmediateContext->PSSetConstantBuffers(2, 1, &g_ProjectionBuffer);

	//ライトプロジェクションマトリクス
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightProjectionBuffer);
	g_ImmediateContext->VSSetConstantBuffers(10, 1, &g_LightProjectionBuffer);
	g_ImmediateContext->PSSetConstantBuffers(10, 1, &g_LightProjectionBuffer);

	//// ワールド・ビュー・射影変換行列
	//g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_WVPBuffer);
	//g_ImmediateContext->VSSetConstantBuffers(13, 1, &g_WVPBuffer);
	//g_ImmediateContext->PSSetConstantBuffers(13, 1, &g_WVPBuffer);



	//マテリアル情報
	hBufferDesc.ByteWidth = sizeof(MATERIAL_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_MaterialBuffer);
	g_ImmediateContext->VSSetConstantBuffers(3, 1, &g_MaterialBuffer);
	g_ImmediateContext->PSSetConstantBuffers(3, 1, &g_MaterialBuffer);

	//ライト情報
	hBufferDesc.ByteWidth = sizeof(LIGHT_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightBuffer);
	g_ImmediateContext->VSSetConstantBuffers(4, 1, &g_LightBuffer);
	g_ImmediateContext->PSSetConstantBuffers(4, 1, &g_LightBuffer);

	//フォグ情報
	hBufferDesc.ByteWidth = sizeof(FOG_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FogBuffer);
	g_ImmediateContext->VSSetConstantBuffers(5, 1, &g_FogBuffer);
	g_ImmediateContext->PSSetConstantBuffers(5, 1, &g_FogBuffer);

	//縁取り
	hBufferDesc.ByteWidth = sizeof(FUCHI);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FuchiBuffer);
	g_ImmediateContext->VSSetConstantBuffers(6, 1, &g_FuchiBuffer);
	g_ImmediateContext->PSSetConstantBuffers(6, 1, &g_FuchiBuffer);

	//カメラ
	hBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_CameraBuffer);
	g_ImmediateContext->VSSetConstantBuffers(7, 1, &g_CameraBuffer);
	g_ImmediateContext->PSSetConstantBuffers(7, 1, &g_CameraBuffer);

	//ブラー
	hBufferDesc.ByteWidth = sizeof(GaussBlurParam);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_BlurParaBuffer);
	g_ImmediateContext->VSSetConstantBuffers(8, 1, &g_BlurParaBuffer);
	g_ImmediateContext->PSSetConstantBuffers(8, 1, &g_BlurParaBuffer);

	//ディゾルブ
	hBufferDesc.ByteWidth = sizeof(DISSOLVE);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_DissolveBuffer);
	g_ImmediateContext->VSSetConstantBuffers(11, 1, &g_DissolveBuffer);
	g_ImmediateContext->PSSetConstantBuffers(11, 1, &g_DissolveBuffer);

	//バハムートの魔法陣
	hBufferDesc.ByteWidth = sizeof(BAHAMUT);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_BahamutBuffer);
	g_ImmediateContext->VSSetConstantBuffers(12, 1, &g_BahamutBuffer);
	g_ImmediateContext->PSSetConstantBuffers(12, 1, &g_BahamutBuffer);



	// 入力レイアウト設定
	g_ImmediateContext->IASetInputLayout( g_VertexLayout );

	// シェーダ設定
	g_ImmediateContext->VSSetShader( g_VertexShader, NULL, 0 );
	g_ImmediateContext->PSSetShader( g_PixelShader, NULL, 0 );

	//ライト初期化
	ZeroMemory(&g_Light, sizeof(LIGHT_CBUFFER));
	g_Light.Direction[0] = XMFLOAT4(1.0f, -1.0f, 1.0f, 0.0f);
	g_Light.Diffuse[0] = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	g_Light.Ambient[0] = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	g_Light.Flags[0].Type = LIGHT_TYPE_DIRECTIONAL;
	SetLightBuffer();


	//マテリアル初期化
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	return S_OK;
}


//=============================================================================
// 終了処理
//=============================================================================
void UninitRenderer(void)
{
	// オブジェクト解放
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
// バックバッファクリア
//=============================================================================
void Clear(void)
{
	// バックバッファクリア
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
// プレゼント
//=============================================================================
void Present(void)
{
	g_SwapChain->Present( 0, 0 );
}


//=============================================================================
// デバッグ用テキスト出力
//=============================================================================
void DebugTextOut(char* text, int x, int y)
{
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	HRESULT hr;

	//バックバッファからサーフェスを取得する
	IDXGISurface1* pBackSurface = NULL;
	hr = g_SwapChain->GetBuffer(0, __uuidof(IDXGISurface1), (void**)&pBackSurface);

	if (SUCCEEDED(hr))
	{
		//取得したサーフェスからデバイスコンテキストを取得する
		HDC hdc;
		hr = pBackSurface->GetDC(FALSE, &hdc);

		if (SUCCEEDED(hr))
		{
			//文字色を白に変更
			SetTextColor(hdc, RGB(255, 255, 255));
			//背景を透明に変更
			SetBkMode(hdc, TRANSPARENT);

			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = SCREEN_WIDTH;
			rect.bottom = SCREEN_HEIGHT;

			//テキスト出力
			DrawText(hdc, text, (int)strlen(text), &rect, DT_LEFT);

			//デバイスコンテキストを解放する
			pBackSurface->ReleaseDC(NULL);
		}
		//サーフェスを解放する
		pBackSurface->Release();

		//レンダリングターゲットがリセットされるのでセットしなおす
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);
	}
#endif
}



//=============================================================================
// シェーダ―の切り替え関数
//=============================================================================
void SetShaderMode(int mode)
{
	switch (mode)
	{
	case SHADER_MODE_DEFAULT:		// 通常描画用のシェーダ―
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShader, NULL, 0);

		break;

	case SHADER_MODE_LUMINACE:		// 輝度抽出用のシェーダ―
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderLuminance, NULL, 0);

		break;

	case SHADER_MODE_BLUR_X:		// x方向のブラー加工用のシェーダ―
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShaderBlurX, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBlurX, NULL, 0);

		break;

	case SHADER_MODE_BLUR_Y:		// xy方向のブラー加工用のシェーダ―
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShaderBlurY, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBlurY, NULL, 0);

		break;

	case SHADER_MODE_COMPOSITE:		// backバッファへ書き込む用のシェーダ―
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderComposite, NULL, 0);

		break;

	case SHADER_MODE_BACKBUFFER:	// backバッファへ書き込む用のシェーダ―
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBackBuffer, NULL, 0);

		break;

	case SHADER_MODE_LIGHTNING:	// ライトニング用のシェーダ―
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderLightning, NULL, 0);

		break;

	case SHADER_MODE_SHADOWMAP:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowMap, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderShadowMap, NULL, 0);

		break;

	case SHADER_MODE_SHADOWPASTE:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowPaste, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderShadowPaste, NULL, 0);

		break;

	case SHADER_MODE_CIRCLE:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderMagicCircle, NULL, 0);

		break;

	case SHADER_MODE_ENEMY:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowPaste, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderEnemy, NULL, 0);

		break;

	case SHADER_MODE_WAND:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowPaste, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderWand, NULL, 0);

		break;

	case SHADER_MODE_BAHAMUT:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShaderShadowPaste, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBahamut, NULL, 0);

		break;

	case SHADER_MODE_BAHAMUT_CIRCLE:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBahamutCircle, NULL, 0);

		break;

	case SHADER_MODE_BAHAMUT_SURGE:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBahamutSurge, NULL, 0);

		break;

	case SHADER_MODE_PARTICLE:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderParticle, NULL, 0);

		break;

	case SHADER_MODE_WATER:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderWater, NULL, 0);

		break;

	case SHADER_MODE_BLAST:
		// 入力レイアウト設定
		g_ImmediateContext->IASetInputLayout(g_VertexLayout);

		// シェーダ設定
		g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
		g_ImmediateContext->PSSetShader(g_PixelShaderBlast, NULL, 0);

		break;
	}
}


//=============================================================================
// レンダーターゲットビューの切り替え関数
//=============================================================================
void SetRenderTargetView(int mode)
{
	switch (mode)
	{
	case VIEW_MODE_BACKBUFFER:	// 描画先をbackバッファへ変更
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);

		break;

	case VIEW_MODE_PREVIOUS:	// 描画先をシェーダ―リソースに使用するレンダーターゲットへ変更
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewPre, g_DepthStencilView);

		break;

	case VIEW_MODE_LUMINACE:	// 描画先を輝度抽出バッファへ変更
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewLuminance, g_DepthStencilView);

		break;

	case VIEW_MODE_BLUR_X:		// 描画先をx方向ブラー加工バッファへ変更
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewBlurX, g_DepthStencilView);

		break;

	case VIEW_MODE_BLUR_XY:		// 描画先をxy方向ブラー加工バッファへ変更
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewBlurXY, g_DepthStencilView);

		break;

	case VIEW_MODE_SHADOWMAP:	// 描画先をシャドウマップバッファへ変更
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetViewShadowMap, g_DepthStencilView);

		break;
	}

}


//=============================================================================
// シェーダ―リソースをブラー用に切り替え
//=============================================================================
void SetShaderResouces(int mode)
{
	switch (mode)
	{
	case SHADER_RESOURCE_MODE_PRE:			// PRErtをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceView);

		break;

	case SHADER_RESOURCE_MODE_LUMINACE:		// 輝度抽出rtをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewLuminace);

		break;

	case SHADER_RESOURCE_MODE_BLUR_X:		// x方向ブラー加工rtをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewBlurX);

		break;

	case SHADER_RESOURCE_MODE_BLUR_XY:		// y方向ブラー加工rtをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewBlurXY);

		break;

	case SHADER_RESOURCE_MODE_COMPOSITE:	// PRErtとブラー加工rtをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(0, 1, &g_ShaderResourceView);
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewBlurXY);

		break;

	case SHADER_RESOURCE_MODE_SHADOWMAP:	// シャドウマップrtをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(1, 1, &g_ShaderResourceViewShadowMap);

		break;

	case SHADER_RESOURCE_MODE_NOISE:		// ノイズテクスチャをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(2, 1, &g_NoiseTexture[0]);

		break;

	case SHADER_RESOURCE_MODE_NOISE_CENTER:		// ノイズテクスチャをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(3, 1, &g_NoiseTexture[1]);
		g_ImmediateContext->PSSetShaderResources(4, 1, &g_NoiseTexture[2]);

		break;

	case SHADER_RESOURCE_MODE_NOISE_WATER:		// ノイズテクスチャをシェーダ―リソースに使用
		g_ImmediateContext->PSSetShaderResources(5, 1, &g_NoiseTexture[3]);

		break;

	case SHADER_RESOURCE_MODE_CLEAR:		// シェーダ―リソース用のレンダーターゲットをクリア
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
// シェーダーの設定もろもろを一気に変更する
//=============================================================================
void SetShaderType(int mode)
{
	switch (mode)
	{
		// 通常描画
		// オブジェクトをPRErtへデフォルトシェーダ―で書き込む
	case SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ:
		SetShaderMode(SHADER_MODE_DEFAULT);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);

		g_ShaderMode = SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ;
		break;

		// 輝度抽出
		// PRErtを輝度抽出rtへLUMINACEシェーダ―で書き込む
	case SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS:
		SetShaderMode(SHADER_MODE_LUMINACE);
		SetRenderTargetView(VIEW_MODE_LUMINACE);
		SetShaderResouces(SHADER_RESOURCE_MODE_PRE);

		g_ShaderMode = SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS;
		break;

		// x方向へのブラー加工
		// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	case SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE:
		SetShaderMode(SHADER_MODE_BLUR_X);
		SetRenderTargetView(VIEW_MODE_BLUR_X);
		SetShaderResouces(SHADER_RESOURCE_MODE_LUMINACE);

		g_ShaderMode = SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE;
		break;

		// y方向へのブラー加工
		// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	case SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR:
		SetShaderMode(SHADER_MODE_BLUR_Y);
		SetRenderTargetView(VIEW_MODE_BLUR_XY);
		SetShaderResouces(SHADER_RESOURCE_MODE_BLUR_X);

		g_ShaderMode = SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR;
		break;

		// 合成
		// 輝度抽出rtとPRErtをbackバッファへCompositeシェーダ―で書き込む
	case SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS:
		SetShaderMode(SHADER_MODE_COMPOSITE);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);
		SetShaderResouces(SHADER_RESOURCE_MODE_COMPOSITE);

		g_ShaderMode = SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS;
		break;

		// backバッファへ
		// PRErtをbackバッファへBACKシェーダ―で書き込む
	case SHADER_BACK__VIEW_BACK__RESOURCE_PREVIOUS:
		SetShaderMode(SHADER_MODE_BACKBUFFER);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);
		SetShaderResouces(SHADER_RESOURCE_MODE_PRE);

		g_ShaderMode = SHADER_BACK__VIEW_BACK__RESOURCE_PREVIOUS;
		break;

		// ブラー加工テスト
		// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	case SHADER_BLUR_X__VIEW_BACK__RESOURCE_LUMINACE:
		SetShaderMode(SHADER_MODE_BLUR_X);
		SetRenderTargetView(VIEW_MODE_BLUR_X);
		SetShaderResouces(SHADER_RESOURCE_MODE_LUMINACE);

		g_ShaderMode = SHADER_BLUR_X__VIEW_BACK__RESOURCE_LUMINACE;
		break;
		
		// ブラー加工テスト
		// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	case SHADER_BLUR_Y__VIEW_BACK__RESOURCE_LUMINACE:
		SetShaderMode(SHADER_MODE_BLUR_Y);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);
		SetShaderResouces(SHADER_RESOURCE_MODE_BLUR_XY);

		g_ShaderMode = SHADER_BLUR_Y__VIEW_BACK__RESOURCE_LUMINACE;
		break;

		// シャドウマップを描画
		// オブジェクトをシャドウマップrtへShadowMapシェーダ―で書き込む
	case SHADER_SHADOWMAP__VIEW_SHADOWMAP__RESOUCE_OBJ:
		SetShaderMode(SHADER_MODE_SHADOWMAP);
		SetRenderTargetView(VIEW_MODE_SHADOWMAP);

		g_ShaderMode = SHADER_SHADOWMAP__VIEW_SHADOWMAP__RESOUCE_OBJ;
		break;

		// シャドウマップをbackバッファへ
		// シャドウマップrtをbackバッファへデフォルトシェーダ―で書き込む
	case SHADER_DEFAULT__VIEW_BACK__RESOUCE_SHADOWMAP:
		SetShaderMode(SHADER_MODE_DEFAULT);
		SetRenderTargetView(VIEW_MODE_BACKBUFFER);
		SetShaderResouces(SHADER_RESOURCE_MODE_SHADOWMAP);

		g_ShaderMode = SHADER_DEFAULT__VIEW_BACK__RESOUCE_SHADOWMAP;
		break;

		// 影の付いたオブジェクトをPRErtへ
		// シャドウマップrtを元にPRErtへシャドウペーストシェーダ―で書き込む
	case SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ:
		SetShaderMode(SHADER_MODE_SHADOWPASTE);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);
		SetShaderResouces(SHADER_RESOURCE_MODE_SHADOWMAP);

		g_ShaderMode = SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ;
		break;

		// ディゾルブ処理をして影のないオブジェクトをPRErtへ
		// ノイズテクスチャを元にPRErtへ魔法陣用シェーダ―で書き込む
	case SHADER_CIRCLR__VIEW_PRE__RESOUCE_OBJ_NOISE:
		SetShaderMode(SHADER_MODE_CIRCLE);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);
		SetShaderResouces(SHADER_RESOURCE_MODE_NOISE);

		g_ShaderMode = SHADER_CIRCLR__VIEW_PRE__RESOUCE_OBJ_NOISE;
		break;

		// ディゾルブ処理をして影の付いたオブジェクトをPRErtへ
		// ノイズテクスチャを元にPRErtへエネミー用シェーダ―で書き込む
	case SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE:
		SetShaderMode(SHADER_MODE_ENEMY);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);
		SetShaderResouces(SHADER_RESOURCE_MODE_NOISE);

		g_ShaderMode = SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE;
		break;

		// ディゾルブの処理をして影の使いないオブジェクトをPRErtへ
		// ノイズテクスチャを元に水魔法に加工を加えPRErtへ水魔法用シェーダ―で書き込む
	case SHADER_WATER__VIEW_PRE__RESOUCE_OBJ_NOISE:
		SetShaderMode(SHADER_MODE_WATER);
		SetRenderTargetView(VIEW_MODE_PREVIOUS);
		SetShaderResouces(SHADER_RESOURCE_MODE_NOISE_WATER);

		g_ShaderMode = SHADER_WATER__VIEW_PRE__RESOUCE_OBJ_NOISE;
		break;

		// 一定以上の高さから透明にする処理
		// 定数バッファを元に爆風をPRErtへ爆風用シェーダーで書き込む
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
// ガウスの重み計算
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
// ブラーの計算を行い更新処理を行う
//=============================================================================
void RunBlur(void)
{
	// ガウスフィルタ用パラメータを設定
	GaussBlurParam buffer;
	memcpy(buffer.weights, g_weights, sizeof(float) * 8);
	buffer.offset.x = 16.0f / SCREEN_WIDTH;
	buffer.offset.y = 16.0f / SCREEN_HEIGHT;
	buffer.size.x = (float)SCREEN_WIDTH;
	buffer.size.y = (float)SCREEN_HEIGHT;

	g_ImmediateContext->UpdateSubresource(g_BlurParaBuffer, 0, NULL, &buffer, 0, 0);
}


//=============================================================================
// 現在の描画モードを取得
//=============================================================================
int GetShaderMode(void)
{
	return g_ShaderMode;
}