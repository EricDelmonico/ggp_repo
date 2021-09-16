#pragma once
#include <DirectXMath.h>

class Transform
{
public:
    Transform();
    ~Transform();

    // Getters
    DirectX::XMFLOAT3 GetPosition();
    DirectX::XMFLOAT3 GetPitchYawRoll();
    DirectX::XMFLOAT3 GetScale();
    DirectX::XMFLOAT4X4 GetWorldMatrix();
    DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

    // Setters
    void SetPosition(float x, float y, float z);
    void SetPitchYawRoll(float pitch, float yaw, float roll);
    void SetScale(float x, float y, float z);

    // Transformers
    void MoveAbsolute(float x, float y, float z);
    void Rotate(float pitch, float yaw, float roll);
    void Scale(float x, float y, float z);

private:
    // Raw transformation data
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 pitchYawRoll;
    DirectX::XMFLOAT3 scale;

    // Matrices
    bool matricesDirty;
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

    // Helper for updating matrices
    void UpdateMatrices();
};

