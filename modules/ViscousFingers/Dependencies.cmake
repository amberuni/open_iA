set(DEPENDENCIES_LIBRARIES
	iA::guibase
	iA::objectvis
	iA::renderer
)
set(DEPENDENCIES_VTK_MODULES
	ChartsCore            # for vtkAxis
	FiltersHybrid         # for vtkPolyDataSilhouette
	ImagingHybrid         # for vtkSampleFunction used in iABlobCluster
	ImagingGeneral	
)