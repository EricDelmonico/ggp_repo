#include "Entity.h"
#include "BufferStructs.h"

using namespace DirectX;

// Creates a new entity with the given mesh
Entity::Entity(Mesh* _mesh)
{
    mesh = _mesh;
	transform = Transform();
}

Entity::~Entity()
{
}

// Returns a pointer to this Entity's Mesh
Mesh* Entity::GetMesh()
{
    return mesh;
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
	// Set cbuffer data
	VertexShaderExternalData vsData;
	//vsData.colorTint = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.colorTint = XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f);
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
