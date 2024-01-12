#include "Particleactors.h"

// Qt includes
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

// VTK includes
#include <vtkActor.h>
#include <vtkAssignAttribute.h>
#include <vtkCamera.h>
#include <vtkCylinderSource.h>
#include <vtkDataSetAttributes.h>
#include <vtkDataSetMapper.h>
#include <vtkDepthSortPolyData.h>
#include <vtkGeometryFilter.h>
#include <vtkLookupTable.h>
#include <vtkMaskPoints.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkThreshold.h>
#include <vtkTransform.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>

// Constructor for the ParticleActors class
ParticleActors::ParticleActors()
{
}

/**
 * @brief Function to create an actor for concentration view.
 *
 * This function generates a VTK actor for visualizing concentration in the fluid flow.
 * It assigns scalar attribute 'concentration' to the points of the dataset and applies
 * various VTK filters for masking, thresholding, geometry extraction, sorting, and coloring.
 * The resulting actor is returned for visualization.
 *
 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
 * @param camera The VTK camera used for rendering.
 * @return A VTK actor representing the concentration view.
 */
vtkSmartPointer<vtkActor> ParticleActors::Conccenration_view(
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera)
{
	// Assign scalar attribute 'concentration' to the points of the dataset
	vtkSmartPointer<vtkAssignAttribute> aa = vtkSmartPointer<vtkAssignAttribute>::New();
	aa->Assign("concentration", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
	aa->SetInputConnection(reader->GetOutputPort());

	// Mask points based on a ratio to reduce the number of points
	vtkSmartPointer<vtkMaskPoints> mask = vtkSmartPointer<vtkMaskPoints>::New();
	mask->SetOnRatio(1);
	mask->SetInputConnection(aa->GetOutputPort());
	mask->GenerateVerticesOn();
	mask->SingleVertexPerCellOn();

	// Threshold points based on the 'velocity' array
	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "velocity");
	threshold->SetLowerThreshold(0.3);
	threshold->SetInputConnection(mask->GetOutputPort());

	// Extract geometry from the thresholded points
	vtkSmartPointer<vtkGeometryFilter> geom = vtkSmartPointer<vtkGeometryFilter>::New();
	geom->SetInputConnection(threshold->GetOutputPort());

	// Sort geometry based on depth for correct rendering
	vtkSmartPointer<vtkDepthSortPolyData> sort = vtkSmartPointer<vtkDepthSortPolyData>::New();
	sort->SetInputConnection(geom->GetOutputPort());
	sort->SetDirectionToBackToFront();
	sort->SetCamera(camera);

	// Create a lookup table for coloring
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256);
	lut->SetHueRange(0, 4.0 / 6.0);
	lut->Build();

	// Map the sorted data to colors and scalars
	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInputConnection(sort->GetOutputPort());
	mapper->SetLookupTable(lut);
	mapper->SetScalarRange(50, 250);
	mapper->SetScalarModeToUsePointData();
	mapper->ScalarVisibilityOn();

	// Create an actor for the mapped data
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// Set actor properties
	vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
	prop->SetPointSize(1.5);
	prop->SetOpacity(0.7);

	return actor;
}

/**
 * @brief Function to create an actor for concentration cylinder view.
 *
 * This function generates a VTK actor for visualizing concentration in the fluid flow
 * within a cylindrical object. It applies various VTK filters for masking, geometry
 * extraction, sorting, and coloring. The resulting actor is returned for visualization.
 *
 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
 * @param camera The VTK camera used for rendering.
 * @return A VTK actor representing the concentration cylinder view.
 */
