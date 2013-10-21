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
	map<ProjectInfo*, Stream*>::iterator itSi;
	for (itSi = siAndIsdbtStreamList.begin(); itSi != siAndIsdbtStreamList.end(); ++itSi) {
		if (itSi->second) delete itSi->second;
	}
	siAndIsdbtStreamList.clear();
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

RawStream* TMM::prepareNewRawStream(ProjectInfo* proj, int64_t freq,
									int64_t nextSend, bool destroyBlocks) {
	RawStream* rawstream = new RawStream();
	rawstream->setDestroyBlocks(destroyBlocks);
	rawstream->setFrequency(freq);
	rawstream->initiateNextSend(nextSend);
	return rawstream;
}

Stream* TMM::createStream(ProjectInfo* proj) {
	PESStream* pes;
	RawStream* rawstream;
	InputData* indata;
	NPTProject* nptProj;
	PCarousel* carProj;
	PAit* aitProj;
	PPat* patProj;
	PSdt* sdtProj;
	PNit* nitProj;
	PTot* totProj;
	PEit* eitProj;
	PIIP* iipProj;
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
		rawstream = prepareNewRawStream(proj, Stc::secondToStc(1.0),
			muxer->getCurrentStc() + Stc::secondToStc(nptProj->getTransmissionDelay()),
			true);
		rawstream->attach(nptProj);
		return rawstream;
	case PT_CAROUSEL:
		carProj = (PCarousel*) proj;
		rawstream = new RawStream();
		rawstream->addSection(carProj->getOutputFile());
		rawstream->setMaxBitrate(carProj->getBitrate()); //must be set before initiateNextSend
		rawstream->setFrequency(
		   Stc::secondToStc(1.0/(((double) carProj->getBitrate()/8.0)/4182.0)));
		rawstream->initiateNextSend(muxer->getCurrentStc() +
				Stc::secondToStc(carProj->getTransmissionDelay()) +
				Stc::secondToStc(0.005));
		rawstream->fillBuffer();
		return rawstream;
	case PT_AIT:
		aitProj = (PAit*) proj;
		aitProj->setVersionNumber(aitProj->getVersion());
		aitProj->updateStream();
		rawstream = prepareNewRawStream(proj, Stc::secondToStc(0.2),
			muxer->getCurrentStc() + Stc::secondToStc(aitProj->getTransmissionDelay()),
			true);
		rawstream->addSection(aitProj);
		rawstream->fillBuffer();
		return rawstream;
	case PT_PAT:
		patProj = (PPat*) proj;
		patProj->setVersionNumber(patProj->getVersion());
		patProj->updateStream();
		rawstream = prepareNewRawStream(proj, Stc::secondToStc(0.1),
			muxer->getCurrentStc(), true);
		rawstream->addSection(patProj);
		rawstream->fillBuffer();
		return rawstream;
	case PT_SDT:
		sdtProj = (PSdt*) proj;
		sdtProj->setVersionNumber(sdtProj->getVersion());
		sdtProj->updateStream();
		rawstream = prepareNewRawStream(proj, Stc::secondToStc(1.0),
			muxer->getCurrentStc(), true);
		rawstream->addSection(sdtProj);
		rawstream->fillBuffer();
		return rawstream;
	case PT_NIT:
		nitProj = (PNit*) proj;
		nitProj->setVersionNumber(nitProj->getVersion());
		nitProj->updateStream();
		rawstream = prepareNewRawStream(proj, Stc::secondToStc(1.0),
			muxer->getCurrentStc(), true);
		rawstream->addSection(nitProj);
		rawstream->fillBuffer();
		return rawstream;
	case PT_TOT:
		totProj = (PTot*) proj;
		totProj->setOffset((int) (-timeOffset - 0.5f));
		totProj->setStcBegin(muxer->getStcBegin());
		rawstream = prepareNewRawStream(proj, Stc::secondToStc(5.0),
			muxer->getCurrentStc(), false);
		rawstream->attach(totProj);
		return rawstream;
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
		rawstream = prepareNewRawStream(proj, Stc::secondToStc(2.0),
			muxer->getCurrentStc(), true);
		rawstream->attach(eitProj);
		return rawstream;
	case PT_IIP:
		iipProj = (PIIP*) proj;
		rawstream = prepareNewRawStream(proj, Stc::secondToStc(0.237),
			muxer->getCurrentStc(), false);
		rawstream->setType(0); //IIP
		rawstream->attach(iipProj);
		return rawstream;
	default:
		cout << "TMM::multiplex - Stream type not recognized." << endl;
		return NULL;
	}
}

