#include "ShaderIncludes.hlsli"

#define LIGHT_COUNT 5

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float totalTime;
	float3 cameraPos;
	float roughness;
	float3 ambient;
	Light lights[LIGHT_COUNT];
}


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

	// Loop through lights and calculate each light's result
	float3 finalLightResult = 0.0f;
	for (int i = 0; i < LIGHT_COUNT; i++)
	{
		switch (lights[i].Type)
		{
		case LIGHT_TYPE_DIRECTIONAL:
			finalLightResult += DirectionalLight(lights[i], input, cameraPos, roughness, colorTint);
			break;
		case LIGHT_TYPE_POINT:
			finalLightResult += PointLight(lights[i], input, cameraPos, roughness, colorTint);
			break;
		}
	}

	float3 finalColor = finalLightResult + (colorTint * ambient);

	return float4(finalColor, 1);
}