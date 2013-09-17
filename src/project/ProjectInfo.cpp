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
	transmissionDelay = 0.0;
	layer = 0x02;
	layerConfigured = false;
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

double ProjectInfo::getTransmissionDelay() {
	return transmissionDelay;
}

void ProjectInfo::setTransmissionDelay(double delay) {
	transmissionDelay = delay;
}

void ProjectInfo::setLayer(unsigned char layer) {
	this->layer = layer;
}

unsigned char ProjectInfo::getLayer() {
	return layer;
}

void ProjectInfo::setLayerConfigured(bool set) {
	layerConfigured = set;
}

bool ProjectInfo::getLayerConfigured() {
	return layerConfigured;
}

}
}
}
}


