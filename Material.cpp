#include "Material.h"

Material::Material(
    DirectX::XMFLOAT4 colorTint,
    std::shared_ptr<SimplePixelShader> pixelShader,
    std::shared_ptr<SimpleVertexShader> vertexShader)
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

SimplePixelShader* Material::GetPixelShader()
{
    return pixelShader.get();
}

SimpleVertexShader* Material::GetVertexShader()
{
    return vertexShader.get();
}

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint)
{
    this->colorTint = colorTint;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pShader)
{
    pixelShader = std::shared_ptr<SimplePixelShader>(pShader);
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader)
{
    vertexShader = std::shared_ptr<SimpleVertexShader>(vShader);
}
