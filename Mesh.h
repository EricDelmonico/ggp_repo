#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"

class Mesh
{
public:
    Mesh(
        Vertex* _vertices, 
        int _numVerts, 
        unsigned int* _indices, 
        int _numIndices, 
        Microsoft::WRL::ComPtr<ID3D11Device> _device, 
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> _deviceContext);

    ~Mesh();

    Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
    Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
    int GetIndexCount();
    void Draw();

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    int numIndices;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
};

