

//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************

// �}�g���N�X�o�b�t�@
cbuffer WorldBuffer : register( b0 )
{
	matrix World;
}

cbuffer ViewBuffer : register( b1 )
{
	matrix View;
}

cbuffer ProjectionBuffer : register( b2 )
{
	matrix Projection;
}

// �}�e���A���o�b�t�@
struct MATERIAL
{
	float4		Ambient;
	float4		Diffuse;
	float4		Specular;
	float4		Emission;
	float		Shininess;
	int			noTexSampling;
	float		Dummy[2];//16byte���E�p
};

cbuffer MaterialBuffer : register( b3 )
{
	MATERIAL	Material;
}

// ���C�g�p�o�b�t�@
struct LIGHT
{
	float4		Direction[5];
	float4		Position[5];
	float4		Diffuse[5];
	float4		Ambient[5];
	float4		Attenuation[5];
	int4		Flags[5];
	int			Enable;
	int			Dummy[3];//16byte���E�p
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
	float		Dummy[3];//16byte���E�p
};

// �t�H�O�p�o�b�t�@
cbuffer FogBuffer : register( b5 )
{
	FOG			Fog;
};

// �����p�o�b�t�@
cbuffer Fuchi : register(b6)
{
	int			fuchi;
	int			fill[3];
};


cbuffer CameraBuffer : register(b7)
{
	float4 Camera;
};

// �u���[�p�����[�^�o�b�t�@
cbuffer BlurParaBuffer : register(b8)
{
	float4		weights0;
	float4		weights1;

	float2		offset;
	float2		size;
};

// ���C�g����̃r���[
cbuffer ShadowViewBuffer : register(b9)
{
	matrix ShadowView;
}

// ���C�g����̎ˉe
cbuffer ProjectionBuffer : register(b10)
{
	matrix ShadowProjection;
}

//�f�B�]���u�p�o�b�t�@
cbuffer Dissolve : register(b11)
{
	float		threshold;
	int			Dummy[3];//16byte���E�p
};


// �o�n���[�g�̖��@�w
cbuffer BahamutCircle : register(b12)
{
	float3		Circle;
	bool		open;
};



//=============================================================================
// ���_�V�F�[�_
//=============================================================================
void VertexShaderPolygon( in  float4 inPosition		: POSITION0,
						  in  float4 inNormal		: NORMAL0,
						  in  float4 inDiffuse		: COLOR0,
						  in  float2 inTexCoord		: TEXCOORD0,
						  //in  uint   instID			: SV_InstanceID,

						  out float4 outPosition	: SV_POSITION,
						  out float4 outNormal		: NORMAL0,
						  out float2 outTexCoord	: TEXCOORD0,
						  out float4 outDiffuse		: COLOR0,
						  out float4 outWorldPos    : POSITION0)
{
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	outPosition = mul(inPosition, wvp);

	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));

	outTexCoord = inTexCoord;

	outWorldPos = mul(inPosition, World);

	outDiffuse = inDiffuse;
}

// x�����ւ̃u���[���H�̒��_�V�F�[�_�[
void VertexShaderBlurPass1( in  float4 inPosition		: POSITION0,
							in  float4 inNormal			: NORMAL0,
							in  float4 inDiffuse		: COLOR0,
							in  float2 inTexCoord		: TEXCOORD0,

							out float4 outPosition		: SV_POSITION,
							out float4 outNormal		: NORMAL0,
							out float2 outTexCoord0		: TEXCOORD0,
							out float2 outTexCoord1		: TEXCOORD1,
							out float2 outTexCoord2		: TEXCOORD2,
							out float2 outTexCoord3		: TEXCOORD3,
							out float2 outTexCoord4		: TEXCOORD4,
							out float2 outTexCoord5		: TEXCOORD5,
							out float2 outTexCoord6		: TEXCOORD6,
							out float2 outTexCoord7		: TEXCOORD7,
							out float4 outDiffuse		: COLOR0,
							out float4 outWorldPos		: POSITION0)
{
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	outPosition = mul(inPosition, wvp);

	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));

	outTexCoord0 = inTexCoord + float2( -1.0f / size.x, 0.0f);
	outTexCoord1 = inTexCoord + float2( -3.0f / size.x, 0.0f);
	outTexCoord2 = inTexCoord + float2( -5.0f / size.x, 0.0f);
	outTexCoord3 = inTexCoord + float2( -7.0f / size.x, 0.0f);
	outTexCoord4 = inTexCoord + float2( -9.0f / size.x, 0.0f);
	outTexCoord5 = inTexCoord + float2(-11.0f / size.x, 0.0f);
	outTexCoord6 = inTexCoord + float2(-13.0f / size.x, 0.0f);
	outTexCoord7 = inTexCoord + float2(-15.0f / size.x, 0.0f);

	outWorldPos = mul(inPosition, World);

	outDiffuse = inDiffuse;
}

