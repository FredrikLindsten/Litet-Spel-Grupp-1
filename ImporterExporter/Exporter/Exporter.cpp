// Jonas Petersson
// jnp@bth.se
// 14-03-31

#include "Exporter.h"

// konstuktor
Exporter::Exporter()
{

}

// destruktor
Exporter::~Exporter()
{
	CloseExportFiles();
	CleanUpMaya();
}

// initiera exportern
bool Exporter::InitializeMaya()
{
	// [Ur Maya-dokumentationen]
	// Initialize the Maya library.
	// This method must be called before any Maya functions sets are used.
	// It is acceptable to instantiate Maya fundamental types before calling this, but attempts to do anything else cause unpredictable results.
	// When writing a Maya library mode application, a call to this method should be the first line of main.
	if (!MLibrary::initialize("Exporter", true))
	{
		std::cout << "<Error> MLibrary::initialize()" << std::endl;
		return false;
	}
	return true;
}

// f�rst�r dynamiskt allokerade objekt etc...
void Exporter::CleanUpMaya()
{
	
	// [Ur Maya-dokumentationen]
	// Undo the initialization performed by the initialize method, cleanup allocated Maya structures in an orderly fashion and terminate the application.
	// Note: It is important that when a Library mode process terminates it calls this method before doing so. Failure to do so could result in the leaking of licenses and errors from Maya's static destructors.
	// If the exitWhenDone parameter is true, which is the default, this method will terminate the process, using the supplied exitCode, and control will NOT be returned to the caller.
	// If the Library app was launched from another process as a separate thread or a child process, this can result in the parent process terminating as well.
	// To avoid that, set exitWhenDone to false, which will prevent the current process from being terminated and will return control to the caller.
	// However, in that case, Library mode must not be re-entered within the same process as Maya will be in an undetermined state and its behavour would be unpredictable.
	MLibrary::cleanup(0, false);
}

bool Exporter::CreateExportFiles(std::string file_path)
{
	// hitta index f�r punkten innan filtypen.
	int sub_string_length = (int)file_path.find_last_of(".", file_path.size() - 1);

	// spargenv�g f�r den exporterade filen.
	std::string save_path = file_path.substr(0, sub_string_length) + ".txt";

	std::cout << "Exporting file to " << save_path.c_str() << std::endl << std::endl;

	export_stream_.open(save_path.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!export_stream_.is_open())
	{
		std::cout << "<Error> fstream::open()" << std::endl;
		return false;
	}

	return true;
}

void Exporter::CloseExportFiles()
{
	if (export_stream_.is_open())
	{
		export_stream_.close();
	}
}

bool Exporter::GetMayaFilenamesInDirectory(char *folder_path, std::vector<std::string> &list_to_fill)
{
	WIN32_FIND_DATA fdata;
	HANDLE dhandle;

	// m�ste l�gga till \* till genv�gen
	{
		char buf[MAX_PATH];
		sprintf_s(buf, sizeof(buf), "%s\\*", folder_path);
		if ((dhandle = FindFirstFile(buf, &fdata)) == INVALID_HANDLE_VALUE) {
			return false;
		}
	}

	// �ven en "tom" katalog ger tv� resultat - . och ..
	// vi vill inte ha dem :-)
	//fileList.push_back(fdata.cFileName);

	while (true)
	{
		if (FindNextFile(dhandle, &fdata))
		{
			// vi vill endast ha ".mb"-filer
			if (strlen(fdata.cFileName) > 4)
			{
				if (strcmp(&fdata.cFileName[strlen(fdata.cFileName)-3], ".mb") == 0)
				{
					list_to_fill.push_back(fdata.cFileName);
				}
			}
		}
		else
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				break;
			}
			else
			{
				FindClose(dhandle);
				return false;
			}
		}
	}

	if (!FindClose(dhandle))
	{
		return false;
	}

	return true;
}

