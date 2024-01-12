#pragma once
#include <iAGUIModuleInterface.h>

#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>


class iAViscousFingersModuleInterface : public iAGUIModuleInterface
{
	Q_OBJECT

public:
	/**
	 * @brief The iAViscousFingersModuleInterface class represents the interface for the Viscous Fingers module.
	 *
	 * This class is responsible for initializing and integrating the Viscous Fingers module
	 * with the application's main window and menu system.
	 */
	void Initialize() override;
private slots:
	/**
	 * @brief The iAViscousFingersModuleInterface class represents the interface for the Viscous Fingers module.
	 *
	 * This class is responsible for initializing and integrating the Viscous Fingers module
	 * with the application's main window and menu system.
	 */
	void openSubWindow();
	/**
	 * @brief The iAViscousFingersModuleInterface class represents the interface for the Viscous Fingers module.
	 *
	 * This class is responsible for initializing and integrating the Viscous Fingers module
	 * with the application's main window and menu system.
	 */
	void loadDataFromFolder();
	/**
	 * @brief The iAViscousFingersModuleInterface class represents the interface for the Viscous Fingers module.
	 *
	 * This class is responsible for initializing and integrating the Viscous Fingers module
	 * with the application's main window and menu system.
	 */
	void interaction_window();
};