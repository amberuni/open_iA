#include "ViscousFingersReebGraph.h"

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkReebGraph.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGridToReebGraphFilter.h>

#include <QMessageBox>

ViscousFingersReebGraph::ViscousFingersReebGraph()
{ }


void ViscousFingersReebGraph::createSingleReebGraph(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader)
{
	vtkUnstructuredGrid* grid = reader->GetOutput();

	vtkPointData* pointData = grid->GetPointData();
	int numArrays = pointData->GetNumberOfArrays();

	std::string arraysInfo = "Arrays in PointData:\n";

	for (int i = 0; i < numArrays; ++i)
	{
		vtkDataArray* array = pointData->GetArray(i);
		if (array)
		{
			arraysInfo += "Array " + std::to_string(i) + ": " + array->GetName() + "\n";
		}
	}

	QMessageBox::information(nullptr, "Arrays in PointData", arraysInfo.c_str());
	vtkSmartPointer <vtkUnstructuredGridToReebGraphFilter> m_volumeReebGraphFilter =
		vtkSmartPointer<vtkUnstructuredGridToReebGraphFilter>::New();
	m_volumeReebGraphFilter->SetInputData(grid);
	m_volumeReebGraphFilter->Update();
	vtkReebGraph* volumeReebGraph = m_volumeReebGraphFilter->GetOutput();
}
