#pragma once
#include <vector>
#include <string>
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>
#include <fstream>
#include "WICTextureLoader\WICTextureLoader.h"

using namespace DirectX;

namespace assetUtility {
	struct MainHeader 
	{
		int meshCount, matCount, camCount, ambientLightSize, areaLightSize, dirLightSize, pointLightSize, spotLightSize;
	};

	struct MeshHeader 
	{
		int nameLength, numberPoints, numberNormals, numberCoords, numberFaces;
	};

	struct MatHeader 
	{
		int diffuseNameLength, ambientNameLength, specularNameLength, transparencyNameLength, glowNameLength;
	};

	struct Vertex 
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
		XMFLOAT3 normal;
		int boneIndices[4];
		float boneWeigths[4];
	};

	struct point{
		XMFLOAT3 position;
		int boneIndices[4];
		float boneWeigths[4];
	};

	struct AmbientLightStruct 
	{
		double intensity;
		XMFLOAT3 color;
		XMFLOAT3 pos;
	};

	struct AreaLightStruct 
	{
		double intensity;
		XMFLOAT3 color;
		XMFLOAT3 pos;
	};

	struct DirectionalLightStruct 
	{
		double intensity;
		XMFLOAT3 color;
		XMFLOAT3 dir;
		XMFLOAT3 pos;
	};

	struct PointLightStruct 
	{
		double intensity;
		XMFLOAT3 color;
		XMFLOAT3 pos;
	};

	struct SpotLightStruct 
	{
		double intensity;
		XMFLOAT3 color;
		XMFLOAT3 dir;
		XMFLOAT3 pos;

		double coneAngle;
		double penumbraAngle;
		double dropoff;
	};

	struct Model 
	{
		~Model ( ) {
			vertexBuffer->Release ( );
			pointLights.clear ( );
		}
		ID3D11Buffer* vertexBuffer;
		int vertexBufferSize;
		std::vector<PointLightStruct> pointLights;
		SpotLightStruct spotLight;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
	};

	struct RenderObject 
	{
		Model* model;
		ID3D11ShaderResourceView* diffuseTexture = nullptr;
		ID3D11ShaderResourceView* specularTexture = nullptr;
	};

	struct LightData 
	{
		std::vector<PointLightStruct> pointLights;
		std::vector<AmbientLightStruct> ambientLights;
		std::vector<AreaLightStruct> areaLights;
		std::vector<DirectionalLightStruct> dirLights;
		std::vector<SpotLightStruct> spotLights;
	};

	static void splitStringToVector ( std::string input, std::vector<std::string> &output, std::string delimiter ) {
		size_t pos = 0;
		std::string token;
		while ( ( pos = input.find ( delimiter ) ) != std::string::npos ) {
			token = input.substr ( 0, pos );
			if ( !token.empty ( ) ) {
				output.push_back ( token );
			}
			input.erase ( 0, pos + delimiter.length ( ) );
		}
		output.push_back ( input );
	};

	//turns a text file into a vector of strings line-by-line
	static void fileToStrings ( std::string file_path, std::vector<std::string> &output ) 
	{
		std::ifstream file ( file_path );
		if ( !file.is_open ( ) ) 
		{
			printf ( "Could not open " );
			printf ( file_path.c_str() );
			printf ( "\n" );
			return;
		}
		std::string s ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char> ( ) );

		splitStringToVector ( s, output, "\n" );
	};

	static std::vector<int> stringToIntArray ( std::string input ) {
		std::vector<int> output;
		int from = 0;

		for ( int to = 0; to < ( signed )input.size ( ); to++ ) {
			if ( input[ to ] == ',' ) {
				output.push_back ( std::stoi ( input.substr ( from, to - from ) ) );
				from = to + 1;
			}
		}
		output.push_back ( std::stoi ( input.substr ( from, input.size ( ) - from ) ) );


		return output;
	}
}
