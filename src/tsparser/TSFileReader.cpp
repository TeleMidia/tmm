/*
 * TSFileReader.cpp
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#include "tsparser/TSFileReader.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

TSFileReader::TSFileReader() {
	pFile = NULL;
	buffer = new char[BUFFER_SIZE];
	packetSize = 188;
	loopCount = 0;
	bufferPos = 0;
	bufferLength = 0;
}

TSFileReader::TSFileReader(string filename) {
	pFile = NULL;
	buffer = new char[BUFFER_SIZE];
	packetSize = 188;
	loopCount = 0;
	bufferPos = 0;
	bufferLength = 0;
	this->filename = filename;
}

TSFileReader::~TSFileReader() {
	close();
	delete buffer;
}

void TSFileReader::setFilename(string filename) {
	this->filename = filename;
	close();
}

int TSFileReader::open() {
	packetSize = readPacketSize(filename);
	if (packetSize < 0) return -1;
	pFile = fopen64(filename.c_str(), "rb");
	if (pFile == NULL ) {
		cout << "TSFileReader::open - Unable to open file: " << filename << endl;
		return -2;
	}
	fseeko64(pFile, 0, SEEK_END);
	length = ftello64(pFile);
	fseeko64(pFile, 0, SEEK_SET);
	return read();
}

int TSFileReader::close() {
	if (pFile) fclose(pFile);
	pFile = NULL;
	loopCount = 0;
	return 0;
}

int TSFileReader::getLoopCount() {
	return loopCount;
}

int64_t TSFileReader::getLength() {
	return length;
}

void TSFileReader::resetLoopCount() {
	loopCount = 0;
}

int TSFileReader::goTo(int64_t position) {
	int ret = fseeko64(pFile, position, SEEK_SET);
	bufferPos = 0;
	bufferLength = 0;
	read();
	return ret;
}

unsigned char TSFileReader::getPacketSize() {
	return packetSize;
}

int TSFileReader::searchSyncByte() {
	int counter = 0;
	while (1) {
		if (buffer[bufferPos] == 0x47) {
			return counter;
		}
		bufferPos++;
		counter++;
		if (bufferPos == bufferLength) {
			if (read() < 0) return -1;
		}
	}
	return -2;
}

int TSFileReader::readPacketSize(string filename) {
	//weak method
	FILE* fd;
	char buf[2041];
	int i = 0, rb;

	fd = fopen(filename.c_str(), "rb");
	if (fd == NULL) return -1;

	rb = fread(buf, 1, 2041, fd);
	fclose(fd);
	if (rb == 0) return -2;

	while (i < 1837) {
		if (buf[i] == 0x47) {
			if (buf[i+188] == 0x47) {
				return 188;
			} else if (buf[i+204] == 0x47) {
				return 204;
			}
		}
		++i;
	}

	return -1;
}

int TSFileReader::getBuffer(char** buffer) {
	if (!pFile) {
		if (open() < 0) return -1;
	}
	while (bufferPos + packetSize > bufferLength) {
		if (read() < packetSize) {
			*buffer = NULL;
			return 0;
		}
		if (this->buffer[bufferPos] != 0x47) {
			if (searchSyncByte() < 0) return -2;
		}
	}

	*buffer = this->buffer + bufferPos;
	bufferPos += packetSize;
	return packetSize;
}

int TSFileReader::read() {
	int pos = 0;
	if (!pFile) {
		cout << "TSFileReader::read - File descriptor is NULL." << endl;
		return -1;
	}

	if (feof(pFile)) {
		fseeko64(pFile, 0, SEEK_SET);
		loopCount++;
	}

	if (bufferPos < bufferLength) {
		pos = bufferLength - bufferPos;
		memcpy(buffer, buffer + bufferPos, pos);
	}
	bufferLength = fread(buffer + pos, 1, BUFFER_SIZE - pos, pFile) + pos;
	bufferPos = 0;
	return bufferLength;
}

bool TSFileReader::rewind() {
	if (!pFile) {
		if (open() < 0) return false;
	}
	fseeko64(pFile, 0, SEEK_SET);
	bufferPos = 0;
	bufferLength = 0;
	read();
	return true;
}

}
}
}
}