// y�����ւ̃u���[���H�̒��_�V�F�[�_�[
void VertexShaderBlurPass2( in  float4 inPosition	: POSITION0,
							in  float4 inNormal		: NORMAL0,
							in  float4 inDiffuse	: COLOR0,
							in  float2 inTexCoord	: TEXCOORD0,

							out float4 outPosition	: SV_POSITION,
							out float4 outNormal	: NORMAL0,
							out float2 outTexCoord0		: TEXCOORD0,
							out float2 outTexCoord1		: TEXCOORD1,
							out float2 outTexCoord2		: TEXCOORD2,
							out float2 outTexCoord3		: TEXCOORD3,
							out float2 outTexCoord4		: TEXCOORD4,
							out float2 outTexCoord5		: TEXCOORD5,
							out float2 outTexCoord6		: TEXCOORD6,
							out float2 outTexCoord7		: TEXCOORD7,
							out float4 outDiffuse	: COLOR0,
							out float4 outWorldPos  : POSITION0)
{
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	outPosition = mul(inPosition, wvp);

	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));

	outTexCoord0 = inTexCoord + float2( 0.0f, -1.0f / size.y);
	outTexCoord1 = inTexCoord + float2( 0.0f, -3.0f / size.y);
	outTexCoord2 = inTexCoord + float2( 0.0f, -5.0f / size.y);
	outTexCoord3 = inTexCoord + float2( 0.0f, -7.0f / size.y);
	outTexCoord4 = inTexCoord + float2( 0.0f, -9.0f / size.y);
	outTexCoord5 = inTexCoord + float2( 0.0f,-11.0f / size.y);
	outTexCoord6 = inTexCoord + float2( 0.0f,-13.0f / size.y);
	outTexCoord7 = inTexCoord + float2( 0.0f,-15.0f / size.y);

	outWorldPos = mul(inPosition, World);

	outDiffuse = inDiffuse;
}

// �V���h�E�}�b�v�p���_�V�F�[�_�[
void VertexShaderShadowMap( in  float4 inPosition		: POSITION0,
							in  float4 inNormal			: NORMAL0,
							in  float4 inDiffuse		: COLOR0,
							in  float2 inTexCoord		: TEXCOORD0,

							out float4 outPosition		: SV_POSITION,
							out float4 outNormal		: NORMAL0,
							out float4 outDepth			: TEXCOORD0,
							out float4 outDiffuse		: COLOR0,
							out float4 outWorldPos		: POSITION0)
{
	matrix wvp;
	wvp = mul(World, ShadowView);
	wvp = mul(wvp, ShadowProjection);
	outPosition = mul(inPosition, wvp);

	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));

	outDepth = outPosition;

	outWorldPos = mul(inPosition, World);

	outDiffuse = inDiffuse;
}

// �e�𒣂�t���钸�_�V�F�[�_�[
void VertexShaderShadowPaste(	in  float4 inPosition		: POSITION0,
								in  float4 inNormal			: NORMAL0,
								in  float4 inDiffuse		: COLOR0,
								in  float2 inTexCoord		: TEXCOORD0,

								out float4 outPosition		: SV_POSITION,
								out float4 outNormal		: NORMAL0,
								out float2 outTexCoord0		: TEXCOORD0,
								out float4 outLightPos		: TEXCOORD1,
								out float4 outDiffuse		: COLOR0,
								out float4 outWorldPos		: POSITION0)
{
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	outPosition = mul(inPosition, wvp);

	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));

	outTexCoord0 = inTexCoord;

	outWorldPos = mul(inPosition, World);

	outDiffuse = inDiffuse;

	matrix Shadowwvp;
	Shadowwvp = mul(World, ShadowView);
	Shadowwvp = mul(Shadowwvp, ShadowProjection);
	outLightPos = mul(inPosition, Shadowwvp);
}

////=============================================================================
//// �p�[�e�B�N���p�̒��_�V�F�[�_
////=============================================================================
//void VertexShaderParticle(	in  float4 inPosition	: POSITION0,
//							in  float4 inNormal		: NORMAL0,
//							in  float4 inDiffuse	: COLOR0,
//							in  float2 inTexCoord	: TEXCOORD0,
//							in  uint   instID		: SV_InstanceID,
//						
//							out float4 outPosition	: SV_POSITION,
//							out float4 outNormal	: NORMAL0,
//							out float2 outTexCoord	: TEXCOORD0,
//							out float4 outDiffuse	: COLOR0,
//							out float4 outWorldPos	: POSITION0)
//{
//	outPosition = mul(inPosition, WVP[instID % 10000]);
//
//	outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));
//
//	outTexCoord = inTexCoord;
//
//	outWorldPos = mul(inPosition, World);
//
//	outDiffuse = inDiffuse;
//}



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
Texture2D		g_Texture				: register( t0 );
Texture2D		g_TextureRenderTarget	: register( t1 );
Texture2D		g_NoiceTexture			: register( t2 );
Texture2D		g_NoiceTexture1			: register( t3 );
Texture2D		g_NoiceTexture2			: register( t4 );
Texture2D		g_NoiceTexture3			: register( t5 );
SamplerState	g_SamplerState			: register( s0 );
SamplerState	g_SamplerState2			: register( s1 );
SamplerState	g_SamplerState3			: register( s2 );


