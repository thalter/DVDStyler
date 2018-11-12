/////////////////////////////////////////////////////////////////////////////
// Name:        Process.h
// Author:      Alex Thuering
// Created:     26.09.2014
// RCS-ID:      $Id: Process.h,v 1.2 2014/10/20 05:58:08 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef DS_PROCESS_H
#define DS_PROCESS_H

#include "ProgressDlg.h"

/**
 * Abstract process class
 */
class Process {
public:
	/** Constructor */
	Process(ProgressDlg* progressDlg) { this->progressDlg = progressDlg; }
	/** Denstructor */
	virtual ~Process() {}
	
	/** Executes process */
    virtual bool Execute() = 0;
    
    /** Returns true, if process need be executed */
    virtual bool IsNeedExecute() = 0;
    
    /** Returns true, if gauge need be updated */
    virtual bool IsUpdateGauge() { return true; };

protected:
    ProgressDlg* progressDlg;
	
	/** Executes given command */
	bool Exec(wxString command, wxString inputFile = wxEmptyString, wxString outputFile = wxEmptyString);
	
	/** Executes given command in background */
	bool ExecAsync(wxString command);
	
	/** Removes given file and print error message if it failed */
	bool DeleteFile(wxString fname);
};  

#endif // DS_PROCESS_H
