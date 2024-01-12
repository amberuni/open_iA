#pragma once

#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>

class ViscousFingersReebGraph
{
public:
	ViscousFingersReebGraph();

	void createSingleReebGraph(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader);
};