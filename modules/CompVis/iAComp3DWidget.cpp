#include "iAComp3DWidget.h"

//CompVis
#include "iAMainWindow.h"

//iA
#include "iAQVTKWidget.h"
#include "iACsvIO.h"
#include "iACsvConfig.h"
#include "iA3DObjectVis.h"
#include "iA3DEllipseObjectVis.h"
#include "iA3DCylinderObjectVis.h"
#include "iA3DPolyObjectActor.h"
#include "iAVec3.h"

// Qt
#include <QBoxLayout>
#include <QSharedPointer>
#include <QMap>
#include <QColor>

//vtk
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTable.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkInteractorObserver.h"

#include <map>
#include <vector>
#include <algorithm>


iAComp3DWidget::iAComp3DWidget(
	iAMainWindow* parent, vtkSmartPointer<vtkTable> objectTable, iACsvIO* io, const iACsvConfig* csvConfig) :
	QDockWidget(parent),
	m_objectColor(QColor(140, 140, 140, 255)),
	m_interactionStyle(vtkSmartPointer<iAComp3DWidgetInteractionStyle>::New())
{
	setupUi(this);

	QVBoxLayout* layout = new QVBoxLayout;
	dockWidgetContents->setLayout(layout);

	m_qvtkWidget = new iAQVTKWidget(this);
	layout->addWidget(m_qvtkWidget);

	m_renderer = vtkSmartPointer<vtkRenderer>::New();
	addRendererToWidget(m_renderer);

	//interaction
	initializeInteraction();

	//rendering
	create3DVis(objectTable, io, csvConfig);
}

/*************** Rendering ****************************/
void iAComp3DWidget::showEvent(QShowEvent* event)
{
	QDockWidget::showEvent(event);
	
	renderWidget();
}

void iAComp3DWidget::renderWidget()
{
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 0, 0)
	m_qvtkWidget->GetRenderWindow()->GetInteractor()->Render();
#else
	m_qvtkWidget->renderWindow()->GetInteractor()->Render();
#endif
}

void iAComp3DWidget::addRendererToWidget(vtkSmartPointer<vtkRenderer> renderer)
{
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 0, 0)
	m_qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
#else
	m_qvtkWidget->renderWindow()->AddRenderer(renderer);
#endif
}

void iAComp3DWidget::setInteractorStyleToWidget(vtkSmartPointer<vtkInteractorObserver> style)
{
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 0, 0)
	m_qvtkWidget->GetInteractor()->SetInteractorStyle(style);
#else
	m_qvtkWidget->interactor()->SetInteractorStyle(style);
#endif
}

void iAComp3DWidget::removeAllRendererFromWidget()
{
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 0, 0)

#else
	vtkRendererCollection* rendererList = m_qvtkWidget->renderWindow()->GetRenderers();

	vtkCollectionSimpleIterator sit;
	rendererList->InitTraversal(sit);
	for (int i = 0; i < rendererList->GetNumberOfItems(); i++)
	{
		vtkRenderer* currRend = rendererList->GetNextRenderer(sit);
		m_qvtkWidget->renderWindow()->RemoveRenderer(currRend);
	}
#endif
}

/*************** Initialization ****************************/
void iAComp3DWidget::create3DVis(vtkSmartPointer<vtkTable> objectTable, iACsvIO* io, const iACsvConfig* csvConfig)
{
	QSharedPointer<QMap<uint, uint>> columnMapping = io->getOutputMapping();

	if (csvConfig->visType == iACsvConfig::Cylinders)
	{
		int cylinderQuality = csvConfig->cylinderQuality;
		size_t segmentSkip = csvConfig->segmentSkip;
		m_3dvisData = std::make_shared<iA3DCylinderObjectVis>(
			objectTable,
			columnMapping,
			m_objectColor,
			std::map<size_t, std::vector<iAVec3f>>(),	// empty curved fiber info
			cylinderQuality,
			segmentSkip);
	}
	else if (csvConfig->visType == iACsvConfig::Ellipses)
	{
		m_3dvisData = std::make_shared<iA3DEllipseObjectVis>(
			objectTable,
			columnMapping,
			m_objectColor);
	}
	m_3dvisActor = std::make_shared<iA3DPolyObjectActor>(m_renderer.Get(), m_3dvisData.get());
	m_3dvisActor->show();
}

void iAComp3DWidget::initializeInteraction()
{
	m_interactionStyle->setVisualization(this);
	m_interactionStyle->SetDefaultRenderer(m_renderer);
	setInteractorStyleToWidget(m_interactionStyle);
}

/*************** Update ****************************/

void iAComp3DWidget::resetWidget()
{
	//reset selection by rendering an empty vector of selectionIds
	m_3dvisData->renderSelection(std::vector<size_t>(), 0, m_objectColor, nullptr);
	renderWidget();
}

void iAComp3DWidget::drawSelection(std::vector<size_t> selectedIds)
{
	std::sort(selectedIds.begin(), selectedIds.end());
	const std::vector<size_t> constSelectedIds(selectedIds);
	m_3dvisData->renderSelection(constSelectedIds, 0, m_objectColor, nullptr);
	renderWidget();
}