vtkSmartPointer<vtkActor> ParticleActors::Conccenration_cyclinder_view(
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera)
{
	// Assign scalar attribute 'concentration' to the points of the dataset
	vtkSmartPointer<vtkAssignAttribute> aa = vtkSmartPointer<vtkAssignAttribute>::New();
	aa->Assign("concentration", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
	aa->SetInputConnection(reader->GetOutputPort());

	// Mask points based on a ratio to reduce the number of points
	vtkSmartPointer<vtkMaskPoints> mask = vtkSmartPointer<vtkMaskPoints>::New();
	mask->SetOnRatio(1);
	mask->SetInputConnection(aa->GetOutputPort());
	mask->GenerateVerticesOn();
	mask->SingleVertexPerCellOn();

	// Extract geometry from the masked points
	vtkSmartPointer<vtkGeometryFilter> geom = vtkSmartPointer<vtkGeometryFilter>::New();
	geom->SetInputConnection(mask->GetOutputPort());

	// Sort geometry based on depth for correct rendering
	vtkSmartPointer<vtkDepthSortPolyData> sort = vtkSmartPointer<vtkDepthSortPolyData>::New();
	sort->SetInputConnection(geom->GetOutputPort());
	sort->SetDirectionToBackToFront();
	sort->SetCamera(camera);

	// Create a lookup table for coloring
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256);
	lut->SetHueRange(0, 4.0 / 6.0);
	lut->Build();

	// Map the sorted data to colors and scalars
	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInputConnection(sort->GetOutputPort());
	mapper->SetLookupTable(lut);
	mapper->SetScalarRange(50, 250);
	mapper->SetScalarModeToUsePointData();
	mapper->ScalarVisibilityOn();

	// Create an actor for the mapped data
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// Set actor properties
	vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
	prop->SetPointSize(1.5);
	prop->SetOpacity(0.7);

	return actor;
}

/**
 * @brief Function to create an actor for a cylinder view.
 *
 * This function generates a VTK actor for visualizing a cylindrical object. It utilizes
 * the VTK cylinder source and applies transformations to the actor for proper rendering.
 * The resulting actor is returned for visualization.
 *
 * @return A VTK actor representing the cylinder view.
 */
vtkSmartPointer<vtkActor> ParticleActors::Cylinder_view()
	{
	// Create a cylinder source
	vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
	cylinder->SetCenter(0, 0, 0);
	cylinder->SetHeight(10);
	cylinder->SetRadius(5);
	cylinder->SetResolution(100);

	// Map the cylinder source to actor
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(cylinder->GetOutputPort());

	// Create an actor for the mapped cylinder data
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// Apply transformations to the actor
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->PostMultiply();
	transform->RotateX(90.0);
	transform->Translate(0, 0, 5);

	actor->SetUserTransform(transform);

	// Set actor properties
	vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
	prop->SetOpacity(0.2);

	return actor;
}

/**
 * @brief Function to create an actor for velocity view.
 *
 * This function generates a VTK actor for visualizing particles with varying velocity.
 * It uses the 'velocity' array for thresholding and applies various VTK filters for masking,
 * thresholding, geometry extraction, sorting, and coloring. The resulting actor is returned
 * for visualization.
 *
 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
 * @param camera The VTK camera used for rendering.
 * @return A VTK actor representing the velocity view.
 */
vtkSmartPointer<vtkActor> ParticleActors::velocity_view(
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera)
{
	// Fetch point data arrays
	vtkPointData* pointData = reader->GetOutput()->GetPointData();
	vtkDataArray* velocityArray = pointData->GetArray("velocity");
	vtkDataArray* concentrationArray = pointData->GetArray("concentration");
	vtkDataArray* pointsArray = reader->GetOutput()->GetPoints()->GetData();

	// Check if required arrays are present
	if (!velocityArray || !concentrationArray || !pointsArray)
	{
		qDebug() << "Required arrays not found in the dataset.";
		return nullptr;  // Return null actor if required arrays are not present
	}

	// Create a mask to filter particles based on velocity magnitude
	vtkSmartPointer<vtkMaskPoints> mask = vtkSmartPointer<vtkMaskPoints>::New();
	mask->SetOnRatio(1);
	mask->SetInputConnection(reader->GetOutputPort());
	mask->GenerateVerticesOn();
	mask->SingleVertexPerCellOn();

	// Threshold points based on the 'velocity' array
	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "velocity");
	threshold->SetLowerThreshold(0.3);

	// Connect mask to threshold
	threshold->SetInputConnection(mask->GetOutputPort());

	// Extract geometry from the thresholded points
	vtkSmartPointer<vtkGeometryFilter> geom = vtkSmartPointer<vtkGeometryFilter>::New();
	geom->SetInputConnection(threshold->GetOutputPort());

	// Create a lookup table for coloring
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256);
	lut->SetHueRange(0, 4.0 / 6.0);
	lut->Build();

	// Map the geometry to colors and scalars
	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInputConnection(geom->GetOutputPort());
	mapper->SetLookupTable(lut);
	mapper->SetScalarRange(50, 250);
	mapper->SetScalarModeToUsePointData();
	mapper->ScalarVisibilityOn();

	// Create an actor for the mapped data
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// Set actor properties
	vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
	prop->SetPointSize(1.5);
	prop->SetOpacity(0.7);

	return actor;
}