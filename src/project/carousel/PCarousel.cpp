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
 * PCarousel.cpp
 *
 *  Created on: 01/04/2013
 *      Author: Felippe Nagato
 */

#include "project/carousel/PCarousel.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PCarousel::PCarousel() {
	projectType = PT_CAROUSEL;
	mounted = false;
}

PCarousel::~PCarousel() {

}

void PCarousel::setOutputFile(const string& filename) {
	outputFile = filename;
}

void PCarousel::setTempFolder(const string& folder) {
	tempFolder = folder;
}

string PCarousel::getOutputFile() {
	return outputFile;
}

string PCarousel::getTempFolder() {
	return tempFolder;
}

void PCarousel::setMounted(bool mounted) {
	this->mounted = mounted;
}

bool PCarousel::getMounted() {
	return mounted;
}

}
}
}
}



