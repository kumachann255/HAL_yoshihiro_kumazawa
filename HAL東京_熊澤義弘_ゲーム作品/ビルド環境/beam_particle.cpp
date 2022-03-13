//=============================================================================
//
// ビームのパーティクル処理 [beam_particle.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "beam_particle.h"
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

#define	BEAM_PARTICLE_SIZE_X		(2.5f)		// 頂点サイズ
#define	BEAM_PARTICLE_SIZE_Y		(2.5f)		// 頂点サイズ
#define	VALUE_MOVE_BEAM_PARTICLE	(0.2f)		// 移動速度


#define BEAM_PARTICLE_OFFSET		(100)		// バハムート中心からどのくらい離して発生させるか

#define BEAM_SPEED					(0.012f)	// ビームの進む速度
#define BEAM_INTARVAL				(2)			// ビームの出る間隔

#define CONPOS_RADIUS				(30.0f)		// 制御点のランダム範囲

#define	DISP_SHADOW						// 影の表示
//#undef DISP_SHADOW


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexBeamParticle(void);
int SetBezierParticle(XMFLOAT3 pos, XMFLOAT3 posbase, XMFLOAT3 conpos1, XMFLOAT3 conpos1base, XMFLOAT3 conpos2, XMFLOAT3 conpos2base, XMFLOAT4 col, float fSize);
//int SetBezierParticle(XMFLOAT3 pos, XMFLOAT3 conpos1, XMFLOAT3 conpos2, XMFLOAT3 conpos3, XMFLOAT4 col, float fSize);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static BEAM_PARTICLE			g_aBeamParticle[MAX_BEAM_PARTICLE];		// パーティクルワーク
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
static BOOL						g_End = TRUE;		// パーティクルを発生させたか
static int						g_Count = 0;		// どの剣からパーティクルを発生させるか
static float					g_Move;				// パーティクルの移動量のベース
static BOOL						g_Charge;			// チャージしているか、ビームを出しているか
static BOOL						g_MoveOn;			// チャージパーティクㇽが大きくなるかどうか

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBeamParticle(void)
{
	// 頂点情報の作成
	MakeVertexBeamParticle();

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
	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE; nCntParticle++)
	{
		g_aBeamParticle[nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle[nCntParticle].distance = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle[nCntParticle].ControlPos0 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle[nCntParticle].ControlPos1 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle[nCntParticle].ControlPos2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle[nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aBeamParticle[nCntParticle].scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
		g_aBeamParticle[nCntParticle].move = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_aBeamParticle[nCntParticle].baseMove = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_aBeamParticle[nCntParticle].first = TRUE;

		ZeroMemory(&g_aBeamParticle[nCntParticle].material, sizeof(g_aBeamParticle[nCntParticle].material));
		g_aBeamParticle[nCntParticle].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aBeamParticle[nCntParticle].fSizeX = BEAM_PARTICLE_SIZE_X;
		g_aBeamParticle[nCntParticle].fSizeY = BEAM_PARTICLE_SIZE_Y;
		g_aBeamParticle[nCntParticle].nIdxShadow = -1;
		g_aBeamParticle[nCntParticle].nLife = 0;
		g_aBeamParticle[nCntParticle].bUse = FALSE;
	}

	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;
	g_Move = 0.0f;
	g_Charge = TRUE;
	g_MoveOn = TRUE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBeamParticle(void)
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
void UpdateBeamParticle(void)
{
	BAHAMUT *bahamut = GetBahamut();

	if ((GetKeyboardPress(DIK_C)) || (IsButtonPressed(0, BUTTON_A)))
	{
		SetSummonScene(charge);
		SetSummon();
		//g_On = TRUE;

		//SetBeamOrbitSwitch(TRUE);
	}


	if (g_Charge)
	{
		for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE; nCntParticle++)
		{
			if (g_aBeamParticle[nCntParticle].bUse)
			{// 使用中
				g_aBeamParticle[nCntParticle].time += (float)(rand() % 70 + 50) / 1000.0f;

				g_aBeamParticle[nCntParticle].pos.x += g_aBeamParticle[nCntParticle].move.x * sinf(g_aBeamParticle[nCntParticle].time);
				g_aBeamParticle[nCntParticle].pos.z += g_aBeamParticle[nCntParticle].move.z * cosf(g_aBeamParticle[nCntParticle].time);
				g_aBeamParticle[nCntParticle].pos.y += g_aBeamParticle[nCntParticle].move.y * sinf(g_aBeamParticle[nCntParticle].time);


				// 寿命近くなると透明になっていく処理
				g_aBeamParticle[nCntParticle].nLife--;
				if ((g_aBeamParticle[nCntParticle].nLife <= 0))
				{
					g_aBeamParticle[nCntParticle].bUse = FALSE;
					g_aBeamParticle[nCntParticle].nIdxShadow = -1;
				}
				else
				{
					if (g_aBeamParticle[nCntParticle].nLife <= 10)
					{
						// α値設定
						g_aBeamParticle[nCntParticle].material.Diffuse.w -= 0.05f;
						if (g_aBeamParticle[nCntParticle].material.Diffuse.w < 0.0f)
						{
							g_aBeamParticle[nCntParticle].material.Diffuse.w = 0.0f;
						}
					}
				}
			}
		}

		// 翼が開いたタイミングでパーティクルを発生させる
		if ((GetKeyboardPress(DIK_B)) || (IsButtonPressed(0, BUTTON_A)))
		{
			g_On = TRUE;
		}

		if ((GetKeyboardPress(DIK_9)) || (IsButtonPressed(0, BUTTON_A)))
		{
			g_Move += 0.01f;
		}
		if ((GetKeyboardPress(DIK_8)) || (IsButtonPressed(0, BUTTON_A)))
		{
			g_Move -= 0.01f;
		}


		// パーティクル発生
		if (g_On)
		{
			// 少しずつ半径を大きくしていく
			if((g_Move < 1.0f) && (g_MoveOn)) g_Move += 0.006f;

			for (int p = 0; p < 12; p++)
			{
				for (int i = 0; i < 20; i++)
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

					//if (g_MoveOn)
					//{
					//	move.x *= sinf(g_aBeamParticle[0].time + (XM_2PI / 20) * i);
					//	move.z *= cosf(g_aBeamParticle[0].time * (p + 1) / 4 + (XM_2PI / 20) * i);
					//	move.y *= cosf(g_aBeamParticle[0].time * (p + 1) / 2 + (XM_2PI / 20) * i);
					//}
					//else
					{
						move.x *= sinf(g_aBeamParticle[0].time + (XM_2PI / 20) * i);
						move.z *= cosf(g_aBeamParticle[0].time + (XM_2PI / 20) * i);
						move.y *= cosf(g_aBeamParticle[0].time + (XM_2PI / 20) * i);
					}

					base = move;

					move.x += (float)(rand() % 70 - 50) / 1000.0f;
					move.z += (float)(rand() % 70 - 50) / 1000.0f;
					move.y += (float)(rand() % 70 - 50) / 1000.0f;

					nLife = rand() & 40 + 20;

					fSize = RamdomFloat(3, 0.5f, 0.3f);

					color.x = (float)(rand() % 60 + 40) / 100.0f;
					color.y = (float)(rand() % 60 + 40) / 100.0f;
					color.z = (float)(rand() % 10 + 90) / 100.0f;


					// ビルボードの設定
					SetBeamParticle(pos, move, base, XMFLOAT4(color.x, color.y, color.z, 1.0f), fSize, fSize, nLife);
				}
			}
			//g_End = TRUE;
		}
	}
	else
	{
		XMFLOAT3 target = { 0.0f ,0.0f ,0.0f };
		target.x = bahamut->pos.x + BLAST_X_OFFSET + 585.0f;
		target.y = -160.0f;
		for (int i = 0; i < MAX_BEAM_PARTICLE; i++)
		{
			if (g_aBeamParticle[i].bUse)
			{// 使用中
				// 移動前の座標を保存
				XMVECTOR pos_old = XMLoadFloat3(&g_aBeamParticle[i].pos);

				// ベジェ曲線の処理
				XMVECTOR p0 = XMLoadFloat3(&g_aBeamParticle[i].ControlPos0);
				XMVECTOR p1 = XMLoadFloat3(&g_aBeamParticle[i].ControlPos1);
				XMVECTOR p2 = XMLoadFloat3(&g_aBeamParticle[i].ControlPos2);
				XMVECTOR ta = XMLoadFloat3(&target);
				XMVECTOR pos;

				float t = g_aBeamParticle[i].time;
				float t1 = 1.0f - g_aBeamParticle[i].time;

				pos = ((t1 * t1 * t1) * p0)
					+ (3 * t * t1 * t1 * p1)
					+ (3 * t * t * t1 * p2)
					+ (t * t * t * ta);

				XMStoreFloat3(&g_aBeamParticle[i].pos, pos);

				XMStoreFloat3(&g_aBeamParticle[i].distance, pos - pos_old);

				//	時間を進める
				g_aBeamParticle[i].time += BEAM_SPEED;

				// 目的地に着いたら消す
				if (g_aBeamParticle[i].time >= 1.0f) g_aBeamParticle[i].bUse = FALSE;		// 移動が終わったら消す
			}
		}

		// g_On = FALSE;
		// 翼が開いたタイミングでパーティクルを発生させる
		if ((GetKeyboardPress(DIK_C)) || (IsButtonPressed(0, BUTTON_A)))
		{
			SetSummonScene(charge);
			//g_On = TRUE;

			//SetBeamOrbitSwitch(TRUE);
		}

		// パーティクル発生
		if (g_On)
		{
			for( int p = 0 ; p < 11 ; p++ )
			{
				XMFLOAT3 pos, posbase, conpos1, conpos1base, conpos2, conpos2base;
				XMFLOAT4 color;
				float fSize;

				BAHAMUT *bahamut = GetBahamut();

				pos = bahamut->pos;

				// 口元に合わせる
				pos.y += 125.0f;
				pos.x += 85.0f;

				posbase = pos;

				// 制御点の初期化
				conpos1 = conpos2 = { 0.0f, 0.0f ,0.0f };

				// 制御点の基点をセット
				conpos1.x = pos.x + ((BLAST_X_OFFSET + 100.0f) / 2);
				conpos1.y = pos.y / 2;

				conpos1base = conpos1;

				conpos2.x = pos.x + (BLAST_X_OFFSET + 100.0f);
				conpos2.y = 0.0f;

				conpos2base = conpos2;

				// 制御点のランダム化
				pos.x += CONPOS_RADIUS / 7 * ((float)(rand() % 200 - 100) / 100.0f);
				pos.y += CONPOS_RADIUS / 7 * ((float)(rand() % 200 - 100) / 100.0f);
				pos.z += CONPOS_RADIUS / 7 * ((float)(rand() % 200 - 100) / 100.0f);

				conpos1.x += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
				conpos1.y += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
				conpos1.z += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);

				conpos2.x += CONPOS_RADIUS * 2 * ((float)(rand() % 200 - 100) / 100.0f);
				conpos2.y += CONPOS_RADIUS * 2 * ((float)(rand() % 200 - 100) / 100.0f);
				conpos2.z += CONPOS_RADIUS * 2 * ((float)(rand() % 200 - 100) / 100.0f);

				// サイズのランダム化
				//fSize = RamdomFloat(2, 1.5f, 0.5f);
				fSize = (float)(rand() % 50 + 50) / 100.0f;

				// 色のランダム化
				color.x = (float)(rand() % 60 + 40) / 100.0f;
				color.y = (float)(rand() % 60 + 40) / 100.0f;
				color.z = (float)(rand() % 10 + 90) / 100.0f;
				color.w = 1.0f;
				// パーティクルをセット
				SetBezierParticle(pos, posbase, conpos1, conpos1base, conpos2, conpos2base, color, fSize);
			}
		}
	}