//Automatic creation/update of componentTagList, however a declared
//component tag in project has always the priority.
bool TMM::updateComponentTagList(vector<pmtViewInfo*>* currentTimeline,
				vector<pmtViewInfo*>* newTimeline) {
	vector<pmtViewInfo*>::iterator itPmtCurr;
	vector<pmtViewInfo*>::iterator itPmtNew;
	bool found;

	found = false;
	itPmtNew = newTimeline->begin();
	while (itPmtNew != newTimeline->end()) {
		if (currentTimeline) {
			itPmtCurr = currentTimeline->begin();
			while (itPmtCurr != currentTimeline->end()) {
				if (((*itPmtNew)->pv->getPid() == (*itPmtCurr)->pv->getPid()) && // Is it necessary?
						((*itPmtNew)->pv->getId() == (*itPmtCurr)->pv->getId())) {
					found = true; //Identical
					break;
				}
				++itPmtCurr;
			}
		}
		if (!found) {
			if (currentTimeline) {
				itPmtCurr = currentTimeline->begin();
				while (itPmtCurr != currentTimeline->end()) {
					if ((*itPmtNew)->pv->getPid() == (*itPmtCurr)->pv->getPid()) {
						found = true;
						break;
					}
					++itPmtCurr;
				}
			}
			if (!found) {
				//create a new one
				(*itPmtNew)->pv->fulfillComponentTagList(NULL);
			} else {
				//update it
				(*itPmtNew)->pv->fulfillComponentTagList((*itPmtCurr)->pv);
			}
		}
		++itPmtNew;
		found = false;
	}

	return true;
}

