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

}
}
}
}



