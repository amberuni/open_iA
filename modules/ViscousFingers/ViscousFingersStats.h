#pragma once

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>

/**
 * @brief StatGraph class to access different graph views for the Viscous fingers.
 */
class StatGraph
{
public:
	/**
     * @brief Constructor for the StatGraph class.
     */
	StatGraph();

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

	/**
	 * @brief Plot bag plots for multiple VTU files.
	 *
	 * This function takes a vector of VTK XML Unstructured Grid readers, extracts relevant data
	 * for each file, and plots bag plots for all files using VTK charts.
	 *
	 * @param readers A vector of VTK XML Unstructured Grid readers containing simulation data.
	 */
	void Plot_bag_plots_for_all_vtu(const std::vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>>& readers);

	/**
	 * @brief Plot a bag plot for a single VTU file.
	 *
	 * This function takes a VTK XML Unstructured Grid reader as input, extracts relevant data
	 * such as velocity and concentration, and plots a bag plot using VTK charts.
	 *
	 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
	 */
	void Plot_bag_plot_for_one_vtu(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader);

	/**
	 * @brief Plot a line graph showing the average concentration for each VTU file versus time.
	 *
	 * This function takes a vector of VTK XML Unstructured Grid readers as input, extracts
	 * relevant data such as concentration and time for each file, and plots a line graph
	 * showing the average concentration for each file versus time using VTK charts.
	 *
	 * @param readers A vector of VTK XML Unstructured Grid readers containing simulation data.
	 */
	void Plot_average_concentration_line_graph(
		const std::vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>>& readers);

};
