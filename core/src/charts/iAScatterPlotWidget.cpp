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
#include "iAScatterPlotWidget.h"

#include "iALog.h"
#include "iALookupTable.h"
#include "iAScatterPlot.h"
#include "iAScatterPlotSelectionHandler.h"
#include "iASPLOMData.h"

#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

#include <QMouseEvent>
#include <QPainter>

// for popup / tooltip:
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>

iAScatterPlotSelectionHandler::~iAScatterPlotSelectionHandler()
{}

class iAScatterPlotStandaloneHandler : public iAScatterPlotSelectionHandler
{
public:
	SelectionType & getSelection() override
	{
		return m_selection;
	}
	SelectionType const & getSelection() const override
	{
		return m_selection;
	}
	SelectionType const & getFilteredSelection() const override
	{
		return m_selection;
	}
	void setSelection(SelectionType const & selection)
	{
		m_selection = selection;
	}
	SelectionType const & getHighlightedPoints() const override
	{
		return m_highlight;
	}
	void clearHighlighted()
	{
		m_highlight.clear();
	}
	void addHighlightedPoint(size_t idx)
	{
		m_highlight.push_back(idx);
	}
	bool isHighlighted(size_t idx) const
	{
		return std::find(m_highlight.begin(), m_highlight.end(), idx) != m_highlight.end();
	}
	void removeHighlightedPoint(size_t idx)
	{
		m_highlight.erase(std::find(m_highlight.begin(), m_highlight.end(), idx));
	}
	int getVisibleParametersCount() const override
	{
		return 2;
	}
	double getAnimIn() const override
	{
		return 1.0;
	}
	double getAnimOut() const override
	{
		return 0.0;
	}
private:
	SelectionType m_highlight;
	SelectionType m_selection;
};

class iADefaultScatterPlotPointInfo : public iAScatterPlotPointInfo
{
public:
	iADefaultScatterPlotPointInfo(QSharedPointer<iASPLOMData> data) :
		m_data(data)
	{}
	QString text(const size_t paramIdx[2], size_t pointIdx) override
	{
		return m_data->parameterName(paramIdx[0]) + ": " +
			QString::number(m_data->paramData(paramIdx[0])[pointIdx]) + "<br>" +
			m_data->parameterName(paramIdx[1]) + ": " +
			QString::number(m_data->paramData(paramIdx[1])[pointIdx]);
	}
private:
	QSharedPointer<iASPLOMData> m_data;
};

namespace
{
	const int PaddingLeftBase = 2;
	const int PaddingBottomBase = 2;
}
const int iAScatterPlotWidget::PaddingTop = 5;
const int iAScatterPlotWidget::PaddingRight = 5;
const int iAScatterPlotWidget::TextPadding = 5;


iAScatterPlotWidget::iAScatterPlotWidget(QSharedPointer<iASPLOMData> data) :
	m_data(data),
	m_scatterPlotHandler(new iAScatterPlotStandaloneHandler()),
	m_fontHeight(0),
	m_maxTickLabelWidth(0),
	m_fixPointsEnabled(false),
	m_pointInfo(new iADefaultScatterPlotPointInfo(data))
{
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	m_scatterplot = new iAScatterPlot(m_scatterPlotHandler.data(), this);
	m_scatterplot->settings.selectionEnabled = true;
	data->updateRanges();
	if (data->numPoints() > std::numeric_limits<int>::max())
	{
		LOG(lvlWarn, QString("Number of points (%1) larger than supported (%2)")
			.arg(data->numPoints())
			.arg(std::numeric_limits<int>::max()));
	}
	m_scatterplot->setData(0, 1, data);
	connect(m_scatterplot, &iAScatterPlot::selectionModified, this, &iAScatterPlotWidget::selectionModified);
}

void iAScatterPlotWidget::setPlotColor(QColor const & c, double rangeMin, double rangeMax)
{
	QSharedPointer<iALookupTable> lut(new iALookupTable());
	double lutRange[2] = { rangeMin, rangeMax };
	lut->setRange(lutRange);
	lut->allocate(2);
	for (int i = 0; i < 2; ++i)
	{
		lut->setColor(i, c);
	}
	m_scatterplot->setLookupTable(lut, 0);
}

void iAScatterPlotWidget::setLookupTable(QSharedPointer<iALookupTable> lut, size_t paramIdx)
{
	m_scatterplot->setLookupTable(lut, paramIdx);
}

