#pragma once
#include <iAGUIModuleInterface.h>

#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>


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