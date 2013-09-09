/*
 * TMM.cpp
 *
 *  Created on: 27/02/2013
 *      Author: Felippe Nagato
 */

#include "TMM.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

TMM::TMM() {
	project = NULL;
	init();
}

TMM::TMM(Project* project) {
	this->project = project;
	init();
}

TMM::~TMM() {
	if (muxer) delete muxer;
	releaseSiStreamList();
}

void TMM::releaseSiStreamList() {
	map<ProjectInfo*, SectionStream*>::iterator itSi;
	for (itSi = siStreamList.begin(); itSi != siStreamList.end(); ++itSi) {
		if (itSi->second) delete itSi->second;
	}
	siStreamList.clear();
}

void TMM::init() {
	muxer = NULL;
	destination = "";
	lastStcPrinter = 0;
}

void TMM::setProject(Project* project) {
	this->project = project;
}

int TMM::getTSInfo(InputData *inputList) {
	TSFileReader tsFile;
	TSInfo tsInfo;
	unsigned char st;
	char hd;

	tsFile.setFilename(inputList->getFilename());
	tsInfo.setTSFileReader(&tsFile);
	if (tsInfo.readInfo()) {
		st = tsInfo.getStreamType(inputList->getPid());
		if (st != 255) {
			inputList->setStreamType(st);
			hd = tsInfo.hasDts(inputList->getPid(), 60);
			if (hd == 1) inputList->setHasDts(true);
			inputList->setDuration(tsInfo.duration(inputList->getPid()));
			inputList->setFirstPts(tsInfo.getFirstPts());
			if (!inputList->getInputRangeList()->size()) {
				inputList->addRange(-1, 0, (inputList->getDuration()*1000));
			}
		} else {
			return -1;
		}
	}

	return 0;
}

bool TMM::loadProject() {
	map<int, ProjectInfo*>::iterator it;

	//read project file
	if (project->readFile()) return false;

	//get stream type of all pids
	if (project->getProjectList()) {
		it = project->getProjectList()->begin();
		while (it != project->getProjectList()->end()) {
			if (it->second && (it->second->getProjectType() == PT_INPUTDATA)) {
				if (getTSInfo((InputData*)it->second) < 0) return false;
			}
			++it;
		}
	}

	//mount carousels
	project->mountCarousels();

	return true;
}

