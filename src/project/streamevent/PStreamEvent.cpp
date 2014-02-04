/*
 * PStreamEvent.cpp
 *
 *  Created on: 03/02/2014
 *      Author: Felippe Nagato
 */

#include "project/streamevent/PStreamEvent.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PStreamEvent::PStreamEvent() {
	firstReference = 0;
	projectType = PT_STREAMEVENT;
	firstReferenceOffset = 0.0;
	period = 1000;
	sampleLimit = 0;
	carouselProj = NULL;
	entryPoint.clear();
	baseId.clear();
}

PStreamEvent::~PStreamEvent() {
	vector<StreamEvent*>::iterator it;

	it = streamEventList.begin();
	while (it != streamEventList.end()) {
		if (*it)
			delete (*it);
		++it;
	}
}

void PStreamEvent::setPeriod(unsigned int p) {
	period = p;
}

unsigned int PStreamEvent::getPeriod() {
	return period;
}

void PStreamEvent::setSampleLimit(unsigned int sl) {
	sampleLimit = sl;
}

unsigned int PStreamEvent::getSampleLimit() {
	return sampleLimit;
}

void PStreamEvent::addStreamEvent(StreamEvent* se) {
	streamEventList.push_back(se);
}

void PStreamEvent::setFirstReference(int64_t first) {
	firstReference = first;
}

double PStreamEvent::getFirstReferenceOffset() {
	return firstReferenceOffset;
}

void PStreamEvent::setFirstReferenceOffset(double offset) {
	firstReferenceOffset = offset;
}

void PStreamEvent::setCarouselProj(ProjectInfo* proj) {
	carouselProj = proj;
}

ProjectInfo* PStreamEvent::getCarouselProj() {
	return carouselProj;
}

void PStreamEvent::setEntryPoint(string ep) {
	entryPoint = ep;
}

string PStreamEvent::getEntryPoint() {
	return entryPoint;
}

vector<StreamEvent*>* PStreamEvent::getStreamEventList() {
	return &streamEventList;
}

void PStreamEvent::setBaseId(unsigned short bi) {
	char buffer[6];
	sprintf(buffer, "%d", bi);
	baseId.assign(buffer);
}

void PStreamEvent::setBaseId(string bi) {
	baseId = bi;
}

string PStreamEvent::getBaseId() {
	return baseId;
}

void PStreamEvent::setDocumentId(string di) {
	documentId = di;
}

string PStreamEvent::getDocumentId() {
	return documentId;
}

int PStreamEvent::encode(int64_t stc, vector<pair<char*,int>*>* list) {
	vector<StreamEvent*>::iterator it;
	DSMCCSection* dsmccSection = NULL;
	char* stream;
	int len;

	if (streamEventList.size()) {
		dsmccSection = new DSMCCSection();
		for (it = streamEventList.begin(); it != streamEventList.end(); ++it) {
			StreamEvent* se = new StreamEvent();
			se->setCommandTag((*it)->getCommandTag());
			se->setEventId((*it)->getEventId());
			se->setEventNPT((*it)->getEventNPT());
			se->setFinalFlag((*it)->getFinalFlag());
			se->setSequenceNumber((*it)->getSequenceNumber());
			len = (*it)->getPrivateDataPayload(&stream);
			se->setPrivateDataPayload(stream, len);

			dsmccSection->addDsmccDescriptor(se);
		}

		dsmccSection->setTableId(0x3D);
		dsmccSection->setSectionSyntaxIndicator(1);
		dsmccSection->setTableIdExtension(0x00);
		dsmccSection->setCurrentNextIndicator(1);
		dsmccSection->setSectionNumber(0x00);
		dsmccSection->setLastSectionNumber(0x00);
		dsmccSection->setVersionNumber(0);//TODO: changes in descriptors must change this
		dsmccSection->updateStream();
	}

	char* tempStream;
	pair<char*,int>* myp = new pair<char*,int>;
	int length = dsmccSection->getStream(&tempStream);
	myp->first = new char[length];
	memcpy(myp->first, tempStream, length);
	myp->second = length;
	list->push_back(myp);

	delete (dsmccSection);

	return 0;
}

}
}
}
}


