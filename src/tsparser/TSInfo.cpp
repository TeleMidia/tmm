/*
 * TSInfo.cpp
 *
 *  Created on: 12/03/2013
 *      Author: Felippe Nagato
 */

#include "tsparser/TSInfo.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

TSInfo::TSInfo() {
	pat = NULL;
	tsReader = NULL;
	firstPts = -1;
}

TSInfo::TSInfo(TSFileReader* tsReader) {
	pat = NULL;
	this->tsReader = tsReader;
	firstPts = -1;
}

TSInfo::~TSInfo() {
	if (pat) delete pat;
	clearPmtList();
}

void TSInfo::clearPmtList() {
	map<unsigned short,Pmt*>::iterator it;

	it = pmtList.begin();
	while (it != pmtList.end()) {
		delete it->second;
		++it;
	}
	pmtList.clear();
}

void TSInfo::setTSFileReader(TSFileReader* tsReader) {
	this->tsReader = tsReader;
	if (pat) delete pat;
	pat = NULL;
	clearPmtList();
}

unsigned char TSInfo::getStreamType(unsigned short pid) {
	map<unsigned short,Pmt*>::iterator it;
	map<unsigned short, unsigned char>::iterator itEs;
	it = pmtList.begin();
	while (it != pmtList.end()) {
		itEs = it->second->getEsList()->find(pid);
		if (itEs != it->second->getEsList()->end()) {
			return itEs->second;
		}
		++it;
	}
	return 255;
}

bool TSInfo::readInfo() {
	unsigned short pid;
	unsigned char len;
	char* payload;
	Pmt* pmt;
	map<unsigned short, Pmt*>::iterator pmtIt;
	TSPacket *packet;
	char* buffer;

	if (!tsReader) return false;

	while (!tsReader->getLoopCount()) {
		if (tsReader->getBuffer(&buffer) > 0) {
			packet = new TSPacket(buffer);
		} else {
			return true;
		}

		pid = packet->getPid();

		if (pid == 0x00) {
			packet->process();
			if (pat) {
				if (pat->isConsolidated()) {
					continue;
				} else {
					len = packet->getPayload(&payload);
					pat->addData(payload, len);
				}
			} else if (packet->getStartIndicator() &&
					  (packet->getAdaptationFieldControl() == 1 ||
					   packet->getAdaptationFieldControl() == 3)) {
				len = packet->getPayload(&payload);
				pat = new Pat();
				pat->addData(payload, len);
			}
		} else if (pat && pat->isConsolidated()) {
			if (pat->getPmtList()->count(pid)) {
				packet->process();
				pmtIt = pmtList.find(pid);
				if (pmtIt != pmtList.end()) {
					pmt = pmtList[pid];
					if (pmt->isConsolidated()) {
						continue;
					} else {
						len = packet->getPayload(&payload);
						pmt->addData(payload, len);
					}
				} else if (packet->getStartIndicator() &&
						  (packet->getAdaptationFieldControl() == 1 ||
						   packet->getAdaptationFieldControl() == 3)) {
					len = packet->getPayload(&payload);
					pmt = new Pmt();
					pmt->addData(payload, len);
					pmtList[pid] = pmt;
				}
				if (checkPmtsCount()) break;
			}
		}
		delete packet;
	}
	return true;
}

bool TSInfo::checkPmtsCount() {
	if (pat->getPmtList()->size() == pmtList.size()) {
		map<unsigned short,Pmt*>::iterator it;
		it = pmtList.begin();
		while (it != pmtList.end()) {
			if (!it->second->isConsolidated()) {
				return false;
			}
			++it;
		}
		return true;
	}
	return false;
}

