//
//  GCubeWriter.cxx
//  gcubefbx
//
//  Created by Takashi Tsuchiya on 2013/09/12.
//  Copyright (c) 2013年 GClue, Inc. All rights reserved.
//

#include "GCubeWriter.h"

#include <vector>


// 頂点座標のデータ群を表すタグ
#define TYPE_VERTEX 1
// 法線ベクトルのデータ群を表すタグ
#define TYPE_NORMAL 2
// テクスチャのデータ群を表すタグ
#define TYPE_TEXCOOD 3
// インデックスのデータ群を表すタグ
#define TYPE_INDEX 4
// ジョイントのデータ群を表すタグ
#define TYPE_NODE 5
// ジョイントのウェイト群を表すタグ
#define TYPE_WEIGHT 6
// カラーのデータ群を表すタグ
#define TYPE_COLOR 7


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
GCubeWriter::GCubeWriter(FbxManager &pManager, int pID):
	FbxWriter(pManager, pID, FbxStatusGlobal::GetRef()),
	mManager(&pManager)
{
}

//Destructor
GCubeWriter::~GCubeWriter()
{
}

// Create a file stream with pFileName
bool GCubeWriter::FileCreate(char* pFileName)
{
	if (bos.open(pFileName)) {
		bos.writeByte('g');
		bos.writeByte('c');
		bos.writeByte('u');
		bos.writeByte('b');
		return true;
	} else {
		return false;
	}
}

// Close the file stream
bool GCubeWriter::FileClose()
{
	bos.close();
	return true;
}

// Check whether the file stream is open.
bool GCubeWriter::IsFileOpen()
{
	return bos.fout->IsOpen();
}

// Get the file stream options
void GCubeWriter::GetWriteOptions()
{
}

static FbxNode *RootNode;
// Write file with stream options
bool GCubeWriter::Write(FbxDocument* pDocument)
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
		printf("WriteVertexData\n");
		WriteVertexData(lRootNode);
		printf("WriteSkeletonData\n");
		WriteSkeletonData(lRootNode);
		printf("WriteWeightData\n");
		WriteWeightData(lRootNode);
		
		PostprocessScene(*lScene);
		lResult = true;
	}
	return lResult;
}

