/*
 * ExecApp.h
 *
 *  Created on: 22/11/2013
 *      Author: Felippe Nagato
 */

#ifndef EXECAPP_H_
#define EXECAPP_H_

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include <string>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class ExecApp {

	private:

	protected:

	public:
		ExecApp();
		virtual ~ExecApp();

		static bool execute(string filename, string parameters);

};

}
}
}
}

#endif /* EXECAPP_H_ */
