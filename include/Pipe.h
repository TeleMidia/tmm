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
extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
}
#include <cstdio>
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
		Pipe(const string& pipeName);
		virtual ~Pipe();

		bool checkPipeDescriptor();
		void setPipeName(const string& name);
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
