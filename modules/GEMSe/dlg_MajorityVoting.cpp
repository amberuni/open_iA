/*********************************  open_iA 2016 06  ******************************** *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, J. Weissenb�ck, *
*                     Artem & Alexander Amirkhanov, B. Fr�hler                        *
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
* Contact: FH O� Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#include "pch.h"
#include "dlg_MajorityVoting.h"

#include "dlg_GEMSe.h"
#include "iAConsole.h"
#include "iADockWidgetWrapper.h"
#include "iAImageTreeNode.h"
#include "iALookupTable.h"
#include "iAQSplom.h"
#include "iASingleResult.h"
#include "mdichild.h"

#include "ParametrizableLabelVotingImageFilter.h"
#include "FilteringLabelOverlapMeasuresImageFilter.h"

#include <itkLabelStatisticsImageFilter.h>

#include <QCheckBox>
#include <QMessageBox>
#include <QTableWidget>

dlg_MajorityVoting::dlg_MajorityVoting(MdiChild* mdiChild, dlg_GEMSe* dlgGEMSe, int labelCount) :
	m_mdiChild(mdiChild),
	m_dlgGEMSe(dlgGEMSe),
	m_labelCount(labelCount),
	m_splom(new iAQSplom()),
{
	m_lut->setRange(0, 1);
	m_lut->allocate(2);
	m_lut->setColor(0, QColor(0, 0, 0));
	m_lut->setColor(1, QColor(255, 0, 0));
	m_splom->setLookupTable(*m_lut, QString("test"));
	iADockWidgetWrapper* w(new iADockWidgetWrapper(m_splom, "Charts", "Charts"));
	mdiChild->splitDockWidget(this, w, Qt::Horizontal);

	connect(pbSample, SIGNAL(clicked()), this, SLOT(Sample()));
	connect(pbMinAbsPercent_Plot, SIGNAL(clicked()), this, SLOT(MinAbsPlot()));
	connect(pbMinDiffPercent_Plot, SIGNAL(clicked()), this, SLOT(MinDiffPlot()));
	connect(pbMinRatio_Plot, SIGNAL(clicked()), this, SLOT(RatioPlot()));
	connect(pbMaxPixelEntropy_Plot, SIGNAL(clicked()), this, SLOT(MaxPixelEntropyPlot()));
	connect(slAbsMinPercent, SIGNAL(valueChanged(int)), this, SLOT(AbsMinPercentSlider(int)));
	connect(slMinDiffPercent, SIGNAL(valueChanged(int)), this, SLOT(MinDiffPercentSlider(int)));
	connect(slMinRatio, SIGNAL(valueChanged(int)), this, SLOT(MinRatioSlider(int)));
	connect(slMaxPixelEntropy, SIGNAL(valueChanged(int)), this, SLOT(MaxPixelEntropySlider(int)));
}

void dlg_MajorityVoting::SetGroundTruthImage(LabelImagePointer groundTruthImage)
{
	m_groundTruthImage = groundTruthImage;
}

iAITKIO::ImagePointer GetMajorityVotingImage(QVector<QSharedPointer<iASingleResult> > selection,
	double minAbsPercentage, double minDiffPercentage, double minRatio, double maxPixelEntropy,
	int labelCount)
{
	typedef ParametrizableLabelVotingImageFilter<LabelImageType> LabelVotingType;
	LabelVotingType::Pointer labelVotingFilter;
	labelVotingFilter = LabelVotingType::New();
	labelVotingFilter->SetAbsoluteMinimumPercentage(minAbsPercentage);
	labelVotingFilter->SetMinimumDifferencePercentage(minDiffPercentage);
	labelVotingFilter->SetMinimumRatio(minRatio);
	labelVotingFilter->SetMaxPixelEntropy(maxPixelEntropy);

	for (unsigned int i = 0; i < static_cast<unsigned int>(selection.size()); ++i)
	{
		LabelImageType* lblImg = dynamic_cast<LabelImageType*>(selection[i]->GetLabelledImage().GetPointer());
		labelVotingFilter->SetInput(i, lblImg);
		if (maxPixelEntropy >= 0)
		{
			typedef LabelVotingType::DoubleImg::Pointer DblImgPtr;
			std::vector<DblImgPtr> probImgs;
			for (int l = 0; l < labelCount; ++l)
			{
				iAITKIO::ImagePointer p = selection[i]->GetProbabilityImg(l);
				DblImgPtr dp = dynamic_cast<typename LabelVotingType::DoubleImg*>(p.GetPointer());
				probImgs.push_back(dp);
			}
			labelVotingFilter->SetProbabilityImages(i, probImgs);
		}
	}

	labelVotingFilter->Update();
	LabelImagePointer labelResult = labelVotingFilter->GetOutput();
	// according to https://stackoverflow.com/questions/27016173/pointer-casts-for-itksmartpointer,
	// the following does not leave a "dangling" smart pointer:
	iAITKIO::ImagePointer result = dynamic_cast<iAITKIO::ImageBaseType *>(labelResult.GetPointer());
	return result;
}


void dlg_MajorityVoting::AbsMinPercentSlider(int)
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minAbs = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
	lbMinAbsPercent->setText(QString::number(minAbs * 100, 'f', 2) + " %");
	iAITKIO::ImagePointer result = GetMajorityVotingImage(m_selection, minAbs, -1, -1, -1, m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(result);
}

void dlg_MajorityVoting::MinDiffPercentSlider(int)
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minDiff = static_cast<double>(slMinDiffPercent->value()) / slMinDiffPercent->maximum();
	lbMinDiffPercent->setText(QString::number(minDiff * 100, 'f', 2) + " %");
	iAITKIO::ImagePointer result = GetMajorityVotingImage(m_selection, -1, minDiff, -1, -1, m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(result);
}

void dlg_MajorityVoting::MinRatioSlider(int)
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minRatio = static_cast<double>(slMinRatio->value()) / 100;
	lbMinRatio->setText(QString::number(minRatio, 'f', 2));
	iAITKIO::ImagePointer result = GetMajorityVotingImage(m_selection, -1, -1, minRatio, -1, m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(result);
}

void dlg_MajorityVoting::MaxPixelEntropySlider(int)
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double maxPixelEntropy = static_cast<double>(slMaxPixelEntropy->value()) / slMaxPixelEntropy->maximum();
	lbMaxPixelEntropy->setText(QString::number(maxPixelEntropy*100, 'f', 2)+" %");
	iAITKIO::ImagePointer result = GetMajorityVotingImage(m_selection, -1, -1, -1, maxPixelEntropy, m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(result);
}

iAITKIO::ImagePointer GetMajorityVotingNumbers(QVector<QSharedPointer<iASingleResult> > selection,
	double minAbsPercentage, double minDiffPercentage, double minRatio, double maxPixelEntropy, int mode)
{
	typedef ParametrizableLabelVotingImageFilter<LabelImageType> LabelVotingType;
	LabelVotingType::Pointer labelVotingFilter;
	labelVotingFilter = LabelVotingType::New();
	labelVotingFilter->SetAbsoluteMinimumPercentage(minAbsPercentage);
	labelVotingFilter->SetMinimumDifferencePercentage(minDiffPercentage);
	labelVotingFilter->SetMinimumRatio(minRatio);
	labelVotingFilter->SetMaxPixelEntropy(maxPixelEntropy);

	for (unsigned int i = 0; i < static_cast<unsigned int>(selection.size()); ++i)
	{
		LabelImageType* lblImg = dynamic_cast<LabelImageType*>(selection[i]->GetLabelledImage().GetPointer());
		labelVotingFilter->SetInput(i, lblImg);
	}
	labelVotingFilter->Update();
	typename LabelVotingType::DoubleImg::Pointer labelResult = labelVotingFilter->GetNumbers(mode);
	// according to https://stackoverflow.com/questions/27016173/pointer-casts-for-itksmartpointer,
	// the following does not leave a "dangling" smart pointer:
	iAITKIO::ImagePointer result = dynamic_cast<iAITKIO::ImageBaseType *>(labelResult.GetPointer());
	return result;
}

void dlg_MajorityVoting::MinAbsPlot()
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minAbs = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
	iAITKIO::ImagePointer result = GetMajorityVotingNumbers(m_selection, minAbs, -1, -1, -1, AbsolutePercentage);
	m_dlgGEMSe->AddMajorityVotingNumbers(result);
}

void dlg_MajorityVoting::MinDiffPlot()
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minDiff = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
	iAITKIO::ImagePointer result = GetMajorityVotingNumbers(m_selection, -1, minDiff, -1, -1, DiffPercentage);
	m_dlgGEMSe->AddMajorityVotingNumbers(result);
}

void dlg_MajorityVoting::RatioPlot()
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minRatio = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
	iAITKIO::ImagePointer result = GetMajorityVotingNumbers(m_selection, -1, -1, minRatio, -1, Ratio);
	m_dlgGEMSe->AddMajorityVotingNumbers(result);
}


void dlg_MajorityVoting::MaxPixelEntropyPlot()
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double maxPixelEntropy = static_cast<double>(slMaxPixelEntropy->value()) / slMaxPixelEntropy->maximum();
	iAITKIO::ImagePointer result = GetMajorityVotingNumbers(m_selection, -1, -1, -1, maxPixelEntropy, PixelEntropy);
	m_dlgGEMSe->AddMajorityVotingNumbers(result);
}

/*
void dlg_MajorityVoting::MinAbsPercentStore()
{

static int majorityVotingID = 0;
QVector<QSharedPointer<iASingleResult> > m_selection;
m_dlgGEMSe->GetSelection(m_selection);
double percentage = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
iAITKIO::ImagePointer result = GetMajorityVotingImage(m_selection, percentage, -1, -1, -1, -1);
// Put output somewhere!
// Options / Design considerations:
//  * integrate into clustering
//      + image is then part of rest of analysis
//      ~ follow-up decision required:
//          - consider MJ separate algorithm?
//          - how to preserve creation "parameters"?
//		- have to re-run whole clustering? or integrate it somehow faked?
//	* intermediate step: add as separate result (e.g. in favorite view)
//      // (chance to include in clustering after renewed clustering)
//  * separate list of  majority-voted results
//		- separate from
//  * detail view
//      + prominent display
//      + close to current analysis
//      - lots to rewrite, as it expects node with linked values
//      - volatile - will be gone after next cluster / example image selection
//  * new dock widget in same mdichild
//		+ closer to current analysis than separate mdi child
//		- lots of new implementation required
//		- no clear benefit - if each
//  * new window (mdichild?)
//      - detached from current design
//      + completely independent of other implementation (should continue working if anything else changes)
//      - completely independent of other implementation (not integrated into current analysis)
//      +/- theoretically easier to do/practically probably also not little work to make it happen

// store image to disk
QString mvOutFolder = m_outputFolder + "/majorityVoting";
iAITKIO::ScalarPixelType pixelType = itk::ImageIOBase::INT;
QString mvResultFolder = mvOutFolder + "/sample" + QString::number(majorityVotingID);
QDir qdir;
if (!qdir.mkpath(mvResultFolder))
{
DEBUG_LOG(QString("Can't create output directory %1!").arg(mvResultFolder));
return;
}
iAITKIO::writeFile(mvResultFolder+"/label.mhd", result, pixelType);
m_dlgGEMSe->AddMajorityVotingImage(mvOutFolder, majorityVotingID, percentage);
majorityVotingID++;
//m_dlgSamplings->Add(majoritySamplingResults);
}
*/

