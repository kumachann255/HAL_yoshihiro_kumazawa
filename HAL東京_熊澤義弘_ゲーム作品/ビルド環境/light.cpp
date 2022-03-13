//=============================================================================
//
// ライト処理 [light.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "player.h"
#include "bahamut.h"
#include "beam_particle.h"
#include "input.h"
#include "camera.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_WIDTH)		// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(10000.0f)										// ビュー平面のFarZ値



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];

static FOG		g_Fog;

// 移動方向
static float	vector = -1.0f;

static BOOL		g_FogSwitch;

//=============================================================================
// 初期化処理
//=============================================================================
void InitLight(void)
{

	//ライト初期化
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[0].dir       = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[i].Direction = XMFLOAT3( 0.0f, -1.0f, 0.0f );
		g_Light[i].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
		g_Light[i].Ambient   = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		g_Light[i].Attenuation = 100.0f;	// 減衰距離
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ライトのタイプ
		g_Light[i].Enable = FALSE;			// ON / OFF
		SetLight(i, &g_Light[i]);
	}

	// 並行光源の設定（世界を照らす光）
	g_Light[0].Position = XMFLOAT3(-200.0f, 800.0f, 0.0f);
	g_Light[0].dir = XMFLOAT3(200.0f, -800.0f, 0.0f);
	g_Light[0].At = XMFLOAT3( 0.0f, 0.0f, 0.0f);
	g_Light[0].up = { 0.0f, 1.0f, 0.0f };
	g_Light[0].Direction = XMFLOAT3( 2.0f, -8.0f, 0.0f );		// 光の向き
	g_Light[0].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );	// 光の色
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源
	g_Light[0].Enable = TRUE;									// このライトをON
	SetLight(0, &g_Light[0]);									// これで設定している

	g_Light[1].Position = XMFLOAT3(0.0f, 50.0f,  50.0f);
	g_Light[1].dir = XMFLOAT3(10.0f, 10.0f, 10.0f);
	g_Light[1].At = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Light[1].up = { 0.0f, 1.0f, 0.0f };
	g_Light[1].Direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);		// 光の向き
	g_Light[1].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);	// 光の色
	g_Light[1].Type = LIGHT_TYPE_POINT;						// 並行光源
	g_Light[1].Enable = FALSE;								// このライトをON
	g_Light[1].Attenuation = 130.0f;						// 減衰距離

	SetLight(1, &g_Light[1]);								// これで設定している

	g_Light[2].Position = XMFLOAT3(0.0f, 50.0f, -50.0f);
	g_Light[2].dir = XMFLOAT3(2.0f, 2.0f, 2.0f);
	g_Light[2].At = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Light[2].up = { 0.0f, 1.0f, 0.0f };
	g_Light[2].Direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);		// 光の向き
	g_Light[2].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);	// 光の色
	g_Light[2].Type = LIGHT_TYPE_POINT;						// 並行光源
	g_Light[2].Enable = FALSE;								// このライトをON
	g_Light[2].Attenuation = 130.0f;						// 減衰距離

	SetLight(2, &g_Light[2]);								// これで設定している

	g_Light[3].Position = XMFLOAT3(0.0f, 50.0f, 10.0f);
	g_Light[3].dir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	g_Light[3].At = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Light[3].up = { 0.0f, 1.0f, 0.0f };
	g_Light[3].Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);		// 光の向き
	g_Light[3].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);	// 光の色
	g_Light[3].Type = LIGHT_TYPE_POINT;						// 並行光源
	g_Light[3].Enable = FALSE;								// このライトをON
	SetLight(3, &g_Light[3]);								// これで設定している

	// フォグの初期化（霧の効果）
	g_Fog.FogStart = 300.0f;								// 視点からこの距離離れるとフォグがかかり始める
	g_Fog.FogEnd   = 800.0f;								// ここまで離れるとフォグの色で見えなくなる
	g_Fog.FogColor = XMFLOAT4( 0.2f, 0.0f, 0.2f, 1.0f );	// フォグの色
	SetFog(&g_Fog);
	SetFogEnable(TRUE);
	g_FogSwitch = TRUE;

}


//=============================================================================
// 更新処理
//=============================================================================
void UpdateLight(void)
{
	// 光源をプレイヤーのポジション基準にする
	PLAYER *player = GetPlayer();

	g_Light[0].Position.x = player->pos.x - 200.0f;
	g_Light[0].Position.z = player->pos.z;
	g_Light[0].Position.y = player->pos.y + 800.0f;
	g_Light[0].At.x = player->pos.x;
	g_Light[0].At.z = player->pos.z;

	// ブレスを使用していた場合
	if (GetScene() >= charge)
	{
		BAHAMUT *bahamut = GetBahamut();

		for (int i = 1; i < 4; i++)
		{
			g_Light[i].Enable = TRUE;	// このライトをON

		// ライトの位置をバハムートの口ものに調整
			if (i == 3)
			{	// 目のポイントライト
				g_Light[i].Position.x = bahamut->pos.x + 79.0f;
				g_Light[i].Position.y = bahamut->pos.y + 142.7f;

				if (g_Light[i].Diffuse.x < 30.0f)
				{
					g_Light[i].Diffuse.x += 0.1f;
					g_Light[i].Diffuse.y += 0.1f;
					g_Light[i].Diffuse.z += 0.1f;
				}
			}
			else
			{	// 口元のポイントライト
				//g_Light[i].Position.x = bahamut->pos.x + 85.0f;
				g_Light[i].Position.x = bahamut->pos.x + 100.0f;
				g_Light[i].Position.y = bahamut->pos.y + 125.0f;

				if (g_Light[i].Diffuse.x < 30.0f)
				{
					g_Light[i].Diffuse.x += 0.1f;
					g_Light[i].Diffuse.y += 0.1f;
					g_Light[i].Diffuse.z += 0.1f;
				}
			}


			// 更新
			SetLight(i, &g_Light[i]);
		}
	}
	else
	{
		for (int i = 1; i < 4; i++)
		{
			g_Light[i].Enable = FALSE;
			g_Light[i].Diffuse.x = 0.0f;
			g_Light[i].Diffuse.y = 0.0f;
			g_Light[i].Diffuse.z = 0.0f;

			// 更新
			SetLight(i, &g_Light[i]);
		}
	}
}


//=============================================================================
// ライトの設定
// Typeによってセットするメンバー変数が変わってくる
//=============================================================================
void SetLightData(int index, LIGHT *light)
{
	SetLight(index, light);
}


LIGHT *GetLightData(int index)
{
	return(&g_Light[index]);
}


//=============================================================================
// フォグの設定
//=============================================================================
void SetFogData(FOG *fog)
{
	SetFog(fog);
}


//=============================================================================
// ライトの更新
//=============================================================================
void SetDirectionalLight(void)
{
	// ビューマトリックス設定
	XMMATRIX mtxView;
	mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Light[0].Position), XMLoadFloat3(&g_Light[0].At), XMLoadFloat3(&g_Light[0].up));
	SetLightViewMatrix(&mtxView);
	XMStoreFloat4x4(&g_Light[0].mtxView, mtxView);

	XMMATRIX mtxInvView;
	mtxInvView = XMMatrixInverse(nullptr, mtxView);
	XMStoreFloat4x4(&g_Light[0].mtxInvView, mtxInvView);


	// プロジェクションマトリックス設定
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE * 1.2f, VIEW_ASPECT, VIEW_NEAR_Z * 20.0f, VIEW_FAR_Z);

	SetLightProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&g_Light[0].mtxProjection, mtxProjection);
}


BOOL GetFogOnOff(void)
{
	return g_FogSwitch;
}