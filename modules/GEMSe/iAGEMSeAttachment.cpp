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
#include "iAGEMSeAttachment.h"

#include "dlg_GEMSeControl.h"
#include "dlg_GEMSe.h"
#include "dlg_samplings.h"
#include "iAGEMSeTool.h"

#include <dlg_modalities.h>
#include <iALog.h>
#include <iAColorTheme.h>
#include <iALogger.h>
#include <iAModality.h>
#include <iARenderer.h>
#include <iASlicer.h>
#include <iAMdiChild.h>
#include <iAMainWindow.h>

iAGEMSeAttachment::iAGEMSeAttachment(iAMainWindow * mainWnd, iAMdiChild * child):
	iAModuleAttachmentToChild(mainWnd, child),
	m_dummyTitleWidget(new QWidget())
{
	auto tool = std::make_shared<iAGEMSeTool>();
	tool->setMainWindow(mainWnd);
	child->addTool(iAGEMSeTool::ID, tool);
}

iAGEMSeAttachment* iAGEMSeAttachment::create(iAMainWindow * mainWnd, iAMdiChild * child)
{
	iAGEMSeAttachment * newAttachment = new iAGEMSeAttachment(mainWnd, child);

	QString defaultThemeName("Brewer Set3 (max. 12)");
	iAColorTheme const * colorTheme = iAColorThemeManager::instance().theme(defaultThemeName);

	newAttachment->m_dlgGEMSe = new dlg_GEMSe(child, iALog::get(), colorTheme);
	newAttachment->m_dlgSamplings = new dlg_samplings();
	newAttachment->m_dlgGEMSeControl = new dlg_GEMSeControl(
		child,
		newAttachment->m_dlgGEMSe,
		child->dataDockWidget(),
		newAttachment->m_dlgSamplings,
		colorTheme
	);
	child->splitDockWidget(child->renderDockWidget(), newAttachment->m_dlgGEMSe, Qt::Vertical);
	child->splitDockWidget(child->renderDockWidget(), newAttachment->m_dlgGEMSeControl, Qt::Horizontal);
	child->splitDockWidget(newAttachment->m_dlgGEMSeControl, newAttachment->m_dlgSamplings, Qt::Vertical);
	return newAttachment;
}

bool iAGEMSeAttachment::loadSampling(QString const & smpFileName, int labelCount, int datasetID)
{
	return m_dlgGEMSeControl->loadSampling(smpFileName, labelCount, datasetID);
}

bool iAGEMSeAttachment::loadClustering(QString const & fileName)
{
	return m_dlgGEMSeControl->loadClustering(fileName);
}

bool iAGEMSeAttachment::loadRefImg(QString const & refImgName)
{
	return m_dlgGEMSeControl->loadRefImg(refImgName);
}

void iAGEMSeAttachment::setSerializedHiddenCharts(QString const & hiddenCharts)
{
	return m_dlgGEMSeControl->setSerializedHiddenCharts(hiddenCharts);
}

void iAGEMSeAttachment::resetFilter()
{
	m_dlgGEMSe->ResetFilters();
}

void iAGEMSeAttachment::toggleAutoShrink()
{
	m_dlgGEMSe->ToggleAutoShrink();
}

void iAGEMSeAttachment::toggleDockWidgetTitleBar()
{
	QWidget* titleBar = m_dlgGEMSe->titleBarWidget();
	if (titleBar == m_dummyTitleWidget)
	{
		m_dlgGEMSe->setTitleBarWidget(nullptr);
	}
	else
	{
		m_dlgGEMSe->setTitleBarWidget(m_dummyTitleWidget);
	}
}

void iAGEMSeAttachment::exportClusterIDs()
{
	m_dlgGEMSeControl->exportIDs();
}

void iAGEMSeAttachment::exportAttributeRangeRanking()
{
	m_dlgGEMSeControl->exportAttributeRangeRanking();
}

void iAGEMSeAttachment::exportRankings()
{
	m_dlgGEMSeControl->exportRankings();
}

void iAGEMSeAttachment::importRankings()
{
	m_dlgGEMSeControl->importRankings();
}

void iAGEMSeAttachment::setLabelInfo(QString const & colorTheme, QString const & labelNames)
{
	m_dlgGEMSeControl->setLabelInfo(colorTheme, labelNames);
}

void iAGEMSeAttachment::saveProject(QSettings & metaFile, QString const & fileName)
{
	m_dlgGEMSeControl->saveProject(metaFile, fileName);
}