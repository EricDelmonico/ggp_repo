#pragma once
#include "Mesh.h"
#include "Transform.h"
#include <memory>

class Entity
{
public:
    // Creates a new entity with the given mesh
    Entity(Mesh* _mesh);
    ~Entity();

    // Returns a pointer to this Entity's Mesh
    Mesh* GetMesh();
    // Returns a pointer to this Entity's transform
    Transform* GetTransform();
    // Draws this Entity using its mesh and transform
    void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer);
private:
    Transform transform;
    Mesh* mesh;
};

