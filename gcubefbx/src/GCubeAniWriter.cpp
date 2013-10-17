//
//  GCubeAniWriter.cpp
//  gcubefbx
//
//  Created by Takashi Tsuchiya on 2013/10/10.
//  Copyright (c) 2013年 GClue, Inc. All rights reserved.
//

#include "GCubeAniWriter.h"

#include <vector>


static FbxString matrixToString(const FbxAMatrix& mx)
{
	
    FbxString lStr = '\t';
    lStr += FbxString(mx.mData[0][0]) + '\t';
    lStr += FbxString(mx.mData[1][0]) + '\t';
    lStr += FbxString(mx.mData[2][0]) + '\t';
    lStr += FbxString(mx.mData[3][0]) + "\n\t";
    lStr += FbxString(mx.mData[0][1]) + '\t';
    lStr += FbxString(mx.mData[1][1]) + '\t';
    lStr += FbxString(mx.mData[2][1]) + '\t';
    lStr += FbxString(mx.mData[3][1]) + "\n\t";
    lStr += FbxString(mx.mData[0][2]) + '\t';
    lStr += FbxString(mx.mData[1][2]) + '\t';
    lStr += FbxString(mx.mData[2][2]) + '\t';
    lStr += FbxString(mx.mData[3][2]) + "\n\t";
    lStr += FbxString(mx.mData[0][3]) + '\t';
    lStr += FbxString(mx.mData[1][3]) + '\t';
    lStr += FbxString(mx.mData[2][3]) + '\t';
    lStr += FbxString(mx.mData[3][3]) + '\n';
    return lStr;
}


//Constructor
GCubeAniWriter::GCubeAniWriter(FbxManager &pManager, int pID):
	FbxWriter(pManager, pID, FbxStatusGlobal::GetRef()),
	mManager(&pManager)
{
}

//Destructor
GCubeAniWriter::~GCubeAniWriter()
{
}

// Create a file stream with pFileName
bool GCubeAniWriter::FileCreate(char* pFileName)
{
	if (bos.open(pFileName)) {
		bos.writeByte('g');
		bos.writeByte('a');
		bos.writeByte('n');
		bos.writeByte('i');
		return true;
	} else {
		return false;
	}
}

// Close the file stream
bool GCubeAniWriter::FileClose()
{
	bos.close();
	return true;
}

// Check whether the file stream is open.
bool GCubeAniWriter::IsFileOpen()
{
	return bos.fout->IsOpen();
}

// Get the file stream options
void GCubeAniWriter::GetWriteOptions()
{
}

static FbxNode *RootNode;
// Write file with stream options
bool GCubeAniWriter::Write(FbxDocument* pDocument)
{
	if (!pDocument)
	{
		GetStatus().SetCode(FbxStatus::eFailure, "Invalid document handle");
		return false;
	}
	
	FbxScene* lScene = FbxCast<FbxScene>(pDocument);
	
	if (!lScene)
	{
		GetStatus().SetCode(FbxStatus::eFailure, "Document not supported");
		return false;
	}
	
	bool lIsAScene = (lScene != NULL);
	bool lResult = false;
	
	if(lIsAScene)
	{
		PreprocessScene(*lScene);
		
		// convert axis
		FbxAxisSystem lAxisSystem( FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded );
		lAxisSystem.ConvertScene(lScene);
		
		// Triangulate
		FbxGeometryConverter converter(this->mManager);
		if (!converter.Triangulate(lScene, true)) {
			FBXSDK_printf("Triangulate error\n");
			return false;
		}
		
		// write data
		FbxNode* lRootNode = lScene->GetRootNode();
		RootNode = lRootNode;
		printf("WriteAnimationData\n");
		animationCount = 0;
		CountAnimationData(lRootNode);
		bos.writeInt(animationCount);
		WriteAnimationData(lRootNode);
		
		PostprocessScene(*lScene);
		lResult = true;
	}
	return lResult;
}


