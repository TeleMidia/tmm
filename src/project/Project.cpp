/*
 * Project.cpp
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#include "project/Project.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

Project::Project() {
	projectList = new map<int, ProjectInfo*>;
	stcBegin = SYSTEM_CLOCK_FREQUENCY;
	isLoop = false;
	vbvBuffer = 1.0;
	ttl = 16;
}

Project::~Project() {
	map<int, ProjectInfo*>::iterator itProj;

	if (projectList) {
		itProj = projectList->begin();
		while (itProj != projectList->end()) {
			delete (itProj->second);
			++itProj;
		}
		delete projectList;
	}
}

bool Project::mountCarousels() {
	char number[11];
	string path, tempPath;
	map<int, ProjectInfo*>::iterator itProj;
	PCarousel* pcar;

	if (projectList) {
		itProj = projectList->begin();
		while (itProj != projectList->end()) {
			if (itProj->second->getProjectType() != PT_CAROUSEL) {
				++itProj;
				continue;
			}
			pcar = (PCarousel*) itProj->second;
			pcar->setSectionEncapsulationMode(true);
			pcar->setBlockSize(4066);
			snprintf(number, 11, "%d", pcar->getServiceDomain());

			path = "./carousel/";
			makeDir(path.c_str(), 0777);
			path.insert(path.size(), number);
			makeDir(path.c_str(), 0777);
			tempPath.assign(path);
			path.insert(path.size(), "/output/");
			makeDir(path.c_str(), 0777);
			tempPath.insert(tempPath.size(), "/temp/");
			makeDir(tempPath.c_str(), 0);
			path.insert(path.size(), number);
			path.insert(path.size(), ".sec");
			pcar->setOutputFile(path);
			pcar->setTempFolder(tempPath);
			pcar->createCarousel(path, tempPath);
			++itProj;
		}
	}
	return true;
}

int Project::configAit(PAit* ait, unsigned int ctag, string aName, string lang,
			  string baseDir, string initClass, unsigned int orgId,
			  unsigned short appId, unsigned int appcode) {
	vector<MpegDescriptor*>* dlist = new vector<MpegDescriptor*>;

	TransportProtocol* tp = new TransportProtocol();
	tp->setProtocolId(0x01);
	tp->setTransportProtocolLabel(0x00);
	ObjectCarouselProtocol* ocp = new ObjectCarouselProtocol;
	ocp->remoteConnection = false;
	ocp->componentTag = ctag;
	tp->setOcProtocol(ocp);
	dlist->push_back(tp);

	Application* appDesc = new Application();
	appDesc->addAppProfile(0x8001, 0x01, 0x00, 0x00);
	appDesc->setServiceBoundFlag(true);
	appDesc->setVisibility(0x03);
	appDesc->setApplicationPriority(0x01);
	char pl[1];
	pl[0] = 0x00;
	appDesc->setTransportProtocolLabel(pl, 1);
	dlist->push_back(appDesc);

	ApplicationName* appName = new ApplicationName();
	appName->setAppName(lang, aName);
	dlist->push_back(appName);

	GingaApplication* ga = new GingaApplication(0x06);
	dlist->push_back(ga);

	GingaApplicationLocation* gal = new GingaApplicationLocation(0x07);
	gal->setBaseDirectory(baseDir);
	gal->setInitialClass(initClass);
	dlist->push_back(gal);

	ait->addApplicationInfo(orgId, appId, appcode, dlist);

	return 0;
}

void Project::setFilename(string filename) {
	this->filename = filename;
}

string Project::getFilename() {
	return filename;
}

string Project::getProjectName() {
	return projectName;
}

string Project::getProjectDescription() {
	return projectDescription;
}

void Project::setDestination(string dest) {
	destination = dest;
}

string Project::getDestination() {
	return destination;
}

void Project::setProviderName(string name) {
	providerName = name;
}

string Project::getProviderName() {
	return providerName;
}

void Project::setTsid(int id) {
	tsid = id;
}

int Project::getTsid() {
	return tsid;
}

void Project::setTsBitrate(int rate) {
	tsBitrate = rate;
}

int Project::getTsBitrate() {
	return tsBitrate;
}

void Project::setStcBegin(int64_t stc) {
	stcBegin = stc;
}

int64_t Project::getStcBegin() {
	return stcBegin;
}

map<int, ProjectInfo*>* Project::getProjectList() {
	return projectList;
}

double Project::getVbvBuffer() {
	return vbvBuffer;
}

bool Project::getIsLoop(){
	return isLoop;
}

unsigned char Project::getTTL() {
	return ttl;
}

unsigned short Project::getOriginalNetworkId() {
	return originalNetworkId;
}

string Project::getTsName() {
	return tsName;
}

unsigned short Project::getBroadcastFrequency() {
	return broadcastFrequency;
}

unsigned char Project::getVirtualChannel() {
	return virtualChannel;
}

unsigned char Project::getGuardInterval() {
	return guardInterval;
}

unsigned char Project::getTransmissionMode() {
	return transmissionMode;
}

ProjectInfo* Project::findProject(int id) {
	map<int, ProjectInfo*>::iterator itProj;

	if (projectList) {
		itProj = projectList->find(id);
		if (itProj != projectList->end()) {
			return itProj->second;
		}
	}

	return NULL;
}

}
}
}
}


