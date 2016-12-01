/*
 * SharedMemory.h
 *
 *  Created on: 30/06/2014
 *      Author: Felippe Nagato
 */

#ifndef SHAREDMEMORY_H_
#define SHAREDMEMORY_H_

#include <stdio.h>
#include <iostream>
#include <string>

#ifdef _WIN32

#include <windows.h>
#include <conio.h>

#else

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#endif

#ifdef _WIN32
	typedef HANDLE MemoryDescriptor;
#else
	typedef int MemoryDescriptor;
#endif

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class SharedMemory {

	#define MEMORY_BUFFER_SIZE (128*1024)+5

	private:

	protected:
	string sharedMemoryName;
	MemoryDescriptor md;
        char *buffer;
	char control;
	char foreignControl;
	bool showErrorMsg;

	public:
		SharedMemory();
		SharedMemory(const string& sharedMemoryName);
		virtual ~SharedMemory();

		bool checkMemoryDescriptor();
		void setSharedMemoryName(const string& name);
		string getSharedMemoryName();
		bool createSharedMemory();
		bool openSharedMemory();
		void closeSharedMemory();
		int readSharedMemory(const char** data);
		int writeSharedMemory(char* data, int dataSize);
		int grantAccessToForeign();
		void showErrorMessages(bool enabled);
};

}
}
}
}


#endif /* SHAREDMEMORY_H_ */