#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("life:%d \n", g_aBeamParticle[100].nLife);
	PrintDebugProc("bezier.x:%f \n", g_aBeamParticle[0].pos.x);
	PrintDebugProc("beam count : %d\n", g_count);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBeamParticle(void)
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
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	//for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE; nCntParticle++)
	//{
	//	if (g_aBeamParticle[nCntParticle].bUse)
	//	{
	//		// ワールドマトリックスの初期化
	//		mtxWorld = XMMatrixIdentity();

	//		// ビューマトリックスを取得
	//		mtxView = XMLoadFloat4x4(&cam->mtxView);

	//		// 転置行列処理
	//		mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
	//		mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
	//		mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

	//		mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
	//		mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
	//		mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

	//		mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
	//		mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
	//		mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

	//		// スケールを反映
	//		mtxScl = XMMatrixScaling(g_aBeamParticle[nCntParticle].scale.x, g_aBeamParticle[nCntParticle].scale.y, g_aBeamParticle[nCntParticle].scale.z);
	//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	//		// 移動を反映
	//		mtxTranslate = XMMatrixTranslation(g_aBeamParticle[nCntParticle].pos.x, g_aBeamParticle[nCntParticle].pos.y, g_aBeamParticle[nCntParticle].pos.z);
	//		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	//		// ワールドマトリックスの設定
	//		SetWorldMatrix(&mtxWorld);

	//		// マテリアル設定
	//		SetMaterial(g_aBeamParticle[nCntParticle].material);

	//		// ポリゴンの描画
	//		GetDeviceContext()->Draw(4, 0);
	//	}
	//}

	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE; nCntParticle++)
	{
		if (g_aBeamParticle[nCntParticle].bUse)
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
			mtxScl = XMMatrixScaling(g_aBeamParticle[nCntParticle].scale.x, g_aBeamParticle[nCntParticle].scale.y, g_aBeamParticle[nCntParticle].scale.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aBeamParticle[nCntParticle].pos.x, g_aBeamParticle[nCntParticle].pos.y, g_aBeamParticle[nCntParticle].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			// マテリアル設定
			SetMaterial(g_aBeamParticle[nCntParticle].material);

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
HRESULT MakeVertexBeamParticle(void)
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
void SetColorBeamParticle(int nIdxParticle, XMFLOAT4 col)
{
	g_aBeamParticle[nIdxParticle].material.Diffuse = col;
}

//=============================================================================
// チャージ時のパーティクルの発生処理
//=============================================================================
int SetBeamParticle(XMFLOAT3 pos, XMFLOAT3 move, XMFLOAT3 base, XMFLOAT4 col, float fSizeX, float fSizeY, int nLife)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE; nCntParticle++)
	{
		if (!g_aBeamParticle[nCntParticle].bUse)
		{
			g_aBeamParticle[nCntParticle].pos = pos;
			g_aBeamParticle[nCntParticle].scale = { fSizeX, fSizeX, fSizeX };
			g_aBeamParticle[nCntParticle].move = move;
			g_aBeamParticle[nCntParticle].material.Diffuse = col;
			g_aBeamParticle[nCntParticle].fSizeX = fSizeX;
			g_aBeamParticle[nCntParticle].fSizeY = fSizeY;
			g_aBeamParticle[nCntParticle].nLife = nLife;
			g_aBeamParticle[nCntParticle].bUse = TRUE;

			g_aBeamParticle[nCntParticle].time = (float)(rand() % 1000 - 500) / 1000.0f;
			g_aBeamParticle[nCntParticle].baseMove = base;

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
int SetBezierParticle(XMFLOAT3 pos, XMFLOAT3 posbase, XMFLOAT3 conpos1, XMFLOAT3 conpos1base, XMFLOAT3 conpos2, XMFLOAT3 conpos2base, XMFLOAT4 col, float fSize)
{
	int nIdxParticle = -1;

	for (int nCntParticle = 0; nCntParticle < MAX_BEAM_PARTICLE; nCntParticle++)
	{
		if (!g_aBeamParticle[nCntParticle].bUse)
		{
			g_aBeamParticle[nCntParticle].pos = pos;
			g_aBeamParticle[nCntParticle].distance = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_aBeamParticle[nCntParticle].scale = { fSize, fSize, fSize };
			g_aBeamParticle[nCntParticle].material.Diffuse = col;
			g_aBeamParticle[nCntParticle].bUse = TRUE;
			g_aBeamParticle[nCntParticle].first = TRUE;

			// 0番はまっすぐの直線
			if (nCntParticle == 0)
			{
				g_aBeamParticle[nCntParticle].ControlPos0 = posbase;
				g_aBeamParticle[nCntParticle].ControlPos1 = conpos1base;
				g_aBeamParticle[nCntParticle].ControlPos2 = conpos2base;
			}
			else
			{
				g_aBeamParticle[nCntParticle].ControlPos0 = pos;
				g_aBeamParticle[nCntParticle].ControlPos1 = conpos1;
				g_aBeamParticle[nCntParticle].ControlPos2 = conpos2;
			}

			g_aBeamParticle[nCntParticle].time = 0.0f;

			nIdxParticle = nCntParticle;

			break;
		}
	}

	return nIdxParticle;
}

//=============================================================================
// パーティクルの発生スイッチ
//=============================================================================
void SetBeamParticleSwitch(BOOL data)
{
	g_On = data;
}


//=============================================================================
// パーティクルの種類スイッチ(TRUE: チャージ　FALSE:ビーム)
//=============================================================================
void SetBeamParticleCharge(BOOL data)
{
	g_Charge = data;
}



//=============================================================================
// パーティクルの座標を取得
//=============================================================================
BEAM_PARTICLE *GetBeamParticle(void)
{
	return &g_aBeamParticle[0];
}


//=============================================================================
// パーティクルの一斉削除と停止
//=============================================================================
void SetBeamDelete(void)
{
	for (int i = 0; i < MAX_BEAM_PARTICLE; i++)
	{
		g_aBeamParticle[i].bUse = FALSE;
	}

	g_On = FALSE;
	g_Move = 0.0f;
	g_MoveOn = TRUE;
}


//=============================================================================
// チャージ時の半径を変更
//=============================================================================
void SetChargeRadius(float radius)
{
	g_Move = radius;
	g_MoveOn = FALSE;
}


//=============================================================================
// ブレスのパーティクルを使用してるか
//=============================================================================
BOOL GetBeamOnOff(void)
{
	return g_On;
}