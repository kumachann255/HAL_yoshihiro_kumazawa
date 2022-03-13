//=============================================================================
//
// レンダリング処理 [renderer.h]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#pragma once


//*********************************************************
// マクロ定義
//*********************************************************
#define LIGHT_MAX		(5)

enum LIGHT_TYPE
{
	LIGHT_TYPE_NONE,		//ライト無し
	LIGHT_TYPE_DIRECTIONAL,	//ディレクショナルライト
	LIGHT_TYPE_POINT,		//ポイントライト

	LIGHT_TYPE_NUM
};

enum BLEND_MODE
{
	BLEND_MODE_NONE,		//ブレンド無し
	BLEND_MODE_ALPHABLEND,	//αブレンド
	BLEND_MODE_ADD,			//加算ブレンド
	BLEND_MODE_SUBTRACT,	//減算ブレンド

	BLEDD_MODE_NUM
};

enum CULL_MODE
{
	CULL_MODE_NONE,			//カリング無し
	CULL_MODE_FRONT,		//表のポリゴンを描画しない(CW)
	CULL_MODE_BACK,			//裏のポリゴンを描画しない(CCW)

	CULL_MODE_NUM
};

// シェーダーのモード
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

// レンダービューのモード
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

// シェーダ―リソースビューのモード
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

// シェーダーの設定一括変換モード
enum SHADER_OPTION
{
	// rtへ描画
	// オブジェクトをPRErtへデフォルトシェーダ―で書き込む
	SHADER_DEFAULT__VIEW_PRE__RESOURCE_OBJ,

	// 輝度抽出
	// PRErtを輝度抽出rtへLUMINACEシェーダ―で書き込む
	SHADER_LUMINACE__VIEW_LUMINACE__RESOUCE_PREVIOUS,

	// 輝度抽出(fire用)
	// fireを輝度抽出rtへLUMINACE_FIREシェーダ―で書き込む
	SHADER_LUMINACE_FIRE__VIEW_LUMINACE__RESOUCE_FIRE,

	// x方向へのブラー加工
	// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	SHADER_BLUR_X__VIEW_LUMINACE__RESOURCE_LUMINACE,

	// y方向へのブラー加工
	// 輝度抽出rtを輝度抽出rtへBlurシェーダ―で書き込む
	SHADER_BLUR_Y__VIEW_LUMINACE__RESOURCE_BULR,

	// 合成
	// 輝度抽出rtとPRErtをbackバッファへCompositeシェーダ―で書き込む
	SHADER_COMPOSITE__VIEW_BACK__RESOUCE_BLUR_PREVIOUS,

	// backバッファへ
	// PRErtをbackバッファへBACKシェーダ―で書き込む
	SHADER_BACK__VIEW_BACK__RESOURCE_PREVIOUS,

	// ブラー加工テスト
	// 輝度抽出rtをbackバッファへBlurシェーダ―で書き込む
	SHADER_BLUR_X__VIEW_BACK__RESOURCE_LUMINACE,
	SHADER_BLUR_Y__VIEW_BACK__RESOURCE_LUMINACE,

	// シャドウマップを描画
	// オブジェクトをシャドウマップrtへShadowMapシェーダ―で書き込む
	SHADER_SHADOWMAP__VIEW_SHADOWMAP__RESOUCE_OBJ,

	// シャドウマップをbackバッファへ
	// シャドウマップrtをbackバッファへデフォルトシェーダ―で書き込む
	SHADER_DEFAULT__VIEW_BACK__RESOUCE_SHADOWMAP,

	// 影の付いたオブジェクトをPRErtへ
	// シャドウマップrtを元にPRErtへシャドウペーストシェーダ―で書き込む
	SHADER_SHADOWPASTE__VIEW_PRE__RESOUCE_OBJ,

	// ディゾルブ処理をして影のないオブジェクトをPRErtへ
	// ノイズテクスチャを元にPRErtへ魔法陣用シェーダ―で書き込む
	SHADER_CIRCLR__VIEW_PRE__RESOUCE_OBJ_NOISE,

	// ディゾルブ処理をして影の付いたオブジェクトをPRErtへ
	// ノイズテクスチャを元にPRErtへエネミー用シェーダ―で書き込む
	SHADER_ENEMY__VIEW_PRE__RESOUCE_OBJ_NOISE,

	// ディゾルブの処理をして影の使いないオブジェクトをPRErtへ
	// ノイズテクスチャを元に水魔法に加工を加えPRErtへ水魔法用シェーダ―で書き込む
	SHADER_WATER__VIEW_PRE__RESOUCE_OBJ_NOISE,

	// 一定以上の高さから透明にする処理
	// 定数バッファを元に爆風をPRErtへ爆風用シェーダーで書き込む
	SHADER_BLAST__VIEW_PRE__RESOUCE_OBJ,


	// 通常描画
	// backバッファへ直接書き込む
	DEFAULT,
};


//*********************************************************
// 構造体
//*********************************************************

// 頂点構造体
struct VERTEX_3D
{
    XMFLOAT3	Position;
    XMFLOAT3	Normal;
    XMFLOAT4	Diffuse;
    XMFLOAT2	TexCoord;
};

// マテリアル構造体
struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	int			noTexSampling;
};

// ライト構造体
struct LIGHT {
	XMFLOAT4X4	mtxView;		// ビューマトリックス
	XMFLOAT4X4	mtxInvView;		// ビューマトリックス
	XMFLOAT4X4	mtxProjection;	// プロジェクションマトリックス

	XMFLOAT3	Direction;	// ライトの方向
	XMFLOAT3	Position;	// ライトの位置
	XMFLOAT3	dir;		// ライトの向き
	XMFLOAT3	At;			// ライトの注視点
	XMFLOAT3	up;			// ライトの上方向ベクトル
	XMFLOAT4	Diffuse;	// 拡散光の色
	XMFLOAT4	Ambient;	// 環境光の色
	float		Attenuation;// 減衰率
	int			Type;		// ライト種別・有効フラグ
	int			Enable;		// ライト種別・有効フラグ

};

// フォグ構造体
struct FOG {
	float		FogStart;	// フォグの開始距離
	float		FogEnd;		// フォグの最大距離
	XMFLOAT4	FogColor;	// フォグの色
};

// ブラー構造体
struct GaussBlurParam
{
	float		weights[8];
	XMFLOAT2	offset;
	XMFLOAT2	size;
};


//*****************************************************************************
// プロトタイプ宣言
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
