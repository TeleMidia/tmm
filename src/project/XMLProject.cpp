/******************************************************************************
TeleMídia Multiplexer
Copyright (C) 2016 TeleMídia Lab/PUC-Rio
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
 * XMLProject.cpp
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#include "project/XMLProject.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

XMLProject::XMLProject() : Project() {
	init();
}

XMLProject::XMLProject(const string& filename) : Project() {
	init();
	this->filename = filename;
}

XMLProject::XMLProject(const char* filename) : Project() {
	init();
	this->filename.assign(filename);
}

XMLProject::~XMLProject() {
	vector<MetaData*>::iterator it;
	if (xmldoc) delete xmldoc;
	it = metaDataList.begin();
	while (it != metaDataList.end()) {
		delete (*it);
		++it;
	}
}

void XMLProject::init() {
	xmldoc = new XMLDocument();
	projectName.assign("Untitled project");
	idIndex = 1;
	relStc = 0;
}

vector<MetaData*>* XMLProject::getMetaDataList() {
	return &metaDataList;
}

bool XMLProject::createNewId(const string& id){
	if (idList.count(id)) {
		cout << "The id = " << id << " already exists." << endl;
		return false;
	} else {
		idList[id] = idIndex++;
		return true;
	}
}

int XMLProject::getId(const string& id) {
	if (idList.count(id)) {
		return idList[id];
	} else {
		return -1;
	}
}

int XMLProject::createAndGetId(ProjectInfo* proj, const string& name) {
	int id;

	if (!createNewId(name)) return -3;
	id = getId(name);
	if (id == -1) {
		cout << "The id = " << name << " doesn't exists." << endl;
		return -8;
	}
	proj->setId(id);

	return 0;
}

int XMLProject::parseAV(XMLNode* m, XMLElement* f) {
	XMLNode *o;
	XMLElement *g = NULL;
	string value;
	int num, ret;

	if (strcmp(m->Value(), "av") == 0) {
		value = LocalLibrary::getAttribute(f, "id");
		InputData* input = new InputData();
		ret = createAndGetId(input, value);
		if (ret < 0) {
			delete input;
			return ret;
		}
		value = LocalLibrary::getAttribute(f, "src");
		input->setFilename(value);
		if (f->QueryAttribute("offset", &num) == XML_NO_ERROR) {
			input->setOffset(num);
		}
		if (f->QueryAttribute("pid", &num) == XML_NO_ERROR) {
			input->setPid(num);
		} else {
			cout << "AV: attribute 'pid' not found." << endl;
			return -4;
		}
		for (o = f->FirstChild(); o; o = o->NextSibling()) {
			g = o->ToElement();
			if (strcmp(o->Value(), "range") == 0) {
				InputRange* ir = new InputRange();
				g->QueryAttribute("begin", &num);
				ir->setBegin(num);
				g->QueryAttribute("end", &num);
				ir->setEnd(num);
				input->addRange(ir);
			}
		}
		(*projectList)[input->getId()] = input;
	}

	return 0;
}

int XMLProject::parseNPT(XMLNode* m, XMLElement* f) {
	XMLNode *o, *p;
	XMLElement *g = NULL, *h;
	string value;
	int num, ret;

	if (strcmp(m->Value(), "npt") == 0) {
		value = LocalLibrary::getAttribute(f, "id");
		NPTProject* npt = new NPTProject();
		ret = createAndGetId(npt, value);
		if (ret < 0) {
			delete npt;
			return ret;
		}
		if (f->QueryAttribute("transmissiondelay", &num) == XML_NO_ERROR) {
			npt->setTransmissionDelay((double) num / 1000);
		}
		if (f->QueryAttribute("offset", &num) == XML_NO_ERROR) {
			npt->setFirstReferenceOffset((double) num / 1000);
		}
		for (o = f->FirstChild(); o; o = o->NextSibling()) {
			int epbegin = 0, epend = 0;
			bool firstTimeEpBegin = true;
			g = o->ToElement();
			if (strcmp(o->Value(), "nptref") == 0) {
				value = LocalLibrary::getAttribute(g, "groupid");
				if (!createNewId(value)) return -3;
				num = getId(value);
				if (num == -1) {
					cout << "The id = " << value << " doesn't exists." << endl;
					delete npt;
					return -8;
				}
				for (p = g->FirstChild(); p; p = p->NextSibling()) {
					h = p->ToElement();
					if (strcmp(p->Value(), "timeref") == 0) {
						int cid, nptbegin, nptend, absbegin, absend,
							numerator, denominator, hasnext, temp;
						numerator = 1;
						denominator = 1;
						hasnext = 1;
						if (h->QueryAttribute("contentid", &cid) != XML_NO_ERROR) {
							cout << "npt: attribute 'contentid' not found." << endl;
							return -4;
						}
						if (h->QueryAttribute("nptbegin", &nptbegin) != XML_NO_ERROR) {
							cout << "npt: attribute 'nptbegin' not found." << endl;
							return -4;
						}
						if (h->QueryAttribute("absbegin", &absbegin) != XML_NO_ERROR) {
							cout << "npt: attribute 'absbegin' not found." << endl;
							return -4;
						}
						if (h->QueryAttribute("absend", &absend) != XML_NO_ERROR) {
							cout << "npt: attribute 'absend' not found." << endl;
							return -4;
						}
						if (h->QueryAttribute("numerator", &temp) == XML_NO_ERROR) {
							numerator = temp;
						}
						if (h->QueryAttribute("denominator", &temp) == XML_NO_ERROR) {
							denominator = temp;
						}
						if (h->QueryAttribute("hasnext", &temp) == XML_NO_ERROR) {
							hasnext = temp;
						} else {
							if (!p->NextSibling()) hasnext = 0;
						}
						nptend = (int)(nptbegin+((absend-absbegin)*
								 ((double)numerator/denominator)));
						npt->addReference(
							num,
							cid,
							Stc::secondToStc((double)absbegin/1000),
							Stc::secondToStc((double)absend/1000),
							Stc::secondToStc((double)nptbegin/1000),
							Stc::secondToStc((double)nptend/1000),
							numerator,
							denominator,
							hasnext);
						if (firstTimeEpBegin) {
							firstTimeEpBegin = false;
							epbegin = nptbegin;
						}
						if (!p->NextSibling()) {
							epend = nptend;
						}
					} else {
						cout << "npt: <timeref> not found." << endl;
						return -5;
					}
				}
				if (!firstTimeEpBegin) {
					npt->addEndpoint(
							num,
							Stc::secondToStc((double)epbegin/1000),
							Stc::secondToStc((double)epend/1000));
				}
			} else {
				cout << "npt: <nptref> not found." << endl;
				return -5;
			}
		}
		(*projectList)[npt->getId()] = npt;
	}

	return 0;
}

int XMLProject::parseStreamEvent(XMLNode* m, XMLElement* f) {
	XMLNode *o;
	XMLElement *g;
	string value, value1, entrypoint, params;
	int num, ret;
	ProjectInfo* proj;

	if (strcmp(m->Value(), "streamevent") == 0) {
		PStreamEvent *pse = new PStreamEvent();

		value = LocalLibrary::getAttribute(f, "id");
		ret = createAndGetId(pse, value);
		if (ret < 0) {
			delete pse;
			return ret;
		}
		if (f->QueryAttribute("period", &num) == XML_NO_ERROR) {
			pse->setPeriod((unsigned int)num);
		}
		if (f->QueryAttribute("samplelimit", &num) == XML_NO_ERROR) {
			pse->setSampleLimit((unsigned int)num);
		}
		if (f->QueryAttribute("offset", &num) == XML_NO_ERROR) {
			pse->setFirstReferenceOffset((double) num / 1000);
		}
		value = LocalLibrary::getAttribute(f, "baseid");
		if (value.size()) {
			pse->setBaseId(value);
		} else {
			pse->setBaseId("null");
		}
		value = LocalLibrary::getAttribute(f, "documentid");
		if (value.size()) {
			pse->setDocumentId(value);
		} else {
			pse->setDocumentId("null");
		}
		value = LocalLibrary::getAttribute(f, "layer");
		if (value.size()) {
			num = toLayer(value);
			if (num != 0xFF) {
				pse->setLayer((unsigned char)num);
				pse->setLayerConfigured(true);
			} else {
				cout << "streamevent: 'layer' not recognized ("
					 << value << ")" << endl;
				return -6;
			}
		}
		entrypoint = LocalLibrary::getAttribute(f, "entrypoint");
		if (entrypoint.size()) {
			pse->setEntryPoint(entrypoint);
			value = LocalLibrary::getAttribute(f, "carouselid");
			num = getId(value);
			if (num == -1) {
				cout << "The id = " << value << " doesn't exists."
						<< endl;
				return -8;
			}
			proj = findProject(num);
			if (!proj) {
				return -7;
			}
			if (proj->getProjectType() != PT_CAROUSEL) {
				cout << "streamevent: id mismatch error." << endl;
				return -9;
			}
			pse->setCarouselProj(proj);
		} else {
			pse->setEntryPoint("null");
		}
		for (o = f->FirstChild(); o; o = o->NextSibling()) {
			g = o->ToElement();
			if (strcmp(o->Value(), "event") == 0) {
				unsigned int eventId, eventNpt;
				if (g->QueryAttribute("eventid", &eventId) != XML_NO_ERROR) {
					cout << "streamevent: attribute 'eventid' not found." << endl;
					return -4;
				}
				StreamEvent* se = new StreamEvent();
				se->setEventId(eventId);
				if (g->QueryAttribute("eventnpt", &eventNpt) == XML_NO_ERROR) {
					se->setEventNPT(eventNpt);
				} else se->setEventNPT(0);
				value1 = LocalLibrary::getAttribute(g, "commandtag");
				if (value1.size()) {
					if (commandTagList.count(value1)) {
						se->setCommandTag(commandTagList[value1]);
					} else {
						cout << "streamevent: 'commandtag' not recognized." << endl;
						delete se;
						delete pse;
						return -6;
					}
				} else {
					cout << "streamevent: attribute 'commandtag' not found." << endl;
					delete se;
					delete pse;
					return -6;
				}
				params = LocalLibrary::getAttribute(g, "params");
				if (params.size()) {
					se->setPrivateDataPayload((char*)params.c_str(), params.size());
				}
				se->setSequenceNumber(0);
				se->setFinalFlag(0);
				pse->addStreamEvent(se);
			}
		}
		(*projectList)[pse->getId()] = pse;
	}

	return 0;
}

int XMLProject::parseCarousel(XMLNode* m, XMLElement* f) {
	XMLNode *o, *p;
	XMLElement *g, *h;
	string value, value2;
	int num, ret;

	if (strcmp(m->Value(), "carousel") == 0) {
		value = LocalLibrary::getAttribute(f, "id");
		PCarousel* carousel = new PCarousel();
		ret = createAndGetId(carousel, value);
		if (ret < 0) {
			delete carousel;
			return ret;
		}
		value = LocalLibrary::getAttribute(f, "src");
		carousel->setServiceGatewayFolder(value);
		if (f->QueryAttribute("bitrate", &num) != XML_NO_ERROR) {
			cout << "carousel: attribute 'bitrate' not found." << endl;
			delete carousel;
			return -4;
		}
		carousel->setBitrate(num);
		if (f->QueryAttribute("version", &num) == XML_NO_ERROR) {
			carousel->setVersion((unsigned char)num);
		} else {
			carousel->setVersion(0x01);
		}
		if (f->QueryAttribute("servicedomain", &num) != XML_NO_ERROR) {
			cout << "carousel: attribute 'servicedomain' not found." << endl;
			delete carousel;
			return -4;
		}
		carousel->setServiceDomain(num);
		if (f->QueryAttribute("transactionid", &num) == XML_NO_ERROR) {
			carousel->setTransactionId(num);
		} else {
			carousel->setTransactionId(2);
		}
		if (f->QueryAttribute("transmissiondelay", &num) == XML_NO_ERROR) {
			carousel->setTransmissionDelay((double) num / 1000);
		}

		for (o = f->FirstChild(); o; o = o->NextSibling()) {
			g = o->ToElement();
			if (strcmp(o->Value(), "streameventobject") == 0) {
				StreamEventMessage* sem = new StreamEventMessage();
				StreamInfoT* oi = new StreamInfoT();
				sem->setObjectInfo(oi);
				for (p = g->FirstChild(); p; p = p->NextSibling()) {
					h = p->ToElement();
					if (strcmp(p->Value(), "event") == 0) {
						value = LocalLibrary::getAttribute(h, "eventname");
						if (!value.size()) {
							cout << "carousel: attribute 'eventname' is null." << endl;
							delete sem;
							delete carousel;
							return -12;
						}
						if (h->QueryAttribute("eventid", &num) != XML_NO_ERROR) {
							cout << "carousel: attribute 'eventid' not found." << endl;
							delete sem;
							delete carousel;
							return -4;
						}
						sem->addEvent(num, (char*)value.c_str(), value.size());
					}
					if (strcmp(p->Value(), "tap") == 0) {
						Tap* tap = new Tap();
						tap->setSelectorLength(0);
						value = LocalLibrary::getAttribute(h, "tapuse");
						if (!value.size()) {
							cout << "carousel: attribute 'tapuse' is null." << endl;
							delete tap;
							delete carousel;
							return -12;
						}
						if (value == "strnptuse") {
							tap->setUse(Tap::STR_NPT_USE);
						} else if (value == "strstatusandeventuse") {
							tap->setUse(Tap::STR_STATUS_AND_EVENT_USE);
						} else if (value == "streventuse") {
							tap->setUse(Tap::STR_EVENT_USE);
						} else if (value == "strstatususe") {
							tap->setUse(Tap::STR_STATUS_USE);
						} else if (value == "biopesuse") {
							tap->setUse(Tap::BIOP_ES_USE);
						} else if (value == "biopprogramuse") {
							tap->setUse(Tap::BIOP_PROGRAM_USE);
						} else {
							cout << "carousel: 'tapuse' not recognized ("
								 << value << ")" << endl;
							delete tap;
							delete carousel;
							return -6;
						}
						if (h->QueryAttribute("associationtag", &num) != XML_NO_ERROR) {
							cout << "carousel: attribute 'associationtag' not found." << endl;
							delete tap;
							delete carousel;
							return -4;
						}
						tap->setAssocTag(num);
						if (h->QueryAttribute("tapid", &num) == XML_NO_ERROR) {
							tap->setId(num);
						} else {
							tap->setId(0);
						}
						sem->addTap(tap);
					}
				}
				carousel->addStreamEventMessage(sem);
			}
		}

		(*projectList)[carousel->getId()] = carousel;
	}

	return 0;
}

int XMLProject::parseAIT(XMLNode* m, XMLElement* f) {
	ProjectInfo* proj;
	XMLNode *o;
	XMLElement *g;
	string value;
	int num, ret;

	if (strcmp(m->Value(), "ait") == 0) {
		PAit *ait = new PAit();
		ait->setCurrentNextIndicator(1);
		ait->setSectionSyntaxIndicator(1);
		ait->setPrivateIndicator(1);
		value = LocalLibrary::getAttribute(f, "id");
		ret = createAndGetId(ait, value);
		if (ret < 0) {
			delete ait;
			return ret;
		}
		if (f->QueryAttribute("transmissiondelay", &num) == XML_NO_ERROR) {
			ait->setTransmissionDelay((double) num / 1000);
		}
		for (o = f->FirstChild(); o; o = o->NextSibling()) {
			g = o->ToElement();
			if (strcmp(o->Value(), "carouselref") == 0) {
				unsigned int orgId, appId, appcode, rres;
				string appname, lang, basedir, entrypoint;
				value = LocalLibrary::getAttribute(g, "carouselid");
				num = getId(value);
				if (num == -1) {
					cout << "The id = " << value << " doesn't exists." << endl;
					return -8;
				}
				value = LocalLibrary::getAttribute(g, "apptype");
				if (value == "ginga-ncl") {
					ait->setTableIdExtension(AT_GINGA_NCL);
				} else if (value == "ginga-j") {
					ait->setTableIdExtension(AT_GINGA_J);
				} else if (value == "ginga-html") {
					ait->setTableIdExtension(0x000B);
				} else {
					cout << "ait: 'apptype' not recognized ("
						 << value << ")" << endl;
					return -6;
				}
				appcode = CC_AUTO_START;
				value = LocalLibrary::getAttribute(g, "appcontrolcode");
				if (value.size()) {
					if (value == "autostart") {
						appcode = CC_AUTO_START;
					} else if (value == "present") {
						appcode = CC_PRESENT;
					} else if (value == "destroy") {
						appcode = CC_DESTROY;
					} else if (value == "kill") {
						appcode = CC_KILL;
					} else if (value == "remote") {
						appcode = CC_REMOTE;
					} else if (value == "unbound") {
						appcode = CC_UNBOUND;
					} else {
						cout << "ait: 'appcontrolcode' not recognized ("
							 << value << ")" << endl;
						return -6;
					}
				}
				rres = RR_MULTIPLE_SIZES_RESOLUTIONS;
				value = LocalLibrary::getAttribute(g, "resolution");
				if (value.size()) {
					if (value == "1920-1080-16-9") {
						rres = RR_1920_1080_16_9;
					} else if (value == "1280-720-16-9") {
						rres = RR_1280_720_16_9;
					} else if (value == "960-540-16-9") {
						rres = RR_960_540_16_9;
					} else if (value == "720-480-16-9") {
						rres = RR_720_480_16_9;
					} else if (value == "720-480-4-3") {
						rres = RR_720_480_4_3;
					} else if (value == "160-120-4-3") {
						rres = RR_160_120_4_3;
					} else if (value == "160-90-16-9") {
						rres = RR_160_90_16_9;
					} else if (value == "320-240-4-3") {
						rres = RR_320_240_4_3;
					} else if (value == "320-180-16-9") {
						rres = RR_320_180_16_9;
					} else if (value == "352-288-4-3") {
						rres = RR_352_288_4_3;
					} else if (value == "240-n-portrait") {
						rres = RR_240_N_PORTRAIT;
					} else if (value == "n-240-landscape") {
						rres = RR_N_240_LANDSCAPE;
					} else if (value == "480-n-portrait") {
							rres = RR_480_N_PORTRAIT;
					} else if (value == "n-480-landscape") {
						rres = RR_N_480_LANDSCAPE;
					} else if (value == "multiplesizesresolutions") {
						rres = RR_MULTIPLE_SIZES_RESOLUTIONS;
					} else {
						cout << "ait: 'resolution' not recognized ("
							 << value << ")" << endl;
						return -6;
					}
				}
				if (g->QueryAttribute("organizationid", &orgId) != XML_NO_ERROR) {
					//See ABNT NBR 15606-3 2012
					orgId = (((unsigned int) originalNetworkId) << 16) |
											 originalNetworkId;
				}
				if (g->QueryAttribute("applicationid", &appId) != XML_NO_ERROR) {
					cout << "ait: attribute 'applicationid' not found." << endl;
					return -4;
				}
				appname = LocalLibrary::getAttribute(g, "appname");
				value = LocalLibrary::getAttribute(g, "language");
				if (value.size()) {
					lang = value;
				} else {
					lang.assign("por");
				}
				value = LocalLibrary::getAttribute(g, "basedirectory");
				if (value.size()) {
					basedir = value;
				} else {
					basedir.assign("/");
				}
				entrypoint = LocalLibrary::getAttribute(g, "entrypoint");
				proj = findProject(num);
				if (!proj) {
					return -7;
				}
				if (proj->getProjectType() != PT_CAROUSEL) {
					cout << "ait: id mismatch error." << endl;
					return -9;
				}
				if (proj) {
					ait->setCarouselProj(proj);
					configAit(ait, ((PCarousel*)proj)->getServiceDomain(),
							appname, lang, basedir,
							entrypoint, orgId, appId, appcode, rres);
					(*projectList)[ait->getId()] = ait;
				} else {
					delete ait;
					return false;
				}
			}
		}
	}

	return 0;
}

int XMLProject::parseEIT(XMLNode* m, XMLElement* f) {
	XMLNode *o, *p;
	XMLElement *g, *h;
	string value, value2;
	int num, ret;

	if (strcmp(m->Value(), "eit") == 0) {
		PEit *eit = new PEit();
		eit->setCurrentNextIndicator(1);
		eit->setSectionSyntaxIndicator(1);
		eit->setPrivateIndicator(1);
		value = LocalLibrary::getAttribute(f, "id");
		ret = createAndGetId(eit, value);
		if (ret < 0) {
			delete eit;
			return ret;
		}
		value = LocalLibrary::getAttribute(f, "layer");
		if (value.size()) {
			num = toLayer(value);
			if (num != 0xFF) {
				eit->setLayer((unsigned char)num);
				eit->setLayerConfigured(true);
			} else {
				cout << "eit: 'layer' not recognized ("
					 << value << ")" << endl;
				return -6;
			}
		}
		for (o = f->FirstChild(); o; o = o->NextSibling()) {
			g = o->ToElement();
			if (strcmp(o->Value(), "event") == 0) {
				unsigned int eventId, durt;
				time_t stime;
				if (g->QueryAttribute("eventid", &eventId) != XML_NO_ERROR) {
					cout << "eit: attribute 'eventid' not found." << endl;
					return -4;
				}
				if (eit->hasEvent(eventId)) {
					cout << "eit: 'eventid' must be unique." << endl;
					return -4;
				}
				value = LocalLibrary::getAttribute(g, "time");
				if (value.size()) {
					stime = PTot::makeUtcDate(value);
				} else {
					cout << "eit: attribute 'time' not found or null." << endl;
					return -4;
				}
				if (g->QueryAttribute("dur", &durt) != XML_NO_ERROR) {
					cout << "eit: attribute 'dur' not found." << endl;
					return -4;
				}
				EventInfo* ei = new EventInfo();
				ei->eventId = eventId;
				ei->duration = durt/1000;
				ei->startTime = stime;
				ei->freeCaMode = false;
				ei->runningStatus = RS_NOT_RUNNING;
				for (p = g->FirstChild(); p; p = p->NextSibling()) {
					h = p->ToElement();
					if (strcmp(p->Value(), "shortevent") == 0) {
						value = LocalLibrary::getAttribute(h, "name");
						if (value.size() > 96) {
							cout << "eit: 'name' value is too long." << endl;
							return -12;
						}
						value2 = LocalLibrary::getAttribute(h, "text");
						if (value2.size() > 192) {
							cout << "eit: 'text' value is too long." << endl;
							return -12;
						}
						ShortEvent* si = new ShortEvent();
						si->setEventName(value);
						si->setText(value2);
						value = LocalLibrary::getAttribute(h, "language");
						if (value.size()) {
							if (value.size() != 3) {
								cout << "eit: 'language' value has 3 letters." << endl;
								return -12;
							}
							si->setIso639LanguageCode(value);
						}
						ei->descriptorsList.push_back(si);
					}
					if (strcmp(p->Value(), "component") == 0) {
						int streamContent, componentType, ctag;
						if (h->QueryAttribute("streamcontent", &streamContent) != XML_NO_ERROR) {
							cout << "eit: attribute 'streamcontent' not found." << endl;
							return -4;
						}
						if (h->QueryAttribute("componenttype", &componentType) != XML_NO_ERROR) {
							cout << "eit: attribute 'componentType' not found." << endl;
							return -4;
						}
						if (h->QueryAttribute("ctag", &ctag) != XML_NO_ERROR) {
							cout << "eit: attribute 'ctag' not found." << endl;
							return -4;
						}
						Component* comp = new Component();
						comp->setStreamContent(streamContent);
						comp->setComponentType(componentType);
						comp->setComponentTag(ctag);
						value = LocalLibrary::getAttribute(h, "language");
						if (value.size()) {
							if (value.size() != 3) {
								cout << "eit: 'language' value has 3 letters." << endl;
								return -12;
							}
							comp->setIso639LanguageCode(value);
						}
						value2 = LocalLibrary::getAttribute(h, "text");
						if (value2.size() > 192) {
							cout << "eit: 'text' value is too long." << endl;
							return -12;
						}
						comp->setText(value2);
						ei->descriptorsList.push_back(comp);
					}
					if (strcmp(p->Value(), "parentalrating") == 0) {
						int rating = 1;
						unsigned char objective = 0;
						value = LocalLibrary::getAttribute(h, "agerating");
						if (value.size()) {
							if (value == "10") {
								rating = 2;
							} else if (value == "12") {
								rating = 3;
							} else if (value == "14") {
								rating = 4;
							} else if (value == "16") {
								rating = 5;
							} else if (value == "18") {
								rating = 6;
							} else rating = 1;
						}
						value = LocalLibrary::getAttribute(h, "objectivecontentdescription");
						if (value.size()) {
							size_t found = value.find("drugs");
							if (found != std::string::npos) {
								objective = objective | 1;
							}
							found = value.find("violence");
							if (found != std::string::npos) {
								objective = objective | 2;
							}
							found = value.find("sex");
							if (found != std::string::npos) {
								objective = objective | 4;
							}
						}
						rating = rating | (objective << 4);
						if (h->QueryAttribute("rating", &num) == XML_NO_ERROR) {
							rating = num;
						}
						ParentalRatingInfo* pri = new ParentalRatingInfo();
						pri->rating = rating;
						value = LocalLibrary::getAttribute(h, "countrycode");
						if (value.size()) {
							if (value.size() != 3) {
								cout << "eit: 'countrycode' value has 3 letters." << endl;
								return -12;
							}
							pri->countryCode = LocalLibrary::upperCase(value);
						}
						ParentalRating* pr = new ParentalRating();
						pr->addParentalRating(pri);
						ei->descriptorsList.push_back(pr);
					}
				}
				eit->addEventInfo(ei);
			}
		}
		(*projectList)[eit->getId()] = eit;
	}

	return 0;
}

int XMLProject::parsePMT(XMLNode* m, XMLElement* f) {
	ProjectInfo* proj;
	XMLNode *o, *p;
	XMLElement *g, *h;
	string value;
	int num, ret;
	bool useServiceNumber;

	if (strcmp(m->Value(), "pmt") == 0) {
		PMTView* pmtView = new PMTView();
		value = LocalLibrary::getAttribute(f, "id");
		ret = createAndGetId(pmtView, value);
		if (ret < 0) {
			delete pmtView;
			return ret;
		}
		useServiceNumber = false;
		value = LocalLibrary::getAttribute(f, "servicetype");
		if (value.size()) {
			if (value == "tv") {
				pmtView->setServiceType(SRV_TYPE_TV);
				pmtView->setLayer(HIERARCHY_B);
			} else if (value == "data1") {
				pmtView->setServiceType(SRV_TYPE_DATA1);
			} else if (value == "data2") {
				pmtView->setServiceType(SRV_TYPE_DATA2);
			} else if (value == "oneseg") {
				pmtView->setServiceType(SRV_TYPE_ONESEG);
				pmtView->setLayer(HIERARCHY_A);
			} else {
				cout << "pmt: 'servicetype' not recognized ("
					 << value << ")" << endl;
				return -6;
			}
			pmtView->setLayerConfigured(true);
		}
		/*
		 * Program number -> ABNTNBR 15603-2, Annex H.3
		 */
		if (f->QueryAttribute("servicenumber", &num) == XML_NO_ERROR) {
			unsigned short pn = pmtView->getServiceType() << 3;
			if (num > 7) {
				cout << "pmt: 'servicenumber' range is 0 to 7." << endl;
				return -12;
			}
			pn = pn | (originalNetworkId << 5);
			pn = pn | (num & 0x07);
			pmtView->setProgramNumber(pn);
			if (pmtView->getServiceType() == SRV_TYPE_ONESEG) {
				pmtView->setPid(0x1FC8 + num);
			}
			useServiceNumber = true;
		} else {
			if (f->QueryAttribute("programnumber", &num) != XML_NO_ERROR) {
				cout << "pmt: attribute 'programnumber' not found." << endl;
				return -4;
			}
			if (num == 0) {
				cout << "pmt: programnumber = 0 is not allowed." << endl;
				return -4;
			}
			pmtView->setProgramNumber(num);
		}
		if (f->QueryAttribute("pcrpid", &num) != XML_NO_ERROR) {
			cout << "pmt: attribute 'pcrpid' not found." << endl;
			return -4;
		}
		pmtView->setPcrPid(num);
		if (f->QueryAttribute("pcrperiod", &num) == XML_NO_ERROR) {
			if (num < 10000) {
				cout << "pmt: 'pcrperiod' value is too low (microseconds)." << endl;
				return -12;
			}
			pmtView->setPcrPeriod(num);
		} else {
			pmtView->setPcrPeriod(60000);
		}
		value = LocalLibrary::getAttribute(f, "name");
		if (value.size() > 20) {
			cout << "pmt: 'name' value is too long." << endl;
			return -12;
		}
		if (value.size()) {
			pmtView->setServiceName(value);
			if (!useSdt) {
				int id2;
				value = SDT_NAME;
				if (!createNewId(value)) return -3;
				pSdt = new PSdt();
				id2 = getId(value);
				pSdt->setId(id2);
				(*projectList)[id2] = pSdt;
			}
			useSdt = true;
		} else {
			pmtView->setServiceName("Unnamed service");
		}
		if (pmtView->getServiceType() != SRV_TYPE_ONESEG) {
			if (f->QueryAttribute("pid", &num) != XML_NO_ERROR) {
				cout << "pmt: attribute 'pid' not found." << endl;
				return -4;
			}
			pmtView->setPid(num);
		} else {
			if (useServiceNumber) {
				if (f->QueryAttribute("pid", &num) == XML_NO_ERROR) {
					if ((num >= 0x1FC8) && (num <= 0x1FCF)) {
						cout << "pmt: the one-seg service overrode your 'pid' " <<
								"value to " << pmtView->getPid() << "." << endl;
					}
				}
			} else {
				if (f->QueryAttribute("pid", &num) != XML_NO_ERROR) {
					cout << "pmt: attribute 'pid' not found." << endl;
					return -4;
				}
				pmtView->setPid(num);
			}
		}
		value = LocalLibrary::getAttribute(f, "layer");
		if (value.size()) {
			num = toLayer(value);
			if (num != 0xFF) {
				pmtView->setLayer((unsigned char)num);
				pmtView->setLayerConfigured(true);
			} else {
				cout << "pmt: 'layer' not recognized ("
					 << value << ")" << endl;
				return -6;
			}
		}
		value = LocalLibrary::getAttribute(f, "eitid");
		if (value.size()) {
			num = getId(value);
			if (num == -1) {
				cout << "The id = " << value << " doesn't exists." << endl;
				return -8;
			}
			proj = findProject(num);
			if (proj) {
				pmtView->setEitProj(proj);
				if (!proj->getLayerConfigured()) {
					proj->setLayer(pmtView->getLayer());
					proj->setLayerConfigured(true);
				}
			} else return -7;
		}
		for (o = f->FirstChild(); o; o = o->NextSibling()) {
			int esPid;
			g = o->ToElement();
			if (strcmp(o->Value(), "es") == 0) {
				value = LocalLibrary::getAttribute(g, "refid");
				proj = findProject(getId(value));
				if (!proj) {
					cout << "pmt: attribute 'refid' not found ("
						 << value << ")" << endl;
					return -7;
				}
				if (g->QueryAttribute("pid", &esPid) != XML_NO_ERROR) {
					cout << "pmt: attribute 'pid' not found." << endl;
					return -4;
				}
				pmtView->addProjectInfo(esPid, proj);
				if (g->QueryAttribute("ctag", &num) == XML_NO_ERROR) {
					pmtView->addDesiredComponentTag(esPid, num);
				}
				if (pmtView->getServiceType() >= 0) {
					switch (pmtView->getServiceType()) {
					case SRV_TYPE_TV:
						pmtView->addPidToLayer(esPid, HIERARCHY_B);
						break;
					case SRV_TYPE_ONESEG:
						pmtView->addPidToLayer(esPid, HIERARCHY_A);
						break;
					}
				}
				value = LocalLibrary::getAttribute(g, "layer");
				if (value.size()) {
					num = toLayer(value);
					if (num != 0xFF) {
						pmtView->addPidToLayer(esPid, (unsigned char)num);
						pmtView->setLayerConfigured(true);
					} else {
						cout << "pmt: 'layer' not recognized ("
							 << value << ")" << endl;
						return -6;
					}
				}
				for (p = g->FirstChild(); p; p = p->NextSibling()) {
					int aacValue;
					h = p->ToElement();
					if (strcmp(p->Value(), "aac") == 0) {
						if (h->QueryAttribute("profileandlevel", &aacValue) != XML_NO_ERROR) {
							cout << "pmt: attribute 'profileandlevel' not found." << endl;
							return -4;
						}
						Aac* aac = new Aac();
						aac->setProfileAndLevel(aacValue);
						if (h->QueryAttribute("aactype", &num) == XML_NO_ERROR) {
							aac->setAacTypeFlag(true);
							aac->setAacType(aacValue);
						}
						pmtView->addEsDescriptor(esPid, aac);
					}
					if (strcmp(p->Value(), "iso639language") == 0) {
						value = LocalLibrary::getAttribute(h, "language");
						if (value.size() != 3) {
							cout << "pmt: attribute 'language' not found or invalid." << endl;
							return -4;
						}
						if (h->QueryAttribute("audiotype", &aacValue) != XML_NO_ERROR) {
							cout << "pmt: attribute 'audiotype' not found." << endl;
							return -4;
						}
						Iso639Language* iso639Language = new Iso639Language();
						iso639Language->addIso639Language(value, aacValue);
						pmtView->addEsDescriptor(esPid, iso639Language);
					}
				}
			}
		}
		(*projectList)[pmtView->getId()] = pmtView;
	}

	return 0;
}

