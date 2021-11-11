#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Material.h"
#include <memory>
#include <vector>
#include "Lights.h"
#include "Sky.h"
#include <unordered_map>

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateBasicGeometry();
	void CreateSampleLights();
	void CreateMaterials();
	void GenerateCircle(float radius, int subdivisions, DirectX::XMFLOAT4 color, float xOffset);
	
	void UpdateEntity(Entity& e, float deltaTime, float totalTime);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr
	
	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShaderSpec;
	std::shared_ptr<SimplePixelShader> pixelShaderSky;
	std::shared_ptr<SimplePixelShader> pixelShaderSpecAndNormal;
	std::shared_ptr<SimplePixelShader> pixelShaderSpecNormalRefl;
	std::shared_ptr<SimplePixelShader> customPixelShader;

	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimpleVertexShader> vertexShaderSky;
	std::shared_ptr<SimpleVertexShader> vertexShaderNormalMap;

	// Some sample meshes
	std::shared_ptr<Mesh> tri;
	std::shared_ptr<Mesh> pent;
	std::shared_ptr<Mesh> circle;
	std::shared_ptr<Mesh> cube;

	// All the entities that will be drawn
	std::vector<Entity> entities;
	std::vector<Entity> entitiesAllSpheres;

	std::shared_ptr<Camera> camera;

	// Lights
	std::vector<Light> lights;

	// Materials
	std::vector<std::wstring> textureFiles;
	std::unordered_map<std::wstring, std::shared_ptr<Material>> materials;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	// Skybox things
	std::shared_ptr<Sky> skybox;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyboxSrv;

	bool moveEntities = true;
	bool offsetUvs = false;
	bool scaleUvs = false;
	bool spheresOnly = true;
};