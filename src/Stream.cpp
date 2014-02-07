/*
 * Stream.cpp
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#include "Stream.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

Stream::Stream() {
	maxBufferLength = 10;
	nextSend = SYSTEM_CLOCK_FREQUENCY;
	period = SYSTEM_CLOCK_FREQUENCY;
	preponeTicks = 0;
	projectId = -1;
}

Stream::~Stream() {
	releaseBufferList();
}

void Stream::setProjectId(int id) {
	projectId = id;
}

int Stream::getProjectId() {
	return projectId;
}

void Stream::setCurrStc(int64_t stc) {
	currStc = stc;
}

bool Stream::getBuffer(Buffer** buffer) {
	if (bufferList.size()) {
		(*buffer)->buffer = bufferList[0]->buffer + bufferList[0]->pos;
		(*buffer)->pos = 0;
		if ((bufferList[0]->size - bufferList[0]->pos) > maxBytesRate) {
			(*buffer)->size = maxBytesRate;
		} else {
			(*buffer)->size = bufferList[0]->size - bufferList[0]->pos;
		}
		(*buffer)->startIndicator = (bufferList[0]->pos == 0);
		return true;
	}
	return false;
}

unsigned int Stream::getBufferSize() {
	return bufferList.size();
}

bool Stream::disposeBuffer() {
	if (bufferList.size()) {
		if ((bufferList[0]->size - bufferList[0]->pos) > maxBytesRate) {
			bufferList[0]->pos += maxBytesRate;
		} else {
			bufferList[0]->pos += (bufferList[0]->size - bufferList[0]->pos);
		}
		if (bufferList[0]->pos == bufferList[0]->size) {
			delete bufferList[0]->buffer;
			delete *(bufferList.begin());
			bufferList.erase(bufferList.begin());
			return true;
		}
	}
	return false;
}

void Stream::setMaxBufferLength(unsigned int max) {
	maxBufferLength = max;
}

unsigned int Stream::getMaxBufferLength() {
	return maxBufferLength;
}

void Stream::releaseBufferList() {
	vector<Buffer*>::iterator it;

	for (it = bufferList.begin(); it != bufferList.end(); it++) {
		if ((*it)->buffer) delete ((*it)->buffer);
		delete (*it);
	}
	bufferList.clear();
}

void Stream::initiateNextSend(int64_t stc) {
	nextSend = stc;
	maxBytesRate = maxBitrate / 8;
}

void Stream::updateNextSend(int64_t stc) {
	nextSend += period;
}

int64_t Stream::getNextSend() {
	return nextSend;
}

void Stream::setPeriod(unsigned int freq) {
	period = freq;
}

unsigned int Stream::getPeriod() {
	return period;
}

void Stream::setMaxBitrate(unsigned int rate) {
	maxBitrate = rate;
}

unsigned int Stream::getMaxBitrate() {
	return maxBitrate;
}

void Stream::setType(unsigned char type) {
	this->type = type;
}

unsigned char Stream::getType() {
	return type;
}

void Stream::setPreponeTicks(unsigned int ticks) {
	preponeTicks = ticks;
}

unsigned int Stream::getPreponeTicks() {
	return preponeTicks;
}


}
}
}
}


