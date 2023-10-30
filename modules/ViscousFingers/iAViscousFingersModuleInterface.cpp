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
	if (!filePath.isEmpty())
	{
		if (filePath.endsWith(".vtu", Qt::CaseInsensitive))
		{
			// Load the data from the selected .vtu file
			// You can implement your data loading logic here
			QMessageBox::information(m_mainWnd, "Data Loaded", "Data loaded from file: " + filePath);
		}
		else
		{
			QMessageBox::critical(m_mainWnd, "Invalid File", "Please select a .vtu file.");
		}
	}
}
