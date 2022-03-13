//=============================================================================
//
// ライトニング処理 [lightning.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "debugproc.h"
#include "camera.h"
#include "model.h"
#include "lightning.h"
#include "player.h"
#include "sound.h"
#include "tutorial.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX				(1)			// テクスチャの数

#define	LIGHTNING_SIZE_X		(2.0f)		// 頂点サイズ
#define	LIGHTNING_SIZE_Y		(2.0f)		// 頂点サイズ
#define	VALUE_MOVE_LIGHTNING	(3.0f)		// 移動速度

#define	LIGHTNING_SET_Y			(150.0f)	// ライトニングの開始高度
#define LIGHTNING_DISTANCE		(150.0f)	// ライトニングをどのくらい離して発生させるか

#define	LIGHTNING_SET_X			(90.0f)		// ライトニングの当たり判定のX方向の辺の長さ
#define	LIGHTNING_SET_Z			(90.0f)		// ライトニングの当たり判定のZ方向の辺の長さ

#define LIGHTNING_SPEED			(25)		// 1フレームで進むスピード

#define BIFURCATION_COUNT		(25)		// 分岐までのカウント

#define LIGHTNING_LIFE			(30)		// ライトニングの表示時間
#define LIGHTNING_A_SUB			(0.1f)		// α値の減算速度

#define	DISP_SHADOW				// 影の表示
//#undef DISP_SHADOW


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexLightning(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;		// 頂点バッファ

static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static int							g_TexNo;					// テクスチャ番号

								// ライトニングワーク x:分岐までのカウント y分岐した枝
static LIGHTNING				g_Lightning[MAX_LIGHTNING][MAX_REFRACTION][BIFURCATION_COUNT];
static LIGHTNING_COLLI			g_LightningCollition[MAX_LIGHTNING];		// 当たり判定用の入れ物

static XMFLOAT3					g_posBase;						// ビルボード発生位置
static float					g_fWidthBase = 5.0f;			// 基準の幅
static float					g_fHeightBase = 10.0f;			// 基準の高さ
static float					g_roty = 0.0f;					// 移動方向
static float					g_spd = 0.0f;					// 移動スピード

static XMFLOAT3					g_vector[MAX_LIGHTNING][MAX_REFRACTION];		// 移動方向のベクトル
static XMFLOAT3					g_scale[MAX_LIGHTNING][MAX_REFRACTION];			// スケール
static int						g_vecType[MAX_LIGHTNING + 1];					// 現在のベクトルタイプの最大数
static int						g_bifurcation[MAX_LIGHTNING];					// 分岐回数

static int						g_SE = 0;						// SEの番号を管理
static BOOL						g_LightningOnOff;				// 雷魔法を使用しているかどうか

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/bloom.png",
};

