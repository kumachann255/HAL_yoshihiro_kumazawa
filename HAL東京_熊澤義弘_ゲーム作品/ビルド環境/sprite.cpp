//=============================================================================
//
// �X�v���C�g���� [sprite.cpp]
// Author : GP11A132 10 �F�V�`�O
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************



//=============================================================================
// ���_�f�[�^�ݒ�
//=============================================================================
void SetSprite(ID3D11Buffer *buf, float X, float Y, float Width, float Height, float U, float V, float UW, float VH)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D *vertex = (VERTEX_3D*)msr.pData;

	float hw, hh;
	hw = Width * 0.5f;		// �R���s���[�^�[�͊���Z�����
	hh = Height * 0.5f;		// �|���Z�̕�������������

	// �w�肳�ꂽ���W�𒆐S�ɐݒ肷��v���O����

	// ���_�O�ԁi����̒��_�j
	vertex[0].Position = XMFLOAT3(X - hw, Y - hh, 0.0f);
	vertex[0].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(U, V);

	// ���_�P�ԁi�E��̒��_�j
	vertex[1].Position = XMFLOAT3(X + hw, Y - hh, 0.0f);
	vertex[1].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);

	// ���_�Q�ԁi�����̒��_�j
	vertex[2].Position = XMFLOAT3(X - hw, Y + hh, 0.0f);
	vertex[2].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);

	// ���_�R�ԁi�E���̒��_�j
	vertex[3].Position = XMFLOAT3(X + hw, Y + hh, 0.0f);
	vertex[3].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

	// ��������_�Ƃ��Đݒ肷��v���O����
	//vertex[0].Position = XMFLOAT3(X, Y, 0.0f);
	//vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//vertex[0].TexCoord = D3DXVECTOR2(U, V);

	//vertex[1].Position = XMFLOAT3(X + Width, Y, 0.0f);
	//vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//vertex[1].TexCoord = D3DXVECTOR2(U + UW, V);

	//vertex[2].Position = XMFLOAT3(X, Y + Height, 0.0f);
	//vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//vertex[2].TexCoord = D3DXVECTOR2(U, V + VH);

	//vertex[3].Position = XMFLOAT3(X + Width, Y + Height, 0.0f);
	//vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//vertex[3].TexCoord = D3DXVECTOR2(U + UW, V + VH);



	GetDeviceContext()->Unmap(buf, 0);

}


void SetSpriteLeftTop(ID3D11Buffer *buf, float X, float Y, float Width, float Height, float U, float V, float UW, float VH)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D *vertex = (VERTEX_3D*)msr.pData;

	// ��������_�Ƃ��Đݒ肷��v���O����
	vertex[0].Position = XMFLOAT3(X, Y, 0.0f);
	vertex[0].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(U, V);

	vertex[1].Position = XMFLOAT3(X + Width, Y, 0.0f);
	vertex[1].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);

	vertex[2].Position = XMFLOAT3(X, Y + Height, 0.0f);
	vertex[2].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);

	vertex[3].Position = XMFLOAT3(X + Width, Y + Height, 0.0f);
	vertex[3].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);



	GetDeviceContext()->Unmap(buf, 0);

}



void SetSpriteColor(ID3D11Buffer *buf, float X, float Y, float Width, float Height,
		float U, float V, float UW, float VH,
		XMFLOAT4 color)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D *vertex = (VERTEX_3D*)msr.pData;

	float hw, hh;
	hw = Width * 0.5f;		// �R���s���[�^�[�͊���Z�����
	hh = Height * 0.5f;		// �|���Z�̕�������������

	// �w�肳�ꂽ���W�𒆐S�ɐݒ肷��v���O����

	// ���_�O�ԁi����̒��_�j
	vertex[0].Position = XMFLOAT3(X - hw, Y - hh, 0.0f);
	vertex[0].Diffuse  = color;
	vertex[0].TexCoord = XMFLOAT2(U, V);

	// ���_�P�ԁi�E��̒��_�j
	vertex[1].Position = XMFLOAT3(X + hw, Y - hh, 0.0f);
	vertex[1].Diffuse  = color;
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);

	// ���_�Q�ԁi�����̒��_�j
	vertex[2].Position = XMFLOAT3(X - hw, Y + hh, 0.0f);
	vertex[2].Diffuse  = color;
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);

	// ���_�R�ԁi�E���̒��_�j
	vertex[3].Position = XMFLOAT3(X + hw, Y + hh, 0.0f);
	vertex[3].Diffuse  = color;
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

	GetDeviceContext()->Unmap(buf, 0);

}


void SetSpriteColorRotation(ID3D11Buffer *buf, float X, float Y, float Width, float Height,
	float U, float V, float UW, float VH,
	XMFLOAT4 Color, float Rot)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D *vertex = (VERTEX_3D*)msr.pData;

	float hw, hh;
	hw = Width * 0.5f;		// �R���s���[�^�[�͊���Z�����
	hh = Height * 0.5f;		// �|���Z�̕�������������

	// �w�肳�ꂽ���W�𒆐S�ɉ�]����v���O����
	float BaseAngle = atan2f(hh, hw);			// ���S�_���璸�_�ɑ΂���p�x
	XMVECTOR temp = { hw, hh, 0.0f, 0.0f };
	temp = XMVector2Length(temp);				// ���S�_���璸�_�ɑ΂��鋗��
	float Radius = 0.0f;
	XMStoreFloat(&Radius, temp);

	// �����ŃA�t�B���ϊ��isincos�̂�j���g����4���_����]������
	float x = X - cosf(BaseAngle + Rot) * Radius;
	float y = Y - sinf(BaseAngle + Rot) * Radius;
	vertex[0].Position = XMFLOAT3(x, y, 0.0f);

	x = X + cosf(BaseAngle - Rot) * Radius;
	y = Y - sinf(BaseAngle - Rot) * Radius;
	vertex[1].Position = XMFLOAT3(x, y, 0.0f);

	x = X - cosf(BaseAngle - Rot) * Radius;
	y = Y + sinf(BaseAngle - Rot) * Radius;
	vertex[2].Position = XMFLOAT3(x, y, 0.0f);

	x = X + cosf(BaseAngle + Rot) * Radius;
	y = Y + sinf(BaseAngle + Rot) * Radius;
	vertex[3].Position = XMFLOAT3(x, y, 0.0f);

	vertex[0].Diffuse = Color;
	vertex[1].Diffuse = Color;
	vertex[2].Diffuse = Color;
	vertex[3].Diffuse = Color;

	vertex[0].TexCoord = XMFLOAT2(U, V);
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

	GetDeviceContext()->Unmap(buf, 0);

}

