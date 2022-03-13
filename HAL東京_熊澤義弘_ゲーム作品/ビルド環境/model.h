//=============================================================================
//
// ���f���̏��� [model.h]
// Author :GP11A132 10 �F�V�`�O
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"

//*********************************************************
// �\����
//*********************************************************

// �}�e���A���\����

#define MODEL_MAX_MATERIAL		(16)		// �P���f����Max�}�e���A����

struct DX11_MODEL_MATERIAL
{
	MATERIAL					Material;
	ID3D11ShaderResourceView	*Texture;
};

// �`��T�u�Z�b�g�\����
struct DX11_SUBSET
{
	unsigned short	StartIndex;
	unsigned short	IndexNum;
	DX11_MODEL_MATERIAL	Material;
};

struct DX11_MODEL
{
	ID3D11Buffer*	VertexBuffer;
	ID3D11Buffer*	IndexBuffer;

	DX11_SUBSET		*SubsetArray;
	unsigned short	SubsetNum;
};

// �}�e���A���\����
struct MODEL_MATERIAL
{
	char						Name[256];
	MATERIAL					Material;
	char						TextureName[256];
};

// �`��T�u�Z�b�g�\����
struct SUBSET
{
	unsigned short	StartIndex;
	unsigned short	IndexNum;
	MODEL_MATERIAL	Material;
};

// ���f���\����
struct MODEL
{
	VERTEX_3D		*VertexArray;
	unsigned short	VertexNum;
	unsigned short	*IndexArray;
	unsigned short	IndexNum;
	SUBSET			*SubsetArray;
	unsigned short	SubsetNum;
};


// �F�ς��悤���f��
struct COLOR
{
	SUBSET			*SubsetArray;
	unsigned short	SubsetNum;
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void LoadModel(char *FileName, DX11_MODEL *Model, int type);
void UnloadModel( DX11_MODEL *Model );
void UnloadModelVertex(DX11_MODEL *Model);
void DrawModel( DX11_MODEL *Model );

// ���f���̃}�e���A���̃f�B�t���[�Y���擾����BMax16���ɂ��Ă���
void GetModelDiffuse(DX11_MODEL *Model, XMFLOAT4 *diffuse);

// ���f���̎w��}�e���A���̃f�B�t���[�Y���Z�b�g����B
void SetModelDiffuse(DX11_MODEL *Model, int mno, XMFLOAT4 diffuse);

// ���f���̓ǂݍ���
void LoadObj(char *FileName, MODEL *Model);
void LoadMaterial(char *FileName, MODEL_MATERIAL **MaterialArray, unsigned short *MaterialNum);

void SetModelColor(DX11_MODEL *Model, COLOR *model);
