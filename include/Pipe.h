/*
 * Pipe.h
 *
 *  Created on: 21/11/2013
 *      Author: Felippe Nagato
 */

#ifndef PIPE_H_
#define PIPE_H_

#ifdef _WIN32
#include <windows.h>
#else

#endif

#include <iostream>
#include <string>

#ifdef _WIN32
	typedef HANDLE PipeDescriptor;
#else
	typedef int PipeDescriptor;
#endif

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class Pipe {

	private:

	protected:
	string pipeName;
	PipeDescriptor pd;

	public:
		Pipe();
		Pipe(string pipeName);
		virtual ~Pipe();

		bool checkPipeDescriptor();
		void setPipeName(string name);
		string getPipeName();
		bool createPipe();
		bool openPipe();
		void closePipe();
		int readPipe(char* buffer, int buffSize);
		int writePipe(char* data, int dataSize);
};

}
}
}
}

#endif /* PIPE_H_ */
