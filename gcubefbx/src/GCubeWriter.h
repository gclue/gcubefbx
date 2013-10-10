//
//  GCubeWriter.h
//  gcubefbx
//
//  Created by Takashi Tsuchiya on 2013/09/12.
//  Copyright (c) 2013年 GClue, Inc. All rights reserved.
//

#ifndef __gcubefbx__GCubeWriter__
#define __gcubefbx__GCubeWriter__

#include <fbxsdk.h>
#include "BinaryOutputStream.h"
#include <map>
#include <vector>

class GCubeWriter : public FbxWriter
{
public:
    GCubeWriter(FbxManager &pManager, int pID);
    virtual ~GCubeWriter();
	
    virtual bool FileCreate(char* pFileName) ;
    virtual bool FileClose() ;
    virtual bool IsFileOpen();
    virtual void GetWriteOptions() ;
    virtual void WriteVertexData(FbxNode* pStartNode, int index=0);
    virtual void WriteSkeletonData(FbxNode *startNode, int index=0);
	virtual void WriteWeightData(FbxNode* pStartNode);
    virtual bool Write(FbxDocument* pDocument);
	
    virtual bool PreprocessScene(FbxScene &pScene);
    virtual bool PostprocessScene(FbxScene &pScene);
	
	void parseVertex(FbxMesh* mesh);
	void parseVertexNormals(FbxMesh *mesh);
	void parseVertexUV(FbxMesh* mesh);
	void parseSkin(FbxMesh* pMesh);
	bool FindBindPoseMatrix(FbxNode* startNode, FbxNode *node, FbxMatrix &outmtx);
	
private:
	BinaryOutputStream bos;
    FbxManager*	mManager;
	std::map<int, std::vector<float>> skinMap;
	std::vector<const char*> nodeVec;
};

#endif /* defined(__gcubefbx__GCubeWriter__) */
