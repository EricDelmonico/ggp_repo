#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

//
// VERT TO PIXEL--------------------------------------------------------
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
// VERTEX SHADER INPUT--------------------------------------------------
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
// LIGHTS STRUCT--------------------------------------------------------
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
// NON-PBR LIGHTING EQUATIONS-------------------------------------------
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



//
// PBR CONSTANTS -------------------------------------------------------
//

// Fresnel values
static const float F0_NON_METAL = 0.04f;
static const float F0_CHROME = 0.6f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f;

static const float PI = 3.14159265359f;



//
// PBR FUNCTIONS--------------------------------------------------------
//

// Lambert diffuse BRDF - Same as basic diffuse
// NOTE: function assumes vectors are already normalized
float DiffusePBR(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}

// Calculates diffuse amount while maintaining conservation of energy
//
// diffuse - diffuse amount
// specular - specular color
// metalness - surface metalness amount
float3 DiffuseEnergyConserve(float3 diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

// Trowbridge-Reitz/GGX
//
// a - roughness
// h - half vector
// n - normal
//
// Formula: D(h, n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float SpecDistribution(float a, float3 h, float3 n)
{
	// Calculate some of the terms
	float nDotH = saturate(dot(n, h));
	float nDotHSqr = nDotH * nDotH;
	a = a * a;
	float aSqr = max(a * a, MIN_ROUGHNESS); // Done after remapping a

	float denominator = PI * pow((nDotHSqr) * (aSqr - 1) + 1, 2);

	return aSqr / denominator;
}

// Fresnel Term - Schlick
//
// v - view vector
// h - half vector
// f0 - reflection factor for material when normal is straight on
//
// F(v, h, f0) = f0 + (1 - f0)(1 - (v dot h))^5
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	float vDotH = saturate(dot(v, h));
	return f0 + (1 - f0) * pow(1 - vDotH, 5);
}

// Geometric Shadowing - Schlick-GGX (Based on Schlick-Beckmann)
// -k is remapped to a / 2, roughness remapped to (r+1) / 2
//
// n - normal
// v - view vector
// roughness - roughness of material
//
// G(n, v) = (n dot v) / ((n dot v) * (1 - k) + k) 
float GeometricShadowing(float3 n, float3 v, float roughness)
{
	// Remap
	float k = pow(roughness + 1, 2) / 8.0f;
	float nDotV = saturate(dot(n, v));

	// Final value
	return nDotV / (nDotV * (1 - k) + k);
}

// Microfacet BRDF (Specular)
//
// f(l, v) = D(h)F(v, h)G(l, v, h) / 4(n dot l)(n dot v)
// - part of the denominator cancelled out by the numerator
//
// D() - Spec dist - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric shadowing - Schlick-GGX
//
// n - normal
// l - normalized direction to light
// v - view vector
// roughness - material roughness
// specColor - material's specular color
//
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 specColor)
{
	float3 h = normalize(v + l);

	// Get results from the various functions
	float D = SpecDistribution(roughness, h, n);
	float3 F = Fresnel(v, h, specColor);
	float G = GeometricShadowing(n, v, roughness) * GeometricShadowing(n, l, roughness);

	// Final formula
	// Denominator dot products partially cancelled by G()
	// (Page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdfreturn)
	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}



//
// PBR LIGHT HELPER FUNCTIONS-------------------------------------------
//

// Calculates directional light result for a PBR object
//
// n - normal
// camPos - position of the camera in the world
// worldPos - position of this pixel in the world
// roughness - material roughness
// specColor - material's specular color
// metalness - metalness value for this pixel
// surfaceColor - surface color at this pixel
//
float3 DirLightPBR(Light light, float3 n, float3 camPos, float3 worldPos, float roughness, float3 specColor, float metalness, float3 surfaceColor)
{
	// Calculate l and v
	float3 l = -light.Direction;
	float3 v = normalize(camPos - worldPos);

	// Get diffuse and spec amounts
	float diff = DiffusePBR(n, l);
	float3 spec = MicrofacetBRDF(n, l, v, roughness, specColor);

	// Calculate diffuse taking conservation of energy into account
	// (Reflected light does not get diffused)
	float3 balancedDiff = DiffuseEnergyConserve(diff, spec, metalness);

	// Combine the total diffuse and spec values for this light
	return (balancedDiff * surfaceColor + spec) * light.Intensity * light.Color;
}

// Calculates point light result for a PBR object
//
// n - normal
// camPos - position of the camera in the world
// worldPos - position of this pixel in the world
// roughness - material roughness
// specColor - material's specular color
// metalness - metalness value for this pixel
// surfaceColor - surface color at this pixel
//
float3 PointLightPBR(Light light, float3 n, float3 camPos, float3 worldPos, float roughness, float3 specColor, float metalness, float3 surfaceColor)
{
	// Calculate l and v vector (direction to the light and direction to the camera
	float3 l = normalize(light.Position - worldPos);
	float3 v = normalize(camPos - worldPos);

	// Get diffuse and spec amounts, along with the attenuation since this is a point light
	float diff = DiffusePBR(n, l);
	float3 spec = MicrofacetBRDF(n, l, v, roughness, specColor);
	float atten = Attenuate(light, worldPos);

	// Calculate diffuse taking conservation of energy into account
	// (Reflected light does not get diffused)
	float3 balancedDiff = DiffuseEnergyConserve(diff, spec, metalness);

	// Combine the total diffuse and spec values for this light
	return (balancedDiff * surfaceColor + spec) * atten * light.Intensity * light.Color;
}

#endif