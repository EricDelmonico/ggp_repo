#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

class Sky
{
public:
    Sky(std::shared_ptr<Mesh> mesh,
        std::shared_ptr<SimplePixelShader> pixelShader,
        std::shared_ptr<SimpleVertexShader> vertexShader,
        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState,
        Microsoft::WRL::ComPtr<ID3D11Device> device);

    ~Sky();

    void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera);

private:
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<SimplePixelShader> pixelShader;
    std::shared_ptr<SimpleVertexShader> vertexShader;
};

