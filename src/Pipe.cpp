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
 * Pipe.cpp
 *
 *  Created on: 21/11/2013
 *      Author: Felippe Nagato
 */

#include "Pipe.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

Pipe::Pipe() {
	setPipeName("tmm.unnamedpipe.ts");
}

Pipe::Pipe(const string& pipeName) {
	setPipeName(pipeName);
}

Pipe::~Pipe() {
	if (checkPipeDescriptor()) {
		closePipe();
	}
}

bool Pipe::checkPipeDescriptor() {
#if defined(_WIN32)
		if (pd > 0) return true; else return false;
#else
		if (pd >= 0) return true; else return false;
#endif
}

void Pipe::setPipeName(const string& name) {
	pipeName = name;
#if defined(_WIN32)
		if (pipeName.find("\\\\.\\pipe\\") == std::string::npos) {
			pipeName = "\\\\.\\pipe\\" + name;
		}
#endif
}

string Pipe::getPipeName() {
	return pipeName;
}

bool Pipe::createPipe() {
#if defined(_WIN32)
	pd = CreateNamedPipe(
			pipeName.c_str(),
			PIPE_ACCESS_OUTBOUND, // 1-way pipe
			PIPE_TYPE_BYTE, // send data as a byte stream
			1, // only allow 1 instance of this pipe
			0, // no outbound buffer
			0, // no inbound buffer
			0, // use default wait time
			NULL); // use default security attributes);

	if (pd == NULL || pd == INVALID_HANDLE_VALUE) {
		clog << "Pipe::createPipe - Failed to create '";
		clog << pipeName << "' pipe instance.";
		clog << endl;
		// TODO: look up error code: GetLastError()
		return false;
	}

	cout << "Waiting for client to connect to '" << pipeName <<
			"'..." << endl;
	// This call blocks until a client process connects to the pipe
	BOOL result = ConnectNamedPipe(pd, NULL);
	if (!result) {
		clog << "Pipe::createPipe - Failed to make ";
		clog << "connection on " << pipeName << endl;
		// TODO: look up error code: GetLastError()
		CloseHandle(pd); // close the pipe
		return false;
	}
	cout << "Client is connected." << endl;
#else
	mkfifo(pipeName.c_str(), S_IFIFO);

	pd = open(pipeName.c_str(), O_WRONLY);
	if (pd == -1) {
		clog << "Pipe::createPipe - Failed to make ";
		clog << "connection on " << pipeName << endl;

		return false;
	}
#endif
	return true;
}

bool Pipe::openPipe() {
#if defined(_WIN32)
	pd = CreateFile(
			pipeName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	if (pd == INVALID_HANDLE_VALUE) {
		clog << "Pipe::openPipe - Failed to open '";
		clog << pipeName << "'" << endl;
		// TODO: look up error code: GetLastError()
		return false;
	}
#else
	pd = open(pipeName.c_str(), O_RDONLY);
	if (pd < 0) {
		clog << "Pipe::openPipe - Can't open '" << pipeName;
		clog << "'" << endl;
		perror("Pipe::openPipe - Can't open pipe");
		return false;
	}
#endif
	return true;
}

void Pipe::closePipe() {
	if (!checkPipeDescriptor()) return;
#if defined(_WIN32)
	CloseHandle(pd);
#else
	close(pd);
#endif
}

int Pipe::readPipe(char* buffer, int buffSize) {
	int bytesRead = 0;

	if (!checkPipeDescriptor()) return -1;

#if defined(_WIN32)
	DWORD bRead = 0;
	BOOL result = ReadFile(
			pd,
			buffer,
			buffSize,
			&bRead,
			NULL);
	if (result) {
		bytesRead = (int)bRead;
	} else {
		return -1;
	}
#else
		bytesRead = read(pd, buffer, buffSize);
#endif

	return bytesRead;
}

int Pipe::writePipe(char* data, int dataSize) {
	int bytesWritten = 0;

	if (!checkPipeDescriptor()) return -1;

#if defined(_WIN32)
	// This call blocks until a client process reads all the data
	DWORD bWritten = 0;
	BOOL result = WriteFile(
			pd,
			data,
			dataSize,
			&bWritten,
			NULL); // not using overlapped IO
	if (result) {
		bytesWritten = (int)bWritten;
	} else {
		return -1;
	}
#else
	bytesWritten = write(pd, (void*)data, dataSize);
#endif

	return bytesWritten;
}

}
}
}
}
