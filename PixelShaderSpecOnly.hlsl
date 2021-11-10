#include "ShaderIncludes.hlsli"

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
	float3 surfaceColor = pow(SurfaceTexture.Sample(BasicSampler, input.uv).rgb, 2.2f);
	float specMapValue = SpecularMap.Sample(BasicSampler, input.uv).r;

	// Loop through lights and calculate each light's result
	float3 finalLightResult =
		LightingLoop(
			lights,
			input.normal,
			input.worldPosition,
			cameraPos,
			roughness,
			surfaceColor,
			specMapValue);

	float3 finalColor = finalLightResult + (colorTint.rgb * ambient);

	return float4(pow(finalColor, 1.0f / 2.2f), 1);
}