#ifdef CHART_OPENGL
void iAScatterPlotWidget::paintGL()
#else
void iAScatterPlotWidget::paintEvent(QPaintEvent* event)
#endif
{
	QPainter painter(this);
	QFontMetrics fm = painter.fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	if (m_fontHeight != fm.height() || m_maxTickLabelWidth != fm.horizontalAdvance("0.99"))
	{
		m_fontHeight = fm.height();
		m_maxTickLabelWidth = fm.horizontalAdvance("0.99");
#else
	if (m_fontHeight != fm.height() || m_maxTickLabelWidth != fm.width("0.99"))
	{
		m_fontHeight = fm.height();
		m_maxTickLabelWidth = fm.width("0.99");
#endif
	}
	painter.setRenderHint(QPainter::Antialiasing);
	QColor bgColor(QWidget::palette().color(QWidget::backgroundRole()));
	QColor fg(QWidget::palette().color(QPalette::Text));
	m_scatterplot->settings.tickLabelColor = fg;
#ifdef CHART_OPENGL
	painter.beginNativePainting();
	glClearColor(bgColor.red() / 255.0, bgColor.green() / 255.0, bgColor.blue() / 255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	painter.endNativePainting();
#else
	Q_UNUSED(event);
	painter.fillRect(rect(), bgColor);
#endif
	m_scatterplot->paintOnParent(painter);
	// print axes tick labels:
	painter.save();
	QList<double> ticksX, ticksY; QList<QString> textX, textY;
	m_scatterplot->printTicksInfo(&ticksX, &ticksY, &textX, &textY);
	painter.setPen(m_scatterplot->settings.tickLabelColor);
	QPoint tOfs(PaddingLeft(), PaddingBottom());
	long tSpc = 5;
	for (long i = 0; i < ticksY.size(); ++i)
	{
		double t = ticksY[i]; QString text = textY[i];
		QRectF textRect(0, t - tOfs.y(), tOfs.x() - tSpc, 2 * tOfs.y());
		//LOG(lvlInfo, QString("text rect: %1,%2, %3x%4").arg(textRect.left()).arg(textRect.top()).arg(textRect.width()).arg(textRect.height()));
		painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, text);
	}
	painter.rotate(-90);
	for (long i = 0; i < ticksX.size(); ++i)
	{
		double t = ticksX[i]; QString text = textX[i];
		painter.drawText(QRectF(-tOfs.y() + tSpc + PaddingBottom() - height() - TextPadding,
				t - tOfs.x(), tOfs.y() - tSpc, 2 * tOfs.x()), Qt::AlignRight | Qt::AlignVCenter, text);
	}
	painter.restore();

	// print axes labels:
	painter.save();
	painter.setPen(m_scatterplot->settings.tickLabelColor);
	painter.drawText(QRectF(-PaddingLeft(), height() - fm.height() - TextPadding, width(), fm.height()),
			Qt::AlignHCenter | Qt::AlignTop, m_data->parameterName(0));
	painter.rotate(-90);
	painter.drawText(QRectF(-height(), 0, height(), fm.height()), Qt::AlignCenter | Qt::AlignTop, m_data->parameterName(1));
	painter.restore();

	drawTooltip(painter);
}

void iAScatterPlotWidget::drawTooltip(QPainter& painter)
{
	size_t curInd = m_scatterplot->getCurrentPoint();
	if (curInd == iAScatterPlot::NoPointIndex)
	{
		return;
	}
	const size_t* pInds = m_scatterplot->getIndices();

	painter.save();
	QPointF popupPos = m_scatterplot->getPointPosition(curInd);
	double pPM = m_scatterplot->settings.pickedPointMagnification;
	double ptRad = m_scatterplot->getPointRadius();
	popupPos.setY(popupPos.y() - pPM * ptRad);
	QColor popupFillColor(palette().color(QPalette::Window));
	painter.setBrush(popupFillColor);
	QColor popupBorderColor(palette().color(QPalette::Dark));
	painter.setPen(popupBorderColor);
	painter.translate(popupPos);
	QString text = "<b>#" + QString::number(curInd) + "</b><br> " +
		m_pointInfo->text(pInds, curInd);
	QTextDocument doc;
	doc.setHtml(text);
	int popupWidth = 200;	// = settings.popupWidth
	doc.setTextWidth(popupWidth);
	double tipDim[2] = { 5, 10 }; // = settings.popupTipDim
	double popupWidthHalf = popupWidth / 2; // settings.popupWidth / 2
	auto popupHeight = doc.size().height();
	QPointF points[7] = {
		QPointF(0, 0),
		QPointF(-tipDim[0], -tipDim[1]),
		QPointF(-popupWidthHalf, -tipDim[1]),
		QPointF(-popupWidthHalf, -popupHeight - tipDim[1]),
		QPointF(popupWidthHalf, -popupHeight - tipDim[1]),
		QPointF(popupWidthHalf, -tipDim[1]),
		QPointF(tipDim[0], -tipDim[1]),
	};
	painter.drawPolygon(points, 7);

	painter.translate(-popupWidthHalf, -popupHeight - tipDim[1]);
	QAbstractTextDocumentLayout::PaintContext ctx;
	QColor popupTextColor(palette().color(QPalette::ToolTipText) ); // = settings.popupTextColor;
	ctx.palette.setColor(QPalette::Text, popupTextColor);
	doc.documentLayout()->draw(&painter, ctx); //doc.drawContents( &painter );
	painter.restore();
}

void iAScatterPlotWidget::adjustScatterPlotSize()
{
	QRect size(geometry());
	size.moveTop(0);
	size.moveLeft(0);
	size.adjust(PaddingLeft(), PaddingTop, -PaddingRight, -PaddingBottom());
	//LOG(lvlInfo, QString("%1,%2 %3x%4").arg(size.top()).arg(size.left()).arg(size.width()).arg(size.height()));
	if (size.width() > 0 && size.height() > 0)
	{
		m_scatterplot->setRect(size);
	}
}

void iAScatterPlotWidget::resizeEvent(QResizeEvent* event)
{
	adjustScatterPlotSize();
	iAQGLWidget::resizeEvent( event );
}

int iAScatterPlotWidget::PaddingLeft()
{
	return PaddingLeftBase+m_fontHeight+m_maxTickLabelWidth+TextPadding;
}

int iAScatterPlotWidget::PaddingBottom()
{
	return PaddingBottomBase+m_fontHeight+m_maxTickLabelWidth+TextPadding;
}

void iAScatterPlotWidget::wheelEvent(QWheelEvent * event)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	if (event->x() >= PaddingLeft() && event->x() <= (width() - PaddingRight) &&
		event->y() >= PaddingTop && event->y() <= (height() - PaddingBottom()))
#else
	QPointF p = event->position();
	if (p.x() >= PaddingLeft() && p.x() <= (width() - PaddingRight) &&
		p.y() >= PaddingTop && p.y() <= (height() - PaddingBottom()))
#endif
	{
		m_scatterplot->SPLOMWheelEvent(event);
		update();
	}
}

