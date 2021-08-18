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
#include "iAChartFunctionTransfer.h"

#include "iAChartWithFunctionsWidget.h"
#include "iALog.h"
#include "iAMapper.h"
#include "iAMathUtility.h"

#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkPiecewiseFunction.h>

#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

iAChartTransferFunction::iAChartTransferFunction(iAChartWithFunctionsWidget *chart, QColor color):
	iAChartFunction(chart),
	m_rangeSliderHandles(false),
	m_selectedPoint(-1),
	m_color(color),
	m_tf(nullptr)
{
	m_gradient.setSpread(QGradient::PadSpread);
}

void iAChartTransferFunction::draw(QPainter &painter, QColor color, int lineWidth)
{
	bool active = (m_chart->selectedFunction() == this);

	QPen pen = painter.pen();
	pen.setColor(color); pen.setWidth(lineWidth);
	QPen pointSelPen = painter.pen();
	pointSelPen.setColor(Qt::red); pointSelPen.setWidth(1);

	painter.setPen(pen);
	painter.setBrush(QColor(128, 128, 128, 255));

	double gradientWidth = m_chart->chartWidth()*m_chart->xZoom();

	m_gradient = QLinearGradient();
	m_gradient.setStart(0, 0);
	m_gradient.setFinalStop(gradientWidth, 0);

	// draw opacity and color tf
	if (m_tf->opacityTF()->GetSize() != m_tf->colorTF()->GetSize())
	{
		LOG(lvlWarn, QString("Definition mismatch: opacity TF has %1 positions, color TF %2!")
			.arg(m_tf->opacityTF()->GetSize())
			.arg(m_tf->colorTF()->GetSize()));
		return;
	}
	double opacityTFValue[4];
	double colorTFValue[6];

	m_tf->opacityTF()->GetNodeValue(0, opacityTFValue);
	m_tf->colorTF()->GetNodeValue(0, colorTFValue);

	int x1 = m_chart->xMapper().srcToDst(opacityTFValue[0]);
	int y1 = opacity2PixelY(opacityTFValue[1]);

	QColor c; c.setRgbF(colorTFValue[1], colorTFValue[2], colorTFValue[3], 0.588);
	double gradKey = static_cast<double>(x1) / gradientWidth;
	assert(gradKey >= 0 && gradKey <= 1);
	m_gradient.setColorAt(gradKey, c);

	int lastX = x1;
	for ( int i = 1; i < m_tf->opacityTF()->GetSize(); i++)
	{
		m_tf->opacityTF()->GetNodeValue(i, opacityTFValue);
		m_tf->colorTF()->GetNodeValue(i, colorTFValue);

		int x2 = m_chart->xMapper().srcToDst(opacityTFValue[0]);
		if (x2 == lastX)
		{
			++x2;
		}
		lastX = x2;
		int y2 = opacity2PixelY(opacityTFValue[1]);
		painter.drawLine(x1, y1, x2, y2); // draw line
		if (active)
		{
			if (!m_rangeSliderHandles)
			{
				drawPoint(painter, x1, y1, (i - 1 == m_selectedPoint), c);
			}
			else
			{
				if ( i - 1 == m_selectedPoint &&  i - 1 > 0 )
				{
					painter.setPen(pointSelPen);
					painter.setBrush( QBrush( QColor( 254, 153, 41, 150 ) ) );
					QRectF rectangle( x1 - SELECTED_PIE_RADIUS, y1 - SELECTED_PIE_RADIUS,
										SELECTED_PIE_SIZE, SELECTED_PIE_SIZE );
					painter.drawPie( rectangle, 60 * 16, 60 * 16 );
				}
				else if ( i - 1 > 0 )
				{
					painter.setPen(pointSelPen);
					painter.setBrush( QBrush( QColor( 254, 153, 41, 150 ) ) );
					QRectF rectangle( x1 - PIE_RADIUS, y1 - PIE_RADIUS,
										PIE_SIZE, PIE_SIZE );
					painter.drawPie( rectangle, 60 * 16, 60 * 16 );
				}
			}
		}

		painter.setPen(pen);
		c.setRgbF(colorTFValue[1], colorTFValue[2], colorTFValue[3], 0.588);
		gradKey = static_cast<double>(x2) / gradientWidth;
		assert(gradKey >= 0 && gradKey <= 1);
		m_gradient.setColorAt(gradKey, c);
		x1 = x2;
		y1 = y2;
	}
	// draw last point:
	if (active && !m_rangeSliderHandles)
	{
		drawPoint(painter, x1, y1, m_selectedPoint == m_tf->opacityTF()->GetSize() - 1, c);
	}
}

void iAChartTransferFunction::draw(QPainter &painter)
{
	draw(painter, m_color, 1);
}

void iAChartTransferFunction::drawOnTop(QPainter &painter)
{
	if (m_tf->opacityTF()->GetSize() == m_tf->colorTF()->GetSize())
	{
		double gradientWidth = m_chart->chartWidth()*m_chart->xZoom();

		painter.fillRect( 0, 1, gradientWidth, -(m_chart->bottomMargin()+1), m_gradient );
	}
}

