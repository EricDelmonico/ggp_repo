#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include <vector>
#include <cmath>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <iostream>

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
    : camera(0),
    DXCore(
        hInstance,		   // The application's handle
        "DirectX Game",	   // Text for the window's title bar
        1280,			   // Width of the window's client area
        720,			   // Height of the window's client area
        true)			   // Show extra stats (fps) in title bar?

{

#if defined(DEBUG) || defined(_DEBUG)
    // Do we want a console window?  Probably only in debug mode
    CreateConsoleWindow(500, 120, 32, 120);
    printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
    // Note: Since we're using smart pointers (ComPtr),
    // we don't need to explicitly clean up those DirectX objects
    // - If we weren't using smart pointers, we'd need
    //   to call Release() on each DirectX object created in Game

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
    // Helper methods for loading shaders, creating some basic
    // geometry to draw and some simple camera matrices.
    //  - You'll be expanding and/or replacing these later
    LoadShaders();

    // Shadow map init
    shadowMapResolution = 1024;
    D3D11_TEXTURE2D_DESC shadowTexDesc = {};
    shadowTexDesc.Width = shadowMapResolution;
    shadowTexDesc.Height = shadowMapResolution;
    shadowTexDesc.MipLevels = 1;
    shadowTexDesc.ArraySize = 1;
    shadowTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    shadowTexDesc.Usage = D3D11_USAGE_DEFAULT;
    shadowTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    shadowTexDesc.CPUAccessFlags = 0;
    shadowTexDesc.MiscFlags = 0;
    shadowTexDesc.SampleDesc.Count = 1;
    shadowTexDesc.SampleDesc.Quality = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowMapTex;
    device->CreateTexture2D(&shadowTexDesc, 0, shadowMapTex.GetAddressOf());

    // Create depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencDesc = {};
    depthStencDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencDesc.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(
        shadowMapTex.Get(),
        &depthStencDesc,
        shadowMapDSV.GetAddressOf());

    // Create shadow map srv
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    device->CreateShaderResourceView(shadowMapTex.Get(), &srvDesc, shadowMapSRV.GetAddressOf());

    // Create the shadow sampler
    D3D11_SAMPLER_DESC shadowSampDesc = {};
    shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // COMPARISON filter!
    shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    /*shadowSampDesc.BorderColor[0] = 1.0f;
    shadowSampDesc.BorderColor[1] = 1.0f;
    shadowSampDesc.BorderColor[2] = 1.0f;
    shadowSampDesc.BorderColor[3] = 1.0f;*/
    device->CreateSamplerState(&shadowSampDesc, shadowSampler.GetAddressOf());

    // Create a rasterizer state
    D3D11_RASTERIZER_DESC shadowRastDesc = {};
    shadowRastDesc.FillMode = D3D11_FILL_SOLID;
    shadowRastDesc.CullMode = D3D11_CULL_BACK;
    shadowRastDesc.DepthClipEnable = true;
    shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible positive value storable in the depth buffer)
    shadowRastDesc.DepthBiasClamp = 0.0f;
    shadowRastDesc.SlopeScaledDepthBias = 1.0f;
    device->CreateRasterizerState(&shadowRastDesc, shadowMapRasterizerState.GetAddressOf());

    // Light camera init
    shadowMapCamera = std::make_shared<Camera>(
        0,                      // x
        10,                     // y
        5,                      // z
        0,                      // move speed
        0,                      // look speed
        0,                      // fov
        (float)width / height,  // aspect ratio
        false,                  // perspective bool
        20,                     // orthographic viewport height
        90,                     // pitch
        0,                      // yaw
        0);                     // roll

    CreateMaterials();
    CreateBasicGeometry();

    // Tell the input assembler stage of the pipeline what kind of
    // geometric primitives (points, lines or triangles) we want to draw.  
    // Essentially: "What kind of shape should the GPU draw with our data?"
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Camera once we have aspect ratio available
    camera = std::shared_ptr<Camera>(
        new Camera(
            0,                          // x
            0,                          // y
            -20,                        // z
            5.0f,                       // Move speed
            3.0f,                       // Look speed  
            XM_PIDIV4,                  // FOV
            (float)width / height));    // Aspect

    // Create sky
    skybox = std::make_shared<Sky>(cube, skyboxSrv, pixelShaderSky, vertexShaderSky, samplerState, device);

    CreateSampleLights();
}

