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
	(*projectList)[0] = new PPat();
	stcBegin = SYSTEM_CLOCK_FREQUENCY * 10;
	isLoop = false;
	vbvBuffer = 1.0;
	ttl = 16;
	packetsInBuffer = 40;
	useTot = false;
	useSdt = false;
	useNit = false;
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

			path = "." + getUriSlash() + "carousel" + getUriSlash();
			makeDir(path.c_str(), 0777);
			path.insert(path.size(), number);
			makeDir(path.c_str(), 0777);
			tempPath.assign(path);
			path.insert(path.size(), getUriSlash() + "output" + getUriSlash());
			makeDir(path.c_str(), 0777);
			tempPath.insert(tempPath.size(), getUriSlash() + "temp" + getUriSlash());
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
	ocp->originalNetworkId = originalNetworkId;
	ocp->transportStreamId = tsid;
	//ocp->serviceId: use configAitService() to define this field,
	//because it depends on the service that it's broadcasted.
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

int Project::configAitService(ProjectInfo* ait, unsigned short serviceId) {
	vector<MpegDescriptor*>::iterator itDesc;
	PAit* pAit = (PAit*)ait;
	vector<MpegDescriptor*>* dlist = pAit->getDescriptorList();

	for (itDesc = dlist->begin(); itDesc != dlist->end(); ++itDesc) {
		if ((*itDesc)->getDescriptorTag() == 0x02) {
			TransportProtocol* tp = (TransportProtocol*)(*itDesc);
			if (tp) {
				ObjectCarouselProtocol* ocp = tp->getOcProtocol();
				if (ocp) {
					ocp->serviceId = serviceId;
				}
			}
		}
	}

	return 0;
}

int Project::configSdt(vector<pmtViewInfo*>* newTimeline, ProjectInfo* sdt) {
	vector<pmtViewInfo*>::iterator itPmt;
	PSdt* pSdt = (PSdt*)sdt;

	if (sdt == NULL) return -1;
	pSdt->releaseAllServiceInformation();
	pSdt->setCurrentNextIndicator(1);
	pSdt->setOriginalNetworkId(originalNetworkId);
	pSdt->setTableIdExtension(tsid);
	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		ServiceInformation* si = new ServiceInformation();
		Service* service = new Service();
		service->setServiceName((*itPmt)->pv->getServiceName());
		service->setProviderName(providerName);
		if ((*itPmt)->pv->getEitProj()) {
			si->eitPresentFollowingFlag = true;
		} else {
			si->eitPresentFollowingFlag = false;
		}
		si->eitScheduleFlag = false;
		si->freeCaMode = 0;
		si->runningStatus = 4; //Running
		si->serviceId = (*itPmt)->pv->getProgramNumber();
		si->descriptorList.push_back(service);
		pSdt->addServiceInformation(si);
		++itPmt;
	}

	return 0;
}

int Project::configNit(vector<pmtViewInfo*>* newTimeline, ProjectInfo* nit) {
	vector<pmtViewInfo*>::iterator itPmt;
	PNit* pNit = (PNit*)nit;
	unsigned int serviceId = 65536;

	if (pNit == NULL) return -1;

	pNit->releaseAllDescriptors();
	pNit->releaseAllTransportInformation();
	pNit->setCurrentNextIndicator(1);
	pNit->setTableIdExtension(originalNetworkId);
	NetworkName* netName = new NetworkName();
	netName->setNetworkName(providerName);
	pNit->addDescriptor(netName);
	SystemManagement* sysMan = new SystemManagement();
	pNit->addDescriptor(sysMan);
	TransportInformation* ti = new TransportInformation();
	ti->transportStreamId = tsid;
	ti->originalNetworkId = pNit->getTableIdExtension();
	ServiceList* sl = new ServiceList();
	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		sl->addService((*itPmt)->pv->getProgramNumber(), DIGITAL_TELEVISION_SERVICE);
		++itPmt;
	}
	ti->descriptorList.push_back(sl);
	TerrestrialDeliverySystem* tds = new TerrestrialDeliverySystem();
	tds->setGuardInterval(guardInterval);
	tds->setTransmissionMode(transmissionMode);
	tds->addFrequency(broadcastFrequency);
	ti->descriptorList.push_back(tds);
	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		if ((*itPmt)->pv->getProgramNumber() < serviceId) {
			serviceId = (*itPmt)->pv->getProgramNumber();
		}
		++itPmt;
	}
	if (serviceId != 65536) {
		PartialReception* pr = new PartialReception();
		pr->addServiceId(serviceId);
		ti->descriptorList.push_back(pr);
	}
	TSInformation* tsinfo = new TSInformation();
	tsinfo->setRemoteControlKeyId(virtualChannel);
	tsinfo->setTsName(tsName);
	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		TransmissionType* tt = new TransmissionType();
		tt->transmissionTypeInfo = 0x0F; // ???
		tt->serviceIdList.insert((*itPmt)->pv->getProgramNumber());
		tsinfo->addTransmissionTypeList(tt);
		++itPmt;
	}
	ti->descriptorList.push_back(tsinfo);
	pNit->addTransportInformation(ti);

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

unsigned short Project::getPacketsInBuffer() {
	return packetsInBuffer;
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

void Project::setUseTot(bool use) {
	useTot = use;
}

bool Project::getUseTot() {
	return useTot;
}

void Project::setUseSdt(bool use) {
	useSdt = use;
}

bool Project::getUseSdt() {
	return useSdt;
}

void Project::setUseNit(bool use) {
	useNit = use;
}

bool Project::getUseNit() {
	return useNit;
}

}
}
}
}


