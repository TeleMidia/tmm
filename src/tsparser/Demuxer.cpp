/*
 * Demuxer.cpp
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#include "tsparser/Demuxer.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

Demuxer::Demuxer() {
	tsReader = NULL;
	pidFilterList = new set<unsigned short>;
}

Demuxer::Demuxer(TSFileReader* tsReader) {
	this->tsReader = tsReader;
	pidFilterList = new set<unsigned short>;
}

Demuxer::~Demuxer() {
	delete pidFilterList;
}

void Demuxer::setTsReader(TSFileReader* tsReader) {
	this->tsReader = tsReader;
}

void Demuxer::addPidFilter(unsigned short pid) {
	pidFilterList->insert(pid);
}

void Demuxer::removePidFilter(unsigned short pid) {
	pidFilterList->erase(pid);
}

int Demuxer::getNextPacketbyFilter(TSPacket **packet) {
	char* buffer;
	unsigned short pid;

	if (!tsReader) return -1;

	if (tsReader->getBuffer(&buffer) > 0) {
		pid = (((buffer[1] & 0x1F) << 8) | (buffer[2] & 0xFF));
		if (pidFilterList->count(pid)) {
			*packet = new TSPacket(buffer);
			return 0;
		}
	}
	*packet = NULL;
	return -1;
}

void Demuxer::setContinuityCounter(unsigned short pid, char cc) {
	continuityCounterList[pid] = cc;
}

char Demuxer::getContinuityCounter(unsigned short pid) {
	if (continuityCounterList.count(pid)) {
		return continuityCounterList[pid];
	} else {
		return -1;
	}
}

}
}
}
}


