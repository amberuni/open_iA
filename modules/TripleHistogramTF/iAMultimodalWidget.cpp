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

#include "iAMultimodalWidget.h"

#include "iASimpleSlicerWidget.h"

#include <charts/iADiagramFctWidget.h>
#include <charts/iAHistogramData.h>
#include <charts/iAPlotTypes.h>
#include <charts/iAProfileWidget.h>
#include <iAChannelSlicerData.h>
#include <iASlicer.h>
#include <iAModality.h>
#include <iAModalityList.h>
#include <iAModalityTransfer.h>
//#include <iAPerformanceHelper.h>
#include <iAPreferences.h>
#include <iASlicerMode.h>
#include <iAToolsVTK.h>
#include <iATransferFunction.h>
#include <iARenderer.h>
#include <iAVolumeRenderer.h>
#include <dlg_modalities.h>
#include <dlg_transfer.h>
#include <mdichild.h>

#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartPointer.h>
#include <vtkImageAppendComponents.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkRenderer.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QSharedPointer>
#include <QStackedLayout>
#include <QMessageBox>
#include <QCheckBox>
#include <QTimer>

// Debug
#include <QDebug>

//static const char *WEIGHT_FORMAT = "%.10f";
static const QString DISABLED_TEXT_COLOR = "rgb(0,0,0)"; // black
static const QString DISABLED_BACKGROUND_COLOR = "rgba(255,255,255)"; // white
static const int TIMER_UPDATE_VISUALIZATIONS_WAIT_MS = 250; // in milliseconds

iAMultimodalWidget::iAMultimodalWidget(QWidget* parent, MdiChild* mdiChild, NumOfMod num)
	:
	m_numOfMod(num),
	m_mdiChild(mdiChild),
	m_mainSlicersInitialized(false),
	m_slicerMode(iASlicerMode::XY),
	m_minimumWeight(0.01),
	m_timer_updateVisualizations(new QTimer())
{
	m_stackedLayout = new QStackedLayout(this);
	m_stackedLayout->setStackingMode(QStackedLayout::StackOne);

	m_disabledLabel = new QLabel(this);
	m_disabledLabel->setAlignment(Qt::AlignCenter);
	//m_disabledLabel->setStyleSheet("background-color: " + DISABLED_BACKGROUND_COLOR + "; color: " + DISABLED_TEXT_COLOR);

	QWidget *innerWidget = new QWidget(this);
	m_innerLayout = new QHBoxLayout(innerWidget);
	m_innerLayout->setMargin(0);

	m_stackedLayout->addWidget(innerWidget);
	m_stackedLayout->addWidget(m_disabledLabel);
	m_stackedLayout->setCurrentIndex(1);

	m_slicerModeLabel = new QLabel();
	m_sliceNumberLabel = new QLabel();
	updateLabels();

	m_checkBox_weightByOpacity = new QCheckBox("Weight by opacity");
	m_checkBox_weightByOpacity->setChecked(true);

	m_checkBox_syncedCamera = new QCheckBox("Synchronize cameras");
	m_checkBox_syncedCamera->setChecked(true);

	m_timer_updateVisualizations->setSingleShot(true);
	m_timerWait_updateVisualizations = TIMER_UPDATE_VISUALIZATIONS_WAIT_MS;

	for (int i = 0; i < m_numOfMod; i++) {
		m_histograms.push_back(Q_NULLPTR);
		m_slicerWidgets.push_back(Q_NULLPTR);
		m_modalitiesActive.push_back(Q_NULLPTR);
		m_modalitiesHistogramAvailable.push_back(false);
		m_copyTFs.push_back(Q_NULLPTR);
	}

	connect(m_checkBox_weightByOpacity, SIGNAL(stateChanged(int)), this, SLOT(checkBoxWeightByOpacityChanged()));
	connect(m_checkBox_syncedCamera,    SIGNAL(stateChanged(int)), this, SLOT(checkBoxSyncedCameraChanged()));

	connect(mdiChild->getSlicerDlgXY()->verticalScrollBarXY, SIGNAL(valueChanged(int)), this, SLOT(onMainXYSliceNumberChanged(int)));
	connect(mdiChild->getSlicerDlgXZ()->verticalScrollBarXZ, SIGNAL(valueChanged(int)), this, SLOT(onMainXZSliceNumberChanged(int)));
	connect(mdiChild->getSlicerDlgYZ()->verticalScrollBarYZ, SIGNAL(valueChanged(int)), this, SLOT(onMainYZSliceNumberChanged(int)));

	connect(mdiChild->getSlicerDlgXY()->verticalScrollBarXY, SIGNAL(sliderPressed()), this, SLOT(onMainXYScrollBarPress()));
	connect(mdiChild->getSlicerDlgXZ()->verticalScrollBarXZ, SIGNAL(sliderPressed()), this, SLOT(onMainXZScrollBarPress()));
	connect(mdiChild->getSlicerDlgYZ()->verticalScrollBarYZ, SIGNAL(sliderPressed()), this, SLOT(onMainYZScrollBarPress()));

	connect(mdiChild->getSlicerDlgXY()->spinBoxXY, SIGNAL(valueChanged(int)), this, SLOT(onMainXYSliceNumberChanged(int)));
	connect(mdiChild->getSlicerDlgXZ()->spinBoxXZ, SIGNAL(valueChanged(int)), this, SLOT(onMainXZSliceNumberChanged(int)));
	connect(mdiChild->getSlicerDlgYZ()->spinBoxYZ, SIGNAL(valueChanged(int)), this, SLOT(onMainYZSliceNumberChanged(int)));

	//connect(mdiChild->GetModalitiesDlg(), SIGNAL(ModalitiesChanged()), this, SLOT(modalitiesChanged()));
	connect(mdiChild, SIGNAL(histogramAvailable()), this, SLOT(histogramAvailable()));
	connect(mdiChild, &MdiChild::renderSettingsChanged, this, &iAMultimodalWidget::applyVolumeSettings);
	connect(mdiChild, &MdiChild::slicerSettingsChanged, this, &iAMultimodalWidget::applySlicerSettings);

	connect(m_timer_updateVisualizations, SIGNAL(timeout()), this, SLOT(onUpdateVisualizationsTimeout()));

	histogramAvailable();
}

