/*
 * NPTProject.cpp
 *
 *  Created on: 28/02/2013
 *      Author: Felippe Nagato
 */

#include "project/npt/NPTProject.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

NPTProject::NPTProject() {
	referenceList = new map<int, vector<Reference*>*>;
	endPointList = new map<int, Reference*>;
	currRefList = new vector<Reference*>;
	firstReference = 0;
	priorTime = Stc::secondToStc(2.0);
	projectType = PT_NPT;
}

NPTProject::NPTProject(int id) {
	this->id = id;
	referenceList = new map<int, vector<Reference*>*>;
	endPointList = new map<int, Reference*>;
	currRefList = new vector<Reference*>;
	firstReference = 0;
	priorTime = Stc::secondToStc(2.0);
	projectType = PT_NPT;
}

NPTProject::~NPTProject() {
	map<int, vector<Reference*>*>::iterator itRef;
	vector<Reference*>::iterator it;
	map<int, Reference*>::iterator itInt;

	itRef = referenceList->begin();
	while (itRef != referenceList->end()) {
		it = itRef->second->begin();
		while (it != itRef->second->end()) {
			delete (*it);
			++it;
		}
		++itRef;
	}
	delete referenceList;

	itInt = endPointList->begin();
	while (itInt != endPointList->end()) {
		delete (itInt->second);
		++itInt;
	}
	delete endPointList;

	delete currRefList;
}

bool NPTProject::addReference(int id, char cid, int64_t absStart, int64_t absEnd,
							  int64_t nptStart, int64_t nptEnd, short numerator,
						      unsigned short denominator, bool hasNext) {
	map<int, vector<Reference*>*>::iterator itRef;
	vector<Reference*>::iterator it;
	vector<Reference*>* refList;
	Reference *ref;

	itRef = referenceList->find(id);
	if (itRef == referenceList->end()) {
		refList = new vector<Reference*>;
		(*referenceList)[id] = refList;
	} else {
		refList = itRef->second;
	}

	ref = new Reference();
	ref->setId(id);
	ref->setContentId(cid);
	ref->setAbsStart(absStart);
	ref->setAbsEnd(absEnd);
	ref->setNptStart(nptStart);
	ref->setNptEnd(nptEnd);
	ref->setNumerator(numerator);
	ref->setDenominator(denominator);
	ref->setHasNext(hasNext);

	refList->push_back(ref);

	return true;
}

bool NPTProject::removeReferences(int id) {
	map<int, vector<Reference*>*>::iterator itRef;
	vector<Reference*>::iterator it;

	itRef = referenceList->find(id);
	if (itRef != referenceList->end()) {
		it = itRef->second->begin();
		while (it != itRef->second->end()) {
			delete (*it);
			++it;
		}
		delete itRef->second;
		referenceList->erase(itRef);
		return true;
	}

	return false;
}

bool NPTProject::addEndpoint(int id, int64_t nptStart, int64_t nptEnd) {
	map<int, Reference*>::iterator itEP;
	Reference *ref;

	itEP = endPointList->find(id);
	if (itEP != endPointList->end()) {
		delete itEP->second;
	}

	ref = new Reference();
	ref->setId(id);
	ref->setNptStart(nptStart);
	ref->setNptEnd(nptEnd);

	(*endPointList)[id] = ref;

	return true;
}

bool NPTProject::removeEndPoint(int id) {
	map<int, Reference*>::iterator itEP;

	itEP = endPointList->find(id);
	if (itEP != endPointList->end()) {
		delete itEP->second;
		endPointList->erase(itEP);
		return true;
	}

	return false;
}