// funktionen som startar allting - initierar Maya, h�mtar Mayafilernas namn, g�r igenom scenen, etc...
void Exporter::StartExporter(std::string directory_path)
{
	std::cout << "Initializing exporter" << std::endl << std::endl;
	if (!InitializeMaya())
	{
		std::cout << "Failed to initialize exporter" << std::endl;
		return;
	}

	std::vector<std::string> file_list;

	if (!GetMayaFilenamesInDirectory((char*)directory_path.c_str(), file_list))
	{
		return;
	}

	char tmp_str[MAX_PATH];

	for (auto file = file_list.begin(); file != file_list.end(); file++)
	{
		// Formaterar och lagrar tecken och v�rden i en buffer
		sprintf_s(tmp_str, sizeof(tmp_str), "%s%s", directory_path.c_str(), file->c_str());

		ProcessScene(tmp_str);

		CloseExportFiles();
	}

	return;
}

void Exporter::ProcessScene(const char *file_path)
{
	MStatus status = MS::kSuccess;

	// [Ur Maya-dokumentationen]
	// Set everything back to a new file state.
	status = MFileIO::newFile(true);
	if (!status)
	{
		std::cout << "<Error> MFileIO::newFile()" << std::endl;
		return;
	}

	// [Ur Maya-dokumentationen]
	// Open the given file, and set the current active file to this file. If there are unsaved changes in the current scene, this operation will fail unless the force flag is set to true.
	status = MFileIO::open(file_path, NULL, true);
	if (!status)
	{
		std::cout << "<Error> MFileIO::open()" << std::endl;
		return;
	}

	// skapar exportfilerna
	if (!CreateExportFiles(file_path))
	{
		std::cout << "<Error> CreateExportFiles()" << std::endl;
		return;
	}

	// identifiera och extrahera data fr�n scenen
	std::cout << "Identifying scene information" << std::endl << std::endl;
	if (!IdentifyAndExtractScene())
	{
		std::cout << "<Error> IdentifyAndExtractScene()" << std::endl;
		return;
	}
	else
	{
		// exportera data till fil
		std::cout << "Exporting scene information" << std::endl << std::endl;
		ExportScene();
	}
}

//___________________________________________________________________________________________
//|																							|
//|								FUNKTIONER F�R ATT IDENTIFIERA DATA							|
//|_________________________________________________________________________________________|

// identifiera och extrahera data fr�n scenen
bool Exporter::IdentifyAndExtractScene()
{
	bool status = true;
	
	status = IdentifyAndExtractMeshes();

	return status;
}


// identifierar alla mesharna i scenen och extraherar data fr�n dem
bool Exporter::IdentifyAndExtractMeshes()
{
	UINT index = 0;
	scene_.meshes.clear();

	MDagPath dag_path;
	MItDag dag_iter(MItDag::kBreadthFirst, MFn::kMesh);

	while (!dag_iter.isDone())
	{
		if (dag_iter.getPath(dag_path))
		{
			MFnDagNode dag_node = dag_path.node();
			
			// vill endast ha "icke-history"-f�rem�l
			if (!dag_node.isIntermediateObject())
			{
				MFnMesh mesh(dag_path);
				ExtractMeshData(mesh, index);
				index++;
			}
		}

		dag_iter.next();
	}
	
	dag_iter.reset();
	dag_iter = MItDag(MItDag::kBreadthFirst, MFn::kCamera);

	while (!dag_iter.isDone())
	{
		printf("aye");
		printf("aye");
		dag_iter.next();
	}

	return true;
}

//___________________________________________________________________________________________
//|																							|
//|								FUNKTIONER F�R ATT EXTRAHERA DATA							|
//|_________________________________________________________________________________________|

