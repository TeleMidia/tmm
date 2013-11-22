/*
 * ExecApp.cpp
 *
 *  Created on: 22/11/2013
 *      Author: Felippe Nagato
 */

#include "ExecApp.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

ExecApp::ExecApp() {

}

ExecApp::~ExecApp() {

}

bool ExecApp::execute(string filename, string parameters) {
#ifdef _WIN32
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
	STARTUPINFO StartupInfo; //This is an [in] parameter
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo ; //Only compulsory field
	parameters = filename + parameters;
	if(CreateProcess(filename.c_str(), (char*) parameters.c_str(),
	    NULL,NULL,FALSE,0,NULL,
	    NULL,&StartupInfo,&ProcessInfo)) {
	    return true;
	}
	else {
	    return false;
	}
#else
	pid_t pid;
	pid = fork();
	if(fork == 0){
	    execvp(filename.c_str());
	    return true;
	}
	else {
		return false;
	}
#endif
}

}
}
}
}