//
void GCubeAniWriter::WriteAnimationData(FbxNode* pNode)
{
    int i;
	
    // If the node is animated, export its animation.
	bool isAnimated = FbxAnimUtilities::IsAnimated(pNode);
    if (isAnimated == false && pNode->GetNodeAttribute())
		isAnimated = FbxAnimUtilities::IsAnimated(pNode->GetNodeAttribute());
	
    if (isAnimated) {
        FbxString lName = pNode->GetNameWithoutNameSpacePrefix();
		
		if ( IsTranslationAnimated( pNode ) || IsRotationAnimated( pNode ) || IsScaleAnimated( pNode )) {
			// replace all space to underscore
			FbxString str = "node-";
			str = str + pNode->GetNameOnly();
			str.ReplaceAll(' ', '_');
			const char* childName = str.Buffer();
			printf("name:%s\n", childName);
			
			// name
			bos.writeShort(strlen(childName));
			bos.writeString(childName);
			
			// time span
			FbxTimeSpan lTimeInterval(FBXSDK_TIME_INFINITE, FBXSDK_TIME_MINUS_INFINITE);
            pNode->GetAnimationInterval( lTimeInterval, mAnimStack );
            FbxTime lTime = lTimeInterval.GetStart();
			
			// count
			int cnt = lTimeInterval.GetStop().GetSecondDouble()/mSamplingPeriod.GetSecondDouble() +1;
			printf("cnt:%d\n", cnt);
			bos.writeInt(cnt);
			
			//            FbxArray<FbxAMatrix> lOutputs;
			while(lTime < lTimeInterval.GetStop() + FBXSDK_TIME_EPSILON) //Don't miss the last frame
			{
                FbxAMatrix lThisLocal;
				//For Single Matrix situation, obtain transform matrix from eDestinationPivot, which include pivot offsets and pre/post rotations.
				FbxAMatrix& lThisGlobal = pNode->EvaluateGlobalTransform(lTime, FbxNode::eDestinationPivot);
                FbxNode* lParentNode = pNode->GetParent();
                if ( lParentNode )
                {
					//For Single Matrix situation, obtain transform matrix from eDestinationPivot, which include pivot offsets and pre/post rotations.
					FbxAMatrix& lParentGlobal = lParentNode->EvaluateGlobalTransform(lTime, FbxNode::eDestinationPivot);
                    FbxAMatrix lParentInverted = lParentGlobal.Inverse();
                    lThisLocal = lParentInverted * lThisGlobal;
                }
                else
                {
                    lThisLocal = lThisGlobal;
                }
				
				// target all COLLADA_MATRIX_STRUCTURE - single matrix is written in row-major order in COLLADA document for human readability
				// when represents a transformation (TRS), the last column of the matrix represents the translation part of the transformation.
				
				// time
				bos.writeFloat(lTime.GetSecondDouble());
				// matrix
				for (int i=0; i<4; i++) {
					for (int j=0; j<4; j++) {
						bos.writeFloat(lThisLocal.mData[i][j]);
					}
				}
				
				lTime += mSamplingPeriod;
            }
		}
		
    }
	
    // Export animation recursively
    for (i = 0; i < pNode->GetChildCount(); i++) {
        WriteAnimationData(pNode->GetChild(i));
    }
}


//
void GCubeAniWriter::CountAnimationData(FbxNode* pNode)
{
	pNode->SetPivotState(FbxNode::eSourcePivot, FbxNode::ePivotActive);
	pNode->SetPivotState(FbxNode::eDestinationPivot, FbxNode::ePivotActive);
	
    int i;
	
    // If the node is animated, export its animation.
	bool isAnimated = FbxAnimUtilities::IsAnimated(pNode);
    if (isAnimated == false && pNode->GetNodeAttribute())
		isAnimated = FbxAnimUtilities::IsAnimated(pNode->GetNodeAttribute());
	
    if (isAnimated) {
		
		if ( IsTranslationAnimated( pNode ) || IsRotationAnimated( pNode ) || IsScaleAnimated( pNode )) {
			animationCount++;
		}

    }
	
    // Export animation recursively
    for (i = 0; i < pNode->GetChildCount(); i++) {
        CountAnimationData(pNode->GetChild(i));
    }
}



