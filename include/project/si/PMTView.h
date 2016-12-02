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
 * PMTView.h
 *
 *  Created on: 07/03/2013
 *      Author: Felippe Nagato
 */

#ifndef PMTVIEW_H_
#define PMTVIEW_H_

#include "Stream.h"
#include "Stc.h"
#include "si/descriptor/Aac.h"
#include "si/descriptor/Iso639Language.h"
#include "tsparser/TSInfo.h"
#include "project/ProjectInfo.h"
#include "project/inputdata/InputData.h"
#include <iostream>
#include <map>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PMTView : public ProjectInfo {

	#define SRV_TYPE_TV 	0x00
	#define SRV_TYPE_DATA1  0x01
	#define SRV_TYPE_DATA2  0x02
	#define SRV_TYPE_ONESEG 0x03

	private:

	protected:
		unsigned short pid;
		Stream* pmtStream;
		unsigned short programNumber;
		int tsinput;
		unsigned short pcrPid;
		unsigned int pcrPeriod;
		string serviceName;
		short int serviceType;
		ProjectInfo* eitProj;
		map<unsigned short, vector<ProjectInfo*>*> projectInfoList; //(pid, ProjectInfo)
		map<unsigned short, vector<Stream*>*> streamList; //(pid, Stream)
		map<unsigned short, unsigned char> componentTagList; //(pid, ctag)
		map<unsigned short, unsigned char> desiredComponentTagList; //(pid, ctag)
		map<unsigned short, unsigned char> layerList; //(pid, layer)
		map<unsigned short, vector<MpegDescriptor*>* > esDescriptorList; //(pid, descriptor list)

	public:
		PMTView();
		virtual ~PMTView();

		void setPid(unsigned short pid);
		unsigned short getPid();
		void setPmtStream(Stream* stream);
		Stream* getPmtStream();
		void setProgramNumber(unsigned short pn);
		unsigned short getProgramNumber();
		void setTsinput(int id);
		int getTsinput();
		void setPcrPid(unsigned short pid);
		unsigned short getPcrPid();
		void setPcrPeriod(unsigned int freq);
		unsigned int getPcrPeriod();
		void setServiceName(const string& name);
		string getServiceName();
		void setServiceType(short int st);
		short int getServiceType();
		ProjectInfo* getEitProj();
		void setEitProj(ProjectInfo* proj);
		bool addProjectInfo(unsigned short pid, ProjectInfo* projInfo);
		bool addStream(unsigned short pid, Stream* stream);
		bool addDesiredComponentTag(unsigned short pid, unsigned char tag);
		bool addComponentTag(unsigned short pid, unsigned char tag);
		bool getComponentTag(unsigned short pid, unsigned char* tag);
		bool addPidToLayer(unsigned short pid, unsigned char layer);
		unsigned char getLayerPid(unsigned short pid);
		bool addEsDescriptor(unsigned short pid, MpegDescriptor* md);
		void cleanLayerList();
		bool deleteAllStreams();
		bool releaseEsDescriptorList();
		map<unsigned short, vector<ProjectInfo*>*>* getProjectInfoList();
		map<unsigned short, vector<Stream*>*>* getStreamList();
		map<unsigned short, unsigned char>* getComponentTagList();
		map<unsigned short, unsigned char>* getLayerList();
		map<unsigned short, vector<MpegDescriptor*>* >* getEsDescriptorList();
		int getProjectPid(ProjectInfo* proj);
		bool fulfillComponentTagList(PMTView* previousPmtView);
		bool isDesiredComponentTagInUse(unsigned char ctag);
		bool isComponentTagInUse(unsigned char ctag);
		void markAllProjectsReuse(bool use);

		static bool compareComponentTagList(map<unsigned short, unsigned char>* oldList,
				map<unsigned short, unsigned char>* newList);

};

}
}
}
}


#endif /* PMTVIEW_H_ */
