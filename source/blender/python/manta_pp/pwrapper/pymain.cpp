/******************************************************************************
 *
 * MantaFlow fluid solver framework
 * Copyright 2011 Tobias Pfaff, Nils Thuerey 
 *
 * This program is free software, distributed under the terms of the
 * GNU General Public License (GPL) 
 * http://www.gnu.org/licenses
 *
 * Main file
 *
 ******************************************************************************/
#ifndef _MANTA_PYMAIN_CPP_
#define _MANTA_PYMAIN_CPP_

#include "pythonInclude.h"
#include <stdio.h>
#include "manta.h"
#include "../general.h"
#include "wchar.h"

namespace Manta {
	extern void guiMain(int argc, char* argv[]);
	extern void guiWaitFinish();
}

using namespace std;
using namespace Manta;

#if PY_MAJOR_VERSION >= 3
typedef wchar_t pyChar;
typedef wstring pyString;
#else
typedef char pyChar;
typedef string pyString;
#endif

//*****************************************************************************
// main...
static bool manta_initialized = false;
void runMantaScript(vector<string>& args) {
	string filename = args[0];
	
	// Initialize extension classes and wrappers
	srand(0);
	PyGILState_STATE gilstate = PyGILState_Ensure();
	debMsg("running manta init?", 0);
	
	if (! manta_initialized)
	{	
		debMsg("yes", 0);
		Pb::setup(filename, args);
		manta_initialized = true;
	}	
	// Pass through the command line arguments
	// for Py3k compatability, convert to wstring
	vector<pyString> pyArgs(args.size());
	const pyChar ** cargs = new const pyChar*  [args.size()];
	for (size_t i=0; i<args.size(); i++) {
		pyArgs[i] = pyString(args[i].begin(), args[i].end());
		cargs[i] = pyArgs[i].c_str();
	}
	PySys_SetArgv( args.size(), (pyChar**) cargs);
	
	// Try to load python script
	FILE* fp = fopen(filename.c_str(),"rb");
	if (fp == NULL) {
		debMsg("Cannot open '" << filename << "'", 0);
		Pb::finalize();
		return;
	}
	
	// Run the python script file
	debMsg("Loading script '" << filename << "'", 0);
#if defined(WIN32) || defined(_WIN32)
	// known bug workaround: use simplestring
	fseek(fp,0,SEEK_END);
	long filelen=ftell(fp);
	fseek(fp,0,SEEK_SET);
	char* buf = new char[filelen+1];
	fread(buf,filelen,1,fp);
	buf[filelen] = '\0';
	fclose(fp);
	PyRun_SimpleString(buf);
	delete[] buf;    
#else
	// for linux, use this as it produces nicer error messages
	PyRun_SimpleFileEx(fp, filename.c_str(), 1);    
	fclose(fp);
//	Pb::finalize();
//	Pb::setup(filename, args);
//	fp = fopen(filename.c_str(),"rb");    
//	debMsg("one more time",0);
//	PyRun_SimpleFileEx(fp, filename.c_str(), 1);    
	fclose(fp);    
#endif
	
	debMsg("Script finished.", 0);
#ifdef GUI
//	guiWaitFinish();
#endif

	// finalize
//	Pb::finalize();
	PyGILState_Release(gilstate);

	delete [] cargs;
}

int manta_main(int argc,char* argv[]) {
	debMsg("Version: "<< buildInfoString() , 1);

#ifdef GUI
	guiMain(argc, argv);    
#else
	if (argc<=1) {
		cerr << "Usage : Syntax is 'manta <config.py>'" << endl;  
		return 1;
	}

	vector<string> args;
	for (int i=1; i<argc; i++) args.push_back(argv[i]);
	runMantaScript(args);
#endif        		

	return 0;
}
#endif