// ----------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------

void iAMultimodalWidget::setSlicerMode(iASlicerMode slicerMode) {
	if (m_slicerMode == slicerMode) {
		return;
	}
	disconnectMainSlicer();
	m_slicerMode = slicerMode;
	int sliceNumber = getSliceNumber();
	for (int i = 0; i < m_numOfMod; i++) {
		w_slicer(i)->setSlicerMode(slicerMode);
		w_slicer(i)->setSliceNumber(sliceNumber);
	}
	setMainSlicerCamera();
	updateLabels();
	updateVisualizationsLater();

	emit slicerModeChangedExternally(slicerMode);
}

void iAMultimodalWidget::setSliceNumber(int sliceNumber) {
	for (int i = 0; i < m_numOfMod; i++) {
		w_slicer(i)->setSliceNumber(sliceNumber);
	}
	updateLabels();
	updateVisualizationsLater();

	emit sliceNumberChangedExternally(sliceNumber);
}

void iAMultimodalWidget::setWeightsProtected(BCoord bCoord, double t)
{
	if (bCoord == m_weights) {
		return;
	}

	m_weights = bCoord;
	applyWeights();
	updateVisualizationsLater();
	emit weightsChanged2(t);
	emit weightsChanged3(bCoord);
}

void iAMultimodalWidget::updateVisualizationsLater() {
	m_timer_updateVisualizations->start(m_timerWait_updateVisualizations);
}