bool GCubeAniWriter::IsTranslationAnimated(const FbxNode *pNode)
{
    // If null, there is no animation for sure
    FbxAnimCurveNode* acn = const_cast<FbxNode*>(pNode)->LclTranslation.GetCurveNode(mAnimLayer);
    if (acn == NULL) return false;
    
    for( unsigned int i = 0; i < acn->GetChannelsCount(); i++ )
    {
        FbxAnimCurve* fc = acn->GetCurve(i);
        if (fc && fc->KeyGetCount() > 0)
            return true;
    }
	
    return false;
	
} // IsTranslationAnimated


bool GCubeAniWriter::IsRotationAnimated(const FbxNode *pNode)
{
    FbxAnimCurveNode* acn = const_cast<FbxNode*>(pNode)->LclRotation.GetCurveNode(mAnimLayer);
    if (acn == NULL) return false;
    
    for( unsigned int i = 0; i < acn->GetChannelsCount(); i++ )
    {
        FbxAnimCurve* fc = acn->GetCurve(i);
        if (fc && fc->KeyGetCount() > 0)
            return true;
    }
	
    return false;
	
} // IsRotationAnimated

bool GCubeAniWriter::IsRotationAnimated(const FbxNode *pNode, int pAxis)
{
    FbxAnimCurveNode* acn = const_cast<FbxNode*>(pNode)->LclRotation.GetCurveNode(mAnimLayer);
    if (acn == NULL) return false;
    
    for( unsigned int i = 0; i < acn->GetChannelsCount(); i++ )
    {
        FbxAnimCurve* fc = acn->GetCurve(i);
        if (fc && fc->KeyGetCount() > 0 && i == pAxis)
            return true;
    }
    return false;
}

bool GCubeAniWriter::IsScaleAnimated(const FbxNode *pNode)
{
    FbxAnimCurveNode* acn = const_cast<FbxNode*>(pNode)->LclScaling.GetCurveNode(mAnimLayer);
    if (acn == NULL) return false;
    
    for( unsigned int i = 0; i < acn->GetChannelsCount(); i++ )
    {
        FbxAnimCurve* fc = acn->GetCurve(i);
        if (fc && fc->KeyGetCount() > 0)
            return true;
    }
	
    return false;
}

// Pre-process the scene before write it out
bool GCubeAniWriter::PreprocessScene(FbxScene& pScene)
{
    FbxNode* lRootNode = pScene.GetRootNode();
    mSamplingPeriod.SetSecondDouble( 1./30. );
	lRootNode->ResetPivotSetAndConvertAnimation( 1. / mSamplingPeriod.GetSecondDouble() );

    FbxString lActiveAnimStackName = pScene.ActiveAnimStackName;
    mAnimStack = pScene.FindMember<FbxAnimStack>(lActiveAnimStackName.Buffer());
	if (!mAnimStack)
    {
		// the application has an invalid ActiveAnimStackName, we fallback by using the
		// first animStack.
		mAnimStack = pScene.GetMember<FbxAnimStack>();
    }
    if (mAnimStack == NULL)
    {
        mAnimStack = FbxAnimStack::Create(&pScene, "dummy");
        mAnimLayer = FbxAnimLayer::Create(&pScene, "dummyL");
        mAnimStack->AddMember(mAnimLayer);
    }

    mAnimLayer = mAnimStack->GetMember<FbxAnimLayer>();
	
	return true;
}

// Post-process the scene after write it out
bool GCubeAniWriter::PostprocessScene(FbxScene& /*pScene*/)
{
	return true;
}
