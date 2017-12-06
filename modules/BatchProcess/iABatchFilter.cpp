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
#include "iABatchFilter.h"

#include "iAAttributeDescriptor.h"
#include "iAConnector.h"
#include "iAConsole.h"
#include "iAFilterRegistry.h"
#include "iAStringHelper.h"
#include "io/iAITKIO.h"
#include "io/iAFileUtils.h"

#include <QCollator>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>

iABatchFilter::iABatchFilter():
	iAFilter("Batch...", "Image Ensembles",
		"Runs a filter on a selected set of images.<br/>"
		"Specify an <em>Image folder</em> which contains the images to be processed. "
		"<em>Recursive</em> toggles whether or not to also consider subdirectories. "
		"The <em>File mask</em> is applied to match which files in the given folder are processed "
		"(separate multiple masks via ';', e.g. '*.mhd;*.tif'. "
		"The specified <em>Filter</em> is applied to all files specified with above settings, "
		"every time executed with the same set of <em>Parameters</em>.<br/>"
		"When <em>Output file</em> is not empty, all output values produced by the filter "
		"will be written to the file name given here, one row per image and filter. "
		"If the output csv file exists, and <em>Append to output</em> is enabled, "
		"the output values are appended at the end of each line. Note that this tool does not "
		"try to find the matching filename in the given output csv, and hence the appending only "
		"works properly (i.e. appends values to the proper file) if no images have been added or "
		"removed from the folder, and also the recursive and file mask options are the same as "
		"with the batch run that created the file in the first place. "
		"If <em>Add filename</em> is enabled, then the name of the file processed for that "
		"line will be appended before the first output value from that file.", 0, 0)
{
	AddParameter("Image folder", String, "");
	AddParameter("Recursive", Boolean, false);
	AddParameter("File mask", String, "*.mhd");
	AddParameter("Filter", String, "Image Quality");
	AddParameter("Parameters", String, "");
	AddParameter("Output csv file", String, "");
	AddParameter("Append to output", Boolean, true);
	AddParameter("Add filename", Boolean, true);
}


void BatchDirectory(QString const & directory, QStringList filters, bool recurse, size_t & curLine,
	QStringList & outputBuffer, QMap<QString, QVariant> const & filterParams,
	QVector<iAConnector*> & inputImages, bool addFileName, QSharedPointer<iAFilter> filter,
	QString const & baseDir)
{
	QDir dir(directory);
	dir.setSorting(QDir::NoSort);
	QDir::Filters flags = QDir::Files;
	if (recurse)
		flags = QDir::Files | QDir::AllDirs;
	QStringList entryList = dir.entryList(filters, flags);
	QCollator collator;
	collator.setNumericMode(true);
	std::sort(entryList.begin(), entryList.end(), collator);	// natural sorting
	for (QString fileName: entryList)
	{
		if (fileName == "." || fileName == "..")
			continue;
		QFileInfo fi(directory + "/" + fileName);
		if (fi.isDir())
			BatchDirectory(fi.absoluteFilePath(), filters, recurse, curLine, outputBuffer,
				filterParams, inputImages, addFileName, filter, baseDir);
		else
		{
			iAITKIO::ScalarPixelType pixelType;
			iAITKIO::ImagePointer img = iAITKIO::readFile(fi.absoluteFilePath(), pixelType, false);
			inputImages[0]->SetImage(img);
			filter->Run(filterParams);
			if (curLine == 0)
			{
				QStringList captions;
				if (addFileName)
					captions << "filename";
				for (auto outValue : filter->OutputValues())
					captions << outValue.first;
				if (outputBuffer.empty())
					outputBuffer.append("");
				outputBuffer[0] += (outputBuffer[0].isEmpty() || captions.empty() ? "" : ",") + captions.join(",");
				++curLine;
			}
			if (curLine >= outputBuffer.size())
				outputBuffer.append("");
			QStringList values;
			if (addFileName)
			{
				QString relFileName = MakeRelative(baseDir, fi.absoluteFilePath());
				values << relFileName;
			}
			for (auto outValue : filter->OutputValues())
				values.append(outValue.second.toString());
			QString textToAdd = (outputBuffer[curLine].isEmpty() || values.empty() ? "" : ",") + values.join(",");
			outputBuffer[curLine] += textToAdd;
			++curLine;
		}
	}
}

void iABatchFilter::PerformWork(QMap<QString, QVariant> const & parameters)
{
	auto filter = iAFilterRegistry::Filter(parameters["Filter"].toString());
	if (!filter)
	{
		DEBUG_LOG(QString("Batch: Cannot run filter '%1', it does not exist!").arg(parameters["Filter"].toString()));
		return;
	}
	QMap<QString, QVariant> filterParams;
	QStringList filterParamStrs = SplitPossiblyQuotedString(parameters["Parameters"].toString());
	if (filter->Parameters().size() != filterParamStrs.size())
	{
		DEBUG_LOG(QString("Batch: Invalid number of parameters: %1 expected, %2 given!")
			.arg(filter->Parameters().size())
			.arg(filterParamStrs.size()));
		return;
	}

	iAConnector* con = new iAConnector();
	QVector<iAConnector*> inputImages;
	inputImages.push_back(con);

	for (int i=0; i<filterParamStrs.size(); ++i)
		filterParams.insert(filter->Parameters()[i]->Name(), filterParamStrs[i]);

	QString outputFile = parameters["Output csv file"].toString();
	QStringList outputBuffer;
	if (parameters["Append to output"].toBool() && QFile(outputFile).exists())
	{
		QFile file(outputFile);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QTextStream textStream(&file);
			while (!textStream.atEnd())
				outputBuffer << textStream.readLine();
			file.close();
		}
	}
	filter->SetUp(inputImages, m_log, m_progress);

	QStringList filters = parameters["File mask"].toString().split(";");

	size_t curLine = 0;
	BatchDirectory(parameters["Image folder"].toString(), filters, parameters["Recursive"].toBool(),
		curLine, outputBuffer, filterParams, inputImages, parameters["Add filename"].toBool(),
		filter, parameters["Image folder"].toString());

	if (!outputFile.isEmpty())
	{
		QFile file(outputFile);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream textStream(&file);
			for (QString line : outputBuffer)
			{
				textStream << line << endl;
			}
			file.close();
		}
	}
}

IAFILTER_CREATE(iABatchFilter);
