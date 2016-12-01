/*
 * SharedMemory.cpp
 *
 *  Created on: 30/06/2014
 *      Author: Felippe Nagato
 */

#include "SharedMemory.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

SharedMemory::SharedMemory() {
	md = NULL;
	buffer = NULL;
	control = -1;
	foreignControl = -1;
	showErrorMsg = true;
	setSharedMemoryName("tmm.unnamedmemory.ts");
}

SharedMemory::SharedMemory(const string& sharedMemoryName) {
	setSharedMemoryName(sharedMemoryName);
}

SharedMemory::~SharedMemory() {
	closeSharedMemory();
}

void SharedMemory::showErrorMessages(bool enabled) {
	showErrorMsg = enabled;
}

bool SharedMemory::checkMemoryDescriptor() {
#if defined(_WIN32)
		if (md > 0) return true; else return false;
#else
		if (md >= 0) return true; else return false;
#endif
}

void SharedMemory::setSharedMemoryName(const string& name) {
	sharedMemoryName = name;
#if defined(_WIN32)
		if (sharedMemoryName.find("Local\\") == std::string::npos) {
			sharedMemoryName = "Local\\" + name;
		}
#endif
}

string SharedMemory::getSharedMemoryName() {
	return sharedMemoryName;
}

bool SharedMemory::createSharedMemory() {
	unsigned int dataSize = 0;
	char* size = (char*)&dataSize;

#if defined(_WIN32)
	md = CreateFileMapping(
				 INVALID_HANDLE_VALUE,    // use paging file
				 NULL,                    // default security
				 PAGE_READWRITE,          // read/write access
				 0,                       // maximum object size (high-order DWORD)
				 MEMORY_BUFFER_SIZE,      // maximum object size (low-order DWORD)
				 sharedMemoryName.c_str());// name of mapping object

	if (md == NULL) {
		if (showErrorMsg) {
			clog << "SharedMemory::createSharedMemory - Failed to create '";
			clog << sharedMemoryName << "' shared memory instance.";
			clog << endl;
		}
	    return false;
	}

	buffer = (const char*) MapViewOfFile(md,   // handle to map object
								FILE_MAP_ALL_ACCESS, // read/write permission
								0, 0, MEMORY_BUFFER_SIZE);

	if (buffer == NULL) {
		if (showErrorMsg) {
			clog << "SharedMemory::createSharedMemory - Failed to get ";
			clog << " the pointer to '" << sharedMemoryName << "'.";
			clog << endl;
		}
		closeSharedMemory();
		return false;
	}

	control = 1;
	foreignControl = 0;

	CopyMemory((PVOID)buffer, &control, 1);
	CopyMemory((PVOID)(buffer + 1), size, 4);

#else
	/* Create a new memory object */
	md = shm_open(sharedMemoryName.c_str(), O_RDWR | O_CREAT, 0666 );
	if( md == -1 ) {
		fprintf( stderr, "Open failed:%s\n",
			strerror( errno ) );
		return EXIT_FAILURE;
	}

	/* Set the memory object's size */
	if( ftruncate( md, MEMORY_BUFFER_SIZE ) == -1 ) {
		fprintf( stderr, "ftruncate: %s\n",
			strerror( errno ) );
		return EXIT_FAILURE;
	}

	/* Map the memory object */
	buffer = (char *) mmap( 0, MEMORY_BUFFER_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED, md, 0 );
	if( buffer == MAP_FAILED ) {
		fprintf( stderr, "mmap failed: %s\n",
			strerror( errno ) );
		return EXIT_FAILURE;
	}
#endif

	return true;
}

bool SharedMemory::openSharedMemory() {
#if defined(_WIN32)
	md = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			sharedMemoryName.c_str());// name of mapping object

	if (md == NULL) {
		if (showErrorMsg) {
			clog << "SharedMemory::openSharedMemory - Failed to open '";
			clog << sharedMemoryName << "'";
			clog << endl;
		}
		return false;
	}

	buffer = (const char*) MapViewOfFile(md,// handle to map object
			   FILE_MAP_ALL_ACCESS,  		// read/write permission
			   0, 0, MEMORY_BUFFER_SIZE);

	if (buffer == NULL) {
		if (showErrorMsg) {
			clog << "SharedMemory::openSharedMemory - Failed to get ";
			clog << " the pointer to '" << sharedMemoryName << "'.";
			clog << endl;
		}
		closeSharedMemory();
		return false;
	}

	control = 0;
	foreignControl = 1;

#else
	/* Create a new memory object */
	md = shm_open(sharedMemoryName.c_str(), O_RDWR, 0666 );
	if( md == -1 ) {
		fprintf( stderr, "Open failed:%s\n",
			strerror( errno ) );
		return EXIT_FAILURE;
	}

	/* Map the memory object */
	buffer = (char *) mmap( 0, MEMORY_BUFFER_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED, md, 0 );
	if( buffer == MAP_FAILED ) {
		fprintf( stderr, "mmap failed: %s\n",
			strerror( errno ) );
		return EXIT_FAILURE;
	}
#endif
	return true;
}

void SharedMemory::closeSharedMemory() {
	if (!checkMemoryDescriptor()) return;
#if defined(_WIN32)
	if (buffer) UnmapViewOfFile(buffer);
	CloseHandle(md);
#else
	close(md);
	shm_unlink( sharedMemoryName.c_str() );
#endif
	md = NULL;
	buffer = NULL;
	control = -1;
	foreignControl = -1;
}

int SharedMemory::readSharedMemory(const char** data) {
	unsigned int *size;
	if (!checkMemoryDescriptor()) return -1;

	if (buffer[0] == control) {
		size = (unsigned int*)(buffer+1);
		if (size) {
			*data = (buffer + 5);
		}
	} else return -3;

	return *size;
}

int SharedMemory::writeSharedMemory(char* data, int dataSize) {
	char* size = (char*)&dataSize;

	if (!checkMemoryDescriptor()) return -1;

	if (dataSize > MEMORY_BUFFER_SIZE - 5) {
		cout << "SharedMemory::writeSharedMemory - Message too long." << endl;
		return -2;
	}

#if defined(_WIN32)
	if (buffer[0] == control) {
		CopyMemory((PVOID)(buffer + 1), size, 4);
		CopyMemory((PVOID)(buffer + 5), data, dataSize);
	} else return -3;
#else
	if (buffer[0] == control) {
		memcpy(buffer + 1, size, 4);
		memcpy(buffer + 5, data, dataSize);
	} else return -3;
#endif

	return dataSize;
}

int SharedMemory::grantAccessToForeign() {
	if (!checkMemoryDescriptor()) return -1;

#if defined(_WIN32)
	CopyMemory((PVOID)buffer, &foreignControl, 1);
#else
	memcpy(buffer, &foreignControl, 1);
#endif

	return 0;
}

}
}
}
}


