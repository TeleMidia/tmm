/*
 * PESStream.cpp
 *
 *  Created on: 25/03/2013
 *      Author: Felippe Nagato
 */

#include "PESStream.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PESStream::PESStream() {
	type = 0;
	maxBitrate = 0xFFFFFFFF;

	demuxer = NULL;
	tsReader = NULL;
	isVideoStream = true;
	firstPts = -1;
	frameInterval = 0;
	timestamp1 = -1;
	timestamp2 = -1;
	hasDts = false;

	pesPacket = NULL;
	lastNextSendWithDts = -1;
	lastDtsAvailable = -1;

	inputRangeList = NULL;
	rangeChange = 0;
	lastRangeChange = 0;
	isInRangeNow = false;
}

PESStream::~PESStream() {
	if (demuxer) delete demuxer;
	if (tsReader) delete tsReader;
}

void PESStream::addBuffer() {
	char* stream;
	Buffer* buf;
	int len;

	len = pesPacket->getStream(&stream);
	buf = new Buffer();
	buf->pos = 0;
	buf->buffer = new char[len];
	memcpy(buf->buffer, stream, len);
	buf->size = len;

	bufferList.push_back(buf);
	rangeChangeList.push_back(rangeChange);
	if (pesPacket->getPtsDtsFlags() == 3) {
		ptsDtsDiff.push_back(Stc::calcDiff90khz(pesPacket->getPts(), pesPacket->getDts()));
		frequencyList.push_back(pesPacket->getDts());
		//cout << "DTS original = " << packet->getDts() << " - " << Stc::baseToSecond(packet->getDts()) << endl;
	} else {
		if (!isVideoStream) {
			//Audio stream
			ptsDtsDiff.push_back(0);
			if (pesPacket->getPtsDtsFlags() == 2) {
				frequencyList.push_back(pesPacket->getPts());
			} else {
				frequencyList.push_back(-1);
			}
		} else {
			//Video stream
			ptsDtsDiff.push_back(0);
			if (hasDts) {
				frequencyList.push_back(-1);
			} else {
				if (pesPacket->getPtsDtsFlags() == 2) {
					frequencyList.push_back(pesPacket->getPts());
				} else {
					frequencyList.push_back(-1);
				}
			}
		}
	}
}

//This function is only used to find the first PTS and standard frequency.
bool PESStream::timestampControl(PESPacket* packet) {
	if (timestamp2 != -1) return true;

	if (firstPts == -1) {
		if (packet->getPtsDtsFlags() > 1) firstPts = packet->getPts();
	}

	if (timestamp1 == -1) {
		if (isVideoStream) {
			if (packet->getPtsDtsFlags() == 3) {
				timestamp1 = packet->getDts();
				return true;
			} else if (packet->getPtsDtsFlags() == 2) {
				if (!hasDts) {
					timestamp1 = packet->getPts();
					return true;
				}
			}
		} else {
			if (packet->getPtsDtsFlags() > 1) {
				timestamp1 = packet->getPts();
				return true;
			}
		}
		return false;
	}

	if (timestamp2 == -1) {
		frameInterval++;
		if (isVideoStream) {
			if (packet->getPtsDtsFlags() == 3) {
				timestamp2 = packet->getDts();
			} else if (packet->getPtsDtsFlags() == 2) {
				if (!hasDts) {
					timestamp2 = packet->getPts();
					return true;
				}
			}
		} else {
			if (packet->getPtsDtsFlags() > 1) {
				timestamp2 = packet->getPts();
			}
		}
		if (timestamp2 != -1) {
			frequency = Stc::baseExtToStc((timestamp2 - timestamp1) / frameInterval, 0);
		}
		return true;
	}

	return false;
}

void PESStream::fillBuffer() {
	TSPacket* tsPacket = NULL;
	char* payload;
	int len, ret;
	char lcc;

	while (bufferList.size() < maxBufferLength) {
		ret = demuxer->getNextPacketbyFilter(&tsPacket);
		if (ret > 0) {
			lcc = demuxer->getContinuityCounter(tsPacket->getPid());
			if (lcc >= 0) {
				if ((lcc + 1) != tsPacket->getContinuityCounter()) {
					if ((lcc == 15) && tsPacket->getContinuityCounter() != 0) {
						cout << "continuity counter error!" << endl;
					}
				}
			}
			demuxer->setContinuityCounter(tsPacket->getPid(),
										  tsPacket->getContinuityCounter());
			if ((tsPacket->getAdaptationFieldControl() == 1) ||
				(tsPacket->getAdaptationFieldControl() == 3)) {
				tsPacket->process();
				if (tsPacket->getStartIndicator()) {
					if (tsPacket->getNumberOfPayloads() > 1) {
						//add last data and finish packet
						len = tsPacket->getPayload2(&payload);
						pesPacket->process(payload, len);
						processPesPacket();
						delete pesPacket;
						pesPacket = NULL;
					}
					if (pesPacket) {
						//finish packet
						processPesPacket();
						delete pesPacket;
						pesPacket = NULL;
					}
					len = tsPacket->getPayload(&payload);
					pesPacket = new PESPacket(payload, len);
					//numPesPacketRead++;
					//check if it is a PES start
					if (pesPacket->getStartCodePrefix() != 0x01) {
						delete pesPacket;
						pesPacket = NULL;
						if (timestamp2 == -1) resetControls();
					}
					//if ((bufferList.size() == maxBufferLength) && pesPacket)
					//means that a packet has been read and you can't discard it
					//for the next call of fillbuffer()
				} else {
					//add data
					if (pesPacket) {
						len = tsPacket->getPayload(&payload);
						pesPacket->process(payload, len);
					}
				}
			}
			if (tsPacket) delete tsPacket;
			tsPacket = NULL;
		} else if (ret == -1) {
			cout << "PESStream::fillBuffer - Unable to get a packet." << endl;
		}
		//It not necessary to release the packet here.
		//(because it is always equal to NULL)
	}
}

