/*
 * SectionStream.cpp
 *
 *  Created on: 25/03/2013
 *      Author: Felippe Nagato
 */

#include "SectionStream.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

SectionStream::SectionStream() {
	type = 1;
	currPos = 0;
	maxBitrate = 30000;
}

SectionStream::~SectionStream() {
	releaseSectionList();
}

void SectionStream::releaseSectionList() {
	vector<PrivateSection*>::iterator it;
	it = sectionList.begin();
	while (it != sectionList.end()) {
		if (*it) delete (*it);
		++it;
	}
	sectionList.clear();
	currPos = 0;
}

bool SectionStream::getBuffer(Buffer** buffer) {
	if (bufferList.empty() && !subscriberList.empty()) {
		notify(currStc, &sectionList);
		maxBufferLength = sectionList.size();
		fillBuffer();
		releaseSectionList();
	}
	return Stream::getBuffer(buffer);
}

void SectionStream::fillBuffer() {
	vector<PrivateSection*>::iterator it;
	char* stream;
	Buffer* buf;
	int len;

	if (sectionList.empty()) return;

	while (bufferList.size() < maxBufferLength) {
		if (currPos < sectionList.size()) {
			len = sectionList[currPos]->getStream(&stream);
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

void SectionStream::addSection(PrivateSection* sec) {
	sectionList.push_back(sec);
}

bool SectionStream::addSection(char* stream, int length) {
	PrivateSection* sec;
	sec = new PrivateSection();
	sec->addData(stream, length);
	if (sec->isConsolidated()) {
		sectionList.push_back(sec);
		return true;
	}
	return false;
}

bool SectionStream::addSection(string filename) {
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
			addSection(buffer + secPos, secLen);
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


