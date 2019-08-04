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
#include "iANModalController.h"

#include "iAModality.h"
#include "iAModalityList.h"
#include "iAModalityTransfer.h"
#include "iARenderer.h"
#include "iAVolumeRenderer.h"
#include "iASlicer.h"
#include "iAChannelSlicerData.h"
#include "iAChannelData.h"
#include "mdichild.h"

#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageAppendComponents.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkImageReslice.h>
#include <vtkSmartPointer.h>

iANModalController::iANModalController(MdiChild *mdiChild) {

}

void iANModalController::initialize() {
	assert(!m_initialized);
	_initialize();
}

void iANModalController::reinitialize() {
	assert(m_initialized);
	_initialize();
}

void iANModalController::_initialize() {
	m_modalities.clear();
	auto modalities = m_mdiChild->modalities();
	for (int i = 0; i < modalities->size(); i++) {
		auto modality = modalities->get(i);
		m_modalities.append(modality);
	}

	// TODO: cherry pick modalities

	assert(countModalities() >= 1 && countModalities() < 4);

	m_slicers.clear();
	for (int i = 0; i < m_modalities.size(); i++) {
		_initializeModality(m_modalities[i]);
	}

	auto appendFilter = vtkSmartPointer<vtkImageAppendComponents>::New();
	appendFilter->SetInputData(m_modalities[0]->image());
	for (int i = 1; i < countModalities(); i++) {
		appendFilter->AddInputData(m_modalities[i]->image());
	}
	appendFilter->Update();

	m_combinedVol = vtkSmartPointer<vtkVolume>::New();
	auto combinedVolProp = vtkSmartPointer<vtkVolumeProperty>::New();

	for (int i = 0; i < countModalities(); i++) {
		auto transfer = m_modalities[i]->transfer();
		combinedVolProp->SetColor(i, transfer->colorTF());
		combinedVolProp->SetScalarOpacity(i, transfer->opacityTF());
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
	m_combinedVolRenderer->SetActiveCamera(m_mdiChild->renderer()->camera());
	m_combinedVolRenderer->GetActiveCamera()->ParallelProjectionOn();
	m_combinedVolRenderer->SetLayer(1);
	m_combinedVolRenderer->AddVolume(m_combinedVol);
	//m_combinedVolRenderer->ResetCamera();

	for (int i = 0; i < countModalities(); ++i) {
		QSharedPointer<iAVolumeRenderer> renderer = m_modalities[i]->renderer();
		if (renderer->isRendered())
			renderer->remove();
	}
	m_mdiChild->renderer()->addRenderer(m_combinedVolRenderer);

	// The next code section sets up the main slicers

	iASlicer* slicerArray[] = {
		m_mdiChild->slicer(iASlicerMode::YZ),
		m_mdiChild->slicer(iASlicerMode::XY),
		m_mdiChild->slicer(iASlicerMode::XZ)
	};

	// Hard-coded 3? TODO: remove
	for (int mainSlicerIndex = 0; mainSlicerIndex < 3; mainSlicerIndex++) {
		auto reslicer = slicerArray[mainSlicerIndex]->channel(m_channelIds[0])->reslicer();

		int const *dims = reslicer->GetOutput()->GetDimensions();
		// should be double const once VTK supports it:
		double *spc = reslicer->GetOutput()->GetSpacing();

		//data->GetImageActor()->SetOpacity(0.0);
		//data->SetManualBackground(1.0, 1.0, 1.0);
		//data->SetManualBackground(0.0, 0.0, 0.0);

		auto imgOut = vtkSmartPointer<vtkImageData>::New();
		imgOut->SetDimensions(dims);
		imgOut->SetSpacing(spc);
		imgOut->AllocateScalars(VTK_UNSIGNED_CHAR, 4);
		m_slicerImages[mainSlicerIndex] = imgOut;
	}

	//connectAcrossSlicers();
	//setMainSlicerCamera();

	//m_mainSlicersInitialized = true;
	//updateVisualizationsNow();

	m_initialized = true;
}

void iANModalController::_initializeModality(QSharedPointer<iAModality> modality) {
	// Hide everything except the slice itself
	auto slicerXY = new iASlicer(nullptr, iASlicerMode::XY, /*bool decorations = */false);
	auto slicerXZ = new iASlicer(nullptr, iASlicerMode::XZ, /*bool decorations = */false);
	auto slicerYZ = new iASlicer(nullptr, iASlicerMode::YZ, /*bool decorations = */false);
	iASlicer *slicers[3] = {
		new iASlicer(nullptr, iASlicerMode::XY, /*bool decorations = */false),
		new iASlicer(nullptr, iASlicerMode::XZ, /*bool decorations = */false),
		new iASlicer(nullptr, iASlicerMode::YZ, /*bool decorations = */false)
	};

	vtkImageData *image = modality->image().GetPointer();
	vtkColorTransferFunction* colorTf = modality->transfer()->colorTF();

	double* origin = image->GetOrigin();
	int* extent = image->GetExtent();
	double* spacing = image->GetSpacing();

	double xc = origin[0] + 0.5*(extent[0] + extent[1])*spacing[0];
	double yc = origin[1] + 0.5*(extent[2] + extent[3])*spacing[1];
	double xd = (extent[1] - extent[0] + 1)*spacing[0];
	double yd = (extent[3] - extent[2] + 1)*spacing[1];

	for (int i = 0; i < 3; i++) {
		auto slicer = slicers[i];

		slicer->setup(m_mdiChild->slicerSettings().SingleSlicer);
		slicer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
		slicer->addChannel(0, iAChannelData(modality->name(), image, colorTf), true);

		vtkCamera *camera = slicer->camera();
		double d = camera->GetDistance();
		camera->SetParallelScale(0.5*yd);
		camera->SetFocalPoint(xc, yc, 0.0);
		camera->SetPosition(xc, yc, +d);

		m_slicers.append(QSharedPointer<iASlicer>(slicer));
	}
}

void iANModalController::applyVolumeSettings() {
	auto vs = m_mdiChild->volumeSettings();
	auto volProp = m_combinedVol->GetProperty();
	volProp->SetAmbient(vs.AmbientLighting);
	volProp->SetDiffuse(vs.DiffuseLighting);
	volProp->SetSpecular(vs.SpecularLighting);
	volProp->SetSpecularPower(vs.SpecularPower);
	volProp->SetInterpolationType(vs.LinearInterpolation);
	volProp->SetShade(vs.Shading);
	if (vs.ScalarOpacityUnitDistance > 0)
		volProp->SetScalarOpacityUnitDistance(vs.ScalarOpacityUnitDistance);
	if (m_mdiChild->renderSettings().ShowSlicers)
	{
		m_combinedVolMapper->AddClippingPlane(m_mdiChild->renderer()->plane1());
		m_combinedVolMapper->AddClippingPlane(m_mdiChild->renderer()->plane2());
		m_combinedVolMapper->AddClippingPlane(m_mdiChild->renderer()->plane3());
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

int iANModalController::countModalities() {
	// Cannot be larger than 4 because of VTK limit
	return m_modalities.size();
}

QList<QSharedPointer<iAModality>> iANModalController::cherryPickModalities(QList<QSharedPointer<iAModality>> modalities) {
	QList<QSharedPointer<iAModality>> dummy;
	return dummy;
}

bool iANModalController::_checkModalities(QList<QSharedPointer<iAModality>> modalities) {
	if (modalities.size() < 1 || modalities.size() > 4) {
		return false;
	}

	for (int i = 1; i < modalities.size(); i++) {
		if (!_matchModalities(modalities[0], modalities[i])) {
			return false;
		}
	}

	return true;
}


bool iANModalController::_matchModalities(QSharedPointer<iAModality> m1, QSharedPointer<iAModality> m2) {
	auto image1 = m1->image();
	const int *extent1 = image1->GetExtent();
	const double *spacing1 = image1->GetSpacing();
	const double *origin1 = image1->GetOrigin();

	auto image2 = m2->image();
	const int *extent2 = image2->GetExtent();
	const double *spacing2 = image2->GetSpacing();
	const double *origin2 = image2->GetOrigin();

	return true;
}

bool iANModalController::setModalities(QList<QSharedPointer<iAModality>> modalities) {
	if (!_checkModalities(modalities)) {
		return false;
	}
	m_modalities = modalities;
	return true;
}

void iANModalController::adjustTf(QSharedPointer<iAModality> modality, QList<LabeledVoxel> voxels) {

	double range[2];
	modality->image()->GetScalarRange(range);
	double min = range[0];
	double max = range[1];

	auto tf = modality->transfer();
	
	tf->colorTF()->RemoveAllPoints();
	tf->colorTF()->AddRGBPoint(min, 0.0, 0.0, 0.0);
	tf->colorTF()->AddRGBPoint(max, 0.0, 0.0, 0.0);

	tf->opacityTF()->RemoveAllPoints();
	tf->opacityTF()->AddPoint(min, 0.0);
	tf->opacityTF()->AddPoint(max, 0.0);

	for (int i = 0; i < voxels.size(); i++) {
		LabeledVoxel v = voxels[i];

		double opacity = v.remover ? 0.0 : 0.5;

		tf->colorTF()->AddRGBPoint(v.scalar, v.r, v.g, v.b);
		tf->opacityTF()->AddPoint(v.scalar, opacity);
	}
}