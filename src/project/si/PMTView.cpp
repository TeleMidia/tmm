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
	serviceType = SRV_TYPE_TV;
	eitProj = NULL;
}

PMTView::~PMTView() {
	deleteAllStreams();
	releaseEsDescriptorList();
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

void PMTView::setPcrPeriod(unsigned int freq) {
	pcrPeriod = freq;
}

unsigned int PMTView::getPcrPeriod() {
	return pcrPeriod;
}

void PMTView::setServiceName(const string& name) {
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
	vector<ProjectInfo*>* prjList;
	vector<ProjectInfo*>::iterator itPrj;
	map<unsigned short, vector<ProjectInfo*>*>::iterator it = projectInfoList.begin();

	if (!projectInfoList.count(pid)) {
		prjList = new vector<ProjectInfo*>();
		projectInfoList.insert(
				it, std::pair<unsigned short, vector<ProjectInfo*>*>(pid, prjList));
	} else {
		prjList = projectInfoList[pid];
	}
	for (itPrj = prjList->begin(); itPrj != prjList->end(); ++itPrj) {
		if (*itPrj == projInfo) return false;
	}
	prjList->push_back(projInfo);

	return true;
}

bool PMTView::addStream(unsigned short pid, Stream* stream) {
	vector<Stream*>* strList;
	vector<Stream*>::iterator itStr;
	map<unsigned short, vector<Stream*>*>::iterator it = streamList.begin();

	if (!streamList.count(pid)) {
		strList = new vector<Stream*>();
		streamList.insert(
				it, std::pair<unsigned short, vector<Stream*>*>(pid, strList));
	} else {
		strList = streamList[pid];
	}
	for (itStr = strList->begin(); itStr != strList->end(); ++itStr) {
		if (*itStr == stream) return false;
	}
	strList->push_back(stream);

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

bool PMTView::addPidToLayer(unsigned short pid, unsigned char layer) {
	layerList[pid] = layer;
	return true;
}

unsigned char PMTView::getLayerPid(unsigned short pid) {
	if (layerList.count(pid)) {
		return layerList[pid];
	}
	return 0xFF;
}

bool PMTView::addEsDescriptor(unsigned short pid, MpegDescriptor* md) {
	vector<MpegDescriptor*>* mdlist;
	if (!esDescriptorList.count(pid)) {
		mdlist = new vector<MpegDescriptor*>;
		esDescriptorList[pid] = mdlist;
	} else {
		mdlist = esDescriptorList[pid];
	}
	mdlist->push_back(md);

	return true;
}

void PMTView::cleanLayerList() {
	layerList.clear();
}

bool PMTView::deleteAllStreams() {
	map<unsigned short, vector<Stream*>*>::iterator it;
	vector<Stream*>::iterator itStr;

	for (it = streamList.begin(); it != streamList.end(); ++it) {
		itStr = it->second->begin();
		while (itStr != it->second->end()) {
			delete (*itStr);
			++itStr;
		}
		delete (it->second);
	}
	streamList.clear();

	return true;
}

bool PMTView::releaseEsDescriptorList() {
	map<unsigned short, vector<MpegDescriptor*>* >::iterator it;
	vector<MpegDescriptor*>::iterator itv;
	it = esDescriptorList.begin();
	while (it != esDescriptorList.end()) {
		if (it->second) {
			itv = it->second->begin();
			while (itv != it->second->end()) {
				if (*itv) delete (*itv);
				++itv;
			}
			delete it->second;
		}
		++it;
	}
	esDescriptorList.clear();
	return true;
}

map<unsigned short, vector<ProjectInfo*>*>* PMTView::getProjectInfoList() {
	return &projectInfoList;
}

map<unsigned short, vector<Stream*>*>* PMTView::getStreamList() {
	return &streamList;
}

map<unsigned short, unsigned char>* PMTView::getComponentTagList() {
	return &componentTagList;
}

int PMTView::getProjectPid(ProjectInfo* proj) {
	map<unsigned short, vector<ProjectInfo*>*>::iterator it;
	vector<ProjectInfo*>::iterator itPrj;

	it = projectInfoList.begin();
	while(it != projectInfoList.end()) {
		if (it->second) {
			itPrj = it->second->begin();
			while(itPrj != it->second->end()) {
				if (proj == (*itPrj)) {
					return it->first;
				}
				++itPrj;
			}
		}
		++it;
	}

	return -1;
}

map<unsigned short, unsigned char>* PMTView::getLayerList() {
	return &layerList;
}

map<unsigned short, vector<MpegDescriptor*>* >* PMTView::getEsDescriptorList() {
	return &esDescriptorList;
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
	map<unsigned short, vector<ProjectInfo*>*>* previousProjectList;
	short newVideoTag;
	short newAudioTag;
	short newCarouselTag;
	map<unsigned short, vector<ProjectInfo*>*>::iterator itProj;
	map<unsigned short, vector<ProjectInfo*>*>::iterator itPreviousProj;
	vector<ProjectInfo*>* previousPrjListV;
	vector<ProjectInfo*>::iterator itPrjV;
	vector<ProjectInfo*>::iterator itPreviousPrjV;
	map<unsigned short, unsigned char>::iterator itTag;
	map<unsigned short, unsigned char>::iterator itPreviousTag;
	map<unsigned short, unsigned char>::iterator itFound;
	InputData* indata;
	bool caseFound;

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
				itPrjV = itProj->second->begin();
				while (itPrjV != itProj->second->end()) {
					previousPrjListV = previousProjectList->find(
							itPreviousTag->first)->second;
					itPreviousPrjV = previousPrjListV->begin();
					while (itPreviousPrjV != previousPrjListV->end()) {
						if ((*itPreviousPrjV)->getProjectType() ==
								(*itPrjV)->getProjectType()) {
							//It's the same. Check if it's not already in use
							if (!isComponentTagInUse(itPreviousTag->second)) {
								//No, so create/update it.
								componentTagList[itPreviousTag->first] = itPreviousTag->second;
							} //else this tag cannot be reused from the previous PMTView.
						}
						++itPreviousPrjV;
					}
					++itPrjV;
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
			caseFound = false;
			itPrjV = itProj->second->begin();
			while (itPrjV != itProj->second->end()) {
				switch ((*itPrjV)->getProjectType()) {
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
					caseFound = true;
					break;
				case PT_CAROUSEL:
					while (isComponentTagInUse(newCarouselTag)) {
						newCarouselTag++;
					}
					componentTagList[itProj->first] = newCarouselTag++;
					caseFound = true;
					break;
				}
				if (caseFound) break;
				++itPrjV;
			}
		}
		++itProj;
	}

	return true;
}

void PMTView::markAllProjectsReuse(bool use) {
	map<unsigned short, vector<ProjectInfo*>*>::iterator it;
	vector<ProjectInfo*>::iterator itPrj;

	it = projectInfoList.begin();
	while(it != projectInfoList.end()) {
		if (it->second) {
			itPrj = it->second->begin();
			while(itPrj != it->second->end()) {
				(*itPrj)->setReuse(use);
				++itPrj;
			}
		}
		++it;
	}
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


