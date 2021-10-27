#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 worldPosition	: POSITION;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;		// normal for this vertex
	float2 uv				: TEXCOORD;		// uv coordinates
};

// Light stuff
#define LIGHT_TYPE_DIRECTIONAL		0
#define LIGHT_TYPE_POINT			1
#define LIGHT_TYPE_SPOT				2

#define MAX_SPECULAR_EXPONENT		256.0f;

struct Light
{
	int Type;						// Which kind of light? LIGHT_TYPE_[X]
	float3 Direction;				// Directional/Spot
	float Range;					// Point/Spot attenuation
	float3 Position;				// Point/Spot position in space
	float Intensity;				// All need light intesity
	float3 Color;					// All need a color
	float SpotFalloff;				// Spot needs value to restrict cone
	float3 Padding;					// Padding to hit 16-byte boundary
};

// normal and dirToLight must be normalized
float Diffuse(float3 normal, float3 dirToLight, float intensity)
{
	return saturate(dot(normal, dirToLight)) * intensity;
}

float Specular(
	float3 cameraPos,
	float3 pixWorldPos,
	float3 lightDir,
	float3 normal,
	float roughness,
	float lightIntensity)
{
	if (roughness == 1)
	{
		return 0;
	}
	
	lightDir = normalize(lightDir);
	float3 V = normalize(cameraPos - pixWorldPos);
	float3 R = reflect(lightDir, normal);
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float spec = pow(saturate(dot(R, V)), specExponent) * lightIntensity;
	return spec;
}

float3 DirectionalLight(
	Light light,
	VertexToPixel input, 
	float3 cameraPos,
	float roughness,
	float surfaceColor,
	float specMapValue)
{
	float3 dirToLight = normalize(-light.Direction);
	float diffuse = Diffuse(input.normal, dirToLight, light.Intensity);
	float specular =
		Specular(
			cameraPos,
			input.worldPosition,
			light.Direction,
			input.normal,
			roughness,
			light.Intensity) * specMapValue;

	return (diffuse + specular) * light.Color * surfaceColor;
}

float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

float3 PointLight(
	Light light,
	VertexToPixel input,
	float3 cameraPos,
	float3 roughness,
	float3 surfaceColor,
	float specMapValue)
{
	float3 dirToLight = normalize(light.Position - input.worldPosition);
	float diffuse = Diffuse(input.normal, dirToLight, light.Intensity);
	float specular =
		Specular(
			cameraPos,
			input.worldPosition,
			-dirToLight,
			input.normal,
			roughness,
			light.Intensity) * specMapValue;

	float3 lightResult = (diffuse + specular) * light.Color * surfaceColor;
	float attenuation = Attenuate(light, input.worldPosition);
	return lightResult * attenuation;
}

#endif