vector<Reference*>* NPTProject::getCurrRef(int64_t currStc) {
	map<int, vector<Reference*>*>::iterator itRef;
	vector<Reference*>::iterator it;
	int64_t temp;
	bool hasTransition = false;
	bool deleted = true;

	currRefList->clear();
	itRef = referenceList->begin();
	while (itRef != referenceList->end()) {
		it = itRef->second->begin();
		while (it != itRef->second->end()) {
			if (((*it)->getAbsStart() + firstReference) >= priorTime) {
				temp = (*it)->getAbsStart() + firstReference - priorTime;
			} else {
				temp = 0;
			}
			if ((currStc >= ((*it)->getAbsStart() + firstReference)) &&
				(currStc < ((*it)->getAbsEnd() + firstReference))) {
				//inside interval
				(*it)->setInInterval(true);
				currRefList->push_back(*it);
			} else if ((currStc >= temp) &&
					   (currStc < ((*it)->getAbsStart() + firstReference))) {
				//inside transition
				(*it)->setInTransition(true);
				hasTransition = true;
				currRefList->push_back(*it);
			}
			++it;
		}
		++itRef;
	}

	while (hasTransition && deleted) {
		deleted = false;
		it = currRefList->begin();
		while (it != currRefList->end()) {
			if ((*it)->getInInterval()) {
				currRefList->erase(it);
				deleted = true;
				break;
			}
			++it;
		}
	}
	return currRefList;
}

void NPTProject::setFirstReference(int64_t first) {
	firstReference = first;
}

int NPTProject::encodeSections(int64_t stc, vector<PrivateSection*>* list) {
	NPTReference* nptRef = NULL;
	NPTEndpoint* nptEP = NULL;
	Reference* ref;
	int64_t nptValue;
	bool transition, hasDesc = false;
	vector<Reference*>* currRefList;
	vector<Reference*>::iterator it;
	map<int, Reference*>::iterator itssl;
	DSMCCSection* dsmccSection = NULL;

	dsmccSection = new DSMCCSection();
	currRefList = getCurrRef(stc);
	it = currRefList->begin();
	while (it != currRefList->end()) {
		ref = *it;
		hasDesc = true;
		transition = (stc < (ref->getAbsStart() + firstReference));

		//cout << "NPT CID = " << dec << (ref->getContentId() & 0xFF);

		nptRef = new NPTReference();
		nptRef->setContentId(ref->getContentId());
		if (transition) {
			nptRef->setStcRef(Stc::stcToBase(ref->getAbsStart() + firstReference));
			nptValue = ref->getNptStart();
			//cout << "; (NPT reference transition)";
		} else {
			nptRef->setStcRef(Stc::stcToBase(stc));
			nptValue = (stc - (ref->getAbsStart() + firstReference));
			nptValue = (nptValue*((double)ref->getNumerator()/ref->getDenominator()))+0.5f;
			nptValue = nptValue + ref->getNptStart();
			if (ref->getNumerator() > 0) {
				//Timebase is active
				itssl = endPointList->find(ref->getId());
				if (itssl != endPointList->end()) {
					nptEP = new NPTEndpoint();
					nptEP->setStartNPT(Stc::stcToBase(itssl->second->getNptStart()));
					nptEP->setStopNPT(Stc::stcToBase(itssl->second->getNptEnd()));
					//cout << endl << "startNPT = " << nptEP->getStartNPT() << endl;
					//cout << "stopNPT = " << nptEP->getStopNPT() << endl;
				}
			}
		}
		//cout << "; NPT = " << (double) nptValue / 1000.0;
		//cout << "; STC = " << Stc::baseToSecond(nptRef->getStcRef()) << endl;
		nptRef->setNptRef(Stc::stcToBase(nptValue));
		nptRef->setScaleNumerator(ref->getNumerator());
		nptRef->setScaleDenominator(ref->getDenominator());

		dsmccSection->addDsmccDescriptor(nptRef);
		if (nptEP != NULL) {
			dsmccSection->addDsmccDescriptor(nptEP);
			nptEP = NULL;
		}

		++it;
	}

	if (hasDesc) {
		dsmccSection->setTableId(0x3D);
		dsmccSection->setSectionSyntaxIndicator(1);
		dsmccSection->setTableIdExtension(0x00);
		dsmccSection->setCurrentNextIndicator(1);
		dsmccSection->setSectionNumber(0x00);
		dsmccSection->setLastSectionNumber(0x00);
		dsmccSection->setVersionNumber(0);//TODO: changes in descriptors must change this
		dsmccSection->updateStream();
		list->push_back(dsmccSection);
	} else {
		delete (dsmccSection);
	}
	return 0;
}

}
}
}
}