bool PESStream::disposeBuffer() {
	bool ret = Stream::disposeBuffer();
	if (ret) {
		ptsDtsDiff.erase(ptsDtsDiff.begin());
		frequencyList.erase(frequencyList.begin());
		lastRangeChange = rangeChangeList[0];
		rangeChangeList.erase(rangeChangeList.begin());
	}
	return ret;
}

bool PESStream::getBuffer(Buffer** buffer) {
	if (bufferList.size()) {
		if (isVideoStream) {
			PESPacket::setDtsDirectStream(bufferList[0]->buffer, Stc::stcToBase(nextSend + preponeTicks));
			if (nextSend + preponeTicks < currStc) {
				cout << "PESStream::getBuffer - Warning! TS bitrate to low!" << endl;
				return false;
			}
		}
		PESPacket::setPtsDirectStream(bufferList[0]->buffer, Stc::stcToBase(nextSend + preponeTicks) + ptsDtsDiff[0]);
	}
	return Stream::getBuffer(buffer);
}

void PESStream::updateNextSend(int64_t stc) {
	if (bufferList.size()) {
		if (isVideoStream) {
			//Video stream
			if (hasDts) {
				if (frequencyList[0] == -1) {
					nextSend += frequency;
					return;
				} else {
					if (lastDtsAvailable == -1) {
						lastDtsAvailable = frequencyList[0];
						lastNextSendWithDts = nextSend;
						nextSend += frequency;
					} else {
						if (lastRangeChange != rangeChangeList[0]) {
							lastDtsAvailable = -1;
							lastNextSendWithDts = nextSend;
							nextSend += frequency;
						} else {
							int64_t nextValue = lastNextSendWithDts +
								Stc::baseExtToStc(Stc::calcDiff90khz(frequencyList[0], lastDtsAvailable), 0);
							if (nextSend > nextValue) {
								frequency = (nextSend - nextValue);
								//TODO: try to smooth the nextSend in frames with no DTS.
								cout << "PESStream::updateNextSend - Standard " <<
										"frequency to high. Frequency adjusted." << endl;

							}
							nextSend = nextValue;
							lastDtsAvailable = frequencyList[0];
							lastNextSendWithDts = nextSend;
							//nextSend = nextValue;
						}
					}
					return;
				}
			}
		}
		//Audio stream or video stream without any DTS
		if (lastRangeChange != rangeChangeList[0]) {
			nextSend += frequency;
		} else {
			if (frequencyList.size() > 1) {
				if (rangeChangeList[0] != rangeChangeList[1]) {
					nextSend += frequency;
					return;
				}
				if ((frequencyList[0] >= 0) && (frequencyList[1] >= 0)) {
					nextSend += Stc::baseExtToStc(Stc::calcDiff90khz(frequencyList[1], frequencyList[0]), 0);
				} else {
					cout << "PESStream::updateNextSend - Oh no!" << endl;
				}
			} else {
				cout << "PESStream::updateNextSend - Dammit!" << endl;
			}
		}
	} else {
		cout << "PESStream::updateNextSend - The buffer is empty!" << endl;
	}
}

void PESStream::setIsVideoStream(bool v) {
	isVideoStream = v;
}

void PESStream::setHasDts(bool has) {
	hasDts = has;
}

void PESStream::addPidFilter(unsigned short pid) {
	if (!demuxer) demuxer = new Demuxer();
	demuxer->addPidFilter(pid);
}

void PESStream::setFilename(string filename) {
	if (!tsReader) {
		tsReader = new TSFileReader(filename);
	} else {
		tsReader->setFilename(filename);
	}
	demuxer->setTsReader(tsReader);
}

void PESStream::resetControls() {
	timestamp1 = -1;
	timestamp2 = -1;
	ptsDtsDiff.clear();
	frequencyList.clear();
	rangeChangeList.clear();
	releaseBufferList();
}

void PESStream::setInputRangeList(vector<InputRange*>* irl) {
	inputRangeList = irl;
}

bool PESStream::isInRange(int64_t pts) {
	if (!inputRangeList) return true;

	vector<InputRange*>::iterator it;
	int relTime = (int) (Stc::baseToSecond(Stc::calcDiff90khz(pts, firstPts)) * 1000);
	//TS input guaranteed up to 26.5 hours in duration (limited to 53 hours).
	it = inputRangeList->begin();
	while (it != inputRangeList->end()) {
		if ((relTime >= (*it)->getBegin()) && (relTime < (*it)->getEnd())) {
			return true;
		}
		++it;
	}
	return false;
}

void PESStream::processPesPacket() {
	if (timestampControl(pesPacket)) {
		if (pesPacket->getPtsDtsFlags() > 1) {
			bool reallyNow = isInRange(pesPacket->getPts());
			if (isInRangeNow != reallyNow) rangeChange++;
			isInRangeNow = reallyNow;
		}
		if (isInRangeNow) addBuffer();
	}
}

}
}
}
}


