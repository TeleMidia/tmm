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

unsigned char ProjectInfo::versionTable[];

ProjectInfo::ProjectInfo() {
	projectType = PT_UNKNOWN;
	version = 0;
	transmissionDelay = 0.0;
	layer = 0x02; //HIERARCHY_B
	layerConfigured = false;
	reuse = false;
	currentPid = 0xFFFF;
	lastVersion = 0;
}

ProjectInfo::~ProjectInfo() {

}

bool ProjectInfo::processVersion() {
	if (ProjectInfo::versionTable[currentPid] != lastVersion) {
		incrementVersion();
		return true;
	}
	return false;
}

unsigned char ProjectInfo::incrementVersion() {
	ProjectInfo::versionTable[currentPid]++;
	lastVersion = ProjectInfo::versionTable[currentPid];
	return lastVersion;
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

unsigned char ProjectInfo::getProjectType() {
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

void ProjectInfo::setReuse(bool use) {
	reuse = use;
}

bool ProjectInfo::getReuse() {
	return reuse;
}

unsigned short ProjectInfo::getCurrentPid() {
	return currentPid;
}

void ProjectInfo::setCurrentPid(unsigned short pid) {
	currentPid = pid;
}

}
}
}
}


