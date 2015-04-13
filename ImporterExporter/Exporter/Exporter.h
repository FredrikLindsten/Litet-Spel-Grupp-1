// Jonas Petersson
// jnp@bth.se
// 14-03-31

#pragma once

#define NT_APP //extern Maya-application (inget plug-in | MLibrary based application).

//___________________________________________________________________________________________
//|																							|
//|										INKLUDERA											|
//|_________________________________________________________________________________________|

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <maya\MLibrary.h>
#include <maya\MFileIO.h>
#include <maya\MGlobal.h>

#include <maya\MDagPath.h>
#include <maya\MFloatPointArray.h>

#include <maya\MFnMesh.h>
#include <maya\MFnDependencyNode.h>

#include <maya\MItDag.h>
#include <maya\MItMeshPolygon.h>

#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "OpenMaya.lib")

//___________________________________________________________________________________________
//|																							|
//|										GLOBAL STRUKT-DATA									|
//|_________________________________________________________________________________________|

struct uvSet
{
	MFloatArray Us;
	MFloatArray Vs;
	MFloatVectorArray tangents;
	MFloatVectorArray binormals;
};

struct MeshData
{
	MDagPath mesh_path;
	MString name;
	UINT id;

	MFloatPointArray points;
	MFloatVectorArray normals;
	std::vector<uvSet> uvSets;
};

struct SceneData
{
	std::vector<MeshData> meshes;
};

//___________________________________________________________________________________________
//|																							|
//|										EXPORTERKLASS										|
//|_________________________________________________________________________________________|

class Exporter
{
	public:
		Exporter();
		~Exporter();

		void StartExporter(std::string directory_path);

	private:
		std::fstream export_stream_;
		SceneData scene_;

	private:
		bool InitializeMaya();
		void CleanUpMaya();

		bool CreateExportFiles(std::string file_path);
		void CloseExportFiles();

		bool GetMayaFilenamesInDirectory(char *folder_path, std::vector<std::string> &list_to_fill);

		void ProcessScene(const char *file_path);

		bool IdentifyAndExtractScene();
		bool IdentifyAndExtractMeshes();

		bool ExtractMeshData(MFnMesh &mesh, UINT index);

		void ExportScene();
		void ExportMeshes();
};