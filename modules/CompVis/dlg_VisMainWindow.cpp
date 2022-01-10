#include "dlg_VisMainWindow.h"

//iA
#include "iAMainWindow.h"
#include "iACompVisMain.h"

dlg_VisMainWindow::dlg_VisMainWindow(iACsvDataStorage* dataStorage, iAMultidimensionalScaling* mds, iAMainWindow* parent, iACompVisMain* main, bool computeMDSFlag) :
	QMainWindow(parent), 
	m_main(main),
	m_dataStorage(dataStorage),
	m_data(dataStorage->getData()), 
	m_mds(mds),
	m_computeMDSFlag(computeMDSFlag)
{
	//setup iAMainWindow
	parent->addSubWindow(this);
	setupUi(this);

	if (m_computeMDSFlag)
	{
		//start mds dialog
		startMDSDialog();
		m_dataStorage->setMDSData(m_mds->getResultMatrix());
	}
	
	//finish iAMainWindow setup
	createMenu();
	this->showMaximized();
}

void dlg_VisMainWindow::startMDSDialog()
{
	m_MDSD = new dlg_MultidimensionalScalingDialog(m_data, m_mds);
	if (m_MDSD->exec() != QDialog::Accepted)
	{
		return;
	}
}

void dlg_VisMainWindow::recalculateMDS()
{
	m_main->reintitalizeMetrics();

	if (m_computeMDSFlag)
	{
		startMDSDialog();
		m_dataStorage->setMDSData(m_mds->getResultMatrix());
	}
	//m_main->reinitializeCharts();
}

void dlg_VisMainWindow::updateMDS(iAMultidimensionalScaling* newMds)
{
	m_mds = newMds;
}

QList<csvFileData>* dlg_VisMainWindow::getData()
{
	return m_data;
}

void dlg_VisMainWindow::createMenu()
{
	//activate MDS recalculation
	connect(actionRecalculateMDS, &QAction::triggered, this, &dlg_VisMainWindow::recalculateMDS);
	
	//activate ordering in histogram table
	connect(actionAscending_to_Number_of_Objects, &QAction::triggered, this, &dlg_VisMainWindow::reorderHistogramTableAscending);
	connect(actionDescending_to_Number_of_Objects, &QAction::triggered, this, &dlg_VisMainWindow::reorderHistogramTableDescending);
	connect(actionAs_Loaded, &QAction::triggered, this, &dlg_VisMainWindow::reorderHistogramTableAsLoaded);

	//activate switching between discretization methods
	connect(actionUniform, &QAction::triggered, this, &dlg_VisMainWindow::enableUniformTable);
	connect(actionBayesian_Blocks, &QAction::triggered, this, &dlg_VisMainWindow::enableBayesianBlocks);
	connect(actionNatual_Breaks, &QAction::triggered, this, &dlg_VisMainWindow::enableNaturalBreaks);

	connect(actionCurve_Representation, &QAction::triggered, this, &dlg_VisMainWindow::enableCurveTable);
}

void dlg_VisMainWindow::reorderHistogramTableAscending()
{
	m_main->orderHistogramTableAscending();
}

void dlg_VisMainWindow::reorderHistogramTableDescending()
{
	m_main->orderHistogramTableDescending();
}

void dlg_VisMainWindow::reorderHistogramTableAsLoaded()
{
	m_main->orderHistogramTableAsLoaded();
}

void dlg_VisMainWindow::enableUniformTable()
{
	m_main->enableUniformTable();
}

void dlg_VisMainWindow::enableBayesianBlocks()
{
	m_main->enableBayesianBlocks();
}

void dlg_VisMainWindow::enableNaturalBreaks()
{
	m_main->enableNaturalBreaks();
}

void dlg_VisMainWindow::enableCurveTable()
{
	m_main->enableCurveTable();
}