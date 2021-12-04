#pragma once
#include <DirectXMath.h>
#include "Transform.h"

class Camera
{
public:
    Camera(
        float x,
        float y,
        float z,
        float moveSpeed,
        float lookSpeed,
        float fov,
        float aspectRatio,
        bool perspective = true,
        float orthoSize = 2.5f,
        float pitch = 0,
        float yaw = 0,
        float roll = 0);
    ~Camera();

    // Update methods
    void Update(float dt);
    void UpdateViewMatrix();
    void UpdateProjectionMatrix(float aspectRatio);

    // Getters and Setters
    DirectX::XMFLOAT4X4 GetView();
    DirectX::XMFLOAT4X4 GetProjection();

    Transform* GetTransform();

    float GetFoV();
    void SetFoV(float fov);

    float GetOrthoSize();
    void SetOrthoSize(float size);

private:
    
    // Camera matrices
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;

    Transform transform;

    float movementSpeed;
    float mouseLookSpeed;
    float fieldOfView;
    float aspectRatio;

    bool perspective;

    float orthoSize;
};