void iAMultimodalWidget::updateVisualizationsNow()
{
	m_timer_updateVisualizations->stop();

	m_mdiChild->redrawHistogram();
	m_mdiChild->getRenderer()->update();

	if (!m_mainSlicersInitialized)
		return;

	assert(m_numOfMod != UNDEFINED);

	//iATimeGuard test("updateMainSlicers");

	iASlicerData* slicerDataArray[] = {
		m_mdiChild->getSlicerDataYZ(),
		m_mdiChild->getSlicerDataXY(),
		m_mdiChild->getSlicerDataXZ()
	};

	for (int mainSlicerIndex = 0; mainSlicerIndex < 3; mainSlicerIndex++) {

		auto data = slicerDataArray[mainSlicerIndex];

		vtkSmartPointer<vtkImageData> slicersColored[3];
		vtkSmartPointer<vtkImageData> slicerInput[3];
		vtkPiecewiseFunction* slicerOpacity[3];
		for (int modalityIndex = 0; modalityIndex < m_numOfMod; modalityIndex++) {
			iAChannelID id = static_cast<iAChannelID>(ch_Meta0 + modalityIndex);
			auto channel = data->GetChannel(id);
			data->setChannelOpacity(id, 0);

			//vtkImageData imgMod = data->GetReslicer()->GetOutput();
			//auto imgMod = data->GetChannel(ch_Meta0 + 1)->reslicer->GetOutput();
			// This changes everytime the TF changes!
			auto imgMod = channel->reslicer->GetOutput();
			slicerInput[modalityIndex] = imgMod;
			slicerOpacity[modalityIndex] = channel->m_otf;

			// Source: https://vtk.org/Wiki/VTK/Examples/Cxx/Images/ImageMapToColors
			// This changes everytime the TF changes!
			auto scalarValuesToColors = vtkSmartPointer<vtkImageMapToColors>::New(); // Will it work?
			//scalarValuesToColors->SetLookupTable(channel->m_lut);
			scalarValuesToColors->SetLookupTable(channel->m_ctf);
			scalarValuesToColors->SetInputData(imgMod);
			scalarValuesToColors->Update();
			slicersColored[modalityIndex] = scalarValuesToColors->GetOutput();
		}
		auto imgOut = m_slicerImages[mainSlicerIndex];

		// if you want to try out alternative using buffers below, start commenting out here
		auto w = getWeights();
		FOR_VTKIMG_PIXELS(imgOut, x, y, z) {

			float modRGB[3][3];
			float weight[3];
			float weightSum = 0;
			for (int mod = 0; mod < m_numOfMod; ++mod)
			{
				// compute weight for this modality:
				weight[mod] = w[mod];
				if (m_checkBox_weightByOpacity->isChecked())
				{
					float intensity = slicerInput[mod]->GetScalarComponentAsFloat(x, y, z, 0);
					double opacity = slicerOpacity[mod]->GetValue(intensity);
					weight[mod] *= std::max(m_minimumWeight, opacity);

				}
				weightSum += weight[mod];
				// get color of this modality:
				for (int component = 0; component < 3; ++component)
					modRGB[mod][component] = (mod >= m_numOfMod) ? 0
						: slicersColored[mod]->GetScalarComponentAsFloat(x, y, z, component);
			}
			// "normalize" weights (i.e., make their sum equal to 1):
			if (weightSum == 0)
			{
				for (int mod = 0; mod < m_numOfMod; ++mod)
					weight[mod] = 1/m_numOfMod;
			}
			else
			{
				for (int mod = 0; mod < m_numOfMod; ++mod)
					weight[mod] /= weightSum;
			}
			// compute and set final color values:
			for (int component = 0; component < 3; ++component)
			{
				float value = 0;
				for (int mod = 0; mod < m_numOfMod; ++mod)
					value += modRGB[mod][component] * weight[mod];
				imgOut->SetScalarComponentFromFloat(x, y, z, component, value);
			}
			float a = 255; // Max alpha!
			imgOut->SetScalarComponentFromFloat(x, y, z, 3, a);
		}

		// Sets the INPUT image which will be sliced again, but we have a sliced image already
		//m_mdiChild->getSlicerDataYZ()->changeImageData(imgOut);
		imgOut->Modified();
		data->GetImageActor()->SetInputData(imgOut);
	}

	m_mdiChild->getSlicerXY()->update();
	m_mdiChild->getSlicerXZ()->update();
	m_mdiChild->getSlicerYZ()->update();
}

void iAMultimodalWidget::updateTransferFunction(int index)
{
	updateOriginalTransferFunction(index);
	w_slicer(index)->update();
	w_histogram(index)->update();
	updateVisualizationsLater();
}

void iAMultimodalWidget::updateDisabledLabel()
{
	int count = getModalitiesCount();
	int missing = m_numOfMod - count;
	QString modalit_y_ies_is_are = missing == 1 ? "modality is" : "modalities are";
	m_disabledLabel->setText(
		"Unable to set up this widget.\n" +
		QString::number(missing) + " " + modalit_y_ies_is_are + " missing.\n"
	);
}



// ----------------------------------------------------------------------------------
// Modalities management
// ----------------------------------------------------------------------------------

