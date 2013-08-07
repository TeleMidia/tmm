/*
 * PMTView.cpp
 *
 *  Created on: 07/03/2013
 *      Author: Felippe Nagato
 */

#include "project/si/PMTView.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PMTView::PMTView() {
	ProjectInfo();
	pmtStream = NULL;
	projectType = PT_PMTVIEW;
	serviceType = -1;
	eitProj = NULL;
}

PMTView::~PMTView() {
	deleteAllStreams();
	if (pmtStream) delete pmtStream;
}

void PMTView::setPid(unsigned short pid) {
	this->pid = pid;
}

unsigned short PMTView::getPid() {
	return pid;
}

void PMTView::setPmtStream(Stream* stream) {
	pmtStream = stream;
}

Stream* PMTView::getPmtStream() {
	return pmtStream;
}

void PMTView::setProgramNumber(unsigned short pn) {
	programNumber = pn;
}

unsigned short PMTView::getProgramNumber() {
	return programNumber;
}

void PMTView::setTsinput(int id) {
	tsinput = id;
}

int PMTView::getTsinput() {
	return tsinput;
}

void PMTView::setPcrPid(unsigned short pid) {
	pcrPid = pid;
}

unsigned short PMTView::getPcrPid() {
	return pcrPid;
}

void PMTView::setPcrFrequency(unsigned short freq) {
	pcrFrequency = freq;
}

unsigned short PMTView::getPcrFrequency() {
	return pcrFrequency;
}

void PMTView::setServiceName(string name) {
	serviceName = name;
}

string PMTView::getServiceName() {
	return serviceName;
}

void PMTView::setServiceType(short int st) {
	serviceType = st;
}

short int PMTView::getServiceType() {
	return serviceType;
}

ProjectInfo* PMTView::getEitProj() {
	return eitProj;
}

void PMTView::setEitProj(ProjectInfo* proj) {
	eitProj = proj;
}

bool PMTView::addProjectInfo(unsigned short pid, ProjectInfo* projInfo) {
	projectInfoList[pid] = projInfo;
	return true;
}

bool PMTView::addStream(unsigned short pid, Stream* stream) {
	streamList[pid] = stream;
	return true;
}

bool PMTView::addDesiredComponentTag(unsigned short pid, unsigned char tag) {
	desiredComponentTagList[pid] = tag;
	return true;
}

bool PMTView::addComponentTag(unsigned short pid, unsigned char tag) {
	componentTagList[pid] = tag;
	return true;
}

bool PMTView::getComponentTag(unsigned short pid, unsigned char* tag) {
	if (componentTagList.count(pid) > 0) {
		*tag = componentTagList.find(pid)->second;
		if ((*tag) == -1) return false;
		return true;
	}
	return false;
}

bool PMTView::deleteAllStreams() {
	for (unsigned int i = 0; i < streamList.size(); i++) {
		if (streamList[i]) delete streamList[i];
	}
	streamList.clear();
	return true;
}

map<unsigned short, ProjectInfo*>* PMTView::getProjectInfoList() {
	return &projectInfoList;
}

map<unsigned short, Stream*>* PMTView::getStreamList() {
	return &streamList;
}

map<unsigned short, unsigned char>* PMTView::getComponentTagList() {
	return &componentTagList;
}

int PMTView::getProjectPid(ProjectInfo* proj) {
	map<unsigned short, ProjectInfo*>::iterator it;
	it = projectInfoList.begin();
	while(it != projectInfoList.end()) {
		if (it->second == proj) return it->first;
		++it;
	}
	return -1;
}

bool PMTView::isDesiredComponentTagInUse(unsigned char ctag) {
	map<unsigned short, unsigned char>::iterator itTag;

	itTag = desiredComponentTagList.begin();
	while (itTag != desiredComponentTagList.end()) {
		if (itTag->second == ctag) {
			return true;
		}
		++itTag;
	}
	return false;
}

bool PMTView::isComponentTagInUse(unsigned char ctag) {
	map<unsigned short, unsigned char>::iterator itTag;

	itTag = componentTagList.begin();
	while (itTag != componentTagList.end()) {
		if (itTag->second == ctag) {
			return true;
		}
		++itTag;
	}
	return false;
}

