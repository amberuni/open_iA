#pragma once
#include <iAGUIModuleInterface.h>

#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>


class iAViscousFingersModuleInterface : public iAGUIModuleInterface
{
	Q_OBJECT

public:
	void Initialize() override;
private slots:
	void openSubWindow();
	void loadDataFromFolder();
	void interaction_window();

};