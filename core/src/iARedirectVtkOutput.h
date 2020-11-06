/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2020  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                 Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth, P. Weinberger *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#pragma once

#include "iALog.h"

#include <vtkOutputWindow.h>
#include <vtkObjectFactory.h>

class iARedirectVtkOutput : public vtkOutputWindow
{
public:
	vtkTypeMacro(iARedirectVtkOutput, vtkOutputWindow);
	void PrintSelf(ostream& os, vtkIndent indent) override;
	static iARedirectVtkOutput * New();
	void DisplayText(const char*) override;
private:
	iARedirectVtkOutput();
	iARedirectVtkOutput(const iARedirectVtkOutput &) = delete;
	void operator=(const iARedirectVtkOutput &) = delete;
};

vtkStandardNewMacro(iARedirectVtkOutput);

iARedirectVtkOutput::iARedirectVtkOutput() {}

void iARedirectVtkOutput::DisplayText(const char* someText)
{
	iALogLevel lvl = lvlWarn;
	switch (GetCurrentMessageType())
	{
	case MESSAGE_TYPE_TEXT           : lvl = lvlInfo;  break;
	case MESSAGE_TYPE_ERROR          : lvl = lvlError; break;
	default:
#if __cplusplus >= 201703L
		[[fallthrough]];
#endif
	case MESSAGE_TYPE_WARNING        :
#if __cplusplus >= 201703L
		[[fallthrough]];
#endif
	case MESSAGE_TYPE_GENERIC_WARNING: lvl = lvlWarn;  break;
	case MESSAGE_TYPE_DEBUG          : lvl = lvlDebug; break;
	}
	LOG(lvl, someText);
}

//----------------------------------------------------------------------------
void iARedirectVtkOutput::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
}
