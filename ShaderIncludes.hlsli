#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

//
// VERT TO PIXEL
//

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

struct VertexToPixel_NormalMap
{
	float4 screenPosition	: SV_POSITION;
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 worldPosition	: POSITION;
	float3 tangent			: TANGENT;
};

struct VertexToPixel_Sky 
{
	float4 position			: SV_POSITION;
	float3 sampleDir		: DIRECTION;
};



//
// VERTEX SHADER INPUT
//

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
	float3 tangent			: TANGENT;		// tangent for this vertex
};



//
// LIGHT DEFINES
//

#define LIGHT_TYPE_DIRECTIONAL		0
#define LIGHT_TYPE_POINT			1
#define LIGHT_TYPE_SPOT				2

#define MAX_SPECULAR_EXPONENT		256.0f

#define LIGHT_COUNT					5

//
// LIGHTS STRUCT
//

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



//
// LIGHTING EQUATIONS
//

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

float3 LightResult(float diffuse, float specular, float3 lightColor, float3 surfaceColor) 
{
	specular *= any(diffuse);
	return (diffuse + specular) * lightColor * surfaceColor;
}

float3 DirectionalLight(
	Light light,
	float3 normal,
	float3 worldPosition,
	float3 cameraPos,
	float roughness,
	float3 surfaceColor,
	float specMapValue)
{
	float3 dirToLight = normalize(-light.Direction);
	float diffuse = Diffuse(normal, dirToLight, light.Intensity);
	float specular =
		Specular(
			cameraPos,
			worldPosition,
			light.Direction,
			normal,
			roughness,
			light.Intensity) * specMapValue;

	return LightResult(diffuse, specular, light.Color, surfaceColor);
}

float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

float3 PointLight(
	Light light,
	float3 normal,
	float3 worldPosition,
	float3 cameraPos,
	float roughness,
	float3 surfaceColor,
	float specMapValue)
{
	float3 dirToLight = normalize(light.Position - worldPosition);
	float diffuse = Diffuse(normal, dirToLight, light.Intensity);
	float specular =
		Specular(
			cameraPos,
			worldPosition,
			-dirToLight,
			normal,
			roughness,
			light.Intensity) * specMapValue;

	float3 lightResult = LightResult(diffuse, specular, light.Color, surfaceColor);
	float attenuation = Attenuate(light, worldPosition);
	return lightResult * attenuation;
}

float3 LightingLoop(
	Light lights[LIGHT_COUNT], 
	float3 normal,
	float3 worldPosition,
	float3 cameraPos, 
	float roughness, 
	float3 surfaceColor, 
	float specMapValue)
{
	// Loop through lights and calculate each light's result
	float3 finalLightResult = 0.0f;
	for (int i = 0; i < LIGHT_COUNT; i++)
	{
		int type = lights[i].Type;
		if (type == LIGHT_TYPE_DIRECTIONAL)
		{
			finalLightResult += DirectionalLight(lights[i], normal, worldPosition, cameraPos, roughness, surfaceColor, specMapValue);
		}
		if (type == LIGHT_TYPE_POINT) 
		{
			finalLightResult += PointLight(lights[i], normal, worldPosition, cameraPos, roughness, surfaceColor, specMapValue);
		}
	}

	return finalLightResult;
}

// Returns reflection coefficient based on Schlick approx.
//
// R0 - reflection factor for material when normal is straight on
// normal - surface normal of object
// dirFromCamera - the direction from the pixel's position to the camera
//
float SkyReflectionFresnel(float R0, float3 normal, float3 dirFromCamera)
{
	float3 cosTheta = saturate(dot(normal, -dirFromCamera));
	return R0 + (1 - R0) * pow(1 - cosTheta, 5);
}

#endif