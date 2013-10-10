//
//  BinaryOutputStream.h
//  fbxconv
//
//  Created by 小林 伸郎 on 2013/09/03.
//  Copyright (c) 2013年 小林 伸郎. All rights reserved.
//

#ifndef __fbxconv__BinaryOutputStream__
#define __fbxconv__BinaryOutputStream__

#include "fbxsdk.h"

class BinaryOutputStream {
public:
	FbxFile *fout;
	FbxString	mFileName;
	
public:
	BinaryOutputStream();
	virtual ~BinaryOutputStream();
	
	bool open(const char *name);
	void close();
	
	void writeInt(int value);
	void writeShort(short value);
	void writeFloat(float value);
	void writeByte(char value);
	void writeString(const char *str);
};


#endif /* defined(__fbxconv__BinaryOutputStream__) */
