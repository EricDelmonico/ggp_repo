#pragma once
#include <DirectXMath.h>
#include "SimpleShader.h"
#include <memory>
#include <d3d11.h>
#include <unordered_map>
#include <wrl/client.h>
#include "Camera.h"

class Material
{
public:
    Material(
        DirectX::XMFLOAT4 colorTint,
        std::shared_ptr<SimplePixelShader> pixelShader,
        std::shared_ptr<SimpleVertexShader> vertexShader,
        DirectX::XMFLOAT2 uvScale = { 1, 1 },
        DirectX::XMFLOAT2 uvOffset = { 0, 0 });
    ~Material();

    DirectX::XMFLOAT4* GetColorTint();
    SimplePixelShader* GetPixelShader();
    SimpleVertexShader* GetVertexShader();
    DirectX::XMFLOAT2 GetUvScale();
    DirectX::XMFLOAT2 GetUvOffset();

    void SetColorTint(DirectX::XMFLOAT4 colorTint);
    void SetPixelShader(std::shared_ptr<SimplePixelShader> pShader);
    void SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader);
    void SetUvScale(float u, float v);
    void SetUvOffset(float u, float v);

    void AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv);
    void AddSampler(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

    void PrepareForDraw(Camera& camera, float totalTime, Transform& transform, Camera& lightCamera);

private:
    DirectX::XMFLOAT4 colorTint;
    DirectX::XMFLOAT2 uvScale;
    DirectX::XMFLOAT2 uvOffset;
    std::shared_ptr<SimplePixelShader> pixelShader;
    std::shared_ptr<SimpleVertexShader> vertexShader;

    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

    // Skybox only needed for some shaders
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
};

