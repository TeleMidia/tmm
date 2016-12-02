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
 * Dependents.h
 *
 *  Created on: 20/04/2013
 *      Author: Felippe Nagato
 */

#ifndef DEPENDENTS_H_
#define DEPENDENTS_H_

#include "Stream.h"
#include <iostream>
#include <set>
#include <vector>

using namespace std;
using namespace br::pucrio::telemidia::tool;

class Subscriber {

	private:

	protected:

	public:
		virtual ~Subscriber(){};

		virtual int encode(int64_t stc,
							vector<pair<char*,int>*>* list) = 0;
};

class Publisher {

	private:

	protected:
		set<Subscriber*> subscriberList;

		virtual void notify(int64_t stc, vector<pair<char*,int>*>* list) {
			set<Subscriber*>::iterator it;
			for (it = subscriberList.begin(); it != subscriberList.end(); it++) {
				(*it)->encode(stc, list);
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

		virtual void dettachAll() {
			subscriberList.clear();
		}

		virtual ~Publisher(){};
};


#endif /* DEPENDENTS_H_ */
