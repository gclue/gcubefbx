//
//  main.cpp
//  fbxconv
//
//  Created by Takashi Tsuchiya on 2013/09/12.
//  Copyright (c) 2013年 GClue, Inc. All rights reserved.
//

#include <iostream>
#include <fbxsdk.h>

int main(int argc, const char * argv[])
{
	
	// insert code here...
//	std::cout << "Hello, World!\n";
	
	FbxManager* manager = FbxManager::Create();
	FbxString lExtension = "dylib";
	FbxString lPath = FbxGetApplicationDirectory();
	manager->LoadPluginsDirectory(lPath.Buffer(), lExtension.Buffer());
	
	//	FbxManager* manager = FbxManager::Create();
    FbxImporter* importer = FbxImporter::Create(manager, "Importer");
    FbxScene* scene = FbxScene::Create(manager, "Scene");
	const char *input_path = "/Users/tsuchiya/Downloads/dae/lica_jang/jang.FBX";
//	const char *input_path = "/Users/tsuchiya/Downloads/dae/jang.DAE";
	const char *output_path = "/Users/tsuchiya/Downloads/dae/lica_jang/jang.gcb";
//	const char *input_path = "/Users/tsuchiya/Downloads/dae/test.fbx";
//	const char *output_path = "/Users/tsuchiya/Downloads/dae/test.gc";
	
    importer->Initialize(input_path);
    importer->Import(scene);
	
//	int pFileFormat = manager->GetIOPluginRegistry()->GetNativeWriterFormat();
//	FBXSDK_printf("%d", pFileFormat);
	
	FbxExporter* lExporter = FbxExporter::Create(manager, "");
	if(lExporter->Initialize(output_path, -1, manager->GetIOSettings()) == false)
    {
        FBXSDK_printf("Call to FbxExporter::Initialize() failed.¥n");
        FBXSDK_printf("Error returned: %s¥n¥n", lExporter->GetStatus().GetErrorString());
        return false;
    }
	
	lExporter->Export(scene);
	
	
    manager->Destroy();
	
	
	
    return 0;
}

