//=============================================================================
//
// 杖の雷処理 [wand_lightning.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "wand_lightning.h"
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

#define	BEAM_PARTICLE_SIZE_X		(1.0f)		// 頂点サイズ
#define	BEAM_PARTICLE_SIZE_Y		(1.0f)		// 頂点サイズ
#define	VALUE_MOVE_BEAM_PARTICLE	(0.2f)		// 移動速度


#define BEAM_PARTICLE_OFFSET		(100)		// バハムート中心からどのくらい離して発生させるか

#define BEAM_SPEED					(0.012f)	// ビームの進む速度
#define BEAM_INTARVAL				(2)			// ビームの出る間隔

#define CONPOS_RADIUS				(10.0f)		// 制御点のランダム範囲

#define SIZE_DWON					(0.08f)		// サイズを小さくしていく値
#define SIZE_BASE					(1.4f)		// サイズを小さくしていく値

#define	DISP_SHADOW						// 影の表示
//#undef DISP_SHADOW


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexWandLightning(void);
int SetBezierParticle(XMFLOAT3 pos, XMFLOAT3 posbase, XMFLOAT3 conpos1, XMFLOAT3 conpos1base, XMFLOAT3 conpos2, XMFLOAT3 conpos2base, XMFLOAT4 col, float fSize);
//int SetBezierParticle(XMFLOAT3 pos, XMFLOAT3 conpos1, XMFLOAT3 conpos2, XMFLOAT3 conpos3, XMFLOAT4 col, float fSize);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

static WAND_LIGHTNING			g_aWandLightning;				// 杖の雷の位置管理
static WAND_ORBIT				g_aWandOrbit[MAX_WAND_REFRACTION][MAX_WAND_LIGHTNING];			// 杖の雷の軌跡
static XMFLOAT3					g_WandControlPos[7];			// 雷が屈折するポイント  0:スタート 6:エンド
static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 1.0f;			// 基準の幅
static float					g_fHeightBase = 1.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード
static int						g_Time;							// 経過時間

