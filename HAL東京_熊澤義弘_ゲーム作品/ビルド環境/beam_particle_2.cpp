//=============================================================================
//
// ビームのパーティクル2処理 [beam_particle_2.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "beam_particle_2.h"
#include "player.h"
#include "holy.h"
#include "bahamut.h"
#include "lightning.h"
#include "debugproc.h"
#include "bahamut_bless.h"
#include "beam_orbit.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX			(1)			// テクスチャの数

#define	BEAM_PARTICLE_SIZE_X		(3.0f)		// 頂点サイズ
#define	BEAM_PARTICLE_SIZE_Y		(3.0f)		// 頂点サイズ
#define	VALUE_MOVE_BEAM_PARTICLE	(0.2f)		// 移動速度


#define BEAM_PARTICLE_OFFSET		(100)		// バハムート中心からどのくらい離して発生させるか

#define BEAM_SPEED					(0.025f)		// ビームの進む速度
#define BEAM_INTARVAL				(2)			// ビームの出る間隔

#define CONPOS_RADIUS				(30.0f)		// 制御点のランダム範囲

#define ROT_RADIUS					(20.0f)		// 回転の半径
#define RADIUS_SPEED				(0.7f)		// 半径の大きくなる速度


#define	DISP_SHADOW						// 影の表示
//#undef DISP_SHADOW


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexBeamParticle2(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static BEAM_PARTICLE_2			g_aBeamParticle2[MAX_BEAM_PARTICLE2];		// パーティクルワーク
static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 1.0f;			// 基準の幅
static float					g_fHeightBase = 1.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード

static int						g_count = 0;			// 何個のパーティクルを使用しているか超調査

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/particle.png",
};

