//=============================================================================
//
// �`���[�g���A���p�o���A���� [barrier.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "barrier.h"
#include "tutorial.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX			(1)				// �e�N�X�`���̐�

#define	BARRIER_WIDTH		(240.0f)		// ���_�T�C�Y
#define	BARRIER_HEIGHT		(240.0f)		// ���_�T�C�Y

#define DISSOLVE_SPEED		(0.005f)			// �f�B�]���u�̑��x

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
typedef struct
{
	XMFLOAT3	pos;			// �ʒu
	XMFLOAT3	scl;			// �X�P�[��
	MATERIAL	material;		// �}�e���A��
	float		fWidth;			// ��
	float		fHeight;		// ����
	int			nIdxShadow;		// �eID
	BOOL		bUse;			// �g�p���Ă��邩�ǂ���
	float		dissolve;		// �f�B�]���u��臒l

} BARRIER;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexBarrier(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;	// ���_�o�b�t�@
static ID3D11ShaderResourceView		*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static BARRIER				g_Barrier;			// �o���A���[�N
static int					g_TexNo;			// �e�N�X�`���ԍ�
static BOOL					g_bAlpaTest;		// �A���t�@�e�X�gON/OFF
//static int				g_nAlpha;			// �A���t�@�e�X�g��臒l

static BOOL					g_Load = FALSE;

static char *g_TextureName[TEXTURE_MAX] =
{
	"data/TEXTURE/barrier_plate.png",
};


//=============================================================================
// ����������
//=============================================================================
HRESULT InitBarrier(void)
{
	MakeVertexBarrier();

	// �e�N�X�`������
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

	// �o���A���[�N�̏�����
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

	// �o���A�̐ݒ�
	SetBarrier(XMFLOAT3(195.0f, BARRIER_HEIGHT / 2.0f, 0.0f), BARRIER_WIDTH, BARRIER_HEIGHT, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitBarrier(void)
{
	if (g_Load == FALSE) return;

	for(int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if(g_Texture[nCntTex] != NULL)
		{// �e�N�X�`���̉��
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	if(g_VertexBuffer != NULL)
	{// ���_�o�b�t�@�̉��
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateBarrier(void)
{
	// �`���[�g���A���N���A���I�����Ă���ꍇ�͏���
	if (GetTutorialClear(tx_clear)) g_Barrier.bUse = FALSE;

	// �f�B�]���u����
	if ((g_Barrier.bUse == FALSE) && (g_Barrier.dissolve > 0.0f))
	{
		g_Barrier.dissolve -= DISSOLVE_SPEED;
	}

#ifdef _DEBUG
	// �A���t�@�e�X�gON/OFF
	if(GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? FALSE: TRUE;
	}

#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawBarrier(void)
{
	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���e�X�g�ݒ�
	if (g_bAlpaTest == TRUE)
	{
		// ���e�X�g��L����
		SetAlphaTestEnable(TRUE);
	}

	// ���C�e�B���O�𖳌�
	SetLightEnable(FALSE);

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA *cam = GetCamera();

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �r���[�}�g���b�N�X���擾
	mtxView = XMLoadFloat4x4(&cam->mtxView);

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Barrier.scl.x, g_Barrier.scl.y, g_Barrier.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Barrier.pos.x, g_Barrier.pos.y, g_Barrier.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	// �f�B�]���u�̐ݒ�
	SetDissolve(g_Barrier.dissolve);

	// �}�e���A���ݒ�
	SetMaterial(g_Barrier.material);

	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	// �|���S���̕`��
	GetDeviceContext()->Draw(4, 0);

	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// ���e�X�g�𖳌���
	SetAlphaTestEnable(FALSE);

	// �J�����O����
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexBarrier(void)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// ���_�o�b�t�@�ɒl���Z�b�g����
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = BARRIER_WIDTH;
	float fHeight = BARRIER_HEIGHT;

	// ���_���W�̐ݒ�
	vertex[0].Position = XMFLOAT3(fWidth / 2.0f,  fHeight / 2.0f, -fWidth / 2.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f,  fHeight / 2.0f,  fWidth / 2.0f);
	vertex[2].Position = XMFLOAT3(fWidth / 2.0f, -fHeight / 2.0f, -fWidth / 2.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, -fHeight / 2.0f,  fWidth / 2.0f);

	// �g�U���̐ݒ�
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// �e�N�X�`�����W�̐ݒ�
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}

//=============================================================================
// �o���A�̃p�����[�^���Z�b�g
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
