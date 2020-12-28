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

#include <iAVtkWidgetFwd.h>

#include <vtkSmartPointer.h>

#include <QList>
#include <QObject>
#include <QSharedPointer>

class iAMeanObjectData;
class iAMeanObjectDockWidget;
class iAMeanObjectTFView;

class iAMdiChild;

class vtkCamera;
class vtkTable;

class QColor;
class QDockWidget;
class QStandardItem;

class iAMeanObject: public QObject
{
	Q_OBJECT
public:
	iAMeanObject(iAMdiChild* activeChild, QString const& sourcePath);
	void render(QStandardItem* root, int classCount, QList<vtkSmartPointer<vtkTable>> const & tableList,
		int filterID, QDockWidget* nextToDW, vtkCamera* commonCamera, QList<QColor> const & classColor);
private slots:
	void modifyMeanObjectTF();
	void updateMOView();
	void browseFolderDialog();
	void saveStl();
private:
	iAMeanObjectDockWidget* m_dwMO;
	iAMeanObjectTFView* m_motfView;
	QSharedPointer<iAMeanObjectData> m_MOData;
	iAVtkOldWidget* m_meanObjectWidget;
	iAMdiChild* m_activeChild;
	int m_filterID;
	QString m_sourcePath;
};