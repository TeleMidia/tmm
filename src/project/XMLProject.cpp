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
	xmldoc = new XMLDocument();
	projectName.assign("Untitled project");
	projectDescription.clear();
	idIndex = 0;
}

XMLProject::XMLProject(string filename) : Project() {
	xmldoc = new XMLDocument();
	this->filename = filename;
	projectName.assign("Untitled project");
	projectDescription.clear();
	idIndex = 0;
}

XMLProject::XMLProject(const char* filename) : Project() {
	xmldoc = new XMLDocument();
	this->filename.assign(filename);
	projectName.assign("Untitled project");
	projectDescription.clear();
	idIndex = 0;
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

int XMLProject::readFile() {
	enum XMLError err;
	bool hasInput = false;
	ProjectInfo* proj;

	if (filename.empty()) return -1;
	err = xmldoc->LoadFile(filename.c_str());
	if (err != XML_SUCCESS) {
		return -2;
	}

	XMLNode *n, *m, *o, *p;
	XMLElement *top, *e, *f, *g = NULL, *h;
	string value, value1, value2;
	int num, id;
	double dnum;

	e = xmldoc->FirstChildElement("tmm");
	if (e) {
		value = getAttribute(e, "projectname");
		if (!value.empty()) {
			projectName = value;
			cout << projectName << endl;
		}

		top = e->FirstChildElement("head");
		if (top) {
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
		}

		top = xmldoc->FirstChildElement("tmm")->FirstChildElement("body");
		if (top) {
			for (n = top->FirstChild(); n; n = n->NextSibling()) {
				e = n->ToElement();
				if ((strcmp(n->Value(), "inputs") == 0) && (!hasInput)) {
					hasInput = true;
					for (m = e->FirstChild(); m; m = m->NextSibling()) {
						f = m->ToElement();
						if (strcmp(m->Value(), "av") == 0) {
							value1 = getAttribute(f, "id");
							if (!createNewId(value1)) return -3;
							id = getId(value1);
							if (id == -1) {
								cout << "The id = " << value1 << " doesn't exists." << endl;
								return -8;
							}
							InputData* input = new InputData(id);
							value1 = getAttribute(f, "src");
							input->setFilename(value1);
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
							(*projectList)[id] = input;
						}
					}

					for (m = e->FirstChild(); m; m = m->NextSibling()) {
						f = m->ToElement();
						if (strcmp(m->Value(), "npt") == 0) {
							value1 = getAttribute(f, "id");
							if (!createNewId(value1)) return -3;
							id = getId(value1);
							if (id == -1) {
								cout << "The id = " << value1 << " doesn't exists." << endl;
								return -8;
							}
							NPTProject* npt = new NPTProject(id);
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
									value1 = getAttribute(g, "groupid");
									if (!createNewId(value1)) return -3;
									num = getId(value1);
									if (num == -1) {
										cout << "The id = " << value1 << " doesn't exists." << endl;
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
							(*projectList)[id] = npt;
						}
						if (strcmp(m->Value(), "carousel") == 0) {
							value1 = getAttribute(f, "id");
							if (!createNewId(value1)) return -3;
							id = getId(value1);
							if (id == -1) {
								cout << "The id = " << value1 << " doesn't exists." << endl;
								return -8;
							}
							PCarousel* carousel = new PCarousel();
							carousel->setId(id);
							value1 = getAttribute(f, "src");
							carousel->setServiceGatewayFolder(value1);
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
							(*projectList)[id] = carousel;
						}
					}

					for (m = e->FirstChild(); m; m = m->NextSibling()) {
						f = m->ToElement();
						if (strcmp(m->Value(), "ait") == 0) {
							PAit *ait = new PAit();
							ait->setCurrentNextIndicator(1);
							ait->setSectionSyntaxIndicator(1);
							ait->setPrivateIndicator(1);
							value1 = getAttribute(f, "id");
							if (!createNewId(value1)) return -3;
							id = getId(value1);
							if (id == -1) {
								cout << "The id = " << value1 << " doesn't exists." << endl;
								return -8;
							}
							if (f->QueryAttribute("transmissiondelay", &num) == XML_NO_ERROR) {
								ait->setTransmissionDelay((double) num / 1000);
							}
							for (o = f->FirstChild(); o; o = o->NextSibling()) {
								g = o->ToElement();
								if (strcmp(o->Value(), "carouselref") == 0) {
									unsigned int orgId, appId, appcode;
									string appname, lang, basedir, entrypoint;
									value1 = getAttribute(g, "carouselid");
									num = getId(value1);
									if (num == -1) {
										cout << "The id = " << value1 << " doesn't exists." << endl;
										return -8;
									}
									value1 = getAttribute(g, "apptype");
									if (value1 == "ginga-ncl") {
										ait->setTableIdExtension(AT_GINGA_NCL);
									} else if (value1 == "ginga-j") {
										ait->setTableIdExtension(AT_GINGA_J);
									} else {
										cout << "ait: 'apptype' not recognized ("
											 << value1 << ")" << endl;
										return -6;
									}
									appcode = CC_AUTO_START;
									value1 = getAttribute(g, "appcontrolcode");
									if (value1.size()) {
										if (value1 == "autostart") {
											appcode = CC_AUTO_START;
										} else if (value1 == "present") {
											appcode = CC_PRESENT;
										} else if (value1 == "destroy") {
											appcode = CC_DESTROY;
										} else if (value1 == "kill") {
											appcode = CC_KILL;
										} else if (value1 == "remote") {
											appcode = CC_REMOTE;
										} else if (value1 == "unbound") {
											appcode = CC_UNBOUND;
										} else {
											cout << "ait: 'appcontrolcode' not recognized ("
												 << value1 << ")" << endl;
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
									value1 = getAttribute(g, "language");
									if (value1.size()) {
										lang = value1;
									} else {
										lang.assign("por");
									}
									value1 = getAttribute(g, "basedirectory");
									if (value1.size()) {
										basedir = value1;
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
										configAit(ait, ((PCarousel*)proj)->getServiceDomain(),
												appname, lang, basedir,
												entrypoint, orgId, appId, appcode);
										(*projectList)[id] = ait;
									} else {
										delete ait;
										return false;
									}
								}
							}
						}
					}

					for (m = e->FirstChild(); m; m = m->NextSibling()) {
						f = m->ToElement();
						if (strcmp(m->Value(), "eit") == 0) {
							PEit *eit = new PEit();
							eit->setCurrentNextIndicator(1);
							eit->setSectionSyntaxIndicator(1);
							eit->setPrivateIndicator(1);
							value1 = getAttribute(f, "id");
							if (!createNewId(value1)) return -3;
							id = getId(value1);
							if (id == -1) {
								cout << "The id = " << value1 << " doesn't exists." << endl;
								return -8;
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
									value1 = getAttribute(g, "time");
									if (value1.size()) {
										stime = PTot::makeUtcDate(value1);
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
											value1 = getAttribute(h, "name");
											if (value1.size() > 96) {
												cout << "eit: 'name' value is too long." << endl;
												return -12;
											}
											value2 = getAttribute(h, "text");
											if (value2.size() > 192) {
												cout << "eit: 'text' value is too long." << endl;
												return -12;
											}
											ShortEvent* si = new ShortEvent();
											si->setEventName(value1);
											si->setText(value2);
											value1 = getAttribute(h, "language");
											if (value1.size()) {
												if (value1.size() != 3) {
													cout << "eit: 'language' value has 3 letters." << endl;
													return -12;
												}
												si->setIso639LanguageCode(value1);
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
											value1 = getAttribute(h, "language");
											if (value1.size()) {
												if (value1.size() != 3) {
													cout << "eit: 'language' value has 3 letters." << endl;
													return -12;
												}
												comp->setIso639LanguageCode(value1);
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
											value1 = getAttribute(h, "countrycode");
											if (value1.size()) {
												if (value1.size() != 3) {
													cout << "eit: 'countrycode' value has 3 letters." << endl;
													return -12;
												}
												pri->countryCode = value1;
											}
											ParentalRating* pr = new ParentalRating();
											pr->addParentalRating(pri);
											ei->descriptorsList.push_back(pr);
										}
									}
									eit->addEventInfo(ei);
								}
							}
							(*projectList)[id] = eit;
						}
					}

					for (m = e->FirstChild(); m; m = m->NextSibling()) {
						PSdt* pSdt = NULL;
						f = m->ToElement();
						if (strcmp(m->Value(), "pmt") == 0) {
							PMTView* pmtView = new PMTView();
							value1 = getAttribute(f, "id");
							if (!createNewId(value1)) return -3;
							id = getId(value1);
							if (id == -1) {
								cout << "The id = " << value1 << " doesn't exists." << endl;
								return -8;
							}
							pmtView->setId(id);
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
							value1 = getAttribute(f, "name");
							if (value1.size() > 20) {
								cout << "pmt: 'name' value is too long." << endl;
								return -12;
							}
							if (value1.size()) {
								pmtView->setServiceName(value1);
								if (!useSdt) {
									int id2;
									value1 = SDT_NAME;
									if (!createNewId(value1)) return -3;
									pSdt = new PSdt();
									id2 = getId(value1);
									(*projectList)[id2] = pSdt;
								}
								useSdt = true;
							} else {
								pmtView->setServiceName("Unnamed service");
							}
							value1 = getAttribute(f, "servicetype");
							if (value1.size()) {
								if (value1 == "tv") {
									pmtView->setServiceType(SRV_TYPE_TV);
								} else if (value1 == "data1") {
									pmtView->setServiceType(SRV_TYPE_DATA1);
								} else if (value1 == "data2") {
									pmtView->setServiceType(SRV_TYPE_DATA2);
								} else if (value1 == "oneseg") {
									pmtView->setServiceType(SRV_TYPE_ONESEG);
									pmtView->setPid(0x1FC8);
								} else {
									cout << "pmt: 'servicetype' not recognized ("
										 << value1 << ")" << endl;
									return -6;
								}
							}
							if (pmtView->getServiceType() != SRV_TYPE_ONESEG) {
								if (f->QueryAttribute("pid", &num) != XML_NO_ERROR) {
									cout << "pmt: attribute 'pid' not found." << endl;
									return -4;
								}
								pmtView->setPid(num);
							} else {
								if (f->QueryAttribute("pid", &num) == XML_NO_ERROR) {
									cout << "pmt: the one-seg service overrode your 'pid' " <<
											"value to 0x1FC8." << endl;
								}
							}
							value1 = getAttribute(f, "eitid");
							if (value1.size()) {
								num = getId(value1);
								if (num == -1) {
									cout << "The id = " << value1 << " doesn't exists." << endl;
									return -8;
								}
								proj = findProject(num);
								if (proj) {
									pmtView->setEitProj(proj);
								} else return -7;
							}
							for (o = f->FirstChild(); o; o = o->NextSibling()) {
								int esPid;
								g = o->ToElement();
								if (strcmp(o->Value(), "es") == 0) {
									value1 = getAttribute(g, "refid");
									proj = findProject(getId(value1));
									if (!proj) {
										cout << "pmt: attribute 'refid' not found ("
											 << value1 << ")" << endl;
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
								}
							}
							(*projectList)[id] = pmtView;
						}
					}
				}

				if ((strcmp(n->Value(), "output") == 0) && (hasInput)) {
					PTot* pTot = NULL;
					PNit* pNit = NULL;
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
						tsBitrate = 19000000;
					}
					value1 = getAttribute(e, "name");
					if (value1.size() > 20) {
						cout << "output: 'name' value is too long." << endl;
						return -12;
					}
					if (value1.size()) {
						providerName = value1;
						if (!useNit) {
							int id2;
							value1 = NIT_NAME;
							if (!createNewId(value1)) return -3;
							pNit = new PNit();
							id2 = getId(value1);
							(*projectList)[id2] = pNit;
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
							value2 = TOT_NAME;
							if (!createNewId(value2)) return -3;
							id = getId(value2);
							if (id == -1) {
								cout << "The id = " << value2 << " doesn't exists." << endl;
								return -8;
							}
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
							value2 = TOT_NAME;
							if (!createNewId(value2)) return -3;
							id = getId(value2);
							if (id == -1) {
								cout << "The id = " << value2 << " doesn't exists." << endl;
								return -8;
							}
						}
						pTot->setTimeBegin(value1);
						if (!systimeAttrib) pTot->setUseCurrentTime(false);
					}
					value1 = getAttribute(e, "daylightsavingtime");
					if (value1.size()) {
						if (!pTot) {
							pTot = new PTot();
							value2 = TOT_NAME;
							if (!createNewId(value2)) return -3;
							id = getId(value2);
							if (id == -1) {
								cout << "The id = " << value2 << " doesn't exists." << endl;
								return -8;
							}
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
							value2 = TOT_NAME;
							if (!createNewId(value2)) return -3;
							id = getId(value2);
							if (id == -1) {
								cout << "The id = " << value1 << " doesn't exists." << endl;
								return -8;
							}
						}
						pTot->setCountryCode(value1);
					}
					if (e->QueryAttribute("countryregionid", &num) == XML_NO_ERROR) {
						if (!pTot) {
							pTot = new PTot();
							value2 = TOT_NAME;
							if (!createNewId(value2)) return -3;
							id = getId(value2);
							if (id == -1) {
								cout << "The id = " << value2 << " doesn't exists." << endl;
								return -8;
							}
						}
						pTot->setCountryRegionId(num);
					}
					if (e->QueryAttribute("utcoffset", &num) == XML_NO_ERROR) {
						if (!pTot) {
							pTot = new PTot();
							value2 = TOT_NAME;
							if (!createNewId(value2)) return -3;
							id = getId(value2);
							if (id == -1) {
								cout << "The id = " << value2 << " doesn't exists." << endl;
								return -8;
							}
						}
						pTot->setUtcOffset(num * 3600);
					}
					if (pTot) {
						(*projectList)[id] = pTot;
						useTot = true;
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
						value1 = TIMELINE_NAME;
						if (!createNewId(value1)) return -3;
						id = getId(value1);
						if (id == -1) {
							cout << "The id = " << value1 << " doesn't exists." << endl;
							return -8;
						}
						(*projectList)[id] = timeline;
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
				}
			}
		} else {
			return -10;
		}
	} else {
		return -11;
	}

	return 0;
}


}
}
}
}
