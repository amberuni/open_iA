#include "ViscousFingersStats.h"

#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkChartXYZ.h>
#include <vtkContextView.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkPlot.h>
#include <vtkPlot3D.h>
#include <vtkPlotPoints3D.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkPlotBar.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPlotPoints.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <random>
#include <vector>
#include <vector>


// Constructor for the StatGraph class
StatGraph::StatGraph()
{
}

/**
 * @brief Plot a scatter graph for a single VTU file.
 *
 * This function takes a VTK XML Unstructured Grid reader as input, extracts relevant data
 * such as velocity and concentration, and plots a scatter graph using VTK charts.
 *
 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
 */
void StatGraph::Plot_scatter_graph_for_one_vtu(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader)
{
	// Get the unstructured grid from the reader
	vtkUnstructuredGrid* grid = reader->GetOutput();

	// Get point data and field data from the grid
	vtkPointData* pointData = grid->GetPointData();
	vtkFieldData* fieldData = grid->GetFieldData();

	// Extract arrays from point and field data
	vtkDataArray* time = fieldData->GetArray("time");
	vtkDataArray* concentration = pointData->GetArray("concentration");
	vtkDataArray* velocity = pointData->GetArray("velocity");

	// Assuming time, concentration, and velocity arrays are valid
	if (time && concentration && velocity)
	{
		// Create a table to hold the data
		vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
		table->AddColumn(velocity);
		table->AddColumn(concentration);

		// Create a chart
		vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();

		// Add the concentration vs time plot to the chart
		vtkSmartPointer<vtkPlot> plot = chart->AddPlot(vtkChart::POINTS);
		plot->SetInputData(table, 0, 1);
		plot->SetColorF(1.0, 0.0, 0.0);  // Set color to red

		// Set up axes
		vtkAxis* xAxis = chart->GetAxis(vtkAxis::BOTTOM);
		xAxis->SetTitle("velocity");

		vtkAxis* yAxis = chart->GetAxis(vtkAxis::LEFT);
		yAxis->SetTitle("Concentration");

		// Create a renderer
		vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

		// Create a render window
		vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
		renderWindow->AddRenderer(renderer);

		// Create a render window interactor
		vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
			vtkSmartPointer<vtkRenderWindowInteractor>::New();
		renderWindowInteractor->SetRenderWindow(renderWindow);

		// Create a context view
		vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
		view->SetRenderWindow(renderWindow);

		// Set the chart to the context view
		view->GetScene()->AddItem(chart);
		view->GetRenderWindow()->SetSize(400, 300);

		// Add the renderer to the render window
		renderWindow->AddRenderer(renderer);

		// Initialize the interactor and start the rendering loop
		renderWindow->Render();
		renderWindowInteractor->Initialize();
		renderWindowInteractor->Start();
	}
}

/**
 * @brief Generate a random RGB color.
 *
 * This function generates a random RGB color and stores it in the given color array.
 *
 * @param color An array to store the generated RGB color.
 */
static inline void generateRandomColor(std::array<double, 3>& color)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<double> dis(0.0, 1.0);

	color[0] = dis(gen);
	color[1] = dis(gen);
	color[2] = dis(gen);
}

/**
 * @brief Plot scatter graphs for multiple VTU files.
 *
 * This function takes a vector of VTK XML Unstructured Grid readers, extracts relevant data
 * for each file, and plots scatter graphs for all files using VTK charts.
 *
 * @param readers A vector of VTK XML Unstructured Grid readers containing simulation data.
 */
void StatGraph::Plot_scatter_graph_for_all_vtu(
	const std::vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>>& readers)
{
	// Create a renderer
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

	// Create a render window
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	// Create a render window interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// Create a context view
	vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
	view->SetRenderWindow(renderWindow);

	// Create a chart
	vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();

	// Loop through readers
	for (size_t i = 0; i < readers.size(); ++i)
	{
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = readers[i];

		// Get the unstructured grid from the reader
		vtkUnstructuredGrid* grid = reader->GetOutput();

		// Get point data and field data from the grid
		vtkPointData* pointData = grid->GetPointData();
		vtkFieldData* fieldData = grid->GetFieldData();

		// Extract arrays from point and field data
		vtkDataArray* time = fieldData->GetArray("time");
		vtkDataArray* concentration = pointData->GetArray("concentration");
		vtkDataArray* velocity = pointData->GetArray("velocity");

		// Assuming time, concentration, and velocity arrays are valid
		if (time && concentration && velocity)
		{
			// Create a table to hold the data
			vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
			table->AddColumn(velocity);
			table->AddColumn(concentration);

			// Add the concentration vs time plot to the chart
			vtkSmartPointer<vtkPlotPoints> plot = vtkSmartPointer<vtkPlotPoints>::New();
			plot->SetInputData(table, 0, 1);

			// Generate a random color for the plot
			std::array<double, 3> color;
			generateRandomColor(color);
			plot->SetColorF(color[0], color[1], color[2]);

			// Set up axes (only for the first iteration)
			if (i == 0)
			{
				vtkAxis* xAxis = chart->GetAxis(vtkAxis::BOTTOM);
				xAxis->SetTitle("velocity");

				vtkAxis* yAxis = chart->GetAxis(vtkAxis::LEFT);
				yAxis->SetTitle("Concentration");
			}

			// Add the plot to the chart
			chart->AddPlot(plot);
		}
	}

	// Set the chart to the context view
	view->GetScene()->AddItem(chart);

	// Set the size of the render window
	view->GetRenderWindow()->SetSize(1920, 1000);

	// Add the renderer to the render window
	renderWindow->AddRenderer(renderer);

	// Initialize the interactor and start the rendering loop
	renderWindow->Render();
	renderWindowInteractor->Initialize();
	renderWindowInteractor->Start();
}

