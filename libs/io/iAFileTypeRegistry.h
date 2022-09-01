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

#include "iAio_export.h"

#include "iADataSetType.h"
#include "iAFileIO.h"
#include "iAGenericFactory.h"
#include "iALog.h"

#include <QMap>

#include <vector>

class QString;

using iAIFileIOFactory = iAGenericFactory<iAFileIO>;

//! Registry for file types (of type iAFileIO).
class iAio_API iAFileTypeRegistry final
{
public:
	//! Adds a given file type to the registry.
	template <typename FileIOType> static void addFileType();

	//! Create a file I/O for the given extension
	static std::shared_ptr<iAFileIO> createIO(QString const & fileName);

	//! Retrieve list of file types for file open/save dialog
	static QString registeredFileTypes(iAFileIO::Operation op, iADataSetTypes allowedTypes = iADataSetType::All);

private:
	static std::vector<std::shared_ptr<iAIFileIOFactory>> m_fileIOs;
	static QMap<QString, size_t> m_fileTypes;
	iAFileTypeRegistry() = delete;  //!< class is meant to be used statically only, prevent creation of objects
};


template <typename FileIOType>
using iAFileIOFactory = iASpecificFactory<FileIOType, iAFileIO>;

template <typename FileIOType>
void iAFileTypeRegistry::addFileType()
{
	auto ioFactory = std::make_shared<iAFileIOFactory<FileIOType>>();
	m_fileIOs.push_back(ioFactory);
	auto io = ioFactory->create();
	for (auto extension : io->extensions())
	{
		auto lowerExt = extension.toLower();
		if (m_fileTypes.contains(extension))
		{
			LOG(lvlWarn, QString("File IO %1 tries to add a handler for file extension %2, already registered to file IO %3!")
				.arg(io->name())
				.arg(lowerExt)
				.arg(m_fileIOs[m_fileTypes[lowerExt]]->create()->name()));
		}
		m_fileTypes.insert(lowerExt, m_fileIOs.size() - 1);
	}
}