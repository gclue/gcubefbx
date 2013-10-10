//
//  BinaryOutputStream.cpp
//  fbxconv
//
//  Created by 小林 伸郎 on 2013/09/03.
//  Copyright (c) 2013年 小林 伸郎. All rights reserved.
//

#define kBigEndian

#include "BinaryOutputStream.h"

BinaryOutputStream::BinaryOutputStream()
{
	fout = FbxNew<FbxFile>();
}

BinaryOutputStream::~BinaryOutputStream()
{
	if (fout->IsOpen())
    {
        fout->Close();
    }
    FBX_SAFE_DELETE(fout);
}

bool BinaryOutputStream::open(const char *name)
{
	this->close();
	mFileName = FbxPathUtils::Clean(name);
    FbxPathUtils::Create(FbxPathUtils::GetFolderName(mFileName));
	return fout->Open(mFileName.Buffer(), FbxFile::eCreateWriteOnly, false);
}

void BinaryOutputStream::close()
{
	if (fout->IsOpen()) {
		fout->Close();
	}
}

void BinaryOutputStream::writeInt(int value)
{
#ifdef kBigEndian
	unsigned char a[4];
	unsigned char v[4];
	memcpy(a, &value, 4);
	v[0] = a[3];
	v[1] = a[2];
	v[2] = a[1];
	v[3] = a[0];
	fout->Write((char *) v, sizeof(unsigned char) * 4);
#else
	fout->Write(&value, sizeof(int));
#endif
}

void BinaryOutputStream::writeShort(short value)
{
#ifdef kBigEndian
	unsigned char a[2];
	unsigned char v[2];
	memcpy(a, &value, 2);
	v[0] = a[1];
	v[1] = a[0];
	fout->Write((char *) v, sizeof(unsigned char) * 2);
#else
	fout->Write(&value, sizeof(short));
#endif
}

void BinaryOutputStream::writeFloat(float value)
{
#ifdef kBigEndian
	unsigned char a[4];
	unsigned char v[4];
	memcpy(a, &value, 4);
	v[0] = a[3];
	v[1] = a[2];
	v[2] = a[1];
	v[3] = a[0];
	fout->Write((char *) v, sizeof(unsigned char) * 4);
#else
	fout->Write(&value, sizeof(float));
#endif
}

void BinaryOutputStream::writeByte(char value)
{
	fout->Write((char *) &value, sizeof(char));
}

void BinaryOutputStream::writeString(const char *str)
{
	fout->WriteFormat("%s", str);
}