void iAScatterPlotWidget::mousePressEvent(QMouseEvent * event)
{
	if (event->x() >= PaddingLeft() && event->x() <= (width() - PaddingRight) &&
		event->y() >= PaddingTop && event->y() <= (height() - PaddingBottom()))
	{
		m_scatterplot->SPLOMMousePressEvent(event);
	}
	if (m_fixPointsEnabled)
	{
		auto curPoint = m_scatterplot->getCurrentPoint();
		if (!event->modifiers().testFlag(Qt::ControlModifier))
		{	// if Ctrl key not pressed, deselect all highlighted points on any click
			for (auto idx : m_scatterPlotHandler->getHighlightedPoints())
			{
				emit pointHighlighted(idx, false);
			}
			m_scatterPlotHandler->clearHighlighted();
		}
		if (curPoint == iAScatterPlot::NoPointIndex)
		{
			return;
		}
		auto wasHighlighted = m_scatterPlotHandler->isHighlighted(curPoint);
		if (event->modifiers().testFlag(Qt::ControlModifier) && wasHighlighted)
		{   // remove just the highlight of current point if Ctrl _is_ pressed
			m_scatterPlotHandler->removeHighlightedPoint(curPoint);
			emit pointHighlighted(curPoint, false);
		}
		else if (!wasHighlighted)
		{   // if current point was not highlighted before, add it
			m_scatterPlotHandler->addHighlightedPoint(curPoint);
			emit pointHighlighted(curPoint, true);
		}
		emit highlightChanged();
		update();
	}
}

void iAScatterPlotWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if (event->x() >= PaddingLeft() && event->x() <= (width() - PaddingRight) &&
		event->y() >= PaddingTop && event->y() <= (height() - PaddingBottom()))
	{
		m_scatterplot->SPLOMMouseReleaseEvent(event);
		update();
	}
}

void iAScatterPlotWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (event->x() >= PaddingLeft() && event->x() <= (width() - PaddingRight) &&
		event->y() >= PaddingTop && event->y() <= (height() - PaddingBottom()))
	{
		m_scatterplot->SPLOMMouseMoveEvent(event);
	}
}

void iAScatterPlotWidget::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_R) //if R is pressed, reset all the applied transformation as offset and scaling
	{
		m_scatterplot->setTransform(1.0, QPointF(0.0f, 0.0f));
	}
}

std::vector<size_t> & iAScatterPlotWidget::selection()
{
	return m_scatterPlotHandler->getSelection();
}

void iAScatterPlotWidget::setSelection(std::vector<size_t> const & selection)
{
	m_scatterPlotHandler->setSelection(selection);
}

void iAScatterPlotWidget::setSelectionColor(QColor const & c)
{
	m_scatterplot->settings.selectionColor = c;
}

void iAScatterPlotWidget::setSelectionMode(iAScatterPlot::SelectionMode mode)
{
	m_scatterplot->settings.selectionMode = mode;
}

void iAScatterPlotWidget::setPointRadius(double pointRadius)
{
	m_scatterplot->setPointRadius(pointRadius);
}

void iAScatterPlotWidget::setFixPointsEnabled(bool enabled)
{
	m_fixPointsEnabled = enabled;
}

void iAScatterPlotWidget::setPointInfo(QSharedPointer<iAScatterPlotPointInfo> pointInfo)
{
	m_pointInfo = pointInfo;
}

std::vector<size_t> const& iAScatterPlotWidget::highlightedPoints() const
{
	return m_scatterPlotHandler->getHighlightedPoints();
}
