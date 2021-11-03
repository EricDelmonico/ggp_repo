#include "Sky.h"

Sky::Sky(
    std::shared_ptr<Mesh> mesh,
    std::shared_ptr<SimplePixelShader> pixelShader,
    std::shared_ptr<SimpleVertexShader> vertexShader,
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState,
    Microsoft::WRL::ComPtr<ID3D11Device> device)
    : mesh(mesh), pixelShader(pixelShader), vertexShader(vertexShader), samplerState(samplerState)
{
    // Set up rasterizer state
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_FRONT;
    device->CreateRasterizerState(&rastDesc, rasterizerState.GetAddressOf());

    // Set up depth stencil state
    D3D11_DEPTH_STENCIL_DESC depthStencDesc = {};
    depthStencDesc.DepthEnable = true;
    depthStencDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    device->CreateDepthStencilState(&depthStencDesc, depthStencilState.GetAddressOf());
}

Sky::~Sky()
{
}
