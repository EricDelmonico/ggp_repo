#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

float4 main(VertexShaderInput input) : SV_POSITION
{
	// Only need the screen position to get the depth buffer.
	// There's not a pixel shader so no need for the output struct either.
	matrix wvp = mul(projection, mul(view, world));
	float4 screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	return screenPosition;
}