/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2022  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
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

#include "iAguibase_export.h"

#include <QString>

//! Interface for anything that can be saved as a project.
//! Necessary since not all current tools employ iAMdiChild as container for their widgets.
//! So every such container needs to implement this class and its doLoadProject method,
//! in order to be called when the user selects to "Save Project".
//!
//! Refactoring ideas:
//! - Move dataset rendering capabilities from iAMdiChild / MdiChild into a separate "viewer"
//!   tool, and make all separate tools using this as base class (only iAFeatureAnalyzer!) a
//!   tool under iAMdiChild
class iAguibase_API iASavableProject
{
public:
	//! Called from main window to save the project of the current window.
	//! In case you're wondering why there are two methods in this class, this one
	//! and the virtual "doSaveProject": This is because it follows the "Non-Virtual Interface
	//! Idiom", see http://www.gotw.ca/publications/mill18.htm
	bool saveProject(QString const & basePath);
	//! return the name of the last file that was stored
	QString const & fileName() const;
protected:
	//! Prevent destruction of the object through this interface.
	virtual ~iASavableProject();
private:
	//! Override this method to implement the actual saving of the project
	virtual bool doSaveProject(QString const & projectFileName) = 0;

	QString m_fileName;
};