int iAChartTransferFunction::selectPoint(int mouseX, int mouseY)
{
	int index = -1;
	double pointValue[4];
	for (int pointIndex = 0; pointIndex < m_tf->opacityTF()->GetSize(); pointIndex++)
	{
		m_tf->opacityTF()->GetNodeValue(pointIndex, pointValue);
		int pointX = m_chart->data2MouseX(pointValue[0]);
		int pointY = opacity2PixelY(pointValue[1]);
		int pointRadius = iAChartFunction::pointRadius(pointIndex == m_selectedPoint);
		if ( !m_rangeSliderHandles )
		{
			if (std::abs(mouseX - pointX) < pointRadius && std::abs(mouseY - pointY) < pointRadius)
			{
				index = pointIndex;
				break;
			}
		}
		else
		{
			if ( (mouseX >= pointX - SELECTED_PIE_RADIUS && mouseX <= pointX + SELECTED_PIE_RADIUS
				&& mouseY >= pointY - SELECTED_PIE_RADIUS && mouseY <= pointY)
				|| (mouseX >= pointX - PIE_RADIUS && mouseX <= pointX + PIE_RADIUS
				&& mouseY >= pointY - PIE_RADIUS && mouseY <= pointY) )
			{
				// FeatureAnalyzer: range slider widget; only handles can get selected (no end points)
				if (this->isEndPoint(pointIndex))
				{
					continue;
				}

				index = pointIndex;
				break;
			}
		}
	}
	m_selectedPoint = index;
	return index;
}

int iAChartTransferFunction::addPoint(int mouseX, int mouseY)
{
	double dataX = m_chart->mouse2DataX(mouseX);
	double const * tfRange = m_tf->opacityTF()->GetRange();
	if (dataX < tfRange[0] || dataX > tfRange[1])
	{
		LOG(lvlDebug, QString("Will not add point at %1, because it is outside transfer function range (%2, %3)")
			.arg(dataX).arg(tfRange[0]).arg(tfRange[1]));
		m_selectedPoint = -1;
		return -1;
	}
	m_selectedPoint = m_tf->opacityTF()->AddPoint(dataX, pixelY2Opacity(mouseY));
	return m_selectedPoint;
}

void iAChartTransferFunction::addColorPoint(int x, double red, double green, double blue)
{
	if (red < 0 || green < 0 || blue < 0)
	{
		QGradientStops stops = m_gradient.stops();
		double gradientWidth = m_chart->chartWidth() * m_chart->xZoom();
		double pos = x / gradientWidth;

		// find stops before and after pos
		int i = 0;
		QGradientStop stop = stops.at(0);
		while (stop.first < pos)
		{
			++i;
			if (i >= stops.size())
			{
				break;
			}
			stop = stops.at(i);
		}

		double firstPos;
		QColor firstColor;
		int idx = (i == 0) ? 0 : i-1;
		firstPos = stops.at(idx).first;
		firstColor = stops.at(idx).second;

		double secondPos;
		QColor secondColor;
		idx = (i == stops.size()) ? i-1 : i;
		secondPos = stops.at(idx).first;
		secondColor =  stops.at(idx).second;

		double secondWeight = (secondPos == firstPos) ? 1.0 : (pos-firstPos)/(secondPos-firstPos);
		double firstWeight = 1.0-secondWeight;

		red = (firstColor.red()*firstWeight +secondColor.red()*secondWeight)/255.0;
		green = (firstColor.green()*firstWeight +secondColor.green()*secondWeight)/255.0;
		blue = (firstColor.blue()*firstWeight +secondColor.blue()*secondWeight)/255.0;
	}

	m_tf->colorTF()->AddRGBPoint(m_chart->mouse2DataX(x), red, green, blue);
	m_tf->colorTF()->Build();
	triggerOnChange();
}

void iAChartTransferFunction::removePoint(int index)
{
	if (m_tf->opacityTF()->GetSize() < 2)
	{
		return;
	}
	double values[4];
	m_tf->opacityTF()->GetNodeValue(index, values);

	double cvalues[6];
	m_tf->colorTF()->GetNodeValue(index, cvalues);
	m_tf->opacityTF()->RemovePoint(values[0]);
	m_tf->colorTF()->RemovePoint(values[0]);
	m_tf->colorTF()->Build();
	triggerOnChange();
}

