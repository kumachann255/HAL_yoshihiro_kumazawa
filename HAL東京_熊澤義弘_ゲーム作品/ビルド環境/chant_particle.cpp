//=============================================================================
//
// 詠唱時のパーティクル処理 [chant_particle.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "chant_particle.h"
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

#define	CHANT_PARTICLE_SIZE_X		(1.5f)		// 頂点サイズ
#define	CHANT_PARTICLE_SIZE_Y		(1.5f)		// 頂点サイズ
#define	VALUE_MOVE_CHANT_PARTICLE	(0.2f)		// 移動速度


#define CHANT_PARTICLE_OFFSET		(100)		// バハムート中心からどのくらい離して発生させるか

#define CHANT_SPEED					(0.05f)		// ビームの進む速度
#define CHANT_INTARVAL				(2)			// ビームの出る間隔

#define CHANT_RADIUS_BASE			(5.0f)		// 制御点のランダム範囲
#define CHANT_INTERVAL				(20)			// パーティクルを発生させる間隔

#define	DISP_SHADOW						// 影の表示
//#undef DISP_SHADOW


//*****************************************************************************
// 構造体定義
//*****************************************************************************
typedef struct 
{
	XMFLOAT3		pos;			// 位置
	XMFLOAT3		posbase;		// 基準座標
	XMFLOAT3		rot;			// 回転
	XMFLOAT3		scale;			// スケール
	XMFLOAT3		move;			// 移動量
	MATERIAL		material;		// マテリアル
	float			fSizeX;			// 幅
	float			fSizeY;			// 高さ
	int				nIdxShadow;		// 影ID
	int				nLife;			// 寿命
	BOOL			bUse;			// 使用しているかどうか
	float			radius;			// 回転の半径
	float			thetaX;			// 回転
	float			thetaZ;			// 回転
}PARTICLE;


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexChantParticle(void);
int SetBezierParticle(XMFLOAT3 pos, XMFLOAT3 posbase, XMFLOAT3 conpos1, XMFLOAT3 conpos1base, XMFLOAT3 conpos2, XMFLOAT3 conpos2base, XMFLOAT4 col, float fSize);
//int SetBezierParticle(XMFLOAT3 pos, XMFLOAT3 conpos1, XMFLOAT3 conpos2, XMFLOAT3 conpos3, XMFLOAT4 col, float fSize);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static PARTICLE					g_aChantParticle[MAX_CHANT_PARTICLE];		// パーティクルワーク
static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 1.0f;			// 基準の幅
static float					g_fHeightBase = 1.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード

static int						g_count = 0;			// 何個のパーティクルを使用しているか超調査

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/effect000.jpg",
};

