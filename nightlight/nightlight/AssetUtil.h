#pragma once
#include <vector>
#include <string>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct MainHeader{
	int meshCount, matCount, camCount, lightCount;
};

struct MeshHeader
{
	int nameLength, numberPoints, numberNormals, numberCoords, numberFaces;
};

struct MatHeader{
	int diffuseNameLength, ambientNameLength, specularNameLength, transparencyNameLength, glowNameLength;
};

struct LightHeader{
	int ambientLightSize, areaLightSize, dirLightSize, pointLightSize, spotLightSize;
};

struct Texture
{
	std::string textureName = "";
	ID3D11ShaderResourceView* texturePointer = nullptr;
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
};

struct RenderObject
{
	ID3D11Buffer* vertexBuffer = nullptr;
	int vertexBufferSize;
	XMFLOAT3 diffuse;
	XMFLOAT3 specular;
	float shine = 0;
	Texture* diffuseTexture = nullptr;
	Texture* specularTexture = nullptr;
};



struct ambientLightStruct
{
	std::vector<double> intensity;
	std::vector<XMFLOAT3> color;
	std::vector<XMFLOAT3> pos;
};

struct areaLightStruct
{
	double intensity;
	XMFLOAT3 color;
	XMFLOAT3 pos;
};

struct directionalLightStruct
{
	double intensity;
	XMFLOAT3 color;
	XMFLOAT3 dir;
	XMFLOAT3 pos;
};

struct pointLightStruct
{
	double intensity;
	XMFLOAT3 color;
	XMFLOAT3 pos;
};

struct spotLightStruct
{
	double intensity;
	XMFLOAT3 color;
	double coneAngle;
	double penumbraAngle;
	double dropoff;
	XMFLOAT3 dir;
	XMFLOAT3 pos;
};


struct lightData
{
	std::vector<pointLightStruct> pointLights;
	std::vector<ambientLightStruct> ambientLights;
	std::vector<areaLightStruct> areaLights;
	std::vector<directionalLightStruct> dirLights;
	std::vector<spotLightStruct> spotLights;
};