int XMLProject::processInputs(XMLElement *top) {
	XMLNode *n, *m;
	XMLElement *e, *f;
	int ret, parseNum;

	for (n = top->FirstChild(); n; n = n->NextSibling()) {
		e = n->ToElement();
		if (strcmp(n->Value(), "inputs") == 0) {
			for (parseNum = 0; parseNum < 7; parseNum++) {
				for (m = e->FirstChild(); m; m = m->NextSibling()) {
					f = m->ToElement();
					switch (parseNum) {
					case 0:
						ret = parseAV(m, f);
						break;
					case 1:
						ret = parseNPT(m, f);
						break;
					case 2:
						ret = parseCarousel(m, f);
						break;
					case 3:
						ret = parseAIT(m, f);
						break;
					case 4:
						ret = parseStreamEvent(m, f);
						break;
					case 5:
						ret = parseEIT(m, f);
						break;
					case 6:
						ret = parsePMT(m, f);
						break;
					}
					if (ret < 0) return ret;
				}
			}
		}
	}

	return 0;
}

int XMLProject::processOutputProperties(XMLElement *top) {
	map<string, unsigned char>::iterator itRegion;
	ProjectInfo* proj;
	XMLNode *n;
	XMLElement *e = NULL;
	string value1;
	int num, id, ret;
	double dnum;

	PTot* pTot = NULL;
	PNit* pNit = NULL;

	for (n = top->FirstChild(); n; n = n->NextSibling()) {
		e = n->ToElement();
		if (strcmp(n->Value(), "output") == 0) {
			bool systimeAttrib = false;
			id = getId(TOT_NAME);
			if (id >= 0) {
				proj = findProject(id);
				if (proj && proj->getProjectType() == PT_TOT) {
					pTot = (PTot*) proj;
				}
			}
			value1 = LocalLibrary::getAttribute(e, "loop");
			if (value1 == "true") {
				isLoop = true;
			}
			if (e->QueryAttribute("ttl", &num) == XML_NO_ERROR) {
				ttl = num;
			}
			destination = LocalLibrary::getAttribute(e, "dest");
			value1 = LocalLibrary::getAttribute(e, "usepipe");
			if (value1 == "true") {
				isPipe = true;
			}
			externalApp = LocalLibrary::getAttribute(e, "externalapp");
			if (externalApp.size()) {
				unsigned found = externalApp.find_first_of("/\\");
				if (found != 0) {
					externalApp = tmmPath + getUriSlash() + externalApp;
				} else {
					externalApp = tmmPath + externalApp;
				}
			}
			appParams = LocalLibrary::getAttribute(e, "appparams");
			value1 = LocalLibrary::getAttribute(e, "generatingstation");
			if (value1.size()) {
				unsigned char i;
				unsigned int net;
				string netstr;
				char letter = tolower(value1[2]);
				switch (letter) {
					case 'a': netstr = "0";
							break;
					case 'b': netstr = "1";
							break;
					case 'p': netstr = "2";
							break;
					case 'q': netstr = "3";
							break;
					case 't': netstr = "4";
							break;
				}
				if (!netstr.size()) {
					cout << "output: 'generatingstation' value is invalid." << endl;
					return -12;
				}
				for (i = 3; i < value1.size(); i++) {
					if (isdigit(value1[i])) {
						netstr = netstr + value1[i];
					}
				}
				if (sscanf(netstr.c_str(),"%d",&net) > 0) {
					generatingStation.assign(value1);
					tsid = net;
					originalNetworkId = net;
				} else {
					cout << "output: 'generatingstation' value is invalid." << endl;
					return -12;
				}
			}
			if (!generatingStation.size()) {
				if (e->QueryAttribute("tsid", &num) == XML_NO_ERROR) {
					tsid = num;
				} else {
					tsid = 1;
				}
				if (e->QueryAttribute("originalnetworkid", &num) == XML_NO_ERROR) {
					originalNetworkId = num;
				} else {
					originalNetworkId = tsid;
				}
			}
			if (e->QueryAttribute("vbv", &num) == XML_NO_ERROR) {
				vbvBuffer = (double) num / 1000;
			} else {
				vbvBuffer = 0.0;
			}
			if (e->QueryAttribute("bitrate", &num) == XML_NO_ERROR) {
				tsBitrate = num;
			} else {
				tsBitrate = 18000000;
			}
			value1 = LocalLibrary::getAttribute(e, "name");
			if (value1.size() > 20) {
				cout << "output: 'name' value is too long." << endl;
				return -12;
			}
			if (value1.size()) {
				providerName = value1;
				if (!useNit) {
					pNit = new PNit();
					ret = createAndGetId(pNit, NIT_NAME);
					if (ret < 0) return ret;
					(*projectList)[pNit->getId()] = pNit;
				}
				useNit = true;
			} else {
				providerName.assign("Unnamed provider");
			}
			value1 = LocalLibrary::getAttribute(e, "tsname");
			if (value1.size() > 20) {
				cout << "output: 'tsname' value is too long." << endl;
				return -12;
			}
			if (value1.size()) {
				tsName = value1;
			} else {
				tsName.assign(providerName);
			}
			if (e->QueryAttribute("stcbegin", &dnum) == XML_NO_ERROR) {
				if (dnum < 10.0) {
					cout << "output: 'stcbegin' value is too low " <<
							"(minimum is 10 seconds)." << endl;
					return -12;
				}
				stcBegin = Stc::secondToStc(dnum);
			}
			if (e->QueryAttribute("packetspermessage", &num) == XML_NO_ERROR) {
				packetsInBuffer = num;
			}
			if (e->QueryAttribute("broadcastfrequency", &num) == XML_NO_ERROR) {
				broadcastFrequency = num;
			} else {
				broadcastFrequency = 395;
			}
			if (e->QueryAttribute("virtualchannel", &num) == XML_NO_ERROR) {
				virtualChannel = num;
			} else {
				virtualChannel = 1;
			}
			guardInterval = GUARD_INTERVAL_1_16;
			value1 = LocalLibrary::getAttribute(e, "guardinterval");
			if (value1.size()) {
				if (value1 == "1/32") {
					guardInterval = GUARD_INTERVAL_1_32;
				} else if (value1 == "1/16") {
					guardInterval = GUARD_INTERVAL_1_16;
				} else if (value1 == "1/8") {
					guardInterval = GUARD_INTERVAL_1_8;
				} else if (value1 == "1/4") {
					guardInterval = GUARD_INTERVAL_1_4;
				} else {
					cout << "output: 'guardinterval' not recognized ("
						 << value1 << ")" << endl;
					return -6;
				}
			}
			transmissionMode = TRANSMISSION_MODE_3;
			value1 = LocalLibrary::getAttribute(e, "transmissionmode");
			if (value1.size()) {
				if (value1 == "1") {
					transmissionMode = TRANSMISSION_MODE_1;
				} else if (value1 == "2") {
					transmissionMode = TRANSMISSION_MODE_2;
				} else if (value1 == "3") {
					transmissionMode = TRANSMISSION_MODE_3;
				} else if (value1 == "undefined") {
					transmissionMode = TRANSMISSION_MODE_UNDEFINED;
				} else {
					cout << "output: 'transmissionmode' not recognized ("
						 << value1 << ")" << endl;
					return -6;
				}
			}
			value1 = LocalLibrary::getAttribute(e, "usesystime");
			if (value1.size()) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				if (value1 == "true") {
					pTot->setUseCurrentTime(true);
					systimeAttrib = true;
				} else {
					pTot->setUseCurrentTime(false);
				}
			}
			value1 = LocalLibrary::getAttribute(e, "time");
			if (value1.size()) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				pTot->setTimeBegin(value1);
				if (!systimeAttrib) pTot->setUseCurrentTime(false);
			}
			value1 = LocalLibrary::getAttribute(e, "daylightsavingtime");
			if (value1.size()) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				if (value1 == "true") {
					pTot->setDaylightSavingTime(true);
				} else {
					pTot->setDaylightSavingTime(false);
				}
			}
			value1 = LocalLibrary::getAttribute(e, "countrycode");
			if (value1.size()) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				pTot->setCountryCode(LocalLibrary::upperCase(value1));
			}
			if (e->QueryAttribute("countryregionid", &num) == XML_NO_ERROR) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				pTot->setCountryRegionId(num);
			}
			value1 = LocalLibrary::getAttribute(e, "fu");
			if (value1.size()) {
				itRegion = regionList.find(value1);
				if (itRegion != regionList.end()) {
					areaCode1 = itRegion->second;
				}
			}
			value1 = LocalLibrary::getAttribute(e, "microregion");
			if (value1.size()) {
				itRegion = regionList.find(value1);
				if (itRegion != regionList.end()) {
					areaCode2 = itRegion->second;
				}
			}
			if (e->QueryAttribute("utcoffset", &num) == XML_NO_ERROR) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				pTot->setUtcOffset(num * 3600);
			}
			if (pTot) {
				(*projectList)[pTot->getId()] = pTot;
				useTot = true;
			}
			if (useTot) {
				value1 = LocalLibrary::getAttribute(e, "totlayer");
				if (value1.size()) {
					num = toLayer(value1);
					if (num != 0xFF) {
						pTot->setLayer((unsigned char)num);
						pTot->setLayerConfigured(true);
					} else {
						cout << "output: 'totlayer' not recognized ("
							 << value1 << ")" << endl;
						return -6;
					}
				}
			}
			if (useNit) {
				value1 = LocalLibrary::getAttribute(e, "nitlayer");
				if (value1.size()) {
					num = toLayer(value1);
					if (num != 0xFF) {
						pNit->setLayer((unsigned char)num);
						pNit->setLayerConfigured(true);
					} else {
						cout << "output: 'nitlayer' not recognized ("
							 << value1 << ")" << endl;
						return -6;
					}
				}
			}
			if (useSdt) {
				value1 = LocalLibrary::getAttribute(e, "sdtlayer");
				if (value1.size()) {
					num = toLayer(value1);
					if (num != 0xFF) {
						pSdt->setLayer((unsigned char)num);
						pSdt->setLayerConfigured(true);
					} else {
						cout << "output: 'sdtlayer' not recognized ("
							 << value1 << ")" << endl;
						return -6;
					}
				}
			}
			value1 = LocalLibrary::getAttribute(e, "patlayer");
			if (value1.size()) {
				num = toLayer(value1);
				if (num != 0xFF) {
					proj = (*projectList)[0];
					proj->setLayer((unsigned char)num);
					proj->setLayerConfigured(true);
				} else {
					cout << "output: 'patlayer' not recognized ("
						 << value1 << ")" << endl;
					return -6;
				}
			}

			if (e->QueryAttribute("packetsize", &num) == XML_NO_ERROR) {
				if ((num == 188) || (num == 204)) {
					packetSize = num;
				} else {
					cout << "output: 'packetsize' not recognized ("
						 << value1 << ")" << endl;
					return -6;
				}
			}
			value1 = LocalLibrary::getAttribute(e, "partialreception");
			if (value1.size()) {
				if (value1 == "true") {
					partialReception = true;
				} else {
					partialReception = false;
				}
			}

			if (packetSize == 204) {
				iip = new IIP();
				TransmissionParameters* tp;
				MCCI* mcci = new MCCI();
				ConfigurationInformation* ci = new ConfigurationInformation;
				ci->partialReceptionFlag = partialReception;
				value1 = LocalLibrary::getAttribute(e, "modulationlayera");
				if (value1.size()) {
					tp = new TransmissionParameters;
					if (value1 == "dqpsk") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_DQPSK;
					} else if (value1 == "qpsk") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_QPSK;
					} else if (value1 == "16qam") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_16QAM;
					} else if (value1 == "64qam") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_64QAM;
					} else {
						cout << "output: 'modulationlayera' not recognized ("
							 << value1 << ")" << endl;
						delete tp;
						delete mcci;
						delete iip;
						iip = NULL;
						return -6;
					}
					value1 = LocalLibrary::getAttribute(e, "codingratelayera");
					if (value1.size()) {
						if (value1 == "7/8") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_7_8;
						} else if (value1 == "5/6") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_5_6;
						} else if (value1 == "3/4") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_3_4;
						} else if (value1 == "2/3") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_2_3;
						} else if (value1 == "1/2") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_1_2;
						} else {
							cout << "output: 'codingratelayera' not recognized ("
								 << value1 << ")" << endl;
							delete tp;
							delete mcci;
							delete iip;
							iip = NULL;
							return -6;
						}
					} else {
						tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_5_6;
					}
					if (e->QueryAttribute("numsegmentlayera", &num) == XML_NO_ERROR) {
						tp->numberOfSegments = num;
					} else {
						tp->numberOfSegments = 0x01;
					}
					if (e->QueryAttribute("interleavinglayera", &num) == XML_NO_ERROR) {
						tp->lengthOfTimeInterleaving = num;
					} else {
						tp->lengthOfTimeInterleaving = 0x02;
					}
					ci->tpLayerA = tp;
				} else {
					ci->tpLayerA = NULL;
				}

				value1 = LocalLibrary::getAttribute(e, "modulationlayerb");
				if (value1.size()) {
					tp = new TransmissionParameters;
					if (value1 == "dqpsk") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_DQPSK;
					} else if (value1 == "qpsk") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_QPSK;
					} else if (value1 == "16qam") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_16QAM;
					} else if (value1 == "64qam") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_64QAM;
					} else {
						cout << "output: 'modulationlayerb' not recognized ("
							 << value1 << ")" << endl;
						delete tp;
						delete mcci;
						delete iip;
						iip = NULL;
						return -6;
					}
					value1 = LocalLibrary::getAttribute(e, "codingratelayerb");
					if (value1.size()) {
						if (value1 == "7/8") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_7_8;
						} else if (value1 == "5/6") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_5_6;
						} else if (value1 == "3/4") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_3_4;
						} else if (value1 == "2/3") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_2_3;
						} else if (value1 == "1/2") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_1_2;
						} else {
							cout << "output: 'codingratelayerb' not recognized ("
								 << value1 << ")" << endl;
							delete tp;
							delete mcci;
							delete iip;
							iip = NULL;
							return -6;
						}
					} else {
						tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_3_4;
					}
					if (e->QueryAttribute("numsegmentlayerb", &num) == XML_NO_ERROR) {
						tp->numberOfSegments = num;
					} else {
						tp->numberOfSegments = 0x0C;
					}
					if (e->QueryAttribute("interleavinglayerb", &num) == XML_NO_ERROR) {
						tp->lengthOfTimeInterleaving = num;
					} else {
						tp->lengthOfTimeInterleaving = 0x02;
					}
					ci->tpLayerB = tp;
				} else {
					ci->tpLayerB = NULL;
				}

				value1 = LocalLibrary::getAttribute(e, "modulationlayerc");
				if (value1.size()) {
					tp = new TransmissionParameters;
					if (value1 == "dqpsk") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_DQPSK;
					} else if (value1 == "qpsk") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_QPSK;
					} else if (value1 == "16qam") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_16QAM;
					} else if (value1 == "64qam") {
						tp->modulationScheme = MCCI_MODULATION_SCHEME_64QAM;
					} else {
						cout << "output: 'modulationlayerc' not recognized ("
							 << value1 << ")" << endl;
						delete tp;
						delete mcci;
						delete iip;
						iip = NULL;
						return -6;
					}
					value1 = LocalLibrary::getAttribute(e, "codingratelayerc");
					if (value1.size()) {
						if (value1 == "7/8") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_7_8;
						} else if (value1 == "5/6") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_5_6;
						} else if (value1 == "3/4") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_3_4;
						} else if (value1 == "2/3") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_2_3;
						} else if (value1 == "1/2") {
							tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_1_2;
						} else {
							cout << "output: 'codingratelayerc' not recognized ("
								 << value1 << ")" << endl;
							delete tp;
							delete mcci;
							delete iip;
							iip = NULL;
							return -6;
						}
					} else {
						tp->codingRateOfInnerCode = MCCI_CONVOLUTIONAL_CODING_RATE_3_4;
					}
					if (e->QueryAttribute("numsegmentlayerc", &num) == XML_NO_ERROR) {
						tp->numberOfSegments = num;
					} else {
						tp->numberOfSegments = 0x00;
					}
					if (e->QueryAttribute("interleavinglayerc", &num) == XML_NO_ERROR) {
						tp->lengthOfTimeInterleaving = num;
					} else {
						tp->lengthOfTimeInterleaving = 0x02;
					}
					ci->tpLayerC = tp;
				} else {
					ci->tpLayerC = NULL;
				}

				if (ci->tpLayerA || ci->tpLayerB || ci->tpLayerC) {
					mcci->setCurrentGuardInterval(guardInterval);
					//CHECK: Terrestrial Delivery System Descriptor <> MCCI
					mcci->setCurrentMode(transmissionMode + 1);
					mcci->setCurrentCI(ci);
					mcci->copyCurrentToNext();
					iip->setMcci(mcci);
					//iip is ok
				} else {
					cout << "output: hierarchical layers hasn't been defined." << endl;
					delete mcci;
					delete iip;
					iip = NULL;
					return -4;
				}

				if (tsBitrate != 29958294) {
					cout << "output: The muxer overrode your 'bitrate' " <<
								"value to 29958294 bps" << endl;
					cout << "due to ISDB-T standard." << endl;
					tsBitrate = 29958294;
				}
			}
		}
	}

	return 0;
}

