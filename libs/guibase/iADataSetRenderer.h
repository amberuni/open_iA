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
#pragma once

#include <iAguibase_export.h>

#include "iAAttributes.h"

#include <memory>

class iAAABB;
class iADataSet;
class iADataForDisplay;
class iAOutlineImpl;

class vtkProp3D;
class vtkRenderer;

//! abstract interface for a class for 3D rendering of a dataset (in an iARenderer)
class iAguibase_API iADataSetRenderer
{
public:
	static const QString Position;
	static const QString Orientation;
	static const QString OutlineColor;
	static const QString Pickable;
	static const QString Shading;
	static const QString AmbientLighting;
	static const QString DiffuseLighting;
	static const QString SpecularLighting;
	static const QString SpecularPower;

	//! Create a dataset renderer
	iADataSetRenderer(vtkRenderer* renderer, bool defaultVisibility);
	//! called when dataset renderer is removed from display and destroyed
	virtual ~iADataSetRenderer();
	//! Set visibility of dataset
	void setVisible(bool visible);
	//! Whether dataset is currently visible
	bool isVisible() const;
	
	//! Retrieves the list of attributes, merged with their current values as default values:
	iAAttributes attributesWithValues() const;
	//! Call to change the attributes of this renderer
	void setAttributes(QVariantMap const& values);

	//! convenience method for setting the pickable attribute
	void setPickable(bool pickable);
	//! whether dataset is currently set to be pickable
	bool isPickable() const;

	//! Set the visibility of the axis-aligned bounding box:
	void setBoundsVisible(bool visible);
	//! The coordinates of the axis-aligned bounding box (of the dataset, untransformed to any current position/orientation changes)
	virtual iAAABB bounds() = 0;

	//! Get Position of this dataset in scene
	virtual double const* position() const = 0;
	//! Get Orientation of this dataset in scene
	virtual double const* orientation() const =0;
	//! Set position of this dataset in scene
	virtual void setPosition(double pos[3]) = 0;
	//! Set orientation of this dataset in scene
	virtual void setOrientation(double ori[3]) = 0;

	virtual vtkProp3D* vtkProp() = 0;

	//! optional additional control widget needed for additional settings in the renderer
	//virtual QWidget* controlWidget();

	// optional additional features:
	/*
	
	// interactions:
	virtual void setMovable(bool movable);

	void setCuttingPlanes(vtkPlane* p1, vtkPlane* p2, vtkPlane* p3);
	void removeCuttingPlanes();
	*/

protected:
	void addAttribute(QString const& name, iAValueType valueType, QVariant defaultValue = 0.0,
		double min = std::numeric_limits<double>::lowest(), double max = std::numeric_limits<double>::max());

	//! needs to be called by derived classes whenever the bounds of the dataset change (position, orientation, ...)
	void updateOutlineTransform();

	vtkRenderer* m_renderer;
	QVariantMap m_attribValues;

private:
	//! @{ internal logic of showing/hiding dataset in renderer; called internally from setVisible; implement in derived classes
	virtual void showDataSet() = 0;
	virtual void hideDataSet() = 0;
	//! @}
	//! called when the attributes have changed; derive to apply such a change to renderer
	virtual void applyAttributes(QVariantMap const& values) = 0;

	iAAttributes m_attributes;
	std::shared_ptr<iAOutlineImpl> m_outline;
	bool m_visible;
};
