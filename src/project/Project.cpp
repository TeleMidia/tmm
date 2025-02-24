/******************************************************************************
TeleM�dia Multiplexer
Copyright (C) 2016 TeleM�dia Lab/PUC-Rio
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
	partialReception = true;
	useTot = false;
	useSdt = false;
	useNit = false;
	packetSize = 188;
	iip = NULL;
	isPipe = false;
	areaCode1 = 0;
	areaCode2 = 0;
	isLive = false;

	fillCommandTagList();
	fillRegionList();

	liveServer.setSharedMemoryName("tmm.live");
	liveServer.showErrorMessages(false);
	cout << "External communication status: ";
	if (liveServer.createSharedMemory()) {
		liveServer.grantAccessToForeign();
		cout << "Available.";
	} else {
		cout << "Unavailable.";
	}
	cout << endl;
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

	if (iip) delete iip;
}

bool Project::changeToProjectDir() {
	char* currDir = getcwd(NULL, 0);
	if (currDir) {
		string currDirString;
		currDirString.assign(currDir);
		tmmPath.assign(currDir);
		unsigned found = filename.find_last_of("/\\");
		currDirString += getUriSlash() + filename.substr(0,found);
		chdir(currDirString.c_str());
		return true;
	}
	return false;
}

unsigned char Project::toLayer(const string& layer) {
	if (layer == "none") {
		return NULL_TSP;
	} else if (tolower(layer.c_str()[0]) == 'a') {
		return HIERARCHY_A;
	} else if (tolower(layer.c_str()[0]) == 'b') {
		return HIERARCHY_B;
	} else if (tolower(layer.c_str()[0]) == 'c') {
		return HIERARCHY_C;
	} else {
		return 0xFF;
	}
}

void Project::setPacketSize(unsigned char size) {
	packetSize = size;
}

unsigned char Project::getPacketSize() {
	return packetSize;
}

bool Project::mountCarousel(PCarousel* pcar) {
	char number[11];
	string path, tempPath;

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
	pcar->setCarouselVersion(pcar->getVersion());
	pcar->createCarousel(path, tempPath);
	pcar->setMounted(true);

	return true;
}

bool Project::mountCarousels() {
	map<int, ProjectInfo*>::iterator itProj;

	if (projectList) {
		itProj = projectList->begin();
		while (itProj != projectList->end()) {
			if (itProj->second->getProjectType() != PT_CAROUSEL) {
				++itProj;
				continue;
			}
			if (!((PCarousel*) itProj->second)->getMounted()) {
				mountCarousel((PCarousel*) itProj->second);
			}
			++itProj;
		}
	}
	return true;
}

bool Project::createStreamEvent(PStreamEvent* pse) {
	char privateStr[252];
	StreamEvent* se;
	vector<StreamEvent*>::iterator itSe;
	vector<StreamEvent*>* sel;
	map<string, InternalIor*>* fi;
	map<string, InternalIor*>::iterator it;
	InternalIor* iior = NULL;
	int ret;

	if (!pse) {
		return false;
	}

	if (pse->getCarouselProj() && pse->getEntryPoint().size()) {
		fi = ((PCarousel*)pse->getCarouselProj())->getFilesIor();
		it = fi->find(pse->getEntryPoint());
		if (it == fi->end()) {
			string fn1, fn2;
			unsigned int found;
			it = fi->begin();
			while (it != fi->end()) {
				fn1.assign(it->first);
				found = fn1.find_last_of("/\\");
				if (found != std::string::npos) {
					fn1 = fn1.substr(found + 1);
					found = pse->getEntryPoint().find_last_of("/\\");
					if (found != std::string::npos) {
						fn2 = pse->getEntryPoint().substr(found + 1);
						if (fn2 == fn1) {
							if (!pse->getDocumentId().size())
								pse->setDocumentId(LocalLibrary::extractBaseId(it->first));
							cout << "Project::createStreamEvent - Selected entrypoint: "
								 << it->first << endl;
							break;
						}
					}
				}
				++it;
			}
		}
		if (it != fi->end()) {
			iior = it->second;
		} else {
			cout << "Project::createStreamEvent - The " << pse->getEntryPoint()
				 << " file doesn't exists." << endl;
			return -8;
		}
	}
	if (!pse->getBaseId().size()) {
		pse->setBaseId(tsid);
	}
	sel = pse->getStreamEventList();
	for (itSe = sel->begin(); itSe != sel->end(); ++itSe) {
		se = *itSe;
		if (!se->getPrivateDataPayloadLength()) {
			ret = 0;
			switch (se->getCommandTag()) {
			case SE_ADD_DOCUMENT:
				ret = sprintf(privateStr,
						"%s,x-sbtvd://\"%s\",\"%d\",\"%d\",\"%d\"",
						pse->getBaseId().c_str(), pse->getEntryPoint().c_str(),
						iior->carousel, iior->moduleId,
						iior->key);
				break;
			case SE_START_DOCUMENT:
				ret = sprintf(privateStr,
						"\"%s\",\"%s\",\"null\",\"null\",\"null\",\"null\"",
						pse->getBaseId().c_str(),
						pse->getDocumentId().c_str());
				break;
			}
			if (ret > 0) se->setPrivateDataPayload(privateStr, ret);
		}
	}

	pse->setProcessed(true);

	return true;
}

bool Project::createStreamEvents() {
	map<int, ProjectInfo*>::iterator itProj;

	if (projectList) {
		itProj = projectList->begin();
		while (itProj != projectList->end()) {
			if (itProj->second->getProjectType() != PT_STREAMEVENT) {
				++itProj;
				continue;
			}
			if (!((PStreamEvent*) itProj->second)->getProcessed()) {
				createStreamEvent((PStreamEvent*) itProj->second);
			}
			++itProj;
		}
	}
	return true;
}

int Project::configAit(PAit* ait, unsigned int ctag, const string& aName,
		const string& lang, const string& baseDir, const string& initClass,
		unsigned int orgId, unsigned short appId, unsigned int appcode,
		unsigned char recommendedResolution,
		int application_profile) {
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
	appDesc->addAppProfile(application_profile, 0x01, 0x08, 0x08);
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

	GingaApplication* ga = NULL;
	GingaApplicationLocation* gal = NULL;
	if (ait->getTableIdExtension() == AT_GINGA_NCL) {
		ga = new GingaApplication(0x06);
		gal = new GingaApplicationLocation(0x07);
	} else if (ait->getTableIdExtension() == AT_GINGA_J) {
		ga = new GingaApplication(0x03);
		gal = new GingaApplicationLocation(0x04);
	} else if (ait->getTableIdExtension() == 0x000B) {
		ga = new GingaApplication(0x0E);
		gal = new GingaApplicationLocation(0x0F);
	}
	if (ga) dlist->push_back(ga);
	if (gal) {
		gal->setBaseDirectory(baseDir);
		gal->setInitialClass(initClass);
		dlist->push_back(gal);
	}

	ait->addApplicationInfo(orgId, appId, appcode, recommendedResolution, dlist);

	return 0;
}

int Project::configAitService(ProjectInfo* ait, unsigned short serviceId,
		unsigned char ctag) {
	vector<MpegDescriptor*>::iterator itDesc;
	vector<AppInformation*>::iterator itApp;
	PAit* pAit = (PAit*)ait;
	vector<AppInformation*>* appList = pAit->getAppInformationList();

	for (itApp = appList->begin(); itApp != appList->end(); ++itApp) {
		for (itDesc = (*itApp)->appDescriptorList->begin();
				itDesc != (*itApp)->appDescriptorList->end(); ++itDesc) {
			if ((*itDesc)->getDescriptorTag() == 0x02) {
				TransportProtocol* tp = (TransportProtocol*)(*itDesc);
				if (tp) {
					ObjectCarouselProtocol* ocp = tp->getOcProtocol();
					if (ocp) {
						ocp->serviceId = serviceId;
						ocp->componentTag = ctag;
					}
				}
				return 0;
			}
		}
	}

	return 0;
}

int Project::configSdt(vector<pmtViewInfo*>* newTimeline, ProjectInfo* sdt) {
	map<unsigned short, PMTView*> orderedPmtList;
	map<unsigned short, PMTView*>::iterator itOrd;
	vector<pmtViewInfo*>::iterator itPmt;

	PSdt* pSdt = (PSdt*)sdt;

	if (sdt == NULL) return -1;
	pSdt->releaseAllServiceInformation();
	pSdt->setCurrentNextIndicator(1);
	pSdt->setOriginalNetworkId(originalNetworkId);
	pSdt->setTableIdExtension(tsid);
	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		orderedPmtList[(*itPmt)->pv->getPid()] = (*itPmt)->pv;
		++itPmt;
	}
	itOrd = orderedPmtList.begin();
	while (itOrd != orderedPmtList.end()) {
		ServiceInformation* si = new ServiceInformation();
		Service* service = new Service();
		service->setServiceName(itOrd->second->getServiceName());
		service->setProviderName(providerName);
		if (itOrd->second->getEitProj()) {
			si->eitPresentFollowingFlag = true;
		} else {
			si->eitPresentFollowingFlag = false;
		}
		si->eitScheduleFlag = false;
		si->freeCaMode = 0;
		si->runningStatus = 4; //Running
		si->serviceId = itOrd->second->getProgramNumber();
		si->descriptorList.push_back(service);
		pSdt->addServiceInformation(si);
		++itOrd;
	}

	return 0;
}

int Project::configNit(vector<pmtViewInfo*>* newTimeline, ProjectInfo* nit) {
	map<unsigned short, PMTView*> orderedPmtList;
	map<unsigned short, PMTView*>::iterator itOrd;
	vector<pmtViewInfo*>::iterator itPmt;
	PNit* pNit = (PNit*)nit;

	if (pNit == NULL) return -1;

	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		orderedPmtList[(*itPmt)->pv->getPid()] = (*itPmt)->pv;
		++itPmt;
	}

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
	itOrd = orderedPmtList.begin();
	while (itOrd != orderedPmtList.end()) {
		sl->addService(itOrd->second->getProgramNumber(), DIGITAL_TELEVISION_SERVICE);
		++itOrd;
	}
	ti->descriptorList.push_back(sl);
	TerrestrialDeliverySystem* tds = new TerrestrialDeliverySystem();
	tds->setGuardInterval(guardInterval);
	tds->setTransmissionMode(transmissionMode);
	tds->addFrequency(broadcastFrequency * 7);
	if ((areaCode1 > 0) && (areaCode2 > 0)) {
		unsigned short areaCode = areaCode1 << 7;
		areaCode = areaCode | areaCode2;
		tds->setAreaCode(areaCode);
	}
	ti->descriptorList.push_back(tds);

	if (partialReception) {
		PartialReception* pr = new PartialReception();
		itOrd = orderedPmtList.begin();
		while (itOrd != orderedPmtList.end()) {
			if (itOrd->second->getServiceType() == SRV_TYPE_ONESEG) {
				pr->addServiceId(itOrd->second->getProgramNumber());
			}
			++itOrd;
		}
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

void Project::setFilename(const string& filename) {
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

void Project::setDestination(const string& dest) {
	destination = dest;
}

string Project::getDestination() {
	return destination;
}

bool Project::getIsPipe() {
	return isPipe;
}

string Project::getExternalApp() {
	return externalApp;
}

string Project::getAppParams() {
	return appParams;
}

void Project::setProviderName(const string& name) {
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

IIP* Project::getIip() {
	return iip;
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

void Project::updateRelativeStc(int64_t relStc) {
	this->relStc = relStc;
}

bool Project::getIsLive() {
	return isLive;
}

void Project::fillCommandTagList() {
	commandTagList["openbase"] = SE_OPEN_BASE;
	commandTagList["activatebase"] = SE_ACTIVATE_BASE;
	commandTagList["deactivatebase"] = SE_DEACTIVATE_BASE;
	commandTagList["savebase"] = SE_SAVE_BASE;
	commandTagList["closebase"] = SE_CLOSE_BASE;
	commandTagList["adddocument"] = SE_ADD_DOCUMENT;
	commandTagList["removedocument"] = SE_REMOVE_DOCUMENT;
	commandTagList["startdocument"] = SE_START_DOCUMENT;
	commandTagList["stopdocument"] = SE_STOP_DOCUMENT;
	commandTagList["pausedocument"] = SE_PAUSE_DOCUMENT;
	commandTagList["resumedocument"] = SE_RESUME_DOCUMENT;
	commandTagList["savedocument"] = SE_SAVE_DOCUMENT;
	commandTagList["addregionbase"] = SE_ADD_REGION_BASE;
	commandTagList["removeregionbase"] = SE_REMOVE_REGION_BASE;
	commandTagList["addrule"] = SE_ADD_RULE;
	commandTagList["removerule"] = SE_REMOVE_RULE;
	commandTagList["addrulebase"] = SE_ADD_RULE_BASE;
	commandTagList["removerulebase"] = SE_REMOVE_RULE_BASE;
	commandTagList["addconnector"] = SE_ADD_CONNECTOR;
	commandTagList["removeconnector"] = SE_REMOVE_CONNECTOR;
	commandTagList["addconnectorbase"] = SE_ADD_CONNECTOR_BASE;
	commandTagList["removeconnectorbase"] = SE_REMOVE_CONNECTOR_BASE;
	commandTagList["adddescriptor"] = SE_ADD_DESCRIPTOR;
	commandTagList["removedescriptor"] = SE_REMOVE_DESCRIPTOR;
	commandTagList["adddescriptorswitch"] = SE_ADD_DESCRIPTOR_SWITCH;
	commandTagList["removedescriptorswitch"] = SE_REMOVE_DESCRIPTOR_SWITCH;
	commandTagList["adddescriptorbase"] = SE_ADD_DESCRIPTOR_BASE;
	commandTagList["removedescriptorbase"] = SE_REMOVE_DESCRIPTOR_BASE;
	commandTagList["addtransition"] = SE_ADD_TRANSITION;
	commandTagList["removetransition"] = SE_REMOVE_TRANSITION;
	commandTagList["addtransitionbase"] = SE_ADD_TRANSITION_BASE;
	commandTagList["removetransitionbase"] = SE_REMOVE_TRANSITION_BASE;
	commandTagList["addimportbase"] = SE_ADD_IMPORT_BASE;
	commandTagList["removeimportbase"] = SE_REMOVE_IMPORT_BASE;
	commandTagList["addimporteddocumentbase"] = SE_ADD_IMPORTED_DOCUMENT_BASE;
	commandTagList["removeimporteddocumentbase"] = SE_REMOVE_IMPORTED_DOCUMENT_BASE;
	commandTagList["addimportncl"] = SE_ADD_IMPORT_NCL;
	commandTagList["removeimportncl"] = SE_REMOVE_IMPORT_NCL;
	commandTagList["addnode"] = SE_ADD_NODE;
	commandTagList["removenode"] = SE_REMOVE_NODE;
	commandTagList["addinterface"] = SE_ADD_INTERFACE;
	commandTagList["removeinterface"] = SE_REMOVE_INTERFACE;
	commandTagList["addlink"] = SE_ADD_LINK;
	commandTagList["removelink"] = SE_REMOVE_LINK;
	commandTagList["setpropertyvalue"] = SE_SET_PROPERTY_VALUE;
}

void Project::fillRegionList() {
	regionList["porto velho"] = 1;
	regionList["guajar�-mirim"] = 2;
	regionList["ariquemes"] = 3;
	regionList["ji-paran�"] = 4;
	regionList["alvorada d'oeste"] = 5;
	regionList["cacoal"] = 6;
	regionList["vilhena"] = 7;
	regionList["colorado do oeste"] = 8;
	regionList["cruzeiro do sul"] = 1;
	regionList["tarauac�"] = 2;
	regionList["sena madureira"] = 3;
	regionList["rio branco"] = 4;
	regionList["brasil�ia"] = 5;
	regionList["rio negro"] = 1;
	regionList["japur�"] = 2;
	regionList["alto solim�es"] = 3;
	regionList["juru�"] = 4;
	regionList["tef�"] = 5;
	regionList["coari"] = 6;
	regionList["manaus"] = 7;
	regionList["rio preto da eva"] = 8;
	regionList["itacoatiara"] = 9;
	regionList["parintins"] = 10;
	regionList["boca do acre"] = 11;
	regionList["purus"] = 12;
	regionList["madeira"] = 13;
	regionList["boa vista"] = 1;
	regionList["nordeste de roraima"] = 2;
	regionList["caracara�"] = 3;
	regionList["sudeste de roraima"] = 4;
	regionList["�bidos"] = 1;
	regionList["santar�m"] = 2;
	regionList["almeirim"] = 3;
	regionList["portel"] = 4;
	regionList["furos de breves"] = 5;
	regionList["arari"] = 6;
	regionList["bel�m"] = 7;
	regionList["castanhal"] = 8;
	regionList["salgado"] = 9;
	regionList["bragantina"] = 10;
	regionList["camet�"] = 11;
	regionList["tom�-a�u"] = 12;
	regionList["guam�"] = 13;
	regionList["itaituba"] = 14;
	regionList["altamira"] = 15;
	regionList["tucuru�"] = 16;
	regionList["paragominas"] = 17;
	regionList["s�o f�lix do xingu"] = 18;
	regionList["parauapebas"] = 19;
	regionList["marab�"] = 20;
	regionList["reden��o"] = 21;
	regionList["concei��o do araguaia"] = 22;
	regionList["oiapoque"] = 1;
	regionList["amap�"] = 2;
	regionList["macap�"] = 3;
	regionList["mazag�o"] = 4;
	regionList["bico do papagaio"] = 1;
	regionList["aragua�na"] = 2;
	regionList["miracema do tocantins"] = 3;
	regionList["rio formoso"] = 4;
	regionList["gurupi"] = 5;
	regionList["porto nacional"] = 6;
	regionList["jalap�o"] = 7;
	regionList["dian�polis"] = 8;
	regionList["litoral ocidental maranhense"] = 1;
	regionList["aglomera��o urbana de s�o lu�s"] = 2;
	regionList["ros�rio"] = 3;
	regionList["len�ois maranhenses"] = 4;
	regionList["baixada maranhense"] = 5;
	regionList["itapecuru mirim"] = 6;
	regionList["gurupi"] = 7;
	regionList["pindar�"] = 8;
	regionList["imperatriz"] = 9;
	regionList["m�dio mearim"] = 10;
	regionList["alto mearim e graja�"] = 11;
	regionList["presidente dutra"] = 12;
	regionList["baixo parna�ba maranhense"] = 13;
	regionList["chapadinha"] = 14;
	regionList["cod�"] = 15;
	regionList["coelho neto"] = 16;
	regionList["caxias"] = 17;
	regionList["chapadas do alto itapecuru"] = 18;
	regionList["porto franco"] = 19;
	regionList["gerais de balsas"] = 20;
	regionList["chapadas das mangabeiras"] = 21;
	regionList["baixo parna�ba piauiense"] = 1;
	regionList["litoral piauiense"] = 2;
	regionList["teresina"] = 3;
	regionList["campo maior"] = 4;
	regionList["m�dio parna�ba piauiense"] = 5;
	regionList["valen�a do piau�"] = 6;
	regionList["alto parna�ba piauiense"] = 7;
	regionList["bertol�nia"] = 8;
	regionList["floriano"] = 9;
	regionList["alto m�dio gurgu�ia"] = 10;
	regionList["s�o raimundo nonato"] = 11;
	regionList["chapadas do extremo sul piauiense"] = 12;
	regionList["picos"] = 13;
	regionList["pio ix"] = 14;
	regionList["alto m�dio canind�"] = 15;
	regionList["litoral de camocim e acara�"] = 1;
	regionList["ibiapaba"] = 2;
	regionList["corea�"] = 3;
	regionList["meruoca"] = 4;
	regionList["sobral"] = 5;
	regionList["ipu"] = 6;
	regionList["santa quit�ria"] = 7;
	regionList["itapipoca"] = 8;
	regionList["baixo curu"] = 9;
	regionList["uruburetama"] = 10;
	regionList["m�dio curu"] = 11;
	regionList["canind�"] = 12;
	regionList["baturit�"] = 13;
	regionList["chorozinho"] = 14;
	regionList["cascavel"] = 15;
	regionList["fortaleza"] = 16;
	regionList["pacajus"] = 17;
	regionList["sert�o de crat�us"] = 18;
	regionList["sert�o de quixeramobim"] = 19;
	regionList["sert�o de inhamuns"] = 20;
	regionList["sert�o de senador pompeu"] = 21;
	regionList["litoral de aracati"] = 22;
	regionList["baixo jaguaribe"] = 23;
	regionList["m�dio jaguaribe"] = 24;
	regionList["serra do pereiro"] = 25;
	regionList["iguatu"] = 26;
	regionList["v�rzea alegre"] = 27;
	regionList["lavras da mangabeira"] = 28;
	regionList["chapada do araripe"] = 29;
	regionList["cariria�u"] = 30;
	regionList["barro"] = 31;
	regionList["cariri"] = 32;
	regionList["brejo santo"] = 33;
	regionList["mossor�"] = 1;
	regionList["chapada do apodi"] = 2;
	regionList["m�dio oeste"] = 3;
	regionList["vale do a�u"] = 4;
	regionList["serra de s�o miguel"] = 5;
	regionList["pau dos ferros"] = 6;
	regionList["umarizal"] = 7;
	regionList["macau"] = 8;
	regionList["angicos"] = 9;
	regionList["serra de santana"] = 10;
	regionList["serid� ocidental"] = 11;
	regionList["serid� oriental"] = 12;
	regionList["baixa verde"] = 13;
	regionList["borborema potiguar"] = 14;
	regionList["agreste potiguar"] = 15;
	regionList["litoral nordeste"] = 16;
	regionList["maca�ba"] = 17;
	regionList["natal"] = 18;
	regionList["litoral sul"] = 19;
	regionList["catol� do rocha"] = 1;
	regionList["cajazeiras"] = 2;
	regionList["sousa"] = 3;
	regionList["patos"] = 4;
	regionList["pianc�"] = 5;
	regionList["itaporanga"] = 6;
	regionList["serra do teixeira"] = 7;
	regionList["serid� ocidental paraibano"] = 8;
	regionList["serid� oriental paraibano"] = 9;
	regionList["cariri ocidental"] = 10;
	regionList["cariri oriental"] = 11;
	regionList["curimata� ocidental"] = 12;
	regionList["curimata� oriental"] = 13;
	regionList["esperan�a"] = 14;
	regionList["brejo paraibano"] = 15;
	regionList["guarabira"] = 16;
	regionList["campina grande"] = 17;
	regionList["itabaiana"] = 18;
	regionList["umbuzeiro"] = 19;
	regionList["litoral norte"] = 20;
	regionList["sap�"] = 21;
	regionList["jo�o pessoa"] = 22;
	regionList["litoral sul"] = 23;
	regionList["araripina"] = 1;
	regionList["salgueiro"] = 2;
	regionList["paje�"] = 3;
	regionList["sert�o do moxot�"] = 4;
	regionList["petrolina"] = 5;
	regionList["itaparica"] = 6;
	regionList["vale do ipanema"] = 7;
	regionList["vale do ipojuca"] = 8;
	regionList["alto capibaribe"] = 9;
	regionList["m�dio capibaribe"] = 10;
	regionList["garanhuns"] = 11;
	regionList["brejo pernambucano"] = 12;
	regionList["mata setentrional pernambucana"] = 13;
	regionList["vit�ria de santo ant�o"] = 14;
	regionList["mata meridional pernambucana"] = 15;
	regionList["itamarac�"] = 16;
	regionList["recife"] = 17;
	regionList["suape"] = 18;
	regionList["fernando de noronha"] = 19;
	regionList["serrana do sert�o alagoano"] = 1;
	regionList["alagoana do sert�o do s�o francisco"] = 2;
	regionList["santana do ipanema"] = 3;
	regionList["batalha"] = 4;
	regionList["palmeira dos �ndios"] = 5;
	regionList["arapiraca"] = 6;
	regionList["traipu"] = 7;
	regionList["serrana dos quilombos"] = 8;
	regionList["mata alagoana"] = 9;
	regionList["litoral norte alagoano"] = 10;
	regionList["macei�"] = 11;
	regionList["s�o miguel dos campos"] = 12;
	regionList["penedo"] = 13;
	regionList["sergipana do sert�o do s�o francisco"] = 1;
	regionList["carira"] = 2;
	regionList["nossa senhora das dores"] = 3;
	regionList["agreste de itabaiana"] = 4;
	regionList["tobias barreto"] = 5;
	regionList["agreste de lagarto"] = 6;
	regionList["propri�"] = 7;
	regionList["cotinguiba"] = 8;
	regionList["japaratuba"] = 9;
	regionList["baixo cotinguiba"] = 10;
	regionList["aracaju"] = 11;
	regionList["boquim"] = 12;
	regionList["est�ncia"] = 13;
	regionList["barreiras"] = 1;
	regionList["cotegipe"] = 2;
	regionList["santa maria da vit�ria"] = 3;
	regionList["juazeiro"] = 4;
	regionList["paulo afonso"] = 5;
	regionList["barra"] = 6;
	regionList["bom jesus da lapa"] = 7;
	regionList["senhor do bonfim"] = 8;
	regionList["irec�"] = 9;
	regionList["jacobina"] = 10;
	regionList["itaberaba"] = 11;
	regionList["feira de santana"] = 12;
	regionList["jeremoabo"] = 13;
	regionList["euclides da cunha"] = 14;
	regionList["ribeira do pombal"] = 15;
	regionList["serrinha"] = 16;
	regionList["alagoinhas"] = 17;
	regionList["entre rios"] = 18;
	regionList["catu"] = 19;
	regionList["santo ant�nio de jesus"] = 20;
	regionList["salvador"] = 21;
	regionList["boquira"] = 22;
	regionList["seabra"] = 23;
	regionList["jequi�"] = 24;
	regionList["livramento do brumado"] = 25;
	regionList["guanambi"] = 26;
	regionList["brumado"] = 27;
	regionList["vit�ria da conquista"] = 28;
	regionList["itapetinga"] = 29;
	regionList["valen�a"] = 30;
	regionList["ilh�us-itabuna"] = 31;
	regionList["porto seguro"] = 32;
	regionList["una�"] = 1;
	regionList["paracatu"] = 2;
	regionList["janu�ria"] = 3;
	regionList["jana�ba"] = 4;
	regionList["salinas"] = 5;
	regionList["pirapora"] = 6;
	regionList["montes claros"] = 7;
	regionList["gr�o mogol"] = 8;
	regionList["bocai�va"] = 9;
	regionList["diamantina"] = 10;
	regionList["capelinha"] = 11;
	regionList["ara�ua�"] = 12;
	regionList["pedra azul"] = 13;
	regionList["almenara"] = 14;
	regionList["te�filo otoni"] = 15;
	regionList["nanuque"] = 16;
	regionList["ituiutaba"] = 17;
	regionList["uberl�ndia"] = 18;
	regionList["patroc�nio"] = 19;
	regionList["patos de minas"] = 20;
	regionList["frutal"] = 21;
	regionList["uberaba"] = 22;
	regionList["arax�"] = 23;
	regionList["tr�s marias"] = 24;
	regionList["curvelo"] = 25;
	regionList["bom despacho"] = 26;
	regionList["sete lagoas"] = 27;
	regionList["concei��o do mato dentro"] = 28;
	regionList["par� de minas"] = 29;
	regionList["belo horizonte"] = 30;
	regionList["itabira"] = 31;
	regionList["itaguara"] = 32;
	regionList["ouro preto"] = 33;
	regionList["conselheiro lafaiete"] = 34;
	regionList["guanh�es"] = 35;
	regionList["pe�anha"] = 36;
	regionList["governador valadares"] = 37;
	regionList["mantena"] = 38;
	regionList["ipatinga"] = 39;
	regionList["caratinga"] = 40;
	regionList["aimor�s"] = 41;
	regionList["piu�"] = 42;
	regionList["divin�polis"] = 43;
	regionList["formiga"] = 44;
	regionList["campo belo"] = 45;
	regionList["oliveira"] = 46;
	regionList["passos"] = 47;
	regionList["s�o sebasti�o do para�so"] = 48;
	regionList["alfenas"] = 49;
	regionList["varginha"] = 50;
	regionList["po�os de caldas"] = 51;
	regionList["pouso alegre"] = 52;
	regionList["santa rita do sapuca�"] = 53;
	regionList["s�o louren�o"] = 54;
	regionList["andrel�ndia"] = 55;
	regionList["itajub�"] = 56;
	regionList["lavras"] = 57;
	regionList["s�o jo�o del rei"] = 58;
	regionList["barbacena"] = 59;
	regionList["ponte nova"] = 60;
	regionList["manhua�u"] = 61;
	regionList["vi�osa"] = 62;
	regionList["muria�"] = 63;
	regionList["ub�"] = 64;
	regionList["juiz de fora"] = 65;
	regionList["cataguases"] = 66;
	regionList["barra de s�o francisco"] = 1;
	regionList["nova ven�cia"] = 2;
	regionList["colatina"] = 3;
	regionList["montanha"] = 4;
	regionList["s�o mateus"] = 5;
	regionList["linhares"] = 6;
	regionList["afonso cl�udio"] = 7;
	regionList["santa teresa"] = 8;
	regionList["vit�ria"] = 9;
	regionList["guarapari"] = 10;
	regionList["alegre"] = 11;
	regionList["cachoeiro de itapemirim"] = 12;
	regionList["itapemirim"] = 13;
	regionList["itaperuna"] = 1;
	regionList["santo ant�nio de p�dua"] = 2;
	regionList["campos dos goytacazes"] = 3;
	regionList["maca�"] = 4;
	regionList["tr�s rios"] = 5;
	regionList["cantagalo-cordeiro"] = 6;
	regionList["nova friburgo"] = 7;
	regionList["santa maria madalena"] = 8;
	regionList["bacia de s�o jo�o"] = 9;
	regionList["lagos"] = 10;
	regionList["vale do para�ba fluminense"] = 11;
	regionList["barra do pira�"] = 12;
	regionList["ba�a da ilha grande"] = 13;
	regionList["vassouras"] = 14;
	regionList["serrana"] = 15;
	regionList["macacu-caceribu"] = 16;
	regionList["itagua�"] = 17;
	regionList["rio de janeiro"] = 18;
	regionList["jales"] = 1;
	regionList["fernand�polis"] = 2;
	regionList["votuporanga"] = 3;
	regionList["s�o jos� do rio preto"] = 4;
	regionList["catanduva"] = 5;
	regionList["auriflama"] = 6;
	regionList["nhandeara"] = 7;
	regionList["novo horizonte"] = 8;
	regionList["barretos"] = 9;
	regionList["s�o joaquim da barra"] = 10;
	regionList["ituverava"] = 11;
	regionList["franca"] = 12;
	regionList["jaboticabal"] = 13;
	regionList["ribeir�o preto"] = 14;
	regionList["batatais"] = 15;
	regionList["andradina"] = 16;
	regionList["ara�atuba"] = 17;
	regionList["birigui"] = 18;
	regionList["lins"] = 19;
	regionList["bauru"] = 20;
	regionList["ja�"] = 21;
	regionList["avar�"] = 22;
	regionList["botucatu"] = 23;
	regionList["araraquara"] = 24;
	regionList["s�o carlos"] = 25;
	regionList["rio claro"] = 26;
	regionList["limeira"] = 27;
	regionList["piracicaba"] = 28;
	regionList["pirassununga"] = 29;
	regionList["s�o jo�o da boa vista"] = 30;
	regionList["moji mirim"] = 31;
	regionList["campinas"] = 32;
	regionList["amparo"] = 33;
	regionList["dracena"] = 34;
	regionList["adamantina"] = 35;
	regionList["presidente prudente"] = 36;
	regionList["tup�"] = 37;
	regionList["mar�lia"] = 38;
	regionList["assis"] = 39;
	regionList["ourinhos"] = 40;
	regionList["itapeva"] = 41;
	regionList["itapetininga"] = 42;
	regionList["tatu�"] = 43;
	regionList["cap�o bonito"] = 44;
	regionList["piedade"] = 45;
	regionList["sorocaba"] = 46;
	regionList["jundia�"] = 47;
	regionList["bragan�a paulista"] = 48;
	regionList["campos do jord�o"] = 49;
	regionList["s�o jos� dos campos"] = 50;
	regionList["guaratinguet�"] = 51;
	regionList["bananal"] = 52;
	regionList["paraibuna/paraitinga"] = 53;
	regionList["caraguatatuba"] = 54;
	regionList["registro"] = 55;
	regionList["itanha�m"] = 56;
	regionList["osasco"] = 57;
	regionList["franco da rocha"] = 58;
	regionList["guarulhos"] = 59;
	regionList["itapecerica da serra"] = 60;
	regionList["s�o paulo"] = 61;
	regionList["mogi das cruzes"] = 62;
	regionList["santos"] = 63;
	regionList["paranava�"] = 1;
	regionList["umuarama"] = 2;
	regionList["cianorte"] = 3;
	regionList["goioer�"] = 4;
	regionList["campo mour�o"] = 5;
	regionList["astorga"] = 6;
	regionList["porecatu"] = 7;
	regionList["flora�"] = 8;
	regionList["maring�"] = 9;
	regionList["apucarana"] = 10;
	regionList["londrina"] = 11;
	regionList["faxinal"] = 12;
	regionList["ivaipor�"] = 13;
	regionList["assa�"] = 14;
	regionList["corn�lio proc�pio"] = 15;
	regionList["jacarezinho"] = 16;
	regionList["ibaiti"] = 17;
	regionList["wenceslau braz"] = 18;
	regionList["tel�maco borba"] = 19;
	regionList["jaguaria�va"] = 20;
	regionList["ponta grossa"] = 21;
	regionList["toledo"] = 22;
	regionList["cascavel"] = 23;
	regionList["foz do igua�u"] = 24;
	regionList["capanema"] = 25;
	regionList["francisco beltr�o"] = 26;
	regionList["pato branco"] = 27;
	regionList["pitanga"] = 28;
	regionList["guarapuava"] = 29;
	regionList["palmas"] = 30;
	regionList["prudent�polis"] = 31;
	regionList["irati"] = 32;
	regionList["uni�o da vit�ria"] = 33;
	regionList["s�o mateus do sul"] = 34;
	regionList["cerro azul"] = 35;
	regionList["lapa"] = 36;
	regionList["curitiba"] = 37;
	regionList["paranagu�"] = 38;
	regionList["rio negro"] = 39;
	regionList["s�o miguel do oeste"] = 1;
	regionList["chapec�"] = 2;
	regionList["xanxer�"] = 3;
	regionList["joa�aba"] = 4;
	regionList["conc�rdia"] = 5;
	regionList["canoinhas"] = 6;
	regionList["s�o bento do sul"] = 7;
	regionList["joinville"] = 8;
	regionList["curitibanos"] = 9;
	regionList["campos de lages"] = 10;
	regionList["rio do sul"] = 11;
	regionList["blumenau"] = 12;
	regionList["itaja�"] = 13;
	regionList["ituporanga"] = 14;
	regionList["tijucas"] = 15;
	regionList["florian�polis"] = 16;
	regionList["tabuleiro"] = 17;
	regionList["tubar�o"] = 18;
	regionList["crici�ma"] = 19;
	regionList["ararangu�"] = 20;
	regionList["santa rosa"] = 1;
	regionList["tr�s passos"] = 2;
	regionList["frederico westphalen"] = 3;
	regionList["erechim"] = 4;
	regionList["sananduva"] = 5;
	regionList["cerro largo"] = 6;
	regionList["santo �ngelo"] = 7;
	regionList["iju�"] = 8;
	regionList["carazinho"] = 9;
	regionList["passo fundo"] = 10;
	regionList["cruz alta"] = 11;
	regionList["n�o-me-toque"] = 12;
	regionList["soledade"] = 13;
	regionList["guapor�"] = 14;
	regionList["vacaria"] = 15;
	regionList["caxias do sul"] = 16;
	regionList["santiago"] = 17;
	regionList["santa maria"] = 18;
	regionList["restinga seca"] = 19;
	regionList["santa cruz do sul"] = 20;
	regionList["lajeado-estrela"] = 21;
	regionList["cachoeira do sul"] = 22;
	regionList["montenegro"] = 23;
	regionList["gramado-canela"] = 24;
	regionList["s�o jer�nimo"] = 25;
	regionList["porto alegre"] = 26;
	regionList["os�rio"] = 27;
	regionList["camaqu�"] = 28;
	regionList["campanha ocidental"] = 29;
	regionList["campanha central"] = 30;
	regionList["campanha meridional"] = 31;
	regionList["serras de sudeste"] = 32;
	regionList["pelotas"] = 33;
	regionList["jaguar�o"] = 34;
	regionList["litoral lagunar"] = 35;
	regionList["baixo pantanal"] = 1;
	regionList["aquidauana"] = 2;
	regionList["alto taquari"] = 3;
	regionList["campo grande"] = 4;
	regionList["cassil�ndia"] = 5;
	regionList["parana�ba"] = 6;
	regionList["tr�s lagoas"] = 7;
	regionList["nova andradina"] = 8;
	regionList["bodoquena"] = 9;
	regionList["dourados"] = 10;
	regionList["iguatemi"] = 11;
	regionList["aripuan�"] = 1;
	regionList["alta floresta"] = 2;
	regionList["col�der"] = 3;
	regionList["parecis"] = 4;
	regionList["arinos"] = 5;
	regionList["alto teles pires"] = 6;
	regionList["sinop"] = 7;
	regionList["paranatinga"] = 8;
	regionList["norte araguaia"] = 9;
	regionList["canarana"] = 10;
	regionList["m�dio araguaia"] = 11;
	regionList["alto guapor�"] = 12;
	regionList["tangar� da serra"] = 13;
	regionList["jauru"] = 14;
	regionList["alto paraguai"] = 15;
	regionList["ros�rio oeste"] = 16;
	regionList["cuiab�"] = 17;
	regionList["alto pantanal"] = 18;
	regionList["primavera do leste"] = 19;
	regionList["tesouro"] = 20;
	regionList["rondon�polis"] = 21;
	regionList["alto araguaia"] = 22;
	regionList["s�o miguel do araguaia"] = 1;
	regionList["rio vermelho"] = 2;
	regionList["aragar�as"] = 3;
	regionList["porangatu"] = 4;
	regionList["chapada dos veadeiros"] = 5;
	regionList["ceres"] = 6;
	regionList["an�polis"] = 7;
	regionList["ipor�"] = 8;
	regionList["anicuns"] = 9;
	regionList["goi�nia"] = 10;
	regionList["v�o do paran�"] = 11;
	regionList["entorno de bras�lia"] = 12;
	regionList["sudoeste de goi�s"] = 13;
	regionList["vale do rio dos bois"] = 14;
	regionList["meia ponte"] = 15;
	regionList["pires do rio"] = 16;
	regionList["catal�o"] = 17;
	regionList["quirin�polis"] = 18;
	regionList["bras�lia"] = 1;
	regionList["ro"] = 1;
	regionList["ac"] = 2;
	regionList["am"] = 3;
	regionList["rr"] = 4;
	regionList["pa"] = 5;
	regionList["ap"] = 6;
	regionList["to"] = 7;
	regionList["ma"] = 8;
	regionList["pi"] = 9;
	regionList["ce"] = 10;
	regionList["rn"] = 11;
	regionList["pb"] = 12;
	regionList["pe"] = 13;
	regionList["se"] = 14;
	regionList["al"] = 15;
	regionList["ba"] = 16;
	regionList["mg"] = 17;
	regionList["es"] = 18;
	regionList["rj"] = 19;
	regionList["sp"] = 20;
	regionList["pr"] = 21;
	regionList["sc"] = 22;
	regionList["rs"] = 23;
	regionList["ms"] = 24;
	regionList["mt"] = 25;
	regionList["go"] = 26;
	regionList["df"] = 27;
}

}
}
}
}


