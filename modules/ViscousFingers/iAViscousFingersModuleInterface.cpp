#include "iAViscousFingersModuleInterface.h"
#include "Particleactors.h"
#include "ViscousFingersReebGraph.h"

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
#include <vtkUnstructuredGridToReebGraphFilter.h>

// Store all the read .vtu files in the vector.
std:: vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>> fileReaders;

// Global variables to be used with Qselect for selecting the .vtu files.
vtkSmartPointer<vtkXMLUnstructuredGridReader> selectedReader;
QStringList vtuFiles;

// Global flags to select the right views from Qbox select and unselect.
bool pactor_set = false;
bool cactor_set = false;
bool velocity_set = false;
bool conc_set = false;

/**
 * @brief The iAViscousFingersModuleInterface class represents the interface for the Viscous Fingers module.
 *
 * This class is responsible for initializing and integrating the Viscous Fingers module
 * with the application's main window and menu system.
 */
void iAViscousFingersModuleInterface::Initialize()
{
	// Check if the main window is set; if not, we are running in command line mode
	if (!m_mainWnd)
	{
		// In command line mode, we cannot add a menu entry, so we do nothing
		return;
	}

	// Create a QAction for loading ViscousFingers data
	QAction* loadDataAction = new QAction(tr("ViscousFingers"), m_mainWnd);

	// Connect the trigger signal of the QAction to the openSubWindow slot of this class
	connect(loadDataAction, &QAction::triggered, this, &iAViscousFingersModuleInterface::openSubWindow);

	// Add the QAction to the tools menu of the main window, sorted appropriately
	addToMenuSorted(m_mainWnd->toolsMenu(), loadDataAction);
}

/**
 * @brief The iAViscousFingersModuleInterface class represents the interface for the Viscous Fingers module.
 *
 * This class is responsible for initializing and integrating the Viscous Fingers module
 * with the application's main window and menu system.
 */
void iAViscousFingersModuleInterface::openSubWindow()
{
	// Create a modal dialog as a subwindow with the main window as its parent
	QDialog subWindow(m_mainWnd);
	subWindow.setWindowTitle("ViscousFingers");
	subWindow.setFixedSize(640, 480);

	// Create QLabel for title and set its properties
	QLabel* titleLabel = new QLabel("Load Viscous Fingers Data");
	titleLabel->setFont(QFont("Arial", 14, QFont::Bold));

	// Create QLabel for instruction
	QLabel* instructionLabel = new QLabel("Data Format: .vtu");

	// Create QVBoxLayout for the layout of the subwindow
	QVBoxLayout* layout = new QVBoxLayout(&subWindow);
	layout->addWidget(titleLabel);
	layout->addWidget(instructionLabel);

	// Create QTextEdit to display the data format information
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

	// Add the QTextEdit to the layout
	layout->addWidget(dataFormatDisplay);

	// Create QPushButton for loading data from a folder
	QPushButton* openFolderButton = new QPushButton("Load Folder");
	layout->addWidget(openFolderButton);

	// Connect the button click signal to the loadDataFromFolder slot of this class
	connect(openFolderButton, &QPushButton::clicked, this, &iAViscousFingersModuleInterface::loadDataFromFolder);

	// Create QPushButton for visualizing data
	QPushButton* openvButton = new QPushButton("Visualize Data");
	layout->addWidget(openvButton);

	// Connect the button click signal to the interaction_window slot of this class
	connect(openvButton, &QPushButton::clicked, this, &iAViscousFingersModuleInterface::interaction_window);

	// Execute the subwindow as a modal dialog, handling further actions if accepted
	if (subWindow.exec() == QDialog::Accepted)
	{
		// Handle any further actions here if needed
	}
}

/**
 * @brief The iAViscousFingersModuleInterface class represents the interface for the Viscous Fingers module.
 *
 * This class is responsible for initializing and integrating the Viscous Fingers module
 * with the application's main window and menu system.
 */
void iAViscousFingersModuleInterface::loadDataFromFolder()
{
	// Open a dialog to select a directory containing .vtu files
	QString directoryPath = QFileDialog::getExistingDirectory(m_mainWnd, "Select Directory Containing .vtu Files", "");

	// Check if a directory was selected
	if (!directoryPath.isEmpty())
	{
		// Create a QDir object for the selected directory
		QDir directory(directoryPath);

		// Get a list of .vtu files in the selected directory
		vtuFiles = directory.entryList(QStringList() << "*.vtu", QDir::Files);

		// Check if any .vtu files were found
		if (!vtuFiles.isEmpty())
		{
			// Store the paths of the .vtu files in a QVector<QString> or any suitable data structure
			QVector<QString> vtuFilePaths;

			// Iterate through the list of .vtu files and store their absolute paths
			for (const QString& vtuFile : vtuFiles)
			{
				QString filePath = directory.absoluteFilePath(vtuFile);
				vtuFilePaths.push_back(filePath);
			}

			// Iterate through the stored file paths and read each .vtu file
			for (const QString& filePath : vtuFilePaths)
			{
				// Read .vtu file using VTK library
				vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
					vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
				reader->SetFileName(filePath.toStdString().c_str());
				reader->Update();
				fileReaders.push_back(reader);
			}

			// Set the selected reader to the first one in the list ad default
			selectedReader = fileReaders[0];

			// Show information message indicating successful loading of .vtu files
			QMessageBox::information(m_mainWnd, "Files Loaded", "All .vtu files loaded");
		}
		else
		{
			// Show warning message if no .vtu files were found in the selected directory
			QMessageBox::warning(m_mainWnd, "No .vtu Files", "No .vtu files found in the selected directory.");
		}
	}
}

