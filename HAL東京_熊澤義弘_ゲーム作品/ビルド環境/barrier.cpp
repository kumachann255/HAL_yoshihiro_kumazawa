//=============================================================================
//
// チュートリアル用バリア処理 [barrier.cpp]
// Author : GP11A132 10 熊澤義弘
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "barrier.h"
#include "tutorial.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX			(1)				// テクスチャの数

#define	BARRIER_WIDTH		(240.0f)		// 頂点サイズ
#define	BARRIER_HEIGHT		(240.0f)		// 頂点サイズ

#define DISSOLVE_SPEED		(0.005f)			// ディゾルブの速度

//*****************************************************************************
// 構造体定義
//*****************************************************************************
typedef struct
{
	XMFLOAT3	pos;			// 位置
	XMFLOAT3	scl;			// スケール
	MATERIAL	material;		// マテリアル
	float		fWidth;			// 幅
	float		fHeight;		// 高さ
	int			nIdxShadow;		// 影ID
	BOOL		bUse;			// 使用しているかどうか
	float		dissolve;		// ディゾルブの閾値

} BARRIER;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexBarrier(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;	// 頂点バッファ
static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static BARRIER				g_Barrier;			// バリアワーク
static int					g_TexNo;			// テクスチャ番号
static BOOL					g_bAlpaTest;		// アルファテストON/OFF
//static int				g_nAlpha;			// アルファテストの閾値

static BOOL					g_Load = FALSE;

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/barrier_plate.png",
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBarrier(void)
{
	MakeVertexBarrier();

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	g_TexNo = 0;

	// バリアワークの初期化
	ZeroMemory(&g_Barrier.material, sizeof(g_Barrier.material));
	g_Barrier.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	g_Barrier.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Barrier.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Barrier.fWidth = BARRIER_WIDTH;
	g_Barrier.fHeight = BARRIER_HEIGHT;
	g_Barrier.bUse = FALSE;
	g_Barrier.dissolve = 1.0f;

	g_bAlpaTest = TRUE;
	//g_nAlpha = 0x0;

	// バリアの設定
	SetBarrier(XMFLOAT3(195.0f, BARRIER_HEIGHT / 2.0f, 0.0f), BARRIER_WIDTH, BARRIER_HEIGHT, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBarrier(void)
{
	if (g_Load == FALSE) return;

	for(int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if(g_Texture[nCntTex] != NULL)
		{// テクスチャの解放
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	if(g_VertexBuffer != NULL)
	{// 頂点バッファの解放
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateBarrier(void)
{
	// チュートリアルクリアが終了している場合は消す
	if (GetTutorialClear(tx_clear)) g_Barrier.bUse = FALSE;

	// ディゾルブ処理
	if ((g_Barrier.bUse == FALSE) && (g_Barrier.dissolve > 0.0f))
	{
		g_Barrier.dissolve -= DISSOLVE_SPEED;
	}

#ifdef _DEBUG
	// アルファテストON/OFF
	if(GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? FALSE: TRUE;
	}

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBarrier(void)
{
	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// αテスト設定
	if (g_bAlpaTest == TRUE)
	{
		// αテストを有効に
		SetAlphaTestEnable(TRUE);
	}

	// ライティングを無効
	SetLightEnable(FALSE);

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// ビューマトリックスを取得
	mtxView = XMLoadFloat4x4(&cam->mtxView);

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Barrier.scl.x, g_Barrier.scl.y, g_Barrier.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Barrier.pos.x, g_Barrier.pos.y, g_Barrier.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	// ディゾルブの設定
	SetDissolve(g_Barrier.dissolve);

	// マテリアル設定
	SetMaterial(g_Barrier.material);

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	// ポリゴンの描画
	GetDeviceContext()->Draw(4, 0);

	// ライティングを有効に
	SetLightEnable(TRUE);

	// αテストを無効に
	SetAlphaTestEnable(FALSE);

	// カリング無効
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexBarrier(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// 頂点バッファに値をセットする
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = BARRIER_WIDTH;
	float fHeight = BARRIER_HEIGHT;

	// 頂点座標の設定
	vertex[0].Position = XMFLOAT3(fWidth / 2.0f,  fHeight / 2.0f, -fWidth / 2.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f,  fHeight / 2.0f,  fWidth / 2.0f);
	vertex[2].Position = XMFLOAT3(fWidth / 2.0f, -fHeight / 2.0f, -fWidth / 2.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, -fHeight / 2.0f,  fWidth / 2.0f);

	// 拡散光の設定
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャ座標の設定
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}

//=============================================================================
// バリアのパラメータをセット
//=============================================================================
int SetBarrier(XMFLOAT3 pos, float fWidth, float fHeight, XMFLOAT4 col)
{
	int nIdxTree = -1;

	if(!g_Barrier.bUse)
	{
		g_Barrier.pos = pos;
		g_Barrier.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Barrier.fWidth = fWidth;
		g_Barrier.fHeight = fHeight;
		g_Barrier.bUse = TRUE;
	}

	return nIdxTree;
}