//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================
void PixelShaderPolygon( in  float4 inPosition		: SV_POSITION,
						 in  float4 inNormal		: NORMAL0,
						 in  float2 inTexCoord		: TEXCOORD0,
						 in  float4 inDiffuse		: COLOR0,
						 in  float4 inWorldPos      : POSITION0,

						 out float4 outDiffuse		: SV_Target )
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	if (Light.Enable == 0)
	{
		color = color * Material.Diffuse;
	}
	else
	{
		float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 5; i++)
		{
			float3 lightDir;
			float light;

			if (Light.Flags[i].y == 1)
			{
				if (Light.Flags[i].x == 1)
				{
					lightDir = normalize(Light.Direction[i].xyz);
					light = dot(lightDir, inNormal.xyz);

					light = 0.5 - 0.5 * light;
					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];
				}
				else if (Light.Flags[i].x == 2)
				{
					lightDir = normalize(Light.Position[i].xyz - inWorldPos.xyz);
					light = dot(lightDir, inNormal.xyz);

					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];

					float distance = length(inWorldPos - Light.Position[i]);

					float att = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
					tempColor *= att;
				}
				else
				{
					tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
				}

				outColor += tempColor;
			}
		}

		color = outColor;
		color.a = inDiffuse.a * Material.Diffuse.a;
	}

	//�t�H�O
	if (Fog.Enable == 1)
	{
		float z = inPosition.z*inPosition.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		outDiffuse = f * color + (1 - f)*Fog.FogColor;
		outDiffuse.a = color.a;
	}
	else
	{
		outDiffuse = color;
	}

	//�����
	if (fuchi == 1)
	{
		float angle = dot(normalize(inWorldPos.xyz - Camera.xyz), normalize(inNormal));
		//if ((angle < 0.5f)&&(angle > -0.5f))
		if (angle > -0.2f)
		{
			outDiffuse.rgb  = 1.0f;
		}
	}
}


// PRE�����_�[�^�[�Q�b�g����back�o�b�t�@�֏������ލۂ̃s�N�Z���V�F�[�_�[
void PixelShaderMap(	in  float4 inPosition		: SV_POSITION,
						in  float4 inNormal			: NORMAL0,
						in  float2 inTexCoord		: TEXCOORD0,
						in  float4 inDiffuse		: COLOR0,
						in  float4 inWorldPos		: POSITION0,

						out float4 outDiffuse		: SV_Target )
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	outDiffuse = color;

}


// �P�x���o����s�N�Z���V�F�[�_�[
void PixelShaderLuminance(	in  float4 inPosition	: SV_POSITION,
							in  float4 inNormal		: NORMAL0,
							in  float2 inTexCoord	: TEXCOORD0,
							in  float4 inDiffuse	: COLOR0,
							in  float4 inWorldPos	: POSITION0,

							out float4 outDiffuse	: SV_Target)
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_TextureRenderTarget.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	if ((color.x + color.y + color.z) / 3.0f > 0.7f)
	{
		outDiffuse = color;
	}
	else
	{
		outDiffuse.xyz = 0.0f;
		outDiffuse.w = 1.0f;
	}
}


// x�����փu���[���H����s�N�Z���V�F�[�_�[
void PixelShaderBlurPass1(	in  float4 inPosition	: SV_POSITION,
							in  float4 inNormal		: NORMAL0,
							in  float2 inTexCoord0	: TEXCOORD0,
							in  float2 inTexCoord1	: TEXCOORD1,
							in  float2 inTexCoord2	: TEXCOORD2,
							in  float2 inTexCoord3	: TEXCOORD3,
							in  float2 inTexCoord4	: TEXCOORD4,
							in  float2 inTexCoord5	: TEXCOORD5,
							in  float2 inTexCoord6	: TEXCOORD6,
							in  float2 inTexCoord7	: TEXCOORD7,
							in  float4 inDiffuse	: COLOR0,
							in  float4 inWorldPos	: POSITION0,
							
							out float4 outDiffuse	: SV_Target)
{
	float4 color = 0.0f;
	
	color += weights0.x * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord0) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord7 + float2(offset.x, 0.0f)));
	color += weights0.y * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord1) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord6 + float2(offset.x, 0.0f)));
	color += weights0.z * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord2) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord5 + float2(offset.x, 0.0f)));
	color += weights0.w * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord3) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord4 + float2(offset.x, 0.0f)));


	color += weights1.x * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord4) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord3 + float2(offset.x, 0.0f)));
	color += weights1.y * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord5) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord2 + float2(offset.x, 0.0f)));
	color += weights1.z * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord6) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord1 + float2(offset.x, 0.0f)));
	color += weights1.w * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord7) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord0 + float2(offset.x, 0.0f)));


	outDiffuse = color;
}

