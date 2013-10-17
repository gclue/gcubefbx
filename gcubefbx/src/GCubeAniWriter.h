//
//  GCubeAniWriter.h
//  gcubefbx
//
//  Created by Takashi Tsuchiya on 2013/10/10.
//  Copyright (c) 2013年 GClue, Inc. All rights reserved.
//

#ifndef __gcubefbx__GCubeAniWriter__
#define __gcubefbx__GCubeAniWriter__


#include <fbxsdk.h>
#include "BinaryOutputStream.h"
#include <map>
#include <vector>

class GCubeAniWriter : public FbxWriter
{
public:
    GCubeAniWriter(FbxManager &pManager, int pID);
    virtual ~GCubeAniWriter();
	
    virtual bool FileCreate(char* pFileName) ;
    virtual bool FileClose() ;
    virtual bool IsFileOpen();
    virtual void GetWriteOptions() ;
	virtual void WriteAnimationData(FbxNode* pStartNode);
	virtual void CountAnimationData(FbxNode* pStartNode);
    virtual bool Write(FbxDocument* pDocument);
	
    virtual bool PreprocessScene(FbxScene &pScene);
    virtual bool PostprocessScene(FbxScene &pScene);
	
	bool IsTranslationAnimated(const FbxNode *pNode);
    bool IsRotationAnimated(const FbxNode *pNode);
    bool IsRotationAnimated(const FbxNode *pNode, int pAxis);
    bool IsScaleAnimated(const FbxNode *pNode);

	
private:
	BinaryOutputStream bos;
    FbxManager*	mManager;
    FbxAnimStack* mAnimStack;
    FbxAnimLayer* mAnimLayer;
    FbxTime mSamplingPeriod;
	int animationCount;
};

#endif /* defined(__gcubefbx__GCubeAniWriter__) */