/**
 * @brief The iAViscousFingersModuleInterface class represents the interface for the Viscous Fingers module.
 *
 * This class is responsible for initializing and integrating the Viscous Fingers module
 * with the application's main window and menu system.
 */
void iAViscousFingersModuleInterface::interaction_window()
{
	// Check if no .vtu files have been loaded
	if (fileReaders.empty())
	{
		// Display a warning message if no .vtu files are loaded
		QMessageBox::warning(m_mainWnd, "No .vtu Files", "First Load data");
		return;
	}

	// Create a modal dialog as a subwindow with the main window as its parent
	QDialog subWindow(m_mainWnd);
	subWindow.setWindowTitle("ViscousFingers");
	subWindow.setFixedSize(1024, 768);

	// Create a QSplitter to divide the window into two parts
	QSplitter* splitter = new QSplitter(&subWindow);

	// Visualization space (part 1)
	QWidget* Reeb_graph_Space = new QWidget();

	// Create VTK renderer and camera
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
	renderer->SetActiveCamera(camera);
	camera->SetPosition(0, -25.0, 12.5);
	camera->SetFocalPoint(0, 0, 4.1);

	splitter->addWidget(Reeb_graph_Space);

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
	controlLayout->addWidget(showConcCheckBox);

    splitter->addWidget(controlPanel);
	splitter->setStretchFactor(0, 3);  // Visualization space takes 3/4 of the window
	splitter->setStretchFactor(1, 1);  // Control panel takes 1/4 of the window

	// Set the splitter as the main layout for the dialog
	QVBoxLayout* mainLayout = new QVBoxLayout(&subWindow);
	mainLayout->addWidget(splitter);

	QPushButton* Visualize = new QPushButton("Visualize Data");
	controlLayout->addWidget(Visualize);

	QPushButton* reeb_graph = new QPushButton("Reeb Graph");
	controlLayout->addWidget(reeb_graph);

    // Connect signals and slots for button clicks and checkbox state changes
	connect(Visualize, &QPushButton::clicked, this,
		[=]()
		{
			try
			{
				// Clear the renderer before adding new actors
				renderer->RemoveAllViewProps();

				// Create an instance of ParticleActors
				ParticleActors particleActors;

				// Create actors for visualization
				vtkSmartPointer<vtkActor> cylinderActor = particleActors.Cylinder_view();
				vtkSmartPointer<vtkActor> particlesActor =
					particleActors.Conccenration_cyclinder_view(selectedReader, camera);
				vtkSmartPointer<vtkActor> particleActorConc = particleActors.Conccenration_view(selectedReader, camera);
				vtkSmartPointer<vtkActor> particleActorVel = particleActors.velocity_view(selectedReader, camera);

				// Check if actors are valid before adding them to the renderer
				if (cylinderActor && cactor_set)
					renderer->AddActor(cylinderActor);
				if (particlesActor && pactor_set)
					renderer->AddActor(particlesActor);
				if (particleActorConc && conc_set)
					renderer->AddActor(particleActorConc);
				if (particleActorVel && velocity_set)
					renderer->AddActor(particleActorVel);

				// Reset the camera for the new visualization
				renderer->ResetCamera();

				// Create a render window and an interactor for visualization
				vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
				renderWindow->AddRenderer(renderer);
				renderWindow->SetSize(800, 600);
				vtkSmartPointer<vtkRenderWindowInteractor> interactor =
					vtkSmartPointer<vtkRenderWindowInteractor>::New();
				interactor->SetRenderWindow(renderWindow);

				// Initialize and start the interactor
				interactor->Initialize();
				interactor->Start();
			}
			catch (const std::exception& e)
			{
				// Log error messages or handle exceptions here
				qDebug() << "Exception occurred: " << e.what();
			}
		});

	// Connect Reeb Graph button click to createSingleReebGraph slot
	connect(reeb_graph, &QPushButton::clicked, this,
		[=]()
		{
			// Create an instance of ViscousFingersReebGraph
			ViscousFingersReebGraph reebGraphHandler;

			// Create a Reeb Graph based on the selected .vtu file
			reebGraphHandler.createSingleReebGraph(selectedReader);
		});
	// Set the file name for the reader based on the selected file
	connect(fileComboBox, QOverload<int>::of(&QComboBox::activated),
		[=](int index)
		{
			selectedReader = fileReaders[index];
		});

	// Connect slots to handle checkbox state changes for different views
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
				velocity_set = true;
			}
			else
			{
				velocity_set = false;
			}
		});
	connect(showConcCheckBox, &QCheckBox::stateChanged,
		[=](int state)
		{
			if (state == Qt::Checked)
			{
				conc_set = true;
			}
			else
			{
				conc_set = false;
			}
		});

    // Execute the subwindow as a modal dialog, handling further actions if accepted
	if (subWindow.exec() == QDialog::Accepted)
	{
		// Handle any further actions here if needed
	}
}

