/*
 * InputData.cpp
 *
 *  Created on: 27/02/2013
 *      Author: Felippe Nagato
 */

#include "project/inputdata/InputData.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

InputData::InputData() {
	inputRangeList = new vector<InputRange*>;
	hasDts = false;
	offset = -1;
	firstPts = -1;
	projectType = PT_INPUTDATA;
}

InputData::InputData(int id) {
	this->id = id;
	inputRangeList = new vector<InputRange*>;
	hasDts = false;
	offset = -1;
	firstPts = -1;
	projectType = PT_INPUTDATA;
}

InputData::~InputData() {
	vector<InputRange*>::iterator it;
	it = inputRangeList->begin();
	while (it != inputRangeList->end()) {
		delete *it;
		++it;
	}
	delete inputRangeList;
}

void InputData::setFilename(string filename) {
	this->filename = filename;
}

string InputData::getFilename() {
	return filename;
}

void InputData::setPid(unsigned short pid) {
	this->pid = pid;
}

unsigned short InputData::getPid() {
	return pid;
}

void InputData::setNewPid(unsigned short pid) {
	newPid = pid;
}

unsigned short InputData::getNewPid() {
	return newPid;
}

void InputData::setStreamType(unsigned char st) {
	streamType = st;
}

unsigned short InputData::getStreamType() {
	return streamType;
}

void InputData::setDuration(double dur) {
	duration = dur;
}

double InputData::getDuration() {
	return duration;
}

void InputData::setHasDts(bool has) {
	hasDts = has;
}

bool InputData::getHasDts() {
	return hasDts;
}

void InputData::setOffset(int os) {
	offset = os;
}

int InputData::getOffset() {
	return offset;
}

void InputData::setFirstPts(int64_t pts) {
	firstPts = pts;
}

int64_t InputData::getFirstPts() {
	return firstPts;
}

bool InputData::addRange(InputRange* ir) {
	inputRangeList->push_back(ir);
	return true;
}

bool InputData::addRange(int id, int64_t begin, int64_t end) {
	InputRange* ir;
	vector<InputRange*>::iterator it;

	if (begin > end) return false;

	it = inputRangeList->begin();
	while (it != inputRangeList->end()) {
		if ((*it)->getId() == id) {
			return false;
		}
		++it;
	}

	ir = new InputRange();
	ir->setId(id);
	ir->setBegin(begin);
	ir->setEnd(end);
	inputRangeList->push_back(ir);

	return true;
}

bool InputData::removeRange(int id) {
	vector<InputRange*>::iterator it;

	it = inputRangeList->begin();
	while (it != inputRangeList->end()) {
		if ((*it)->getId() == id) {
			delete (*it);
			return true;
		}
		++it;
	}

	return false;
}

bool InputData::getRange(int id, int64_t* begin, int64_t *end) {
	vector<InputRange*>::iterator it;

	it = inputRangeList->begin();
	while (it != inputRangeList->end()) {
		if ((*it)->getId() == id) {
			*begin = (*it)->getBegin();
			*end = (*it)->getEnd();
			return true;
		}
		++it;
	}

	return false;
}

vector<InputRange*>* InputData::getInputRangeList() {
	return inputRangeList;
}


}
}
}
}