// y�����փu���[���H����s�N�Z���V�F�[�_�[
void PixelShaderBlurPass2(	in  float4 inPosition	: SV_POSITION,
							in  float4 inNormal		: NORMAL0,
							in  float2 inTexCoord0	: TEXCOORD0,
							in  float2 inTexCoord1	: TEXCOORD1,
							in  float2 inTexCoord2	: TEXCOORD2,
							in  float2 inTexCoord3	: TEXCOORD3,
							in  float2 inTexCoord4	: TEXCOORD4,
							in  float2 inTexCoord5	: TEXCOORD5,
							in  float2 inTexCoord6	: TEXCOORD6,
							in  float2 inTexCoord7	: TEXCOORD7,
							in  float4 inDiffuse	: COLOR0,
							in  float4 inWorldPos	: POSITION0,
							
							out float4 outDiffuse	: SV_Target)
{
	float4 color = 0.0f;

	color += weights0.x * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord0) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord7 + float2( 0.0f, offset.y)));
	color += weights0.y * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord1) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord6 + float2( 0.0f, offset.y)));
	color += weights0.z * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord2) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord5 + float2( 0.0f, offset.y)));
	color += weights0.w * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord3) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord4 + float2( 0.0f, offset.y)));

	color += weights1.x * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord4) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord3 + float2( 0.0f, offset.y)));
	color += weights1.y * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord5) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord2 + float2( 0.0f, offset.y)));
	color += weights1.z * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord6) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord1 + float2( 0.0f, offset.y)));
	color += weights1.w * (g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord7) + g_TextureRenderTarget.Sample(g_SamplerState2, inTexCoord0 + float2( 0.0f, offset.y)));

	outDiffuse = color;
}


// PRErt��BLURrt���������Aback�o�b�t�@�֏������ލۂ̃s�N�Z���V�F�[�_�[
void PixelShaderComposite(	in  float4 inPosition		: SV_POSITION,
							in  float4 inNormal			: NORMAL0,
							in  float2 inTexCoord		: TEXCOORD0,
							in  float4 inDiffuse		: COLOR0,
							in  float4 inWorldPos		: POSITION0,

							out float4 outDiffuse		: SV_Target )
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	// �����F�𒲐����ĉ��Z����
	color += 2.0f * g_TextureRenderTarget.SampleLevel(g_SamplerState, inTexCoord, 0);

	outDiffuse = color;
}


// �����p�֐�
float4 FetchColor(Texture2D map, float2 uv)
{
	float4 outcolor = 0;

	outcolor += map.SampleLevel(g_SamplerState2, uv, 0);
	outcolor += map.SampleLevel(g_SamplerState2, uv, 0, int2(-1, -1));
	outcolor += map.SampleLevel(g_SamplerState2, uv, 0, int2(1, -1));
	outcolor += map.SampleLevel(g_SamplerState2, uv, 0, int2(-1, 1));
	outcolor += map.SampleLevel(g_SamplerState2, uv, 0, int2(1, 1));
	outcolor *= 0.2f;

	return outcolor;
}


// �V���h�E�}�b�v�𐶐�����s�N�Z���V�F�[�_�[
void PixelShaderShadowMap( in  float4 inPosition		: SV_POSITION,
						   in  float4 inNormal			: NORMAL0,
						   in  float4 inTexCoord		: TEXCOORD0,
						   in  float4 inDiffuse			: COLOR0,
						   in  float4 inWorldPos		: POSITION0,
						  
						   out float4 outDiffuse		: SV_Target )
{
	float4 color;
	float4 acheck;

	acheck = g_Texture.Sample(g_SamplerState, inTexCoord);


	// �s�N�Z���̐[�x�����v�Z���ďo��
	color = inTexCoord.z / inTexCoord.w;

	if (acheck.a < 0.2f) color = 0.0f;
	else
	{
		color.a = 1.0f;
		outDiffuse = color;
	}
}


