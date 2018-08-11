/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2018  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
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
 
#include "dlg_TripleHistogramTF.h"

#include <qsplitter.h>

// TODO: why tf do I need this?
#include "iAModalityList.h"

// Debug
#include "qdebug.h"
#include "ColorInterpolator.h"

dlg_TripleHistogramTF::dlg_TripleHistogramTF(MdiChild * mdiChild /*= 0*/, Qt::WindowFlags f /*= 0 */) :
	//TripleHistogramTFConnector(mdiChild, f), m_mdiChild(mdiChild)
	QDockWidget("Triple Histogram Transfer Function", mdiChild, f),
	m_mdiChild(mdiChild)
{

	//-----------------------------------------------
	// Test vvv // TODO: remove comments
	resize(779, 501);
	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar);

	//QWidget *dockWidgetContents = new QWidget();
	QSplitter *dockWidgetContents = new QSplitter(Qt::Horizontal);
	dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));

	//RightBorderLayout *mainLayout = new RightBorderLayout(dockWidgetContents, RightBorderLayout::Right);
	//m_mainLayout = new QHBoxLayout(dockWidgetContents);
	//m_mainLayout->setSpacing(0);
	//m_mainLayout->setObjectName(QStringLiteral("horizontalLayout_2"));
	//m_mainLayout->setContentsMargins(0, 0, 0, 0);

	QWidget *optionsContainer = new QWidget();
	optionsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	// Test ^^^
	//-----------------------------------------------

	m_triangleWidget = new iABarycentricTriangleWidget(dockWidgetContents);

	m_slicerModeComboBox = new QComboBox(optionsContainer);
	m_slicerModeComboBox->addItem("YZ", QVariant(iASlicerMode::YZ));
	m_slicerModeComboBox->addItem("XY", QVariant(iASlicerMode::XY));
	m_slicerModeComboBox->addItem("XZ", QVariant(iASlicerMode::XZ));

	m_sliceSlider = new QSlider(Qt::Horizontal, optionsContainer);
	m_sliceSlider->setMinimum(0);

	QHBoxLayout *optionsContainerLayout = new QHBoxLayout(optionsContainer);
	optionsContainerLayout->addWidget(m_slicerModeComboBox);
	optionsContainerLayout->addWidget(m_sliceSlider);

	QWidget *histogramStackContainer = new QWidget();
	QVBoxLayout *histogramStackContainerLayout = new QVBoxLayout(histogramStackContainer);

	if (m_mdiChild->GetModalities()->size() > 0) // What if it isn't bigger than 0? Then what?
	{
		iAModalityWidget *modalities[3];
		QString names[3];
		if (m_mdiChild->GetModalities()->size() == 1) // TODO: remove
		{
			for (int i = 0; i < 3/*mdiChild->GetModalities()->size()*/; ++i)
			{
				modalities[i] = new iAModalityWidget(histogramStackContainer, mdiChild->GetModality(0), mdiChild);
				histogramStackContainerLayout->addWidget(modalities[i]);
				names[i] = mdiChild->GetModality(0)->GetName();
			}
		}
		else if (m_mdiChild->GetModalities()->size() >= 3) // TODO: handle more than 3 available modalities
		{
			for (int i = 0; i < 3/*mdiChild->GetModalities()->size()*/; ++i)
			{
				modalities[i] = new iAModalityWidget(histogramStackContainer, mdiChild->GetModality(i), mdiChild);
				histogramStackContainerLayout->addWidget(modalities[i]);
				names[i] = mdiChild->GetModality(i)->GetName();
			}
		}

		names[0] = "A (" + names[0] + ")";
		names[1] = "B (" + names[1] + ")";
		names[2] = "C (" + names[2] + ")";

		m_modality1 = modalities[0];
		m_modality2 = modalities[1];
		m_modality3 = modalities[2];

		m_modality1->setModalityLabel(names[0]);
		m_modality2->setModalityLabel(names[1]);
		m_modality3->setModalityLabel(names[2]);

		// TODO: include modality name
		//m_triangleWidget->setModality1label("A (" + names[0] + ")");
		//m_triangleWidget->setModality2label("B (" + names[1] + ")");
		//m_triangleWidget->setModality3label("C (" + names[2] + ")");
		m_triangleWidget->setModality1label(names[0]);
		m_triangleWidget->setModality2label(names[1]);
		m_triangleWidget->setModality3label(names[2]);
	}
	else { // TODO: remove?
		m_modality1 = 0;
		m_modality2 = 0;
		m_modality3 = 0;
	}


	QWidget *leftWidget = new QWidget();
	QVBoxLayout *leftWidgetLayout = new QVBoxLayout(leftWidget);
	leftWidgetLayout->addWidget(optionsContainer);
	leftWidgetLayout->addWidget(histogramStackContainer);

	dockWidgetContents->addWidget(leftWidget);
	dockWidgetContents->addWidget(m_triangleWidget);

	// Initialize transfer function
	m_transferFunction = new iAWeightedTransfer(
		m_modality1->getTransferFunction(),
		m_modality2->getTransferFunction(),
		m_modality3->getTransferFunction());

	// Does not work. TODO: fix
	mdiChild->getSlicerXY()->reInitialize(
		mdiChild->getImageData(),
		vtkTransform::New(), // no way of getting current transform, so create a new one // TODO: fix?
		m_transferFunction); // here is where the magic happens ;)

	//m_mdiChild->getRaycaster()->setTransferFunction(m_transferFunction);
	//m_mdiChild->getSlicerXY()->setTransferFunction(m_transferFunction);
	//...

	// Initialize
	updateSlicerMode();
	setWidget(dockWidgetContents);
	setWeight(m_triangleWidget->getControlPointCoordinates());

	// TODO: remove this class (ColorInterpolator), probably
	ColorInterpolator::setInstance(new LinearRGBColorInterpolator());

	//Connect
	connect(m_triangleWidget, SIGNAL(weightChanged(BCoord)), this, SLOT(setWeight(BCoord)));
	connect(m_slicerModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSlicerMode()));
	connect(m_sliceSlider, SIGNAL(valueChanged(int)), this, SLOT(setSliceNumber(int)));
	// TODO: move into for-loop?
	// {
	// TODO: necessary?
	//     {
	connect(m_modality1, SIGNAL(modalityTfChanged()), this, SLOT(updateTransferFunction()));
	connect(m_modality2, SIGNAL(modalityTfChanged()), this, SLOT(updateTransferFunction()));
	connect(m_modality3, SIGNAL(modalityTfChanged()), this, SLOT(updateTransferFunction()));
	//     }
	connect(this, SIGNAL(transferFunctionUpdated()), m_mdiChild, SLOT(ModalityTFChanged()));
	connect(this, SIGNAL(transferFunctionUpdated()), m_mdiChild, SLOT(ModalityTFChanged()));
	connect(this, SIGNAL(transferFunctionUpdated()), m_mdiChild, SLOT(ModalityTFChanged()));
	// }
}