Stream* TMM::createStream(ProjectInfo* proj) {
	PESStream* pes;
	SectionStream* sec;
	InputData* indata;
	NPTProject* nptProj;
	PCarousel* carProj;
	PAit* aitProj;
	PPat* patProj;
	PSdt* sdtProj;
	PNit* nitProj;
	PTot* totProj;
	PEit* eitProj;
	double nextSendOffset, timeOffset, bufOffset, preponeDiff = 0.0;
	int64_t currStc;
	time_t ctime;

	bufOffset = project->getVbvBuffer();
	unsigned found = destination.find("://");
	if (found != std::string::npos) {
		if (project->getVbvBuffer() <= 0.0) {
			//increasing the amount of buffer for network (seconds)
			bufOffset = 1.0;
		}
	}

	timeOffset = ((2*PREPONETICKS_VIDEO) + (bufOffset)) - PREPONETICKS_AUDIO;

	switch (proj->getProjectType()) {
	case PT_INPUTDATA:
		indata = (InputData*) proj;
		pes = new PESStream();
		pes->addPidFilter(indata->getPid());
		pes->setFilename(indata->getFilename());
		pes->setInputRangeList(indata->getInputRangeList());
		if (TSInfo::isAudioStreamType(indata->getStreamType())) {
			preponeDiff = PREPONETICKS_VIDEO - PREPONETICKS_AUDIO;
		}
		nextSendOffset = ((double) indata->getOffset() / 1000) + 0.005;
		if (nextSendOffset >= 0) {
			pes->initiateNextSend(Stc::secondToStc(preponeDiff) +
								  muxer->getCurrentStc() +
								  Stc::secondToStc(nextSendOffset));
		} else {
			nextSendOffset = nextSendOffset * -1.0;
			pes->initiateNextSend(Stc::secondToStc(preponeDiff) +
								  muxer->getCurrentStc() -
								  Stc::secondToStc(nextSendOffset));
		}
		if (TSInfo::isAudioStreamType(indata->getStreamType())) {
			pes->setPreponeTicks(Stc::secondToStc(PREPONETICKS_AUDIO + bufOffset));
			pes->setIsVideoStream(false);
		} else if (TSInfo::isVideoStreamType(indata->getStreamType())) {
			pes->setPreponeTicks(Stc::secondToStc(PREPONETICKS_VIDEO + bufOffset));
			pes->setHasDts(indata->getHasDts());
		} else {
			cout << "TMM::createStream - Undefined stream type." << endl;
			return NULL;
		}
		pes->fillBuffer();
		return pes;
	case PT_NPT:
		nptProj = (NPTProject*) proj;
		nptProj->setFirstReference(muxer->getCurrentStc() +
				Stc::secondToStc(timeOffset + nptProj->getFirstReferenceOffset()));
		sec = new SectionStream();
		sec->attach(nptProj);
		sec->setFrequency(Stc::secondToStc(1.0));
		sec->initiateNextSend(muxer->getCurrentStc() +
				Stc::secondToStc(nptProj->getTransmissionDelay()));
		sec->fillBuffer();
		return sec;
	case PT_CAROUSEL:
		carProj = (PCarousel*) proj;
		sec = new SectionStream();
		sec->addSection(carProj->getOutputFile());
		sec->setMaxBitrate(carProj->getBitrate());
		sec->setFrequency(
		   Stc::secondToStc(1.0/(((double) carProj->getBitrate()/8.0)/4182.0)));
		sec->initiateNextSend(muxer->getCurrentStc() +
				Stc::secondToStc(carProj->getTransmissionDelay()) +
				Stc::secondToStc(0.005));
		sec->fillBuffer();
		return sec;
	case PT_AIT:
		aitProj = (PAit*) proj;
		aitProj->setVersionNumber(aitProj->getVersion());
		sec = new SectionStream();
		aitProj->updateStream();
		sec->addSection(aitProj);
		sec->setDestroySections(false);
		sec->setFrequency(Stc::secondToStc(0.2));
		sec->initiateNextSend(muxer->getCurrentStc() +
				Stc::secondToStc(aitProj->getTransmissionDelay()));
		sec->fillBuffer();
		return sec;
	case PT_PAT:
		patProj = (PPat*) proj;
		patProj->setVersionNumber(patProj->getVersion());
		sec = new SectionStream();
		patProj->updateStream();
		sec->addSection(patProj);
		sec->setDestroySections(false);
		sec->setFrequency(Stc::secondToStc(0.1));
		sec->initiateNextSend(muxer->getCurrentStc());
		sec->fillBuffer();
		return sec;
	case PT_SDT:
		sdtProj = (PSdt*) proj;
		sdtProj->setVersionNumber(sdtProj->getVersion());
		sec = new SectionStream();
		sdtProj->updateStream();
		sec->addSection(sdtProj);
		sec->setDestroySections(false);
		sec->setFrequency(Stc::secondToStc(1.0));
		sec->initiateNextSend(muxer->getCurrentStc());
		sec->fillBuffer();
		return sec;
	case PT_NIT:
		nitProj = (PNit*) proj;
		nitProj->setVersionNumber(nitProj->getVersion());
		sec = new SectionStream();
		nitProj->updateStream();
		sec->addSection(nitProj);
		sec->setDestroySections(false);
		sec->setFrequency(Stc::secondToStc(1.0));
		sec->initiateNextSend(muxer->getCurrentStc());
		sec->fillBuffer();
		return sec;
	case PT_TOT:
		totProj = (PTot*) proj;
		totProj->setOffset((int) (-timeOffset - 0.5f));
		totProj->setStcBegin(muxer->getStcBegin());
		sec = new SectionStream();
		sec->attach(totProj);
		sec->setDestroySections(false);
		sec->setFrequency(Stc::secondToStc(5.0));
		sec->initiateNextSend(muxer->getCurrentStc());
		sec->fillBuffer();
		return sec;
	case PT_EIT_PF:
		eitProj = (PEit*) proj;
		currStc = muxer->getCurrentStc();
		eitProj->setStcBegin(currStc);
		totProj = (PTot*) getFirstProject(PT_TOT);
		if (totProj) {
			((PTot*)totProj)->updateDateTime(currStc);
			eitProj->setTimeBegin(((PTot*)totProj)->getDateTime() -
									((PTot*)totProj)->getOffset());
			eitProj->adjustUtcOffset(((PTot*)totProj)->getUtcOffset());
		} else {
			time(&ctime);
			eitProj->setTimeBegin(ctime + (-timeOffset - 0.5f));
			eitProj->adjustUtcOffset(Tot::localTimezone()*60);
		}
		sec = new SectionStream();
		sec->attach(eitProj);
		sec->setDestroySections(false);
		sec->setFrequency(Stc::secondToStc(2.0));
		sec->initiateNextSend(muxer->getCurrentStc());
		sec->fillBuffer();
		return sec;
	default:
		cout << "TMM::multiplex - Stream type not recognized." << endl;
		return NULL;
	}
}

