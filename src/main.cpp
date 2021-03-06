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
 * main.cpp
 *
 *  Created on: 30/01/2013
 *      Author: Felippe Nagato
 */

#include "util/functions.h"
#include "project/XMLProject.h"
#include "TMM.h"
#include "tsparser/TSInfo.h"
#include <string>

using namespace br::pucrio::telemidia::tool;

const char *program_name;

int readTSInfo(const string& file, unsigned char mode) {
	TSFileReader tsFile;
	TSInfo tsInfo;

	tsFile.setFilename(file);
	tsInfo.setTSFileReader(&tsFile);
	if (tsInfo.readInfo(mode)) {
		switch (mode) {
		case 0:
			tsInfo.printESTable();
			break;
		case 1:
			tsInfo.printTables();
			break;
		}
		return 0;
	} else {
		return -1;
	}
}

void printHelp() {
	cout << endl;
	cout << "Telemidia Multiplexer. Version 0.5 (Build 1)." << endl;
	cout << "Copyright(c) 1989-2014, PUC-RIO/LABORATORIO TELEMIDIA." << endl;
	cout << "All Rights Reserved." << endl;
	cout << "http://www.telemidia.puc-rio.br" << endl;
	cout << endl;
	cout << "Usage: " << program_name << " <project_file> <optional_ts_output>" << endl;
	cout << "Example 1: " << program_name << " project.tmm" << endl;
	cout << "Example 2: " << program_name << " project.tmm output.ts" << endl;
	cout << "Example 3: " << program_name << " project.tmm udp://224.1.1.2:1234"
		 << endl << endl;
}

int main(int argc, char *argv[]) {
	const char *project_file = NULL;
	const char *ts_output = NULL;
	TMM* tmm;

	program_name = argv[0];
	if (argc == 2 || argc == 3) {
		project_file = argv[1];
		if (argc == 3) ts_output = argv[2];
	} else {
		printHelp();
		return 0;
	}

	if (((strcmp(project_file, "--info") == 0) ||
		 (strcmp(project_file, "-i") == 0)) && ts_output) {
		readTSInfo(ts_output, 0);
		return 0;
	}

	if (((strcmp(project_file, "--fullinfo") == 0) ||
		 (strcmp(project_file, "-fi") == 0)) && ts_output) {
		readTSInfo(ts_output, 1);
		return 0;
	}

	if ((strcmp(project_file, "-h") == 0) ||
		(strcmp(project_file, "--help") == 0)) {
		printHelp();
		return 0;
	}

	XMLProject *project = new XMLProject(project_file);
	tmm = new TMM(project);
	tmm->sendTo(ts_output);
	delete tmm;
	delete project;

	return 0;
}
