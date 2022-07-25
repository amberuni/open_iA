/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2020  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
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

#include "iAVRCubicVis.h"
#include "iACsvIO.h"

#include <vtkTable.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkLookupTable.h>


class vtkPointData;
class iA3DColoredPolyObjectVis;
class iA3DPolyObjectActor;

class QStandardItem;

//! Class which represents the rendered volume
class iAVRObjectModel: public iAVRCubicVis
{
public:
	iAVRObjectModel(vtkRenderer* ren, iA3DColoredPolyObjectVis* polyObject, vtkTable* objectTable, iACsvIO io, iACsvConfig csvConfig);
	void resetVolume();
	void showVolume();
	void hideVolume();
	void createCubeModel() override;
	void showRegionLinks();
	void hideRegionLinks();
	vtkSmartPointer<vtkActor> getVolumeActor();
	double* getCubePos(int region);
	double getCubeSize(int region);
	void setNodeColor(std::vector<vtkIdType> regions, std::vector<QColor> color);
	void resetNodeColor();
	iA3DColoredPolyObjectVis* getPolyObject();
	void renderSelection(std::vector<size_t> const& sortedSelInds, int classID, QColor const& classColor, QStandardItem* activeClassItem);
	void moveFibersByMaxCoverage(std::vector<std::vector<std::vector<vtkIdType>>>* m_maxCoverage, double offset, bool relativMovement);
	void moveFibersbyAllCoveredRegions(double offset, bool relativMovement);
	void moveFibersbyOctant(std::vector<std::vector<std::vector<vtkIdType>>>* m_maxCoverage, double offset);
	
	void createSimilarityNetwork(std::vector<std::vector<std::vector<double>>>* similarityMetric, double maxFibersInRegions, double worldSize);
	
	void filterRegionLinks(int sign);
	double getJaccardFilterVal();

private:
	vtkSmartPointer<vtkActor> m_volumeActor;
	vtkSmartPointer<vtkActor> m_RegionLinksActor;
	vtkSmartPointer<vtkActor> m_RegionNodesActor;
	iA3DColoredPolyObjectVis* m_polyObject;
	vtkSmartPointer<vtkPoints> m_initialPoints;
	//QSharedPointer<> m_PolyObjectActor;
	vtkSmartPointer<vtkTable> m_objectTable;
	vtkSmartPointer<vtkPolyData> m_linePolyData;
	iACsvIO m_io;
	iACsvConfig m_csvConfig;
	vtkSmartPointer<vtkLookupTable> m_lut;
	vtkSmartPointer<vtkDoubleArray> nodeGlyphScales;
	vtkSmartPointer<vtkUnsignedCharArray> linkGlyphColor;
	vtkSmartPointer<vtkUnsignedCharArray> nodeGlyphColor;
	vtkSmartPointer<vtkUnsignedCharArray> nodeGlyphResetColor;
	vtkSmartPointer<vtkGlyph3D> nodeGlyph3D;
	bool m_volumeVisible;
	bool m_regionLinksVisible;
	double m_regionLinkDrawRadius;

	void createRegionLinks(std::vector<std::vector<std::vector<double>>>* similarityMetric, double worldSize);
	void createRegionNodes(double maxFibersInRegions, double worldSize);
	void calculateNodeLUT(double min, double max, int colorScheme);
};