bool TMM::getCarouselComponentTagFromService(PMTView* pv, ProjectInfo* carouselProj,
		unsigned char* ctag) {
	int pid;

	if (pv->getServiceType() == SRV_TYPE_TV) {
		*ctag = 0x40;
	} else {
		*ctag = 0x80;
	}

	if (carouselProj->getProjectType() != PT_CAROUSEL) return false;

	pid = pv->getProjectPid(carouselProj);
	if (pid > 0) {
		if (!pv->getComponentTag(pid, ctag)) {
			cout << "TMM::getCarouselComponentTagFromService - Warning: AIT component tag undefined." << endl;
		} else return true;
	} else {
		cout << "TMM::getCarouselComponentTagFromService - Warning: AIT presence without carousel." << endl;
	}

	return false;
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
	unsigned char ctag;

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
									getCarouselComponentTagFromService((*itPmtNew)->pv,
											((PAit*)itProjNew->second)->getCarouselProj(),
											&ctag);
									project->configAitService(itProjNew->second,
											(*itPmtNew)->pv->getProgramNumber(),
											ctag);
									//The stream will be created below.
								}
							}
							++itProjCurr;
						}
					}
					if (!canReuse) {
						//Unable to reuse. Create a new ES for the new timeline
						if (itProjNew->second->getProjectType() == PT_AIT) {
							getCarouselComponentTagFromService((*itPmtNew)->pv,
									((PAit*)itProjNew->second)->getCarouselProj(),
									&ctag);
							project->configAitService(itProjNew->second,
									(*itPmtNew)->pv->getProgramNumber(), ctag);
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
					getCarouselComponentTagFromService((*itPmtNew)->pv,
							((PAit*)itProjNew->second)->getCarouselProj(),
							&ctag);
					project->configAitService(itProjNew->second,
							(*itPmtNew)->pv->getProgramNumber(), ctag);
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
	muxer = new Muxer(project->getPacketSize(), project->getPacketsInBuffer());
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
			updateComponentTagList(currTimeline, newTimeline);
			//begin to switch elementary streams
			createStreamList(currTimeline, newTimeline);
			muxer->removeAllElementaryStreams();
			muxer->clearPidToLayerList();
			itPmt = newTimeline->begin();
			while (itPmt != newTimeline->end()) {
				itStream = (*itPmt)->pv->getStreamList()->begin();
				while (itStream != (*itPmt)->pv->getStreamList()->end()) {
					muxer->addElementaryStream(itStream->first, itStream->second);
					muxer->addPidToLayer(itStream->first,
							(*itPmt)->pv->getLayerPid(itStream->first));
					++itStream;
				}
				++itPmt;
			}
			processPcrsInUse(newTimeline);
			//end of switch elementary streams

			if (currTimeline) {
				//update pat, pmts and other si tables
				restoreSiTables(currTimeline, newTimeline);
				if (project->getPacketSize() == 204) {
					restoreIsdbtInfo();
				}

			} else {
				//first time (create pat, pmts and other si tables)
				if (createSiTables(newTimeline) == -1) {
					return -2;
				}
				if (project->getPacketSize() == 204) {
					createIsdbtInfo();
				}
			}
			currTimeline = newTimeline;
		}

		ret = muxer->mainLoop();

		if (!project->getUseTot()) {
			if (lastStcPrinter != (unsigned int) Stc::stcToSecond(muxer->getCurrentStc())/5) {
				lastStcPrinter = Stc::stcToSecond(muxer->getCurrentStc())/5;
				time(&now);
				PTot::printDateTime(now, "");
				cout << " ~ elapsed time = " <<
					(int64_t) Stc::stcToSecond(muxer->getRelativeStc()) << endl;
			}
		}
	}

	return ret;
}

