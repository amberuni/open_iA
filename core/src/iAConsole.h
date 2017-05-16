/*************************************  open_iA  ************************************ *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2017  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
*                          J. Weissenböck, Artem & Alexander Amirkhanov, B. Fröhler   *
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

#include "open_iA_Core_export.h"

#include <vtkSmartPointer.h>

#include <QObject>

#include <string>
#include <sstream>
#include <QString>

class dlg_console;
class iARedirectVtkOutput;

#define DEBUG_LOG(t) iAConsole::GetInstance().Log(t)

//! Debugging helper class. Instantiates a dlg_console to
//! log debug messages
//! TODO: check if we can't reuse logging window here!
class open_iA_Core_API iAConsole: public QObject
{
	Q_OBJECT
public:
	static iAConsole& GetInstance();
	static void Close();

	void Log(std::string const & text);
	void Log(char const * text);
	void Log(QString const & text);
	void SetLogToFile(bool value, QString const & fileName);
	bool IsLogToFileOn() const;
	QString GetLogFileName() const;
// decouple logging methods from GUI logging (to allow logging from any thread):
signals:
	void LogSignal(QString const & text);
private slots:
	void LogSlot(QString const & text);
private:
	iAConsole();
	~iAConsole();

	iAConsole(iAConsole const&)			= delete;
	void operator=(iAConsole const&)	= delete;

	void close();

	QString m_logFileName;
	dlg_console* m_console;
	bool m_logToFile;
	bool m_closed;
	vtkSmartPointer<iARedirectVtkOutput> m_vtkOutputWindow;
};
