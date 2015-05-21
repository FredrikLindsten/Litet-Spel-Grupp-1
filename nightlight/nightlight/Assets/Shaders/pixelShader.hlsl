Texture2D AssetTexture : register(t0);
Texture2D ShadowMap : register(t1);

SamplerState sampleStateClamp : register(s0);
SamplerState sampleStateWrap : register(s1);
SamplerState sampleStateComparison : register(s2);

cbuffer lightBuffer : register(cb0)
{
	matrix lightView;
	matrix lightProj;

	float3 lightPosSpot;
	float  lightRangeSpot;
	float3 lightDirSpot;
	float  lightConeSpot;
	float3 lightAttSpot;
	float4 lightAmbientSpot;
	float4 lightDiffuseSpot;

	//door point light
	float3 lightPosPoint;
	float4 lightDiffusePoint;

	//player point light
	float3 lightPosPoint2;
	float4 lightDiffusePoint2;
};

struct pixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;

	float4 worldPos : TEXCOORD1;
	float3 viewDir : POSITION;
	float3 colorModifier : COLORMODIFIER;
};

float4 pixelShader(pixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	
	float3 wp = input.worldPos.xyz;
	float3 reflection;
	float4 specular = float4(0.0f, 1.0f, 0.0f, 1.0f);
	float3 finalColor = float3(0.0f, 0.0f, 0.0f);

	float4 diffuse = AssetTexture.Sample(sampleStateClamp, input.tex);
	float3 finalAmbient = (diffuse * lightAmbientSpot).xyz;

	//calculate light to pixel vector for spotlight
	float3 lightToPixelVec = lightPosSpot - wp;;
	float d = length(lightToPixelVec);
	lightToPixelVec /= d;

	//Sample and add shadows for the shadow map.
	float4 lightSpacePos = input.worldPos;
	lightSpacePos = mul(lightSpacePos, lightView);
	lightSpacePos = mul(lightSpacePos, lightProj);

	float howMuchLight = dot(lightToPixelVec, input.normal);
	if (howMuchLight > 0.0f)
	{
		
		float2 smTex;
		smTex.x = 0.5f + (lightSpacePos.x / lightSpacePos.w * 0.5f);
		smTex.y = 0.5f - (lightSpacePos.y / lightSpacePos.w * 0.5f);

		float depth = lightSpacePos.z / lightSpacePos.w;

		float dx = 1.0f / 2048;
		float s0 = ShadowMap.Sample(sampleStateClamp, smTex).r;
		float s1 = ShadowMap.Sample(sampleStateClamp, smTex + float2(dx, 0.0f)).r;
		float s2 = ShadowMap.Sample(sampleStateClamp, smTex + float2(0.0f, dx)).r;
		float s3 = ShadowMap.Sample(sampleStateClamp, smTex + float2(dx, dx)).r;

		float epsilon = 0.001f;
		float2 texelPos = smTex * 2048;

		float2 lerps = frac(texelPos);
		float shadowCoeff = lerp(lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);

		if (shadowCoeff < depth - epsilon)
		{
			finalColor = saturate(finalColor * shadowCoeff);
		}
		else
		{
			finalColor += (diffuse * lightDiffuseSpot).xyz;																			//Add light to the finalColor of the pixel
			finalColor /= (lightAttSpot[0] + (lightAttSpot[1] * (d* d* d) / 4.5) + (lightAttSpot[2] * (d * d * d * d) / 4.5));		//Calculate Light's Distance Falloff factor
			finalColor *= pow(max(dot(-lightToPixelVec, lightDirSpot), 0.0f), lightConeSpot);										//Calculate falloff from center to edge of pointlight cone
		}
	}

	//Calculate the point lights directions
	float3 pointLightDir = normalize(wp - lightPosPoint);
	float3 pointLightDir2 = normalize(wp - lightPosPoint2);

	float3 diffuseLighting = saturate(dot(input.normal, -pointLightDir));
	float3 diffuseLighting2 = saturate(dot(input.normal, -pointLightDir2));

	//Add the two point lights and calculate falloff
	diffuseLighting *= (1) / dot(lightPosPoint - wp, lightPosPoint - wp);
	diffuseLighting2 *= (5) / dot(lightPosPoint2 - wp, lightPosPoint2 - wp);

	finalColor = saturate(finalColor + finalAmbient);
	finalColor += (diffuseLighting *  lightDiffusePoint);
	finalColor += (diffuse *(diffuseLighting2 * lightDiffusePoint2));

	if (diffuseLighting.x > 0 || diffuseLighting.y > 0 || diffuseLighting.z > 0)
	{
		reflection = normalize(2 * diffuseLighting * input.normal + pointLightDir);
		reflection = normalize(2 * diffuseLighting2 * input.normal + pointLightDir2);
		reflection *= normalize(2 * diffuseLighting * input.normal + lightDirSpot);
		
		//Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
		specular = pow(saturate(dot(reflection, input.viewDir)), 20);
		finalColor += (specular);
	}
	
	//Return Final Color
	return float4(finalColor += input.colorModifier, diffuse.a);
}