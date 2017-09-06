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

#include "iAUncertaintyImages.h"

#include <itkImage.h>

#include <QSharedPointer>
#include <QString>
#include <QVector>

class iAEnsembleDescriptorFile;
class iASamplingResults;

typedef itk::Image<double, 3> DoubleImage;
typedef itk::Image<int, 3> IntImage;

class iAEnsemble: public iAUncertaintyImages
{
public:
	//! create from string
	static QSharedPointer<iAEnsemble> create();
	bool load(QString const & ensembleFileName, iAEnsembleDescriptorFile const & ensembleFile);
	virtual vtkImagePointer GetEntropy(int source);
	virtual QString GetSourceName(int source);
private:
	bool loadSampling(QString const & fileName, int labelCount, int id);
	void createUncertaintyImages(int labelCount, QString const & cachePath);
	//! constructor; use static Create methods instead!
	iAEnsemble();
	QVector<QSharedPointer<iASamplingResults> > m_samplings;

	QVector<vtkImagePointer> m_entropy;

	QVector<IntImage::Pointer> m_labelDistr;
	DoubleImage::Pointer m_entropyAvgEntropy;
	DoubleImage::Pointer m_labelDistrEntropy;
	DoubleImage::Pointer m_probSumEntropy;
	DoubleImage::Pointer m_neighbourhoodAvgEntropy3x3;
	DoubleImage::Pointer m_neighbourhoodAvgEntropy5x5;
	QVector<DoubleImage::Pointer> m_probDistr;
};
