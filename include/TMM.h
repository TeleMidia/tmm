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
#include "RawStream.h"
#include "si/descriptor/CarouselIdentifier.h"
#include "si/descriptor/StreamIdentifier.h"
#include "si/descriptor/DataStreamAlignment.h"
#include "si/descriptor/HierarchicalTransmission.h"
#include "si/descriptor/DeferredAssociationTags.h"
#include "si/descriptor/AssociationTag.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class TMM {
	#define PREPONETICKS_AUDIO 0.2
	#define PREPONETICKS_VIDEO 1.0

	#define DEFAULT_PAT_PID 0x0000
	#define DEFAULT_NIT_PID 0x0010
	#define DEFAULT_SDT_PID 0x0011
	#define DEFAULT_EIT_PID 0x0012
	#define DEFAULT_TOT_PID 0x0014

	private:
		void init();
		void releaseSiStreamList();

	protected:
		Project *project;
		Muxer* muxer;
		string destination;
		map<ProjectInfo*, Stream*> siAndIsdbtStreamList;
		unsigned int lastStcPrinter;

		ProjectInfo* getFirstProject(unsigned char projectType);
		ProjectInfo* getFirstProjectReversed(unsigned char projectType);
		bool loadProject();

		RawStream* prepareNewRawStream(ProjectInfo* proj, int64_t freq,
										int64_t nextSend, bool destroyBlocks);
		Stream *createStream(ProjectInfo* proj);
		bool createStreamList(vector<pmtViewInfo*>* currentTimeline,
							  vector<pmtViewInfo*>* newTimeline);
		bool releaseStreamFromList(ProjectInfo* proj);

		void processPcrsInUse(vector<pmtViewInfo*>* newTimeline);
		int createPmt(PMTView* currentPmtView, PMTView* newPmtView, Pmt** pmt);
		int createSiTables(vector<pmtViewInfo*>* newTimeline);
		int restoreSiTables(vector<pmtViewInfo*>* currentTimeline,
							vector<pmtViewInfo*>* newTimeline);
		void addStreamToMuxer(Stream* stream, unsigned short pid, unsigned char layer);

		int multiplex();
		int getTSInfo(InputData *inputList);
		unsigned short calculateNumberOfTsps(TransmissionParameters* tp,
												unsigned char mode);
		int multiplexSetup();

		bool getCarouselComponentTagFromService(PMTView* pv, ProjectInfo* carouselProj,
				unsigned char* ctag);

		bool updateComponentTagList(vector<pmtViewInfo*>* currentTimeline,
				vector<pmtViewInfo*>* newTimeline);

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
