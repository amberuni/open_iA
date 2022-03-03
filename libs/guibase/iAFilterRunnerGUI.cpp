/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2021  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
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
#include "iAFilterRunnerGUI.h"

// base
#include "iAAttributeDescriptor.h"
#include "iAFileUtils.h"
#include "iAFilter.h"
#include "iALog.h"
//#include "iALogger.h"
#include "iAProgress.h"

// guibase
#include "dlg_modalities.h"
#include "iAConnector.h"
#include "iAJobListView.h"
#include "iAMainWindow.h"
#include "iAMdiChild.h"
#include "iAModality.h"
#include "iAModalityList.h"
#include "iAParameterDlg.h"
#include "iAPerformanceHelper.h"    // for formatDuration
#include "iAPreferences.h"

#include <vtkImageData.h>
#include <vtkPolyData.h>

#include <QElapsedTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>
#include <QSharedPointer>
#include <QStatusBar>
#include <QString>
#include <QVariant>

class iAFilter;

class vtkImageData;


// iAFilterRunnerGUIThread

iAFilterRunnerGUIThread::iAFilterRunnerGUIThread(QSharedPointer<iAFilter> filter, QMap<QString, QVariant> paramValues, iAMdiChild* sourceMDI) :
	m_filter(filter),
	m_paramValues(paramValues),
	m_sourceMDI(sourceMDI),
	m_aborted(false)
{
}

void iAFilterRunnerGUIThread::run()
{
	QElapsedTimer time;
	time.start();
	try
	{
		if (!m_filter->run(m_paramValues))
		{
			LOG(lvlError, "Running filter failed!");
			return;
		}
		if (m_aborted)
		{
			return;
		}
	}
	catch (itk::ExceptionObject& e)
	{
		LOG(lvlError, tr("%1 terminated unexpectedly. Error: %2; in File %3, Line %4. Elapsed time: %5")
				   .arg(m_filter->name())
				   .arg(e.GetDescription())
				   .arg(e.GetFile())
				   .arg(e.GetLine())
				.arg(formatDuration(time.elapsed() / 1000.0, true, false)));
		return;
	}
	catch (const std::exception& e)
	{
		LOG(lvlError, tr("%1 terminated unexpectedly. Error: %2. Elapsed time: %3")
				   .arg(m_filter->name())
				   .arg(e.what())
				.arg(formatDuration(time.elapsed() / 1000.0, true, false)));
		return;
	}
	LOG(lvlInfo,
		tr("%1 finished. Elapsed time: %2")
			.arg(m_filter->name())
			.arg(formatDuration(time.elapsed() / 1000.0, true, false)));
}

void iAFilterRunnerGUIThread::abort()
{
	m_aborted = true;
	m_filter->abort();
}

QSharedPointer<iAFilter> iAFilterRunnerGUIThread::filter()
{
	return m_filter;
}

void iAFilterRunnerGUIThread::addInput(vtkImageData* img, QString const& fileName)
{
	m_filter->addInput(img, fileName);
}

size_t iAFilterRunnerGUIThread::inputCount() const
{
	return m_filter->inputCount();
}

iAMdiChild* iAFilterRunnerGUIThread::sourceMDI()
{
	return m_sourceMDI;
}


namespace
{
	QString SettingName(QSharedPointer<iAFilter> filter, QString paramName)
	{
		QString filterNameShort(filter->name());
		filterNameShort.replace(" ", "");
		return QString("Filters/%1/%2/%3").arg(filter->category()).arg(filterNameShort).arg(paramName);
	}
}


// iAFilterRunnerGUI


QSharedPointer<iAFilterRunnerGUI> iAFilterRunnerGUI::create()
{
	return QSharedPointer<iAFilterRunnerGUI>::create();
}

QMap<QString, QVariant> iAFilterRunnerGUI::loadParameters(QSharedPointer<iAFilter> filter, iAMdiChild* sourceMdi)
{
	auto params = filter->parameters();
	QMap<QString, QVariant> result;
	QSettings settings;
	for (auto param : params)
	{
		QVariant defaultValue = (param->valueType() == iAValueType::Categorical) ? "" : param->defaultValue();
		QVariant value = (param->valueType() == iAValueType::FileNameSave && sourceMdi)
			? pathFileBaseName(sourceMdi->fileInfo()) + param->defaultValue().toString()
			: settings.value(SettingName(filter, param->name()), defaultValue);
		result.insert(param->name(), value);
	}
	return result;
}

void iAFilterRunnerGUI::storeParameters(QSharedPointer<iAFilter> filter, QMap<QString, QVariant> & paramValues)
{
	auto params = filter->parameters();
	QSettings settings;
	for (QString key : paramValues.keys())
	{
		settings.setValue(SettingName(filter, key), paramValues[key]);
	}
	// just some checking whether there are values for all parameters:
	for (auto param : params)
	{
		if (!paramValues.contains(param->name()))
		{
			LOG(lvlError, QString("No value for parameter '%1'").arg(param->name()));
		}
	}
}

