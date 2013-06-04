/*
 * InputRange.cpp
 *
 *  Created on: 27/02/2013
 *      Author: Felippe Nagato
 */

#include "project/inputdata/InputRange.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

InputRange::InputRange() {

}

InputRange::InputRange(int id) {
	this->id = id;
}

InputRange::~InputRange() {

}

void InputRange::setId(int id) {
	this->id = id;
}

void InputRange::setBegin(int begin) {
	this->begin = begin;
}

void InputRange::setEnd(int end) {
	this->end = end;
}

int InputRange::getId() {
	return id;
}

int InputRange::getBegin() {
	return begin;
}

int InputRange::getEnd()  {
	return end;
}

}
}
}
}