// h�mta all n�dv�ndig data och l�gger det i ett MeshData-objekt, som senare anv�nds vid exportering.
bool Exporter::ExtractMeshData(MFnMesh &mesh, UINT index)
{
	MeshData mesh_data;

	MSpace::Space world_space = MSpace::kWorld;

	// DAG-path
	mesh_data.mesh_path = mesh.dagPath();

	// namn och id
	mesh_data.name = mesh.name();
	mesh_data.id = index;

	// triangulera meshen innan man h�mtar punkterna
	MString command = "polyTriangulate -ch 1 " + mesh_data.name;
	if (!MGlobal::executeCommand(command))
	{
		return false;
	}

	// h�mta icke-indexerade vertexpunkter
	if (!mesh.getPoints(mesh_data.points, world_space))
	{
		return false;
	}

	// h�mta icke-indexerade normaler
	if (!mesh.getNormals(mesh_data.normals, world_space))
	{
		return false;
	}

	//variabler f�r att mellanlagra uvdata och tangenter/bitangenter
	MStringArray uvSets;
	mesh.getUVSetNames(uvSets);

	uvSet tempUVSet;
	// iterera �ver uvsets och ta ut koordinater, tangenter och bitangenter
	for (int i = 0; i < uvSets.length(); i++)
	{
		MString currentSet = uvSets[i];
		mesh.getUVs(tempUVSet.Us, tempUVSet.Vs, &currentSet);
		mesh_data.uvSets.push_back(tempUVSet);

		mesh.getTangents(mesh_data.uvSets[i].tangents, world_space, &currentSet);
		mesh.getBinormals(mesh_data.uvSets[i].binormals, world_space, &currentSet);
	}

	// l�gg till mesh_data i scen-datan
	scene_.meshes.push_back(mesh_data);

	return true;
}


//___________________________________________________________________________________________
//|																							|
//|								FUNKTIONER F�R ATT EXPORTERA								|
//|_________________________________________________________________________________________|
#include<maya/MFnCamera.h>
// exporterar scenen
void Exporter::ExportScene()
{
	ExportMeshes();
}

// exporterar alla meshar (antal meshar, namn per mesh, antal vertexpunkter per mesh och vertexpositioner per mesh)
void Exporter::ExportMeshes()
{
	export_stream_ << "\tmeshes " << scene_.meshes.size() << std::endl;
	for (auto mesh_iter = scene_.meshes.begin(); mesh_iter != scene_.meshes.end(); mesh_iter++)
	{
		export_stream_ << "\t\tname " << mesh_iter->name.asChar() << std::endl;

		MItMeshPolygon polygon_iter(mesh_iter->mesh_path);
		int vertexIndex = 0;

		export_stream_ << "\t\t\tvertices " << polygon_iter.count() * 3 << std::endl;

		int j = 0;
		while (j != 2)
		{
			while (!polygon_iter.isDone())
			{
				MIntArray index_array;
				polygon_iter.getVertices(index_array);
				if (index_array.length() == 3)
				{
					for (int i = 0; i < 3; i++)
					{
						if (j == 0)
						{
							vertexIndex = polygon_iter.vertexIndex(2 - i);
							export_stream_ << "\t\t\t\tp " <<
								mesh_iter->points[vertexIndex].x << " " <<
								mesh_iter->points[vertexIndex].y << " " <<
								mesh_iter->points[vertexIndex].z << std::endl;
						}
						else
						{
							export_stream_ << "\t\t\t\tn " <<
								mesh_iter->normals[vertexIndex].x << " " <<
								mesh_iter->normals[vertexIndex].y << " " <<
								mesh_iter->normals[vertexIndex].z << std::endl;
						}
					}
				}
				else
				{
					std::cout << "Error: non-triangular polygon detected. Please only use triangles." << std::endl;
					std::cout << "Attempts to continue export with missing polygon..." << std::endl;
				}
				polygon_iter.next();
			}
			//Reset Polygon iterationen f�r normalerna
			polygon_iter.reset();
			j++;
		}


		for (int i = 0; i < mesh_iter->uvSets.size(); i++)
		{
			export_stream_ << "\t\t\tUVSet " << i << std::endl;

			for (int x = 0; x < mesh_iter->uvSets[i].Us.length(); x++)
			{
				for (int x = 0; x < mesh_iter->uvSets[i].Us.length(); x++)
				{
					export_stream_ << "\t\t\t\tuv " << mesh_iter->uvSets[i].Us[x] << " " << mesh_iter->uvSets[i].Vs[x] << std::endl;
				}

				for (int x = 0; x < mesh_iter->uvSets[i].Us.length(); x++)
				{
					export_stream_ << "\t\t\t\tt " << mesh_iter->uvSets[i].tangents[x] << std::endl;
				}

				for (int x = 0; x < mesh_iter->uvSets[i].Us.length(); x++)
				{
					export_stream_ << "\t\t\t\tb " << mesh_iter->uvSets[i].binormals[x] << std::endl;
				}
			}
		}
	}
}