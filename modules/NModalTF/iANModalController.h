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
#pragma once

#include <vtkSmartPointer.h>

#include <QString>
#include <QList>
#include <QSharedPointer>

class iAModality;
class iASlicer;
class dlg_labels;
class MdiChild;

class vtkVolume;
class vtkSmartVolumeMapper;
class vtkRenderer;
class vtkImageData;

struct LabeledVoxel {
	int x;
	int y;
	int z;
	int id;
	double scalar;
	double r;
	double g;
	double b;
	bool remover = false;
	QString text() {
		return QString::number(x) + "," + QString::number(y) + "," + QString::number(z) + "," + QString::number(scalar) + "," + QString::number(r) + "," + QString::number(g) + "," + QString::number(b);
	}
};

class iANModalController : public QObject {
	Q_OBJECT

	friend class iANModalWidget;

public:
	iANModalController(MdiChild *mdiChild);
	void initialize();

	int countModalities();
	QList<QSharedPointer<iAModality>> cherryPickModalities(QList<QSharedPointer<iAModality>> modalities);
	bool setModalities(QList<QSharedPointer<iAModality>> modalities);
	void reinitialize();

	// TEMPORARY STUFF
	void adjustTf(QSharedPointer<iAModality> modality, QList<LabeledVoxel> voxels);

private:
	MdiChild *m_mdiChild;

	void _initialize();
	iASlicer* _initializeSlicer(QSharedPointer<iAModality> modality);
	void _initializeCombinedVol();
	void _initializeMainSlicers();
	bool _checkModalities(QList<QSharedPointer<iAModality>> modalities);
	bool _matchModalities(QSharedPointer<iAModality> m1, QSharedPointer<iAModality> m2);
	QList<QSharedPointer<iAModality>> m_modalities;
	QList<uint> m_channelIds;
	vtkSmartPointer<vtkVolume> m_combinedVol;
	vtkSmartPointer<vtkSmartVolumeMapper> m_combinedVolMapper;
	vtkSmartPointer<vtkRenderer> m_combinedVolRenderer;
	vtkSmartPointer<vtkImageData> m_slicerImages[3];
	bool m_initialized = false;

	uint m_slicerChannel_main = 0;
	uint m_slicerChannel_label = 1;
	uint m_mainSlicerChannel_nModal;

	void applyVolumeSettings();

	// Internal widgets
	QList<iASlicer*> m_slicers;
	QList<int> m_labelIds;

	// Labeling widgets


	// MdiChild widgets
	dlg_labels *m_dlg_labels;

signals:
	void allSlicersInitialized();
	void allSlicersReinitialized();

private slots:
	void onHistogramAvailable();
};