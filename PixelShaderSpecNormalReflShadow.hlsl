#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float totalTime;
    float3 cameraPos;
    float3 ambient;
    float _;
    float2 uvScale;
    float2 uvOffset;
    Light lights[LIGHT_COUNT];
}

Texture2D Albedo        	: register(t0);
Texture2D NormalMap 		: register(t1);
Texture2D RoughnessMap		: register(t2);
Texture2D MetalnessMap		: register(t3);
TextureCube SkyTexture      : register(t4);
Texture2D ShadowMap         : register(t5);
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
float4 main(VertexToPixel_NormalMapShadowMap input) : SV_TARGET
{
    //
    // NORMAL SAMPLING
    // 

    // Make sure input normal and tangent are normalized before using them for anything
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);

    input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(input.tangent, input.normal);
    float3x3 TBN = float3x3(input.tangent, B, input.normal);

    // Scale UV
    input.uv = float2(input.uv.x * uvScale.x + uvOffset.x, input.uv.y * uvScale.y + uvOffset.y);

    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2.0f - 1.0f;
    input.normal = mul(unpackedNormal, TBN);

    //
    // ALBEDO SAMPLING
    //

    // Get surface color and roughness from texture
    float3 surfaceColor = Albedo.Sample(BasicSampler, input.uv).rgb;
    // Texture needs to be reverse-gamma-corrected since gamma correction happens later
    surfaceColor = pow(surfaceColor, 2.2f);
    surfaceColor *= colorTint;

    //
    // METALNESS/ROUGHNESS SAMPLING
    //

    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

    // Determine specular color based on metalness
    float3 specColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metalness);

    //
    // LIGHT CALCULATIONS
    //

    // Loop through lights and calculate each light's result
    float3 finalLightResult = (0.0f).rrr;
    for (int i = 0; i < LIGHT_COUNT; i++)
    {
        int type = lights[i].Type;
        if (type == LIGHT_TYPE_DIRECTIONAL)
        {
            finalLightResult += DirLightPBR(lights[i], input.normal, cameraPos, input.worldPosition, roughness, specColor, metalness, surfaceColor);
        }
        if (type == LIGHT_TYPE_POINT)
        {
            finalLightResult += PointLightPBR(lights[i], input.normal, cameraPos, input.worldPosition, roughness, specColor, metalness, surfaceColor);
        }
    }

    float3 finalColor = finalLightResult + (surfaceColor.rgb * ambient);

    //
    // ENVIRONMENT MAP/GAMMA CALCULATIONS
    //

    // Take care of skybox reflection
    float3 dirFromCamera = normalize(input.worldPosition - cameraPos);
    float3 skySample = SkyTexture.Sample(BasicSampler, reflect(dirFromCamera, input.normal)).rgb;

    // Gamma correct finalColor before lerping between it and the already gamma-correct sky sample
    finalColor = pow(finalColor, 1.0f / 2.2f);

    // Lerp between surface color up to this point and the sampled sky color based on fresnel term
    finalColor = lerp(finalColor, skySample, SkyReflectionFresnel(F0_NON_METAL, input.normal, dirFromCamera));

    return float4(finalColor, 1);
}