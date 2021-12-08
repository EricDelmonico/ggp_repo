#pragma once
#include <DirectXMath.h>

#define LIGHT_TYPE_DIRECTIONAL		0
#define LIGHT_TYPE_POINT			1
#define LIGHT_TYPE_SPOT				2

struct Light 
{
	int Type;						// Which kind of light? LIGHT_TYPE_[X]
	DirectX::XMFLOAT3 Direction;	// Directional/Spot
	float Range;					// Point/Spot attenuation
	DirectX::XMFLOAT3 Position;		// Point/Spot position in space
	float Intensity;				// All need light intesity
	DirectX::XMFLOAT3 Color;		// All need a color
	float SpotFalloff;				// Spot needs value to restrict cone
	int ShadowCasting;				// Whether the light should cast shadows
	DirectX::XMFLOAT2 Padding;		// Padding to hit 16-byte boundary

	// Creates a directional light with the given parameters
	Light(
		DirectX::XMFLOAT3 direction,
		DirectX::XMFLOAT3 color,
		float intensity,
		int shadowCasting = 0) :
		Type(LIGHT_TYPE_DIRECTIONAL),
		Direction(direction),
		Range(),
		Position(),
		Intensity(intensity),
		Color(color),
		SpotFalloff(),
		ShadowCasting(shadowCasting),
		Padding()
	{
	}

	// Creates a point light with the given parameters
	Light(
		DirectX::XMFLOAT3 position,
		DirectX::XMFLOAT3 color,
		float range,
		float intensity) :
		Type(LIGHT_TYPE_POINT),
		Direction(),
		Range(range),
		Position(position),
		Intensity(intensity),
		Color(color),
		SpotFalloff(),
		Padding()
	{
	}
};