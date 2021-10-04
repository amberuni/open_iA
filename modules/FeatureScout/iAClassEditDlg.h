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
#pragma once

#include <QDialog>

class QDialogButtonBox;
class QLineEdit;
class QLabel;
class QString;

class iAClassEditDlg : public QDialog
{
	Q_OBJECT
public:
	static QString getClassInfo(const QString &title, const QString &text, QColor &color, bool &ok);

private:
	iAClassEditDlg();
	void setTextValue(const QString &text);
	QString getTextValue();
	void setColor(QColor const & color);
	void getColor(QColor & color);
	void getColorDialog();

	QLabel *cNameLabel, *cColorLabel;
	QLineEdit *nameEdit;
	QPushButton *colorButton;
	QDialogButtonBox *buttonBox;
	QColor dcolor;
};