static int						g_count = 0;					// 何個のパーティクルを使用しているか超調査

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
HRESULT InitWandLightning(void)
{
	// 頂点情報の作成
	MakeVertexWandLightning();

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

	// 基準の初期化
	{
		g_aWandLightning.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aWandLightning.distance = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aWandLightning.ControlPos0 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aWandLightning.ControlPos1 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aWandLightning.ControlPos2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aWandLightning.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aWandLightning.scale = XMFLOAT3(1.5f, 1.5f, 1.5f);

		ZeroMemory(&g_aWandLightning.material, sizeof(g_aWandLightning.material));
		g_aWandLightning.material.Diffuse = XMFLOAT4(0.6f, 0.6f, 1.0f, 1.0f);

		g_aWandLightning.fSizeX = BEAM_PARTICLE_SIZE_X;
		g_aWandLightning.fSizeY = BEAM_PARTICLE_SIZE_Y;
		g_aWandLightning.nLife = 0;
		g_aWandLightning.bUse = TRUE;
	}


	// パーティクルの初期化
	for (int r = 0; r < MAX_WAND_REFRACTION; r++)
	{
		for (int nCntParticle = 0; nCntParticle < MAX_WAND_LIGHTNING; nCntParticle++)
		{
			g_aWandOrbit[r][nCntParticle].bUse = FALSE;
			g_aWandOrbit[r][nCntParticle].fSizeX = 1.0f;
			g_aWandOrbit[r][nCntParticle].fSizeY = 1.0f;
			g_aWandOrbit[r][nCntParticle].nLife = 0;

			g_aWandOrbit[r][nCntParticle].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_aWandOrbit[r][nCntParticle].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_aWandOrbit[r][nCntParticle].scale = XMFLOAT3(0.0f, 0.0f, 0.0f);

			ZeroMemory(&g_aWandOrbit[r][nCntParticle].material, sizeof(g_aWandOrbit[r][nCntParticle].material));
			g_aWandOrbit[r][nCntParticle].material.Diffuse = XMFLOAT4(0.6f, 0.6f, 1.0f, 1.0f);
		}
	}


	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;
	g_Move = 0.0f;
	g_Charge = TRUE;
	g_MoveOn = TRUE;
	g_Time = 0;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitWandLightning(void)
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
void UpdateWandLightning(void)
{
	PLAYER *player = GetPlayer();

	if ((GetKeyboardPress(DIK_C)) || (IsButtonPressed(0, BUTTON_A)))
	{
		SetSummonScene(charge);
		SetSummon();
		//g_On = TRUE;

		//SetBeamOrbitSwitch(TRUE);
	}

	// 一旦全部消す
	for (int r = 0; r < MAX_WAND_REFRACTION; r++)
	{
		for (int nCntParticle = 0; nCntParticle < MAX_WAND_LIGHTNING; nCntParticle++)
		{
			g_aWandOrbit[r][nCntParticle].bUse = FALSE;
		}
	}

	// 雷魔法を実行していなければここで終了
	if (!GetLightningOnOff()) return;

	// 杖の雷の位置情報を管理
	{
		for (int i = 0; i < MAX_WAND_LIGHTNING; i++)
		{
			if (g_aWandLightning.bUse)
			{// 使用中
				g_aWandLightning.pos = player->pos;
				g_WandControlPos[0] = player->pos;
				g_WandControlPos[6] = player->pos;

				// スタートポジション
				g_WandControlPos[0].x -= sinf(player->rot.y) * 23.0f;
				g_WandControlPos[0].z -= cosf(player->rot.y) * 23.0f;
				g_WandControlPos[0].y += 38.0f;


				// エンドポジション
				g_WandControlPos[6].x -= sinf(player->rot.y + 1.2f) * 16.5f;
				g_WandControlPos[6].z -= cosf(player->rot.y + 1.2f) * 16.5f;
				g_WandControlPos[6].y -= 3.0f;

				g_aWandLightning.pos.x -= sinf(player->rot.y + 1.2f) * 16.5f;
				g_aWandLightning.pos.z -= cosf(player->rot.y + 1.2f) * 16.5f;
				g_aWandLightning.pos.y -= 3.0f;
			}
		}

		// 時間を進める
		g_Time++;

		// 更新
		if(g_Time % 5 == 0)
		{
			XMFLOAT3 pos, conpos1, conpos2, conpos3, conpos4, conpos5;
			XMFLOAT4 color;
			float fSize;

			pos = player->pos;

			// 制御点の初期化
			conpos1 = conpos2 = conpos3 = conpos4 = conpos5 = { 0.0f, 0.0f ,0.0f };

			// 制御点のランダム化
			conpos1.x += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos1.y += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos1.z += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);

			conpos2.x += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos2.y += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos2.z += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);

			conpos3.x += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos3.y += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos3.z += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);

			conpos4.x += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos4.y += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos4.z += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);

			conpos5.x += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos5.y += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);
			conpos5.z += CONPOS_RADIUS * ((float)(rand() % 200 - 100) / 100.0f);

			// サイズのランダム化
			fSize = 1.0f;

			// 色のランダム化
			color.x = (float)(rand() % 10 + 10) / 100.0f;
			color.y = (float)(rand() % 10 + 10) / 100.0f;
			color.z = (float)(rand() % 30 + 300) / 100.0f;
			color.w = 1.0f;

			// パーティクルをセット
			SetWandLightning(pos, conpos1, conpos2, conpos3, conpos4, conpos5, color, fSize);
		}
	}







	// 軌跡を描くパーティクルの管理
	{
		// 屈折位置の基礎座標を算出
		// スタート座標からエンド座標までの間を均等に6等分
		XMVECTOR sPos, ePos, conPos1, conPos2, conPos3, conPos4, conPos5;
		XMVECTOR conPos1rand, conPos2rand, conPos3rand, conPos4rand, conPos5rand;
		sPos = XMLoadFloat3(&g_WandControlPos[0]);
		ePos = XMLoadFloat3(&g_WandControlPos[6]);
		conPos1rand = XMLoadFloat3(&g_aWandLightning.ControlPos1);
		conPos2rand = XMLoadFloat3(&g_aWandLightning.ControlPos2);
		conPos3rand = XMLoadFloat3(&g_aWandLightning.ControlPos3);
		conPos4rand = XMLoadFloat3(&g_aWandLightning.ControlPos4);
		conPos5rand = XMLoadFloat3(&g_aWandLightning.ControlPos5);

		conPos1 = ((ePos - sPos) / 6.0f) * 1.0f + sPos + conPos1rand;
		conPos2 = ((ePos - sPos) / 6.0f) * 2.0f + sPos + conPos2rand;
		conPos3 = ((ePos - sPos) / 6.0f) * 3.0f + sPos + conPos3rand;
		conPos4 = ((ePos - sPos) / 6.0f) * 4.0f + sPos + conPos4rand;
		conPos5 = ((ePos - sPos) / 6.0f) * 5.0f + sPos + conPos5rand;

		XMStoreFloat3(&g_WandControlPos[1], conPos1);
		XMStoreFloat3(&g_WandControlPos[2], conPos2);
		XMStoreFloat3(&g_WandControlPos[3], conPos3);
		XMStoreFloat3(&g_WandControlPos[4], conPos4);
		XMStoreFloat3(&g_WandControlPos[5], conPos5);


		for (int y = 0; y < MAX_WAND_REFRACTION; y++)
		{
			for (int nCntParticle = 0; nCntParticle < MAX_WAND_LIGHTNING; nCntParticle++)
			{
				XMVECTOR dis, tempPos;
				XMFLOAT3 pos;
				float size = 0.0f;

				// 位置調整

				switch (y)
				{	// 差を少しずつ埋める
				case 0:
					dis = XMLoadFloat3(&g_WandControlPos[1]);
					dis -= sPos;

					dis = dis / (float)MAX_WAND_LIGHTNING * (float)nCntParticle;

					dis += sPos;

					XMStoreFloat3(&pos, dis);

					// 下に行くにつれて少しずつ小さくなっていく
					size = SIZE_BASE - SIZE_DWON * y;

					break;

				case 1:
					dis = XMLoadFloat3(&g_WandControlPos[2]);
					tempPos = XMLoadFloat3(&g_WandControlPos[1]);
					dis -= tempPos;

					dis = dis / (float)MAX_WAND_LIGHTNING * (float)nCntParticle;

					dis += tempPos;

					XMStoreFloat3(&pos, dis);

					// 下に行くにつれて少しずつ小さくなっていく
					size = SIZE_BASE - SIZE_DWON * y;

					break;

				case 2:
					dis = XMLoadFloat3(&g_WandControlPos[3]);
					tempPos = XMLoadFloat3(&g_WandControlPos[2]);
					dis -= tempPos;

					dis = dis / (float)MAX_WAND_LIGHTNING * (float)nCntParticle;

					dis += tempPos;

					XMStoreFloat3(&pos, dis);

					// 下に行くにつれて少しずつ小さくなっていく
					size = SIZE_BASE - SIZE_DWON * y;

					break;

				case 3:
					dis = XMLoadFloat3(&g_WandControlPos[4]);
					tempPos = XMLoadFloat3(&g_WandControlPos[3]);
					dis -= tempPos;

					dis = dis / (float)MAX_WAND_LIGHTNING * (float)nCntParticle;

					dis += tempPos;

					XMStoreFloat3(&pos, dis);

					// 下に行くにつれて少しずつ小さくなっていく
					size = SIZE_BASE - SIZE_DWON * y;

					break;

				case 4:
					dis = XMLoadFloat3(&g_WandControlPos[5]);
					tempPos = XMLoadFloat3(&g_WandControlPos[4]);
					dis -= tempPos;

					dis = dis / (float)MAX_WAND_LIGHTNING * (float)nCntParticle;

					dis += tempPos;

					XMStoreFloat3(&pos, dis);

					// 下に行くにつれて少しずつ小さくなっていく
					size = SIZE_BASE - SIZE_DWON * y;

					break;

				case 5:
					dis = XMLoadFloat3(&g_WandControlPos[5]);
					dis -= ePos;

					dis = dis / (float)MAX_WAND_LIGHTNING * (float)nCntParticle;

					dis += ePos;

					XMStoreFloat3(&pos, dis);

					// 下に行くにつれて少しずつ小さくなっていく
					size = SIZE_BASE - SIZE_DWON * y;

					break;

				}

				SetWandOrbit(pos, g_aWandLightning.material.Diffuse, size, size);
			}
		}
	}