void iAMultimodalWidget::histogramAvailable() {
	updateModalities();

	if (getModalitiesCount() < m_numOfMod) {
		updateDisabledLabel();
		m_stackedLayout->setCurrentIndex(1);
		return;
	}

	m_stackedLayout->setCurrentIndex(0);

	auto appendFilter = vtkSmartPointer<vtkImageAppendComponents>::New();
	appendFilter->SetInputData(getModality(0)->GetImage());
	for (int i = 1; i < m_numOfMod; i++) {
		appendFilter->AddInputData(getModality(i)->GetImage());
	}
	appendFilter->Update();

	m_combinedVol = vtkSmartPointer<vtkVolume>::New();
	auto combinedVolProp = vtkSmartPointer<vtkVolumeProperty>::New();

	for (int i = 0; i < m_numOfMod; i++) {
		auto transfer = getModality(i)->GetTransfer();
		combinedVolProp->SetColor(i, transfer->getColorFunction());
		combinedVolProp->SetScalarOpacity(i, transfer->getOpacityFunction());
	}

	m_combinedVol->SetProperty(combinedVolProp);

	m_combinedVolMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	m_combinedVolMapper->SetBlendModeToComposite();
	m_combinedVolMapper->SetInputData(appendFilter->GetOutput());
	m_combinedVolMapper->Update();
	applyVolumeSettings();
	m_combinedVol->SetMapper(m_combinedVolMapper);
	m_combinedVol->Update();

	m_combinedVolRenderer = vtkSmartPointer<vtkRenderer>::New();
	m_combinedVolRenderer->SetActiveCamera(m_mdiChild->getRenderer()->getCamera());
	m_combinedVolRenderer->GetActiveCamera()->ParallelProjectionOn();
	m_combinedVolRenderer->SetLayer(1);
	m_combinedVolRenderer->AddVolume(m_combinedVol);
	//m_combinedVolRenderer->ResetCamera();

	for (int i = 0; i < m_numOfMod; ++i)
	{
		QSharedPointer<iAVolumeRenderer> renderer = getModality(i)->GetRenderer();
		if (renderer->isRendered())
			renderer->Remove();
	}
	m_mdiChild->getRenderer()->AddRenderer(m_combinedVolRenderer);

	// The next code section sets up the main slicers

	iASlicerData* slicerDataArray[] = {
		m_mdiChild->getSlicerDataYZ(),
		m_mdiChild->getSlicerDataXY(),
		m_mdiChild->getSlicerDataXZ()
	};

	iAChannelID id = static_cast<iAChannelID>(ch_Meta0 + 0);
	for (int mainSlicerIndex = 0; mainSlicerIndex < 3; mainSlicerIndex++) {
		iASlicerData *data = slicerDataArray[mainSlicerIndex];
		int const *dims = slicerDataArray[mainSlicerIndex]->GetChannel(id)->reslicer->GetOutput()->GetDimensions();
		// should be double const once VTK supports it:
		double * spc = slicerDataArray[mainSlicerIndex]->GetChannel(id)->reslicer->GetOutput()->GetSpacing();

		//data->GetImageActor()->SetOpacity(0.0);
		//data->SetManualBackground(1.0, 1.0, 1.0);
		//data->SetManualBackground(0.0, 0.0, 0.0);

		//vtkSmartPointer<vtkImageData>::New() OR WHATEVER
		//open_iA_Core_API vtkSmartPointer<vtkImageData> AllocateImage(vtkSmartPointer<vtkImageData> img);
		//auto imgOut = AllocateImage(imgMod1);
		auto imgOut = vtkSmartPointer<vtkImageData>::New();
		imgOut->SetDimensions(dims);
		imgOut->SetSpacing(spc);
		imgOut->AllocateScalars(VTK_UNSIGNED_CHAR, 4);
		m_slicerImages[mainSlicerIndex] = imgOut;
	}
	connectAcrossSlicers();
	setMainSlicerCamera();

	m_mainSlicersInitialized = true;
	updateVisualizationsNow();
}

void iAMultimodalWidget::applyVolumeSettings()
{
	auto vs = m_mdiChild->GetVolumeSettings();
	auto volProp = m_combinedVol->GetProperty();
	volProp->SetAmbient(vs.AmbientLighting);
	volProp->SetDiffuse(vs.DiffuseLighting);
	volProp->SetSpecular(vs.SpecularLighting);
	volProp->SetSpecularPower(vs.SpecularPower);
	volProp->SetInterpolationType(vs.LinearInterpolation);
	volProp->SetShade(vs.Shading);
	if (vs.ScalarOpacityUnitDistance > 0)
		volProp->SetScalarOpacityUnitDistance(vs.ScalarOpacityUnitDistance);
	if (m_mdiChild->GetRenderSettings().ShowSlicers)
	{
		m_combinedVolMapper->AddClippingPlane(m_mdiChild->getRenderer()->getPlane1());
		m_combinedVolMapper->AddClippingPlane(m_mdiChild->getRenderer()->getPlane2());
		m_combinedVolMapper->AddClippingPlane(m_mdiChild->getRenderer()->getPlane3());
	}
	else
	{
		m_combinedVolMapper->RemoveAllClippingPlanes();
	}
#ifdef VTK_OPENGL2_BACKEND
	m_combinedVolMapper->SetSampleDistance(vs.SampleDistance);
	m_combinedVolMapper->InteractiveAdjustSampleDistancesOff();
#endif
}