static BOOL						g_Load = FALSE;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitLightning(void)
{
	// 頂点情報の作成
	MakeVertexLightning();

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

	// ライトニングワークの初期化
	for (int z = 0; z < MAX_LIGHTNING; z++)
	{
		for (int y = 0; y < MAX_REFRACTION; y++)
		{
			for (int x = 0; x < BIFURCATION_COUNT; x++)
			{
				g_Lightning[z][y][x].pos = XMFLOAT3(0.0f, LIGHTNING_SET_Y, -10.0f);
				g_Lightning[z][y][x].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
				g_Lightning[z][y][x].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
				g_Lightning[z][y][x].move = XMFLOAT3(1.0f, 1.0f, 1.0f);

				ZeroMemory(&g_Lightning[z][y][x].material, sizeof(g_Lightning[z][y][x].material));
				g_Lightning[z][y][x].material.Diffuse = XMFLOAT4(0.8f, 0.7f, 0.3f, 0.6f);

				g_Lightning[z][y][x].fSizeX = LIGHTNING_SIZE_X;
				g_Lightning[z][y][x].fSizeY = LIGHTNING_SIZE_Y;
				g_Lightning[z][y][x].nIdxShadow = -1;
				g_Lightning[z][y][x].nLife = LIGHTNING_LIFE;
				g_Lightning[z][y][x].bUse = FALSE;

				g_Lightning[z][y][x].vecType = 0;
				g_Lightning[z][y][x].count = 0;
			}

			g_scale[z][y] = { 1.0f, 1.0f,1.0f };
		}

		g_Lightning[z][0][0].bUse = FALSE;
		g_vecType[z] = 0;
		g_bifurcation[z] = 0;
		g_vector[z][0] = { 0.3f, -0.5f, 0.2f };
		g_scale[z][0] = { 1.0f, 1.0f,1.0f };

	}

	for (int i = 0; i < MAX_LIGHTNING; i++)
	{
		g_LightningCollition[i].pos = XMFLOAT3(0.0f, LIGHTNING_SET_Y, -10.0f);
		g_LightningCollition[i].bUse = FALSE;
		g_LightningCollition[i].fSizeY = LIGHTNING_SET_Y * 2;
		g_LightningCollition[i].fSizeX = LIGHTNING_SET_X;
		g_LightningCollition[i].fSizeZ = LIGHTNING_SET_Z;
	}

	// 変数の初期化
	g_posBase = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_roty = 0.0f;
	g_spd = 0.0f;
	g_SE = 0;
	g_LightningOnOff = FALSE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitLightning(void)
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
void UpdateLightning(void)
{
	for (int z = 0; z < MAX_LIGHTNING; z++)
	{
		// 最初のライトニングがTRUEなら処理開始
		if (g_Lightning[z][0][0].bUse)
		{
			for (int i = 0; i < LIGHTNING_SPEED; i++)
			{
				for (int y = 0; y < g_vecType[z] + 1; y++)
				{
					SetUpdateLightning(z, y);
				}
			}
		}

		// データの更新
		g_LightningCollition[z].bUse = g_Lightning[z][0][0].bUse;
		g_LightningCollition[z].pos.x = g_Lightning[z][0][0].pos.x;
		g_LightningCollition[z].pos.z = g_Lightning[z][0][0].pos.z;
		g_LightningCollition[z].pos.y = 0.0f;

		// 表示寿命が0になったら終了処理開始
		for (int y = 0; y < MAX_REFRACTION; y++)
		{
			for (int x = 0; x < BIFURCATION_COUNT; x++)
			{
				if (g_Lightning[z][y][x].bUse)
				{
					g_Lightning[z][y][x].nLife--;
				}
				if ((g_Lightning[z][y][x].bUse) && (g_Lightning[z][y][x].nLife <= 0))
				{
					g_Lightning[z][y][x].material.Diffuse.w -= LIGHTNING_A_SUB;

					// 透明になったら未使用に
					if (g_Lightning[z][y][x].material.Diffuse.w > 0.0f) continue;

					ResetLightning(z);

					// チュートリアルクリア
					SetTutorialClear(tx_lightning);
				}
			}
		}


#ifdef _DEBUG	// デバッグ情報を表示する
		PrintDebugProc("g_vecType[%d]: %d\n", z, g_vecType[z]);
#endif

	}
	PrintDebugProc("g_vecType[%d]: %d\n", 4, g_vecType[4]);

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawLightning(void)
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

	for (int z = 0; z < MAX_LIGHTNING; z++)
	{
		for (int y = 0; y < MAX_REFRACTION; y++)
		{
			if (g_Lightning[z][y][0].bUse)
			{
				for (int x = 0; x < BIFURCATION_COUNT; x++)
				{
					if (g_Lightning[z][y][x].bUse)
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
						mtxScl = XMMatrixScaling(g_Lightning[z][y][x].scale.x, g_Lightning[z][y][x].scale.y, g_Lightning[z][y][x].scale.z);
						mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

						// 移動を反映
						mtxTranslate = XMMatrixTranslation(g_Lightning[z][y][x].pos.x, g_Lightning[z][y][x].pos.y, g_Lightning[z][y][x].pos.z);
						mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

						// ワールドマトリックスの設定
						SetWorldMatrix(&mtxWorld);

						// マテリアル設定
						SetMaterial(g_Lightning[z][y][x].material);

						// 加算合成に設定
						//SetBlendState(BLEND_MODE_ADD);

						// ポリゴンの描画
						GetDeviceContext()->Draw(4, 0);
					}
				}
			}
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
HRESULT MakeVertexLightning(void)
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
		vertex[0].Position = XMFLOAT3(-LIGHTNING_SIZE_X / 2, LIGHTNING_SIZE_Y / 2, 0.0f);
		vertex[1].Position = XMFLOAT3(LIGHTNING_SIZE_X / 2, LIGHTNING_SIZE_Y / 2, 0.0f);
		vertex[2].Position = XMFLOAT3(-LIGHTNING_SIZE_X / 2, -LIGHTNING_SIZE_Y / 2, 0.0f);
		vertex[3].Position = XMFLOAT3(LIGHTNING_SIZE_X / 2, -LIGHTNING_SIZE_Y / 2, 0.0f);

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
// ライトニングの情報を取得
//=============================================================================
LIGHTNING_COLLI *GetLightningCollition(void)
{
	return &g_LightningCollition[0];
}

//=============================================================================
// ライトニングの発生処理
//=============================================================================
void SetLightning(void)
{
	PLAYER *player = GetPlayer();
	g_LightningOnOff = TRUE;

	// ライトニングのすべて初期化
	for (int z = 0; z < MAX_LIGHTNING; z++)
	{
		if (g_Lightning[z][0][0].bUse == TRUE) continue;

		// 念のためもう一度初期化
		ResetLightning(z);

		// 最初の1つをセット
		g_Lightning[z][0][0].bUse = TRUE;

		// SEを再生
		g_SE++;

		switch (g_SE % 3)
		{
		case 0:
			PlaySound(SOUND_LABEL_SE_MAGIC_lightning01);
			break;

		case 1:
			PlaySound(SOUND_LABEL_SE_MAGIC_lightning02);
			break;

		case 2:
			PlaySound(SOUND_LABEL_SE_MAGIC_lightning03);
			break;
		}

		return;
	}
}

//=============================================================================
// ライトニングの発生処理
//=============================================================================
void SetUpdateLightning(int z, int vectype)
{
	vectype %= MAX_REFRACTION;

	for (int x = 0; x < BIFURCATION_COUNT; x++)
	{
		if (g_Lightning[z][vectype][x].bUse != TRUE)
		{
			XMFLOAT3 pos = g_Lightning[z][vectype][x - 1].pos;
			int vecType = g_Lightning[z][vectype][x - 1].vecType;
			int count = g_Lightning[z][vectype][x - 1].count;

			// 地面を突き抜けいないかを確認
			if (pos.y > 0.0f)
			{
				if (count < BIFURCATION_COUNT - 2)	// ライトニングをBIFURCATION_COUNT個連続で描画したか
				{
					// 進行方向のまま発生
					count++;

					// 使用している
					g_Lightning[z][vectype][x].bUse = TRUE;

					pos.x += g_vector[z][g_Lightning[z][vectype][0].vecType].x;
					pos.y += g_vector[z][g_Lightning[z][vectype][0].vecType].y;
					pos.z += g_vector[z][g_Lightning[z][vectype][0].vecType].z;

					// 新しいライトニングのポジションとして設定
					g_Lightning[z][vectype][x].pos = pos;
					g_Lightning[z][vectype][x].vecType = vecType;
					g_Lightning[z][vectype][x].scale = g_Lightning[z][vectype][0].scale;

					// countを構造体のcountへ代入
					g_Lightning[z][vectype][x].count = count;
				}
				else
				{
					// 分岐回数が最大まで行っていない場合、ランダムで分岐を行う
					if (g_bifurcation[z] < MAX_BIFURCATION)
					{
						g_bifurcation[z]++;

						// 分岐(今後ランダム方向へ)
						int random = (rand() % 4 + 1) / 2;
						if (random == 0) random = 1;
						for (int i = 0; i < random; i++)
						{
							// ベクトルタイプを増やす
							if (g_vecType[z] < MAX_REFRACTION)
							{
								g_vecType[z]++;

								// 使用している
								g_Lightning[z][vectype][x].bUse = TRUE;
								g_Lightning[z][g_vecType[z]][0].bUse = TRUE;

								// ベクトルのリセット
								g_vector[z][g_vecType[z]].x = RamdomFloat(3, 0.7f, -0.7f);
								g_vector[z][g_vecType[z]].z = RamdomFloat(3, 0.7f, -0.7f);
								g_vector[z][g_vecType[z]].y = RamdomFloat(3, -0.1f, -0.8f);

								// 雷の太さもランダムで設定
								g_scale[z][g_vecType[z]].x = g_scale[z][g_vecType[z]].y = RamdomFloat(3, 3.0f, 1.5f);

								pos.x += g_vector[z][g_vecType[z]].x;
								pos.y += g_vector[z][g_vecType[z]].y;
								pos.z += g_vector[z][g_vecType[z]].z;

								// 新しいライトニングのポジションとして設定
								g_Lightning[z][g_vecType[z]][0].pos = pos;
								g_Lightning[z][g_vecType[z]][0].vecType = g_vecType[z];
								g_Lightning[z][g_vecType[z]][0].scale = g_scale[z][g_vecType[z]];
							}
						}
					}
					else
					{	// 分岐を行わない
						// ベクトルタイプを増やす
						if (g_vecType[z] < MAX_REFRACTION)
						{
							g_vecType[z]++;

							// 使用している
							g_Lightning[z][vectype][x].bUse = TRUE;
							g_Lightning[z][g_vecType[z]][0].bUse = TRUE;

							// ベクトルのリセット
							g_vector[z][g_vecType[z]].x = RamdomFloat(3, 0.7f, -0.7f);
							g_vector[z][g_vecType[z]].z = RamdomFloat(3, 0.7f, -0.7f);
							g_vector[z][g_vecType[z]].y = RamdomFloat(3, -0.1f, -0.8f);

							// 雷の太さもランダムで設定
							g_scale[z][g_vecType[z]].x = g_scale[z][g_vecType[z]].y = RamdomFloat(3, 3.0f, 1.5f);

							pos.x += g_vector[z][g_vecType[z]].x;
							pos.y += g_vector[z][g_vecType[z]].y;
							pos.z += g_vector[z][g_vecType[z]].z;

							// 新しいライトニングのポジションとして設定
							g_Lightning[z][g_vecType[z]][0].pos = pos;
							g_Lightning[z][g_vecType[z]][0].vecType = g_vecType[z];
							g_Lightning[z][g_vecType[z]][0].scale = g_scale[z][g_vecType[z]];
						}
					}
				}
			}

			return;
		}
	}
}


//=============================================================================
// 雷魔法のリセット処理
//=============================================================================
void ResetLightning(int z)
{
	PLAYER *player = GetPlayer();

	for (int y = 0; y < MAX_REFRACTION; y++)
	{
		for (int x = 0; x < BIFURCATION_COUNT; x++)
		{
			g_Lightning[z][y][x].pos.x = player->pos.x;
			g_Lightning[z][y][x].pos.z = player->pos.z;
			g_Lightning[z][y][x].pos.y = LIGHTNING_SET_Y;

			g_Lightning[z][y][x].pos.x -= sinf(player->rot.y) * LIGHTNING_DISTANCE;
			g_Lightning[z][y][x].pos.z -= cosf(player->rot.y) * LIGHTNING_DISTANCE;

			g_Lightning[z][y][x].bUse = FALSE;
			g_Lightning[z][y][x].material.Diffuse.w = 1.0f;
			g_Lightning[z][y][x].vecType = 0;
			g_Lightning[z][y][x].count = 0;

			g_Lightning[z][y][x].nLife = LIGHTNING_LIFE;
		}
	}

	// ベクトルも初期化
	for (int i = 1; i < MAX_REFRACTION; i++)
	{
		g_vector[z][i] = { 0.0f,0.0f,0.0f };
	}

	g_vecType[z] = 0;
	g_bifurcation[z] = 0;
}


//=============================================================================
// float型で負数にも対応しているランダムの値を返す関数
// digits:小数点以下の桁数(0.01f → 2), max:欲しい最大値, min:欲しい最小値
//=============================================================================
float RamdomFloat(int digits, float max, float min)
{
	// 小数点を消すための変数を作成
	int l_digits = 1;	
	for (int i = 0; i < digits; i++)
	{
		l_digits *= 10;
	}

	// 剰余算するための値を作成
	float surplus = (max - min) * l_digits;

	int random = rand() % (int)surplus;

	float ans;

	// 整数のランダムの値をfloat型へキャストと負数の処理
	ans = (float)random / l_digits + min;

	return ans;
}


//=============================================================================
//	雷魔法を実行中かどうか
//=============================================================================
BOOL GetLightningOnOff(void)
{
	return g_LightningOnOff;
}


//=============================================================================
//	雷魔法のOnOffを管理
//=============================================================================
void SetLightningOnOff(BOOL data)
{
	g_LightningOnOff = data;
}