#include "Material.h"

Material::Material(
    DirectX::XMFLOAT4 colorTint,
    float roughness,
    std::shared_ptr<SimplePixelShader> pixelShader,
    std::shared_ptr<SimpleVertexShader> vertexShader,
    DirectX::XMFLOAT2 uvScale,
    DirectX::XMFLOAT2 uvOffset)
    : colorTint(colorTint), roughness(roughness), pixelShader(pixelShader), vertexShader(vertexShader), uvScale(uvScale), uvOffset(uvOffset)
{
}

Material::~Material()
{
}

DirectX::XMFLOAT4* Material::GetColorTint()
{
    return &colorTint;
}

float Material::GetRoughness()
{
    return roughness;
}

SimplePixelShader* Material::GetPixelShader()
{
    return pixelShader.get();
}

SimpleVertexShader* Material::GetVertexShader()
{
    return vertexShader.get();
}

DirectX::XMFLOAT2 Material::GetUvScale()
{
    return uvScale;
}

DirectX::XMFLOAT2 Material::GetUvOffset()
{
    return uvOffset;
}

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint)
{
    this->colorTint = colorTint;
}

void Material::SetRoughness(float r)
{
    roughness = r;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pShader)
{
    pixelShader = std::shared_ptr<SimplePixelShader>(pShader);
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader)
{
    vertexShader = std::shared_ptr<SimpleVertexShader>(vShader);
}

void Material::SetUvScale(float u, float v)
{
    uvScale = { u, v };
}

void Material::SetUvOffset(float u, float v)
{
    uvOffset = { u, v };
}

void Material::AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
    textureSRVs.insert({ shaderName, srv });
}

void Material::AddSampler(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
    samplers.insert({ shaderName, sampler });
}

void Material::PrepareForDraw(Camera& camera, float totalTime, Transform& transform)
{
    // Set vertex shader data
    vertexShader->SetShader();
    vertexShader->SetMatrix4x4("world", transform.GetWorldMatrix());
    vertexShader->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());
    vertexShader->SetMatrix4x4("view", camera.GetView());
    vertexShader->SetMatrix4x4("projection", camera.GetProjection());
    vertexShader->CopyAllBufferData();

    // Set pixel shader data
    pixelShader->SetShader();
    pixelShader->SetFloat4("colorTint", colorTint);
    pixelShader->SetFloat("totalTime", totalTime);
    pixelShader->SetFloat3("cameraPos", camera.GetTransform()->GetPosition());
    pixelShader->SetFloat2("uvScale", uvScale);
    pixelShader->SetFloat2("uvOffset", uvOffset);
    pixelShader->SetFloat("roughness", roughness);
    // Set up textures
    for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first, t.second); }
    for (auto& s : samplers) { pixelShader->SetSamplerState(s.first, s.second); }
    pixelShader->CopyAllBufferData();
}
