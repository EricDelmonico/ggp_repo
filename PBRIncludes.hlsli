//
// CONSTANTS 
//

// Fresnel values
static const float F0_NON_METAL = 0.04f;
static const float F0_CHROME = 0.6f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f;

static const float PI = 3.14159265359f;



//
// PBR FUNCTIONS
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
// l - light direction
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