bool TMM::releaseStreamFromList(ProjectInfo* proj) {
	map<ProjectInfo*, Stream*>::iterator itSi;
	itSi = siAndIsdbtStreamList.find(proj);
	if (itSi != siAndIsdbtStreamList.find(proj)) {
		if (itSi->second) {
			delete itSi->second;
			siAndIsdbtStreamList.erase(itSi);
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
		muxer->addPidToLayer((*itPmt)->pv->getPcrPid(), (*itPmt)->pv->getLayer());
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
int TMM::createIsdbtInfo() {
	ProjectInfo* proj;

	proj = getFirstProject(PT_IIP);
	if (proj) {
		releaseStreamFromList(proj);
		Stream* stream = createStream(proj);
		siAndIsdbtStreamList[proj] = stream;
		addStreamToMuxer(stream, DEFAULT_IIP_PID, proj->getLayer());
	}

	return 0;
}

//This method must be called just once.
int TMM::createSiTables(vector<pmtViewInfo*>* newTimeline) {
	Stream* stream;
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
		stream = new RawStream();
		pmt->updateStream();
		((RawStream*)stream)->addSection(pmt);
		stream->fillBuffer();
		stream->setFrequency(Stc::secondToStc(0.1));
		stream->initiateNextSend(muxer->getCurrentStc() + 1);
		(*itPmt)->pv->setPmtStream(stream);
		addStreamToMuxer(stream, (*itPmt)->pv->getPid(), (*itPmt)->pv->getLayer());
		++itPmt;
	}
	stream = createStream(proj);
	siAndIsdbtStreamList[proj] = stream;
	addStreamToMuxer(stream, DEFAULT_PAT_PID, proj->getLayer());
	//Program Association Table & Program Map Table end

	//Time Offset Table begins
	proj = getFirstProject(PT_TOT);
	releaseStreamFromList(proj);
	if (proj) {
		stream = createStream(proj);
		siAndIsdbtStreamList[proj] = stream;
		addStreamToMuxer(stream, DEFAULT_TOT_PID, proj->getLayer());
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
			stream = createStream(proj);
			siAndIsdbtStreamList[proj] = stream;
			addStreamToMuxer(stream, DEFAULT_SDT_PID, proj->getLayer());
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
			stream = createStream(proj);
			siAndIsdbtStreamList[proj] = stream;
			addStreamToMuxer(stream, DEFAULT_NIT_PID, proj->getLayer());
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
			stream = createStream(proj);
			siAndIsdbtStreamList[proj] = stream;
			addStreamToMuxer(stream, DEFAULT_EIT_PID, proj->getLayer());
		}
		++itPmt;
	}
	//Event Information Table (present/following) ends

	return 0;
}

int TMM::restoreIsdbtInfo() {
	ProjectInfo* proj;
	map<ProjectInfo*, Stream*>::iterator it;

	proj = getFirstProject(PT_IIP);
	if (proj) {
		it = siAndIsdbtStreamList.find(proj);
		if (it != siAndIsdbtStreamList.end()) {
			addStreamToMuxer(it->second, DEFAULT_IIP_PID, proj->getLayer());
		}
	}

	return 0;
}

int TMM::restoreSiTables(vector<pmtViewInfo*>* currentTimeline,
					vector<pmtViewInfo*>* newTimeline) {
	vector<pmtViewInfo*>::iterator itPmtCurr;
	vector<pmtViewInfo*>::iterator itPmtNew;
	map<int, ProjectInfo*>::iterator itProj;
	map<ProjectInfo*, Stream*>::iterator itSi;
	ProjectInfo* proj;
	RawStream *rawstream;
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
		itSi = siAndIsdbtStreamList.find(proj);
		if (itSi != siAndIsdbtStreamList.end()) {
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
			rawstream = (RawStream*) itSi->second;
			rawstream->releaseBlockList();
			rawstream->releaseBufferList();
			((PPat*)proj)->updateStream();
			rawstream->addSection((PPat*)proj);
			rawstream->fillBuffer();
			addStreamToMuxer(rawstream, DEFAULT_PAT_PID, proj->getLayer());
		}
		patChanged = true;
	} else {
		//add the previous one
		addStreamToMuxer(siAndIsdbtStreamList[proj], DEFAULT_PAT_PID, proj->getLayer());
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
				rawstream = new RawStream();
				pmt->updateStream();
				rawstream->addSection(pmt);
				rawstream->fillBuffer();
				rawstream->setFrequency(Stc::secondToStc(0.1));
				rawstream->initiateNextSend(siAndIsdbtStreamList[proj]->getNextSend() + 1);
				(*itPmtNew)->pv->setPmtStream(rawstream);
				addStreamToMuxer(rawstream, (*itPmtNew)->pv->getPid(),
								(*itPmtNew)->pv->getLayer());
			} else {
				//update it
				(*itPmtNew)->pv->setVersion((*itPmtCurr)->pv->getVersion() + 1);
				ret = createPmt((*itPmtCurr)->pv, (*itPmtNew)->pv, &pmt);
				if (ret < 0) return -1;
				pmt->setVersionNumber((*itPmtNew)->pv->getVersion());
				rawstream = (RawStream*) (*itPmtCurr)->pv->getPmtStream();
				if (!rawstream) {
					rawstream = new RawStream();
				} else {
					(*itPmtCurr)->pv->setPmtStream(NULL); //Is it necessary?
					rawstream->releaseBlockList();
					rawstream->releaseBufferList();
				}
				pmt->updateStream();
				rawstream->addSection(pmt);
				rawstream->fillBuffer();
				rawstream->setFrequency(Stc::secondToStc(0.1));
				rawstream->initiateNextSend(siAndIsdbtStreamList[proj]->getNextSend() + 1);
				(*itPmtNew)->pv->setPmtStream(rawstream);
				addStreamToMuxer(rawstream, (*itPmtCurr)->pv->getPid(),
								(*itPmtCurr)->pv->getLayer());
			}
		} else {
			//it's the same PMT
			(*itPmtNew)->pv->setVersion((*itPmtCurr)->pv->getVersion());
			addStreamToMuxer((*itPmtCurr)->pv->getPmtStream(),
							 (*itPmtCurr)->pv->getPid(),
							 (*itPmtCurr)->pv->getLayer());
		}
		++itPmtNew;
		found = false;
	}

	//Service Descriptor Table begins
	if (project->getUseSdt()) {
		proj = getFirstProject(PT_SDT);
		if (proj) {
			rawstream = (RawStream*) siAndIsdbtStreamList[proj];
			if (patChanged) {
				((PSdt*)proj)->setVersion(((PSdt*)proj)->getVersion() + 1);
				ret = project->configSdt(newTimeline, proj);
				if (ret < 0) {
					return -2;
				}
				rawstream->releaseBlockList();
				rawstream->releaseBufferList();
				((PSdt*)proj)->updateStream();
				rawstream->addSection((PSdt*)proj);
				rawstream->fillBuffer();
			}
			addStreamToMuxer(rawstream, DEFAULT_SDT_PID, proj->getLayer());
		}
	}
	//Service Descriptor Table ends

	//Network Information Table begins
	if (project->getUseNit()) {
		proj = getFirstProject(PT_NIT);
		if (proj) {
			rawstream = (RawStream*) siAndIsdbtStreamList[proj];
			if (patChanged) {
				((PNit*)proj)->setVersion(((PNit*)proj)->getVersion() + 1);
				ret = project->configNit(newTimeline, proj);
				if (ret < 0) {
					return -2;
				}
				rawstream->releaseBlockList();
				rawstream->releaseBufferList();
				((PNit*)proj)->updateStream();
				rawstream->addSection((PNit*)proj);
				rawstream->fillBuffer();
			}
			addStreamToMuxer(rawstream, DEFAULT_NIT_PID, proj->getLayer());
		}
	}
	//Network Information Table ends

	//Time Offset Table begins

	proj = getFirstProject(PT_TOT);
	if (project->getUseTot() && proj) {
		itSi = siAndIsdbtStreamList.find(proj);
		if (itSi != siAndIsdbtStreamList.end()) {
			addStreamToMuxer(itSi->second, DEFAULT_TOT_PID, proj->getLayer());
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

			itSi = siAndIsdbtStreamList.find(proj);
			if (itSi == siAndIsdbtStreamList.end()) {
				siAndIsdbtStreamList[proj] = (RawStream*) createStream(proj);
			}
			rawstream = (RawStream*) siAndIsdbtStreamList[proj];
			if (found) {
				rawstream = (RawStream*) siAndIsdbtStreamList[proj];
				rawstream->releaseBlockList();
				rawstream->releaseBufferList();
				rawstream->fillBuffer();
			}
			addStreamToMuxer(rawstream, DEFAULT_EIT_PID, proj->getLayer());
		}
		++itPmtNew;
	}
	//Event Information Table (present/following) ends

	return 0;
}

void TMM::addStreamToMuxer(Stream* stream, unsigned short pid, unsigned char layer) {
	muxer->addElementaryStream(pid, stream);
	muxer->addPidToLayer(pid, layer);
}

ProjectInfo* TMM::getFirstProject(unsigned char projectType) {
	map<int, ProjectInfo*>::iterator itProj = project->getProjectList()->begin();
	while (itProj != project->getProjectList()->end()) {
		if (itProj->second->getProjectType() == projectType) {
			return itProj->second;
		}
		++itProj;
	}
	return NULL;
}

ProjectInfo* TMM::getFirstProjectReversed(unsigned char projectType) {
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