//Notice: This function can make you get lost for sure.
bool TMM::createStreamList(vector<pmtViewInfo*>* currentTimeline,
						   vector<pmtViewInfo*>* newTimeline) {
	ProjectInfo* proj;
	vector<pmtViewInfo*>::iterator itPmtNew;
	map<unsigned short, ProjectInfo*>::iterator itProjCurr;
	map<unsigned short, ProjectInfo*>::iterator itProjNew;
	map<unsigned short, Stream*>::iterator itStreamCurr;
	bool canReuse;
	Stream* stream;

	if (!newTimeline) return false;

	itPmtNew = newTimeline->begin();
	while (itPmtNew != newTimeline->end()) {

		if (((*itPmtNew)->priorPmtId >= 0) && (currentTimeline)) {
			//Reuse ES from last timeline
			if ((*itPmtNew)->pv->getId() != (*itPmtNew)->priorPmtId) {
				(*itPmtNew)->pv->deleteAllStreams();
				itProjNew = (*itPmtNew)->pv->getProjectInfoList()->begin();
				while (itProjNew != (*itPmtNew)->pv->getProjectInfoList()->end()) {
					canReuse = false;
					proj = project->findProject((*itPmtNew)->priorPmtId);
					if (proj) {
						itProjCurr = ((PMTView*)proj)->getProjectInfoList()->begin();
						while (itProjCurr != ((PMTView*)proj)->getProjectInfoList()->end()) {
							if ((itProjCurr->second->getId() == itProjNew->second->getId()) &&
									itProjCurr->second->getProjectType() == itProjNew->second->getProjectType()) {
								itStreamCurr = ((PMTView*)proj)->getStreamList()->find(itProjCurr->first);
								if (itStreamCurr != ((PMTView*)proj)->getStreamList()->end()) {
									if (itStreamCurr->second) {
										//Stream can be reused
										canReuse = true;
										(*itPmtNew)->pv->addStream(itProjNew->first, itStreamCurr->second);
										itStreamCurr->second = NULL;
									} else {
										cout << "TMM::createStreamList - Error reusing stream pid = " <<
												itProjCurr->first << endl;
									}
								} else {
									cout << "TMM::createStreamList - Error reusing stream pid = " <<
											itProjCurr->first << endl;
								}
								break;
							}
							if ((itProjNew->second->getProjectType() == PT_AIT) &&
								(itProjCurr->second->getProjectType() == PT_AIT) &&
								(!canReuse)) {
								if ((*itPmtNew)->pv->getProjectPid(itProjNew->second) ==
									((PMTView*)proj)->getProjectPid(itProjCurr->second)) {
									//AIT version must be updated, because there is a previous one.
									itProjNew->second->setVersion(itProjCurr->second->getVersion() + 1);
									project->configAitService(itProjNew->second,
											(*itPmtNew)->pv->getProgramNumber());
									//The stream will be created below.
								}
							}
							++itProjCurr;
						}
					}
					if (!canReuse) {
						//Unable to reuse. Create a new ES for the new timeline
						if (itProjNew->second->getProjectType() == PT_AIT) {
							project->configAitService(itProjNew->second,
									(*itPmtNew)->pv->getProgramNumber());
						}
						stream = createStream(itProjNew->second);
						if (stream) {
							(*itPmtNew)->pv->addStream(itProjNew->first, stream);
						} else {
							cout << "TMM::createStreamList - Error creating stream pid = " <<
									itProjNew->first << endl;
						}
					}
					++itProjNew;
				}
			}
		} else {
			(*itPmtNew)->pv->deleteAllStreams();
			//Create a new ES for the new timeline
			itProjNew = (*itPmtNew)->pv->getProjectInfoList()->begin();
			while (itProjNew != (*itPmtNew)->pv->getProjectInfoList()->end()) {
				if (itProjNew->second->getProjectType() == PT_AIT) {
					project->configAitService(itProjNew->second,
							(*itPmtNew)->pv->getProgramNumber());
				}
				stream = createStream(itProjNew->second);
				if (stream) {
					(*itPmtNew)->pv->addStream(itProjNew->first, stream);
				} else {
					cout << "TMM::createStreamList - Error creating stream pid = " <<
									itProjNew->first << endl;
				}
				++itProjNew;
			}
		}
		++itPmtNew;
	}

	return true;
}

