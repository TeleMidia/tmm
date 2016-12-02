/******************************************************************************
TeleMídia Multiplexer
Copyright (C) 2016 TeleMídia Lab/PUC-Rio
https://github.com/TeleMidia/tmm-mpeg2ts/graphs/contributors

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version. This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
for more details. You should have received a copy of the GNU Affero General
Public License along with this program. If not, see http://www.gnu.org/licenses/.

*******************************************************************************/

/*
 * RawStream.cpp
 *
 *  Created on: 26/09/2013
 *      Author: Felippe Nagato
 */

#include "RawStream.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

RawStream::RawStream() {
	type = 1; //Section type
	currPos = 0;
	maxBitrate = 30000;
	destroyBlocks = true;
}

RawStream::~RawStream() {
	releaseBlockList();
}

void RawStream::releaseBlockList() {
	vector<pair<char*,int>*>::iterator it;

	it = blockList.begin();
	while (it != blockList.end()) {
		if (*it) {
			if (destroyBlocks) {
				delete (*it)->first;
			}
			delete (*it);
		}
		++it;
	}
	blockList.clear();
	currPos = 0;
}

void RawStream::setDestroyBlocks(bool destroy) {
	destroyBlocks = destroy;
}

bool RawStream::getBuffer(Buffer** buffer) {
	if (bufferList.empty() && !subscriberList.empty()) {
		notify(currStc, &blockList);
		maxBufferLength = blockList.size();
		fillBuffer();
		releaseBlockList();
	}
	return Stream::getBuffer(buffer);
}

void RawStream::fillBuffer() {
	vector<pair<char*,int>*>::iterator it;
	char* stream;
	Buffer* buf;
	int len;

	if (blockList.empty()) return;

	while (bufferList.size() < maxBufferLength) {
		if (currPos < blockList.size()) {
			stream = blockList.at(currPos)->first;
			len = blockList.at(currPos)->second;
			buf = new Buffer();
			buf->pos = 0;
			buf->buffer = new char[len];
			memcpy(buf->buffer, stream, len);
			buf->size = len;
			bufferList.push_back(buf);
			currPos++;
		} else {
			currPos = 0;
		}
	}
}

bool RawStream::addBlock(char* stream, int length) {
	if (stream && length) {
		char* myStream = new char[length];
		pair<char*,int>* myp = new pair<char*,int>;
		memcpy(myStream, stream, length);
		myp->first = myStream;
		myp->second = length;
		blockList.push_back(myp);
		return true;
	}
	return false;
}

void RawStream::addSection(PrivateSection* sec) {
	char* myStream;
	char* stream;
	int length;
	pair<char*,int>* myp = new pair<char*,int>;

	length = sec->getStream(&stream);
	myStream = new char[length];
	memcpy(myStream, stream, length);
	myp->first = myStream;
	myp->second = length;
	blockList.push_back(myp);
}

bool RawStream::addSection(const string& filename) {
	FILE * pFile;
	size_t size, result, secPos = 0;
	unsigned short secLen;
	char* buffer;
	bool success = false;

	pFile = fopen(filename.c_str(), "rb");
	if (pFile == NULL ) {
		return success;
	}
	fseek(pFile, 0, SEEK_END);
	size = ftell(pFile);
	buffer = new char[size];
	fseek(pFile, 0, SEEK_SET);
	result = fread(buffer, 1, size, pFile);
	if (result == size) {
		while (secPos < size) {
			secLen = (((buffer[secPos + 1] & 0x0F) << 8) | (buffer[secPos + 2] & 0xFF));
			secLen = secLen + 3;
			addBlock(buffer + secPos, secLen);
			secPos += secLen;
		}
	}
	delete buffer;
	fclose(pFile);

	return success;
}

}
}
}
}