void iAMultimodalWidget::applySlicerSettings()
{
	for (int i = 0; i < m_numOfMod; ++i)
	{
		m_slicerWidgets[i]->applySettings(m_mdiChild->GetSlicerSettings().SingleSlicer);
	}
}

// When new modalities are added/removed
void iAMultimodalWidget::updateModalities()
{
	if (m_mdiChild->GetModalities()->size() >= m_numOfMod) {
		bool allModalitiesAreHere = true;
		for (int i = 0; i < m_numOfMod; i++) {
			if (/*NOT*/ ! containsModality(m_mdiChild->GetModality(i))) {
				allModalitiesAreHere = false;
				break;
			}
		}
		if (allModalitiesAreHere) {
			return; // No need to update modalities if all of them are already here!
		}

	} else {
		int i = 0;
		for (; i < m_numOfMod && i < m_mdiChild->GetModalities()->size(); ++i) {
			m_modalitiesActive[i] = m_mdiChild->GetModality(i);
		}
		for (; i < m_numOfMod; i++) {
			m_modalitiesActive[i] = Q_NULLPTR;
		}
		return;
	}

	// Initialize modalities being added
	for (int i = 0; i < m_numOfMod; ++i) {
		m_modalitiesActive[i] = m_mdiChild->GetModality(i);

		// Histogram {
		if (!m_modalitiesActive[i]->GetHistogramData() || m_modalitiesActive[i]->GetHistogramData()->GetNumBin() != m_mdiChild->GetPreferences().HistogramBins)
		{
			m_modalitiesActive[i]->ComputeImageStatistics();
			m_modalitiesActive[i]->ComputeHistogramData(m_mdiChild->GetPreferences().HistogramBins);
		}
		m_modalitiesHistogramAvailable[i] = true;

		vtkColorTransferFunction *colorFuncCopy = vtkColorTransferFunction::New();
		vtkPiecewiseFunction *opFuncCopy = vtkPiecewiseFunction::New();
		m_copyTFs[i] = createCopyTf(i, colorFuncCopy, opFuncCopy);

		m_histograms[i] = QSharedPointer<iADiagramFctWidget>(new iADiagramFctWidget(nullptr, m_mdiChild, m_modalitiesActive[i]->GetName()+" gray value", "Frequency"));
		QSharedPointer<iAPlot> histogramPlot = QSharedPointer<iAPlot>(
			new	iABarGraphPlot(m_modalitiesActive[i]->GetHistogramData(), QColor(70, 70, 70, 255)));
		m_histograms[i]->addPlot(histogramPlot);
		m_histograms[i]->setTransferFunctions(m_copyTFs[i]->getColorFunction(), m_copyTFs[i]->getOpacityFunction());
		m_histograms[i]->updateTrf();
		// }

		// Slicer {
		resetSlicer(i);
		// }

		iAChannelID id = static_cast<iAChannelID>(ch_Meta0 + i);
		iAChannelVisualizationData* chData = new iAChannelVisualizationData();
		m_mdiChild->InsertChannelData(id, chData);
		vtkImageData* imageData = m_modalitiesActive[i]->GetImage();
		vtkColorTransferFunction* ctf = m_modalitiesActive[i]->GetTransfer()->getColorFunction();
		vtkPiecewiseFunction* otf = m_modalitiesActive[i]->GetTransfer()->getOpacityFunction();
		ResetChannel(chData, imageData, ctf, otf);
		m_mdiChild->InitChannelRenderer(id, false, true);
	}

	connect((dlg_transfer*)(m_histograms[0]->getFunctions()[0]), SIGNAL(Changed()), this, SLOT(updateTransferFunction1()));
	connect((dlg_transfer*)(m_histograms[1]->getFunctions()[0]), SIGNAL(Changed()), this, SLOT(updateTransferFunction2()));
	if (m_numOfMod >= THREE) {
		connect((dlg_transfer*)(m_histograms[2]->getFunctions()[0]), SIGNAL(Changed()), this, SLOT(updateTransferFunction3()));
	}

	applyWeights();
	connect((dlg_transfer*)(m_mdiChild->getHistogram()->getFunctions()[0]), SIGNAL(Changed()), this, SLOT(originalHistogramChanged()));

	emit(modalitiesLoaded_beforeUpdate());

	update();
}

