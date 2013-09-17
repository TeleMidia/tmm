/*
 * ISDBTInformation.cpp
 *
 *  Created on: 16/09/2013
 *      Author: Felippe Nagato
 */

#include "tsparser/ISDBTInformation.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
namespace isdbt {

ISDBTInformation::ISDBTInformation() {
	TMCCIdentifier = TERRESTRIAL_DIGITAL_TV;
	stream = new char[8];
	ACData = NULL;
	bufferResetControlFlag = false;
	switchOnControlFlagForEmergencyBroadcasting = false;
	initializationTimingHeadPacketFlag = false;
	frameHeadPacketFlag = false;
	frameIndicator = false;
	ACDataInvalidFlag = true;
	countDownIndex = 0x0F; //?
	ACDataLength = 0;
	TSPCounter = 0;
}

ISDBTInformation::~ISDBTInformation() {
	if (stream) delete stream;
	if (ACData) delete ACData;
}

void ISDBTInformation::setTMCCIdentifier(unsigned char id) {
	TMCCIdentifier = id;
}

unsigned char ISDBTInformation::getTMCCIdentifier() {
	return TMCCIdentifier;
}

void ISDBTInformation::setBufferResetControlFlag(bool flag) {
	bufferResetControlFlag = flag;
}

bool ISDBTInformation::getBufferResetControlFlag() {
	return bufferResetControlFlag;
}

void ISDBTInformation::setSwitchOnControlFlagForEmergencyBroadcasting(bool flag) {
	switchOnControlFlagForEmergencyBroadcasting = flag;
}

bool ISDBTInformation::getSwitchOnControlFlagForEmergencyBroadcasting() {
	return switchOnControlFlagForEmergencyBroadcasting;
}

void ISDBTInformation::setInitializationTimingHeadPacketFlag(bool flag) {
	initializationTimingHeadPacketFlag = flag;
}

bool ISDBTInformation::getInitializationTimingHeadPacketFlag() {
	return initializationTimingHeadPacketFlag;
}

void ISDBTInformation::setFrameHeadPacketFlag(bool flag) {
	frameHeadPacketFlag = flag;
}

bool ISDBTInformation::getFrameHeadPacketFlag() {
	return frameHeadPacketFlag;
}

void ISDBTInformation::setFrameIndicator(bool indicator) {
	frameIndicator = indicator;
}

bool ISDBTInformation::getFrameIndicator() {
	return frameIndicator;
}

void ISDBTInformation::setLayerIndicator(unsigned char layer) {
	layerIndicator = layer;
}

unsigned char ISDBTInformation::getLayerIndicator() {
	return layerIndicator;
}

void ISDBTInformation::setCountDownIndex(unsigned char cindex) {
	countDownIndex = cindex;
}

unsigned char ISDBTInformation::getCountDownIndex() {
	return countDownIndex;
}

void ISDBTInformation::setACDataInvalidFlag(bool flag) {
	ACDataInvalidFlag = flag;
}

bool ISDBTInformation::getACDataInvalidFlag() {
	return ACDataInvalidFlag;
}

void ISDBTInformation::setACDataEffectiveBytes(unsigned char acDataEB) {
	ACDataEffectiveBytes = acDataEB;
}

unsigned char ISDBTInformation::getACDataEffectiveBytes() {
	return ACDataEffectiveBytes;
}

unsigned short ISDBTInformation::incrementTSPCounter() {
	return ++TSPCounter;
}

void ISDBTInformation::setTSPCounter(unsigned char counter) {
	TSPCounter = counter;
}

unsigned short ISDBTInformation::getTSPCounter() {
	return (TSPCounter & 0x1FFF);
}

void ISDBTInformation::setACData(char* acData, unsigned short length) {
	if (ACData) delete ACData;
	ACData = new char[length];
	memcpy(ACData, ACData, length);
	ACDataLength = length;
}

unsigned short ISDBTInformation::getACData(char** acData) {
	if (ACData) {
		*acData = ACData;
		return ACDataLength;
	}
	return 0;
}

unsigned short ISDBTInformation::getACDataLength() {
	return ACDataLength;
}

int ISDBTInformation::updateStream() {
	int pos = 0;

	stream[pos] = 0x20;
	stream[pos] = stream[pos] | ((TMCCIdentifier << 6) & 0xC0);
	stream[pos] = stream[pos] | ((bufferResetControlFlag << 4) & 0x10);
	stream[pos] = stream[pos] | ((switchOnControlFlagForEmergencyBroadcasting << 3) & 0x08);
	stream[pos] = stream[pos] | ((initializationTimingHeadPacketFlag << 2) & 0x04);
	stream[pos] = stream[pos] | ((frameHeadPacketFlag << 1) & 0x02);
	stream[pos] = stream[pos] | frameIndicator;

	pos++;

	stream[pos] = ((layerIndicator << 4) & 0xF0);
	stream[pos] = stream[pos] | (countDownIndex & 0x0F);

	pos++;

	stream[pos] = ((ACDataInvalidFlag << 7) & 0x80);
	stream[pos] = stream[pos] | ((ACDataEffectiveBytes << 5) & 0x60);
	stream[pos] = stream[pos] | ((TSPCounter >> 8) & 0x1F);

	pos++;

	stream[pos] = (TSPCounter & 0xFF);

	pos++;

	if (ACDataInvalidFlag) {
		memset(stream+pos, 0xFF, 4);
	} else {
		if (ACData) {
			memcpy(stream+pos, ACData, 4);
		}
	}

	pos += 4;

	return pos;
}

int ISDBTInformation::getStream(char** dataStream) {
	if (dataStream != NULL) *dataStream = stream;
	return 8;
}


}
}
}
}
}


