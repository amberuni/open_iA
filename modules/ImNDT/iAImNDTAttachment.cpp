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
#include "iAImNDTAttachment.h"

#include "iAVREnvironment.h"

// FeatureScout - 3D cylinder visualization
#include "iA3DCylinderObjectVis.h"
#include "iACsvConfig.h"

// qthelper
#include <iADockWidgetWrapper.h>

// core
#include <iAModality.h>
#include <iAModalityTransfer.h>
#include <iAVolumeRenderer.h>

#include <vtkFloatArray.h>
#include <vtkTable.h>

// must be after vtk includes, otherwise -> #error:  gl.h included before glew.h
#include <iAMdiChild.h>
#include <iAMainWindow.h>

#include <QPushButton>

iAImNDTAttachment::iAImNDTAttachment( iAMainWindow * mainWnd, iAMdiChild* child )
	: iAModuleAttachmentToChild( mainWnd, child ),
	m_vrEnv(new iAVREnvironment)
{
	m_toggleVR = new QPushButton("Start VR");
	iADockWidgetWrapper* vrDockWidget = new iADockWidgetWrapper(m_toggleVR, "VR", "vrDockWidget");
	connect(m_toggleVR, &QPushButton::clicked, this, &iAImNDTAttachment::toggleVR);
	child->splitDockWidget(child->renderDockWidget(), vrDockWidget, Qt::Horizontal);
}

void iAImNDTAttachment::toggleVR()
{
	if (m_vrEnv->isRunning())
	{
		m_vrEnv->stop();
		return;
	}
	m_toggleVR->setText("Stop VR");
	connect(m_vrEnv.data(), &iAVREnvironment::finished, this, &iAImNDTAttachment::vrDone);
	m_volumeRenderer = QSharedPointer<iAVolumeRenderer>::create(m_child->modality(0)->transfer().data(), m_child->modality(0)->image());
	m_volumeRenderer->applySettings(m_child->volumeSettings());
	m_volumeRenderer->addTo(m_vrEnv->renderer());
	m_volumeRenderer->addBoundingBoxTo(m_vrEnv->renderer());
	m_vrEnv->start();
}

void iAImNDTAttachment::vrDone()
{
	m_toggleVR->setText("Start VR");
}
