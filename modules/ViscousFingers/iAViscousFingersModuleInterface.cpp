#include "iAViscousFingersModuleInterface.h"

#include "iAMainWindow.h"

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextEdit>

#include <vtkAssignAttribute.h>
#include <vtkMaskPoints.h>
#include <vtkThreshold.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkActor.h>
#include <vtkGeometryFilter.h>
#include <vtkDataSetMapper.h>
#include <vtkTransform.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>


vtkSmartPointer<vtkActor> createParticlesActor(
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader);
vtkSmartPointer<vtkActor> createCylinderActor();

void iAViscousFingersModuleInterface::Initialize()
{
	if (!m_mainWnd)  // if m_mainWnd is not set, we are running in command line mode
	{
		return;  // in that case, we do not do anything as we cannot add a menu entry there
	}

	QAction* loadDataAction = new QAction(tr("ViscousFingers"), m_mainWnd);
	connect(loadDataAction, &QAction::triggered, this, &iAViscousFingersModuleInterface::openSubWindow);
	addToMenuSorted(m_mainWnd->toolsMenu(), loadDataAction);
}

void iAViscousFingersModuleInterface::openSubWindow()
{
	QDialog subWindow(m_mainWnd);
	subWindow.setWindowTitle("ViscousFingers");
	subWindow.setFixedSize(640, 480);

	QLabel* titleLabel = new QLabel("Load Viscous Fingers Data");
	titleLabel->setFont(QFont("Arial", 14, QFont::Bold));

	QLabel* instructionLabel = new QLabel("Data Format: .vtu");

	QVBoxLayout* layout = new QVBoxLayout(&subWindow);
	layout->addWidget(titleLabel);
	layout->addWidget(instructionLabel);

	QTextEdit* dataFormatDisplay = new QTextEdit(
		"Viscous Finger Visualization Data Format:\n"
		"-----------------------------------------\n"
		"The 'Viscous Finger Visualization' format is used to represent simulation data "
		"for visualizing the behavior of viscous fingers in a fluid flow. The format typically "
		"consists of a collection of time-dependent data points that describe the position and "
		"properties of these fingers.\n"
		"\n"
		"Sample Data Format:\n"
		"------------------\n"
		"Each data file follows a structured format. Here is an example of the format:\n"
		"\n"
		"Time (s)  | X-Coordinate (m)  | Y-Coordinate (m)  | Viscosity (Pa·s)\n"
		"---------------------------------------------------------------\n"
		"0.0       | 0.0               | 0.0               | 1.5\n"
		"0.1       | 0.1               | 0.5               | 1.6\n"
		"0.2       | 0.2               | 1.0               | 1.8\n"
		"...\n"
		"\n"
		"The data format includes time, X and Y coordinates, and viscosity for each time step. "
		"You should ensure that your data files adhere to this structure to enable accurate visualization.");
	dataFormatDisplay->setReadOnly(true);
	dataFormatDisplay->setFixedSize(500, 480);

	layout->addWidget(dataFormatDisplay);

	QPushButton* openFileButton = new QPushButton("Open File");
	layout->addWidget(openFileButton);

	connect(openFileButton, &QPushButton::clicked, this, &iAViscousFingersModuleInterface::loadDataFromSubWindow);

	if (subWindow.exec() == QDialog::Accepted)
	{
		// Handle any further actions here if needed
	}
}

void iAViscousFingersModuleInterface::loadDataFromSubWindow()
{
	QString filePath = QFileDialog::getOpenFileName(m_mainWnd, "Open .vtu File", "", "VTU Files (*.vtu)");
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(0, -25.0, 12.5);
	camera->SetFocalPoint(0, 0, 4.1);


	if (!filePath.isEmpty())
	{
		if (filePath.endsWith(".vtu", Qt::CaseInsensitive))
		{
			// Load the data from the selected .vtu file
			// You can implement your data loading logic here
			QMessageBox::information(m_mainWnd, "Data Loaded", "Data loaded from file: " + filePath);
			vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
			reader->SetFileName(filePath.toStdString().c_str());
			reader->Update();  // Perform the actual data loading

			// Create actors using loaded data (assuming functions createCylinderActor and createParticlesActor are defined)
			vtkSmartPointer<vtkActor> cylinderActor = createCylinderActor();
			vtkSmartPointer<vtkActor> particlesActor = createParticlesActor(reader);  // Pass the loaded data to the function
			// Add these actors to the renderer
			if (renderer)  // Assuming 'renderer' is accessible here
			{
				renderer->AddActor(cylinderActor);
				renderer->AddActor(particlesActor);
				renderer->ResetCamera();  // Reset camera view for the new actors

				 // Create a render window
				vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
				renderWindow->AddRenderer(renderer);

               // Create a VTK interactor
				vtkSmartPointer<vtkRenderWindowInteractor> interactor =
					vtkSmartPointer<vtkRenderWindowInteractor>::New();
				interactor->SetRenderWindow(renderWindow);

				interactor->Initialize();
				interactor->Start();
			}
		}
		else
		{
			QMessageBox::critical(m_mainWnd, "Invalid File", "Please select a .vtu file.");
		}
	}
}

vtkSmartPointer<vtkActor> createParticlesActor(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader)
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

	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetNumberOfColors(256);
	lut->SetHueRange(0, 4.0 / 6.0);
	lut->Build();

	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInputConnection(geom->GetOutputPort());
	mapper->SetLookupTable(lut);
	mapper->SetScalarRange(50, 350);
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
	// You might need to set more properties of the cylinder source

	// Create a mapper and actor for the cylinder
	vtkSmartPointer<vtkPolyDataMapper> cylinderMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

	vtkSmartPointer<vtkActor> cylinderActor = vtkSmartPointer<vtkActor>::New();
	cylinderActor->SetMapper(cylinderMapper);

	// Set the transparency (opacity) of the cylinder
	vtkSmartPointer<vtkProperty> cylinderProp = cylinderActor->GetProperty();
	cylinderProp->SetOpacity(0.3);  // Adjust the opacity as needed (0.0 fully transparent, 1.0 fully opaque)

	return actor;
}



vtkSmartPointer<vtkActor> createCylinderActor()
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