// Weight
void GCubeWriter::WriteWeightData(FbxNode* pStartNode)
{
	FbxNode* lChildNode;

	for(int i = 0; i<pStartNode->GetChildCount(); i++)
	{
		lChildNode = pStartNode->GetChild(i);
		FbxMesh *pMesh = lChildNode->GetMesh();
		
		static bool flg = false;
		if (pMesh && !flg) {
			if (pMesh->GetDeformerCount(FbxDeformer::eSkin)==0) continue;
			int ccount = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetClusterCount();
			if (ccount==0) continue;
			
			printf("WriteWeightData (%s) ...\n", lChildNode->GetName());
			
			int lDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
			if(lDeformerCount > 1)
				FBX_ASSERT_NOW("Unexpected number of skin greater than 1");

			FbxArray<FbxVector4> lPositions;
			int scnt = 0;
			
			for (int lIndexLink = 0; lIndexLink < ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetClusterCount(); lIndexLink++)
			{
				FbxSkin *skin = (FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin);
				
				FbxCluster* lLink = skin->GetCluster(lIndexLink);
				if (!lLink || !lLink->GetLink()) continue;
				
				int cnt = lLink->GetControlPointIndicesCount();
				
				// find node index
				int nodeIndex;
				printf("skin ... %s(%d)\n", lLink->GetLink()->GetNameWithoutNameSpacePrefix().Buffer(), cnt);
				for(nodeIndex = 0; nodeIndex < nodeVec.size(); nodeIndex++) {
					if(strcmp(lLink->GetLink()->GetNameWithoutNameSpacePrefix().Buffer(), nodeVec[nodeIndex]) == 0) {
						break;
					}
				}
				
				int *idx = lLink->GetControlPointIndices();
				double *wgt = lLink->GetControlPointWeights();
				
				for(int j = 0; j < cnt; ++j){
					int* lLinkControlPointIndices = lLink->GetControlPointIndices();
					if (!lLinkControlPointIndices) continue;
					
					int lIndex = -1;
					int polycnt = pMesh->GetPolygonCount();
					for (int l=0; l<polycnt; l++) {
						for (int m=0; m<3; m++) {
							int index = pMesh->GetPolygonVertex(l, m);
							if (idx[j] == index) {
								// vertex index
								lIndex = l * 3 + m;
								// joint/weight count
								scnt++;
								// make skinmap
								if (skinMap[lIndex].size()>0) {
									std::vector<float> vv = skinMap[lIndex];
									vv.push_back(nodeIndex);
									vv.push_back(wgt[j]);
									skinMap[lIndex] = vv;
								} else {
									std::vector<float> vv;
									vv.push_back(nodeIndex);
									vv.push_back(wgt[j]);
									skinMap[lIndex] = vv;
								}
							}
						}
					}
				}
			}
			
			
			// weight
			bos.writeShort(TYPE_WEIGHT);
			
			// joint
			printf("joint:%d\n", scnt);
			bos.writeInt(scnt);
			int max = (int)skinMap.size();
			for (int ii = 0; ii < max; ii++) {
				int max2 = (int)skinMap[ii].size();
				for (int jj = 0; jj < max2; jj+=2) {
					bos.writeInt((int)skinMap[ii][jj]);
					//printf("%d-%d(%s), ", ii, (int)skinMap[ii][jj], nodeVec[(int)skinMap[ii][jj]]);
				}
			}
			printf("\n");

			// weight
			printf("weight:%d\n", scnt);
			bos.writeInt(scnt);
			for (int ii = 0; ii < max; ii++) {
				int max2 = (int)skinMap[ii].size();
				for (int jj = 1; jj < max2; jj+=2) {
					bos.writeFloat(skinMap[ii][jj]);
					//printf("%f, ", skinMap[ii][jj]);
				}
			}
			printf("\n");

			// count
			max = (int)skinMap.size();
			printf("count:%d\n", max);
			bos.writeInt(max);
			for (int ii = 0; ii < max; ii++) {
				int max2 = (int)skinMap[ii].size()/2;
				bos.writeInt(max2);
				//printf("%d, ", max2);
			}
			printf("\n");
			
			flg = true;
			return;
		}

	}
	
	int lNodeChildCount = pStartNode->GetChildCount ();
	while (lNodeChildCount > 0)
	{
		lNodeChildCount--;
		lChildNode = pStartNode->GetChild (lNodeChildCount);
		WriteWeightData(lChildNode);
	}
}

// vertex data
void GCubeWriter::WriteVertexData(FbxNode* pStartNode, int index)
{
	pStartNode->SetPivotState(FbxNode::eSourcePivot, FbxNode::ePivotActive);
	pStartNode->SetPivotState(FbxNode::eDestinationPivot, FbxNode::ePivotActive);

	FbxMesh* mesh = pStartNode->GetMesh();
	if (mesh) {
		parseVertex(mesh);
	}
	
	FbxNode* lChildNode;
	for(int i = 0; i<pStartNode->GetChildCount(); i++)
	{
		lChildNode = pStartNode->GetChild (i);
		WriteVertexData(lChildNode);
	}
}

static FbxAMatrix lTransformMatrix;

