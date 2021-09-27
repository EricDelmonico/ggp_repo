#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>

class Material
{
public:
    Material(
        DirectX::XMFLOAT4 colorTint, 
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader,
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader);
    ~Material();

    DirectX::XMFLOAT4* GetColorTint();
    Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
    Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();

    void SetColorTint(DirectX::XMFLOAT4 colorTint);
    void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> pShader);
    void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vShader);

private:
    DirectX::XMFLOAT4 colorTint;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
};

