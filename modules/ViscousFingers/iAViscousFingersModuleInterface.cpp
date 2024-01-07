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
#include <QSplitter>
#include <QComboBox>
#include <QCheckBox>
#include <iAQVTKWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
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
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkReebGraph.h>
#include <vtkDepthSortPolyData.h>
#include <vtkGaussianSplatter.h>

std:: vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>> fileReaders;
std:: vector<vtkReebGraph> reeb_graphs;
vtkSmartPointer<vtkXMLUnstructuredGridReader> selectedReader;
QStringList vtuFiles;
bool pactor_set = false;
bool cactor_set = false;
bool velocitychnage_set = false;
bool velocityconc_set = false;

vtkSmartPointer<vtkActor> particle_actor_conc_only(
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);
vtkSmartPointer<vtkActor> particle_actor_conc_cyclinder(
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);
vtkSmartPointer<vtkActor> particle_actor_cyckinder_only();
vtkSmartPointer<vtkActor> particle_actor_conc_velocity(
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);

void create_single_reeb_graph(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader);
void create_multiple_reeb_graphs(
	std::vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>> fileReaders, std::vector<vtkReebGraph> reeb_graphs);

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

			" Always First Load the Data then Can call Data Visualize."
			"################################################################"
			"Viscous Finger Visualization Data Format:\n"
			"-----------------------------------------\n"
			"The 'Viscous Finger Visualization' format is used to represent simulation data "
			"for visualizing the behavior of viscous fingers in a fluid flow. The format typically "
			"consists of a collection of time-dependent data points that describe the position and "
			"properties of these fingers.\n"
			"\n"
			"Sample Data Format:\n"
			"------------------\n"
			"Each run now consists of one file per timestep, stored in VTK's .vtu format (see the documentation), "
			"along with an additional file named 'timesteps' listing all timesteps and their corresponding time values. "
			"For example:\n"
			"\n"
			"...\n"
			"004.vtu 2.12750005722\n"
			"005.vtu 2.65980005264\n"
			"006.vtu 3.14350008965\n"
			"007.vtu 3.55870008469\n"
			"008.vtu 4.09310007095\n"
			"009.vtu 4.52099990845\n"
			"010.vtu 5.07929992676\n"
			"...\n"
			"\n"
			"Each .vtu file contains the following information:\n"
			"- The 'points' array holds particle positions.\n"
			"- The 'velocity' point data array contains the flow velocity at the particle positions.\n"
			"- The 'concentration' point data array describes the concentration at the particle positions.\n"
			"\n"
			"As is typical in VTK, the 'velocity' and 'concentration' arrays' ith values correspond to the ith point "
			"described in the 'points' array.\n"
			"\n"
			"Additionally, there are three single-element arrays stored in the file's field data:\n"
			"- 'step' indicates the simulation timestep.\n"
			"- 'time' indicates the simulation time.\n"
			"- 'size' indicates the number of particles in this timestep.");
	dataFormatDisplay->setReadOnly(true);
	dataFormatDisplay->setFixedSize(720, 640);

	layout->addWidget(dataFormatDisplay);

	QPushButton* openFolderButton = new QPushButton("Load Folder");
	layout->addWidget(openFolderButton);

	connect(openFolderButton, &QPushButton::clicked, this, &iAViscousFingersModuleInterface::loadDataFromFolder);

	QPushButton* openvButton = new QPushButton("Visualize Data");
	layout->addWidget(openvButton);

	connect(openvButton, &QPushButton::clicked, this, &iAViscousFingersModuleInterface::interaction_window);

	if (subWindow.exec() == QDialog::Accepted)
	{
		// Handle any further actions here if needed
	}
}

void iAViscousFingersModuleInterface::loadDataFromFolder()
{
	QString directoryPath = QFileDialog::getExistingDirectory(m_mainWnd, "Select Directory Containing .vtu Files", "");

	if (!directoryPath.isEmpty())
	{
		QDir directory(directoryPath);
		vtuFiles = directory.entryList(QStringList() << "*.vtu", QDir::Files);

		if (!vtuFiles.isEmpty())
		{
			// Store the paths of the .vtu files in a QVector<QString> or any suitable data structure
			QVector<QString> vtuFilePaths;

			for (const QString& vtuFile : vtuFiles)
			{
				QString filePath = directory.absoluteFilePath(vtuFile);
				vtuFilePaths.push_back(filePath);
			}

			for (const QString& filePath : vtuFilePaths)
			{
				// Read .vtu file
				vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
					vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
				reader->SetFileName(filePath.toStdString().c_str());
				reader->Update();
				fileReaders.push_back(reader);
				
			}
			QMessageBox::information(m_mainWnd, "Files Loaded", "All vtu files loaded");
		}
		else
		{
			QMessageBox::warning(m_mainWnd, "No .vtu Files", "No .vtu files found in the selected directory.");
		}
	}
}