#ifdef _DEBUG	// デバッグ情報を表示する
	//PrintDebugProc("life:%d \n", g_aWandLightning[100].nLife);
	//PrintDebugProc("bezier.x:%f \n", g_aWandLightning[0].pos.x);
	//PrintDebugProc("beam count : %d\n", g_count);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawWandLightning(void)
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

	for (int r = 0; r < MAX_WAND_REFRACTION; r++)
	{
		for (int nCntParticle = 0; nCntParticle < MAX_WAND_LIGHTNING; nCntParticle++)
		{
			if (g_aWandOrbit[r][nCntParticle].bUse)
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
				mtxScl = XMMatrixScaling(g_aWandOrbit[r][nCntParticle].scale.x, g_aWandOrbit[r][nCntParticle].scale.y, g_aWandOrbit[r][nCntParticle].scale.z);
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

				// 移動を反映
				mtxTranslate = XMMatrixTranslation(g_aWandOrbit[r][nCntParticle].pos.x, g_aWandOrbit[r][nCntParticle].pos.y, g_aWandOrbit[r][nCntParticle].pos.z);
				mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

				// ワールドマトリックスの設定
				SetWorldMatrix(&mtxWorld);

				// マテリアル設定
				SetMaterial(g_aWandOrbit[r][nCntParticle].material);

				// ポリゴンの描画
				GetDeviceContext()->Draw(4, 0);
				// ポリゴンの描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}

	//if (g_aWandLightning.bUse)
	//{
	//	// ワールドマトリックスの初期化
	//	mtxWorld = XMMatrixIdentity();

	//	// ビューマトリックスを取得
	//	mtxView = XMLoadFloat4x4(&cam->mtxView);

	//	// 転置行列処理
	//	mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
	//	mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
	//	mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

	//	mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
	//	mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
	//	mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

	//	mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
	//	mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
	//	mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

	//	// スケールを反映
	//	mtxScl = XMMatrixScaling(g_aWandLightning.scale.x, g_aWandLightning.scale.y, g_aWandLightning.scale.z);
	//	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	//	// 移動を反映
	//	mtxTranslate = XMMatrixTranslation(g_aWandLightning.pos.x, g_aWandLightning.pos.y, g_aWandLightning.pos.z);
	//	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	//	// ワールドマトリックスの設定
	//	SetWorldMatrix(&mtxWorld);

	//	// マテリアル設定
	//	SetMaterial(g_aWandLightning.material);

	//	// ポリゴンの描画
	//	GetDeviceContext()->Draw(4, 0);
	//	// ポリゴンの描画
	//	GetDeviceContext()->Draw(4, 0);
	//}


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
HRESULT MakeVertexWandLightning(void)
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
void SetColorWandLightning(int nIdxParticle, XMFLOAT4 col)
{
	g_aWandLightning.material.Diffuse = col;
}

//=============================================================================
// 軌跡のパーティクルの発生処理
//=============================================================================
int SetWandOrbit(XMFLOAT3 pos, XMFLOAT4 col, float fSizeX, float fSizeY)
{
	int nIdxParticle = -1;

	for (int r = 0; r < MAX_WAND_REFRACTION; r++)
	{
		for (int nCntParticle = 0; nCntParticle < MAX_WAND_LIGHTNING; nCntParticle++)
		{
			if (!g_aWandOrbit[r][nCntParticle].bUse)
			{
				g_aWandOrbit[r][nCntParticle].pos = pos;
				g_aWandOrbit[r][nCntParticle].scale = { fSizeX, fSizeX, fSizeX };
				g_aWandOrbit[r][nCntParticle].material.Diffuse = col;
				g_aWandOrbit[r][nCntParticle].fSizeX = fSizeX;
				g_aWandOrbit[r][nCntParticle].fSizeY = fSizeY;
				g_aWandOrbit[r][nCntParticle].bUse = TRUE;

				nIdxParticle = nCntParticle;

				if (g_count < nCntParticle) g_count = nCntParticle;

				return nIdxParticle;
			}
		}
	}

	return nIdxParticle;
}

//=============================================================================
// 基礎情報の更新処理
//=============================================================================
int SetWandLightning(XMFLOAT3 pos, XMFLOAT3 conpos1, XMFLOAT3 conpos2, XMFLOAT3 conpos3, XMFLOAT3 conpos4, XMFLOAT3 conpos5, XMFLOAT4 col, float fSize)
{
	int nIdxParticle = -1;

	g_aWandLightning.pos = pos;
	g_aWandLightning.distance = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_aWandLightning.scale = { fSize, fSize, fSize };
	g_aWandLightning.material.Diffuse = col;
	g_aWandLightning.bUse = TRUE;

	g_aWandLightning.ControlPos0 = pos;
	g_aWandLightning.ControlPos1 = conpos1;
	g_aWandLightning.ControlPos2 = conpos2;
	g_aWandLightning.ControlPos3 = conpos3;
	g_aWandLightning.ControlPos4 = conpos4;
	g_aWandLightning.ControlPos5 = conpos5;

	return nIdxParticle;
}

//=============================================================================
// パーティクルの発生スイッチ
//=============================================================================
void SetWandLightningSwitch(BOOL data)
{
	g_On = data;
}



//=============================================================================
// パーティクルの座標を取得
//=============================================================================
WAND_LIGHTNING *GetWandLightning(void)
{
	return &g_aWandLightning;
}


////=============================================================================
//// パーティクルの一斉削除と停止
////=============================================================================
//void SetBeamDelete(void)
//{
//	for (int i = 0; i < MAX_WAND_LIGHTNING; i++)
//	{
//		g_aWandLightning[i].bUse = FALSE;
//	}
//
//	g_On = FALSE;
//	g_Move = 0.0f;
//	g_MoveOn = TRUE;
//}


////=============================================================================
//// チャージ時の半径を変更
////=============================================================================
//void SetChargeRadius(float radius)
//{
//	g_Move = radius;
//	g_MoveOn = FALSE;
//}
//
//
////=============================================================================
//// ブレスのパーティクルを使用してるか
////=============================================================================
//BOOL GetBeamOnOff(void)
//{
//	return g_On;
//}