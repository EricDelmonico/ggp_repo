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
void Entity::Draw(Camera* camera, float totalTime)
{
	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	SimpleVertexShader* vs = material->GetVertexShader();
	SimplePixelShader* ps = material->GetPixelShader();

	// Set vertex shader data
	vs->SetShader();
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	vs->CopyAllBufferData();

	// Set pixel shader data
	ps->SetShader();
	ps->SetFloat4("colorTint", *material->GetColorTint());
	ps->SetFloat("totalTime", totalTime);
	ps->SetFloat3("cameraPos", camera->GetTransform()->GetPosition());
	ps->SetFloat("roughness", material->GetRoughness());
	ps->CopyAllBufferData();

	mesh->Draw();
}
