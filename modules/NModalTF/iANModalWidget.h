/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2019  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                          Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth       *
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

#include "iAModuleAttachmentToChild.h"

#include <QDockWidget>

class MdiChild;


// n-Modal Widget -------------------------------------------------------------------------

class QLabel;

struct LabeledVoxel {
	int x;
	int y;
	int z;
	double scalar;
	double r;
	double g;
	double b;
	bool remover = false;
	QString text() {
		return QString::number(x) + "," + QString::number(y) + "," + QString::number(z) + "," + QString::number(scalar) + "," + QString::number(r) + "," + QString::number(g) + "," + QString::number(b);
	}
};

class iANModalWidget : public QDockWidget {
	Q_OBJECT

public:
	iANModalWidget(MdiChild *mdiChild);

private:
	MdiChild *m_mdiChild;
	QLabel *m_label;

	// TEMPORARY STUFF
	void adjustTf();

private slots:
	void onButtonClicked();
};


class iANModalWidgetAttachment : public iAModuleAttachmentToChild {
	Q_OBJECT
public:
	static iANModalWidgetAttachment* create(MainWindow *mainWnd, MdiChild *child);
	void start();
private:
	iANModalWidgetAttachment(MainWindow *mainWnd, MdiChild *child);
	iANModalWidget *m_nModalWidget;
};