int XMLProject::processOutput(XMLElement *top) {
	ProjectInfo* proj;
	XMLNode *n, *m, *o;
	XMLElement *e, *f, *g = NULL;
	string value1, value2;
	int num, id, ret;
	bool isLiveVar = false;

	for (n = top->FirstChild(); n; n = n->NextSibling()) {
		e = n->ToElement();
		if (strcmp(n->Value(), "output") == 0) {
			Timeline* timeline = NULL;
			id = getId(TIMELINE_NAME);
			if (id >= 0) {
				proj = findProject(id);
				if (proj && proj->getProjectType() == PT_TIMELINE) {
					timeline = (Timeline*) proj;
				}
			}
			if (!timeline) {
				timeline = new Timeline();
				ret = createAndGetId(timeline, TIMELINE_NAME);
				if (ret < 0) return ret;
				(*projectList)[timeline->getId()] = timeline;
			}

			timeline->setIsLoop(isLoop);

			uint64_t startTime = 0; //Each timeline duration is limited to 24 days
			for (m = e->FirstChild(); m; m = m->NextSibling()) {
				if (isLiveVar) {
					cout << "output: only the first timeline will be considered." << endl;
					break;
				}
				f = m->ToElement();
				if (strcmp(m->Value(), "item") == 0) {
					value1 = LocalLibrary::getAttribute(f, "live");
					if (value1.size()) {
						if (value1 == "true") {
							isLiveVar = true;
							isLive = true;
						}
					}
					if (isLiveVar) {
						num = 0;
						startTime = (uint64_t) Stc::stcToSecond(relStc) * 1000;
						if (startTime > 1000) {
							cout << "Live action at " << startTime/1000 << endl;
						}
					} else {
						if (f->QueryAttribute("dur", &num) != XML_NO_ERROR) {
							cout << "output: attribute 'dur' not found." << endl;
							return -4;
						}
					}
					for (o = f->FirstChild(); o; o = o->NextSibling()) {
						g = o->ToElement();
						if (strcmp(o->Value(), "pmtref") == 0) {
							map<int, PMTView*>::iterator itPMTView;
							PMTView* pPMTView;
							int prior = -1;
							value1 = LocalLibrary::getAttribute(g, "pmtid");
							value2 = LocalLibrary::getAttribute(g, "previous");
							if (value2.size()) {
								if (getId(value2) != -1) {
									prior = getId(value2);
								}
							}
							proj = findProject(getId(value1));
							if (!proj) {
								return -7;
							}
							if (proj->getProjectType() != PT_PMTVIEW) {
								cout << "output: id mismatch error." << endl;
								return -9;
							}
							pPMTView = (PMTView*) proj;
							if (pPMTView) {
								timeline->addTimeline(startTime,
													  num,
													  pPMTView,
													  prior);
							}
						}
					}
					startTime += num;
				}
			}
			if (!isLiveVar) {
				 //end of timeline (for reference purpose)
				timeline->addTimeline(startTime, 0, NULL, -1);
			}
		}
	}

	return 0;
}