double TSInfo::duration(unsigned short pid) {
	TSPacket* tsPacket = NULL;
	PESPacket* pesPacket = NULL;
	char* payload;
	int len, lc;
	Demuxer demuxer;
	int64_t pts1 = -1, pts2 = -1;

	if (!tsReader) {
		return -1;
	} else {
		demuxer.setTsReader(tsReader);
	}
	demuxer.addPidFilter(pid);

	if (!tsReader->rewind()) {
		return -2;
	}

	for (int i = 0; i < 2; i++) {
		lc = tsReader->getLoopCount();
		if (i == 1) {
			int64_t temp = tsReader->getLength() - (6000 * tsReader->getPacketSize());
			if (temp < 0) temp = 0;
			tsReader->goTo(temp);
		}
		while (tsReader->getLoopCount() == lc) {
			if (demuxer.getNextPacketbyFilter(&tsPacket) >= 0) {
				if ((tsPacket->getAdaptationFieldControl() == 1) ||
					(tsPacket->getAdaptationFieldControl() == 3)) {
					tsPacket->process();
					if (tsPacket->getStartIndicator()) {
						if (tsPacket->getNumberOfPayloads() > 1) {
							//add last data and finish packet
							len = tsPacket->getPayload2(&payload);
							pesPacket->process(payload, len);
							if (pesPacket->getPtsDtsFlags() >= 2){
								if (pts1 == -1) {
									pts1 = pesPacket->getPts();
									break;
								} else {
									pts2 = pesPacket->getPts();
								}
							}
							delete pesPacket;
							pesPacket = NULL;
						}
						if (pesPacket) {
							//finish packet
							if (pesPacket->getPtsDtsFlags() >= 2) {
								if (pts1 == -1) {
									pts1 = pesPacket->getPts();
									firstPts = pts1;
									break;
								} else {
									pts2 = pesPacket->getPts();
								}
							}
							delete pesPacket;
							pesPacket = NULL;
						}
						//check if is a PES start and add create packet
						len = tsPacket->getPayload(&payload);
						pesPacket = new PESPacket(payload, len);
						if (pesPacket->getStartCodePrefix() != 0x01) {
							delete pesPacket;
							pesPacket = NULL;
						}
					} else {
						//add data
						if (pesPacket) {
							len = tsPacket->getPayload(&payload);
							pesPacket->process(payload, len);
						}
					}
				}
			}
			if (tsPacket) delete tsPacket;
		}
	}
	return Stc::baseToSecond(Stc::calcDiff90khz(pts2, pts1));
}

int64_t TSInfo::getFirstPts() {
	return firstPts;
}

/*hasDts() return values:
-1 = error
 0 = no DTS
 1 = with DTS
 */
char TSInfo::hasDts(unsigned short pid, unsigned int pktCount) {
	TSPacket* tsPacket = NULL;
	PESPacket* pesPacket = NULL;
	char* payload;
	int len;
	unsigned int pktNum = 0;
	Demuxer demuxer;
	unsigned char flag = 0;

	if (!tsReader) {
		return -1;
	} else {
		demuxer.setTsReader(tsReader);
	}
	demuxer.addPidFilter(pid);

	if (!tsReader->rewind()) {
		return -2;
	}

	while (pktNum < pktCount) {
		if (demuxer.getNextPacketbyFilter(&tsPacket) >= 0) {
			if ((tsPacket->getAdaptationFieldControl() == 1) ||
				(tsPacket->getAdaptationFieldControl() == 3)) {
				tsPacket->process();
				if (tsPacket->getStartIndicator()) {
					if (tsPacket->getNumberOfPayloads() > 1) {
						//add last data and finish packet
						len = tsPacket->getPayload2(&payload);
						pesPacket->process(payload, len);
						flag = pesPacket->getPtsDtsFlags();
						pktNum++;
						delete pesPacket;
						pesPacket = NULL;
						if (flag == 3) {
							return 1;
						}
					}
					if (pesPacket) {
						//finish packet
						flag = pesPacket->getPtsDtsFlags();
						pktNum++;
						delete pesPacket;
						pesPacket = NULL;
						if (flag == 3) {
							return 1;
						}
					}
					//check if is a PES start and add create packet
					len = tsPacket->getPayload(&payload);
					pesPacket = new PESPacket(payload, len);
					if (pesPacket->getStartCodePrefix() != 0x01) {
						delete pesPacket;
						pesPacket = NULL;
					}
				} else {
					//add data
					if (pesPacket) {
						len = tsPacket->getPayload(&payload);
						pesPacket->process(payload, len);
					}
				}
			}
		}
		if (tsPacket) delete tsPacket;
	}
	return 0;
}

bool TSInfo::isAudioStreamType(unsigned char st) {
	switch (st) {
		case 3: break;
		case 4: break;
		case 15: break;
		case 17: break;
		case 129: break;
		case 138: break;
		default: return false;
	}
	return true;
}

bool TSInfo::isVideoStreamType(unsigned char st) {
	switch (st) {
		case 1: break;
		case 2: break;
		case 16: break;
		case 27: break;
		default: return false;
	}
	return true;
}

void TSInfo::printTable() {
	Pmt* pmt;
	map<unsigned short, Pmt*>::iterator j;
	map<unsigned short, unsigned char>* esList;
	map<unsigned short, unsigned char>::iterator k;

	j = pmtList.begin();
	while (j != pmtList.end()) {
		pmt = j->second;
		if (pmt != NULL) {
			esList = pmt->getEsList();
			if (esList != NULL) {
				if (j->first != 0) {
					k = esList->begin();
					while (k != esList->end()) {
						if (isVideoStreamType(k->second)) {
							cout << k->first << " - ";
							cout << "Video" << endl;
						} else if (isAudioStreamType(k->second)) {
							cout << k->first << " - ";
							cout << "Audio" << endl;
						}
						++k;
					}
				}
			}
		}
		++j;
	}
}

}
}
}
}