void dlg_MajorityVoting::Sample()
{
	if (!m_groundTruthImage)
	{
		QMessageBox::warning(this, "GEMSe", "Please load a reference image first!");
		return;
	}

	const int SampleCount = 100;
	const int ResultCount = 4;
	const int UndecidedLabel = m_labelCount;

	QTableWidget * tables[ResultCount];
	QString titles[ResultCount] =
	{
		QString("Minimum Absolute Percentage"),
		QString("Minimum Percentage Difference"),
		QString("Ratio"),
		QString("Maximum Pixel Uncertainty")
	};
	for (int r = 0; r < ResultCount; ++r)
	{
		tables[r] = new QTableWidget();
		tables[r]->setColumnCount(3);		// "value", Dice, Undecided Pixels
		tables[r]->setRowCount(SampleCount);
		tables[r]->setItem(0, 0, new QTableWidgetItem(titles[r]));
		tables[r]->setItem(0, 1, new QTableWidgetItem("Dice"));
		tables[r]->setItem(0, 2, new QTableWidgetItem("Undecided"));
	}

	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);

	double absPercMin = 1.0 / m_labelCount;
	double absPercMax = 1;

	double ratioMin = 1;
	double ratioMax = m_selection.size();
	DEBUG_LOG(QString("Majority Voting evaluation for a selection of %1 images").arg(m_selection.size()));

	typedef fhw::FilteringLabelOverlapMeasuresImageFilter<LabelImageType> DiceFilter;
	typedef itk::LabelStatisticsImageFilter<LabelImageType, LabelImageType> StatFilter;

	auto region = m_groundTruthImage->GetLargestPossibleRegion();
	auto size = region.GetSize();
	double pixelCount = size[0] * size[1] * size[2];

	for (int i = 0; i < SampleCount; ++i)
	{
		// calculate current value:
		double norm = mapToNorm(0, SampleCount, i);

		double value[4] = {
			mapNormTo(absPercMin, absPercMax, norm),		// minimum absolute percentage
			norm,											// minimum relative percentage
			mapNormTo(ratioMin, ratioMax, norm),			// ratio
			norm											// maximum pixel uncertainty
		};

		// calculate majority voting using these values:
		iAITKIO::ImagePointer result[ResultCount];

		result[0] = GetMajorityVotingImage(m_selection, value[0], -1, -1, -1, m_labelCount);
		result[1] = GetMajorityVotingImage(m_selection, -1, value[1], -1, -1, m_labelCount);
		result[2] = GetMajorityVotingImage(m_selection, -1, -1, value[2], -1, m_labelCount);
		result[3] = GetMajorityVotingImage(m_selection, -1, -1, -1, value[3], m_labelCount);

		//QString out(QString("absPerc=%1, relPerc=%2, ratio=%3, pixelUnc=%4\t").arg(absPerc).arg(relPerc).arg(ratio).arg(pixelUnc));
		// calculate dice coefficient and percentage of undetermined pixels
		// (percentage of voxels with label = difference marker = max. label + 1)
		for (int r = 0; r < ResultCount; ++r)
		{
			LabelImageType* labelImg = dynamic_cast<LabelImageType*>(result[r].GetPointer());

			auto diceFilter = DiceFilter::New();
			diceFilter->SetSourceImage(m_groundTruthImage);
			diceFilter->SetTargetImage(labelImg);
			diceFilter->SetIgnoredLabel(UndecidedLabel);
			diceFilter->Update();
			auto statFilter = StatFilter::New();
			statFilter->SetInput(labelImg);
			statFilter->SetLabelInput(labelImg);
			statFilter->Update();

			double meanDice = diceFilter->GetMeanOverlap();

			double undefinedPerc =
				statFilter->HasLabel(UndecidedLabel)
				? static_cast<double>(statFilter->GetCount(UndecidedLabel)) / pixelCount
				: 0;
			//out += QString("%1 %2\t").arg(meanDice).arg(undefinedPerc);

			// add values to table
			tables[r]->setItem(i, 0, new QTableWidgetItem(value[r]));
			tables[r]->setItem(i, 1, new QTableWidgetItem(meanDice));
			tables[r]->setItem(i, 2, new QTableWidgetItem(undefinedPerc));
		}
		//DEBUG_LOG(QString::number(i) + ": " + out);
	}

	QString ids;
	for (int s = 0; s < m_selection.size(); ++s)
	{
		ids += QString::number(m_selection[s]->GetDatasetID()) + "-" + QString::number(m_selection[s]->GetID());
		if (s < m_selection.size() - 1)
		{
			ids += ", ";
		}
	}

	m_splom->setData(tables[3]);
	int startIdx = twSampleResults->rowCount();
	twSampleResults->setRowCount(startIdx + ResultCount);
	for (int i = 0; i < ResultCount; ++i)
	{
		QCheckBox * checkBox = new QCheckBox;
		if (i == 3) checkBox->setChecked(true);
		twSampleResults->setCellWidget(startIdx + i, 0, checkBox);
		connect(checkBox, SIGNAL(stateChanged(int state)), this, SLOT(CheckBoxStateChanged(int state)));
		twSampleResults->setItem(startIdx + i, 1, new QTableWidgetItem("Maj. Vote Sampling/" + titles[i] + "/" + ids));
		m_checkBoxResultIDMap.insert(checkBox, startIdx + i);
		if (m_results.size() != startIdx + i)
		{
			DEBUG_LOG("Results vector and table are out of sync!");
		}
		m_results.push_back(tables[i]);
	}
	/*
	// plot graph for all tables
	for (int r = 0; r < ResultCount; ++r)
	{
		auto chart = vtkSmartPointer<vtkChartXY>::New();

		auto xAxis = chart->GetAxis(vtkAxis::BOTTOM);
		auto yAxis = chart->GetAxis(vtkAxis::LEFT);
		xAxis->SetTitle("Accuracy (Dice)");
		xAxis->SetLogScale(false);
		yAxis->SetTitle("Undecided Pixels");
		yAxis->SetLogScale(false);

		vtkPlot* plot = chart->AddPlot(vtkChart::POINTS);
		plot->SetColor(
			static_cast<unsigned char>(DefaultColors::AllDataChartColor.red()),
			static_cast<unsigned char>(DefaultColors::AllDataChartColor.green()),
			static_cast<unsigned char>(DefaultColors::AllDataChartColor.blue()),
			static_cast<unsigned char>(DefaultColors::AllDataChartColor.alpha())
		);
		plot->SetWidth(1.0);
		plot->SetInputData(tables[r], 0, 1);

		auto vtkWidget = new QVTKWidget2();
		auto contextView = vtkSmartPointer<vtkContextView>::New();
		contextView->SetRenderWindow(vtkWidget->GetRenderWindow());
		chart->SetSelectionMode(vtkContextScene::SELECTION_NONE);
		contextView->GetScene()->AddItem(chart);
		iADockWidgetWrapper * w(new iADockWidgetWrapper(vtkWidget, titles[r], titles[r].replace(" ", "")));
		MdiChild* mdiChild = dynamic_cast<MdiChild*>(parent());
		mdiChild->splitDockWidget(this, w, Qt::Vertical);
	}
	*/
}


void dlg_MajorityVoting::CheckBoxStateChanged(int state)
{
	QCheckBox* sender = dynamic_cast<QCheckBox*>(QObject::sender());
	int id = m_checkBoxResultIDMap[sender];
	DEBUG_LOG(QString("STATE CHANGED for sampe result %1!").arg(id));
	m_splom->setData(m_results[id]);
}