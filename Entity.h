#pragma once
#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include "Material.h"
#include <memory>

class Entity
{
public:
    // Creates a new entity with the given mesh
    Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> material);
    ~Entity();

    // Returns a pointer to this Entity's Mesh
    Mesh* GetMesh();
    // Returns a pointer to this Entity's transform
    Transform* GetTransform();
    // Draws this Entity using its mesh and transform
    void Draw(
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
        Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer, 
        Camera* camera);
private:
    Transform transform;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
};