// �e�����ۂɕ`�悷��s�N�Z���V�F�[�_�[
void PixelShaderShadowPaste( in  float4 inPosition		: SV_POSITION,
							 in  float4 inNormal		: NORMAL0,
							 in  float2 inTexCoord		: TEXCOORD0,
							 in  float4 inLightPos		: TEXCOORD1,
							 in  float4 inDiffuse		: COLOR0,
							 in  float4 inWorldPos		: POSITION0,
						  	 
							 out float4 outDiffuse		: SV_Target )
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	// �e����

	// �V���h�E�}�b�v����l���擾
	// �V���h�E�}�b�v�̃X�N���[�����W�̂ǂ��̃s�N�Z����񉻂��Z�o
	float2 shadowUV;
	shadowUV.x = 0.5f * inLightPos.x / inLightPos.w + 0.5f;
	shadowUV.y = 0.5f * inLightPos.y / inLightPos.w + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	shadowUV.x += 0.0008f;
	shadowUV.y += 0.0008f;

	// �V���h�E�}�b�v����[�x(�F)�𒊏o
	float4 shadowMap;
	shadowMap = g_TextureRenderTarget.Sample(g_SamplerState3, shadowUV);

	// ���݂̃s�N�Z���̐[�x�l���Z�o
	float depth = inLightPos.z / inLightPos.w;

	// �V���h�E�}�b�v�̐[�x�l�Ɣ�r���A�e��t����
	if (depth - shadowMap.r > 0.00229f)
	{
		if (shadowMap.r > 0.0f) color.xyz *= 0.3f;
	}

	if (Light.Enable == 0)
	{
		color = color * Material.Diffuse;
	}
	else
	{
		float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 5; i++)
		{
			float3 lightDir;
			float light;

			if (Light.Flags[i].y == 1)
			{
				if (Light.Flags[i].x == 1)
				{
					lightDir = normalize(Light.Direction[i].xyz);
					light = dot(lightDir, inNormal.xyz);

					light = 0.5 - 0.5 * light;
					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];
				}
				else if (Light.Flags[i].x == 2)
				{
					lightDir = normalize(Light.Position[i].xyz - inWorldPos.xyz);
					light = dot(lightDir, inNormal.xyz);

					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];

					float distance = length(inWorldPos - Light.Position[i]);

					float att = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
					tempColor *= att;
				}
				else
				{
					tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
				}

				outColor += tempColor;
			}
		}

		color = outColor;
		color.a = inDiffuse.a * Material.Diffuse.a;
	}

	//�t�H�O
	if (Fog.Enable == 1)
	{
		float z = inPosition.z*inPosition.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		outDiffuse = f * color + (1 - f)*Fog.FogColor;
		outDiffuse.a = color.a;
	}
	else
	{
		outDiffuse = color;
	}

	//�����
	if (fuchi == 1)
	{
		float angle = dot(normalize(inWorldPos.xyz - Camera.xyz), normalize(inNormal));
		//if ((angle < 0.5f)&&(angle > -0.5f))
		if (angle > -0.5f)
		{
			outDiffuse.rgb = 1.0f;
		}
	}

}


// ���@�w�p�̃f�B�]���u�s�N�Z���V�F�[�_�[
void PixelShaderMagicCircle(	in  float4 inPosition		: SV_POSITION,
								in  float4 inNormal			: NORMAL0,
								in  float2 inTexCoord		: TEXCOORD0,
								in  float4 inDiffuse		: COLOR0,
								in  float4 inWorldPos		: POSITION0,

								out float4 outDiffuse		: SV_Target)
{
	float4 color;
	float4 dissolve;

	// �e�N�X�`������F�������Ă���
	color = g_Texture.Sample(g_SamplerState, inTexCoord);

	// �m�C�Y�e�N�X�`������F�������Ă���
	dissolve = g_NoiceTexture.Sample(g_SamplerState, inTexCoord);

	// 臒l�ƃm�C�Y�e�N�X�`����r�l���r
	if (threshold > dissolve.r)
	{	// 臒l�ȉ��̏ꏊ��`�悷��
		color *= inDiffuse;
		color = color * Material.Diffuse;

		// ����n�߂鉏�𔒂����点��
		if ((color.a > 0.2f) && (threshold - 0.06f < dissolve.r))
		{
			color = 1.0f;
		}
	}
	else
	{	// a�l��0.0f�ɂ��ē�����
		color.a = 0.0f;
	}

	outDiffuse = color;
}


