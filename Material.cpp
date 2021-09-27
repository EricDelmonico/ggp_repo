#include "Material.h"

Material::Material(
    DirectX::XMFLOAT4 colorTint,
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader,
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader)
    : colorTint(colorTint), pixelShader(pixelShader), vertexShader(vertexShader)
{
}

Material::~Material()
{
}

DirectX::XMFLOAT4* Material::GetColorTint()
{
    return &colorTint;
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> Material::GetPixelShader()
{
    return pixelShader;
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Material::GetVertexShader()
{
    return vertexShader;
}

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint)
{
    this->colorTint = colorTint;
}

void Material::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> pShader)
{
    pixelShader = pShader;
}

void Material::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vShader)
{
    vertexShader = vShader;
}
