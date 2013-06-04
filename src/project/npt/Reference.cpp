/*
 * Reference.cpp
 *
 *  Created on: 28/02/2013
 *      Author: Felippe Nagato
 */

#include "project/npt/Reference.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

Reference::Reference() {
	numerator = 1;
	denominator = 1;
	hasNext = false;
	inTransition = false;
	inInterval = false;
}

Reference::~Reference() {

}

void Reference::setId(int id) {
	this->id = id;
}

int Reference::getId() {
	return id;
}

void Reference::setContentId(unsigned char id) {
	contentId = id;
}

unsigned char Reference::getContentId() {
	return contentId;
}

void Reference::setAbsStart(int64_t start) {
	absStart = start;
}

int64_t Reference::getAbsStart() {
	return absStart;
}

void Reference::setAbsEnd(int64_t end) {
	absEnd = end;
}

int64_t Reference::getAbsEnd() {
	return absEnd;
}

void Reference::setNptStart(int64_t start) {
	nptStart = start;
}

int64_t Reference::getNptStart() {
	return nptStart;
}

void Reference::setNptEnd(int64_t end) {
	nptEnd = end;
}

int64_t Reference::getNptEnd() {
	return nptEnd;
}

void Reference::setNumerator(short num) {
	numerator = num;
}

short Reference::getNumerator() {
	return numerator;
}

void Reference::setDenominator(unsigned short den) {
	denominator = den;
}

unsigned short Reference::getDenominator() {
	return denominator;
}

void Reference::setHasNext(bool hasNext) {
	this->hasNext = hasNext;
}

bool Reference::getHasNext() {
	return hasNext;
}

void Reference::setInTransition(bool transition) {
	inTransition = transition;
}

bool Reference::getInTransition() {
	return inTransition;
}

void Reference::setInInterval(bool interval) {
	inInterval = interval;
}

bool Reference::getInInterval() {
	return inInterval;
}


}
}
}
}


