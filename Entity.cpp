#include "Entity.h"
#include "BufferStructs.h"

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

// Draws this Entity using its mesh and transform
void Entity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer,
	Camera* camera)
{
	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	context->VSSetShader(material->GetVertexShader().Get(), 0, 0);
	context->PSSetShader(material->GetPixelShader().Get(), 0, 0);

	// Set cbuffer data
	VertexShaderExternalData vsData;
	//vsData.colorTint = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.colorTint = *material->GetColorTint();
	vsData.worldMatrix = transform.GetWorldMatrix();
	vsData.viewMatrix = camera->GetView();
	vsData.projectionMatrix = camera->GetProjection();

	// Copy cbuffer data to the resource
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	context->Unmap(vsConstantBuffer.Get(), 0);
	// Bind cbuffer
	context->VSSetConstantBuffers(
		0, // Which slot/register to bind the buffer to?
		1, // How many are we activating?
		vsConstantBuffer.GetAddressOf());

	mesh->Draw();
}