void iAMultimodalWidget::resetSlicer(int i)
{
	// Slicer is replaced here.
	// Make sure there are no other references to the old iASimpleSlicerWidget
	// referenced by the QSharedPointer!
	m_slicerWidgets[i] = QSharedPointer<iASimpleSlicerWidget>(new iASimpleSlicerWidget(nullptr, true));
	m_slicerWidgets[i]->applySettings(m_mdiChild->GetSlicerSettings().SingleSlicer);
	m_slicerWidgets[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	if (m_modalitiesActive[i]) {
		m_slicerWidgets[i]->changeModality(m_modalitiesActive[i]);
	}
}

QSharedPointer<iATransferFunction> iAMultimodalWidget::createCopyTf(int index, vtkSmartPointer<vtkColorTransferFunction> colorTf, vtkSmartPointer<vtkPiecewiseFunction> opacityFunction)
{
	colorTf->DeepCopy(m_modalitiesActive[index]->GetTransfer()->getColorFunction());
	opacityFunction->DeepCopy(m_modalitiesActive[index]->GetTransfer()->getOpacityFunction());
	return QSharedPointer<iATransferFunction>(
		new iASimpleTransferFunction(colorTf, opacityFunction));
}

void iAMultimodalWidget::alertWeightIsZero(QSharedPointer<iAModality> modality)
{
	QString name = modality->GetFileName();
	QString text =
		"The main transfer function of a modality cannot be changed "
		"while the weight of that modality is zero.\n"
		"Modality:\n" + name +
		"\n\n"
		"To change the transfer function, use an n-modal widget instead "
		"(Double/Triple Histogram Transfer Function).";

	QMessageBox msg;
	msg.setText(text);
	msg.exec();
}

void iAMultimodalWidget::originalHistogramChanged()
{
	QSharedPointer<iAModality> selected = m_mdiChild->GetModalitiesDlg()->GetModalities()->Get(m_mdiChild->GetModalitiesDlg()->GetSelected());
	for (int i = 0; i < m_numOfMod; i++) {
		if (selected == getModality(i)) {
			updateCopyTransferFunction(i);
			updateTransferFunction(i);
			updateVisualizationsLater();
			return;
		}
	}
}

/** Called when the original transfer function changes
* RESETS THE COPY (admit numerical imprecision when setting the copy values)
* => effective / weight = copy
*/
void iAMultimodalWidget::updateCopyTransferFunction(int index)
{
	if (isReady()) {
		double weight = getWeight(index);
		if (weight == 0) {
			updateOriginalTransferFunction(index); // Revert the changes made to the effective TF
			//alertWeightIsZero(getModality(index));
			// For now, just return silently. TODO: show alert?
			return;
		}

		// newly set transfer function (set via the histogram)
		QSharedPointer<iAModalityTransfer> effective = getModality(index)->GetTransfer();

		// copy of previous transfer function, to be updated in this method
		QSharedPointer<iATransferFunction> copy = m_copyTFs[index];

		double valCol[6], valOp[4];
		copy->getColorFunction()->RemoveAllPoints();
		copy->getOpacityFunction()->RemoveAllPoints();

		for (int j = 0; j < effective->getColorFunction()->GetSize(); ++j)
		{
			effective->getColorFunction()->GetNodeValue(j, valCol);
			effective->getOpacityFunction()->GetNodeValue(j, valOp);

			if (valOp[1] > weight) {
				valOp[1] = weight;
			}
			double copyOp = valOp[1] / weight;

			effective->getOpacityFunction()->SetNodeValue(j, valOp);

			copy->getColorFunction()->AddRGBPoint(valCol[0], valCol[1], valCol[2], valCol[3], valCol[4], valCol[5]);
			copy->getOpacityFunction()->AddPoint(valOp[0], copyOp, valOp[2], valOp[3]);
		}
	}
}

/** Called when the copy transfer function changes
* ADD NODES TO THE EFFECTIVE ONLY (clear and repopulate with adjusted effective values)
* => copy * weight ~= effective
*/
void iAMultimodalWidget::updateOriginalTransferFunction(int index)
{
	if (!isReady()) {
		return;
	}

	double weight = getWeight(index);

	// newly set transfer function (set via the histogram)
	QSharedPointer<iAModalityTransfer> effective = m_modalitiesActive[index]->GetTransfer();

	// copy of previous transfer function, to be updated in this method
	QSharedPointer<iATransferFunction> copy = m_copyTFs[index];

	double valCol[6], valOp[4];
	effective->getColorFunction()->RemoveAllPoints();
	effective->getOpacityFunction()->RemoveAllPoints();

	for (int j = 0; j < copy->getColorFunction()->GetSize(); ++j)
	{
		copy->getColorFunction()->GetNodeValue(j, valCol);
		copy->getOpacityFunction()->GetNodeValue(j, valOp);

		valOp[1] = valOp[1] * weight; // index 1 means opacity

		effective->getColorFunction()->AddRGBPoint(valCol[0], valCol[1], valCol[2], valCol[3], valCol[4], valCol[5]);
		effective->getOpacityFunction()->AddPoint(valOp[0], valOp[1], valOp[2], valOp[3]);
	}
}

/** Resets the values of all nodes in the effective transfer function using the values present in the
*     copy of the transfer function, using m_weightCur for the adjustment
* CHANGES THE NODES OF THE EFFECTIVE ONLY (based on the copy)
*/
void iAMultimodalWidget::applyWeights()
{
	if (!isReady())
		return;
	for (int i = 0; i < m_numOfMod; i++) {
		vtkPiecewiseFunction *effective = m_modalitiesActive[i]->GetTransfer()->getOpacityFunction();
		vtkPiecewiseFunction *copy = m_copyTFs[i]->getOpacityFunction();

		double pntVal[4];
		for (int j = 0; j < copy->GetSize(); ++j)
		{
			copy->GetNodeValue(j, pntVal);
			pntVal[1] = pntVal[1] * getWeight(i); // index 1 in pntVal means opacity
			effective->SetNodeValue(j, pntVal);
		}
		m_histograms[i]->update();
	}
}

namespace {
	iASlicer* slicerForMode(MdiChild* mdiChild, int slicerMode) {
		switch (slicerMode) {
		case iASlicerMode::YZ: return mdiChild->getSlicerYZ();
		case iASlicerMode::XZ: return mdiChild->getSlicerXZ();
		case iASlicerMode::XY: return mdiChild->getSlicerXY();
		}
		return nullptr;
	}
}

void iAMultimodalWidget::setMainSlicerCamera()
{
	if (!m_checkBox_syncedCamera->isChecked())
		return;
	vtkCamera * mainCamera = slicerForMode(m_mdiChild, m_slicerMode)->GetCamera();
	for (int i = 0; i < m_numOfMod; ++i)
	{
		w_slicer(i)->setCamera(mainCamera);
		w_slicer(i)->update();
	}
	connectMainSlicer();
}

void iAMultimodalWidget::resetSlicerCamera()
{
	disconnectMainSlicer();
	for (int i = 0; i < m_numOfMod; i++) {
		auto cam = vtkSmartPointer<vtkCamera>::New();
		w_slicer(i)->setCamera(cam);
		w_slicer(i)->getSlicer()->GetSlicerData()->ResetCamera();
		w_slicer(i)->update();
	}
}

void iAMultimodalWidget::connectMainSlicer()
{
	if (!m_checkBox_syncedCamera->isChecked())
		return;
	iASlicer* slicer = slicerForMode(m_mdiChild, m_slicerMode);
	for (int i = 0; i < m_numOfMod; ++i)
	{
		connect(slicer->GetSlicerData(), SIGNAL(UserInteraction()), w_slicer(i).data(), SLOT(update()));
		connect(w_slicer(i)->getSlicer()->GetSlicerData(), SIGNAL(UserInteraction()), slicer, SLOT(update()));
	}
}

void iAMultimodalWidget::disconnectMainSlicer()
{
	iASlicer* slicer = slicerForMode(m_mdiChild, m_slicerMode);
	for (int i = 0; i < m_numOfMod; ++i)
	{
		disconnect(slicer->GetSlicerData(), SIGNAL(UserInteraction()), w_slicer(i).data(), SLOT(update()));
		disconnect(w_slicer(i)->getSlicer()->GetSlicerData(), SIGNAL(UserInteraction()), slicer, SLOT(update()));
	}
}

void iAMultimodalWidget::connectAcrossSlicers()
{
	for (int i = 0; i < m_numOfMod; ++i)
		for (int j = 0; j < m_numOfMod; ++j)
			if (i != j)
				connect(w_slicer(i)->getSlicer()->GetSlicerData(), SIGNAL(UserInteraction()), w_slicer(j).data(), SLOT(update()));
}

void iAMultimodalWidget::disconnectAcrossSlicers()
{
	for (int i = 0; i < m_numOfMod; ++i)
		for (int j = 0; j < m_numOfMod; ++j)
			if (i != j)
				disconnect(w_slicer(i)->getSlicer()->GetSlicerData(), SIGNAL(UserInteraction()), w_slicer(j).data(), SLOT(update()));
}

// ----------------------------------------------------------------------------------
// Short methods
// ----------------------------------------------------------------------------------

void iAMultimodalWidget::onUpdateVisualizationsTimeout() {
	updateVisualizationsNow();
}

iASlicerMode iAMultimodalWidget::getSlicerMode() {
	return m_slicerMode;
}

QString iAMultimodalWidget::getSlicerModeString() {
	switch (m_slicerMode) {
	case iASlicerMode::YZ:
		return "YZ";
	case iASlicerMode::XZ:
		return "XZ";
	case iASlicerMode::XY:
		return "XY";
	default:
		// TODO exception
		return -1;
	}
}

void iAMultimodalWidget::updateLabels() {
	m_slicerModeLabel->setText("Slicer mode: " + getSlicerModeString());
	m_sliceNumberLabel->setText("Slice number: " + QString::number(getSliceNumber()));
}

int iAMultimodalWidget::getSliceNumber() {
	switch (m_slicerMode)
	{
	case iASlicerMode::YZ:
		return m_mdiChild->getSlicerDataYZ()->getSliceNumber();
	case iASlicerMode::XZ:
		return m_mdiChild->getSlicerDataXZ()->getSliceNumber();
	case iASlicerMode::XY:
		return m_mdiChild->getSlicerDataXY()->getSliceNumber();
	default:
		// TODO exception
		return -1;
	}
}

void iAMultimodalWidget::checkBoxWeightByOpacityChanged()
{
	updateVisualizationsNow();
}

void iAMultimodalWidget::checkBoxSyncedCameraChanged()
{
	if (m_checkBox_syncedCamera->isChecked()) {
		connectAcrossSlicers();
		setMainSlicerCamera();
	} else {
		disconnectAcrossSlicers();
		resetSlicerCamera();
	}
}

// SCROLLBARS (private SLOTS)
void iAMultimodalWidget::onMainXYScrollBarPress() {
	setSlicerMode(iASlicerMode::XY);
}

void iAMultimodalWidget::onMainXZScrollBarPress() {
	setSlicerMode(iASlicerMode::XZ);
}

void iAMultimodalWidget::onMainYZScrollBarPress() {
	setSlicerMode(iASlicerMode::YZ);
}

void iAMultimodalWidget::onMainXYSliceNumberChanged(int sliceNumberXY) {
	setSlicerMode(iASlicerMode::XY);
	setSliceNumber(sliceNumberXY);
}

void iAMultimodalWidget::onMainXZSliceNumberChanged(int sliceNumberXZ) {
	setSlicerMode(iASlicerMode::XZ);
	setSliceNumber(sliceNumberXZ);
}

void iAMultimodalWidget::onMainYZSliceNumberChanged(int sliceNumberYZ) {
	setSlicerMode(iASlicerMode::YZ);
	setSliceNumber(sliceNumberYZ);
}

QSharedPointer<iAModality> iAMultimodalWidget::getModality(int index)
{
	return m_modalitiesActive[index];
}

vtkSmartPointer<vtkImageData> iAMultimodalWidget::getModalityImage(int index) {
	return getModality(index)->GetImage();
}

BCoord iAMultimodalWidget::getWeights()
{
	return m_weights;
}

double iAMultimodalWidget::getWeight(int i)
{
	return m_weights[i];
}

bool iAMultimodalWidget::isReady()
{
	if (m_modalitiesActive[m_numOfMod - 1]) {
		for (int i = 0; i < m_numOfMod; i++) {
			if (!m_modalitiesHistogramAvailable[i]) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool iAMultimodalWidget::containsModality(QSharedPointer<iAModality> modality)
{
	for (auto mod : m_modalitiesActive) {
		if (mod == modality) {
			return true;
		}
	}
	return false;
}

int iAMultimodalWidget::getModalitiesCount()
{
	for (int i = m_numOfMod - 1; i >= 0; i--) {
		if (m_modalitiesActive[i]) {
			return i + 1;
		}
	}
	return 0;
}