

//*****************************************************************************
// 定数バッファ
//*****************************************************************************

// マトリクスバッファ
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

// ブラーパラメータバッファ
cbuffer BlurParaBuffer : register(b8)
{
	float4		weights0;
	float4		weights1;

	float2		offset;
	float2		size;
};

// ライトからのビュー
cbuffer ShadowViewBuffer : register(b9)
{
	matrix ShadowView;
}

// ライトからの射影
cbuffer ProjectionBuffer : register(b10)
{
	matrix ShadowProjection;
}

//ディゾルブ用バッファ
cbuffer Dissolve : register(b11)
{
	float		threshold;
	int			Dummy[3];//16byte境界用
};


// バハムートの魔法陣
cbuffer BahamutCircle : register(b12)
{
	float3		Circle;
	bool		open;
};



//=============================================================================
// 頂点シェーダ
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

// x方向へのブラー加工の頂点シェーダー
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

// y方向へのブラー加工の頂点シェーダー
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

// シャドウマップ用頂点シェーダー
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

// 影を張り付ける頂点シェーダー
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
//// パーティクル用の頂点シェーダ
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
// グローバル変数
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
// ピクセルシェーダ
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

	//フォグ
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

	//縁取り
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


// PREレンダーターゲットからbackバッファへ書き込む際のピクセルシェーダー
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


// 輝度抽出するピクセルシェーダー
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


// x方向へブラー加工するピクセルシェーダー
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

// y方向へブラー加工するピクセルシェーダー
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


// PRErtとBLURrtを合成し、backバッファへ書き込む際のピクセルシェーダー
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

	// 少し色を調整して加算合成
	color += 2.0f * g_TextureRenderTarget.SampleLevel(g_SamplerState, inTexCoord, 0);

	outDiffuse = color;
}


// 合成用関数
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


// シャドウマップを生成するピクセルシェーダー
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


	// ピクセルの深度情報を計算して出力
	color = inTexCoord.z / inTexCoord.w;

	if (acheck.a < 0.2f) color = 0.0f;
	else
	{
		color.a = 1.0f;
		outDiffuse = color;
	}
}


// 影を実際に描画するピクセルシェーダー
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

	// 影処理

	// シャドウマップから値を取得
	// シャドウマップのスクリーン座標のどこのピクセル情報化を算出
	float2 shadowUV;
	shadowUV.x = 0.5f * inLightPos.x / inLightPos.w + 0.5f;
	shadowUV.y = 0.5f * inLightPos.y / inLightPos.w + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	shadowUV.x += 0.0008f;
	shadowUV.y += 0.0008f;

	// シャドウマップから深度(色)を抽出
	float4 shadowMap;
	shadowMap = g_TextureRenderTarget.Sample(g_SamplerState3, shadowUV);

	// 現在のピクセルの深度値を算出
	float depth = inLightPos.z / inLightPos.w;

	// シャドウマップの深度値と比較し、影を付ける
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

	//フォグ
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

	//縁取り
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


// 魔法陣用のディゾルブピクセルシェーダー
void PixelShaderMagicCircle(	in  float4 inPosition		: SV_POSITION,
								in  float4 inNormal			: NORMAL0,
								in  float2 inTexCoord		: TEXCOORD0,
								in  float4 inDiffuse		: COLOR0,
								in  float4 inWorldPos		: POSITION0,

								out float4 outDiffuse		: SV_Target)
{
	float4 color;
	float4 dissolve;

	// テクスチャから色を持ってくる
	color = g_Texture.Sample(g_SamplerState, inTexCoord);

	// ノイズテクスチャから色を持ってくる
	dissolve = g_NoiceTexture.Sample(g_SamplerState, inTexCoord);

	// 閾値とノイズテクスチャのr値を比較
	if (threshold > dissolve.r)
	{	// 閾値以下の場所を描画する
		color *= inDiffuse;
		color = color * Material.Diffuse;

		// 現れ始める縁を白く光らせる
		if ((color.a > 0.2f) && (threshold - 0.06f < dissolve.r))
		{
			color = 1.0f;
		}
	}
	else
	{	// a値を0.0fにして透明化
		color.a = 0.0f;
	}

	outDiffuse = color;
}


// エネミー用のシャドウマップ + ディゾルブピクセルシェーダ
// 影を実際に描画するピクセルシェーダー
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


	// 影処理

	// シャドウマップから値を取得
	// シャドウマップのスクリーン座標のどこのピクセル情報化を算出
	float2 shadowUV;
	shadowUV.x = 0.5f * inLightPos.x / inLightPos.w + 0.5f;
	shadowUV.y = 0.5f * inLightPos.y / inLightPos.w + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	shadowUV.x += 0.0008f;
	shadowUV.y += 0.0008f;

	// シャドウマップから深度(色)を抽出
	float4 shadowMap;
	shadowMap = g_TextureRenderTarget.Sample(g_SamplerState3, shadowUV);

	// 現在のピクセルの深度値を算出
	float depth = inLightPos.z / inLightPos.w;

	// シャドウマップの深度値と比較し、影を付ける
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

	//フォグ
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

	//縁取り
	if (fuchi == 1)
	{
		float angle = dot(normalize(inWorldPos.xyz - Camera.xyz), normalize(inNormal));
		//if ((angle < 0.5f)&&(angle > -0.5f))
		if (angle > -0.5f)
		{
			outDiffuse.rgb = 1.0f;
		}
	}

	// ノイズテクスチャから色を持ってくる
	float4 dissolve;
	dissolve = g_NoiceTexture.Sample(g_SamplerState, inTexCoord);

	// 閾値とノイズテクスチャのr値を比較
	if (threshold > dissolve.r)
	{	// 閾値以下の場所を描画する
		// 現れ始める縁を白く光らせる
		if ((color.a > 0.2f) && (threshold - 0.1f < dissolve.r))
		{
			color = 0.7f;
			color.ra = 1.0f;
		}
	}
	else
	{	// a値を0.0fにして透明化
		color.a = 0.0f;
	}

	outDiffuse = color;
}