dlg_TripleHistogramTF::~dlg_TripleHistogramTF()
{}

void dlg_TripleHistogramTF::setWeight(BCoord bCoord)
{
	m_modality1->setWeight(bCoord.getAlpha());
	m_modality2->setWeight(bCoord.getBeta());
	m_modality3->setWeight(bCoord.getGamma());
}

void dlg_TripleHistogramTF::updateSlicerMode()
{
	setSlicerMode((iASlicerMode) m_slicerModeComboBox->currentData().toInt());
}

void dlg_TripleHistogramTF::setSlicerMode(iASlicerMode slicerMode)
{
	int dimensionIndex;

	switch (slicerMode)
	{
	case iASlicerMode::YZ:
		dimensionIndex = 0; // X length is in position 0 in the dimensions array
		break;
	case iASlicerMode::XZ:
		dimensionIndex = 1; // Y length is in position 1 in the dimensions array
		break;
	case iASlicerMode::XY:
		dimensionIndex = 2; // Z length is in position 2 in the dimensions array
		break;
	default:
		// TODO?
		return;
	}

	int dimensionLength = m_mdiChild->getImageData()->GetDimensions()[dimensionIndex];
	m_sliceSlider->setMaximum(dimensionLength - 1);

	m_modality1->setSlicerMode(slicerMode, dimensionLength);
	m_modality2->setSlicerMode(slicerMode, dimensionLength);
	m_modality3->setSlicerMode(slicerMode, dimensionLength);
}

void dlg_TripleHistogramTF::setSliceNumber(int sliceNumber)
{
	m_modality1->setSliceNumber(sliceNumber);
	m_modality2->setSliceNumber(sliceNumber);
	m_modality3->setSliceNumber(sliceNumber);
}

void dlg_TripleHistogramTF::updateTransferFunction()
{
	// TODO: update weighted transfer function (m_transferFunction->...)

	// Exact same code as above. TODO: avoid that
	//mdiChild->getSlicerXY()->reInitialize(
	//	mdiChild->getImageData(),
	//	vtkTransform::New(), // no way of getting current transform, so create a new one // TODO: fix?
	//	m_transferFunction); // here is where the magic happens ;)

	//m_mdiChild->getSlicerXZ()->reInitialize(); // ?
	//m_mdiChild->getSlicerYZ()->reInitialize(); // ?
	emit transferFunctionUpdated();
}