bool iAFilterRunnerGUI::askForParameters(QSharedPointer<iAFilter> filter, QMap<QString, QVariant> & paramValues,
	iAMdiChild* sourceMdi, iAMainWindow* mainWnd, bool askForAdditionalInput)
{
	iAAttributes dlgParams;
	bool showROI = false;	// TODO: find better way to check this?
	for (auto filterParam : filter->parameters())
	{
		QSharedPointer<iAAttributeDescriptor> p(filterParam->clone());
		if (p->valueType() == iAValueType::Categorical)
		{
			QStringList comboValues = p->defaultValue().toStringList();
			QString storedValue = paramValues[p->name()].toString();
			selectOption(comboValues, storedValue);
			p->setDefaultValue(comboValues);
		}
		else
		{
			p->setDefaultValue(paramValues[p->name()]);
		}
		if (p->name() == "Index X")
		{
			showROI = true;
		}
		dlgParams.push_back(p);
	}
	if (filter->requiredInputs() == 1 && dlgParams.empty())
	{
		return true;
	}
	QVector<iAMdiChild*> otherMdis;
	for (auto mdi : mainWnd->mdiChildList())
	{
		if (mdi != sourceMdi)
		{
			otherMdis.push_back(mdi);
		}
	}
	if (askForAdditionalInput && filter->requiredInputs() > static_cast<unsigned int>(otherMdis.size()+1) )
	{
		QMessageBox::warning(mainWnd, filter->name(),
			QString("This filter requires %1 datasets, only %2 open file(s)!")
			.arg(filter->requiredInputs()).arg(otherMdis.size()+1));
		return false;
	}
	QStringList mdiChildrenNames;
	if (askForAdditionalInput && filter->requiredInputs() > 1)
	{
		for (auto mdi: otherMdis)
		{
			mdiChildrenNames << mdi->windowTitle().replace("[*]", "");
		}
		for (unsigned int i = 1; i < filter->requiredInputs(); ++i)
		{
			addParameter(dlgParams, QString("%1").arg(filter->inputName(i)), iAValueType::Categorical, mdiChildrenNames);
		}
	}
	iAParameterDlg dlg(mainWnd, filter->name(), dlgParams, filter->description());
	dlg.setModal(false);
	dlg.hide();	dlg.show(); // required to apply change in modality!
	if (sourceMdi)
	{
		dlg.setSourceMdi(sourceMdi, mainWnd);
	}
	if (showROI)
	{
		dlg.showROI();
	}
	if (dlg.exec() != QDialog::Accepted)
	{
		return false;
	}
	paramValues = dlg.parameterValues();
	if (askForAdditionalInput && filter->requiredInputs() > 1)
	{
		for (unsigned int i = 1; i < filter->requiredInputs(); ++i)
		{
			QString selectedFile = paramValues[QString("%1").arg(filter->inputName(i))].toString();
			int const mdiIdx = mdiChildrenNames.indexOf(selectedFile);
			for (int m = 0; m < otherMdis[mdiIdx]->modalities()->size(); ++m)
			{
				m_additionalInput.push_back(otherMdis[mdiIdx]->modality(m)->image());
				m_additionalFileNames.push_back(otherMdis[mdiIdx]->modality(m)->fileName());
			}
		}
	}
	return true;
}

void iAFilterRunnerGUI::filterGUIPreparations(QSharedPointer<iAFilter> /*filter*/,
	iAMdiChild* /*mdiChild*/, iAMainWindow* /*mainWnd*/, QMap<QString, QVariant> const & /*params*/)
{
}

