//
//  main.cpp
//  fbxconv
//
//  Created by Takashi Tsuchiya on 2013/09/12.
//  Copyright (c) 2013年 GClue, Inc. All rights reserved.
//

#include <iostream>
#include <fbxsdk.h>
#include <stdio.h>
#include <unistd.h>

//int main(int argc, char** argv)
int main(int argc, const char * argv[])
{
	const char *input_path = NULL;
	int c;
	while ((c = getopt(argc, (char**)argv, "hi:")) != -1) {
		switch (c) {
			case 'i':
				input_path = optarg;
				break;
			default:
				printf("引数指定の誤り: 未知の引数が指定されました。\n");
			case 'h':
				return 1;
		}
	}
	
	// デバッグ用
	if (!input_path) {
//		input_path = "/Users/tsuchiya/Downloads/dae/lica_jang/jang.FBX";
//		input_path = "/Users/tsuchiya/Downloads/car0905/car.FBX";
//		input_path = "/Users/tsuchiya/Documents/workspace/neko/examples/collada/01walk.dae";
		input_path = "/Users/tsuchiya/Downloads/dae/j1pf65nqut-lowPolyJet.FBX";
	}

	// Convert
	FbxManager* manager = FbxManager::Create();
	FbxString lExtension = "dylib";
	FbxString lPath = FbxGetApplicationDirectory();
	manager->LoadPluginsDirectory(lPath.Buffer(), lExtension.Buffer());
	
    FbxImporter* importer = FbxImporter::Create(manager, "Importer");
    FbxScene* scene = FbxScene::Create(manager, "Scene");
	
    importer->Initialize(input_path);
    importer->Import(scene);
	
	FbxExporter* lExporter = FbxExporter::Create(manager, "");
	for (int i=0; i<2; i++) {
		FbxString path;
		if (i==0) {
			path = FbxPathUtils::ChangeExtension(input_path, ".gcb");
		} else {
			path = FbxPathUtils::ChangeExtension(input_path, ".gav");
		}
		printf("%s\n", path.Buffer());
		if(lExporter->Initialize(path.Buffer(), -1, manager->GetIOSettings()) == false)
		{
			FBXSDK_printf("Call to FbxExporter::Initialize() failed.¥n");
			FBXSDK_printf("Error returned: %s¥n¥n", lExporter->GetStatus().GetErrorString());
			return false;
		}
		lExporter->Export(scene);
	}
	
	
	
    manager->Destroy();
	
	
	
    return 0;
}

