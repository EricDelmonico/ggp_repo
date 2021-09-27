#include "Camera.h"
#include "Input.h"
#include <iostream>

using namespace DirectX;

Camera::Camera(
    float x,
    float y,
    float z,
    float moveSpeed,
    float lookSpeed,
    float fov,
    float aspectRatio) :
    movementSpeed(moveSpeed),
    mouseLookSpeed(lookSpeed),
    fieldOfView(fov),
    aspectRatio(aspectRatio)
{
    // Set up the transform
    transform.SetPosition(x, y, z);

    // Set up our matrices
    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
}

void Camera::Update(float dt)
{
    // Get a reference to the input manager
    Input& input = Input::GetInstance();

    // Calculate the current speed
    float speed = movementSpeed * dt;

    // Movement
    if (input.KeyDown('W')) { transform.MoveRelative(0, 0, speed); }
    if (input.KeyDown('S')) { transform.MoveRelative(0, 0, -speed); }
    if (input.KeyDown('A')) { transform.MoveRelative(-speed, 0, 0); }
    if (input.KeyDown('D')) { transform.MoveRelative(speed, 0, 0); }
    if (input.KeyDown('E')) { transform.MoveAbsolute(0, speed, 0); }
    if (input.KeyDown('Q')) { transform.MoveAbsolute(0, -speed, 0); }

    // Rotate if the mouse is down
    if (input.MouseLeftDown())
    {
        // Calculate how much the cursor changed
        float xDiff = dt * mouseLookSpeed * input.GetMouseXDelta();
        float yDiff = dt * mouseLookSpeed * input.GetMouseYDelta();

        // Don't allow pitch to go more than 90 degrees or less than -90 degrees
        XMFLOAT3 pitchYawRoll = transform.GetPitchYawRoll();
        if (pitchYawRoll.x + yDiff > DirectX::XM_PIDIV2 - 0.05f ||
            pitchYawRoll.x + yDiff < -DirectX::XM_PIDIV2 + 0.05f)
        {
            yDiff = 0;
        }
        
        // Rotate the transform! SWAP X AND Y!
        transform.Rotate(yDiff, xDiff, 0);
    }

    // At the end, update the view
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    XMFLOAT3 pos = transform.GetPosition();
    XMFLOAT3 forward = transform.GetForward();

    XMMATRIX v = XMMatrixLookToLH(
        XMLoadFloat3(&pos),         // Camera's position
        XMLoadFloat3(&forward),     // Camera's forward vector
        XMVectorSet(0, 1, 0, 0));   // World up (Y)

    XMStoreFloat4x4(&viewMatrix, v);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    XMMATRIX p = XMMatrixPerspectiveFovLH(
        fieldOfView, 
        aspectRatio, 
        0.01f,       // Near clip plane distance
        100.0f);     // Far clip plane distance

    XMStoreFloat4x4(&projectionMatrix, p);
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
    return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
    return projectionMatrix;
}

Transform* Camera::GetTransform()
{
    return &transform;
}

float Camera::GetFoV()
{
    return fieldOfView;
}

void Camera::SetFoV(float fov)
{
    fieldOfView = fov;
    UpdateProjectionMatrix(aspectRatio);
}
