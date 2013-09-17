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
		unsigned short pcrFrequency;
		string serviceName;
		short int serviceType;
		ProjectInfo* eitProj;
		map<unsigned short, ProjectInfo*> projectInfoList; //(pid, ProjectInfo)
		map<unsigned short, Stream*> streamList; //(pid, Stream)
		map<unsigned short, unsigned char> componentTagList; //(pid, ctag)
		map<unsigned short, unsigned char> desiredComponentTagList; //(pid, ctag)
		map<unsigned short, unsigned char> layerList; //(pid, layer)

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
		void setPcrFrequency(unsigned short freq);
		unsigned short getPcrFrequency();
		void setServiceName(string name);
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
		void cleanLayerList();
		bool deleteAllStreams();
		map<unsigned short, ProjectInfo*>* getProjectInfoList();
		map<unsigned short, Stream*>* getStreamList();
		map<unsigned short, unsigned char>* getComponentTagList();
		map<unsigned short, unsigned char>* getLayerList();
		int getProjectPid(ProjectInfo* proj);
		bool fulfillComponentTagList(PMTView* previousPmtView);
		bool isDesiredComponentTagInUse(unsigned char ctag);
		bool isComponentTagInUse(unsigned char ctag);

		static bool compareComponentTagList(map<unsigned short, unsigned char>* oldList,
				map<unsigned short, unsigned char>* newList);

};

}
}
}
}


#endif /* PMTVIEW_H_ */