/**
 * @brief Plot a bag plot for a single VTU file.
 *
 * This function takes a VTK XML Unstructured Grid reader as input, extracts relevant data
 * such as velocity and concentration, and plots a bag plot using VTK charts.
 *
 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
 */
void StatGraph::Plot_bag_plot_for_one_vtu(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader)
{
	// Get the unstructured grid from the reader
	vtkUnstructuredGrid* grid = reader->GetOutput();

	// Get point data and field data from the grid
	vtkPointData* pointData = grid->GetPointData();
	vtkFieldData* fieldData = grid->GetFieldData();

	// Extract arrays from point and field data
	vtkDataArray* time = fieldData->GetArray("time");
	vtkDataArray* concentration = pointData->GetArray("concentration");
	vtkDataArray* velocity = pointData->GetArray("velocity");

	// Assuming time, concentration, and velocity arrays are valid
	if (time && concentration && velocity)
	{
		// Create a table to hold the data
		vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
		table->AddColumn(velocity);
		table->AddColumn(concentration);

		// Create a chart
		vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();

		// Add the concentration vs velocity box plot to the chart
		vtkSmartPointer<vtkPlot> plot = chart->AddPlot(vtkChart::BAG);
		plot->SetInputData(table, 0, 1);
		// Generate a random color for the plot
		std::array<double, 3> color;
		generateRandomColor(color);
		plot->SetColorF(color[0], color[1], color[2]);

		// Set up axes
		vtkAxis* xAxis = chart->GetAxis(vtkAxis::BOTTOM);
		xAxis->SetTitle("velocity");

		vtkAxis* yAxis = chart->GetAxis(vtkAxis::LEFT);
		yAxis->SetTitle("Concentration");

		// Create a renderer
		vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

		// Create a render window
		vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
		renderWindow->AddRenderer(renderer);

		// Create a render window interactor
		vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
			vtkSmartPointer<vtkRenderWindowInteractor>::New();
		renderWindowInteractor->SetRenderWindow(renderWindow);

		// Create a context view
		vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
		view->SetRenderWindow(renderWindow);

		// Set the chart to the context view
		view->GetScene()->AddItem(chart);
		view->GetRenderWindow()->SetSize(400, 300);

		// Add the renderer to the render window
		renderWindow->AddRenderer(renderer);

		// Initialize the interactor and start the rendering loop
		renderWindow->Render();
		renderWindowInteractor->Initialize();
		renderWindowInteractor->Start();
	}
}

/**
 * @brief Plot bag plots for multiple VTU files.
 *
 * This function takes a vector of VTK XML Unstructured Grid readers, extracts relevant data
 * for each file, and plots bag plots for all files using VTK charts.
 *
 * @param readers A vector of VTK XML Unstructured Grid readers containing simulation data.
 */
