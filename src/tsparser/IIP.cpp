/*
 * IIP.cpp
 *
 *  Created on: 24/09/2013
 *      Author: Felippe Nagato
 */

#include "tsparser/IIP.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
namespace isdbt {

IIP::IIP() {
	stream = new char[184];
	mcci = NULL;
	IIPPacketPointer = 1;
	IIPBranchNumber = 0;
	lastIIPBranchNumber = 0;
	nsiLength = 0;
}

IIP::~IIP() {
	if (stream) delete stream;
	if (mcci) delete mcci;
}

void IIP::setIIPPacketPointer(unsigned short pointer) {
	IIPPacketPointer = pointer;
}

void IIP::setMcci(MCCI* mcci) {
	this->mcci = mcci;
}

void IIP::setIIPBranchNumber(unsigned char num) {
	IIPBranchNumber = num;
}

void IIP::setlastIIPBranchNumber(unsigned char last) {
	lastIIPBranchNumber = last;
}

unsigned short IIP::getIIPPacketPointer() {
	return IIPPacketPointer;
}

MCCI* IIP::getMcci() {
	return mcci;
}

unsigned char IIP::getIIPBranchNumber() {
	return IIPBranchNumber;
}

unsigned char IIP::getlastIIPBranchNumber() {
	return lastIIPBranchNumber;
}

unsigned char IIP::getNsiLength() {
	return nsiLength;
}

int IIP::updateStream() {
	char* tempStream;
	unsigned int tempStreamLength = 0;
	int pos = 0;

	stream[pos++] = (IIPPacketPointer >> 8) & 0xFF;
	stream[pos++] = IIPPacketPointer & 0xFF;

	if (mcci) {
		mcci->updateStream();
		tempStreamLength = mcci->getStream(&tempStream);
		memcpy(stream + pos, tempStream, tempStreamLength);
		pos += tempStreamLength;
	} else {
		cout << "IIP::updateStream - Error: No MCCI to encode." << endl;
		return -1;
	}
	stream[pos++] = IIPBranchNumber & 0xFF;
	stream[pos++] = lastIIPBranchNumber & 0xFF;
	stream[pos++] = nsiLength & 0xFF;

	//TODO: Network Synchronization Information

	memset(stream + pos, 0xFF, 159 - nsiLength);

	return 184;
}

int IIP::getStream(char** dataStream) {
	if (dataStream != NULL) {
		*dataStream = stream;
		return 184;
	}
	return 0;
}


}
}
}
}
}


