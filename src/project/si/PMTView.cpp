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
}

PMTView::~PMTView() {
	deleteAllStreams();
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

void PMTView::setPcrFrequency(unsigned short freq) {
	pcrFrequency = freq;
}

unsigned short PMTView::getPcrFrequency() {
	return pcrFrequency;
}

void PMTView::setServiceName(string name) {
	serviceName = name;
}

string PMTView::getServiceName() {
	return serviceName;
}

bool PMTView::addProjectInfo(unsigned short pid, ProjectInfo* projInfo) {
	projectInfoList[pid] = projInfo;
	return true;
}

bool PMTView::addStream(unsigned short pid, Stream* stream) {
	streamList[pid] = stream;
	return true;
}

bool PMTView::deleteAllStreams() {
	for (unsigned int i = 0; i < streamList.size(); i++) {
		if (streamList[i]) delete streamList[i];
	}
	streamList.clear();
	return true;
}

map<unsigned short, ProjectInfo*>* PMTView::getProjectInfoList() {
	return &projectInfoList;
}

map<unsigned short, Stream*>* PMTView::getStreamList() {
	return &streamList;
}

int PMTView::getProjectPid(ProjectInfo* proj) {
	map<unsigned short, ProjectInfo*>::iterator it;
	it = projectInfoList.begin();
	while(it != projectInfoList.end()) {
		if (it->second == proj) return it->first;
		++it;
	}
	return -1;
}

}
}
}
}