void iAFilterRunnerGUI::run(QSharedPointer<iAFilter> filter, iAMainWindow* mainWnd)
{
	iAMdiChild* sourceMdi = mainWnd->activeMdiChild();
	if (filter->requiredInputs() > 0 && (!sourceMdi || !sourceMdi->isFullyLoaded()))
	{
		LOG(lvlWarn,QString("Filter requires %1 input(s), but %2!")
			.arg(filter->requiredInputs())
			.arg(!sourceMdi ? "no source file is available" : "source file is not fully loaded yet"));
		emit finished();
		return;
	}
	QMap<QString, QVariant> paramValues = loadParameters(filter, sourceMdi);
	filter->adaptParametersToInput(paramValues, sourceMdi? sourceMdi->modality(0)->image(): nullptr);

	if (!askForParameters(filter, paramValues, sourceMdi, mainWnd, true))
	{
		emit finished();
		return;
	}
	storeParameters(filter, paramValues);

	//! TODO: find way to check parameters already in iAParameterDlg (before closing)
	if (!filter->checkParameters(paramValues))
	{
		emit finished();
		return;
	}

	QString oldTitle(sourceMdi ? sourceMdi->windowTitle() : "");
	oldTitle = oldTitle.replace("[*]", "").trimmed();
	QString newTitle(filter->outputName(0, filter->name()) + " " + oldTitle);
	m_sourceFileName = sourceMdi ? sourceMdi->modality(0)->fileName() : "";

	filterGUIPreparations(filter, sourceMdi, mainWnd, paramValues);
	auto thread = new iAFilterRunnerGUIThread(filter, paramValues, sourceMdi);
	if (!thread)
	{
		mainWnd->statusBar()->showMessage("Cannot create result calculation thread!", 5000);
		emit finished();
		return;
	}
	if (sourceMdi)
	{
		for (int m = 0; m < sourceMdi->modalities()->size(); ++m)
		{
			thread->addInput(sourceMdi->modality(m)->image(), sourceMdi->modality(m)->fileName());
		}
		filter->setFirstInputChannels(sourceMdi->modalities()->size());
	}
	for (int a=0; a < m_additionalInput.size(); ++a)
	{
		thread->addInput(m_additionalInput[a], m_additionalFileNames[a]);
	}
	if (thread->inputCount() < filter->requiredInputs())
	{
		LOG(lvlError, QString("Not enough inputs specified, filter %1 requires %2 input images!")
			.arg(filter->name()).arg(filter->requiredInputs()));
		emit finished();
		return;
	}
	if (mainWnd->defaultPreferences().PrintParameters && !filter->parameters().isEmpty())
	{
		LOG(lvlInfo, QString("Starting %1 filter with parameters:").arg(filter->name()));
		for (int p = 0; p < filter->parameters().size(); ++p)
		{
			auto paramDescriptor = filter->parameters()[p];
			QString paramName = paramDescriptor->name();
			QString paramValue = paramDescriptor->valueType() == iAValueType::Boolean ?
				(paramValues[paramName].toBool() ? "yes" : "no")
				: paramValues[paramName].toString();
			LOG(lvlInfo, QString("    %1 = %2").arg(paramName).arg(paramValue));
		}
	}
	else
	{
		LOG(lvlInfo, QString("Starting %1 filter.").arg(thread->filter()->name()));
	}
	connect(thread, &QThread::finished, this, &iAFilterRunnerGUI::filterFinished);
	iAJobListView::get()->addJob(filter->name(), filter->progress(), thread, filter->canAbort() ? thread : nullptr);
	mainWnd->statusBar()->showMessage(filter->name(), 5000);
	thread->start();
}

void iAFilterRunnerGUI::filterFinished()
{
	auto thread = qobject_cast<iAFilterRunnerGUIThread*>(sender());
	auto filter = thread->filter();
	if (filter->finalOutputCount() > 0 || filter->polyOutput())
	{
		QFileInfo sourceFI(m_sourceFileName);
		QString newName(filter->name() + " " + sourceFI.baseName());
		iAMdiChild* newChild = thread->sourceMDI()->mainWnd()->resultChild(thread->sourceMDI(), newName);
		newChild->show();
		// TODO: cleanup to add all modalities below!
		newChild->displayResult(newName, filter->finalOutputCount() > 0 ? filter->output(0)->vtkImage() : nullptr,
			filter->polyOutput() ? filter->polyOutput(): nullptr);
		newChild->enableRenderWindows();
		if (filter->finalOutputCount() > 0)
		{
			auto filterNameSaveForFilename = filter->name().replace(QRegularExpression("[\\\\/:*?\"<>| ]"), "_");
			QString suggestedFileName = sourceFI.absolutePath() + "/" + sourceFI.completeBaseName() + "-" +
				filterNameSaveForFilename + "." + sourceFI.suffix();
			newChild->modality(0)->setFileName(suggestedFileName);
		}
		for (size_t p = 1; p < filter->finalOutputCount(); ++p)
		{
			auto img = vtkSmartPointer<vtkImageData>::New();
			// some filters apparently clean up the result image
			// (disregarding that a smart pointer still points to it...)
			// so let's copy it to be on the safe side!
			img->DeepCopy(filter->output(p)->vtkImage());
			QString outputName = filter->outputName(p, QString("Output %1").arg(p));
			auto mod = QSharedPointer<iAModality>::create(outputName, "", -1, img, 0);
			newChild->modalities()->add(mod);
			// signal to add it to list automatically is created to late to be effective here, we have to add it to list ourselves:
			newChild->dataDockWidget()->modalityAdded(mod);
		}
	}
	for (auto outputValue : filter->outputValues())
	{
		LOG(lvlImportant, QString("%1: %2").arg(outputValue.first).arg(outputValue.second.toString()));
	}
	thread->deleteLater();
	emit finished();
}