static BOOL						g_Load = FALSE;
static BOOL						g_On = FALSE;		// パーティクルを発生さているか
static int						g_Count = 0;		// パーティクルを発生させるタイミングを管理
static float					g_Radius;			// パーティクルを発生させる半径

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitChantParticle(void)
{
	// 頂点情報の作成
	MakeVertexChantParticle();

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
	for (int nCntParticle = 0; nCntParticle < MAX_CHANT_PARTICLE; nCntParticle++)
	{
		g_aChantParticle[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aChantParticle[nCntParticle].posbase = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aChantParticle[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aChantParticle[nCntParticle].scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
		g_aChantParticle[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

		ZeroMemory(&g_aChantParticle[nCntParticle].material, sizeof(g_aChantParticle[nCntParticle].material));
		g_aChantParticle[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aChantParticle[nCntParticle].fSizeX = CHANT_PARTICLE_SIZE_X;
		g_aChantParticle[nCntParticle].fSizeY = CHANT_PARTICLE_SIZE_Y;
		g_aChantParticle[nCntParticle].nIdxShadow = -1;
		g_aChantParticle[nCntParticle].nLife = 0;
		g_aChantParticle[nCntParticle].bUse = FALSE;
		g_aChantParticle[nCntParticle].radius = 0.0f;
		g_aChantParticle[nCntParticle].thetaX = 0.0f;
		g_aChantParticle[nCntParticle].thetaZ = 0.0f;
	}

	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;
	g_Radius = 30.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitChantParticle(void)
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
void UpdateChantParticle(void)
{
	PLAYER *player = GetPlayer();

	//g_On = FALSE;
	//// 翼が開いたタイミングでパーティクルを発生させる
	//if ((GetKeyboardPress(DIK_F)) || (IsButtonPressed(0, BUTTON_A)))
	//{
	//	g_On = TRUE;
	//}


	for (int nCntParticle = 0; nCntParticle < MAX_CHANT_PARTICLE; nCntParticle++)
	{
		if (g_aChantParticle[nCntParticle].bUse)
		{// 使用中
			// 回転を進める
			g_aChantParticle[nCntParticle].thetaX += g_aChantParticle[nCntParticle].move.x;
			g_aChantParticle[nCntParticle].thetaZ += g_aChantParticle[nCntParticle].move.z;

			// 座標調整
			g_aChantParticle[nCntParticle].pos.x = g_aChantParticle[nCntParticle].posbase.x + g_aChantParticle[nCntParticle].radius * sinf(g_aChantParticle[nCntParticle].thetaX);
			g_aChantParticle[nCntParticle].pos.z = g_aChantParticle[nCntParticle].posbase.z + g_aChantParticle[nCntParticle].radius * cosf(g_aChantParticle[nCntParticle].thetaZ);
			g_aChantParticle[nCntParticle].pos.y += g_aChantParticle[nCntParticle].move.y;


			// 徐々に発生させる
			if (g_aChantParticle[nCntParticle].material.Diffuse.w < 1.0f)
			{
				g_aChantParticle[nCntParticle].material.Diffuse.w += 0.1f;
			}


			// 寿命近くなると透明になっていく処理
			g_aChantParticle[nCntParticle].nLife--;
			if ((g_aChantParticle[nCntParticle].nLife <= 0))
			{
				g_aChantParticle[nCntParticle].bUse = FALSE;
				g_aChantParticle[nCntParticle].nIdxShadow = -1;
			}
			else
			{
				if (g_aChantParticle[nCntParticle].nLife <= 10)
				{
					// α値設定
					g_aChantParticle[nCntParticle].material.Diffuse.w -= 0.15f;
					if (g_aChantParticle[nCntParticle].material.Diffuse.w < 0.0f)
					{
						g_aChantParticle[nCntParticle].material.Diffuse.w = 0.0f;
					}
				}
			}
		}
	}


	if ((GetKeyboardPress(DIK_6)) || (IsButtonPressed(0, BUTTON_A)))
	{
		g_Radius += 0.01f;
	}
	if ((GetKeyboardPress(DIK_5)) || (IsButtonPressed(0, BUTTON_A)))
	{
		g_Radius -= 0.01f;
	}

	if (g_On)
	{
		// パーティクル発生
		if (g_Count % CHANT_INTERVAL == 0)
		{
			// 少しずつ半径を大きくしていく
			if (g_Radius < 150.0f) g_Radius += 0.3f;

			int count = 1;	// パーティクルの発生個数

			// 半径に応じて発生個数を多くする
			if (g_Radius >= 150.0f) count = 4;
			else if (g_Radius > 100.0f) count = 3;
			else if (g_Radius > 50.0f) count = 2;

			for (int i = 0; i < count; i++)
			{
				XMFLOAT3 pos;
				XMFLOAT3 move;
				XMFLOAT4 color;
				float radius, theta;
				int nLife;
				float fSize;

				pos = player->pos;

				// 半径と角度をランダム化
				radius = (float)(rand() % (int)g_Radius);
				theta = (float)(rand() % 628 - 314) / 100.0f;
				pos.x += g_Radius * sinf(theta);

				// 半径と角度を再ランダム化
				radius = (float)(rand() % (int)g_Radius);
				theta = (float)(rand() % 628 - 314) / 100.0f;
				pos.z += g_Radius * cosf(theta);

				// 回転の半径をランダム化
				radius = (float)(rand() % 200 - 100) / 10.0f;

				// 回転の速さをセット
				move.x = (float)(rand() % 100 - 50) / 1000.0f;
				move.z = (float)(rand() % 100 - 50) / 1000.0f;

				// 上昇で固定
				move.y = (float)(rand() % 100 + 400) / 1000.0f;

				// 寿命をセット
				nLife = rand() & 50 + 80;

				fSize = RamdomFloat(3, 2.0f, 1.0f);

				// 色をセット
				color.x = (float)(rand() % 60 + 40) / 100.0f;
				color.y = (float)(rand() % 60 + 40) / 100.0f;
				color.z = (float)(rand() % 10 + 90) / 100.0f;
				color.w = 0.0f;

				// ビルボードの設定
				SetChantParticle(pos, move, color, fSize, nLife, radius);
			}
			//g_End = TRUE;
		}
	}
	else
	{
		if (g_Radius > 30.0f) g_Radius -= 0.3f;
	}
	//else
	//{
	//	XMFLOAT3 target = { 0.0f ,0.0f ,0.0f };
	//	target.x = bahamut->pos.x + BLAST_X_OFFSET + 585.0f;
	//	target.y = -160.0f;
	//	for (int i = 0; i < MAX_CHANT_PARTICLE; i++)
	//	{
	//		if (g_aChantParticle[i].bUse)
	//		{// 使用中
	//			// 移動前の座標を保存
	//			XMFLOAT3 pos_old;
	//			pos_old = g_aChantParticle[i].pos;

	//			// ベジェ曲線の処理
	//			// 計算式が長くならないよう調整するため
	//			float t = g_aChantParticle[i].time;
	//			float t1 = 1.0f - g_aChantParticle[i].time;

	//			// ベジェ曲線の処理
	//			g_aChantParticle[i].pos.x = ((t1 * t1 * t1) * g_aChantParticle[i].ControlPos0.x)
	//				+ (3 * t * t1 * t1 * g_aChantParticle[i].ControlPos1.x) 
	//				+ (3 * t * t * t1 * g_aChantParticle[i].ControlPos2.x) 
	//				+ ( t * t * t * target.x);

	//			g_aChantParticle[i].pos.z = ((t1 * t1 * t1) * g_aChantParticle[i].ControlPos0.z) 
	//				+ (3 * t * t1 * t1 * g_aChantParticle[i].ControlPos1.z) 
	//				+ (3 * t * t * t1 * g_aChantParticle[i].ControlPos2.z) 
	//				+ ( t * t * t * target.z);

	//			g_aChantParticle[i].pos.y = ((t1 * t1 * t1) * g_aChantParticle[i].ControlPos0.y) 
	//				+ (3 * t * t1 * t1 * g_aChantParticle[i].ControlPos1.y) 
	//				+ (3 * t * t * t1 * g_aChantParticle[i].ControlPos2.y) 
	//				+ ( t * t * t * target.y);


	//			//	時間を進める
	//			g_aChantParticle[i].time += BEAM_SPEED;

	//			// 移動量を保存
	//			g_aChantParticle[i].distance.x = g_aChantParticle[i].pos.x - pos_old.x;
	//			g_aChantParticle[i].distance.z = g_aChantParticle[i].pos.z - pos_old.z;
	//			g_aChantParticle[i].distance.y = g_aChantParticle[i].pos.y - pos_old.y;

	//			// 目的地に着いたら消す
	//			if (g_aChantParticle[i].time >= 1.0f) g_aChantParticle[i].bUse = FALSE;		// 移動が終わったら消す
	//		}
	//	}

	//	// 翼が開いたタイミングでパーティクルを発生させる
	//	if ((GetKeyboardPress(DIK_C)) || (IsButtonPressed(0, BUTTON_A)))
	//	{
	//		g_On = TRUE;

	//		SetBeamOrbitSwitch(TRUE);
	//	}

	//	// パーティクル発生
	//	if (g_On)
	//	{
	//		g_Count++;

	//		if (g_Count % BEAM_INTARVAL == 0)
	//		{
	//			XMFLOAT3 pos, posbase, conpos1, conpos1base, conpos2, conpos2base;
	//			XMFLOAT4 color;
	//			float fSize;

	//			BAHAMUT *bahamut = GetBahamut();

	//			pos = bahamut->pos;

	//			// 口元に合わせる
	//			pos.y += 125.0f;
	//			pos.x += 85.0f;

	//			posbase = pos;

	//			// 制御点の初期化
	//			conpos1 = conpos2 = { 0.0f, 0.0f ,0.0f };

	//			// 制御点の基点をセット
	//			conpos1.x = pos.x + ((BLAST_X_OFFSET + 100.0f) / 2);
	//			conpos1.y = pos.y / 2;

	//			conpos1base = conpos1;

	//			conpos2.x = pos.x + (BLAST_X_OFFSET + 100.0f);
	//			conpos2.y = 0.0f;

	//			conpos2base = conpos2;

	//			// 制御点のランダム化
	//			pos.x += CONPOS_RADIUS / 7 * ((float)(rand() % 200 - 100) / 100.0f);
	//			pos.y += CONPOS_RADIUS / 7 * ((float)(rand() % 200 - 100) / 100.0f);
	//			pos.z += CONPOS_RADIUS / 7 * ((float)(rand() % 200 - 100) / 100.0f);

	//			conpos1.x += CONPOS_RADIUS / 2 * ((float)(rand() % 200 - 100) / 100.0f);
	//			conpos1.y += CONPOS_RADIUS / 2 * ((float)(rand() % 200 - 100) / 100.0f);
	//			conpos1.z += CONPOS_RADIUS / 2 * ((float)(rand() % 200 - 100) / 100.0f);

	//			conpos2.x += CONPOS_RADIUS * 2 * ((float)(rand() % 200 - 100) / 100.0f);
	//			conpos2.y += CONPOS_RADIUS * 2 * ((float)(rand() % 200 - 100) / 100.0f);
	//			conpos2.z += CONPOS_RADIUS * 2 * ((float)(rand() % 200 - 100) / 100.0f);

	//			// サイズのランダム化
	//			//fSize = RamdomFloat(2, 1.5f, 0.5f);
	//			fSize = 1.0f;

	//			// 色のランダム化
	//			color.x = (float)(rand() % 60 + 40) / 100.0f;
	//			color.y = (float)(rand() % 60 + 40) / 100.0f;
	//			color.z = (float)(rand() % 10 + 90) / 100.0f;
	//			color.w = 1.0f;
	//			// パーティクルをセット
	//			SetBezierParticle(pos, posbase, conpos1, conpos1base, conpos2, conpos2base, color, fSize);
	//		}
	//	}
	//}

#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("life:%d \n", g_aChantParticle[100].nLife);
	PrintDebugProc("chant.x:%f y;%f z:%f \n", g_aChantParticle[0].pos.x, g_aChantParticle[0].pos.y, g_aChantParticle[0].pos.z);
	PrintDebugProc("chant count : %d\n", g_count);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawChantParticle(void)
{
	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// ライティングを無効に
	SetLightEnable(FALSE);

	// 加算合成に設定
	SetBlendState(BLEND_MODE_ADD);

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

	for (int nCntParticle = 0; nCntParticle < MAX_CHANT_PARTICLE; nCntParticle++)
	{
		if (g_aChantParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aChantParticle[nCntParticle].scale.x, g_aChantParticle[nCntParticle].scale.y, g_aChantParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aChantParticle[nCntParticle].pos.x, g_aChantParticle[nCntParticle].pos.y, g_aChantParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_aChantParticle[nCntParticle].material);

			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
			// ポリゴンの描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ライティングを有効に
	SetLightEnable(TRUE);

	// 通常ブレンドに戻す
	SetBlendState(BLEND_MODE_ALPHABLEND);

	// Z比較有効
	SetDepthEnable(TRUE);

	// フォグ有効
	SetFogEnable(GetFogSwitch());

}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexChantParticle(void)
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
		vertex[0].Position = XMFLOAT3(-CHANT_PARTICLE_SIZE_X / 2,  CHANT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3( CHANT_PARTICLE_SIZE_X / 2,  CHANT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-CHANT_PARTICLE_SIZE_X / 2, -CHANT_PARTICLE_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3( CHANT_PARTICLE_SIZE_X / 2, -CHANT_PARTICLE_SIZE_Y / 2, 0.0f);

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
void SetColorChantParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aChantParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// チャージ時のパーティクルの発生処理
//=============================================================================
int SetChantParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT4 col, float fSize, int nLife, float radius)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_CHANT_PARTICLE; nCntParticle++)
	{
		if (!g_aChantParticle[nCntParticle].bUse)
		{
			g_aChantParticle[nCntParticle].pos = pos;
			g_aChantParticle[nCntParticle].posbase = pos;
			g_aChantParticle[nCntParticle].scale = { fSize, fSize, fSize };
			g_aChantParticle[nCntParticle].move = move;
			g_aChantParticle[nCntParticle].material.Diffuse = col;
			g_aChantParticle[nCntParticle].nLife = nLife;
			g_aChantParticle[nCntParticle].bUse = TRUE;
			g_aChantParticle[nCntParticle].radius = radius;

			nIdxParticle = nCntParticle;

			if (g_count < nCntParticle) g_count = nCntParticle;
			
			break;
		}
	}

	return nIdxParticle;
}

////=============================================================================
//// ベジェ曲線のパーティクルの発生処理
////=============================================================================
//int SetBezierParticle(XMFLOAT3 pos, XMFLOAT3 posbase, XMFLOAT3 conpos1, XMFLOAT3 conpos1base, XMFLOAT3 conpos2, XMFLOAT3 conpos2base, XMFLOAT4 col, float fSize)
//{
//	int nIdxParticle = -1;
//
//	for (int nCntParticle = 0; nCntParticle < MAX_CHANT_PARTICLE; nCntParticle++)
//	{
//		if (!g_aChantParticle[nCntParticle].bUse)
//		{
//			g_aChantParticle[nCntParticle].pos = pos;
//			g_aChantParticle[nCntParticle].scale = { fSize, fSize, fSize };
//			g_aChantParticle[nCntParticle].material.Diffuse = col;
//			g_aChantParticle[nCntParticle].bUse = TRUE;
//			g_aChantParticle[nCntParticle].first = TRUE;
//
//			// 0番はまっすぐの直線
//			if (nCntParticle == 0)
//			{
//				g_aChantParticle[nCntParticle].ControlPos0 = posbase;
//				g_aChantParticle[nCntParticle].ControlPos1 = conpos1base;
//				g_aChantParticle[nCntParticle].ControlPos2 = conpos2base;
//			}
//			else
//			{
//				g_aChantParticle[nCntParticle].ControlPos0 = pos;
//				g_aChantParticle[nCntParticle].ControlPos1 = conpos1;
//				g_aChantParticle[nCntParticle].ControlPos2 = conpos2;
//			}
//
//			g_aChantParticle[nCntParticle].time = 0.0f;
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
void SetChantParticleSwitch(BOOL data)
{
	g_On = data;
}

//
////=============================================================================
//// パーティクルの座標を取得
////=============================================================================
//PARTICLE *GetChantParticle(void)
//{
//	return &g_aChantParticle[0];
//}
//

//=============================================================================
// パーティクルの一斉削除と停止
//=============================================================================
void SetChantDelete(void)
{
	for (int i = 0; i < MAX_CHANT_PARTICLE; i++)
	{
		g_aChantParticle[i].bUse = FALSE;
	}

	g_On = FALSE;
	g_Count = 0;
}