int TMM::multiplexSetup() {
	if (muxer) delete muxer;
	muxer = new Muxer(project->getPacketsInBuffer());
	muxer->setTTL(project->getTTL());

	muxer->setDestination(destination);
	muxer->setTsBitrate(project->getTsBitrate());

	//add all pcr frequencies used in project
	map<int, ProjectInfo*>::iterator itPvl = project->getProjectList()->begin();
	while (itPvl != project->getProjectList()->end()) {
		if (itPvl->second->getProjectType() == PT_PMTVIEW) {
			muxer->addToListOfAllPossiblePcrsFrequencies(
					((PMTView*)(itPvl->second))->getPcrFrequency());
		}
		++itPvl;
	}

	if (!muxer->prepareMultiplexer(project->getStcBegin())) {
		return -1;
	}
	//if there is any command between the previous one and the return,
	//you must add the following command before start the muxer:
	//muxer->setStcReference(project->getStcBegin());

	return 0;
}

int TMM::multiplex() {
	vector<pmtViewInfo*>* currTimeline = NULL;
	vector<pmtViewInfo*>* newTimeline;
	vector<pmtViewInfo*>::iterator itPmt;
	map<unsigned short, Stream*>::iterator itStream;
	ProjectInfo* proj;
	int ret = 0, condRet;
	time_t now;

	if (multiplexSetup() < 0) return -1;

	while (!ret) { //muxer starts here
		proj = getFirstProjectReversed(PT_TIMELINE);
		if (proj) {
			newTimeline = ((Timeline*)proj)->currTimeline(muxer->getRelativeStc(), &condRet);
		} else {
			cout << "TMM::multiplex - No timeline available in project." << endl;
			return -1;
		}

		if ((currTimeline != newTimeline) || (condRet == 2)) {
			//begin to switch elementary streams
			createStreamList(currTimeline, newTimeline);
			muxer->removeAllElementaryStreams();
			itPmt = newTimeline->begin();
			while (itPmt != newTimeline->end()) {
				itStream = (*itPmt)->pv->getStreamList()->begin();
				while (itStream != (*itPmt)->pv->getStreamList()->end()) {
					muxer->addElementaryStream(itStream->first, itStream->second);
					++itStream;
				}
				++itPmt;
			}
			processPcrsInUse(newTimeline);
			//end of switch elementary streams

			if (currTimeline) {
				//update pat, pmts and other si tables
				restoreSiTables(currTimeline, newTimeline);

			} else {
				//first time (create pat, pmts and other si tables)
				if (createSiTables(newTimeline) == -1) {
					return -2;
				}
			}
			currTimeline = newTimeline;
		}

		ret = muxer->mainLoop();

		if (!project->getUseTot()) {
			if (lastStcPrinter != (unsigned int) Stc::stcToSecond(muxer->getCurrentStc())/5) {
				lastStcPrinter = Stc::stcToSecond(muxer->getCurrentStc())/5;
				time(&now);
				PTot::printDateTime(now, "", true);
				cout << " ~ elapsed time = " <<
					(int64_t) Stc::stcToSecond(muxer->getRelativeStc()) << endl;
			}
		}
	}

	return ret;
}

bool TMM::releaseStreamFromList(ProjectInfo* proj) {
	map<ProjectInfo*, SectionStream*>::iterator itSi;
	itSi = siStreamList.find(proj);
	if (itSi != siStreamList.find(proj)) {
		if (itSi->second) {
			delete itSi->second;
			siStreamList.erase(itSi);
			return true;
		}
	}
	return false;
}