static BOOL						g_Load = FALSE;
static BOOL						g_On = FALSE;		// パーティクルを発生さているか
static BOOL						g_End = TRUE;		// パーティクルを発生させたか
static float					g_Move;				// パーティクルの移動量のベース
static BOOL						g_Charge;			// チャージしているか、ビームを出しているか
static BOOL						g_MoveOn;			// チャージパーティクㇽが大きくなるかどうか
static float					g_Radius;			// 回転の基準角度
static int						g_Count;			// パーティクルを出すタイミングを管理

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBeamParticle2(void)
{
	// 頂点情報の作成
	MakeVertexBeamParticle2();

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	g_TexNo = 0;

	// パーティクルワークの初期化
	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE2; nCntParticle++)
	{
		g_aBeamParticle2[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle2[nCntParticle].basePos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle2[nCntParticle].distance = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle2[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle2[nCntParticle].scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
		g_aBeamParticle2[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_aBeamParticle2[nCntParticle].material, sizeof(g_aBeamParticle2[nCntParticle].material));
		g_aBeamParticle2[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aBeamParticle2[nCntParticle].fSizeX = BEAM_PARTICLE_SIZE_X;
		g_aBeamParticle2[nCntParticle].fSizeY = BEAM_PARTICLE_SIZE_Y;
		g_aBeamParticle2[nCntParticle].nIdxShadow = -1;
		g_aBeamParticle2[nCntParticle].nLife = 0;
		g_aBeamParticle2[nCntParticle].bUse = FALSE;
	}

	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;
	g_Move = 0.0f;
	g_Charge = FALSE;
	g_MoveOn = TRUE;
	g_Radius = 0.0f;
	g_Count = 0;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBeamParticle2(void)
{
	if (g_Load == FALSE) return;

	//テクスチャの解放
	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	// 頂点バッファの解放
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateBeamParticle2(void)
{
	BAHAMUT *bahamut = GetBahamut();

	XMFLOAT3 target = { 0.0f ,0.0f ,0.0f };
	target.x = bahamut->pos.x + BLAST_X_OFFSET - 120.0f;
	target.y -= 8.0f;

	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE2; nCntParticle++)
	{
		if (g_aBeamParticle2[nCntParticle].bUse)
		{// 使用中
			// 時間を進める
			g_aBeamParticle2[nCntParticle].time += BEAM_SPEED;

			// 現在の座標を算出
			g_aBeamParticle2[nCntParticle].pos.x = g_aBeamParticle2[nCntParticle].basePos.x - g_aBeamParticle2[nCntParticle].distance.x * g_aBeamParticle2[nCntParticle].time;
			g_aBeamParticle2[nCntParticle].pos.y = g_aBeamParticle2[nCntParticle].basePos.y - g_aBeamParticle2[nCntParticle].distance.y * g_aBeamParticle2[nCntParticle].time;
			g_aBeamParticle2[nCntParticle].pos.z = g_aBeamParticle2[nCntParticle].basePos.z - g_aBeamParticle2[nCntParticle].distance.z * g_aBeamParticle2[nCntParticle].time;

			// 回転の角度を更新
			if(nCntParticle % 2 == 0) g_aBeamParticle2[nCntParticle].radian += (float)(rand() % 10 + 10) / 100.0f;
			else g_aBeamParticle2[nCntParticle].radian -= (float)(rand() % 10 + 5) / 100.0f;

			// 回転の半径を算出して加算させる
			g_aBeamParticle2[nCntParticle].pos.y += sinf(g_aBeamParticle2[nCntParticle].radian) * g_aBeamParticle2[nCntParticle].radius;
			g_aBeamParticle2[nCntParticle].pos.z += cosf(g_aBeamParticle2[nCntParticle].radian) * g_aBeamParticle2[nCntParticle].radius;

			g_aBeamParticle2[nCntParticle].radius += RADIUS_SPEED;


			// 最初は透明で少しずつ姿を現す
			if(g_aBeamParticle2[nCntParticle].material.Diffuse.w < 1.0f) g_aBeamParticle2[nCntParticle].material.Diffuse.w += 0.2f;

			// y座標が0.0f以下になった際に未使用に
			if (g_aBeamParticle2[nCntParticle].pos.y < 0.0f)
			{
				g_aBeamParticle2[nCntParticle].bUse = FALSE;
			}
		}
	}

	//g_On = FALSE;
	// 翼が開いたタイミングでパーティクルを発生させる
	if ((GetKeyboardPress(DIK_5)) || (IsButtonPressed(0, BUTTON_A)))
	{
		g_On = TRUE;
	}

	if ((GetKeyboardPress(DIK_9)) || (IsButtonPressed(0, BUTTON_A)))
	{
		g_Move += 0.01f;
	}
	if ((GetKeyboardPress(DIK_8)) || (IsButtonPressed(0, BUTTON_A)))
	{
		g_Move -= 0.005f;
	}

	// 基準角度は常に回転している
	g_Radius += 0.3f;

	// 時間を進める
	g_Count++;

	// パーティクル発生
	if ((g_On))
		{
		// 少しずつ半径を大きくしていく
		if((g_Move < 1.0f) && (g_MoveOn)) g_Move += 0.006f;

		for (int i = 0; i < 8; i++)
		{
			XMFLOAT3 pos;

			pos = bahamut->pos;
			pos.y += 125.0f;
			pos.x += 85.0f;

			XMFLOAT3 move, base;
			XMFLOAT4 color;
			int nLife;
			float fSize;

			move = { g_Move,g_Move,g_Move };
			
			pos.x += (float)(rand() % 200 - 100) / 100.0f * ROT_RADIUS / 6;
			pos.y += (float)(rand() % 200 - 100) / 100.0f * ROT_RADIUS / 6;
			pos.z += (float)(rand() % 200 - 100) / 100.0f * ROT_RADIUS / 6;

			base = move;

			nLife = rand() & 40 + 20;

			//fSize = RamdomFloat(3, 0.8f, 0.3f);
			fSize = (float)(rand() % 120 + 60) / 100.0f;

			color.x = 1.0f;
			color.y = 0.3f;
			color.z = 1.0f;


			// ビルボードの設定
			SetBeamParticle2(pos, move, base, XMFLOAT4(color.x, color.y, color.z, 0.0f), fSize, fSize, nLife);
		}
	}

#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("life:%d \n", g_aBeamParticle2[100].nLife);
	PrintDebugProc("bezier.x:%f y:%f z:%f\n", g_aBeamParticle2[0].pos.x, g_aBeamParticle2[0].pos.y, g_aBeamParticle2[0].pos.z);
	PrintDebugProc("beam count : %d\n", g_count);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBeamParticle2(void)
{
	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// ライティングを無効に
	SetLightEnable(FALSE);

	//// 加算合成に設定
	//SetBlendState(BLEND_MODE_ADD);

	// Z比較無し
	SetDepthEnable(FALSE);

	// フォグ無効
	SetFogEnable(FALSE);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE2; nCntParticle++)
	{
		if (g_aBeamParticle2[nCntParticle].bUse)
		{
			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// ビューマトリックスを取得
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			// 転置行列処理
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

			// スケールを反映
			mtxScl = XMMatrixScaling(g_aBeamParticle2[nCntParticle].scale.x, g_aBeamParticle2[nCntParticle].scale.y, g_aBeamParticle2[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aBeamParticle2[nCntParticle].pos.x, g_aBeamParticle2[nCntParticle].pos.y, g_aBeamParticle2[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_aBeamParticle2[nCntParticle].material);

			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
			//// ポリゴンの描画
			//GetDeviceContext()->Draw(4, 0);
		}
	}

	// ライティングを有効に
	SetLightEnable(TRUE);

	//// 通常ブレンドに戻す
	//SetBlendState(BLEND_MODE_ALPHABLEND);

	// Z比較有効
	SetDepthEnable(TRUE);

	// フォグ有効
	SetFogEnable(GetFogSwitch());

}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexBeamParticle2(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	{//頂点バッファの中身を埋める
		D3D11_MAPPED_SUBRESOURCE msr;
		GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

		VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

		// 頂点座標の設定
		vertex[0].Position = XMFLOAT3(-BEAM_PARTICLE_SIZE_X / 2,  BEAM_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3( BEAM_PARTICLE_SIZE_X / 2,  BEAM_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-BEAM_PARTICLE_SIZE_X / 2, -BEAM_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3( BEAM_PARTICLE_SIZE_X / 2, -BEAM_PARTICLE_SIZE_Y / 2, 0.0f);

		// 法線の設定
		vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

		// 反射光の設定
		vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		// テクスチャ座標の設定
		vertex[0].TexCoord = { 0.0f, 0.0f };
		vertex[1].TexCoord = { 1.0f, 0.0f };
		vertex[2].TexCoord = { 0.0f, 1.0f };
		vertex[3].TexCoord = { 1.0f, 1.0f };

		GetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}

	return S_OK;
}

//=============================================================================
// マテリアルカラーの設定
//=============================================================================
void SetColorBeamParticle2(int nIdxParticle, XMFLOAT4 col)
{
	g_aBeamParticle2[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// チャージ時のパーティクルの発生処理
//=============================================================================
int SetBeamParticle2(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT3 base, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife)
{
	BAHAMUT *bahamut = GetBahamut();
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE2; nCntParticle++)
	{
		if (!g_aBeamParticle2[nCntParticle].bUse)
		{
			g_aBeamParticle2[nCntParticle].pos = pos;
			g_aBeamParticle2[nCntParticle].basePos = pos;
			g_aBeamParticle2[nCntParticle].scale = { fSizeX, fSizeX, fSizeX };
			g_aBeamParticle2[nCntParticle].move = move;
			g_aBeamParticle2[nCntParticle].material.Diffuse = col;

			if (nCntParticle % 2 == 0)
			{
				g_aBeamParticle2[nCntParticle].material.Diffuse.x = 0.3f;
				g_aBeamParticle2[nCntParticle].material.Diffuse.y = 1.0f;
				g_aBeamParticle2[nCntParticle].material.Diffuse.z = 1.0f;
			}

			g_aBeamParticle2[nCntParticle].fSizeX = fSizeX;
			g_aBeamParticle2[nCntParticle].fSizeY = fSizeY;
			g_aBeamParticle2[nCntParticle].nLife = nLife;
			g_aBeamParticle2[nCntParticle].bUse = TRUE;
			g_aBeamParticle2[nCntParticle].radius = 5.0f;
			g_aBeamParticle2[nCntParticle].radian = g_Radius;

			// 距離を記録
			g_aBeamParticle2[nCntParticle].distance.x = g_aBeamParticle2[nCntParticle].basePos.x - bahamut->pos.x - BLAST_X_OFFSET + 200.0f;
			g_aBeamParticle2[nCntParticle].distance.y = g_aBeamParticle2[nCntParticle].basePos.y - bahamut->pos.y;
			g_aBeamParticle2[nCntParticle].distance.z = g_aBeamParticle2[nCntParticle].basePos.z - bahamut->pos.z;

			g_aBeamParticle2[nCntParticle].time = 0.0f;

			nIdxParticle = nCntParticle;

			if (g_count < nCntParticle) g_count = nCntParticle;
			
			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// ベジェ曲線のパーティクルの発生処理
//=============================================================================
//int SetBezierParticle2(XMFLOAT3 pos, XMFLOAT3 posbase, XMFLOAT3 conpos1, XMFLOAT3 conpos1base, XMFLOAT3 conpos2, XMFLOAT3 conpos2base, XMFLOAT4 col, float fSize)
//{
//	int nIdxParticle = -1;
//
//	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE2; nCntParticle++)
//	{
//		if (!g_aBeamParticle2[nCntParticle].bUse)
//		{
//			g_aBeamParticle2[nCntParticle].pos = pos;
//			g_aBeamParticle2[nCntParticle].distance = XMFLOAT3(0.0f, 0.0f, 0.0f);
//			g_aBeamParticle2[nCntParticle].scale = { fSize, fSize, fSize };
//			g_aBeamParticle2[nCntParticle].material.Diffuse = col;
//			g_aBeamParticle2[nCntParticle].bUse = TRUE;
//			g_aBeamParticle2[nCntParticle].first = TRUE;
//
//			// 0番はまっすぐの直線
//			if (nCntParticle == 0)
//			{
//				g_aBeamParticle2[nCntParticle].ControlPos0 = posbase;
//				g_aBeamParticle2[nCntParticle].ControlPos1 = conpos1base;
//				g_aBeamParticle2[nCntParticle].ControlPos2 = conpos2base;
//			}
//			else
//			{
//				g_aBeamParticle2[nCntParticle].ControlPos0 = pos;
//				g_aBeamParticle2[nCntParticle].ControlPos1 = conpos1;
//				g_aBeamParticle2[nCntParticle].ControlPos2 = conpos2;
//			}
//
//			g_aBeamParticle2[nCntParticle].time = 0.0f;
//
//			nIdxParticle = nCntParticle;
//
//			break;
//		}
//	}
//
//	return nIdxParticle;
//}

//=============================================================================
// パーティクルの発生スイッチ
//=============================================================================
void SetBeamParticleSwitch2(BOOL data)
{
	g_On = data;
}


//=============================================================================
// パーティクルの種類スイッチ(TRUE: チャージ　FALSE:ビーム)
//=============================================================================
void SetBeamParticleCharge2(BOOL data)
{
	g_Charge = data;
}



//=============================================================================
// パーティクルの座標を取得
//=============================================================================
BEAM_PARTICLE_2 *GetBeamParticle2(void)
{
	return &g_aBeamParticle2[0];
}


//=============================================================================
// パーティクルの一斉削除と停止
//=============================================================================
void SetBeam2Delete(void)
{
	for (int i = 0; i < MAX_BEAM_PARTICLE2; i++)
	{
		g_aBeamParticle2[i].bUse = FALSE;
	}

	g_On = FALSE;
	g_Move = 0.0f;
	g_MoveOn = TRUE;
}


//=============================================================================
// チャージ時の半径を変更
//=============================================================================
void SetChargeRadius2(float radius)
{
	g_Move = radius;
	g_MoveOn = FALSE;
}