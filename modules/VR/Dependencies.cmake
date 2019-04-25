
FIND_PACKAGE(OpenVR)

MESSAGE(STATUS "OpenVR: ${OPENVR_ROOT_DIR}")

SET (DEPENDENCIES_MODULES_NEW
	FeatureScout
)

SET( DEPENDENCIES_CMAKE
	vtkRenderingOpenVR_LOADED
)

SET( DEPENDENCIES_LIBRARIES
	${OPENVR_LIBRARY}
)

SET( DEPENDENCIES_INCLUDE_DIRS
	${OPENVR_INCLUDE_DIR}
)