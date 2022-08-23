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

#include <iAqthelper_export.h>

#include <QApplication>
#include <QWidget>

class QPropertyAnimation;

// inspired from https://stackoverflow.com/a/34445886

class iAqthelper_API iAWidgetAnimationDecorator: public QObject
{
	Q_OBJECT
	Q_PROPERTY(QColor color READ color WRITE setColor)
public:
	~iAWidgetAnimationDecorator();
	void setColor(QColor color);
	QColor color() const;
	static void animate(QWidget* animatedWidget,
		int duration = 2000,
		QColor startValue = QColor(255, 0, 0),
		QColor endValue = QApplication::palette().color(QPalette::Window),
		QString animatedQssProperty = "background-color");
private:
	iAWidgetAnimationDecorator(QWidget* animatedWidget, int duration,
		QColor startValue, QColor endValue, QString animatedQssProperty);
	QSharedPointer<QPropertyAnimation> m_animation;
	QString m_animatedQssProperty;
	QWidget* m_animatedWidget;
};