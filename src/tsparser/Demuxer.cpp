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

	if (!tsReader) {
		cout << "Demuxer::getNextPacketbyFilter - tsReader is not available." << endl;
		return -1;
	}

	if (tsReader->getBuffer(&buffer) > 0) {
		pid = (((buffer[1] & 0x1F) << 8) | (buffer[2] & 0xFF));
		if (pidFilterList->count(pid)) {
			*packet = new TSPacket(buffer);
			return 1;
		}
	}
	*packet = NULL;
	return 0;
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


