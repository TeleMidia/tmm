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

XMLProject::XMLProject(string filename) : Project() {
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
}

string XMLProject::getAttribute(XMLElement* e, string name) {
	string str = "";
	if (e) {
		const char *s = e->Attribute(name.c_str());
		if (s) str.assign(s);
	}
	return str;
}

string XMLProject::getElementText(XMLElement* e) {
	string str;
	XMLText* text = e->FirstChild()->ToText();
	str.assign(text->Value());
	return str;
}

vector<MetaData*>* XMLProject::getMetaDataList() {
	return &metaDataList;
}

bool XMLProject::createNewId(string id){
	if (idList.count(id)) {
		cout << "The id = " << id << " already exists." << endl;
		return false;
	} else {
		idList[id] = idIndex++;
		return true;
	}
}

int XMLProject::getId(string id) {
	if (idList.count(id)) {
		return idList[id];
	} else {
		return -1;
	}
}

int XMLProject::createAndGetId(ProjectInfo* proj, string name) {
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
		value = getAttribute(f, "id");
		InputData* input = new InputData();
		ret = createAndGetId(input, value);
		if (ret < 0) {
			delete input;
			return ret;
		}
		value = getAttribute(f, "src");
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
		value = getAttribute(f, "id");
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
				value = getAttribute(g, "groupid");
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

int XMLProject::parseCarousel(XMLNode* m, XMLElement* f) {
	string value;
	int num, ret;

	if (strcmp(m->Value(), "carousel") == 0) {
		value = getAttribute(f, "id");
		PCarousel* carousel = new PCarousel();
		ret = createAndGetId(carousel, value);
		if (ret < 0) {
			delete carousel;
			return ret;
		}
		value = getAttribute(f, "src");
		carousel->setServiceGatewayFolder(value);
		if (f->QueryAttribute("bitrate", &num) != XML_NO_ERROR) {
			cout << "carousel: attribute 'bitrate' not found." << endl;
			return -4;
		}
		carousel->setBitrate(num);
		if (f->QueryAttribute("servicedomain", &num) != XML_NO_ERROR) {
			cout << "carousel: attribute 'servicedomain' not found." << endl;
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
		value = getAttribute(f, "id");
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
				unsigned int orgId, appId, appcode;
				string appname, lang, basedir, entrypoint;
				value = getAttribute(g, "carouselid");
				num = getId(value);
				if (num == -1) {
					cout << "The id = " << value << " doesn't exists." << endl;
					return -8;
				}
				value = getAttribute(g, "apptype");
				if (value == "ginga-ncl") {
					ait->setTableIdExtension(AT_GINGA_NCL);
				} else if (value == "ginga-j") {
					ait->setTableIdExtension(AT_GINGA_J);
				} else {
					cout << "ait: 'apptype' not recognized ("
						 << value << ")" << endl;
					return -6;
				}
				appcode = CC_AUTO_START;
				value = getAttribute(g, "appcontrolcode");
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
				if (g->QueryAttribute("organizationid", &orgId) != XML_NO_ERROR) {
					cout << "ait: attribute 'organizationid' not found." << endl;
					return -4;
				}
				if (g->QueryAttribute("applicationid", &appId) != XML_NO_ERROR) {
					cout << "ait: attribute 'applicationid' not found." << endl;
					return -4;
				}
				appname = getAttribute(g, "appname");
				value = getAttribute(g, "language");
				if (value.size()) {
					lang = value;
				} else {
					lang.assign("por");
				}
				value = getAttribute(g, "basedirectory");
				if (value.size()) {
					basedir = value;
				} else {
					basedir.assign("/");
				}
				entrypoint = getAttribute(g, "entrypoint");
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
							entrypoint, orgId, appId, appcode);
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
		value = getAttribute(f, "id");
		ret = createAndGetId(eit, value);
		if (ret < 0) {
			delete eit;
			return ret;
		}
		value = getAttribute(f, "layer");
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
				value = getAttribute(g, "time");
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
						value = getAttribute(h, "name");
						if (value.size() > 96) {
							cout << "eit: 'name' value is too long." << endl;
							return -12;
						}
						value2 = getAttribute(h, "text");
						if (value2.size() > 192) {
							cout << "eit: 'text' value is too long." << endl;
							return -12;
						}
						ShortEvent* si = new ShortEvent();
						si->setEventName(value);
						si->setText(value2);
						value = getAttribute(h, "language");
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
						value = getAttribute(h, "language");
						if (value.size()) {
							if (value.size() != 3) {
								cout << "eit: 'language' value has 3 letters." << endl;
								return -12;
							}
							comp->setIso639LanguageCode(value);
						}
						value2 = getAttribute(h, "text");
						if (value2.size() > 192) {
							cout << "eit: 'text' value is too long." << endl;
							return -12;
						}
						comp->setText(value2);
						ei->descriptorsList.push_back(comp);
					}
					if (strcmp(p->Value(), "parentalrating") == 0) {
						int rating;
						if (h->QueryAttribute("rating", &rating) != XML_NO_ERROR) {
							cout << "eit: attribute 'rating' not found." << endl;
							return -4;
						}
						ParentalRatingInfo* pri = new ParentalRatingInfo();
						pri->rating = rating;
						value = getAttribute(h, "countrycode");
						if (value.size()) {
							if (value.size() != 3) {
								cout << "eit: 'countrycode' value has 3 letters." << endl;
								return -12;
							}
							pri->countryCode = value;
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

	if (strcmp(m->Value(), "pmt") == 0) {
		PMTView* pmtView = new PMTView();
		value = getAttribute(f, "id");
		ret = createAndGetId(pmtView, value);
		if (ret < 0) {
			delete pmtView;
			return ret;
		}
		/*
		 * Program number -> ABNTNBR 15603-2, Annex H.3
		 */
		if (f->QueryAttribute("programnumber", &num) != XML_NO_ERROR) {
			cout << "pmt: attribute 'programnumber' not found." << endl;
			return -4;
		}
		if (num == 0) {
			cout << "pmt: programnumber = 0 is not allowed." << endl;
			return -4;
		}
		pmtView->setProgramNumber(num);
		if (f->QueryAttribute("pcrpid", &num) != XML_NO_ERROR) {
			cout << "pmt: attribute 'pcrpid' not found." << endl;
			return -4;
		}
		pmtView->setPcrPid(num);
		if (f->QueryAttribute("pcrfrequency", &num) == XML_NO_ERROR) {
			pmtView->setPcrFrequency(num);
		} else {
			pmtView->setPcrFrequency(60);
		}
		value = getAttribute(f, "name");
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
		value = getAttribute(f, "servicetype");
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
				pmtView->setPid(0x1FC8);
				pmtView->setLayer(HIERARCHY_A);
			} else {
				cout << "pmt: 'servicetype' not recognized ("
					 << value << ")" << endl;
				return -6;
			}
			pmtView->setLayerConfigured(true);
		}
		if (pmtView->getServiceType() != SRV_TYPE_ONESEG) {
			if (f->QueryAttribute("pid", &num) != XML_NO_ERROR) {
				cout << "pmt: attribute 'pid' not found." << endl;
				return -4;
			}
			pmtView->setPid(num);
		} else {
			if (f->QueryAttribute("pid", &num) == XML_NO_ERROR) {
				if (num != 0x1FC8) {
					cout << "pmt: the one-seg service overrode your 'pid' " <<
							"value to 0x1FC8." << endl;
				}
			}
		}
		value = getAttribute(f, "layer");
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
		value = getAttribute(f, "eitid");
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
				value = getAttribute(g, "refid");
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
				value = getAttribute(g, "layer");
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
						value = getAttribute(h, "language");
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
			for (parseNum = 0; parseNum < 6; parseNum++) {
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
						ret = parseEIT(m, f);
						break;
					case 5:
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

int XMLProject::processOutput(XMLElement *top) {
	ProjectInfo* proj;
	XMLNode *n, *m, *o;
	XMLElement *e, *f, *g = NULL;
	string value1, value2;
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
			value1 = getAttribute(e, "loop");
			if (value1 == "true") {
				isLoop = true;
			}
			if (e->QueryAttribute("ttl", &num) == XML_NO_ERROR) {
				ttl = num;
			}
			destination = getAttribute(e, "dest");
			if (e->QueryAttribute("tsid", &num) == XML_NO_ERROR) {
				tsid = num;
			} else {
				tsid = 1;
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
			if (e->QueryAttribute("layerratea", &num) == XML_NO_ERROR) {
				layerBitrateA = num;
			} else {
				layerBitrateA = 500000;
			}
			if (e->QueryAttribute("layerrateb", &num) == XML_NO_ERROR) {
				layerBitrateB = num;
			} else {
				layerBitrateB = 17500000;
			}
			if (e->QueryAttribute("layerratec", &num) == XML_NO_ERROR) {
				layerBitrateC = num;
			} else {
				layerBitrateC = 0;
			}
			value1 = getAttribute(e, "name");
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
			if (e->QueryAttribute("originalnetworkid", &num) == XML_NO_ERROR) {
				originalNetworkId = num;
			} else {
				originalNetworkId = tsid;
			}
			value1 = getAttribute(e, "tsname");
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
			value1 = getAttribute(e, "guardinterval");
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
			value1 = getAttribute(e, "transmissionmode");
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
			value1 = getAttribute(e, "usesystime");
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
			value1 = getAttribute(e, "time");
			if (value1.size()) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				pTot->setTimeBegin(value1);
				if (!systimeAttrib) pTot->setUseCurrentTime(false);
			}
			value1 = getAttribute(e, "daylightsavingtime");
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
			value1 = getAttribute(e, "countrycode");
			if (value1.size()) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				pTot->setCountryCode(value1);
			}
			if (e->QueryAttribute("countryregionid", &num) == XML_NO_ERROR) {
				if (!pTot) {
					pTot = new PTot();
					ret = createAndGetId(pTot, TOT_NAME);
					if (ret < 0) return ret;
				}
				pTot->setCountryRegionId(num);
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
				value1 = getAttribute(e, "totlayer");
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
				value1 = getAttribute(e, "nitlayer");
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
				value1 = getAttribute(e, "sdtlayer");
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
			value1 = getAttribute(e, "patlayer");
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

			unsigned int startTime = 0; //Each timeline duration is limited to 24 days
			for (m = e->FirstChild(); m; m = m->NextSibling()) {
				f = m->ToElement();
				if (strcmp(m->Value(), "item") == 0) {
					if (f->QueryAttribute("dur", &num) != XML_NO_ERROR) {
						cout << "output: attribute 'dur' not found." << endl;
						return -4;
					}
					for (o = f->FirstChild(); o; o = o->NextSibling()) {
						g = o->ToElement();
						if (strcmp(o->Value(), "pmtref") == 0) {
							map<int, PMTView*>::iterator itPMTView;
							PMTView* pPMTView;
							int prior = -1;
							value1 = getAttribute(g, "pmtid");
							value2 = getAttribute(g, "previous");
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
			timeline->addTimeline(startTime, //end of timeline (for reference purpose)
								  0,
								  NULL,
								  -1);
			if (packetSize == 204) {
				PIIP* piip = new PIIP();
				piip->setLayerConfigured(true);
				ret = createAndGetId(piip, IIP_NAME);
				if (ret < 0) return ret;

				TransmissionParameters* tp;
				MCCI* mcci = new MCCI();
				ConfigurationInformation* ci = new ConfigurationInformation;
				ci->partialReceptionFlag = true;
				value1 = getAttribute(e, "partialreception");
				if (value1.size()) {
					if (value1 == "true") {
						ci->partialReceptionFlag = true;
					} else {
						ci->partialReceptionFlag = false;
					}
				}
				value1 = getAttribute(e, "modulationlayera");
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
						delete piip;
						return -6;
					}
					value1 = getAttribute(e, "codingratelayera");
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
							delete piip;
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

				value1 = getAttribute(e, "modulationlayerb");
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
						delete piip;
						return -6;
					}
					value1 = getAttribute(e, "codingratelayerb");
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
							delete piip;
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

				value1 = getAttribute(e, "modulationlayerc");
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
						delete piip;
						return -6;
					}
					value1 = getAttribute(e, "codingratelayerc");
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
							delete piip;
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
					//TODO: Terrestrial Delivery System Descriptor <> MCCI
					mcci->setCurrentMode(transmissionMode + 1);
					mcci->setCurrentCI(ci);
					mcci->copyCurrentToNext();
					piip->setMcci(mcci);

					(*projectList)[piip->getId()] = piip;
				} else {
					cout << "output: hierarchical layers hasn't been defined." << endl;
					delete mcci;
					delete piip;
					return -4;
				}
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
			value1 = getAttribute(e, "name");
			value2 = getAttribute(e, "content");
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

	ret = processInputs(top);
	if (ret < 0) return ret;
	ret = processOutput(top);
	return ret;
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
		value = getAttribute(e, "projectname");
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


}
}
}
}
