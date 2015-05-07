#include <fbxsdk.h>
#include <fbxsdk\fileio\fbxiosettings.h>

#pragma comment(lib, "libfbxsdk.lib")

int main(int argc, char** argv) {

	//Create the FBX SDK manager
	FbxManager* lSdkManager = FbxManager::Create();

	//Create an IOSettings object.
	FbxIOSettings * ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);
	                              
	//Configure the FbxIOSettings object
	(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
	(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
	(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, false);
	(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, true);
	(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, false);
	(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
	(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	bool lEmbedMedia = true;
	(*(lSdkManager->GetIOSettings())).SetBoolProp(EXP_FBX_EMBEDDED, lEmbedMedia);


	FbxScene* lScene = FbxScene::Create(lSdkManager, "newScene");
	
	//Export the contents of the file.	
	//Create an exporter.
	FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

	// Declare the path and filename of the file to which the scene will be exported.
	const char* lFilenameOut = "file.fbx";
		
	// Initialize the exporter.
	bool lExportStatus = lExporter->Initialize(lFilenameOut, -1, lSdkManager->GetIOSettings());

	if (!lExportStatus) {
		printf("Call to FbxExporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		return false;
	}

	// Create a cube.
	typedef double Vector4[4];
	typedef double Vector2[2];

		// indices of the vertices per each polygon
		static int vtxId[24] = {
			0, 1, 2, 3, // front  face  (Z+)
			1, 5, 6, 2, // right  side  (X+)
			5, 4, 7, 6, // back   face  (Z-)
			4, 0, 3, 7, // left   side  (X-)
			0, 4, 5, 1, // bottom face  (Y-)
			3, 2, 6, 7  // top    face  (Y+)
		};

		// control points
		static Vector4 lControlPoints[8] = {
			{ -5.0, 0.0, 5.0, 1.0 }, { 5.0, 0.0, 5.0, 1.0 }, { 5.0, 10.0, 5.0, 1.0 }, { -5.0, 10.0, 5.0, 1.0 },
			{ -5.0, 0.0, -5.0, 1.0 }, { 5.0, 0.0, -5.0, 1.0 }, { 5.0, 10.0, -5.0, 1.0 }, { -5.0, 10.0, -5.0, 1.0 }
		};

		// normals
		static Vector4 lNormals[8] = {
			{ -0.577350258827209, -0.577350258827209, 0.577350258827209, 1.0 },
			{ 0.577350258827209, -0.577350258827209, 0.577350258827209, 1.0 },
			{ 0.577350258827209, 0.577350258827209, 0.577350258827209, 1.0 },
			{ -0.577350258827209, 0.577350258827209, 0.577350258827209, 1.0 },
			{ -0.577350258827209, -0.577350258827209, -0.577350258827209, 1.0 },
			{ 0.577350258827209, -0.577350258827209, -0.577350258827209, 1.0 },
			{ 0.577350258827209, 0.577350258827209, -0.577350258827209, 1.0 },
			{ -0.577350258827209, 0.577350258827209, -0.577350258827209, 1.0 }
		};

		// uvs
		static Vector2 lUVs[14] = {
			{ 0.0, 1.0 },
			{ 1.0, 0.0 },
			{ 0.0, 0.0 },
			{ 1.0, 1.0 }
		};

		// indices of the uvs per each polygon
		static int uvsId[24] = {
			0, 1, 3, 2, 2, 3, 5, 4, 4, 5, 7, 6, 6, 7, 9, 8, 1, 10, 11, 3, 12, 0, 2, 13
		};

		// create the main structure.
		FbxMesh* lMesh = FbxMesh::Create(lScene, "");

		// Create control points.
		lMesh->InitControlPoints(8);
		FbxVector4* vertex = lMesh->GetControlPoints();
		memcpy((void*)vertex, (void*)lControlPoints, 8 * sizeof(FbxVector4));

		// create the materials.
		/* Each polygon face will be assigned a unique material.
		*/
		FbxGeometryElementMaterial* lMaterialElement = lMesh->CreateElementMaterial();
		lMaterialElement->SetMappingMode(FbxGeometryElement::eAllSame);
		lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

		lMaterialElement->GetIndexArray().Add(0);

		// Create polygons later after FbxGeometryElementMaterial is created. Assign material indices.
		int vId = 0;
		for (int f = 0; f<6; f++)
		{
			lMesh->BeginPolygon();
			for (int v = 0; v<4; v++)
				lMesh->AddPolygon(vtxId[vId++]);
			lMesh->EndPolygon();
		}

		// specify normals per control point.
		FbxGeometryElementNormal* lNormalElement = lMesh->CreateElementNormal();
		lNormalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
		lNormalElement->SetReferenceMode(FbxGeometryElement::eDirect);

		for (int n = 0; n<8; n++)
			lNormalElement->GetDirectArray().Add(FbxVector4(lNormals[n][0], lNormals[n][1], lNormals[n][2]));


		// Create the node containing the mesh
		FbxNode* lNode = FbxNode::Create(lScene, "notTHEBEEEEEEES!!!");
		//lNode->LclTranslation.Set(pLclTranslation);

		lNode->SetNodeAttribute(lMesh);
		lNode->SetShadingMode(FbxNode::eTextureShading);

		// create UVset
		FbxGeometryElementUV* lUVElement1 = lMesh->CreateElementUV("UVSet1");
		FBX_ASSERT(lUVElement1 != NULL);
		lUVElement1->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
		lUVElement1->SetReferenceMode(FbxGeometryElement::eIndexToDirect);
		for (int i = 0; i <4; i++)
			lUVElement1->GetDirectArray().Add(FbxVector2(lUVs[i][0], lUVs[i][1]));

		for (int i = 0; i<24; i++)
			lUVElement1->GetIndexArray().Add(uvsId[i % 4]);

		//return lNode;
		// Create a node for our mesh in the scene.
		FbxNode* lMeshNode = FbxNode::Create(lScene, "theThing");

		// Set the node attribute of the mesh node.
		lMeshNode->SetNodeAttribute(lMesh);

		// Add the mesh node to the root node in the scene.
		FbxNode *lRootNode = lScene->GetRootNode();
		lRootNode->AddChild(lNode);

	lExporter->Export(lScene);

	//Get rid of objects
	lExporter->Destroy();

	return 0;
}
