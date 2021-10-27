#include "Entity.h"

using namespace DirectX;

// Creates a new entity with the given mesh
Entity::Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> material)
	: mesh(_mesh), material(material)
{
	transform = Transform();
}

Entity::~Entity()
{
}

// Returns a pointer to this Entity's Mesh
Mesh* Entity::GetMesh()
{
    return mesh.get();
}

// Returns a pointer to this Entity's transform
Transform* Entity::GetTransform()
{
    return &transform;
}

Material* Entity::GetMaterial()
{
	return material.get();
}

// Draws this Entity using its mesh and transform
void Entity::Draw(Camera& camera, float totalTime)
{
	material->PrepareForDraw(camera, totalTime, transform);

	mesh->Draw();
}
