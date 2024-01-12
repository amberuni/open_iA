#pragma once

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>

// ParticleActors class to access different views for the Viscous fingers
class ParticleActors
{
public:
	// Constructor for the ParticleActors class
	ParticleActors();

	/**
	 * @brief Function to create an actor for concentration view.
	 *
	 * This function generates a VTK actor for visualizing concentration in the fluid flow.
	 * It assigns scalar attribute 'concentration' to the points of the dataset and applies
	 * various VTK filters for masking, thresholding, geometry extraction, sorting, and coloring.
	 * The resulting actor is returned for visualization.
	 *
	 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
	 * @param camera The VTK camera used for rendering.
	 * @return A VTK actor representing the concentration view.
	 */
	vtkSmartPointer<vtkActor> Conccenration_view(
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);

	/**
	 * @brief Function to create an actor for concentration cylinder view.
	 *
	 * This function generates a VTK actor for visualizing concentration in the fluid flow
	 * within a cylindrical object. It applies various VTK filters for masking, geometry
	 * extraction, sorting, and coloring. The resulting actor is returned for visualization.
	 *
	 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
	 * @param camera The VTK camera used for rendering.
	 * @return A VTK actor representing the concentration cylinder view.
	 */
	vtkSmartPointer<vtkActor> Conccenration_cyclinder_view(
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);

	/**
	 * @brief Function to create an actor for a cylinder view.
	 *
	 * This function generates a VTK actor for visualizing a cylindrical object. It utilizes
	 * the VTK cylinder source and applies transformations to the actor for proper rendering.
	 * The resulting actor is returned for visualization.
	 *
	 * @return A VTK actor representing the cylinder view.
	 */
	vtkSmartPointer<vtkActor> Cylinder_view();

	/**
	 * @brief Function to create an actor for velocity view.
	 *
	 * This function generates a VTK actor for visualizing particles with varying velocity.
	 * It uses the 'velocity' array for thresholding and applies various VTK filters for masking,
	 * thresholding, geometry extraction, sorting, and coloring. The resulting actor is returned
	 * for visualization.
	 *
	 * @param reader A VTK XML Unstructured Grid reader containing simulation data.
	 * @param camera The VTK camera used for rendering.
	 * @return A VTK actor representing the velocity view.
	 */
	vtkSmartPointer<vtkActor> velocity_view(
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);
};
