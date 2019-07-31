#include "iAdaptiveThresholdModuleInterface.h"
#include "dlg_AdaptiveThreshold.h"
#include "vtkImageData.h"
#include "mainwindow.h"

#include <QMessageBox>

void iAdaptiveThresholdModuleInterface::Initialize()
{
	if (!m_mainWnd)    // if m_mainWnd is not set, we are running in command line mode
	    return;        // in that case, we do not do anything as we can not add a menu entry there
	QMenu * toolsMenu = m_mainWnd->toolsMenu();  // alternatively, you can use getToolsMenu() here if you want to add a tool
	QAction * determineThreshold = new QAction( m_mainWnd );
	determineThreshold->setText( QApplication::translate( "MainWindow", "AdaptiveThresholding", 0 ) );
	AddActionToMenuAlphabeticallySorted(toolsMenu,  determineThreshold, false );
	//connect action with signal
	connect( determineThreshold, SIGNAL( triggered() ), this, SLOT( determineThreshold() ) );
}


#include <mdichild.h>
#include <charts/iADiagramFctWidget.h>
#include <charts/iAPlot.h>
#include <charts/iAPlotData.h>
#include <iAConsole.h>

void iAdaptiveThresholdModuleInterface::determineThreshold()
{
	/*
	QSharedPointer<iAPlotData> data;
	data->binStart(0);
	data->minX();*/

	AdaptiveThreshold dlg_thres;
		
	if (!m_mainWnd->activeMdiChild())
	{
		DEBUG_LOG("image not loaded");
		return; 
	}

	auto hist = m_mainWnd->activeMdiChild()->histogram();
	if (!hist || hist->plots().empty()) {
		DEBUG_LOG("Current data does not have a histogram or histogram not ready"); 
		return;
	}

	try {
			
		auto data = hist->plots()[0]->data();
		dlg_thres.initChart();
		dlg_thres.setHistData(data);
		if (dlg_thres.exec() != QDialog::Accepted)
			return;
	}
	catch (std::invalid_argument& iaex) {
		DEBUG_LOG(iaex.what()); 
		return; 
	}
}