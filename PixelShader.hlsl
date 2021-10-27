#include "ShaderIncludes.hlsli"

#define LIGHT_COUNT 5

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float totalTime;
	float3 cameraPos;
	float roughness;
	float3 ambient;
	float2 uvScale;
	float2 uvOffset;
	Light lights[LIGHT_COUNT];
}

Texture2D SurfaceTexture	: register(t0);
Texture2D SpecularMap		: register(t1);
SamplerState BasicSampler	: register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Make sure input normal is normalized before using it for anything
	input.normal = normalize(input.normal);

	// Scale UV
	input.uv = float2(input.uv.x * uvScale.x + uvOffset.x, input.uv.y * uvScale.y + uvOffset.y);

	// Get surface color and roughness from texture
	float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
	float specMapValue = SpecularMap.Sample(BasicSampler, input.uv).r;

	// Loop through lights and calculate each light's result
	float3 finalLightResult = 0.0f;
	for (int i = 0; i < LIGHT_COUNT; i++)
	{
		switch (lights[i].Type)
		{
		case LIGHT_TYPE_DIRECTIONAL:
			finalLightResult += DirectionalLight(lights[i], input, cameraPos, roughness, surfaceColor, specMapValue);
			break;
		case LIGHT_TYPE_POINT:
			finalLightResult += PointLight(lights[i], input, cameraPos, roughness, surfaceColor, specMapValue);
			break;
		}
	}

	float3 finalColor = finalLightResult + (colorTint * ambient);

	return float4(finalColor, 1);
}