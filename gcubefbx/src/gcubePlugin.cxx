//
//  gcubePlugin.cxx
//  gcubefbx
//
//  Created by Takashi Tsuchiya on 2013/09/12.
//  Copyright (c) 2013年 GClue, Inc. All rights reserved.
//

#include "GCubeWriter.h"

#define PLUGIN_NAME			"GCube"
#define PLUGIN_VERSION		"1.0"
#define PLUGIN_EXTENSION	"gcb"

// Create your own writer.
// And your writer will get a pPluginID and pSubID.
FbxWriter* CreateGCubeWriter(FbxManager& pManager, FbxExporter& pExporter, int pSubID, int pPluginID)
{
	FbxWriter* lWriter = FbxNew< GCubeWriter >(pManager, pPluginID);
	lWriter->SetIOSettings(pExporter.GetIOSettings());
	return lWriter;
}

// Get extension, description or version info about MyOwnWriter
void* GetGCubeWriterInfo(FbxWriter::EInfoRequest pRequest, int pId)
{
    static const char* sExt[] = {PLUGIN_EXTENSION, 0};
    static const char* sDesc[] = {PLUGIN_NAME"Writer", 0};
	
    switch( pRequest )
    {
		case FbxWriter::eInfoExtension:	return sExt;
		case FbxWriter::eInfoDescriptions:	return sDesc;
		case FbxWriter::eInfoVersions:		return 0;
		default:							return 0;
    }
}

void FillGCubeWriterIOSettings(FbxIOSettings& pIOS)
{
    // Here you can write your own FbxIOSettings and parse them.
    FbxProperty FBXExtentionsSDKGroup = pIOS.GetProperty(EXP_FBX_EXT_SDK_GRP);
    if( !FBXExtentionsSDKGroup.IsValid() ) return;
	
    FbxProperty IOPluginGroup = pIOS.AddPropertyGroup(FBXExtentionsSDKGroup, PLUGIN_NAME, FbxStringDT, PLUGIN_NAME);
    if( IOPluginGroup.IsValid() )
    {
        //Add your plugin export options here...
        //Example:
        bool Default_True = true;
        pIOS.AddProperty(IOPluginGroup, "Test", FbxBoolDT, "Test", &Default_True);
    }
}


class GCubeWriterReaderPlugin : public FbxPlugin
{
    FBXSDK_PLUGIN_DECLARE(GCubeWriterReaderPlugin);
	
protected:
    explicit GCubeWriterReaderPlugin(const FbxPluginDef& pDefinition, FbxModule pFbxModule) : FbxPlugin(pDefinition, pFbxModule)
    {
    }
	
    // Implement kfbxmodules::FbxPlugin
    virtual bool SpecificInitialize()
    {
        int FirstPluginID, RegistredCount;
        GetData().mSDKManager->GetIOPluginRegistry()->RegisterWriter(CreateGCubeWriter, GetGCubeWriterInfo, FirstPluginID, RegistredCount, FillGCubeWriterIOSettings);
        return true;
    }
	
    virtual bool SpecificTerminate()
    {
        return true;
    }
};

FBXSDK_PLUGIN_IMPLEMENT(GCubeWriterReaderPlugin);

// FBX Interface
extern "C"
{
    //The DLL is owner of the plug-in
    static GCubeWriterReaderPlugin* sPlugin = NULL;
	
    //This function will be called when an application will request the plug-in
	FBXSDK_DLLEXPORT void FBXPluginRegistration(FbxPluginContainer& pContainer, FbxModule pFbxModule)
    {
        if( sPlugin == NULL )
        {
            //Create the plug-in definition which contains the information about the plug-in
            FbxPluginDef sPluginDef;
            sPluginDef.mName = PLUGIN_NAME"Plugin";
            sPluginDef.mVersion = PLUGIN_VERSION;
			
            //Create an instance of the plug-in.  The DLL has the ownership of the plug-in
            sPlugin = GCubeWriterReaderPlugin::Create(sPluginDef, pFbxModule);
			
            //Register the plug-in
            pContainer.Register(*sPlugin);
        }
    }
}