bool PMTView::fulfillComponentTagList(PMTView* previousPmtView) {
	map<unsigned short, unsigned char>* previousComponentTagList;
	map<unsigned short, ProjectInfo*>* previousProjectList;
	short newVideoTag;
	short newAudioTag;
	short newCarouselTag;
	map<unsigned short, ProjectInfo*>::iterator itProj;
	map<unsigned short, ProjectInfo*>::iterator itPreviousProj;
	map<unsigned short, unsigned char>::iterator itTag;
	map<unsigned short, unsigned char>::iterator itPreviousTag;
	map<unsigned short, unsigned char>::iterator itFound;
	InputData* indata;

	//try to add desired tags based on the current project
	componentTagList.clear();
	itProj = projectInfoList.begin();
	while (itProj != projectInfoList.end()) {
		itFound = desiredComponentTagList.find(itProj->first);
		if (itFound != desiredComponentTagList.end()) {
			//it exists in project
			if (itFound->second != -1) {
				//create a new one, because user forced it in project.
				componentTagList[itFound->first] = itFound->second;
			}
		}
		++itProj;
	}

	if (serviceType == SRV_TYPE_TV) {
		newVideoTag = 0x00;
		newAudioTag = 0x10;
		newCarouselTag = 0x40;
	} else if (serviceType == SRV_TYPE_ONESEG) {
		newVideoTag = 0x81;
		newAudioTag = 0x83;
		newCarouselTag = 0x80;
	} else return false;

	//try to reuse them
	if (previousPmtView) {
		previousComponentTagList = previousPmtView->getComponentTagList();
		previousProjectList = previousPmtView->getProjectInfoList();
		//update current list from the previous one
		itPreviousTag = previousComponentTagList->begin();
		while (itPreviousTag != previousComponentTagList->end()) {
			itProj = projectInfoList.find(itPreviousTag->first);
			if (itProj != projectInfoList.end()) {
				//PID exists, so check if it's the same type
				if (itProj->second->getProjectType() ==
						previousProjectList->find(
								itPreviousTag->first)->second->getProjectType()) {
					//It's the same. Check if it's not already in use
					if (!isComponentTagInUse(itPreviousTag->second)) {
						//No, so create/update it.
						componentTagList[itPreviousTag->first] = itPreviousTag->second;
					} //else this tag cannot be reused from the previous PMTView.
				}
			}
			++itPreviousTag;
		}
	}

	//try to create the remaining tags automatically.
	itProj = projectInfoList.begin();
	while (itProj != projectInfoList.end()) {
		itFound = componentTagList.find(itProj->first);
		if (itFound == componentTagList.end()) {
			//create a new one
			switch (itProj->second->getProjectType()) {
			case PT_INPUTDATA:
				indata = (InputData*) itProj->second;
				if (TSInfo::isVideoStreamType(indata->getStreamType())) {
					while (isComponentTagInUse(newVideoTag)) {
						newVideoTag++;
					}
					componentTagList[itProj->first] = newVideoTag++;
				} else if (TSInfo::isAudioStreamType(indata->getStreamType())) {
					while (isComponentTagInUse(newAudioTag)) {
						newAudioTag++;
					}
					componentTagList[itProj->first] = newAudioTag++;
				}
				break;
			case PT_CAROUSEL:
				while (isComponentTagInUse(newCarouselTag)) {
					newCarouselTag++;
				}
				componentTagList[itProj->first] = newCarouselTag++;
				break;
			}
		}
		++itProj;
	}

	return true;
}

bool PMTView::compareComponentTagList(map<unsigned short, unsigned char>* oldList,
						  map<unsigned short, unsigned char>* newList) {
	map<unsigned short, unsigned char>::iterator itOld;
	map<unsigned short, unsigned char>::iterator itNew;

	if (oldList->size() != newList->size()) return false;

	itNew = newList->begin();
	while (itNew != newList->end()) {
		itOld = oldList->find(itNew->first);
		if (itOld != oldList->end()) {
			if (itNew->second != itOld->second) return false;
		} else return false;
		++itNew;
	}

	return true;
}

}
}
}
}


