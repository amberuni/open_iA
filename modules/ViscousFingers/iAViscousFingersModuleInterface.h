#pragma once
#include <iAGUIModuleInterface.h>

class iAViscousFingersModuleInterface : public iAGUIModuleInterface
{
	Q_OBJECT
public:
	void Initialize() override;
private slots:
	void testAction();
};