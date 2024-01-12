#include "ViscousFingersReebGraph.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkReebGraph.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridToReebGraphFilter.h>
#include <vtkXMLUnstructuredGridReader.h>

// Qt includes
#include <QMessageBox>

// Constructor for the ViscousFingersReebGraph class
ViscousFingersReebGraph::ViscousFingersReebGraph()
{
}

/**
 * @brief Function to create a single Reeb graph from a VTK XML Unstructured Grid reader.
 *
 * This function takes a VTK XML Unstructured Grid reader as input and extracts the
 * unstructured grid data from it. It then retrieves information about the arrays in
 * the point data of the grid and displays this information in a message box. Subsequently,
 * the function creates a filter to convert the unstructured grid to a Reeb graph and
 * generates the Reeb graph using VTK filters.
 *
 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
 */
void ViscousFingersReebGraph::createSingleReebGraph(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader)
{
	// Get the unstructured grid from the reader
	vtkUnstructuredGrid* grid = reader->GetOutput();

	// Get point data from the grid
	vtkPointData* pointData = grid->GetPointData();
	int numArrays = pointData->GetNumberOfArrays();

	// Prepare a string to store information about arrays in point data
	std::string arraysInfo = "Arrays in PointData:\n";

	// Iterate through each array in point data and add its name to arraysInfo
	for (int i = 0; i < numArrays; ++i)
	{
		vtkDataArray* array = pointData->GetArray(i);
		if (array)
		{
			arraysInfo += "Array " + std::to_string(i) + ": " + array->GetName() + "\n";
		}
	}

	// Show a message box with information about arrays in point data
	QMessageBox::information(nullptr, "Arrays in PointData", arraysInfo.c_str());

	// Create a filter to convert the unstructured grid to a Reeb graph
	vtkSmartPointer<vtkUnstructuredGridToReebGraphFilter> m_volumeReebGraphFilter =
		vtkSmartPointer<vtkUnstructuredGridToReebGraphFilter>::New();

	// Set the input data for the Reeb graph filter
	m_volumeReebGraphFilter->SetInputData(grid);

	// Update the filter to generate the Reeb graph
	m_volumeReebGraphFilter->Update();

	// Get the generated Reeb graph from the filter's output
	vtkReebGraph* volumeReebGraph = m_volumeReebGraphFilter->GetOutput();


	// Visualize but crash preveting the implementation
}
