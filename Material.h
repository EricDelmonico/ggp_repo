#pragma once
#include <DirectXMath.h>
#include "SimpleShader.h"
#include <memory>
#include <d3d11.h>

class Material
{
public:
    Material(
        DirectX::XMFLOAT4 colorTint, 
        std::shared_ptr<SimplePixelShader> pixelShader,
        std::shared_ptr<SimpleVertexShader> vertexShader);
    ~Material();

    DirectX::XMFLOAT4* GetColorTint();
    SimplePixelShader* GetPixelShader();
    SimpleVertexShader* GetVertexShader();

    void SetColorTint(DirectX::XMFLOAT4 colorTint);
    void SetPixelShader(std::shared_ptr<SimplePixelShader> pShader);
    void SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader);

private:
    DirectX::XMFLOAT4 colorTint;
    std::shared_ptr<SimplePixelShader> pixelShader;
    std::shared_ptr<SimpleVertexShader> vertexShader;
};

