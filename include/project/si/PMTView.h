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
#include "project/ProjectInfo.h"
#include <iostream>
#include <map>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PMTView : public ProjectInfo {

	private:

	protected:
		unsigned short pid;
		Stream* pmtStream;
		unsigned short programNumber;
		int tsinput;
		unsigned short pcrPid;
		unsigned short pcrFrequency;
		string serviceName;
		map<unsigned short, ProjectInfo*> projectInfoList; //(pid, ProjectInfo)
		map<unsigned short, Stream*> streamList; //(pid, Stream)

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
		bool addProjectInfo(unsigned short pid, ProjectInfo* projInfo);
		bool addStream(unsigned short pid, Stream* stream);
		bool deleteAllStreams();
		map<unsigned short, ProjectInfo*>* getProjectInfoList();
		map<unsigned short, Stream*>* getStreamList();

};

}
}
}
}


#endif /* PMTVIEW_H_ */
