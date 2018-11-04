/////////////////////////////////////////////////////////////////////////////
// Name:        Process.cpp
// Author:      Alex Thuering
// Created:     26.09.2014
// RCS-ID:      $Id: Process.cpp,v 1.2 2014/10/20 05:58:08 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "Process.h"
#include "ProcessExecute.h"

bool Process::Exec(wxString command, wxString inputFile, wxString outputFile) {
	ProcessExecute exec(progressDlg);
	return exec.Execute(command, inputFile, outputFile);
}

bool Process::ExecAsync(wxString command) {
	progressDlg->AddDetailMsg(_("Executing command: ") + command);
	return wxExecute(command) != 0;
}

bool Process::DeleteFile(wxString fname) {
	if (wxFileExists(fname) && !wxRemoveFile(fname)) {
		progressDlg->AddDetailMsg(wxString::Format(_("Can't remove file '%s'"), fname.c_str()), *wxRED);
		return false;
	}
	return true;
}