void Game::CreateSampleLights()
{
    // Initialize Lights

    // Directional lights
    Light directionalLight1 =
        Light(
            { 1, 1, 0 },    // Direction
            { 1, 1, 1 },    // Color
            1.0f);          // Intensity

    // Shadow casting light
    Light directionalLight2 =
        Light(
            { 0, -1, 0 },    // Direction
            { 1, 1, 1 },    // Color
            1.0f);          // Intensity

    Light directionalLight3 =
        Light(
            { -1, 1, -0.5f },   // Direction
            { 1, 1, 1 },        // Color
            1.0f);              // Intensity

    // Point lights
    Light pointLight1 =
        Light(
            { -1.5f, 0, 0 },    // Position
            { 1, 1, 1 },        // Color
            10,                 // Range
            1.0f);              // Intensity

    Light pointLight2 =
        Light(
            { 1.5f, 0, 0 }, // Position
            { 1, 1, 1 },    // Color
            10,             // Range
            0.5f);          // Intensity

    // Push all the lights
    lights.push_back(directionalLight1);
    lights.push_back(directionalLight2);
    lights.push_back(directionalLight3);
    lights.push_back(pointLight1);
    lights.push_back(pointLight2);
}

void Game::CreateMaterials()
{
    // Sampler description/sampler state
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

    // Skybox srv
    CreateDDSTextureFromFile(device.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Sky/skybox.dds").c_str(), nullptr, skyboxSrv.GetAddressOf());

    // Populate texture file names manually
    textureFiles.push_back(L"bronze");
    textureFiles.push_back(L"cobblestone");
    textureFiles.push_back(L"floor");
    textureFiles.push_back(L"paint");
    textureFiles.push_back(L"rough");
    textureFiles.push_back(L"scratched");
    textureFiles.push_back(L"wood");

    // Actually create the materials
    XMFLOAT4 white = { 1, 1, 1, 1 };
    for (auto& t : textureFiles)
    {
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tempAlbedo;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tempNormal;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tempMetalness;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tempRoughness;
        CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/PBR_Textures/" + t + L"_albedo.png").c_str(), nullptr, tempAlbedo.GetAddressOf());
        CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/PBR_Textures/" + t + L"_normals.png").c_str(), nullptr, tempNormal.GetAddressOf());
        CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/PBR_Textures/" + t + L"_metal.png").c_str(), nullptr, tempMetalness.GetAddressOf());
        CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/PBR_Textures/" + t + L"_roughness.png").c_str(), nullptr, tempRoughness.GetAddressOf());

        std::shared_ptr<Material> material = std::make_shared<Material>(white, pixelShaderSpecNormalReflShadow, vertexShaderNormalMapShadowMap);
        material->AddTextureSRV("Albedo", tempAlbedo);
        material->AddTextureSRV("NormalMap", tempNormal);
        material->AddTextureSRV("MetalnessMap", tempMetalness);
        material->AddTextureSRV("RoughnessMap", tempRoughness);
        material->AddTextureSRV("SkyTexture", skyboxSrv);
        material->AddTextureSRV("ShadowMap", shadowMapSRV);
        material->AddSampler("BasicSampler", samplerState);
        material->AddSampler("ShadowSampler", shadowSampler);

        materials.insert({ t, material });
    }
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
    // Vertex shaders
    vertexShader = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShader.cso").c_str());
    vertexShaderNormalMap = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderNormalMap.cso").c_str());
    vertexShaderNormalMapShadowMap = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderNormalMapShadow.cso").c_str());
    vertexShaderSky = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderSky.cso").c_str());
    shadowVS = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"ShadowMapVS.cso").c_str());

    // Pixel shaders
    pixelShaderSky = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSky.cso").c_str());
    pixelShaderSpec = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSpecOnly.cso").c_str());
    pixelShaderSpecAndNormal = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSpecAndNormal.cso").c_str());
    pixelShaderSpecNormalRefl = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSpecNormalRefl.cso").c_str());
    pixelShaderSpecNormalReflShadow = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderSpecNormalReflShadow.cso").c_str());
    customPixelShader = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"CustomPS.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
    // Create some temporary variables to represent colors
    // - Not necessary, just makes things more readable
    XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
    XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
    XMFLOAT4 white = XMFLOAT4(1, 1, 1, 1);

    // Set up the vertices of the triangle we would like to draw
    // - We're going to copy this array, exactly as it exists in memory
    //    over to a DirectX-controlled data structure (the vertex buffer)
    // - Note: Since we don't have a camera or really any concept of
    //    a "3d world" yet, we're simply describing positions within the
    //    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
    // - This means (0,0) is at the very center of the screen.
    // - These are known as "Normalized Device Coordinates" or "Homogeneous 
    //    Screen Coords", which are ways to describe a position without
    //    knowing the exact size (in pixels) of the image/window/etc.  
    // - Long story short: Resizing the window also resizes the triangle,
    //    since we're describing the triangle in terms of the window itself
    Vertex verticesTri[] =
    {
        { XMFLOAT3(+0.0f, +0.5f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
        { XMFLOAT3(+0.5f, -0.5f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
        { XMFLOAT3(-0.5f, -0.5f, +0.1f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) }
    };

    // Set up the indices, which tell us which vertices to use and in which order
    // - This is somewhat redundant for just 3 vertices (it's a simple example)
    // - Indices are technically not required if the vertices are in the buffer 
    //    in the correct order and each one will be used exactly once
    // - But just to see how it's done...
    unsigned int indicesTri[] = { 0, 1, 2 };

    // Set up vertices for a pentagon
    Vertex verticesPent[] =
    {
        { XMFLOAT3(+0.00f, +0.5f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
        { XMFLOAT3(-0.50f, +0.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
        { XMFLOAT3(-0.25f, -0.5f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
        { XMFLOAT3(+0.25f, -0.5f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
        { XMFLOAT3(+0.50f, +0.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
        { XMFLOAT3(+0.00f, +0.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) }
    };

    // Set up the tris for a pentagon
    unsigned int indicesPent[] = { 0, 5, 1, 1, 5, 2, 5, 3, 2, 5, 4, 3, 0, 4, 5 };

    tri = std::make_shared<Mesh>(verticesTri, 3, indicesTri, 3, device, context);
    pent = std::make_shared<Mesh>(verticesPent, 6, indicesPent, 15, device, context);

    // Create a circle and assign it to the circle field
    GenerateCircle(
        0.25f,          // radius
        20,             // subdivisions
        white,          // color
        0);             // x offset

    cube = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device, context);
    std::shared_ptr<Mesh> cylinder = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cylinder.obj").c_str(), device, context);
    std::shared_ptr<Mesh> helix = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device, context);
    std::shared_ptr<Mesh> quad = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad.obj").c_str(), device, context);
    std::shared_ptr<Mesh> floor = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad.obj").c_str(), device, context);
    std::shared_ptr<Mesh> quad_double_sided = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad_double_sided.obj").c_str(), device, context);
    std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device, context);
    std::shared_ptr<Mesh> torus = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/torus.obj").c_str(), device, context);

    // Assign geometry and materials to some entities
    //
    // Materials:
    // bronze
    // cobblestone
    // floor
    // paint
    // rough
    // scratched
    // wood
    entities.push_back(Entity(cube, materials[L"bronze"]));
    entities.push_back(Entity(cylinder, materials[L"cobblestone"]));
    entities.push_back(Entity(helix, materials[L"floor"]));
    entities.push_back(Entity(sphere, materials[L"scratched"]));
    entities.push_back(Entity(torus, materials[L"rough"]));
    entities.push_back(Entity(quad, materials[L"paint"]));
    entities.push_back(Entity(quad_double_sided, materials[L"wood"]));
    entities.push_back(Entity(floor, materials[L"wood"]));

    entitiesAllSpheres.push_back(Entity(sphere, materials[L"bronze"]));
    entitiesAllSpheres.push_back(Entity(sphere, materials[L"cobblestone"]));
    entitiesAllSpheres.push_back(Entity(sphere, materials[L"floor"]));
    entitiesAllSpheres.push_back(Entity(sphere, materials[L"scratched"]));
    entitiesAllSpheres.push_back(Entity(sphere, materials[L"rough"]));
    entitiesAllSpheres.push_back(Entity(sphere, materials[L"paint"]));
    entitiesAllSpheres.push_back(Entity(sphere, materials[L"wood"]));
    entitiesAllSpheres.push_back(Entity(floor, materials[L"wood"]));

    // Move entities so they're lined up nicely
    for (int i = 0; i < entities.size() - 1; i++)
    {
        entities[i].GetTransform()->SetPosition(((float)(i - 3) * 3), 0, 0);
        entitiesAllSpheres[i].GetTransform()->SetPosition(((float)(i - 3) * 3), 0, 0);
    }

    // Scale the floor so it's nice and big to catch shadows
    entities[entities.size() - 1].GetTransform()->SetScale(10, 1, 10);
    entities[entities.size() - 1].GetTransform()->SetPosition(0, -1.5f, 0);
    entitiesAllSpheres[entitiesAllSpheres.size() - 1].GetTransform()->SetScale(10, 1, 10);
    entitiesAllSpheres[entitiesAllSpheres.size() - 1].GetTransform()->SetPosition(0, -1.5f, 0);
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
    // Handle base-level DX resize stuff
    DXCore::OnResize();

    // Ensure we update the camera whenever the window resizes
    // Note: this could trigger before Init(), so ensure our ptr
    // is valid before calling UpdateProjectionMatrix()
    if (camera)
    {
        camera->UpdateProjectionMatrix((float)width / height);
    }
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
    // Example input checking: Quit if the escape key is pressed
    if (Input::GetInstance().KeyDown(VK_ESCAPE))
        Quit();

    // Move/scale/rotate entities every frame=
    if (spheresOnly)
    {
        for (int i = 0; i < entitiesAllSpheres.size() - 1; i++)
        {
            UpdateEntity(entitiesAllSpheres[i], deltaTime, totalTime);
        }
    }
    else
    {
        for (int i = 0; i < entities.size() - 1; i++)
        {
            UpdateEntity(entities[i], deltaTime, totalTime);
        }
    }

    // Update the camera every frame
    camera->Update(deltaTime);

    // Play with field of view
    float fov = camera->GetFoV();
    if (Input::GetInstance().KeyDown('O')) fov += 1.0f * deltaTime;
    if (Input::GetInstance().KeyDown('P')) fov -= 1.0f * deltaTime;
    if (Input::GetInstance().KeyPress('M')) moveEntities = !moveEntities;
    if (Input::GetInstance().KeyPress('I')) scaleUvs = !scaleUvs;
    if (Input::GetInstance().KeyPress('U')) offsetUvs = !offsetUvs;
    if (Input::GetInstance().KeyPress('L')) spheresOnly = !spheresOnly;
    camera->SetFoV(fov);
}

void Game::UpdateEntity(Entity& e, float deltaTime, float totalTime)
{
    if (moveEntities)
    {
        // Move the entities up and down, and rotate them over time
        auto transform = e.GetTransform();
        // Skip up and down for now...
        transform->MoveAbsolute(0, std::sin(totalTime / 3) / 10 * deltaTime, 0);
        transform->Rotate(0.25f * deltaTime, 0.25f * deltaTime, 0);
    }

    if (offsetUvs)
    {
        auto offset = e.GetMaterial()->GetUvOffset();
        e.GetMaterial()->SetUvOffset(offset.x + deltaTime / 10, 0);
    }

    if (scaleUvs)
    {
        auto offset = e.GetMaterial()->GetUvScale();
        e.GetMaterial()->SetUvScale(std::sin(totalTime / 3) + 1, 1);
    }
    else
    {
        e.GetMaterial()->SetUvScale(1, 1);
    }
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
    // Background color (Cornflower Blue in this case) for clearing
    const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

    // Clear the render target and depth buffer (erases what's on the screen)
    //  - Do this ONCE PER FRAME
    //  - At the beginning of Draw (before drawing *anything*)
    context->ClearRenderTargetView(backBufferRTV.Get(), color);
    context->ClearDepthStencilView(
        depthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0);

    // Sun in skybox is yellow-red
    XMFLOAT3 ambientColor = XMFLOAT3(.15f, .125f, .075f);

    // Draw entities
    auto entityList = spheresOnly ? entitiesAllSpheres : entities;

    // Render the shadow map before the other objects
    RenderShadowMap(entityList);
    for (auto& e : entityList)
    {
        auto pixelShader = e.GetMaterial()->GetPixelShader();
        pixelShader->SetFloat3("ambient", ambientColor);
        // Set lights
        pixelShader->SetData(
            "lights",                               // Name of the variable in the shader 
            &lights[0],                             // Address of the data to set the shader variable to
            sizeof(Light) * (int)lights.size());    // The size of the data to set

        e.Draw(*camera, *shadowMapCamera, totalTime);
    }

    // Draw sky last!
    skybox->Draw(context, camera.get());

    // Present the back buffer to the user
    //  - Puts the final frame we're drawing into the window so the user can see it
    //  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
    swapChain->Present(0, 0);

    // Due to the usage of a more sophisticated swap chain,
    // the render target must be re-bound after every call to Present()
    context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}


// This method generates a circle and assigns it to the circle field.
// This will be deleted ASAP because it doesn't need to be here at all
// after assignment 1. xOffset has a default val of 0.75f
void Game::GenerateCircle(float radius, int subdivisions, XMFLOAT4 color, float xOffset)
{
    Vertex center = { XMFLOAT3(0.0f + xOffset, 0.0f, 0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) };

    // get the angle per subdivision and convert to radians
    float fRadsPerSubdiv = (360.0f / subdivisions) * 3.1415926535897932384f / 180;

    // each subdivision constitutes one new outer vertex, except the
    // last tri in which the last and first vertices are re-used. this 
    // std::vector will hold all outer vertices so they can be reused.
    // The center vertex will be inserted at index 0 after all verts have been created.
    std::vector<Vertex> outerVertices;
    std::vector<unsigned int> indices;

    // make first vertex manually, starting at an angle of 0
    outerVertices.push_back({ XMFLOAT3(std::cosf(0) * radius + xOffset, std::sinf(0) * radius, 0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) });

    for (int i = 0; i < subdivisions; i++)
    {
        // include previous outer vertex in this tri
        Vertex prevVert = outerVertices[i];

        // number of the next vertex, with the first being 0, and the subsequent being 1...(a_nSubdivisions - 1)
        int nextVert = (i + 1) % subdivisions;
        // If we're back to the first vertex...
        if (nextVert == 0)
        {
            // add the tri and exit for loop
            indices.push_back(0);
            indices.push_back(i);
            // -1 for now will represent the center
            indices.push_back(-1);
            break;
        }
        else
        {
            // create new outer vertex based on the vertex's number
            Vertex newVert = {
                XMFLOAT3(std::cosf(nextVert * fRadsPerSubdiv) * radius + xOffset, std::sinf(nextVert * fRadsPerSubdiv) * radius, 0.0f),
                XMFLOAT3(0, 0, -1),
                XMFLOAT2(0, 0) };

            // add the tri and push the newly made vertex onto the 
            // list of outer vertices to be used for the next tri
            indices.push_back(-1);
            indices.push_back(i + 1);
            indices.push_back(i);

            outerVertices.push_back(newVert);
        }
    }

    // insert the center, offset all indices by 1 to account for this
    outerVertices.insert(outerVertices.begin(), center);
    for (int i = 0; i < indices.size(); i++)
    {
        indices[i] += 1;
    }

    // assign the circle mesh with these verts/indices
    circle = std::make_shared<Mesh>(&outerVertices[0], (int)outerVertices.size(), &indices[0], (int)indices.size(), device, context);
}

void Game::RenderShadowMap(std::vector<Entity> entityList)
{
    
    // Set null render target 
    context->OMSetRenderTargets(0, 0, shadowMapDSV.Get());
    // Clear shadow depth stencil view before drawing
    context->ClearDepthStencilView(
        shadowMapDSV.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0);
    // Apply custom rasterizer state
    context->RSSetState(shadowMapRasterizerState.Get());

    // Create a viewport matching the shadow map resolution
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (float)shadowMapResolution;
    viewport.Height = (float)shadowMapResolution;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
    
    // Set our shaders and draw with them
    shadowVS->SetShader();
    context->PSSetShader(0, 0, 0);

    for (auto& e : entityList) 
    {
        shadowVS->SetMatrix4x4("world", e.GetTransform()->GetWorldMatrix());
        shadowVS->SetMatrix4x4("view", shadowMapCamera->GetView());
        shadowVS->SetMatrix4x4("projection", shadowMapCamera->GetProjection());
        shadowVS->CopyAllBufferData();
        e.GetMesh()->Draw();
    }

    // Put render target and rasterizer state back to normal
    context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
    viewport.Width = (float)this->width;
    viewport.Height = (float)this->height;
    context->RSSetViewports(1, &viewport);
    context->RSSetState(0);
}