void TMM::processPcrsInUse(vector<pmtViewInfo*>* newTimeline) {
	map<unsigned short, unsigned short>::iterator itPcrsInMuxer;
	vector<pmtViewInfo*>::iterator itPmt;

	itPcrsInMuxer = muxer->getPcrList()->begin();
	while (itPcrsInMuxer != muxer->getPcrList()->end()) {
		bool found = false;
		itPmt = newTimeline->begin();
		while (itPmt != newTimeline->end()) {
			if ((*itPmt)->pv->getPcrPid() == itPcrsInMuxer->first) {
				found = true;
				break;
			}
			++itPmt;
		}
		if (!found) {
			muxer->removePcrPid(itPcrsInMuxer->first);
			itPcrsInMuxer = muxer->getPcrList()->begin();
			continue;
		}
		++itPcrsInMuxer;
	}
	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		muxer->addPcrPid((*itPmt)->pv->getPcrPid(), (*itPmt)->pv->getPcrFrequency());
		++itPmt;
	}
}

int TMM::createPmt(PMTView* currentPmtView, PMTView* newPmtView, Pmt** pmt) {
	map<unsigned short, ProjectInfo*>* pi;
	map<unsigned short, ProjectInfo*>::iterator itPi;
	InputData* indata = NULL;
	unsigned char st;
	CarouselIdentifier* cidesc = NULL;
	StreamIdentifier* sidesc = NULL;
	char* descStream;
	unsigned char descLen, ctag;

	(*pmt) = new Pmt();
	(*pmt)->setCurrentNextIndicator(1);
	(*pmt)->setSectionSyntaxIndicator(1);
	(*pmt)->setVersionNumber(0);
	(*pmt)->setPCRPid(newPmtView->getPcrPid());
	(*pmt)->setProgramNumber(newPmtView->getProgramNumber());
	pi = newPmtView->getProjectInfoList();
	itPi = pi->begin();
	while (itPi != pi->end()) {
		switch (itPi->second->getProjectType()) {
		case PT_INPUTDATA:
			indata = (InputData*) itPi->second;
			st = indata->getStreamType();
			break;
		case PT_NPT:
			st = (*pmt)->STREAM_TYPE_DSMCC_TYPE_C;
			break;
		case PT_CAROUSEL:
			st = (*pmt)->STREAM_TYPE_DSMCC_TYPE_B;
			break;
		case PT_AIT:
			st = (*pmt)->STREAM_TYPE_PRIVATE_SECTION;
			break;
		default:
			cout <<
				"TMM::multiplex - Stream type not recognized." <<
				endl;
			delete (*pmt);
			return -1;
		}

		(*pmt)->addEs(st, itPi->first);

		switch (itPi->second->getProjectType()) {
		case PT_CAROUSEL:
			cidesc = new CarouselIdentifier();
			cidesc->setCarouselId(((PCarousel*)itPi->second)->getServiceDomain());
			descLen = cidesc->getStream(&descStream);
			if (descLen) {
				(*pmt)->addEsDescriptor(itPi->first, descStream, descLen);
			}
			delete cidesc;
			break;
		}

		//Automatic creation/update of componentTagList, however a declared
		//component tag in project has always the priority.
		newPmtView->fulfillComponentTagList(currentPmtView);

		if (newPmtView->getComponentTag(itPi->first, &ctag)) {
			sidesc = new StreamIdentifier();
			sidesc->setComponentTag(ctag);
			descLen = sidesc->getStream(&descStream);
			if (descLen) {
				(*pmt)->addEsDescriptor(itPi->first, descStream, descLen);
			}
		}

		++itPi;
	}

	return 0;
}

