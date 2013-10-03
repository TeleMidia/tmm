/*
 * MCCI.cpp
 *
 *  Created on: 25/09/2013
 *      Author: Felippe Nagato
 */

#include "tsparser/MCCI.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
namespace isdbt {

MCCI::MCCI() {
	stream = new char[20];
	currentCI = NULL;
	nextCI = NULL;
	ACDataEffectivePosition = 0x01;
	initializationTimingIndicator = 0x0F;
	systemIdentifier = 0x00;
	countDownIndex = 0x0F;
	switchOnControlFlag = false;
	phaseCorrection = 0x07;
}

MCCI::~MCCI() {
	if (stream) delete stream;
	if (currentCI) delete currentCI;
	if (nextCI) delete nextCI;
}

void MCCI::setTMCCSynchronizationWord(unsigned char sw) {
	TMCCSynchronizationWord = sw;
}

void MCCI::setACDataEffectivePosition(unsigned char position) {
	ACDataEffectivePosition = position;
}

void MCCI::setInitializationTimingIndicator(unsigned char indicator) {
	initializationTimingIndicator = indicator;
}

void MCCI::setCurrentMode(unsigned char mode) {
	currentMode = mode;
}

void MCCI::setCurrentGuardInterval(unsigned char interval) {
	currentGuardInterval = interval;
}

void MCCI::setNextMode(unsigned char mode) {
	nextMode = mode;
}

void MCCI::setNextGuardInterval(unsigned char interval) {
	nextGuardInterval = interval;
}

void MCCI::setSystemIdentifier(unsigned char identifier) {
	systemIdentifier = identifier;
}

void MCCI::setCountDownIndex(unsigned char index) {
	countDownIndex = index;
}

void MCCI::setSwitchOnControlFlag(unsigned char flag) {
	switchOnControlFlag = flag;
}

void MCCI::setCurrentCI(ConfigurationInformation* ci) {
	if (currentCI) delete currentCI;
	currentCI = ci;
}

void MCCI::setNextCI(ConfigurationInformation* ci) {
	if (nextCI) delete nextCI;
	nextCI = ci;
}

void MCCI::setPhaseCorrection(unsigned char phase) {
	phaseCorrection = phase;
}

unsigned char MCCI::getTMCCSynchronizationWord() {
	return TMCCSynchronizationWord;
}

unsigned char MCCI::getACDataEffectivePosition() {
	return ACDataEffectivePosition;
}

unsigned char MCCI::getInitializationTimingIndicator() {
	return initializationTimingIndicator;
}

unsigned char MCCI::getCurrentMode() {
	return currentMode;
}

unsigned char MCCI::getCurrentGuardInterval() {
	return currentGuardInterval;
}

unsigned char MCCI::getNextMode() {
	return nextMode;
}

unsigned char MCCI::getNextGuardInterval() {
	return nextGuardInterval;
}

unsigned char MCCI::getSystemIdentifier() {
	return systemIdentifier;
}

unsigned char MCCI::getCountDownIndex() {
	return countDownIndex;
}

bool MCCI::getSwitchOnControlFlag() {
	return switchOnControlFlag;
}

ConfigurationInformation* MCCI::getCurrentCI() {
	return currentCI;
}

ConfigurationInformation* MCCI::getNextCI() {
	return nextCI;
}

unsigned char MCCI::getPhaseCorrection() {
	return phaseCorrection;
}

unsigned int MCCI::getCrc32() {
	return crc32;
}

bool MCCI::copyCurrentToNext() {
	ConfigurationInformation* ci;

	nextMode = currentMode;
	nextGuardInterval = currentGuardInterval;
	if (currentCI) {
		ci = new ConfigurationInformation;
		ci->partialReceptionFlag = currentCI->partialReceptionFlag;
		ci->tpLayerA = NULL;
		ci->tpLayerB = NULL;
		ci->tpLayerC = NULL;
		if (currentCI->tpLayerA) {
			ci->tpLayerA = new TransmissionParameters;
			ci->tpLayerA->codingRateOfInnerCode = currentCI->tpLayerA->codingRateOfInnerCode;
			ci->tpLayerA->lengthOfTimeInterleaving = currentCI->tpLayerA->lengthOfTimeInterleaving;
			ci->tpLayerA->modulationScheme = currentCI->tpLayerA->modulationScheme;
			ci->tpLayerA->numberOfSegments = currentCI->tpLayerA->numberOfSegments;
		}
		if (currentCI->tpLayerB) {
			ci->tpLayerB = new TransmissionParameters;
			ci->tpLayerB->codingRateOfInnerCode = currentCI->tpLayerB->codingRateOfInnerCode;
			ci->tpLayerB->lengthOfTimeInterleaving = currentCI->tpLayerB->lengthOfTimeInterleaving;
			ci->tpLayerB->modulationScheme = currentCI->tpLayerB->modulationScheme;
			ci->tpLayerB->numberOfSegments = currentCI->tpLayerB->numberOfSegments;
		}
		if (currentCI->tpLayerC) {
			ci->tpLayerC = new TransmissionParameters;
			ci->tpLayerC->codingRateOfInnerCode = currentCI->tpLayerC->codingRateOfInnerCode;
			ci->tpLayerC->lengthOfTimeInterleaving = currentCI->tpLayerC->lengthOfTimeInterleaving;
			ci->tpLayerC->modulationScheme = currentCI->tpLayerC->modulationScheme;
			ci->tpLayerC->numberOfSegments = currentCI->tpLayerC->numberOfSegments;
		}
		nextCI = ci;
		return true;
	}

	return false;
}

int MCCI::updateStream() {
	ConfigurationInformation* ci;
	int pos = 0;

	stream[pos] = 0x30;
	stream[pos] = stream[pos] | ((TMCCSynchronizationWord << 7) & 0x80);
	stream[pos] = stream[pos] | ((ACDataEffectivePosition << 6) & 0x40);
	stream[pos] = stream[pos] | (initializationTimingIndicator & 0x0F);

	pos++;

	stream[pos] = (currentMode << 6) & 0xC0;
	stream[pos] = stream[pos] | ((currentGuardInterval << 4) & 0x30);
	stream[pos] = stream[pos] | ((nextMode << 2) & 0x0C);
	stream[pos] = stream[pos] | (nextGuardInterval & 0x03);

	pos++;

	stream[pos] = (systemIdentifier << 6) & 0xC0;
	stream[pos] = stream[pos] | ((countDownIndex << 2) & 0x3C);
	stream[pos] = stream[pos] | ((switchOnControlFlag << 1) & 0x02);

	ci = currentCI;
	for (int i = 0; i < 2; i++) {
		if (ci) {
			stream[pos] = stream[pos] | (ci->partialReceptionFlag & 0x01);

			pos++;

			if (ci->tpLayerA) {
				stream[pos] = ((ci->tpLayerA->modulationScheme << 5) & 0xE0);
				stream[pos] = stream[pos] | ((ci->tpLayerA->codingRateOfInnerCode << 2) & 0x1C);
				stream[pos] = stream[pos] | ((ci->tpLayerA->lengthOfTimeInterleaving >> 1) & 0x03);

				pos++;

				stream[pos] = ((ci->tpLayerA->lengthOfTimeInterleaving << 7) & 0x80);
				stream[pos] = stream[pos] | ((ci->tpLayerA->numberOfSegments << 3) & 0x78);
			} else {
				stream[pos++] = 0xFF;
				stream[pos] = 0xF8;
			}

			if (ci->tpLayerB) {
				stream[pos] = stream[pos] | (ci->tpLayerB->modulationScheme & 0x07);

				pos++;

				stream[pos] = (ci->tpLayerB->codingRateOfInnerCode << 5) & 0xE0;
				stream[pos] = stream[pos] | ((ci->tpLayerB->lengthOfTimeInterleaving << 2) & 0x1C);
				stream[pos] = stream[pos] | ((ci->tpLayerB->numberOfSegments >> 2) & 0x03);

				pos++;

				stream[pos] = (ci->tpLayerB->numberOfSegments << 6) & 0xC0;
			} else {
				stream[pos] = stream[pos] | 0x07;
				pos++;
				stream[pos++] = 0xFF;
				stream[pos] = 0xC0;
			}

			if (ci->tpLayerC) {
				stream[pos] = stream[pos] | ((ci->tpLayerC->modulationScheme << 3) & 0x38);
				stream[pos] = stream[pos] | (ci->tpLayerC->codingRateOfInnerCode & 0x03);

				pos++;

				stream[pos] = (ci->tpLayerC->lengthOfTimeInterleaving << 5) & 0xE0;
				stream[pos] = stream[pos] | ((ci->tpLayerC->numberOfSegments << 1) & 0x1E);
			} else {
				stream[pos] = stream[pos] | 0x3F;

				pos++;

				stream[pos] = 0xFE;
			}
		} else {
			stream[pos] = stream[pos] | 0x01;
			pos++;
			memset(stream + pos, 0xFF, 4);
			pos += 4;
			stream[pos] = 0xFE;
		}
		ci = nextCI;
	}

	stream[pos] = stream[pos] | ((phaseCorrection >> 2) & 0x01);

	pos++;

	stream[pos] = (phaseCorrection << 6) & 0xC0;
	stream[pos] = stream[pos] | 0x3F;

	pos++;

	memset(stream + pos, 0xFF, 2);
	pos += 2;

	Crc32 crc;
	crc32 = crc.crc(stream, pos);
	stream[pos++] = (crc32 >> 24) & 0xFF;
	stream[pos++] = (crc32 >> 16) & 0xFF;
	stream[pos++] = (crc32 >> 8) & 0xFF;
	stream[pos++] = crc32 & 0xFF;

	return pos;
}

int MCCI::getStream(char** dataStream) {
	if (dataStream != NULL) {
		*dataStream = stream;
		return 20;
	}
	return 0;
}


}
}
}
}
}


