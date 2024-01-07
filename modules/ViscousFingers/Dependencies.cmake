set(DEPENDENCIES_LIBRARIES
	iA::guibase
	iA::objectvis
	iA::renderer
)
set(DEPENDENCIES_VTK_MODULES
	ChartsCore            # for vtkAxis
	CommonDataModel
	ImagingHybrid         # for vtkSampleFunction used in iABlobCluster
	ImagingGeneral
	ImagingStencil
	ImagingFourier
	FiltersReebGraph
	FiltersPoints
	FiltersGeneric
	FiltersGeometry
	FiltersHybrid
	FiltersExtraction
	FiltersTexture
)
