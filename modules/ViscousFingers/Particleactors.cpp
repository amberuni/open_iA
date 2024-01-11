#include "Particleactors.h"

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QSplitter>
#include <QComboBox>
#include <QCheckBox>

#include <vtkAssignAttribute.h>
#include <vtkGeometryFilter.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkCamera.h>
#include <vtkDepthSortPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>

#include <vtkAssignAttribute.h>
#include <vtkMaskPoints.h>
#include <vtkThreshold.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkActor.h>
#include <vtkGeometryFilter.h>
#include <vtkDataSetMapper.h>
#include <vtkTransform.h>
#include <vtkRenderer.h>

ParticleActors::ParticleActors()
{}


vtkSmartPointer<vtkActor> ParticleActors::Conccenration_view(
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera)
{
	vtkSmartPointer<vtkAssignAttribute> aa = vtkSmartPointer<vtkAssignAttribute>::New();
	aa->Assign("concentration", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
	aa->SetInputConnection(reader->GetOutputPort());

	vtkSmartPointer<vtkMaskPoints> mask = vtkSmartPointer<vtkMaskPoints>::New();
	mask->SetOnRatio(1);
	mask->SetInputConnection(aa->GetOutputPort());
	mask->GenerateVerticesOn();
	mask->SingleVertexPerCellOn();

	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "velocity");
	threshold->SetLowerThreshold(0.3);

	// Connect mask to threshold
	threshold->SetInputConnection(mask->GetOutputPort());

	vtkSmartPointer<vtkGeometryFilter> geom = vtkSmartPointer<vtkGeometryFilter>::New();
	geom->SetInputConnection(threshold->GetOutputPort());

	// Create a vtkDepthSortPolyData object
	vtkSmartPointer<vtkDepthSortPolyData> sort = vtkSmartPointer<vtkDepthSortPolyData>::New();
	// Set input connection to geom's output port
	sort->SetInputConnection(geom->GetOutputPort());
	// Set the sorting direction to back to front
	sort->SetDirectionToBackToFront();
	// Set the camera for depth sorting
	sort->SetCamera(camera);
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256);
	lut->SetHueRange(0, 4.0 / 6.0);
	lut->Build();

	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInputConnection(geom->GetOutputPort());
	mapper->SetLookupTable(lut);
	mapper->SetScalarRange(50, 250);
	mapper->SetScalarModeToUsePointData();
	mapper->ScalarVisibilityOn();

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
	prop->SetPointSize(1.5);
	prop->SetOpacity(0.7);

	// Create a cylinder source
	vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
	cylinder->SetHeight(5.0);  // Set the height of the cylinder
	cylinder->SetRadius(2.0);  // Set the radius of the cylinder

	// Create a mapper and actor for the cylinder
	vtkSmartPointer<vtkPolyDataMapper> cylinderMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

	vtkSmartPointer<vtkActor> cylinderActor = vtkSmartPointer<vtkActor>::New();
	cylinderActor->SetMapper(cylinderMapper);

	// Set the transparency (opacity) of the cylinder
	vtkSmartPointer<vtkProperty> cylinderProp = cylinderActor->GetProperty();
	cylinderProp->SetOpacity(0.0);  // Adjust the opacity as needed (0.0 fully transparent, 1.0 fully opaque)

	return actor;
	}

vtkSmartPointer<vtkActor> ParticleActors::Conccenration_cyclinder_view(
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera)
	{
	vtkSmartPointer<vtkAssignAttribute> aa = vtkSmartPointer<vtkAssignAttribute>::New();
	aa->Assign("concentration", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
	aa->SetInputConnection(reader->GetOutputPort());

	vtkSmartPointer<vtkMaskPoints> mask = vtkSmartPointer<vtkMaskPoints>::New();
	mask->SetOnRatio(1);
	mask->SetInputConnection(aa->GetOutputPort());
	mask->GenerateVerticesOn();
	mask->SingleVertexPerCellOn();

	vtkSmartPointer<vtkGeometryFilter> geom = vtkSmartPointer<vtkGeometryFilter>::New();
	geom->SetInputConnection(mask->GetOutputPort());

	// Create a vtkDepthSortPolyData object
	vtkSmartPointer<vtkDepthSortPolyData> sort = vtkSmartPointer<vtkDepthSortPolyData>::New();
	sort->SetInputConnection(geom->GetOutputPort());
	sort->SetDirectionToBackToFront();
	sort->SetCamera(camera);

	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256);
	lut->SetHueRange(0, 4.0 / 6.0);
	lut->Build();

	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInputConnection(geom->GetOutputPort());
	mapper->SetLookupTable(lut);
	mapper->SetScalarRange(50, 250);
	mapper->SetScalarModeToUsePointData();
	mapper->ScalarVisibilityOn();

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
	prop->SetPointSize(1.5);
	prop->SetOpacity(0.7);

	// Create a cylinder source
	vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
	cylinder->SetHeight(5.0);  // Set the height of the cylinder
	cylinder->SetRadius(2.0);  // Set the radius of the cylinder

	// Create a mapper and actor for the cylinder
	vtkSmartPointer<vtkPolyDataMapper> cylinderMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

	vtkSmartPointer<vtkActor> cylinderActor = vtkSmartPointer<vtkActor>::New();
	cylinderActor->SetMapper(cylinderMapper);

	// Set the transparency (opacity) of the cylinder
	vtkSmartPointer<vtkProperty> cylinderProp = cylinderActor->GetProperty();
	cylinderProp->SetOpacity(0.0);  // Adjust the opacity as needed (0.0 fully transparent, 1.0 fully opaque)

	return actor;
	}

vtkSmartPointer<vtkActor> ParticleActors::Cylinder_view()
	{
	vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
	cylinder->SetCenter(0, 0, 0);
	cylinder->SetHeight(10);
	cylinder->SetRadius(5);
	cylinder->SetResolution(100);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(cylinder->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->PostMultiply();
	transform->RotateX(90.0);
	transform->Translate(0, 0, 5);

	actor->SetUserTransform(transform);

	vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
	prop->SetOpacity(0.2);

	return actor;
	}

vtkSmartPointer<vtkActor> ParticleActors::velocity_view(
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera)
	{
	// Fetch point data arrays
	vtkPointData* pointData = reader->GetOutput()->GetPointData();
	vtkDataArray* velocityArray = pointData->GetArray("velocity");
	vtkDataArray* concentrationArray = pointData->GetArray("concentration");
	vtkDataArray* pointsArray = reader->GetOutput()->GetPoints()->GetData();

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

	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "velocity");
	threshold->SetLowerThreshold(0.3);

	// Connect mask to threshold
	threshold->SetInputConnection(mask->GetOutputPort());

	vtkSmartPointer<vtkGeometryFilter> geom = vtkSmartPointer<vtkGeometryFilter>::New();
	geom->SetInputConnection(threshold->GetOutputPort());

	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256);
	lut->SetHueRange(0, 4.0 / 6.0);
	lut->Build();

	// Create mapper and actor for the filtered particles
	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInputConnection(geom->GetOutputPort());
	mapper->SetLookupTable(lut);
	mapper->SetScalarRange(50, 250);
	mapper->SetScalarModeToUsePointData();
	mapper->ScalarVisibilityOn();

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
	prop->SetPointSize(1.5);
	prop->SetOpacity(0.7);

	return actor;
	}