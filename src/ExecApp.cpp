/*
 * ExecApp.cpp
 *
 *  Created on: 22/11/2013
 *      Author: Felippe Nagato
 */

#include "ExecApp.h"
#include <iostream>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

ExecApp::ExecApp() {

}

ExecApp::~ExecApp() {

}

bool ExecApp::execute(string filename, string parameters, unsigned int* pid) {
	*pid = 0;
#ifdef _WIN32
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
	STARTUPINFO StartupInfo; //This is an [in] parameter
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo ; //Only compulsory field
	parameters = filename + ' ' + parameters;
	if(CreateProcess(filename.c_str(), (char*) parameters.c_str(),
	    NULL,NULL,FALSE,0,NULL,
	    NULL,&StartupInfo,&ProcessInfo)) {
		*pid = (unsigned int) ProcessInfo.dwProcessId;
	    return true;
	}
	else {
		DWORD dw = GetLastError();
		cout << "Error " << dw << endl;
	    return false;
	}
#else
	pid_t mpid;
	mpid = fork();
	if(fork == 0){
	    execvp(filename.c_str());
	    *pid = (unsigned int) mpid;
	    return true;
	}
	else {
		return false;
	}
#endif
}

bool ExecApp::killApp(unsigned int pid) {
#ifdef _WIN32
	DWORD dwDesiredAccess = PROCESS_TERMINATE;
	BOOL bInheritHandle  = FALSE;
	unsigned int uExitCode = 0;
	HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, (DWORD)pid);
	if (hProcess == NULL) return FALSE;
	BOOL result = TerminateProcess(hProcess, uExitCode);
	CloseHandle(hProcess);
	return result;
#else
	kill(pid, SIGTERM);
#endif
}

}
}
}
}


