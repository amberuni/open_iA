#pragma once

#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkActor.h>
#include <vtkCamera.h>

class ParticleActors {

public:
    ParticleActors();

    vtkSmartPointer<vtkActor> Conccenration_view(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);

    vtkSmartPointer<vtkActor> Conccenration_cyclinder_view(
		vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);

    vtkSmartPointer<vtkActor> Cylinder_view();

    vtkSmartPointer<vtkActor> velocity_view(vtkSmartPointer<vtkXMLUnstructuredGridReader> reader, vtkSmartPointer<vtkCamera> camera);
};