void iAChartTransferFunction::moveSelectedPoint(int mouseX, int mouseY)
{
	assert(m_selectedPoint != -1);
	mouseX = clamp(0, m_chart->chartWidth() - 1, mouseX);
	mouseY = clamp(0, m_chart->chartHeight() - 1, mouseY);

	double dataX = m_chart->mouse2DataX(mouseX);
	if (m_selectedPoint != 0 && m_selectedPoint != m_tf->opacityTF()->GetSize()-1)
	{
		double nextOpacityTFValue[4];
		double prevOpacityTFValue[4];

		m_tf->opacityTF()->GetNodeValue(m_selectedPoint+1, nextOpacityTFValue);
		m_tf->opacityTF()->GetNodeValue(m_selectedPoint-1, prevOpacityTFValue);
		int newX = mouseX;
		if (dataX >= nextOpacityTFValue[0])
		{
			newX = m_chart->data2MouseX(nextOpacityTFValue[0]) - 1;
		}
		else if (dataX <= prevOpacityTFValue[0])
		{
			newX = m_chart->data2MouseX(prevOpacityTFValue[0]) + 1;
		}
		setPointOpacity(m_selectedPoint, newX, mouseY);
		double colorTFValue[6];
		m_tf->colorTF()->GetNodeValue(m_selectedPoint, colorTFValue);
		double chartX = m_chart->mouse2DataX(newX);
		setPointColor(m_selectedPoint, chartX, colorTFValue[1], colorTFValue[2], colorTFValue[3]);
	}
	else
	{
		setPointOpacity(m_selectedPoint, mouseY);
	}

	triggerOnChange();
}

void iAChartTransferFunction::changeColor()
{
	if (m_selectedPoint == -1)
	{
		return;
	}
	double colorTFValue[6];
	m_tf->colorTF()->GetNodeValue(m_selectedPoint, colorTFValue);
	QColorDialog colorDlg;
	colorDlg.setCurrentColor(QColor(colorTFValue[1]*255, colorTFValue[2]*255, colorTFValue[3]*255));
	if (colorDlg.exec() != QDialog::Accepted)
	{
		return;
	}

	QColor col = colorDlg.selectedColor();
	setPointColor(m_selectedPoint, colorTFValue[0], col.redF(), col.greenF(), col.blueF());
	m_tf->colorTF()->Modified();
	m_tf->colorTF()->Build();
	triggerOnChange();
}

bool iAChartTransferFunction::isEndPoint(int index) const
{
	return index == 0 || index == m_tf->opacityTF()->GetSize()-1;
}

bool iAChartTransferFunction::isDeletable(int index) const
{
	return !isEndPoint(index);
}

QString iAChartTransferFunction::name() const
{
	return QString("Transfer function (%1 points)").arg(m_tf->opacityTF()->GetSize());
}

void iAChartTransferFunction::reset()
{
	if (m_tf)
	{
		m_tf->resetFunctions();
		triggerOnChange();
	}
}

void iAChartTransferFunction::mouseReleaseEventAfterNewPoint(QMouseEvent *)
{
	double colorTFValue[6];
	m_tf->colorTF()->GetNodeValue(m_selectedPoint, colorTFValue);
	QColorDialog colorDlg;
	colorDlg.setCurrentColor(QColor(colorTFValue[1]*255, colorTFValue[2]*255, colorTFValue[3]*255));
	if (colorDlg.exec() == QDialog::Accepted)
	{
		QColor col = colorDlg.selectedColor();
		setPointColor(m_selectedPoint, colorTFValue[0], (double)col.red()/255.0, (double)col.green()/255.0, (double)col.blue()/255.0 );
	}
	else if (m_selectedPoint > 0 && m_selectedPoint < m_tf->opacityTF()->GetSize()-1)
	{
		removePoint(m_selectedPoint);
	}
}

void iAChartTransferFunction::setPointColor(int selectedPoint, double x, double red, double green, double blue)
{
	double colorVal[6] = { x, red, green, blue, 0.5, 0.0 };
	m_tf->colorTF()->SetNodeValue(selectedPoint, colorVal);
	m_tf->colorTF()->Modified();
	m_tf->colorTF()->Build();
	triggerOnChange();
}

void iAChartTransferFunction::setPointOpacity(int selectedPoint, int pixelX, int pixelY)
{
	double opacityVal[4] = { m_chart->mouse2DataX(pixelX), pixelY2Opacity(pixelY), 0.0, 0.0 };
	m_tf->opacityTF()->SetNodeValue(selectedPoint, opacityVal);
}

void iAChartTransferFunction::setPointOpacity(int selectedPoint, int pixelY)
{
	double opacityTFValues[4];
	m_tf->opacityTF()->GetNodeValue(selectedPoint, opacityTFValues);

	opacityTFValues[1] = pixelY2Opacity(pixelY);
	m_tf->opacityTF()->SetNodeValue(selectedPoint, opacityTFValues);
}

double iAChartTransferFunction::pixelY2Opacity(int pixelY)
{
	return mapToNorm(0, m_chart->chartHeight(), clamp(0, m_chart->chartHeight(), pixelY));
}

int iAChartTransferFunction::opacity2PixelY(double opacity)
{
	return mapNormTo(0, std::max(0, m_chart->chartHeight()), opacity);
}

void iAChartTransferFunction::triggerOnChange()
{
	emit changed();
}

size_t iAChartTransferFunction::numPoints() const
{
	return m_tf->opacityTF()->GetSize();
}

iATransferFunction* iAChartTransferFunction::tf()
{
	return m_tf;
}

void iAChartTransferFunction::setTF(iATransferFunction* tf)
{
	m_tf = tf;
}

void iAChartTransferFunction::enableRangeSliderHandles( bool rangeSliderHandles )
{
	m_rangeSliderHandles = rangeSliderHandles;
}
