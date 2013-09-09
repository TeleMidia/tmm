/*
 * XMLProject.h
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#ifndef XMLPROJECT_H_
#define XMLPROJECT_H_

#include "project/Project.h"
#include "project/tinyxml2.h"

#include <cstdlib>
#include <vector>

using namespace tinyxml2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

struct MetaData {
	 string name;
	 string content;
};

class XMLProject : public Project {
	#define TOT_NAME "_tot_project_"
	#define SDT_NAME "_sdt_project_"
	#define NIT_NAME "_nit_project_"
	#define TIMELINE_NAME "_timeline_project_"

	private:
		XMLDocument* xmldoc;

		void init();
		string getAttribute(XMLElement* e, string name);
		string getElementText(XMLElement* e);

	protected:
		vector<MetaData*> metaDataList;
		map<string, int> idList;
		int idIndex;

		bool createNewId(string id);
		int getId(string id);

	public:
		XMLProject();
		XMLProject(string filename);
		XMLProject(const char *filename);
		virtual ~XMLProject();

		vector<MetaData*>* getMetaDataList();

		int readFile();

};

}
}
}
}


#endif /* XMLPROJECT_H_ */