// �G�l�~�[�p�̃V���h�E�}�b�v + �f�B�]���u�s�N�Z���V�F�[�_
// �e�����ۂɕ`�悷��s�N�Z���V�F�[�_�[
void PixelShaderEnemy(	in  float4 inPosition		: SV_POSITION,
						in  float4 inNormal			: NORMAL0,
						in  float2 inTexCoord		: TEXCOORD0,
						in  float4 inLightPos		: TEXCOORD1,
						in  float4 inDiffuse		: COLOR0,
						in  float4 inWorldPos		: POSITION0,

						out float4 outDiffuse		: SV_Target )
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}


	// �e����

	// �V���h�E�}�b�v����l���擾
	// �V���h�E�}�b�v�̃X�N���[�����W�̂ǂ��̃s�N�Z����񉻂��Z�o
	float2 shadowUV;
	shadowUV.x = 0.5f * inLightPos.x / inLightPos.w + 0.5f;
	shadowUV.y = 0.5f * inLightPos.y / inLightPos.w + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	shadowUV.x += 0.0008f;
	shadowUV.y += 0.0008f;

	// �V���h�E�}�b�v����[�x(�F)�𒊏o
	float4 shadowMap;
	shadowMap = g_TextureRenderTarget.Sample(g_SamplerState3, shadowUV);

	// ���݂̃s�N�Z���̐[�x�l���Z�o
	float depth = inLightPos.z / inLightPos.w;

	// �V���h�E�}�b�v�̐[�x�l�Ɣ�r���A�e��t����
	if (depth - shadowMap.r > 0.00229f)
	{
		if(shadowMap.r > 0.0f) color.xyz *= 0.3f;
	}

	if (Light.Enable == 0)
	{
		color = color * Material.Diffuse;
	}
	else
	{
		float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 5; i++)
		{
			float3 lightDir;
			float light;

			if (Light.Flags[i].y == 1)
			{
				if (Light.Flags[i].x == 1)
				{
					lightDir = normalize(Light.Direction[i].xyz);
					light = dot(lightDir, inNormal.xyz);

					light = 0.5 - 0.5 * light;
					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];
				}
				else if (Light.Flags[i].x == 2)
				{
					lightDir = normalize(Light.Position[i].xyz - inWorldPos.xyz);
					light = dot(lightDir, inNormal.xyz);

					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];

					float distance = length(inWorldPos - Light.Position[i]);

					float att = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
					tempColor *= att;
				}
				else
				{
					tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
				}

				outColor += tempColor;
			}
		}

		color = outColor;
		color.a = inDiffuse.a * Material.Diffuse.a;
	}

	//�t�H�O
	if (Fog.Enable == 1)
	{
		float z = inPosition.z*inPosition.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		outDiffuse = f * color + (1 - f)*Fog.FogColor;
		outDiffuse.a = color.a;
	}
	else
	{
		outDiffuse = color;
	}

	//�����
	if (fuchi == 1)
	{
		float angle = dot(normalize(inWorldPos.xyz - Camera.xyz), normalize(inNormal));
		//if ((angle < 0.5f)&&(angle > -0.5f))
		if (angle > -0.5f)
		{
			outDiffuse.rgb = 1.0f;
		}
	}

	// �m�C�Y�e�N�X�`������F�������Ă���
	float4 dissolve;
	dissolve = g_NoiceTexture.Sample(g_SamplerState, inTexCoord);

	// 臒l�ƃm�C�Y�e�N�X�`����r�l���r
	if (threshold > dissolve.r)
	{	// 臒l�ȉ��̏ꏊ��`�悷��
		// ����n�߂鉏�𔒂����点��
		if ((color.a > 0.2f) && (threshold - 0.1f < dissolve.r))
		{
			color = 0.7f;
			color.ra = 1.0f;
		}
	}
	else
	{	// a�l��0.0f�ɂ��ē�����
		color.a = 0.0f;
	}

	outDiffuse = color;
}


// ��p�̃V���h�E�}�b�v + �f�B�]���u�s�N�Z���V�F�[�_
// �e�����ۂɕ`�悷��s�N�Z���V�F�[�_�[
void PixelShaderWand(	in  float4 inPosition	: SV_POSITION,
						in  float4 inNormal		: NORMAL0,
						in  float2 inTexCoord	: TEXCOORD0,
						in  float4 inLightPos	: TEXCOORD1,
						in  float4 inDiffuse	: COLOR0,
						in  float4 inWorldPos	: POSITION0,
						
						out float4 outDiffuse	: SV_Target )
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	// �e����

	// �V���h�E�}�b�v����l���擾
	// �V���h�E�}�b�v�̃X�N���[�����W�̂ǂ��̃s�N�Z����񉻂��Z�o
	float2 shadowUV;
	shadowUV.x = 0.5f * inLightPos.x / inLightPos.w + 0.5f;
	shadowUV.y = 0.5f * inLightPos.y / inLightPos.w + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	shadowUV.x += 0.0008f;
	shadowUV.y += 0.0008f;

	// �V���h�E�}�b�v����[�x(�F)�𒊏o
	float4 shadowMap;
	shadowMap = g_TextureRenderTarget.Sample(g_SamplerState3, shadowUV);

	// ���݂̃s�N�Z���̐[�x�l���Z�o
	float depth = inLightPos.z / inLightPos.w;

	// �V���h�E�}�b�v�̐[�x�l�Ɣ�r���A�e��t����
	if (depth - shadowMap.r > 0.00229f)
	{
		color.xyz *= 0.3f;
	}

	if (Light.Enable == 0)
	{
		color = color * Material.Diffuse;
	}
	else
	{
		float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 5; i++)
		{
			float3 lightDir;
			float light;

			if (Light.Flags[i].y == 1)
			{
				if (Light.Flags[i].x == 1)
				{
					lightDir = normalize(Light.Direction[i].xyz);
					light = dot(lightDir, inNormal.xyz);

					light = 0.5 - 0.5 * light;
					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];
				}
				else if (Light.Flags[i].x == 2)
				{
					lightDir = normalize(Light.Position[i].xyz - inWorldPos.xyz);
					light = dot(lightDir, inNormal.xyz);

					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];

					float distance = length(inWorldPos - Light.Position[i]);

					float att = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
					tempColor *= att;
				}
				else
				{
					tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
				}

				outColor += tempColor;
			}
		}

		color = outColor;
		color.a = inDiffuse.a * Material.Diffuse.a;
	}


	// �m�C�Y�e�N�X�`������F�������Ă���
	float4 dissolve;
	dissolve = g_NoiceTexture.Sample(g_SamplerState, inTexCoord);

	// 臒l�ƃm�C�Y�e�N�X�`����r�l���r
	if (threshold > dissolve.r)
	{	// 臒l�ȉ��̏ꏊ��`�悷��
		// ����n�߂鉏�𔒂����点��
		if ((color.a > 0.2f) && (threshold - 0.1f < dissolve.r))
		{
			color = 1.0;
		}
	}

	outDiffuse = color;
}



