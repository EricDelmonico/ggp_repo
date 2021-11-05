#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
};

VertexToPixel_Sky main(VertexShaderInput input)
{
    VertexToPixel_Sky output;

    // Remove translation from view matrix
    matrix viewCopy = view;
    viewCopy._14 = 0;
    viewCopy._24 = 0;
    viewCopy._34 = 0;

    output.position = mul(mul(projection, viewCopy), input.localPosition);

    // Depth will always be 1
    output.position.z = 1;

    // Sample direction is input position since sky is centered at origin
    output.sampleDir = input.localPosition;

    return output;
}