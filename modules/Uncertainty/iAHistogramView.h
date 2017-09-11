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
#pragma once

#include "iAAbstractDiagramData.h"
#include "iAUncertaintyImages.h" // for IntImage
#include "iADiagramFctWidget.h"
#include "iAFunctionDrawers.h"

#include <itkMinimumMaximumImageCalculator.h>
#include <itkImageRegionConstIterator.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <QWidget>

class iAAbstractDiagramRangedData;

class iASimpleHistogramData : public iAAbstractDiagramRangedData
{
public:
	static QSharedPointer<iASimpleHistogramData> Create(DataType minX, DataType maxX, size_t numBin, iAValueType valueType);

	// Inherited via iAAbstractDiagramRangedData
	virtual DataType const * GetData() const override;
	virtual size_t GetNumBin() const override;
	virtual double GetSpacing() const override;
	virtual double const * GetDataRange() const override;
	virtual DataType GetMaxValue() const override;
	virtual DataType GetMinValue() const override;
	virtual iAValueType GetRangeType() const override;

	//void AddValue(DataType value);
	void SetBin(size_t binIdx, DataType value);
private:

	iASimpleHistogramData(DataType minX, DataType maxX, size_t numBin, iAValueType xValueType);
	double* m_data;
	double m_rangeX[2];
	double m_rangeY[2];
	size_t m_numBin;
	iAValueType m_xValueType;
};


template <typename PixelT>
QSharedPointer<iASimpleHistogramData> CreateHistogram(QVector<typename itk::Image<PixelT, 3>::Pointer> imgs, size_t numBin, PixelT min, PixelT max, iAValueType xValueType)
{
	/*
	img->ReleaseDataFlagOff();
	itk::MinimumMaximumImageCalculator<TImage> minMaxCalc;
	minMaxCalc->SetInput(img);
	minMaxCalc->Compute();
	auto result = iASimpleHistogramData::Create(minMaxCalc->GetMinimum(), minMaxCalc->GetMaximum(), numBin);
	*/
	auto result = iASimpleHistogramData::Create(min, max, numBin, xValueType);
	for (int i = 0; i < imgs.size(); ++i)
	{
		double sum = 0;
		itk::ImageRegionConstIterator<itk::Image<PixelT, 3> > it(imgs[i], imgs[i]->GetLargestPossibleRegion());
		it.GoToBegin();
		while (!it.IsAtEnd())
		{
			sum += it.Get();
			++it;
		}
		result->SetBin(i, sum);
	}
	return result;
}


template <typename PixelT>
QSharedPointer<iASimpleHistogramData> CreateHistogram(QVector<typename itk::Image<PixelT, 3>::Pointer> imgs, size_t numBin, int index[3], PixelT min, PixelT max, iAValueType xValueType)
{
	auto result = iASimpleHistogramData::Create(min, max, numBin, xValueType);
	itk::Index<3> idx;
	for (int i = 0; i < 3; ++i) { idx[i] = index[i]; }
	for (int m=0; m<imgs.size(); ++m)
	{
		result->SetBin(m, imgs[m]->GetPixel(idx));
	}
	return result;
}


class iAHistogramChartWidget : public iADiagramFctWidget
{
public:
	iAHistogramChartWidget(QSharedPointer<iASimpleHistogramData> data, QString const & caption):
		iADiagramFctWidget(nullptr, nullptr, vtkSmartPointer<vtkPiecewiseFunction>(), vtkSmartPointer<vtkColorTransferFunction>(), caption),
		m_data(data) {
	}
	virtual QSharedPointer<iAAbstractDiagramRangedData> GetData()
	{
		return m_data;
	}
	virtual QSharedPointer<iAAbstractDiagramRangedData> const GetData() const
	{
		return m_data;
	}
	QSharedPointer<iAAbstractDrawableFunction> CreatePrimaryDrawer()
	{
		return QSharedPointer<iAAbstractDrawableFunction>(new iABarGraphDrawer(m_data, QColor(0, 0, 255), 2));
	}
private:
	QSharedPointer<iASimpleHistogramData> m_data;
};


class iAHistogramView : public QWidget
{
	Q_OBJECT
public:
	iAHistogramView();
	void AddChart(QString const & caption, QSharedPointer<iASimpleHistogramData> data);
private:
	iADiagramFctWidget* m_chart;
};