//This method must be called just once.
int TMM::createSiTables(vector<pmtViewInfo*>* newTimeline) {
	SectionStream *sec;
	Pmt* pmt = NULL;
	ProjectInfo* proj;
	vector<pmtViewInfo*>::iterator itPmt;
	int ret;

	//Program Association Table & Program Map Table begin
	proj = getFirstProject(PT_PAT); //always exists, because it's hard coded.
	releaseStreamFromList(proj);
	((PPat*)proj)->releasePmtList();
	((PPat*)proj)->setCurrentNextIndicator(1);
	((PPat*)proj)->setVersionNumber(0);
	((PPat*)proj)->setSectionSyntaxIndicator(1);
	((PPat*)proj)->setTableIdExtension(project->getTsid());
	if (project->getUseNit()) ((PPat*)proj)->addPmt(0, 0x10);
	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		((PPat*)proj)->addPmt((*itPmt)->pv->getProgramNumber(),
								(*itPmt)->pv->getPid());
		ret = createPmt(NULL, (*itPmt)->pv, &pmt);
		if (ret < 0) return -1;
		sec = new SectionStream();
		pmt->updateStream();
		sec->addSection(pmt);
		sec->fillBuffer();
		sec->setFrequency(Stc::secondToStc(0.1));
		sec->initiateNextSend(muxer->getCurrentStc() + 1);
		(*itPmt)->pv->setPmtStream(sec);
		muxer->addElementaryStream((*itPmt)->pv->getPid(), sec);
		++itPmt;
	}
	siStreamList[proj] = (SectionStream*) createStream(proj);
	muxer->addElementaryStream(0x00, siStreamList[proj]);
	//Program Association Table & Program Map Table end

	//Time Offset Table begins
	proj = getFirstProject(PT_TOT);
	releaseStreamFromList(proj);
	if (proj) {
		siStreamList[proj] = (SectionStream*) createStream(proj);
		muxer->addElementaryStream(0x14, siStreamList[proj]);
	}

	//Time Offset Table ends

	//Service Descriptor Table begins
	if (project->getUseSdt()) {
		proj = getFirstProject(PT_SDT);
		if (proj) {
			releaseStreamFromList(proj);
			ret = project->configSdt(newTimeline, proj);
			if (ret < 0) {
				return -2;
			}
			siStreamList[proj] = (SectionStream*) createStream(proj);
			muxer->addElementaryStream(0x11, siStreamList[proj]);
		}
	}
	//Service Descriptor Table ends


	//Network Information Table begins
	if (project->getUseNit()) {
		proj = getFirstProject(PT_NIT);
		if (proj) {
			releaseStreamFromList(proj);
			ret = project->configNit(newTimeline, proj);
			if (ret < 0) {
				return -2;
			}
			siStreamList[proj] = (SectionStream*) createStream(proj);
			muxer->addElementaryStream(0x10, siStreamList[proj]);
		}
	}
	//Network Information Table ends

	//Event Information Table (present/following) begins
	itPmt = newTimeline->begin();
	while (itPmt != newTimeline->end()) {
		if ((*itPmt)->pv->getEitProj()) {
			proj = (PEit*)(*itPmt)->pv->getEitProj();
			((PEit*)proj)->setTableIdExtension((*itPmt)->pv->getProgramNumber());
			((PEit*)proj)->setTransportStreamId(project->getTsid());
			((PEit*)proj)->setOriginalNetworkId(project->getOriginalNetworkId());
			releaseStreamFromList(proj);
			siStreamList[proj] = (SectionStream*) createStream(proj);
			muxer->addElementaryStream(0x12, siStreamList[proj]);
		}
		++itPmt;
	}
	//Event Information Table (present/following) ends

	return 0;
}