//
void GCubeWriter::parseVertex(FbxMesh* mesh)
{
	//////
	static bool flg = false;
	if (flg) return;
	flg = true;
	/////
	
	printf("    @@ vertex parseing...\n");
	
	// Bind shape matrix
	lTransformMatrix.SetIdentity();
    if (mesh->GetDeformerCount(FbxDeformer::eSkin)) {
		((FbxSkin *)mesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetTransformMatrix(lTransformMatrix);
		FbxString lStrMatrix = matrixToString(lTransformMatrix);
		FBXSDK_printf("bsm:\n%s", lStrMatrix.Buffer());
	}

	// 頂点座標
	int vertexCount = mesh->GetPolygonVertexCount();
	printf("vertexCount: %d\n", vertexCount);
	bos.writeShort(TYPE_VERTEX);
	bos.writeInt(vertexCount*3);
	double *m = lTransformMatrix.mData->Buffer();

	int polycnt = mesh->GetPolygonCount();
	FbxVector4* vertex = mesh->GetControlPoints();
	for (int i=0; i<polycnt; i++) {
		for (int j=0; j<3; j++) {
			int index = mesh->GetPolygonVertex(i, j);
			
			float x = vertex[index][0];
			float y = vertex[index][1];
			float z = vertex[index][2];
			float w = 1.0;//vertex[index][3];
			
			float nx = m[0] * x + m[4] * y + m[ 8] * z + m[12] * w;
			float ny = m[1] * x + m[5] * y + m[ 9] * z + m[13] * w;
			float nz = m[2] * x + m[6] * y + m[10] * z + m[14] * w;
//			float nw = m[3] * x + m[7] * y + m[11] * z + m[15] * w;

			bos.writeFloat(nx);
			bos.writeFloat(ny);
			bos.writeFloat(nz);
		}
	}
	
	// Index
	bos.writeShort(TYPE_INDEX);
	bos.writeInt(vertexCount);
	for (int i = 0; i < vertexCount; i++) {
		bos.writeInt(i);
	}
	
	// Normal
	bos.writeShort(TYPE_NORMAL);
	bos.writeInt(vertexCount*3);
	for (int i=0; i<polycnt; i++) {
		for (int j=0; j<3; j++) {
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(i, j, normal);
			bos.writeFloat(normal[0]);
			bos.writeFloat(normal[1]);
			bos.writeFloat(normal[2]);
		}
	}
	
	// UV
	FbxStringList uvNames;
	mesh->GetUVSetNames(uvNames);
	printf("UVSetName:%s\n", uvNames[0].Buffer());
	
	bos.writeShort(TYPE_TEXCOOD);
	bos.writeInt(vertexCount*2);
	for (int i=0; i<polycnt; i++) {
		for (int j=0; j<3; j++) {
			FbxVector2 uv;
			bool flg;
			mesh->GetPolygonVertexUV(i, j, uvNames[0].Buffer(), uv, flg);
			bos.writeFloat(uv[0]);
			bos.writeFloat(uv[1]);
		}
	}
}

// skeleton
void GCubeWriter::WriteSkeletonData(FbxNode* startNode, int index)
{
	// replace all space to underscore
	FbxString str = "node-";
	str = str + startNode->GetNameOnly();
	str.ReplaceAll(' ', '_');
	const char* childName = str.Buffer();
	
	// debug print
	for (int i=0; i<index; i++) {
		printf("*");
	}
	FBXSDK_printf("%s [%s]\n", childName, startNode->GetTypeName());
	
	// eSkeleton only
	if( index > 0 && startNode->GetNodeAttribute()) {
		if( startNode->GetNodeAttribute() && startNode->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::eSkeleton) {
			return;
		}
		// ネコ用
		if( startNode->GetNameOnly()=="Camera" ) return;
		if( startNode->GetNameOnly()=="Light" ) return;
		
		// name
		bos.writeShort(TYPE_NODE);
		bos.writeByte(1);
		bos.writeShort(strlen(childName));
		bos.writeString(childName);
		
		// matrix
		bos.writeByte(1);
        FbxAMatrix lThisLocal;
		FbxAMatrix& lThisGlobal = const_cast<FbxNode*>(startNode)->EvaluateGlobalTransform(FBXSDK_TIME_ZERO, FbxNode::eDestinationPivot);
		const FbxNode* lParentNode = startNode->GetParent();
        if( lParentNode )
        {
			//For Single Matrix situation, obtain transfrom matrix from eDestinationPivot, which include pivot offsets and pre/post rotations.
			FbxAMatrix & lParentGlobal = const_cast<FbxNode*>(lParentNode)->EvaluateGlobalTransform(FBXSDK_TIME_ZERO, FbxNode::eDestinationPivot);
            FbxAMatrix lParentInverted = lParentGlobal.Inverse();
            lThisLocal = lParentInverted * lThisGlobal;
        }
        else
        {
            lThisLocal = lThisGlobal;
        }
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				bos.writeFloat(lThisLocal.mData[i][j]);
			}
		}
		FBXSDK_printf("matrix:\n%s", matrixToString(lThisLocal).Buffer());

		// bind pose
		FbxMatrix matrix;
		bool hasbp = FindBindPoseMatrix(RootNode, startNode, matrix);
		bos.writeByte(hasbp?1:0);
		if (hasbp) {
			nodeVec.push_back(startNode->GetName());
			for (int i=0; i<4; i++) {
				for (int j=0; j<4; j++) {
					bos.writeFloat(matrix.mData[j][i]);
				}
			}
		}
		
		// children count
		short count = 0;
		for(short i = 0; i<startNode->GetChildCount(); i++)
		{
			FbxNode *node = startNode->GetChild(i);
			if( node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() > FbxNodeAttribute::eNull) {
				count++;
			}
		}
		bos.writeShort(count);
	}
	
	// children
	for(short i = 0; i<startNode->GetChildCount(); i++)
	{
		FbxNode *childNode = startNode->GetChild(i);
		WriteSkeletonData(childNode, index+1);
	}
}