void StatGraph::Plot_bag_plots_for_all_vtu(const std::vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>>& readers)
{
	// Create a renderer
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

	// Create a render window
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	// Create a render window interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// Create a context view
	vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
	view->SetRenderWindow(renderWindow);

	// Create a chart
	vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();

	// Loop through readers
	for (size_t i = 0; i < readers.size(); ++i)
	{
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = readers[i];

		// Get the unstructured grid from the reader
		vtkUnstructuredGrid* grid = reader->GetOutput();

		// Get point data and field data from the grid
		vtkPointData* pointData = grid->GetPointData();
		vtkFieldData* fieldData = grid->GetFieldData();

		// Extract arrays from point and field data
		vtkDataArray* time = fieldData->GetArray("time");
		vtkDataArray* concentration = pointData->GetArray("concentration");
		vtkDataArray* velocity = pointData->GetArray("velocity");

		// Assuming time, concentration, and velocity arrays are valid
		if (time && concentration && velocity)
		{
			// Create a table to hold the data
			vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
			table->AddColumn(velocity);
			table->AddColumn(concentration);

		// Add the concentration vs velocity box plot to the chart
			vtkSmartPointer<vtkPlot> plot = chart->AddPlot(vtkChart::BAG);
			plot->SetInputData(table, 0, 1);

			// Generate a random color for the plot
			std::array<double, 3> color;
			generateRandomColor(color);
			plot->SetColorF(color[0], color[1], color[2]);
			// Add the plot to the chart
			chart->AddPlot(plot);
		}
	}

	// Set up axes
	vtkAxis* xAxis = chart->GetAxis(vtkAxis::BOTTOM);
	xAxis->SetTitle("velocity");

	vtkAxis* yAxis = chart->GetAxis(vtkAxis::LEFT);
	yAxis->SetTitle("Concentration");

	// Set the chart to the context view
	view->GetScene()->AddItem(chart);

	// Set the size of the render window
	view->GetRenderWindow()->SetSize(1920, 1000);

	// Add the renderer to the render window
	renderWindow->AddRenderer(renderer);

	// Initialize the interactor and start the rendering loop
	renderWindow->Render();
	renderWindowInteractor->Initialize();
	renderWindowInteractor->Start();
}

/**
 * @brief Plot average concentration change per time stamp.
 *
 * This function takes a vector of VTK XML Unstructured Grid readers, extracts relevant data
 * for each file, and average concentration change per time stamp for all files using VTK charts.
 *
 * @param readers A vector of VTK XML Unstructured Grid readers containing simulation data.
 */
void StatGraph::Plot_average_concentration_line_graph(
	const std::vector<vtkSmartPointer<vtkXMLUnstructuredGridReader>>& readers)
{
	// Create a renderer
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

	// Create a render window
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	// Create a render window interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// Create a context view
	vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
	view->SetRenderWindow(renderWindow);

	// Create a chart
	vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();

	// Create a table to hold data for the line graph
	vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();

	// Add columns for time and average concentration
	vtkSmartPointer<vtkFloatArray> timeArray = vtkSmartPointer<vtkFloatArray>::New();
	timeArray->SetName("Time");
	table->AddColumn(timeArray);

	vtkSmartPointer<vtkFloatArray> avgConcentrationArray = vtkSmartPointer<vtkFloatArray>::New();
	avgConcentrationArray->SetName("Average Concentration");
	table->AddColumn(avgConcentrationArray);

	// Loop through readers
	for (size_t i = 0; i < readers.size(); ++i)
	{
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = readers[i];

		// Get the time from the FieldData of the reader
		vtkFieldData* fieldData = reader->GetOutput()->GetFieldData();
		vtkDataArray* timeData = fieldData->GetArray("time");

		if (timeData)
		{
			double time = timeData->GetTuple1(0);

			// Get point data and field data from the grid
			vtkPointData* pointData = reader->GetOutput()->GetPointData();
			vtkDataArray* concentration = pointData->GetArray("concentration");

			// Assuming concentration array is valid
			if (concentration)
			{
				// Calculate the average concentration
				double sumConcentration = 0.0;
				for (vtkIdType j = 0; j < concentration->GetNumberOfTuples(); ++j)
				{
					sumConcentration += concentration->GetTuple1(j);
				}
				double avgConcentration = sumConcentration / concentration->GetNumberOfTuples();

				// Add data to the table
				timeArray->InsertNextValue(time);                         
				avgConcentrationArray->InsertNextValue(avgConcentration);
			}
		}
	}

	// Add the average concentration line plot to the chart
	vtkSmartPointer<vtkPlot> avgConcentrationPlot = chart->AddPlot(vtkChart::LINE);
	avgConcentrationPlot->SetInputData(table, 0, 1);
	avgConcentrationPlot->SetColorF(255, 0, 0);

	// Set up axes
	vtkAxis* xAxis = chart->GetAxis(vtkAxis::BOTTOM);
	xAxis->SetTitle("Time");

	vtkAxis* yAxis = chart->GetAxis(vtkAxis::LEFT);
	yAxis->SetTitle("Average Concentration");

	// Set the chart to the context view
	view->GetScene()->AddItem(chart);

	// Set the size of the render window
	view->GetRenderWindow()->SetSize(800, 600);

	// Add the renderer to the render window
	renderWindow->AddRenderer(renderer);

	// Initialize the interactor and start the rendering loop
	renderWindow->Render();
	renderWindowInteractor->Initialize();
	renderWindowInteractor->Start();
}