int TMM::restoreSiTables(vector<pmtViewInfo*>* currentTimeline,
					vector<pmtViewInfo*>* newTimeline) {
	vector<pmtViewInfo*>::iterator itPmtCurr;
	vector<pmtViewInfo*>::iterator itPmtNew;
	map<int, ProjectInfo*>::iterator itProj;
	map<ProjectInfo*, SectionStream*>::iterator itSi;
	ProjectInfo* proj;
	SectionStream *sec;
	Pmt* pmt = NULL;
	bool found = false;
	bool patChanged = false;
	int ret;

	//PAT
	if (newTimeline->size() == currentTimeline->size()) {
		itPmtNew = newTimeline->begin();
		while (itPmtNew != newTimeline->end()) {
			found = false;
			itPmtCurr = currentTimeline->begin();
			while (itPmtCurr != currentTimeline->end()) {
				if ((*itPmtNew)->pv->getPid() == (*itPmtCurr)->pv->getPid()) {
					found = true;
				}
				++itPmtCurr;
			}
			if (!found) break;
			++itPmtNew;
		}
	}
	proj = getFirstProject(PT_PAT);
	if (!found) {
		//PAT must be updated.
		itSi = siStreamList.find(proj);
		if (itSi != siStreamList.end()) {
			((PPat*)proj)->releasePmtList();
			((PPat*)proj)->setVersion(((PPat*)proj)->getVersion() + 1);
			((PPat*)proj)->setVersionNumber(((PPat*)proj)->getVersion());
			((PPat*)proj)->setTableIdExtension(project->getTsid());
			itPmtNew = newTimeline->begin();
			while (itPmtNew != newTimeline->end()) {
				((PPat*)proj)->addPmt((*itPmtNew)->pv->getProgramNumber(),
										(*itPmtNew)->pv->getPid());
				++itPmtNew;
			}
			itSi->second->releaseSectionList();
			itSi->second->releaseBufferList();
			((PPat*)proj)->updateStream();
			itSi->second->addSection((PPat*)proj);
			itSi->second->fillBuffer();
			muxer->addElementaryStream(0x00, siStreamList[proj]);
		}
		patChanged = true;
	} else {
		//add the previous one
		muxer->addElementaryStream(0x00, siStreamList[proj]);
	}

	//PMTs ('proj' still in use bellow)
	found = false;
	itPmtNew = newTimeline->begin();
	while (itPmtNew != newTimeline->end()) {
		itPmtCurr = currentTimeline->begin();
		while (itPmtCurr != currentTimeline->end()) {
			if (((*itPmtNew)->pv->getPid() == (*itPmtCurr)->pv->getPid()) && // Is it necessary?
					((*itPmtNew)->pv->getId() == (*itPmtCurr)->pv->getId())) {
				found = true; //Identical
				break;
			}
			++itPmtCurr;
		}
		if (!found) {
			itPmtCurr = currentTimeline->begin();
			while (itPmtCurr != currentTimeline->end()) {
				if ((*itPmtNew)->pv->getPid() == (*itPmtCurr)->pv->getPid()) {
					found = true;
					break;
				}
				++itPmtCurr;
			}
			if (!found) {
				//create a new one
				ret = createPmt(NULL, (*itPmtNew)->pv, &pmt);
				if (ret < 0) return -1;
				sec = new SectionStream();
				pmt->updateStream();
				sec->addSection(pmt);
				sec->fillBuffer();
				sec->setFrequency(Stc::secondToStc(0.1));
				sec->initiateNextSend(siStreamList[proj]->getNextSend() + 1);
				(*itPmtNew)->pv->setPmtStream(sec);
				muxer->addElementaryStream((*itPmtNew)->pv->getPid(), sec);
			} else {
				//update it
				(*itPmtNew)->pv->setVersion((*itPmtCurr)->pv->getVersion() + 1);
				ret = createPmt((*itPmtCurr)->pv, (*itPmtNew)->pv, &pmt);
				if (ret < 0) return -1;
				pmt->setVersionNumber((*itPmtNew)->pv->getVersion());
				sec = (SectionStream*) (*itPmtCurr)->pv->getPmtStream();
				if (!sec) {
					sec = new SectionStream();
				} else {
					(*itPmtCurr)->pv->setPmtStream(NULL); //Is it necessary?
					sec->releaseSectionList();
					sec->releaseBufferList();
				}
				pmt->updateStream();
				sec->addSection(pmt);
				sec->fillBuffer();
				sec->setFrequency(Stc::secondToStc(0.1));
				sec->initiateNextSend(siStreamList[proj]->getNextSend() + 1);
				(*itPmtNew)->pv->setPmtStream(sec);
				muxer->addElementaryStream((*itPmtCurr)->pv->getPid(), sec);
			}
		} else {
			//it's the same PMT
			(*itPmtNew)->pv->setVersion((*itPmtCurr)->pv->getVersion());
			muxer->addElementaryStream((*itPmtCurr)->pv->getPid(),
					(*itPmtCurr)->pv->getPmtStream());
		}
		++itPmtNew;
		found = false;
	}

	//Service Descriptor Table begins
	if (project->getUseSdt()) {
		proj = getFirstProject(PT_SDT);
		if (proj) {
			if (patChanged) {
				((PSdt*)proj)->setVersion(((PSdt*)proj)->getVersion() + 1);
				ret = project->configSdt(newTimeline, proj);
				if (ret < 0) {
					return -2;
				}
				siStreamList[proj]->releaseSectionList();
				siStreamList[proj]->releaseBufferList();
				((PSdt*)proj)->updateStream();
				siStreamList[proj]->addSection((PSdt*)proj);
				siStreamList[proj]->fillBuffer();
			}
			muxer->addElementaryStream(0x11, siStreamList[proj]);
		}
	}
	//Service Descriptor Table ends

	//Network Information Table begins
	if (project->getUseNit()) {
		proj = getFirstProject(PT_NIT);
		if (proj) {
			if (patChanged) {
				((PNit*)proj)->setVersion(((PNit*)proj)->getVersion() + 1);
				ret = project->configNit(newTimeline, proj);
				if (ret < 0) {
					return -2;
				}
				siStreamList[proj]->releaseSectionList();
				siStreamList[proj]->releaseBufferList();
				((PNit*)proj)->updateStream();
				siStreamList[proj]->addSection((PNit*)proj);
				siStreamList[proj]->fillBuffer();
			}
			muxer->addElementaryStream(0x10, siStreamList[proj]);
		}
	}
	//Network Information Table ends

	//Time Offset Table begins

	proj = getFirstProject(PT_TOT);
	if (project->getUseTot() && proj) {
		itSi = siStreamList.find(proj);
		if (itSi != siStreamList.end()) {
			muxer->addElementaryStream(0x14, siStreamList[proj]);
		}
	}
	//Time Offset Table ends

	//Event Information Table (present/following) begins
	itPmtNew = newTimeline->begin();
	while (itPmtNew != newTimeline->end()) {
		if ((*itPmtNew)->pv->getEitProj()) {
			found = false;
			if ((*itPmtNew)->priorPmtId >= 0) {
				itProj = project->getProjectList()->find((*itPmtNew)->priorPmtId);
				if (itProj != project->getProjectList()->end()) {
					if ((*itPmtNew)->pv->getEitProj() !=
						((PMTView*)(itProj->second))->getEitProj()) {
						found = true;
					}
				}
			}
			proj = (PEit*)(*itPmtNew)->pv->getEitProj();
			if ((((PEit*)proj)->getTableIdExtension() !=
				(*itPmtNew)->pv->getProgramNumber()) ||
					found) {
				((PEit*)proj)->setVersionNumber(((PEit*)proj)->getVersionNumber() + 1);
				((PEit*)proj)->setTableIdExtension((*itPmtNew)->pv->getProgramNumber());
				((PEit*)proj)->setTransportStreamId(project->getTsid());
				((PEit*)proj)->setOriginalNetworkId(project->getOriginalNetworkId());
				found = true;
			}

			itSi = siStreamList.find(proj);
			if (itSi == siStreamList.end()) {
				siStreamList[proj] = (SectionStream*) createStream(proj);
			} else {
				if (found) {
					itSi->second->releaseSectionList();
					itSi->second->releaseBufferList();
					itSi->second->fillBuffer();
				}
			}
			muxer->addElementaryStream(0x12, siStreamList[proj]);
		}
		++itPmtNew;
	}
	//Event Information Table (present/following) ends

	return 0;
}

ProjectInfo* TMM::getFirstProject(char projectType) {
	map<int, ProjectInfo*>::iterator itProj = project->getProjectList()->begin();
	while (itProj != project->getProjectList()->end()) {
		if (itProj->second->getProjectType() == projectType) {
			return itProj->second;
		}
		++itProj;
	}
	return NULL;
}

ProjectInfo* TMM::getFirstProjectReversed(char projectType) {
	map<int, ProjectInfo*>::reverse_iterator itProj = project->getProjectList()->rbegin();
	while (itProj != project->getProjectList()->rend()) {
		if (itProj->second->getProjectType() == projectType) {
			return itProj->second;
		}
		++itProj;
	}
	return NULL;
}

int TMM::sendTo(const char* destination) {
	int ret;

	if (!project) return -1;

	if (!loadProject()) {
		cout << "TMM::sendTo - Error opening project." << endl;
		return -1;
	}

	if (destination) this->destination.assign(destination); else
		this->destination = project->getDestination();

	ret = multiplex();
	if (ret == -1) {
		cout << "TMM::sendTo - Unable to find a stream to multiplex." << endl;
	}
	if (ret < 0) return -2;

	return 0;
}

}
}
}
}


