#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"

class Mesh
{
public:
    // Creates a mesh with the given information. This will create a vertex buffer and
    // an index buffer, and it will save the deviceContext and numIndices for future use.
    Mesh(
        Vertex* _vertices, 
        int _numVerts, 
        unsigned int* _indices, 
        int _numIndices, 
        Microsoft::WRL::ComPtr<ID3D11Device> _device, 
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> _deviceContext);

    ~Mesh();

    // Methods to retrieve the otherwise private vertex buffer, index buffer, and index count
    Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
    Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
    int GetIndexCount();

    // Handles the drawing of this mesh.
    void Draw();

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    int numIndices;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
};