// �o�n���[�g�p�̃f�B�]���u�s�N�Z���V�F�[�_
void PixelShaderBahamut(in  float4 inPosition	: SV_POSITION,
						in  float4 inNormal		: NORMAL0,
						in  float2 inTexCoord	: TEXCOORD0,
						in  float4 inLightPos	: TEXCOORD1,
						in  float4 inDiffuse	: COLOR0,
						in  float4 inWorldPos	: POSITION0,

						out float4 outDiffuse	: SV_Target)
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}

	if (Light.Enable == 0)
	{
		color = color * Material.Diffuse;
	}
	else
	{
		float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 5; i++)
		{
			float3 lightDir;
			float light;

			if (Light.Flags[i].y == 1)
			{
				if (Light.Flags[i].x == 1)
				{
					//lightDir = normalize(Light.Direction[i].xyz);
					//light = dot(lightDir, inNormal.xyz);

					//light = 0.5 - 0.5 * light;
					tempColor = color * Material.Diffuse * Light.Diffuse[i];
				}
				else if (Light.Flags[i].x == 2)
				{
					lightDir = normalize(Light.Position[i].xyz - inWorldPos.xyz);
					light = dot(lightDir, inNormal.xyz);

					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];

					float distance = length(inWorldPos - Light.Position[i]);

					float att = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
					tempColor *= att;
				}
				else
				{
					tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
				}

				outColor += tempColor;
			}
		}

		color = outColor;
		color.a = inDiffuse.a * Material.Diffuse.a;
	}

	//�t�H�O
	if (Fog.Enable == 1)
	{
		float z = inPosition.z*inPosition.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		outDiffuse = f * color + (1 - f)*Fog.FogColor;
		outDiffuse.a = color.a;
	}
	else
	{
		outDiffuse = color;
	}

	// �m�C�Y�e�N�X�`������F�������Ă���
	float4 dissolve;

	// �m�C�Y�e�N�X�`�����X�V
	dissolve = g_NoiceTexture.Sample(g_SamplerState, inTexCoord);

	float y = Circle.y + 180.0f;

	// ���@�w�������ɂ��镔�����f�B�]���u�ŕ`��
	if (y > inWorldPos.y)
	{
		float dis = y - inWorldPos.y;

		dis /= 20.0f;
		//dis += 0.4f;

		if (dis < dissolve.r)
		{
			color.a = 0.0f;
		}
		else if ((color.a > 0.2f) && (dis - 0.1f < dissolve.r))
		{
			color = 1.0f;
		}

		if(!open) color = 1.0f;

	}
	else
	{
		color.a = 0.0f;
	}

	outDiffuse = color;
}


// �o�n���[�g�̖��@�w�p�̃f�B�]���u�s�N�Z���V�F�[�_
void PixelShaderBahamutCircle(	in  float4 inPosition	: SV_POSITION,
								in  float4 inNormal		: NORMAL0,
								in  float2 inTexCoord	: TEXCOORD0,
								in  float4 inDiffuse	: COLOR0,
								in  float4 inWorldPos	: POSITION0,

								out float4 outDiffuse	: SV_Target)
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}


	//�t�H�O
	if (Fog.Enable == 1)
	{
		float z = inPosition.z*inPosition.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		outDiffuse = f * color + (1 - f)*Fog.FogColor;
		outDiffuse.a = color.a;
	}
	else
	{
		outDiffuse = color;
	}


	// �m�C�Y�e�N�X�`������F�������Ă���
	float4 dissolve;
	dissolve = g_NoiceTexture1.Sample(g_SamplerState, inTexCoord);

	// 臒l�ƃm�C�Y�e�N�X�`����r�l���r
	if (threshold > dissolve.r)
	{	// 臒l�ȉ��̏ꏊ��`�悷��
		// ����n�߂鉏�𔒂����点��
		//if ((color.a > 0.2f) && (threshold - 0.01f < dissolve.r))
		//{
		//	color = 0.7f;
		//	color.ra = 1.0f;
		//}
	}
	else
	{	// a�l��0.0f�ɂ��ē�����
		color.a = 0.0f;
	}

	outDiffuse = color;
}