int XMLProject::readHead(XMLElement *top) {
	XMLNode *n;
	XMLElement *e;
	string value1, value2;

	for (n = top->FirstChild(); n; n = n->NextSibling()) {
		e = n->ToElement();
		if (strcmp(n->Value(), "meta") == 0) {
			value1 = LocalLibrary::getAttribute(e, "name");
			value2 = LocalLibrary::getAttribute(e, "content");
			MetaData* md = new MetaData;
			md->name = value1;
			md->content = value2;
			metaDataList.push_back(md);
			if (value1 == "description") {
				projectDescription.assign(value2);
				cout << projectDescription << endl;
			}
		}
	}

	return 0;
}

int XMLProject::readBody(XMLElement *top) {
	int ret;

	ret = processOutputProperties(top);
	if (ret < 0) return ret;
	ret = processInputs(top);
	if (ret < 0) return ret;
	return processOutput(top);
}

int XMLProject::readFile() {
	enum XMLError err;
	XMLElement *top;
	XMLElement *e;
	string value;

	if (filename.empty()) return -1;
	err = xmldoc->LoadFile(filename.c_str());
	if (err != XML_SUCCESS) {
		return -2;
	}

	changeToProjectDir();

	pSdt = NULL;

	e = xmldoc->FirstChildElement("tmm");
	if (e) {
		value = LocalLibrary::getAttribute(e, "projectname");
		if (!value.empty()) {
			projectName = value;
			cout << projectName << endl;
		}

		top = e->FirstChildElement("head");
		if (top) {
			readHead(top);
		}

		top = xmldoc->FirstChildElement("tmm")->FirstChildElement("body");
		if (top) {
			return readBody(top);
		} else {
			return -10;
		}
	} else {
		return -11;
	}
}

int XMLProject::readLiveStream() {
	if (!liveServer.checkMemoryDescriptor()) {
		return -1;
	} else {
		const char* buffer;
		int bufferSize;

		bufferSize = liveServer.readSharedMemory(&buffer);
		liveServer.grantAccessToForeign();

		if (bufferSize <= 0) {
			return -1;
		} else {
			enum XMLError err;
			XMLElement *top;
			XMLElement *e;
			string value;
			int ret;

			xmldoc->Clear();
			err = xmldoc->Parse(buffer, bufferSize);
			if (err != XML_SUCCESS) {
				return -2;
			}

			e = xmldoc->FirstChildElement("tmm");
			if (e) {
				top = xmldoc->FirstChildElement("tmm")->FirstChildElement("body");
				if (top) {
					ret = processInputs(top);
					if (ret < 0) return ret;
					return processOutput(top);
				} else {
					return -10;
				}
			} else {
				return -11;
			}
		}
	}
	return 0;
}

}
}
}
}
