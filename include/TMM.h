/*
 * TMM.h
 *
 *  Created on: 27/02/2013
 *      Author: Felippe Nagato
 */

#ifndef TMM_H_
#define TMM_H_

#include "util/functions.h"
#include "project/Project.h"
#include "tsparser/TSFileReader.h"
#include "tsparser/TSInfo.h"
#include "tsparser/Muxer.h"
#include "PESStream.h"
#include "SectionStream.h"
#include "si/descriptor/CarouselIdentifier.h"
#include "si/descriptor/StreamIdentifier.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class TMM {
	#define PREPONETICKS_AUDIO 0.25
	#define PREPONETICKS_VIDEO 1.00

	private:
		void init();

	protected:
		Project *project;
		Muxer* muxer;
		string destination;
		SectionStream* patStream;
		SectionStream* totStream;
		SectionStream* sdtStream;
		SectionStream* nitStream;
		bool useSdt;
		bool useNit;
		unsigned char patVersion;
		unsigned char sdtVersion;
		unsigned char nitVersion;
		unsigned int lastStcPrinter;

		ProjectInfo* getFirstProject(char projectType);
		ProjectInfo* getFirstProjectReversed(char projectType);
		bool loadProject();
		Stream *createStream(ProjectInfo* proj, unsigned char version);
		bool createStreamList(vector<pmtViewInfo*>* currentTimeline,
							  vector<pmtViewInfo*>* newTimeline);

		void processPcrsInUse(vector<pmtViewInfo*>* newTimeline);
		int updateSdt(vector<pmtViewInfo*>* newTimeline, Sdt** sdt);
		int updateNit(vector<pmtViewInfo*>* newTimeline, Nit** nit);
		int createPmt(PMTView* pmtView, Pmt** pmt);
		int createSiTables(vector<pmtViewInfo*>* newTimeline);
		int restoreSiTables(vector<pmtViewInfo*>* currentTimeline,
							vector<pmtViewInfo*>* newTimeline);

		int multiplex();
		int getTSInfo(InputData *inputList);
		int multiplexSetup();

	public:
		TMM();
		TMM(Project* project);
		virtual ~TMM();

		void setProject(Project* project);

		int sendTo(const char* destination);

};

}
}
}
}


#endif /* TMM_H_ */
