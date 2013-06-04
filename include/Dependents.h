/*
 * Dependents.h
 *
 *  Created on: 20/04/2013
 *      Author: Felippe Nagato
 */

#ifndef DEPENDENTS_H_
#define DEPENDENTS_H_

#include "PrivateSection.h"
#include <iostream>
#include <set>
#include <vector>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

class Subscriber {

	private:

	protected:

	public:
		virtual ~Subscriber(){};

		virtual int encodeSections(int64_t stc,
								   vector<PrivateSection*>* list) = 0;
};

class Publisher {

	private:

	protected:
		set<Subscriber*> subscriberList;

		virtual void notify(int64_t stc, vector<PrivateSection*>* list) {
			set<Subscriber*>::iterator it;
			for (it = subscriberList.begin(); it != subscriberList.end(); it++) {
				(*it)->encodeSections(stc, list);
			}
		}

	public:
		virtual void attach(Subscriber *l) {
			subscriberList.insert(l);
		}

		virtual void dettach(Subscriber *l) {
			set<Subscriber *>::const_iterator iter = subscriberList.find(l);
			if (iter != subscriberList.end()) {
				subscriberList.erase(iter);
			}
		}

		virtual ~Publisher(){};
};


#endif /* DEPENDENTS_H_ */
