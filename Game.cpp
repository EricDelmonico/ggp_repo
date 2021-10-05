#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include <vector>
#include <cmath>

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
    CreateBasicGeometry();

    // Tell the input assembler stage of the pipeline what kind of
    // geometric primitives (points, lines or triangles) we want to draw.  
    // Essentially: "What kind of shape should the GPU draw with our data?"
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Camera once we have aspect ratio available
    camera = std::shared_ptr<Camera>(new Camera(0, 0, -5, 5.0f, 0.5f, XM_PIDIV2, (float)width / height));
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
    vertexShader = std::shared_ptr<SimpleVertexShader>(
        new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShader.cso").c_str())); 
    pixelShader = std::shared_ptr<SimplePixelShader>(
        new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShader.cso").c_str()));
    customPixelShader = std::shared_ptr<SimplePixelShader>(
        new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"CustomPS.cso").c_str()));
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

    tri = std::shared_ptr<Mesh>(new Mesh(verticesTri, 3, indicesTri, 3, device, context));
    pent = std::shared_ptr<Mesh>(new Mesh(verticesPent, 6, indicesPent, 15, device, context));

    // Create a circle and assign it to the circle field
    GenerateCircle(
        0.25f,          // radius
        20,             // subdivisions
        white,          // color
        0);             // x offset

    std::shared_ptr<Mesh> cube = std::shared_ptr<Mesh>(new Mesh(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device, context));
    std::shared_ptr<Mesh> cylinder = std::shared_ptr<Mesh>(new Mesh(GetFullPathTo("../../Assets/Models/cylinder.obj").c_str(), device, context));
    std::shared_ptr<Mesh> helix = std::shared_ptr<Mesh>(new Mesh(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device, context));
    std::shared_ptr<Mesh> quad = std::shared_ptr<Mesh>(new Mesh(GetFullPathTo("../../Assets/Models/quad.obj").c_str(), device, context));
    std::shared_ptr<Mesh> quad_double_sided = std::shared_ptr<Mesh>(new Mesh(GetFullPathTo("../../Assets/Models/quad_double_sided.obj").c_str(), device, context));
    std::shared_ptr<Mesh> sphere = std::shared_ptr<Mesh>(new Mesh(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device, context));
    std::shared_ptr<Mesh> torus = std::shared_ptr<Mesh>(new Mesh(GetFullPathTo("../../Assets/Models/torus.obj").c_str(), device, context));

    std::shared_ptr<Material> whiteMaterial = std::shared_ptr<Material>(new Material(white, customPixelShader, vertexShader));

    // Assign geometry and materials to some entities
    entities.push_back(Entity(cube, whiteMaterial));
    entities.push_back(Entity(cylinder, whiteMaterial));
    entities.push_back(Entity(helix, whiteMaterial));
    entities.push_back(Entity(quad, whiteMaterial));
    entities.push_back(Entity(quad_double_sided, whiteMaterial));
    entities.push_back(Entity(sphere, whiteMaterial));
    entities.push_back(Entity(torus, whiteMaterial));

    // Move entities so they're lined up nicely
    for (int i = 0; i < entities.size(); i++)
    {
        entities[i].GetTransform()->SetPosition((i - 3) * 3, 0, 0);
    }
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

    // Move/scale/rotate entities every frame
    for (auto& e : entities)
    {
        auto transform = e.GetTransform();
        transform->MoveAbsolute(0, std::sin(totalTime) / 5 * deltaTime, 0);
        transform->Rotate(1.0f * deltaTime, 1.0f * deltaTime, 0);
    }

    // Update the camera every frame
    camera->Update(deltaTime);

    // Play with field of view
    float fov = camera->GetFoV();
    if (Input::GetInstance().KeyDown('O')) fov += 1.0f * deltaTime;
    if (Input::GetInstance().KeyDown('P')) fov -= 1.0f * deltaTime;
    camera->SetFoV(fov);
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

    // Draw entities
    for (auto& e : entities)
        e.Draw(camera.get(), totalTime);

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
    circle = std::shared_ptr<Mesh>(new Mesh(&outerVertices[0], (int)outerVertices.size(), &indices[0], (int)indices.size(), device, context));
}