// find bind pose
bool GCubeWriter::FindBindPoseMatrix(FbxNode* startNode, FbxNode *node, FbxMatrix &outmtx)
{
	FbxNode* lChildNode;
	
	for(int i = 0; i<startNode->GetChildCount(); i++)
	{
		lChildNode = startNode->GetChild(i);
		FbxMesh *pMesh = lChildNode->GetMesh();
		if (pMesh) {
			int lDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
			if(lDeformerCount > 1)
				FBX_ASSERT_NOW("Unexpected number of skin greater than 1");
			
			FbxArray<FbxVector4> lPositions;
			for (int lIndexLink = 0; lIndexLink < ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetClusterCount(); lIndexLink++)
			{
				FbxSkin *skin = (FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin);
				FbxCluster* lLink = skin->GetCluster(lIndexLink);
				if (!lLink || !lLink->GetLink()) continue;
				
				if (strcmp(lLink->GetLink()->GetNameWithoutNameSpacePrefix().Buffer(), node->GetNameOnly().Buffer()) != 0) {
					continue;
				}
				
				FbxAMatrix lTransformLink;
				lLink->GetTransformLinkMatrix(lTransformLink);
				FbxAMatrix lTransformLinkInverse = lTransformLink.Inverse();
				// We have to convert the FbxAMatrix to a FbxMatrix or else we lose information when transposing.
				for (int ii = 0; ii < 4; ii++)
					for (int jj = 0; jj < 4; jj++)
						outmtx.mData[ii][jj] = lTransformLinkInverse.mData[ii][jj];
				outmtx = outmtx.Transpose();
				
				// debug print
				FbxString lStrMatrix = matrixToString(lTransformLinkInverse);
				FBXSDK_printf("ibp:\n%s", lStrMatrix.Buffer());
				return true;
			}
		}
	}
	
	int lNodeChildCount = startNode->GetChildCount ();
	while (lNodeChildCount > 0)
	{
		
		lNodeChildCount--;
		lChildNode = startNode->GetChild(lNodeChildCount);
		if (FindBindPoseMatrix(lChildNode, node, outmtx)) {
			return true;
		}
	}
	
	return false;
}


// Pre-process the scene before write it out
bool GCubeWriter::PreprocessScene(FbxScene& /*pScene*/)
{
	return true;
}

// Post-process the scene after write it out
bool GCubeWriter::PostprocessScene(FbxScene& /*pScene*/)
{
	return true;
}

