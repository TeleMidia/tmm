/*
 * ProjectInfo.cpp
 *
 *  Created on: 07/03/2013
 *      Author: Felippe Nagato
 */

#include "project/ProjectInfo.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

ProjectInfo::ProjectInfo() {
	projectType = PT_UNKNOWN;
	version = 0;
}

ProjectInfo::~ProjectInfo() {

}

void ProjectInfo::setId(int id) {
	this->id = id;
}

int ProjectInfo::getId() {
	return id;
}

void ProjectInfo::setProjectType(char type) {
	this->projectType = type;
}

char ProjectInfo::getProjectType() {
	return projectType;
}

void ProjectInfo::setVersion(unsigned char ver) {
	version = ver;
}

unsigned char ProjectInfo::getVersion() {
	return version;
}

}
}
}
}


