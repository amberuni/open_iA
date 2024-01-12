#pragma once

#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>


// Class to access viscous finger Reeb graph methods
class ViscousFingersReebGraph
{
public:
	ViscousFingersReebGraph();
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
	void createSingleReebGraph(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader);
};