void iAViscousFingersModuleInterface::interaction_window()
{
	if (fileReaders.empty())
	{
		QMessageBox::warning(m_mainWnd, "No .vtu Files", "First Load data");
		return;
	}

	QDialog subWindow(m_mainWnd);
	subWindow.setWindowTitle("ViscousFingers");
	subWindow.setFixedSize(1024, 768);

	// Splitter to divide the window into two parts
	QSplitter* splitter = new QSplitter(&subWindow);

	// Visualization space (part 1)
	QWidget* Reeb_graph_Space = new QWidget();


	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
	renderer->SetActiveCamera(camera);
	camera->SetPosition(0, -25.0, 12.5);
	camera->SetFocalPoint(0, 0, 4.1);

	splitter->addWidget(Reeb_graph_Space);
	// Set the splitter's stretch factor
	splitter->setStretchFactor(0, 4);  // Visualization space takes 4/5 of the window
	splitter->setStretchFactor(1, 1);  // Control panel takes 1/5 of the window


	// Control panel (part 2)
	QWidget* controlPanel = new QWidget();
	QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

	// Add controls for file selection and visualization options
	QLabel* fileSelectionLabel = new QLabel("Visulzization Control Pannel");
	// QLabel for displaying the description text
	QLabel* descriptionLabel =
		new QLabel("This control panel allows you to select .vtu files and customize visualization options:");

	QLabel* descriptionLabel1 = new QLabel("1.Cyclinder Object View : Only Displays the Container Cyclinder.");
	QLabel* descriptionLabel2 = new QLabel("2.Concentration particle View : Displays the Concentration change only.");
	QLabel* descriptionLabel3 =
		new QLabel("3.Velocity Particle View : Only Displays the particles which chnages velocity.");
	QLabel* descriptionLabel4 = new QLabel(
		"4.Concentration and Cyclinder Particle View : Displays the Concentration and cyclinder in Single frame.");
	QLabel* descriptionLabel5 = new QLabel(" Once can also select more than one option for overlays");
	QLabel* descriptionLabel6 =
		new QLabel(" Interactive displays are allowed and mouse can be used for zoom, pan, tilts");

	QComboBox* fileComboBox = new QComboBox();

	// Populate fileComboBox with .vtu files from a selected folder
	fileComboBox->addItems(vtuFiles);

	// Checkbox to toggle the display of actors
	QCheckBox* showCylinderCheckBox = new QCheckBox("Cyclinder Object View");
	QCheckBox* showParticlesCheckBox = new QCheckBox("Concentration particle View");
	QCheckBox* showvelocitychnageCheckBox = new QCheckBox("Velocity Particle View");
	QCheckBox* showConcCheckBox = new QCheckBox("Concentration and Cyclinder Particle View");

	controlLayout->addWidget(fileSelectionLabel);
	controlLayout->addWidget(descriptionLabel);
	controlLayout->addWidget(descriptionLabel1);
	controlLayout->addWidget(descriptionLabel2);
	controlLayout->addWidget(descriptionLabel3);
	controlLayout->addWidget(descriptionLabel4);
	controlLayout->addWidget(descriptionLabel5);
	controlLayout->addWidget(descriptionLabel6);
	controlLayout->addWidget(fileComboBox);
	controlLayout->addWidget(showCylinderCheckBox);
	controlLayout->addWidget(showParticlesCheckBox);
	controlLayout->addWidget(showvelocitychnageCheckBox);
	controlLayout->addWidget(showConcCheckBox);

	splitter->addWidget(controlPanel);

	// Set the splitter's stretch factor
	splitter->setStretchFactor(0, 3);  // Visualization space takes 3/4 of the window
	splitter->setStretchFactor(1, 1);  // Control panel takes 1/4 of the window

	// Set the splitter as the main layout for the dialog
	QVBoxLayout* mainLayout = new QVBoxLayout(&subWindow);
	mainLayout->addWidget(splitter);

	QPushButton* Visualize = new QPushButton("Visualize Data");
	controlLayout->addWidget(Visualize);

	connect(Visualize, &QPushButton::clicked, this,
		[=]()
		{
			try
			{
				renderer->RemoveAllViewProps();  // Clear the renderer

				vtkSmartPointer<vtkActor> cylinderActor = particle_actor_cyckinder_only();
				vtkSmartPointer<vtkActor> particlesActor = particle_actor_conc_cyclinder(selectedReader, camera);
				vtkSmartPointer<vtkActor> particle_actor_conc = particle_actor_conc_only(selectedReader, camera);
				vtkSmartPointer<vtkActor> particle_actor_conc_vel =
					particle_actor_conc_velocity(selectedReader, camera);


				// Check if actors are valid before adding them
				if (cylinderActor && cactor_set)
					renderer->AddActor(cylinderActor);
				if (particlesActor && pactor_set)
					renderer->AddActor(particlesActor);
				if (particle_actor_conc && velocitychnage_set)
					renderer->AddActor(particle_actor_conc);
				if (particle_actor_conc_vel && velocityconc_set)
					renderer->AddActor(particle_actor_conc_vel);

				renderer->ResetCamera();
				
				vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
				renderWindow->AddRenderer(renderer);
				renderWindow->SetSize(800, 600);
				vtkSmartPointer<vtkRenderWindowInteractor> interactor =
					vtkSmartPointer<vtkRenderWindowInteractor>::New();
				interactor->SetRenderWindow(renderWindow);

				interactor->Initialize();
				interactor->Start();
			}
			catch (const std::exception& e)
			{
				// Log error messages or handle exceptions here
				qDebug() << "Exception occurred: " << e.what();
			}
		});
	// Set the file name for the reader based on the selected file
	connect(fileComboBox, QOverload<int>::of(&QComboBox::activated),
		[=](int index)
		{
			selectedReader = fileReaders[index];
		});

	// Connect slots to handle checkbox state changes
	connect(showCylinderCheckBox, &QCheckBox::stateChanged,
		[=](int state)
		{
			if (state == Qt::Checked)
			{
				cactor_set = true;
			}
			else
			{
				cactor_set = false;
			}
		});

	connect(showParticlesCheckBox, &QCheckBox::stateChanged,
		[=](int state)
		{
			if (state == Qt::Checked)
			{
				pactor_set = true;
			}
			else
			{
				pactor_set = false;
			}
		});
	connect(showvelocitychnageCheckBox, &QCheckBox::stateChanged,
		[=](int state)
		{
			if (state == Qt::Checked)
			{
				velocitychnage_set = true;
			}
			else
			{
				velocitychnage_set = false;
			}
		});
	connect(showConcCheckBox, &QCheckBox::stateChanged,
		[=](int state)
		{
			if (state == Qt::Checked)
			{
				velocityconc_set = true;
			}
			else
			{
				velocityconc_set = false;
			}
		});

	if (subWindow.exec() == QDialog::Accepted)
	{
		// Handle any further actions here if needed
	}
}

vtkSmartPointer<vtkActor> particle_actor_conc_only(
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera)
{

	vtkSmartPointer<vtkAssignAttribute> aa = vtkSmartPointer<vtkAssignAttribute>::New();
	aa->Assign("concentration", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
	aa->SetInputConnection(reader->GetOutputPort());
	vtkPointData* pointData = reader->GetOutput()->GetPointData();
	vtkDataArray* velocityArray = pointData->GetArray("velocity");

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

vtkSmartPointer<vtkActor> particle_actor_conc_cyclinder (
	vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera)
{
	vtkSmartPointer<vtkAssignAttribute> aa = vtkSmartPointer<vtkAssignAttribute>::New();
	aa->Assign("concentration", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
	aa->SetInputConnection(reader->GetOutputPort());
	vtkPointData* pointData = reader->GetOutput()->GetPointData();
	vtkDataArray* velocityArray = pointData->GetArray("velocity");

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


vtkSmartPointer<vtkActor> particle_actor_cyckinder_only()
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

vtkSmartPointer<vtkActor> particle_actor_conc_velocity(
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
