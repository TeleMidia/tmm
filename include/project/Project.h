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
 * Project.h
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#ifndef PROJECT_H_
#define PROJECT_H_

#include "inputdata/InputData.h"
#include "npt/NPTProject.h"
#include "streamevent/PStreamEvent.h"
#include "carousel/PCarousel.h"
#include "si/PPat.h"
#include "si/PAit.h"
#include "si/PTot.h"
#include "si/PSdt.h"
#include "si/PNit.h"
#include "si/PEit.h"
#include "si/PMTView.h"
#include "dsmcc/carousel/Biop/StreamEventMessage.h"
#include "timeline/Timeline.h"
#include "Stc.h"
#include "../tsparser/ISDBTInformation.h"
#include "../tsparser/IIP.h"
#include "../LocalLibrary.h"
#include "../SharedMemory.h"

#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <map>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2::dsmcc;
using namespace br::pucrio::telemidia::tool::isdbt;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class Project {

	private:

	protected:
		map<string, unsigned char> commandTagList;

		string filename;
		string projectName;
		string projectDescription;
		SharedMemory liveServer;

		bool isLoop;
		string destination;
		string providerName;
		int tsid;
		int tsBitrate;
		int64_t stcBegin; //27 MHz - Minimum value = 270000000 (10s)
		double vbvBuffer;
		unsigned char ttl;

		IIP* iip;

		string generatingStation;
		unsigned short originalNetworkId;
		string tsName;
		unsigned short broadcastFrequency;
		unsigned char virtualChannel;
		unsigned char guardInterval;
		unsigned char transmissionMode;
		unsigned short packetsInBuffer;

		bool partialReception;

		map<string, unsigned char> regionList;
		unsigned char areaCode1;
		unsigned char areaCode2;

		bool useTot;
		bool useSdt;
		bool useNit;

		unsigned char packetSize;

		map<int, ProjectInfo*>* projectList;
		//TODO: EIT (schedule)

		bool isPipe;
		string externalApp;
		string appParams;
		string tmmPath;

		uint64_t relStc;
		bool isLive;

		bool changeToProjectDir();
		bool mountCarousel(PCarousel* pcar);
		bool createStreamEvent(PStreamEvent* pse);
		void fillCommandTagList();
		void fillRegionList();

	public:
		Project();
		virtual ~Project();

		void setFilename(const string& filename);
		string getFilename();
		string getProjectName();
		string getProjectDescription();
		bool mountCarousels();
		bool createStreamEvents();
		int configAit(PAit* ait, unsigned int ctag, const string& aName,
				const string& lang, const string& baseDir,
				const string& initClass, unsigned int orgId,
				unsigned short appId, unsigned int appcode,
				unsigned char recommendedResolution,
				int application_profile
			);
		int configAitService(ProjectInfo* ait, unsigned short serviceId,
				unsigned char ctag);
		int configSdt(vector<pmtViewInfo*>* newTimeline, ProjectInfo* sdt);
		int configNit(vector<pmtViewInfo*>* newTimeline, ProjectInfo* nit);
		void setDestination(const string& dest);
		string getDestination();
		bool getIsPipe();
		string getExternalApp();
		string getAppParams();
		void setProviderName(const string& name);
		string getProviderName();
		void setTsid(int id);
		int getTsid();
		IIP* getIip();
		void setTsBitrate(int rate);
		int getTsBitrate();
		void setStcBegin(int64_t stc);
		int64_t getStcBegin();
		map<int, ProjectInfo*>* getProjectList();
		ProjectInfo* findProject(int id);
		double getVbvBuffer();
		bool getIsLoop();
		unsigned char getTTL();
		unsigned short getOriginalNetworkId();
		string getTsName();
		unsigned short getBroadcastFrequency();
		unsigned char getVirtualChannel();
		unsigned char getGuardInterval();
		unsigned char getTransmissionMode();
		unsigned short getPacketsInBuffer();
		void setUseTot(bool use);
		bool getUseTot();
		void setUseSdt(bool use);
		bool getUseSdt();
		void setUseNit(bool use);
		bool getUseNit();
		void setPacketSize(unsigned char size);
		unsigned char getPacketSize();
		bool getIsLive();

		static unsigned char toLayer(const string& layer);

		void updateRelativeStc(int64_t relStc);

		virtual int readFile()=0;
		virtual int readLiveStream()=0;

};

}
}
}
}


#endif /* PROJECT_H_ */
