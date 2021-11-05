#include "Sky.h"

Sky::Sky(
    std::shared_ptr<Mesh> mesh,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv,
    std::shared_ptr<SimplePixelShader> pixelShader,
    std::shared_ptr<SimpleVertexShader> vertexShader,
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState,
    Microsoft::WRL::ComPtr<ID3D11Device> device)
    : mesh(mesh), srv(srv), pixelShader(pixelShader), vertexShader(vertexShader), samplerState(samplerState)
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

void Sky::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera)
{
    // Change render states
    context->RSSetState(rasterizerState.Get());
    context->OMSetDepthStencilState(depthStencilState.Get(), 0);

    // Set up sky shaders for drawing
    vertexShader->SetShader();
    vertexShader->SetMatrix4x4("view", camera->GetView());
    vertexShader->SetMatrix4x4("projection", camera->GetProjection());
    vertexShader->CopyAllBufferData();

    pixelShader->SetShader();
    pixelShader->SetShaderResourceView("SkyTexture", srv);
    pixelShader->SetSamplerState("BasicSampler", samplerState);
    pixelShader->CopyAllBufferData();

    // Draw mesh
    mesh->Draw();

    // Reset render states
    context->RSSetState(nullptr);
    context->OMSetDepthStencilState(nullptr, 0);
}
