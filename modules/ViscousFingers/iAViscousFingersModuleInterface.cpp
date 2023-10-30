#include "iAViscousFingersModuleInterface.h"

#include "iAMainWindow.h"

#include <QAction>
#include <QMessageBox>

void iAViscousFingersModuleInterface::Initialize()
{
	if (!m_mainWnd)    // if m_mainWnd is not set, we are running in command line mode
	{
	    return;        // in that case, we do not do anything as we can not add a menu entry there
	}
	QAction * actionTest = new QAction(tr("ViscousFingers"), m_mainWnd);
	connect(actionTest, &QAction::triggered, this, &iAViscousFingersModuleInterface::testAction);
	addToMenuSorted(m_mainWnd->toolsMenu(), actionTest);
}

void iAViscousFingersModuleInterface::testAction()
{
	QMessageBox::information(m_mainWnd, "Test Module", "This is the Test Module!");
}