

//*****************************************************************************
// 定数バッファ
//*****************************************************************************

// マトリクスバッファ
cbuffer ProjectionBuffer : register(b13)
{
	matrix WVP[10000];
}


// マテリアルバッファ
struct MATERIAL
{
	float4		Ambient;
	float4		Diffuse;
	float4		Specular;
	float4		Emission;
	float		Shininess;
	int			noTexSampling;
	float		Dummy[2];//16byte境界用
};

cbuffer MaterialBuffer : register( b3 )
{
	MATERIAL	Material;
}

// ライト用バッファ
struct LIGHT
{
	float4		Direction[5];
	float4		Position[5];
	float4		Diffuse[5];
	float4		Ambient[5];
	float4		Attenuation[5];
	int4		Flags[5];
	int			Enable;
	int			Dummy[3];//16byte境界用
};

cbuffer LightBuffer : register( b4 )
{
	LIGHT		Light;
}

struct FOG
{
	float4		Distance;
	float4		FogColor;
	int			Enable;
	float		Dummy[3];//16byte境界用
};

// フォグ用バッファ
cbuffer FogBuffer : register( b5 )
{
	FOG			Fog;
};

// 縁取り用バッファ
cbuffer Fuchi : register(b6)
{
	int			fuchi;
	int			fill[3];
};


cbuffer CameraBuffer : register(b7)
{
	float4 Camera;
};



//=============================================================================
// パーティクル用の頂点シェーダ
//=============================================================================
void VertexShaderParticle(in  float4 inPosition	: POSITION0,
	in  float4 inNormal : NORMAL0,
	in  float4 inDiffuse : COLOR0,
	in  float2 inTexCoord : TEXCOORD0,
	in  uint   instID : SV_InstanceID,

	out float4 outPosition : SV_POSITION,
	out float4 outNormal : NORMAL0,
	out float2 outTexCoord : TEXCOORD0,
	out float4 outDiffuse : COLOR0,
	out float4 outWorldPos : POSITION0)
{
	outPosition = mul(inPosition, WVP[instID % 10000]);

	outTexCoord = inTexCoord;

	outDiffuse = inDiffuse;
}

