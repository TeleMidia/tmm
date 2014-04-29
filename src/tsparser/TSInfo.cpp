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
	tsReader = NULL;
	init();
}

TSInfo::TSInfo(TSFileReader* tsReader) {
	this->tsReader = tsReader;
	init();
}

TSInfo::~TSInfo() {
	if (pat) delete pat;
	clearPmtList();
}

void TSInfo::init() {
	pat = NULL;
	tot = NULL;
	sdt = NULL;
	nit = NULL;
	iip = NULL;
	firstPts = -1;
	packetCounter = 0;
}

void TSInfo::clearPmtList() {
	map<unsigned short,PmtInfo*>::iterator it;

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
	map<unsigned short,PmtInfo*>::iterator it;
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

bool TSInfo::processPacket(TSPacket *packet, PrivateSection **section) {
	unsigned char len;
	char* payload;

	packet->process();
	if (*section) {
		if ((*section)->isConsolidated()) {
			return true;
		} else {
			len = packet->getPayload(&payload);
			(*section)->addData(payload, len);
		}
	} else if (packet->getStartIndicator() &&
			  (packet->getAdaptationFieldControl() == 1 ||
			   packet->getAdaptationFieldControl() == 3)) {
		len = packet->getPayload(&payload);
		switch (packet->getPid()) {
		case 0x0000:
			(*section) = new PatInfo();
			break;
		case 0x0010:
			(*section) = new NitInfo();
			break;
		case 0x0011:
			(*section) = new SdtInfo();
			break;
		case 0x0014:
			(*section) = new TotInfo();
			break;
		}
		(*section)->addData(payload, len);
	}
	return true;
}

bool TSInfo::readInfo(unsigned char mode) {
	unsigned short pid;
	unsigned char len;
	char* payload;
	PmtInfo* pmt;
	map<unsigned short, PmtInfo*>::iterator pmtIt;
	TSPacket *packet;
	char* buffer;
	bool pmtListOk = false;
	set<unsigned char>* layers;

	if (!tsReader) return false;

	while (!tsReader->getLoopCount()) {
		if (tsReader->getBuffer(&buffer) > 0) {
			packet = new TSPacket(buffer);
		} else {
			return true;
		}

		pid = packet->getPid();
		packetCounter++;

		if (tsReader->getPacketSize() == 204) {
			ISDBTInformation* isdbt;
			if (layerList.count(pid)) {
				layers = layerList[pid];
			} else {
				layers = new set<unsigned char>;
				layerList[pid] = layers;
			}
			isdbt = new ISDBTInformation(buffer + 188);
			layers->insert(isdbt->getLayerIndicator());
			delete isdbt;
		}

		if (pid == 0x00) {
			processPacket(packet, (PrivateSection**)&pat);
		} else if (pat && pat->isConsolidated()) {
			if ((!pmtListOk) && (pid != 0x0010) &&
					pat->getPmtList()->count(pid)) {
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
					pmt = new PmtInfo();
					pmt->addData(payload, len);
					pmt->setPid(pid);
					pmtList[pid] = pmt;
				}
				pmtListOk = checkPmtsCount();
			} else { //other PIDs
				switch (packet->getPid()) {
				case 0x0010:
					processPacket(packet, (PrivateSection**)&nit);
					break;
				case 0x0011:
					processPacket(packet, (PrivateSection**)&sdt);
					break;
				case 0x0014:
					processPacket(packet, (PrivateSection**)&tot);
					break;
				case 0x1FF0:
					if (!iip) {
						packet->process();
						if (packet->getStartIndicator() &&
							(packet->getAdaptationFieldControl() == 1 ||
							 packet->getAdaptationFieldControl() == 3)) {
							len = packet->getPayload(&payload);
							iip = new IIPInfo();
							iip->addData(payload, len);
						}
					}
					break;
				}
				if ((pmtListOk && (mode == 0)) ||
					(pmtListOk && nit && sdt && tot && iip) ||
					packetCounter >= 550000) break;
			}
		}
		delete packet;
	}
	return true;
}

bool TSInfo::checkPmtsCount() {
	unsigned int pmtCount;

	if (pat->getPmtList()->count(0x0010)) {
		pmtCount = pat->getPmtList()->size() - 1;
	} else {
		pmtCount = pat->getPmtList()->size();
	}

	if (pat->getPmtList()->size() == pmtCount) {
		map<unsigned short,PmtInfo*>::iterator it;
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
			if (demuxer.getNextPacketbyFilter(&tsPacket) > 0) {
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
		if (demuxer.getNextPacketbyFilter(&tsPacket) > 0) {
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

void TSInfo::printESTable() {
	PmtInfo* pmt;
	map<unsigned short, PmtInfo*>::iterator j;
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
						cout << k->first << ",";
						if (isVideoStreamType(k->second)) {
							cout << "Video";
						} else if (isAudioStreamType(k->second)) {
							cout << "Audio";
						} else {
							cout << "Data";
						}
						cout << "," <<
								pmt->getStreamTypeName(k->second) << endl;
						++k;
					}
				}
			}
		}
		++j;
	}
}

bool TSInfo::printTables() {
	map<unsigned short, PmtInfo*>::iterator it;

	if (pat) {
		cout << endl;
		pat->printTable();
	}

	it = pmtList.begin();
	while (it != pmtList.end()) {
		cout << endl;
		it->second->printTable();
		++it;
	}

	if (nit) {
		cout << endl;
		nit->printTable();
	}

	if (sdt) {
		cout << endl;
		sdt->printTable();
	}

	if (tot) {
		cout << endl;
		tot->printTable();
		cout << endl;
	}

	if (iip) {
		cout << endl;
		iip->printTable();
		cout << endl;
	}

	if (layerList.size()) {
		map<unsigned short, set<unsigned char>* >::iterator it;
		set<unsigned char>* layers;
		set<unsigned char>::iterator itLayer;

		cout << endl <<
			"TMCC - Transmission and Multiplexing Configuration Control"
			<< endl <<
			"----------------------------------------------------------"
			<< endl << endl;

		it = layerList.begin();
		while (it != layerList.end()) {
			layers = it->second;
			cout << "PID " << it->first << ": ";
			if (layers) {
				itLayer = layers->begin();
				while (itLayer != layers->end()) {
					switch (*itLayer) {
					case NULL_TSP:
						cout << "NULL_TSP";
						break;
					case HIERARCHY_A:
						cout << "A";
						break;
					case HIERARCHY_B:
						cout << "B";
						break;
					case HIERARCHY_C:
						cout << "C";
						break;
					case ACDATA_NO_HIERARCHY:
						cout << "ACDATA";
						break;
					case IIP_NO_HIERARCHY:
						cout << "IIP";
						break;
					}
					++itLayer;
					if (itLayer != layers->end()) cout << ", ";
				}
				cout << endl;
			} else {
				cout << "Not available." << endl;
			}
			++it;
		}
	}

	return true;
}

}
}
}
}


