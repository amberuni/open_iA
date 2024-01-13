#pragma once

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>

/**
 * @brief ScatterGraph class to access different graph views for the Viscous fingers.
 */
class ScatterGraph
{
public:
	/**
     * @brief Constructor for the ScatterGraph class.
     */
	ScatterGraph();

	/**
     * @brief Plot scatter graphs for multiple VTU files.
     *
     * This function takes a vector of VTK XML Unstructured Grid readers, extracts relevant data
     * for each file, and plots scatter graphs for all files using VTK charts.
     *
     * @param readers A vector of VTK XML Unstructured Grid readers containing simulation data.
     */
	void Plot_scatter_graph_for_all_vtu(const std::vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>>& readers);

	/**
     * @brief Plot a scatter graph for a single VTU file.
     *
     * This function takes a VTK XML Unstructured Grid reader as input, extracts relevant data
     * such as velocity and concentration, and plots a scatter graph using VTK charts.
     *
     * @param reader A VTK XML Unstructured Grid reader containing simulation data.
     */
	void Plot_scatter_graph_for_one_vtu(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader);
};
