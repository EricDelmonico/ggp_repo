#include "Transform.h"

using namespace DirectX;
Transform::Transform()
{
    position = XMFLOAT3(0, 0, 0);
    pitchYawRoll = XMFLOAT3(0, 0, 0);
    scale = XMFLOAT3(1, 1, 1);

    XMMATRIX ident = XMMatrixIdentity();
    XMStoreFloat4x4(&worldMatrix, ident);
    XMStoreFloat4x4(&worldInverseTransposeMatrix, ident);

    matricesDirty = false;
}

Transform::~Transform()
{
}

// TODO use worldMatrix for below getters

DirectX::XMFLOAT3 Transform::GetUp()
{
    // Take the world up vector and rotate it by out own rotation values
    XMVECTOR localUp = XMVector3Rotate(
        XMVectorSet(0, 1, 0, 0),
        XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));

    XMFLOAT3 up;
    XMStoreFloat3(&up, localUp);
    return up;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
    // Take the world right vector and rotate it by out own rotation values
    XMVECTOR localRight = XMVector3Rotate(
        XMVectorSet(1, 0, 0, 0),
        XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));

    XMFLOAT3 right;
    XMStoreFloat3(&right, localRight);
    return right;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
    // Take the world forward vector and rotate it by out own rotation values
    XMVECTOR localForward = XMVector3Rotate(
        XMVectorSet(0, 0, 1, 0),
        XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));

    XMFLOAT3 forward;
    XMStoreFloat3(&forward, localForward);
    return forward;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
    return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
    return pitchYawRoll;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
    return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
    UpdateMatrices();
    return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
    UpdateMatrices();
    return worldInverseTransposeMatrix;
}

void Transform::SetPosition(float x, float y, float z)
{
    position = XMFLOAT3(x, y, z);

    matricesDirty = true;
}

void Transform::SetPitchYawRoll(float pitch, float yaw, float roll)
{
    pitchYawRoll = XMFLOAT3(pitch, yaw, roll);

    matricesDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
    scale = XMFLOAT3(x, y, z);

    matricesDirty = true;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
    position.x += x;
    position.y += y;
    position.z += z;

    matricesDirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
    XMVECTOR rotatedVector = XMVector3Rotate(
        XMVectorSet(x, y, z, 0), 
        XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));

    // Overwrite position with current pos + rotated movement
    XMStoreFloat3(
        &position,
        XMLoadFloat3(&position) + rotatedVector);
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
    pitchYawRoll.x += pitch;
    pitchYawRoll.y += yaw;
    pitchYawRoll.z += roll;

     matricesDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
    scale.x *= x;
    scale.y *= y;
    scale.z *= z;

    matricesDirty = true;
}

void Transform::UpdateMatrices()
{
    // If matrices aren't dirty, leave
    if (!matricesDirty)
        return;

    // TODO: from vector
    // Actually update the matrices by creating the individual transformation 
    // matrices and combining
    XMMATRIX transMat   = XMMatrixTranslation(position.x, position.y, position.z);
    XMMATRIX rotMat     = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
    XMMATRIX scaleMat   = XMMatrixScaling(scale.x, scale.y, scale.z);

    // Combine into a single matrix that represents all transformations
    // and store the results
    XMMATRIX worldMat = scaleMat * rotMat * transMat; // SRT
    XMStoreFloat4x4(&worldMatrix, worldMat);

    // While we're at it, create the inverse transpose matrix
    XMStoreFloat4x4(
        &worldInverseTransposeMatrix,
        XMMatrixInverse(0, XMMatrixTranspose(worldMat))
    );


    // Clean once again!
    matricesDirty = false;
}