// 杖用のシャドウマップ + ディゾルブピクセルシェーダ
// 影を実際に描画するピクセルシェーダー
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

	// 影処理

	// シャドウマップから値を取得
	// シャドウマップのスクリーン座標のどこのピクセル情報化を算出
	float2 shadowUV;
	shadowUV.x = 0.5f * inLightPos.x / inLightPos.w + 0.5f;
	shadowUV.y = 0.5f * inLightPos.y / inLightPos.w + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	shadowUV.x += 0.0008f;
	shadowUV.y += 0.0008f;

	// シャドウマップから深度(色)を抽出
	float4 shadowMap;
	shadowMap = g_TextureRenderTarget.Sample(g_SamplerState3, shadowUV);

	// 現在のピクセルの深度値を算出
	float depth = inLightPos.z / inLightPos.w;

	// シャドウマップの深度値と比較し、影を付ける
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


	// ノイズテクスチャから色を持ってくる
	float4 dissolve;
	dissolve = g_NoiceTexture.Sample(g_SamplerState, inTexCoord);

	// 閾値とノイズテクスチャのr値を比較
	if (threshold > dissolve.r)
	{	// 閾値以下の場所を描画する
		// 現れ始める縁を白く光らせる
		if ((color.a > 0.2f) && (threshold - 0.1f < dissolve.r))
		{
			color = 1.0;
		}
	}

	outDiffuse = color;
}



// バハムート用のディゾルブピクセルシェーダ
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

	//フォグ
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

	// ノイズテクスチャから色を持ってくる
	float4 dissolve;

	// ノイズテクスチャを更新
	dissolve = g_NoiceTexture.Sample(g_SamplerState, inTexCoord);

	float y = Circle.y + 180.0f;

	// 魔法陣よりも下にある部分をディゾルブで描画
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


// バハムートの魔法陣用のディゾルブピクセルシェーダ
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


	//フォグ
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


	// ノイズテクスチャから色を持ってくる
	float4 dissolve;
	dissolve = g_NoiceTexture1.Sample(g_SamplerState, inTexCoord);

	// 閾値とノイズテクスチャのr値を比較
	if (threshold > dissolve.r)
	{	// 閾値以下の場所を描画する
		// 現れ始める縁を白く光らせる
		//if ((color.a > 0.2f) && (threshold - 0.01f < dissolve.r))
		//{
		//	color = 0.7f;
		//	color.ra = 1.0f;
		//}
	}
	else
	{	// a値を0.0fにして透明化
		color.a = 0.0f;
	}

	outDiffuse = color;
}


// バハムートの波動用のディゾルブピクセルシェーダ
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


	//フォグ
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


	// ノイズテクスチャから色を持ってくる
	float4 dissolve;
	dissolve = g_NoiceTexture2.Sample(g_SamplerState, inTexCoord);

	// 基本透明
	color.a = 0.0f;

	// 閾値とノイズテクスチャのr値を比較
	if (threshold > dissolve.r)
	{	// 閾値以下の場所を描画する
		// 閾値周辺のみを描画
		if (threshold - 0.1f < dissolve.r)
		{
			color.a = 1.0f;
		}
	}

	// 波動を2ウェーブ描画
	float r = dissolve.r + 0.5f;
	if (r > 1.0f) r -= 1.0f;

	if (threshold > r )
	{	// 閾値以下の場所を描画する
		// 閾値周辺のみを描画
		if (threshold - 0.1f < r)
		{
			color.a = 1.0f;
		}
	}

	outDiffuse = color;
}


// パーティクル用のピクセルシェーダ
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


// 水魔法用のディゾルブピクセルシェーダ
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


	//フォグ
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


	// ノイズテクスチャから色を持ってくる
	float4 dissolve;
	dissolve = g_NoiceTexture3.Sample(g_SamplerState, inTexCoord);

	// 閾値とノイズテクスチャのr値を比較
	if (threshold > dissolve.r)
	{	// 閾値以下の場所を描画する
		// 閾値周辺のみを白く描画
		if (threshold - 0.05f < dissolve.r)
		{
			color.rgb = 1.0f;
		}
	}

	// 波紋を2ウェーブ描画
	float r = dissolve.r + 0.5f;
	if (r > 1.0f) r -= 1.0f;

	if (threshold > r)
	{	// 閾値以下の場所を描画する
		// 閾値周辺のみを白く描画
		if (threshold - 0.05f < r)
		{
			color.rgb = 1.0f;
		}
	}

	outDiffuse = color;

	//縁取り
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



// 爆風用のピクセルシェーダ
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

	// ある座標より上を透明にする
	float y = Circle.y;

	// 描画しようとしているピクセルのワールド座標が基準値よりも上の場合透明
	if (inWorldPos.y > y)
	{
		color.a = 0.0f;
	}

	outDiffuse = color;
}
