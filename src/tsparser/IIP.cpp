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
	currentSize = 0;
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

int IIP::addData(char* data, unsigned short length) {

	if (currentSize + length > 184) return -1;

	memcpy(stream + currentSize, data, length);

	currentSize = currentSize + length;

	if (currentSize >= 25) {
		IIPPacketPointer = (((stream[0] & 0x1F) << 8) | (stream[1] & 0xFF));
		if (mcci) delete mcci;
		mcci = new MCCI(stream + 2);
		IIPBranchNumber = stream[22] & 0xFF;
		lastIIPBranchNumber = stream[23] & 0xFF;
		nsiLength = stream[24] & 0xFF;

		//TODO: Network Synchronization Information

		return currentSize;
	}

	return currentSize;
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