// �o�n���[�g�̔g���p�̃f�B�]���u�s�N�Z���V�F�[�_
void PixelShaderBahamutSurge(	in  float4 inPosition	: SV_POSITION,
								in  float4 inNormal		: NORMAL0,
								in  float2 inTexCoord	: TEXCOORD0,
								in  float4 inDiffuse	: COLOR0,
								in  float4 inWorldPos	: POSITION0,

								out float4 outDiffuse	: SV_Target)
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}


	//�t�H�O
	if (Fog.Enable == 1)
	{
		float z = inPosition.z*inPosition.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		outDiffuse = f * color + (1 - f)*Fog.FogColor;
		outDiffuse.a = color.a;
	}
	else
	{
		outDiffuse = color;
	}


	// �m�C�Y�e�N�X�`������F�������Ă���
	float4 dissolve;
	dissolve = g_NoiceTexture2.Sample(g_SamplerState, inTexCoord);

	// ��{����
	color.a = 0.0f;

	// 臒l�ƃm�C�Y�e�N�X�`����r�l���r
	if (threshold > dissolve.r)
	{	// 臒l�ȉ��̏ꏊ��`�悷��
		// 臒l���ӂ݂̂�`��
		if (threshold - 0.1f < dissolve.r)
		{
			color.a = 1.0f;
		}
	}

	// �g����2�E�F�[�u�`��
	float r = dissolve.r + 0.5f;
	if (r > 1.0f) r -= 1.0f;

	if (threshold > r )
	{	// 臒l�ȉ��̏ꏊ��`�悷��
		// 臒l���ӂ݂̂�`��
		if (threshold - 0.1f < r)
		{
			color.a = 1.0f;
		}
	}

	outDiffuse = color;
}


// �p�[�e�B�N���p�̃s�N�Z���V�F�[�_
void PixelShaderParticle(	in  float4 inPosition	: SV_POSITION,
							in  float4 inNormal		: NORMAL0,
							in  float2 inTexCoord	: TEXCOORD0,
							in  float4 inDiffuse	: COLOR0,
							in  float4 inWorldPos	: POSITION0,

							out float4 outDiffuse	: SV_Target)
{
	float4 color;

	color = g_Texture.Sample(g_SamplerState, inTexCoord);

	color *= inDiffuse;

	color = color * Material.Diffuse;

	outDiffuse = color;
}


// �����@�p�̃f�B�]���u�s�N�Z���V�F�[�_
void PixelShaderWater(	in  float4 inPosition	: SV_POSITION,
						in  float4 inNormal		: NORMAL0,
						in  float2 inTexCoord	: TEXCOORD0,
						in  float4 inDiffuse	: COLOR0,
						in  float4 inWorldPos	: POSITION0,

						out float4 outDiffuse	: SV_Target)
{
	float4 color;

	if (Material.noTexSampling == 0)
	{
		inTexCoord.y += threshold;
		color = g_Texture.Sample(g_SamplerState, inTexCoord);

		color *= inDiffuse;
	}
	else
	{
		color = inDiffuse;
	}


	//�t�H�O
	if (Fog.Enable == 1)
	{
		float z = inPosition.z*inPosition.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		outDiffuse = f * color + (1 - f)*Fog.FogColor;
		outDiffuse.a = color.a;
	}
	else
	{
		outDiffuse = color;
	}


	// �m�C�Y�e�N�X�`������F�������Ă���
	float4 dissolve;
	dissolve = g_NoiceTexture3.Sample(g_SamplerState, inTexCoord);

	// 臒l�ƃm�C�Y�e�N�X�`����r�l���r
	if (threshold > dissolve.r)
	{	// 臒l�ȉ��̏ꏊ��`�悷��
		// 臒l���ӂ݂̂𔒂��`��
		if (threshold - 0.05f < dissolve.r)
		{
			color.rgb = 1.0f;
		}
	}

	// �g���2�E�F�[�u�`��
	float r = dissolve.r + 0.5f;
	if (r > 1.0f) r -= 1.0f;

	if (threshold > r)
	{	// 臒l�ȉ��̏ꏊ��`�悷��
		// 臒l���ӂ݂̂𔒂��`��
		if (threshold - 0.05f < r)
		{
			color.rgb = 1.0f;
		}
	}

	outDiffuse = color;

	//�����
	if (fuchi == 1)
	{
		float angle = dot(normalize(inWorldPos.xyz - Camera.xyz), normalize(inNormal));
		//if ((angle < 0.5f)&&(angle > -0.5f))
		if (angle > -0.2f)
		{
			outDiffuse.rgb = 1.0f;
		}
	}


}



// �����p�̃s�N�Z���V�F�[�_
void PixelShaderBlast(	in  float4 inPosition	: SV_POSITION,
						in  float4 inNormal		: NORMAL0,
						in  float2 inTexCoord	: TEXCOORD0,
						in  float4 inDiffuse	: COLOR0,
						in  float4 inWorldPos	: POSITION0,

						out float4 outDiffuse	: SV_Target)
{
	float4 color;

	color = g_Texture.Sample(g_SamplerState, inTexCoord);

	color *= inDiffuse;

	color = color * Material.Diffuse;

	// ������W����𓧖��ɂ���
	float y = Circle.y;

	// �`�悵�悤�Ƃ��Ă���s�N�Z���̃��[���h���W����l������̏ꍇ����
	if (inWorldPos.y > y)
	{
		color.a = 0.0f;
	}

	outDiffuse = color;
}
