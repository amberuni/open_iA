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
#include "iACompHistogramTableData.h"

//CompVis
#include "iACsvDataStorage.h"

//vtk
#include <vtkPolyData.h>

//C++
#include <vector>

iACompHistogramTableData::iACompHistogramTableData() :
	m_maxVal(-1),
	m_minVal(-1),
	binData(new QList<bin::BinType*>()),
	zoomedBinData(bin::initialize(1)),
	binDataObjects(new QList<std::vector<csvDataType::ArrayType*>*>()),
	amountObjectsEveryDataset(new std::vector<int>),
	m_maxAmountInAllBins(0),
	m_binsBoundaries(nullptr),
	m_binPolyDatasets(new QList<vtkSmartPointer<vtkPolyData>>())
{
}

/************************** Setter & Getter ***************************************/

QList<bin::BinType*>* iACompHistogramTableData::getBinData()
{
	return binData;
}

double iACompHistogramTableData::getMaxVal()
{
	return m_maxVal;
}

double iACompHistogramTableData::getMinVal()
{
	return m_minVal;
}

QList<std::vector<csvDataType::ArrayType*>*>* iACompHistogramTableData::getObjectsPerBin()
{
	return binDataObjects;
}

std::vector<int>* iACompHistogramTableData::getAmountObjectsEveryDataset()
{
	return amountObjectsEveryDataset;
}

bin::BinType* iACompHistogramTableData::getZoomedBinData()
{
	return zoomedBinData;
}

void iACompHistogramTableData::setMaxVal(double newMax)
{
	m_maxVal = newMax;
}

void iACompHistogramTableData::setMinVal(double newMin)
{
	m_minVal = newMin;
}

void iACompHistogramTableData::setBinData(QList<bin::BinType*>* newBinData)
{
	binData = newBinData;
}

void iACompHistogramTableData::setBinDataObjects(QList<std::vector<csvDataType::ArrayType*>*>* newBinDataObjects)
{
	binDataObjects = newBinDataObjects;
}

void iACompHistogramTableData::setAmountObjectsEveryDataset(std::vector<int>* newAmountObjectsEveryDataset)
{
	amountObjectsEveryDataset = newAmountObjectsEveryDataset;
}

void iACompHistogramTableData::setZoomedBinData(bin::BinType* newZoomedBinData)
{
	zoomedBinData = newZoomedBinData;
}

int iACompHistogramTableData::getMaxAmountInAllBins()
{
	return m_maxAmountInAllBins;
}

void iACompHistogramTableData::setMaxAmountInAllBins(int newMaxAmountInAllBins)
{
	m_maxAmountInAllBins = newMaxAmountInAllBins;
}

QList<std::vector<double>>* iACompHistogramTableData::getNumberOfObjectsPerBinAllDatasets()
{
	QList<std::vector<double>>* result = new QList<std::vector<double>>();

	for (int dataId = 0; dataId < binData->size(); dataId++)
	{  //datasets
		bin::BinType* currDataset = binData->at(dataId);

		std::vector<double> bins = std::vector<double>(currDataset->size(), 0);
		for (int binId = 0; binId < currDataset->size(); binId++)
		{  //bins
			std::vector<double> currBin = currDataset->at(binId);
			bins.at(binId) = currBin.size();
		}

		result->push_back(bins);
	}

	return result;
}

/************************** rendering information storage methods ***************************************/
void iACompHistogramTableData::resetBinPolyData()
{
	m_binPolyDatasets->clear();
}

void iACompHistogramTableData::storeBinPolyData(vtkSmartPointer<vtkPolyData> newBinPolyData)
{
		m_binPolyDatasets->append(newBinPolyData);
}

QList<vtkSmartPointer<vtkPolyData>>* iACompHistogramTableData::getBinPolyData()
{
	return m_binPolyDatasets;
}

/************************** debug methods ***************************************/
void iACompHistogramTableData::debugBinDataObjects()
{
	LOG(lvlDebug, "");

	int NrOfFibers = 0;
	//DEBUG
	for (int i = 0; i < binDataObjects->size(); i++)
	{  //datasets
		LOG(lvlDebug, "Dataset " + QString::number(i));
		for (int k = 0; k < binDataObjects->at(i)->size(); k++)
		{  //bins

			csvDataType::ArrayType* data = binDataObjects->at(i)->at(k);

			for (int j = 0; j < data->size(); j++)
			{
				//LOG(lvlDebug,
				//	"fiberLabelId = " + QString::number(data->at(j).at(0)) + " --> at Bin: " + QString::number(k));
				NrOfFibers++;
			}
			LOG(lvlDebug, "number of fibers = " + QString::number(NrOfFibers));
			NrOfFibers = 0;
			LOG(lvlDebug, "");
		}
	}
	LOG(lvlDebug, "");
	LOG(lvlDebug, "#######################################################");
}

/************************** bin methods ***************************************/

bin::BinType* bin::initialize(int amountBins)
{
	return new bin::BinType(amountBins);
}

void bin::debugBinType(BinType* input)
{
	int amountCols = static_cast<int>(input->size());

	//DEBUG
	LOG(lvlDebug," ");
	LOG(lvlDebug,"Bins: " + QString::number(amountCols));
	LOG(lvlDebug,"Bin Matrix: ");
	for (int col1 = 0; col1 < amountCols; col1++)
	{
		LOG(lvlDebug,"Bin " + QString::number(col1) + ":");
		for (int r1 = 0; r1 < ((int)input->at(col1).size()); r1++)
		{
			LOG(lvlDebug,"  Values " + QString::number(r1) + ": " + QString::number(input->at(col1).at(r1)));
		}
	}
	LOG(lvlDebug," ");
}

QList<bin::BinType*>* bin::DeepCopy(QList<bin::BinType*>* input)
{
	QList<bin::BinType*>* output = new QList<bin::BinType*>();

	for (int binInd = 0; binInd < input->size(); binInd++) 
	{
		bin::BinType* curBin = input->at(binInd);
		bin::BinType* newBin = initialize(static_cast<int>(curBin->size()));

		for (int indVals = 0; indVals < ((int)curBin->size()); indVals++)
		{
			newBin->at(indVals) = (curBin->at(indVals));
		}

		output->append(newBin);
	}

	return output;
}

bin::BinType* bin::copyCells(bin::BinType* input, std::vector<vtkIdType>* indexOfCellsToCopy)
{
	bin::BinType* output = new bin::BinType();

	for(int i = 0; i < ((int)indexOfCellsToCopy->size()); i++)
	{
		output->push_back( input->at(indexOfCellsToCopy->at(i)) );
	}

	return output;
}


std::vector<double>* bin::getMinimumAndMaximum(bin::BinType* input)
{
	std::vector<double>* result = new std::vector<double>();

	double min = INFINITY;
	double max = -INFINITY;

	for (int i = 0; i < input->size(); i++)
	{
		std::vector<double> bin = input->at(i);

		for (int k = 0; k < bin.size(); k++)
		{
			double val = bin.at(k);
			if (min > val)
			{
				min = val;
			}

			if (max < val)
			{
				max = val;
			}
		}
	}

	result->push_back(min);
	result->push_back(max);

	return result;
}