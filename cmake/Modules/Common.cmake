#-------------------------
# CMake Policies
#-------------------------
CMAKE_POLICY(SET CMP0054 NEW)	# suppress warning about variable expansion
CMAKE_POLICY(SET CMP0071 NEW)	# will also automoc/autoui autogenerated Qt files (and suppress CMake warnings about them being excluded)
CMAKE_POLICY(SET CMP0087 NEW)

#-------------------------
# Disable In-Source Build
#-------------------------
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
IF ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds in ${CMAKE_BINARY_DIR} are disabled to avoid "
		"cluttering the source repository. Please delete ./CMakeCache.txt and ./CMakeFiles/, "
		"and run cmake with a newly created build directory.")
ENDIF()

MESSAGE(STATUS "CMake: ${CMAKE_VERSION}")

#-------------------------
# CTest
#-------------------------
option (openiA_TESTING_ENABLED "Whether to enable testing. This allows to run CTest/ CDash builds. Default: disabled." OFF)
IF (openiA_TESTING_ENABLED)
	MESSAGE(STATUS "Testing enabled.")
	INCLUDE (CTest)
	enable_testing()
	IF (openiA_USE_IDE_FOLDERS)
		SET_PROPERTY(TARGET Continuous PROPERTY FOLDER "_CTest")
		SET_PROPERTY(TARGET Experimental PROPERTY FOLDER "_CTest")
		SET_PROPERTY(TARGET Nightly PROPERTY FOLDER "_CTest")
		SET_PROPERTY(TARGET NightlyMemoryCheck PROPERTY FOLDER "_CTest")
	ENDIF()
ENDIF()

#-------------------------
# Precompiled headers
#-------------------------
IF (CMAKE_VERSION VERSION_GREATER "3.15.99")
	option (openiA_PRECOMPILE  "Whether to use precompiled headers to speed up build. Default: disabled." OFF)
	IF (openiA_PRECOMPILE)
		MESSAGE(STATUS "openiA_PRECOMPILE enabled.")
	ENDIF()
ENDIF()

option (openiA_CHART_OPENGL "Whether to use OpenGL in chart widgets (disable this if you see no text, which was observed on AMD graphics cards" ON)

#------------------------------
# Build / Compiler information
#------------------------------
set (BUILD_INFO "\"CMake: ${CMAKE_VERSION} (Generator: ${CMAKE_GENERATOR})\\n\"\n")
IF (MSVC)
	MESSAGE(STATUS "Compiler: Visual C++ (MSVC_VERSION ${MSVC_VERSION} / ${CMAKE_CXX_COMPILER_VERSION})")
	set (BUILD_INFO "${BUILD_INFO}    \"Compiler: Visual C++ (MSVC_VERSION ${MSVC_VERSION} / ${CMAKE_CXX_COMPILER_VERSION})\\n\"\n")
	set (BUILD_INFO "${BUILD_INFO}    \"Windows SDK: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\\n\"\n")
	# Apply file grouping based on regular expressions for Visual Studio IDE.
	SOURCE_GROUP("UI Files" REGULAR_EXPRESSION "[.](ui|qrc)$")
ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	MESSAGE(STATUS "Compiler: Clang (${CMAKE_CXX_COMPILER_VERSION})")
	set (BUILD_INFO "${BUILD_INFO}    \"Compiler: Clang (Version ${CMAKE_CXX_COMPILER_VERSION})\\n\"\n")
ELSEIF (CMAKE_COMPILER_IS_GNUCXX)
	MESSAGE(STATUS "Compiler: G++ (${CMAKE_CXX_COMPILER_VERSION})")
	set (BUILD_INFO "${BUILD_INFO}    \"Compiler: G++ (Version ${CMAKE_CXX_COMPILER_VERSION})\\n\"\n")
ELSE()
	MESSAGE(WARNING "Unknown compiler! Please report any CMake or compilation errors on https://github.com/3dct/open_iA!")
	set (BUILD_INFO "${BUILD_INFO}    \"Compiler: Unknown\\n\"\n")
ENDIF()
set (BUILD_INFO "${BUILD_INFO}    \"Targetting ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}\\n\"\n")
IF (FLATPAK_BUILD)
	set (BUILD_INFO "${BUILD_INFO}    \"Flatpak Build\\n\"\n")
ENDIF()

#-------------------------
# Output Directories
#-------------------------
IF (CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Multi-configuration generator")
	SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/x64/Debug")
	SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/x64/Release")
	SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/x64/RelWithDebInfo")
	SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/x64/MinSizeRel")
ELSE()
	message(STATUS "Single-configuration generator")
	# Set a default build type if none was specified
	if (NOT CMAKE_BUILD_TYPE)
		set(DEFAULT_BUILD_TYPE "Release")
		message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
		set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE} CACHE STRING "Choose the type of build." FORCE)
		# Set the possible values of build type for cmake-gui
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
	endif()
	SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
	SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
ENDIF()


#-------------------------
# LIBRARIES
#-------------------------

# Prepare empty BUNDLE vars:
SET (BUNDLE_DIRS "")
SET (BUNDLE_LIBS "")

# ITK
SET(SAVED_CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
FIND_PACKAGE(ITK REQUIRED)
MESSAGE(STATUS "ITK: ${ITK_VERSION} in ${ITK_DIR}.")
IF (ITK_VERSION VERSION_LESS "4.10.0")
	MESSAGE(FATAL_ERROR "Your ITK version is too old. Please use ITK >= 4.10")
ENDIF()
SET (ITK_COMPONENTS
	ITKConvolution
	ITKDenoising
	ITKDistanceMap
	ITKGPUAnisotropicSmoothing
	ITKImageFeature
	ITKImageFusion
	ITKImageNoise
	ITKLabelMap
	ITKMesh
	ITKReview       # for LabelGeometryImageFilter
	ITKTestKernel   # for PipelineMonitorImageFilter
	ITKVtkGlue
	ITKWatersheds)
IF (ITK_VERSION VERSION_GREATER "4.12.99") # libraries split up in ITK 4.13:
	LIST (APPEND ITK_COMPONENTS ITKImageIO)
	LIST (APPEND ITK_COMPONENTS ITKIORAW)  # apparently not included in ITKImageIO
ELSE()
	FOREACH( mod IN LISTS ITK_MODULES_ENABLED)
		IF( ${mod} MATCHES "IO")
			LIST (APPEND ITK_COMPONENTS ${mod})
		ENDIF()
	ENDFOREACH()
ENDIF()
set (ITK_HGrad_INFO "disabled")
IF (HigherOrderAccurateGradient_LOADED)
	MESSAGE(STATUS "    HigherOrderAccurateGradient available as ITK module.")
	set (ITK_HGrad_INFO "enabled")
	LIST (APPEND ITK_COMPONENTS HigherOrderAccurateGradient)
ENDIF()
set (ITK_RTK_INFO "disabled")
IF (RTK_LOADED)
	MESSAGE(STATUS "    RTK ${RTK_VERSION_MAJOR}.${RTK_VERSION_MINOR}.${RTK_VERSION_PATCH} available as ITK module.")
	set (ITK_RTK_INFO "${RTK_VERSION_MAJOR}.${RTK_VERSION_MINOR}.${RTK_VERSION_PATCH}")
	LIST (APPEND ITK_COMPONENTS RTK)
ENDIF()
# ITK has been found in sufficient version, otherwise above REQUIRED / FATAL_ERROR would have triggered CMake abort
# Now set it up with the components we need:
FIND_PACKAGE(ITK COMPONENTS ${ITK_COMPONENTS})
# apparently ITK (at least v5.0.0) adapts CMAKE_MODULE_PATH (bug?), reset it:
SET(CMAKE_MODULE_PATH "${SAVED_CMAKE_MODULE_PATH}")
INCLUDE(${ITK_USE_FILE}) # <- maybe avoid by using INCLUDE/LINK commands on targets instead?
# problem: also does some factory initialization (IO), which cannot easily be called separately
SET (ITK_BASE_DIR "${ITK_DIR}")
IF (MSVC)
	SET (ITK_LIB_DIR "${ITK_DIR}/bin/Release")
ELSE()
	if (EXISTS "${ITK_DIR}/lib")
		SET (ITK_LIB_DIR "${ITK_DIR}/lib")
	else()
		SET (ITK_BASE_DIR "${ITK_DIR}/../../..")
		SET (ITK_LIB_DIR "${ITK_BASE_DIR}/lib")
	endif()
ENDIF()
MESSAGE(STATUS "    ITK_LIB_DIR: ${ITK_LIB_DIR}")
LIST (APPEND BUNDLE_DIRS "${ITK_LIB_DIR}")
set (ITK_SCIFIO_INFO "disabled")
IF (SCIFIO_LOADED)
	set (ITK_SCIFIO_INFO "enabled")
	MESSAGE(STATUS "    SCIFIO support enabled!\n\
       Notice that in order to run a build with this library on another machine\n\
       than the one you built it, the environment variable SCIFIO_PATH\n\
       has to be set to the path containing the SCIFIO jar files!\n\
       Otherwise loading images will fail!")
	SET (SCIFIO_PATH "${ITK_BASE_DIR}/lib/jars")
	IF (MSVC)
		# variable will be set to the debugging environment instead of copying (see gui/CMakeLists.txt)
	ELSE()
		SET (DESTDIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scifio_jars")
		MESSAGE(STATUS "Copying SCIFIO jars from ${SCIFIO_PATH} to ${DESTDIR}")
		configure_file("${SCIFIO_PATH}/bioformats_package.jar" "${DESTDIR}/bioformats_package.jar" COPYONLY)
		configure_file("${SCIFIO_PATH}/scifio-itk-bridge.jar" "${DESTDIR}/scifio-itk-bridge.jar" COPYONLY)
	ENDIF()
	INSTALL(FILES "${SCIFIO_PATH}/bioformats_package.jar" DESTINATION scifio_jars)
	INSTALL(FILES "${SCIFIO_PATH}/scifio-itk-bridge.jar" DESTINATION scifio_jars)
ENDIF()

set(ITK_GPU_INFO "enabled")
IF ("${ITKGPUCommon_LIBRARY_DIRS}" STREQUAL "")
	set(ITK_GPU_INFO "disabled")
	MESSAGE(WARNING "ITK is built without GPU support (flag ITK_USE_GPU disabled). Some GPU-optimized functionality might not be available!")
ENDIF()
set (BUILD_INFO "${BUILD_INFO}    \"ITK: ${ITK_VERSION} (GPU: ${ITK_GPU_INFO}, SCIFIO: ${ITK_SCIFIO_INFO}, RTK: ${ITK_RTK_INFO}, HigherOrderGradient: ${ITK_HGrad_INFO})\\n\"\n")

# VTK
FIND_PACKAGE(VTK REQUIRED)
MESSAGE(STATUS "VTK: ${VTK_VERSION} in ${VTK_DIR}.")
IF (VTK_VERSION VERSION_LESS "8.0.0")
	MESSAGE(FATAL_ERROR "Your VTK version is too old. Please use VTK >= 8.0")
ENDIF()
SET (VTK_LIB_PREFIX "VTK::")
IF (VTK_VERSION VERSION_LESS "9.0.0")
	SET (VTK_COMP_PREFIX "vtk")
	SET (VTK_BASE_LIB_LIST kwiml)
SET (VTK_LIB_PREFIX "vtk")
ELSE()
	SET (VTK_RENDERING_BACKEND "OpenGL2")     # peculiarity about VTK 9: it sets VTK_RENDERING_BACKEND to "OpenGL", but for our purposes, it behaves exactly like when previously it was set to OpenGL2. The VTK_RENDERING_BACKEND also isn't exposed as user parameter anymore.
	SET (VTK_COMP_PREFIX "")
ENDIF()
MESSAGE(STATUS "    Rendering Backend: ${VTK_RENDERING_BACKEND}")
SET (VTK_COMPONENTS
	${VTK_COMP_PREFIX}FiltersModeling         # for vtkRotationalExtrusionFilter, vtkOutlineFilter
	${VTK_COMP_PREFIX}InteractionImage        # for vtkImageViewer2
	${VTK_COMP_PREFIX}InteractionWidgets      # for vtkScalarBarWidget/Representation
	${VTK_COMP_PREFIX}ImagingStatistics       # for vtkImageAccumulate
	${VTK_COMP_PREFIX}IOGeometry              # for vtkSTLReader/Writer
	${VTK_COMP_PREFIX}IOMovie                 # for vtkGenericMovieWriter
	${VTK_COMP_PREFIX}RenderingAnnotation     # for vtkAnnotatedCubeActor, vtkCaptionActor, vtkScalarBarActor
	${VTK_COMP_PREFIX}RenderingContext${VTK_RENDERING_BACKEND} # required, otherwise 3D renderer CRASHES somewhere with a nullptr access in vtkContextActor::GetDevice !!!
	${VTK_COMP_PREFIX}RenderingImage          # for vtkImageResliceMapper
	${VTK_COMP_PREFIX}RenderingVolume${VTK_RENDERING_BACKEND}  # for volume rendering
	${VTK_COMP_PREFIX}RenderingQt             # for vtkQImageToImageSource, also pulls in vtkGUISupportQt (for QVTKWidgetOpenGL)
	${VTK_COMP_PREFIX}ViewsContext2D          # for vtkContextView, vtkContextInteractorStyle
	${VTK_COMP_PREFIX}ViewsInfovis)           # for vtkGraphItem
IF (VTK_MAJOR_VERSION GREATER_EQUAL 9)
	LIST (APPEND VTK_COMPONENTS         # components not pulled in automatically anymore in VTK >= 9:
		ChartsCore                  # for vtkAxis, vtkChart, vtkChartParallelCoordinates, used in FeatureScout, FuzzyFeatureTracking, GEMSE, PorosityAnalyzer
		CommonColor                 # for vtkNamedColors, vtkColorSeries, used in CompVis
		CommonComputationalGeometry # for vtkParametricSpline, used in core - iASpline/iAParametricSpline
		FiltersExtraction           # for vtkExtractGeometry, used in FIAKER - iASelectionInteractorStyle
		FiltersGeometry             # for vtkImageDataGeometryFilter used in iALabel3D and vtkDataSetSurfaceFilter used in ExtractSurface - iAExtractSurfaceFilter
		FiltersHybrid               # for vtkDepthSortPolyData used in 4DCT, DreamCaster, FeatureScout, vtkPolyDataSilhouette used in FeatureScout
		FiltersStatistics           # for vtkDataSetSurfaceFilter used in BoneThickness - iABoneThickness
		GUISupportQt                # for QVTKOpenGLNativeWidget
		ImagingHybrid               # for vtkSampleFunction.h used in FeatureScout - iABlobCluster
		InfovisLayout               # for vtkGraphLayoutStrategy used in CompVis
		IOXML                       # for vtkXMLImageDataReader used in iAIO
	)
ENDIF()
IF ("${VTK_RENDERING_BACKEND}" STREQUAL "OpenGL2")
	add_compile_definitions(VTK_OPENGL2_BACKEND)
ELSE()
	IF (MSVC)
		ADD_COMPILE_OPTIONS(/wd4081)
	ENDIF()
	LIST (APPEND VTK_COMPONENTS vtkGUISupportQtOpenGL)    # for QVTKWidget2
ENDIF()
IF ("${vtkRenderingOSPRay_LOADED}")
	add_compile_definitions(VTK_OSPRAY_AVAILABLE)
ENDIF()

FUNCTION (ExtractVersion filename identifier output_varname)
	FILE (STRINGS "${filename}" MYLINE REGEX "${identifier}")
	string(FIND "${MYLINE}" "=" MYLINE_EQUAL)
	string(LENGTH "${MYLINE}" MYLINE_LENGTH)
	MATH(EXPR MYVER_START "${MYLINE_EQUAL}+2")
	MATH(EXPR MYVER_LENGTH "${MYLINE_LENGTH}-${MYVER_START}-2")
	STRING(SUBSTRING "${MYLINE}" ${MYVER_START} ${MYVER_LENGTH} version_value)
	SET (${output_varname} "${version_value}" PARENT_SCOPE)
ENDFUNCTION(ExtractVersion)

IF (vtkRenderingOpenVR_LOADED OR TARGET VTK::RenderingOpenVR)
	set (BUILD_INFO_VTK_VR_SUPPORT "enabled")
	LIST (APPEND VTK_COMPONENTS ${VTK_COMP_PREFIX}RenderingOpenVR)
	IF (VTK_MAJOR_VERSION LESS 9)
		STRING(FIND "${vtkRenderingOpenVR_INCLUDE_DIRS}" ";" semicolonpos REVERSE)
		MATH(EXPR aftersemicolon "${semicolonpos}+1")
		STRING(SUBSTRING "${vtkRenderingOpenVR_INCLUDE_DIRS}" ${aftersemicolon} -1 OpenVR_INCLUDE_DIR)
	# no else required as VTK >= 9 requires OpenVR_INCLUDE_DIR to be set anyway
	ENDIF()
	IF (EXISTS "${OpenVR_INCLUDE_DIR}/openvr.h")
		# Parse OpenVR version number:
		ExtractVersion("${OpenVR_INCLUDE_DIR}/openvr.h" "k_nSteamVRVersionMajor" OPENVR_VERSION_MAJOR)
		ExtractVersion("${OpenVR_INCLUDE_DIR}/openvr.h" "k_nSteamVRVersionMinor" OPENVR_VERSION_MINOR)
		ExtractVersion("${OpenVR_INCLUDE_DIR}/openvr.h" "k_nSteamVRVersionBuild" OPENVR_VERSION_BUILD)
	ENDIF()
	MESSAGE(STATUS "    OpenVR: ${OPENVR_VERSION_MAJOR}.${OPENVR_VERSION_MINOR}.${OPENVR_VERSION_BUILD} in ${OpenVR_INCLUDE_DIR} (include dir)")
	STRING(REGEX REPLACE "/headers" "" OPENVR_PATH ${OpenVR_INCLUDE_DIR})
	IF (WIN32)
		SET (OPENVR_LIB_PATH "${OPENVR_PATH}/bin/win64")
	ELSE ()
		SET (OPENVR_LIB_PATH "${OPENVR_PATH}/bin/linux64")
	ENDIF()
	LIST (APPEND BUNDLE_DIRS "${OPENVR_LIB_PATH}")
ELSE()
	set (BUILD_INFO_VTK_VR_SUPPORT "disabled")
	IF (VTK_MAJOR_VERSION LESS 8)
		SET (VTK_VR_OPTION_NAME "Module_vtkRenderingOpenVR")
	ELSE()
		SET (VTK_VR_OPTION_NAME "VTK_MODULE_ENABLE_VTK_RenderingOpenVR")
	ENDIF()
	MESSAGE(STATUS "    RenderingOpenVR: NOT available! Enable ${VTK_VR_OPTION_NAME} in VTK to make it available.")
ENDIF()
IF (VTK_MAJOR_VERSION GREATER 8)
	IF ("theora" IN_LIST VTK_AVAILABLE_COMPONENTS)
		LIST (APPEND VTK_COMPONENTS theora)
	ENDIF()
	IF ("ogg" IN_LIST VTK_AVAILABLE_COMPONENTS)
		LIST (APPEND VTK_COMPONENTS ogg)
	ENDIF()
	IF ("IOOggTheora" IN_LIST VTK_AVAILABLE_COMPONENTS)
		LIST (APPEND VTK_COMPONENTS IOOggTheora)
	ENDIF()
ENDIF()
FIND_PACKAGE(VTK COMPONENTS ${VTK_COMPONENTS})
IF (VTK_MAJOR_VERSION LESS 9)		# VTK >= 9.0 uses imported targets -> include directories are set by TARGET_LINK_LIBRARIES(... VTK_LIBRARIES) call!
	INCLUDE(${VTK_USE_FILE})
ENDIF()
IF (MSVC)
	SET (VTK_LIB_DIR "${VTK_DIR}/bin/Release")
ELSE ()
	if (EXISTS "${VTK_DIR}/lib")
		SET (VTK_LIB_DIR "${VTK_DIR}/lib")
	else()
		SET (VTK_LIB_DIR "${VTK_DIR}/../../../lib")
	endif()
ENDIF()
MESSAGE(STATUS "    VTK_LIB_DIR: ${VTK_LIB_DIR}")
LIST (APPEND BUNDLE_DIRS "${VTK_LIB_DIR}")
IF ( vtkoggtheora_LOADED OR vtkogg_LOADED OR
     (VTK_ogg_FOUND EQUAL 1 AND VTK_theora_FOUND EQUAL 1 AND VTK_IOOggTheora_FOUND EQUAL 1) )
	MESSAGE(STATUS "    Video: Ogg Theora Encoder available")
	SET (VTK_VIDEO_SUPPORT "ogg")
ELSE()
	MESSAGE(WARNING "    Video: No encoder available! You will not be able to record videos.")
	SET (VTK_VIDEO_SUPPORT "disabled")
ENDIF()
if (VTK_MAJOR_VERSION LESS 9)
	set(BUILD_INFO_VTK_DETAILS "Backend: ${VTK_RENDERING_BACKEND}, ")
endif ()
set (BUILD_INFO_VTK_DETAILS "${BUILD_INFO_VTK_DETAILS}OpenVR support: ${BUILD_INFO_VTK_VR_SUPPORT}, Video support: ${VTK_VIDEO_SUPPORT}")
set (BUILD_INFO_VTK "VTK: ${VTK_VERSION} (${BUILD_INFO_VTK_DETAILS})")
set (BUILD_INFO "${BUILD_INFO}    \"${BUILD_INFO_VTK}\\n\"\n")


# Qt (>= 5)
SET(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
SET(QT_USE_QTXML TRUE)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets OpenGLWidgets REQUIRED)
FIND_PACKAGE(Qt${QT_VERSION_MAJOR} COMPONENTS Concurrent Gui OpenGL Svg Widgets Xml REQUIRED)
MESSAGE(STATUS "Qt: ${QT_VERSION} in ${Qt${QT_VERSION_MAJOR}_DIR}")
set (BUILD_INFO "${BUILD_INFO}    \"Qt: ${QT_VERSION}\\n\"\n")
IF (QT_VERSION VERSION_LESS "5.9.0")
	MESSAGE(FATAL_ERROR "Your Qt version is too old. Please use Qt >= 5.9")
ENDIF()
INCLUDE_DIRECTORIES(${Qt${QT_VERSION_MAJOR}Widgets_INCLUDE_DIRS} ${Qt${QT_VERSION_MAJOR}OpenGL_INCLUDE_DIRS} )
SET(QT_LIBRARIES ${Qt${QT_VERSION_MAJOR}Core_LIBRARIES} ${Qt${QT_VERSION_MAJOR}Concurrent_LIBRARIES} ${Qt${QT_VERSION_MAJOR}OpenGL_LIBRARIES} ${Qt${QT_VERSION_MAJOR}Xml_LIBRARIES})

STRING(REGEX REPLACE "/lib/cmake/Qt${QT_VERSION_MAJOR}" "" Qt_BASEDIR ${Qt${QT_VERSION_MAJOR}_DIR})
STRING(REGEX REPLACE "/cmake/Qt${QT_VERSION_MAJOR}" "" Qt_BASEDIR ${Qt_BASEDIR})	# on linux, lib is omitted if installed from package repos

IF (WIN32)
	SET (QT_LIB_DIR "${Qt_BASEDIR}/bin")
ENDIF()
IF (UNIX AND NOT APPLE AND NOT FLATPAK_BUILD)
	IF (EXISTS "${Qt_BASEDIR}/lib")
		SET (QT_LIB_DIR "${Qt_BASEDIR}/lib")
	ELSE()
		SET (QT_LIB_DIR "${Qt_BASEDIR}")
	ENDIF()
ENDIF()

# Install svg imageformats plugin:
IF (FLATPAK_BUILD)
	# I guess plugins should all be available on Flatpak?
	#	INSTALL (FILES "$<TARGET_FILE:Qt5::QSvgPlugin>" DESTINATION bin/imageformats)
	#	INSTALL (FILES "$<TARGET_FILE:Qt5::QSvgIconPlugin>" DESTINATION bin/iconengines)
ELSE()
	IF (${QT_VERSION_MAJOR} GREATER_EQUAL 6) # Qt6 does not expose plugins? at least not the same as in Qt 5
		MESSAGE(STATUS "Qt: ${Qt_BASEDIR}")
		set (LIB_SvgIconPlugin "${Qt_BASEDIR}/plugins/iconengines/${CMAKE_SHARED_LIBRARY_PREFIX}qsvgicon${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set (LIB_SvgPlugin "${Qt_BASEDIR}/plugins/imageformats/${CMAKE_SHARED_LIBRARY_PREFIX}qsvg${CMAKE_SHARED_LIBRARY_SUFFIX}")
		INSTALL (FILES "${LIB_SvgIconPlugin}" DESTINATION iconengines)
		LIST (APPEND BUNDLE_LIBS "${LIB_SvgIconPlugin}")
		INSTALL (FILES "${LIB_SvgPlugin}" DESTINATION imageformats)
		LIST (APPEND BUNDLE_LIBS "${LIB_SvgPlugin}")
	ELSE() # use imported targets & generator expressions:
		INSTALL (FILES "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QSvgIconPlugin>" DESTINATION iconengines)
		LIST (APPEND BUNDLE_LIBS "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QSvgIconPlugin>")
		INSTALL (FILES "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QSvgPlugin>" DESTINATION imageformats)
		LIST (APPEND BUNDLE_LIBS "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QSvgPlugin>")
	ENDIF()
ENDIF()
# on windows, windows platform and vista style plugins are required:
IF (WIN32)
	IF (${QT_VERSION_MAJOR} GREATER_EQUAL 6) # Qt6 does not expose plugins? at least not the same as in Qt 5
		set (LIB_WindowsPlatform "${Qt_BASEDIR}/plugins/platforms/qwindows.dll")
		set (LIB_WindowsVistaStyle "${Qt_BASEDIR}/plugins/styles/qwindowsvistastyle.dll")
		INSTALL (FILES "${LIB_WindowsPlatform}" DESTINATION platforms)
		INSTALL (FILES "${LIB_WindowsVistaStyle}" DESTINATION styles)
	ELSE() # use imported targets & generator expressions:
		INSTALL (FILES "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QWindowsIntegrationPlugin>" DESTINATION platforms)
		INSTALL (FILES "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QWindowsVistaStylePlugin>" DESTINATION styles)
	ENDIF()
ENDIF()
# on linux/unix, xcb platform plugin, and its plugins egl and glx are required:
IF (UNIX AND NOT APPLE AND NOT FLATPAK_BUILD)
	IF (${QT_VERSION_MAJOR} GREATER_EQUAL 6) # Qt6 does not expose plugins? at least not the same as in Qt 5
		set (LIB_XcbPlatform "${Qt_BASEDIR}/plugins/platforms/libqxcb.so")
		set (LIB_XcbEglIntegration "${Qt_BASEDIR}/plugins/xcbglintegrations/libqxcb-egl-integration.so")
		set (LIB_XcbGlxIntegration "${Qt_BASEDIR}/plugins/xcbglintegrations/libqxcb-glx-integration.so")
		INSTALL (FILES "${LIB_XcbPlatform}" DESTINATION platforms)
		INSTALL (FILES "${LIB_XcbEglIntegration}" DESTINATION xcbglintegrations)
		INSTALL (FILES "${LIB_XcbGlxIntegration}" DESTINATION xcbglintegrations)
	ELSE()
		INSTALL (FILES "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QXcbIntegrationPlugin>" DESTINATION platforms)
		INSTALL (FILES "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QXcbEglIntegrationPlugin>" DESTINATION xcbglintegrations)
		INSTALL (FILES "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QXcbGlxIntegrationPlugin>" DESTINATION xcbglintegrations)
	ENDIF()

	# install icu:
	# TODO: find out whether Qt was built with icu library dependencies
	# (typically only the case if webengine/webkit were included); but there
	# doesn't seem to be any CMake variable exposing this...
	SET(ICU_LIBS icudata icui18n icuuc)
	FOREACH(ICU_LIB ${ICU_LIBS})
		SET (ICU_LIB_LINK ${QT_LIB_DIR}/lib${ICU_LIB}.so)
		get_filename_component(ICU_SHAREDLIB "${ICU_LIB_LINK}" REALPATH)
		get_filename_component(ICU_SHAREDLIB_NAMEONLY "${ICU_SHAREDLIB}" NAME)
		STRING(LENGTH "${ICU_SHAREDLIB_NAMEONLY}" ICULIB_STRINGLEN)
		MATH(EXPR ICULIB_STRINGLEN "${ICULIB_STRINGLEN}-2")
		STRING(SUBSTRING "${ICU_SHAREDLIB_NAMEONLY}" 0 ${ICULIB_STRINGLEN} ICU_SHAREDLIB_DST)
		IF (EXISTS "${ICU_SHAREDLIB}")
			INSTALL (FILES "${ICU_SHAREDLIB}" DESTINATION . RENAME "${ICU_SHAREDLIB_DST}")
		ENDIF()
	ENDFOREACH()
ENDIF()
LIST (APPEND BUNDLE_DIRS "${QT_LIB_DIR}")


# Eigen
FIND_PACKAGE(Eigen3)
IF (EIGEN3_FOUND)
	MESSAGE(STATUS "Eigen: ${EIGEN3_VERSION} in ${EIGEN3_INCLUDE_DIR}")
	set (BUILD_INFO "${BUILD_INFO}    \"Eigen: ${EIGEN3_VERSION}\\n\"\n")
ENDIF()


# HDF5
# ToDo: Check for whether hdf5 is build as shared or static library,
# prefer static but also enable utilization of shared?
FIND_PACKAGE(HDF5 NAMES hdf5 COMPONENTS C NO_MODULE QUIET)

IF (HDF5_FOUND)
	SET (HDF5_CORE_LIB_NAME libhdf5${CMAKE_STATIC_LIBRARY_SUFFIX})
	SET (HDF5_SZIP_LIB_NAME libszip${CMAKE_STATIC_LIBRARY_SUFFIX})
	IF (WIN32)
		SET (HDF5_Z_LIB_NAME libzlib.lib)
	ELSE()
		SET (HDF5_Z_LIB_NAME libz.a)
	ENDIF()
	FIND_PATH(HDF5_INCLUDE_OVERWRITE_DIR hdf5.h PATHS "${HDF5_DIR}/../../include" "${HDF5_DIR}/../../../include")
	SET(HDF5_INCLUDE_DIR "${HDF5_INCLUDE_OVERWRITE_DIR}" CACHE PATH "" FORCE)
	UNSET(HDF5_INCLUDE_OVERWRITE_DIR CACHE)
	FIND_LIBRARY(HDF5_CORE_LIB ${HDF5_CORE_LIB_NAME} PATHS ${HDF5_DIR}/../../lib ${HDF5_DIR}/../../../lib)
	FIND_LIBRARY(HDF5_Z_LIB ${HDF5_Z_LIB_NAME} PATHS ${HDF5_DIR}/../../lib ${HDF5_DIR}/../../../lib NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
	FIND_LIBRARY(HDF5_SZIP_LIB ${HDF5_SZIP_LIB_NAME} PATHS ${HDF5_DIR}/../../lib ${HDF5_DIR}/../../../lib)
	SET (HDF5_LIBRARY ${HDF5_CORE_LIB} ${HDF5_CORE_HL_LIB} ${HDF5_TOOL_LIB} ${HDF5_SZIP_LIB} ${HDF5_Z_LIB} CACHE STRING "" FORCE)
	UNSET(HDF5_Z_LIB CACHE)
	UNSET(HDF5_SZIP_LIB CACHE)
	UNSET(HDF5_CORE_LIB CACHE)
	MESSAGE(STATUS "HDF5: ${HDF5_VERSION} in ${HDF5_DIR}.")
	set (BUILD_INFO "${BUILD_INFO}    \"HDF5: ${HDF5_VERSION}\\n\"\n")
ELSE()
	MESSAGE(STATUS "HDF5: Not found.")
ENDIF()


# Astra Toolbox
FIND_PACKAGE(AstraToolbox)
IF (ASTRA_TOOLBOX_FOUND)
	STRING(FIND "${ASTRA_TOOLBOX_DIR}" "-" ASTRA_DASH_POS REVERSE)
	SET (ASTRA_VERSION "unknown version")
	IF (${ASTRA_DASH_POS} GREATER -1)
		MATH(EXPR ASTRA_DASH_POS "${ASTRA_DASH_POS} + 1")
		STRING(SUBSTRING "${ASTRA_TOOLBOX_DIR}" ${ASTRA_DASH_POS} -1 ASTRA_VERSION)
	ENDIF()
	MESSAGE(STATUS "Astra Toolbox: ${ASTRA_VERSION} in ${ASTRA_TOOLBOX_DIR}.")
	set (BUILD_INFO "${BUILD_INFO}    \"Astra: ${ASTRA_VERSION}\\n\"\n")
	IF (WIN32)
		SET (ASTRA_LIB_DIR "${ASTRA_TOOLBOX_DIR}/bin/x64/Release_CUDA")
		IF (NOT EXISTS "${ASTRA_LIB_DIR}/AstraCuda64.dll")
			get_filename_component(ASTRA_LIB_DIR "${ASTRA_TOOLBOX_LIBRARIES_RELEASE}" DIRECTORY)
		ENDIF()
		IF (NOT EXISTS "${ASTRA_LIB_DIR}/AstraCuda64.dll")
			MESSAGE(WARNING "AstraCuda64.dll not found!")
		ENDIF()
	ELSEIF(UNIX AND NOT APPLE)
		get_filename_component(ASTRA_LIB_DIR "${ASTRA_TOOLBOX_LIBRARIES_RELEASE}" DIRECTORY)
	ENDIF ()
	LIST (APPEND BUNDLE_DIRS "${ASTRA_LIB_DIR}")
ENDIF()


# OpenCL
FIND_PACKAGE(OpenCL)
IF (OPENCL_FOUND)

	set (openiA_OPENCL_VERSION_OPTIONS "1.1.0" "1.2.0" "2.0.0" "2.1.0"  "2.2.0")
	list (FIND openiA_OPENCL_VERSION_OPTIONS "${openiA_OPENCL_VERSION}" opencl_version_index)
	if (${opencl_version_index} EQUAL -1)
		set (openiA_OPENCL_VERSION_DEFAULT "1.2.0")
		if (DEFINED openiA_OPENCL_VERSION)
			MESSAGE(WARNING "Invalid openiA_OPENCL_VERSION, resetting to default ${openiA_OPENCL_VERSION_DEFAULT}!")
		endif()
		set (openiA_OPENCL_VERSION "${openiA_OPENCL_VERSION_DEFAULT}" CACHE STRING "The version of OpenCL to target (default: ${openiA_OPENCL_VERSION_DEFAULT})" FORCE)
		set_property(CACHE openiA_OPENCL_VERSION PROPERTY STRINGS ${openiA_OPENCL_VERSION_OPTIONS})
	endif()
	string(REPLACE "." "" CL_TARGET_OPENCL_VERSION "${openiA_OPENCL_VERSION}")
	add_library(OpenCL INTERFACE)
	target_compile_definitions(OpenCL INTERFACE __CL_ENABLE_EXCEPTIONS
		CL_HPP_TARGET_OPENCL_VERSION=${CL_TARGET_OPENCL_VERSION}
		CL_TARGET_OPENCL_VERSION=${CL_TARGET_OPENCL_VERSION})
	target_link_libraries(OpenCL INTERFACE ${OPENCL_LIBRARIES})
	target_include_directories(OpenCL INTERFACE ${OPENCL_INCLUDE_DIRS} ${Toolkit_DIR}/OpenCL)
	MESSAGE(STATUS "OpenCL: include=${OPENCL_INCLUDE_DIRS}, libraries=${OPENCL_LIBRARIES}.")
	set (BUILD_INFO "${BUILD_INFO}    \"OpenCL targeted version: ${openiA_OPENCL_VERSION}\\n\"\n")
	IF (WIN32)
		# Find path of OpenCL.dll to include in release:
		get_filename_component(OPENCL_LIB_DIR "${OPENCL_LIBRARIES}" DIRECTORY)
		get_filename_component(OPENCL_LIB_BASENAME "${OPENCL_LIBRARIES}" NAME_WE)
		
		IF (EXISTS "${OPENCL_LIB_DIR}/${OPENCL_LIB_BASENAME}.dll")
			LIST (APPEND BUNDLE_DIRS "${OPENCL_LIB_DIR}")
		ELSE()
			STRING(REGEX REPLACE "lib" "bin" OPENCL_BIN_DIR "${OPENCL_LIB_DIR}")
			IF (EXISTS "${OPENCL_BIN_DIR}/${OPENCL_LIB_BASENAME}.dll")
				LIST (APPEND BUNDLE_DIRS "${OPENCL_BIN_DIR}")
			ELSE()
				MESSAGE(STATUS "Directory containing ${OPENCL_LIB_BASENAME}.dll was not found. You can continue building, but the program might not run (or it might fail to run when installed/cpacked).")
			ENDIF()
		ENDIF()
	ELSEIF (UNIX AND NOT APPLE)
		# typically OPENCL_LIBRARIES will only contain the one libOpenCL.so anyway, FOREACH just to make sure
		FOREACH(OPENCL_LIB ${OPENCL_LIBRARIES})
			get_filename_component(OPENCL_LIB_DIR "${OPENCL_LIB}" DIRECTORY)
			LIST (APPEND BUNDLE_DIRS "${OPENCL_LIB_DIR}")
		ENDFOREACH()
	ENDIF()
ENDIF()


# CUDA:
FIND_PACKAGE(CUDA)
IF (CUDA_FOUND)
	MESSAGE(STATUS "CUDA: ${CUDA_VERSION} in ${CUDA_TOOLKIT_ROOT_DIR}.")
	set (BUILD_INFO "${BUILD_INFO}    \"CUDA: ${CUDA_VERSION}\\n\"\n")
	IF (WIN32)
		SET (CUDA_LIB_DIR ${CUDA_TOOLKIT_ROOT_DIR}/bin)
	ELSEIF(UNIX AND NOT APPLE)
		get_filename_component(CUDA_LIB_DIR "${CUDA_CUDART_LIBRARY}" DIRECTORY)
		get_filename_component(CUFFT_LIB_DIR "${CUDA_cufft_LIBRARY}" DIRECTORY)
		IF (NOT "${CUDA_LIB_DIR}" STREQUAL "${CUFFT_LIB_DIR}")
			MESSAGE(STATUS "CudaRT / CuFFT libs in different folders!")
			LIST (APPEND BUNDLE_DIRS "${CUFFT_LIB_DIR}")
		ENDIF()
	ENDIF()
	LIST (APPEND BUNDLE_DIRS "${CUDA_LIB_DIR}")
ENDIF()


# OpenMP
INCLUDE(${CMAKE_ROOT}/Modules/FindOpenMP.cmake)
# Above imports the target 'OpenMP::OpenMP_CXX'; only for CMake >= 3.9,
# but project-wide we require a higher version already anyway!


#-------------------------
# Compiler Flags
#-------------------------

set (openiA_AVX_SUPPORT_DISABLED "disabled")
set (openiA_AVX_SUPPORT_OPTIONS "${openiA_AVX_SUPPORT_DISABLED}" "AVX" "AVX2")
list (FIND openiA_AVX_SUPPORT_OPTIONS "${openiA_AVX_SUPPORT}" avx_support_index)
if (${avx_support_index} EQUAL -1)
	set (openiA_AVX_SUPPORT_DEFAULT "AVX")
	if (DEFINED openiA_AVX_SUPPORT)
		MESSAGE(WARNING "Invalid openiA_AVX_SUPPORT, resetting to default ${openiA_AVX_SUPPORT_DEFAULT}!")
	endif()
	set (openiA_AVX_SUPPORT "${openiA_AVX_SUPPORT_DEFAULT}" CACHE STRING
		"AVX extensions to enable (default: ${openiA_AVX_SUPPORT_DEFAULT})." FORCE)
	set_property(CACHE openiA_AVX_SUPPORT PROPERTY STRINGS ${openiA_AVX_SUPPORT_OPTIONS})
endif()
set (BUILD_INFO "${BUILD_INFO}    \"Advanced Vector Extensions support: ${openiA_AVX_SUPPORT}\\n\"\n")

#MESSAGE(STATUS "Aiming for C++20 support.")
#SET(CMAKE_CXX_STANDARD 20)
# Enabling C++20 can cause problems as e.g. ITK 5.0.1 is not yet fully C++20 compatible!
MESSAGE(STATUS "Aiming for C++17 support.")
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_EXTENSIONS OFF)
# use CMAKE_CXX_STANDARD_REQUIRED? e.g.:
# SET (CMAKE_CXX_STANDARD 11)
# SET (CMAKE_CXX_STANDARD_REQUIRED ON)
IF (MSVC)
	# /bigobj            increase the number of sections in .obj file (65,279 -> 2^32), exceeded by some compilations
	# /Zc:__cplusplus    set correct value in __cplusplus macro (https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus)
	# /MP                enable multi-processor compilation
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj /Zc:__cplusplus")
	IF (MSVC_VERSION GREATER_EQUAL 1910)
		# specify standard conformance mode (https://docs.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
	ENDIF()

	# Reduce size of .pdb files:
	OPTION (openiA_COMPRESS_PDB "Whether to compress .pdb files to conserve disk space. Default: enabled." ON)
	IF (openiA_COMPRESS_PDB)
		# significantly reduces size of .pdb files (89 -> 28 MB):
		SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /PDBCOMPRESS")
		# only slightly decrease build sizes (89 -> 80 MB), and disables incremental linking:
		#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /OPT:REF /OPT:ICF")
	ENDIF()

	if (NOT "${openiA_AVX_SUPPORT}" STREQUAL "${openiA_AVX_SUPPORT_DISABLED}")
		ADD_COMPILE_OPTIONS(/arch:${openiA_AVX_SUPPORT})
	endif()
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS
		_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING	# silence warnings when compiling VTK (<= 9.0.1) with C++17
	)
	
	# enable all warnings, disable selected:
	ADD_COMPILE_OPTIONS(/W4 /wd4068 /wd4127 /wd4251 /wd4515)
	# disabled: C4068 - "unknown pragma - ignoring a pragma"
	#           C4127 - caused by QVector
	#           C4251 - "class requires dll interface"
	#           C4515 - "namespace uses itself" - caused by ITK/gdcm
ELSE()
	# enable all warnings:
	ADD_COMPILE_OPTIONS(-Wall -Wextra) # with -Wpedantic, lots of warnings about extra ';' in VTK/ITK code...
ENDIF()

# check: are CMAKE_C_FLAGS really required or are CMAKE_CXX_FLAGS alone enough?
IF (CMAKE_COMPILER_IS_GNUCXX)
	IF ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3")
		SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3")
	ENDIF()
ENDIF()

IF (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# Make sure at least C++ 0x is supported:
	# check if that is required with CMAKE_CXX_STANDARD definition above!
	INCLUDE (CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
	IF (NOT COMPILER_SUPPORTS_CXX0X)
		MESSAGE(WARNING "The used compiler ${CMAKE_CXX_COMPILER} has no C++0x/11 support. Please use a newer C++ compiler.")
	ENDIF()

	set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -fpermissive -fopenmp -march=core2 -O2 -msse4.2")
	set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -fopenmp -march=core2 -O2 -msse4.2")

	if (NOT "${openiA_AVX_SUPPORT}" STREQUAL "${openiA_AVX_SUPPORT_DISABLED}")
		string(TOLOWER "${openiA_AVX_SUPPORT}" openiA_AVX_SUPPORT_LOWER)
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m${openiA_AVX_SUPPORT_LOWER}")
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m${openiA_AVX_SUPPORT_LOWER}")
	endif()

	# we do need to set the RPATH to make lib load path recursive also be able to load dependent libraries from the rpath specified in the executables:
	# see https://stackoverflow.com/questions/58997230/cmake-project-fails-to-find-shared-library
	# strictly speaking, this is only needed for running the executables from the project folder
	# (as in an install, the RPATH of all installed executables and libraries is adapted anyway)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")
ENDIF()

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Mac OS X specific code
	MESSAGE (WARNING "You are using MacOS - note that we do not regularly build on Mac OS, expect there to be some errors.")

	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Cocoa -framework OpenGL")
ENDIF()


#-------------------------
# Visual Studio dll Paths
#-------------------------
IF (MSVC)
	# Set up debugging/running environments	in Visual Studio to point to the correct dll files:
	STRING(REGEX REPLACE "/" "\\\\" VTK_WIN_DIR ${VTK_DIR})
	STRING(REGEX REPLACE "/" "\\\\" ITK_WIN_DIR ${ITK_DIR})
	STRING(REGEX REPLACE "/" "\\\\" Qt_WIN_DIR ${QT_LIB_DIR})
	SET (WinDLLPaths "${VTK_WIN_DIR}\\bin\\$(Configuration);${ITK_WIN_DIR}\\bin\\$(Configuration);${Qt_WIN_DIR}")
	
	IF (OPENCL_FOUND AND EXISTS "${OPENCL_DLL}")
		STRING(REGEX REPLACE "/OpenCL.dll" "" OPENCL_WIN_DIR ${OPENCL_DLL})
		STRING(REGEX REPLACE "/" "\\\\" OPENCL_WIN_DIR ${OPENCL_WIN_DIR})
		SET (WinDLLPaths "${OPENCL_WIN_DIR};${WinDLLPaths}")
	ENDIF()

	IF (CUDA_FOUND)
		STRING(REGEX REPLACE "/" "\\\\" CUDA_WIN_DIR ${CUDA_TOOLKIT_ROOT_DIR})
		SET (WinDLLPaths "${CUDA_WIN_DIR}\\bin;${WinDLLPaths}")
	ENDIF()

	IF (ITK_USE_SYSTEM_FFTW)
		SET (WinDLLPaths "${ITK_FFTW_LIBDIR};${WinDLLPaths}")
	ENDIF(ITK_USE_SYSTEM_FFTW)

	IF (ASTRA_TOOLBOX_FOUND)
		STRING(FIND ${ASTRA_TOOLBOX_LIBRARIES_RELEASE} "/" ASTRA_RELEASE_LIB_LASTSLASHPOS REVERSE)
		STRING(SUBSTRING ${ASTRA_TOOLBOX_LIBRARIES_RELEASE} 0 ${ASTRA_RELEASE_LIB_LASTSLASHPOS} ASTRA_LIBRARIES_RELEASE_PATH)
		STRING(REGEX REPLACE "/" "\\\\" ASTRA_LIBRARIES_RELEASE_WIN_PATH ${ASTRA_LIBRARIES_RELEASE_PATH})
		SET (WinDLLPaths "${ASTRA_LIBRARIES_RELEASE_WIN_PATH};${WinDLLPaths}")
	ENDIF()

	IF (NOT "${ITKZLIB_LIBRARIES}" STREQUAL "itkzlib")
		STRING (FIND "${ITKZLIB_LIBRARIES}" ";" SEMICOLONPOS)
		IF (SEMICOLONPOS EQUAL -1)
			SET(ZLIB_LIBRARY_RELEASE "${ITKZLIB_LIBRARIES}")
			SET(ZLIB_LIBRARY_DEBUG "${ITKZLIB_LIBRARIES}")
		ELSE()
			LIST (GET ITKZLIB_LIBRARIES 1 ZLIB_LIBRARY_RELEASE)
			LIST (GET ITKZLIB_LIBRARIES 3 ZLIB_LIBRARY_DEBUG)
		ENDIF()
		STRING(FIND ${ZLIB_LIBRARY_RELEASE} "/" ZLIBRELLIB_LASTSLASHPOS REVERSE)
		STRING(SUBSTRING ${ZLIB_LIBRARY_RELEASE} 0 ${ZLIBRELLIB_LASTSLASHPOS} ZLIB_REL_LIB_DIR)
		STRING(FIND ${ZLIB_LIBRARY_DEBUG} "/" ZLIBDBGLIB_LASTSLASHPOS REVERSE)
		STRING(SUBSTRING ${ZLIB_LIBRARY_DEBUG} 0 ${ZLIBDBGLIB_LASTSLASHPOS} ZLIB_DBG_LIB_DIR)
		MESSAGE(STATUS "ITK was built with system zlib, adding paths to dll. Release: ${ZLIB_REL_LIB_DIR}, Debug: ${ZLIB_DBG_LIB_DIR}")
		SET (WinDLLPaths "${ZLIB_REL_LIB_DIR};${WinDLLPaths}")
	ENDIF()

	IF (HDF5_FOUND)
		STRING(REGEX REPLACE "/cmake/hdf5" "" HDF5_BASE_DIR ${HDF5_DIR})
		STRING(REGEX REPLACE "/" "\\\\" HDF5_BASE_DIR ${HDF5_BASE_DIR})
		IF (EXISTS "${HDF5_BASE_DIR}\\bin\\Release")
			SET (WinDLLPaths "${HDF5_BASE_DIR}\\bin\\$(Configuration);${WinDLLPaths}")
		ELSE()
			SET (WinDLLPaths "${HDF5_BASE_DIR}\\bin;${WinDLLPaths}")
		ENDIF()
	ENDIF()

	IF (vtkRenderingOpenVR_LOADED OR TARGET VTK::RenderingOpenVR)
		STRING(REGEX REPLACE "/" "\\\\" OPENVR_PATH_WIN ${OPENVR_LIB_PATH})
		SET (WinDLLPaths "${OPENVR_PATH_WIN};${WinDLLPaths}")
	ENDIF()

	IF (ONNX_RUNTIME_LIBRARIES)
		get_filename_component(ONNX_LIB_DIR ${ONNX_RUNTIME_LIBRARIES} DIRECTORY)
		STRING(REGEX REPLACE "/" "\\\\" ONNX_LIB_WIN_DIR ${ONNX_LIB_DIR})
		SET (WinDLLPaths "${ONNX_LIB_WIN_DIR};${WinDLLPaths}")
	ENDIF()

	STRING(REGEX REPLACE "/" "\\\\" CMAKE_BINARY_WIN_DIR ${CMAKE_BINARY_DIR})
ENDIF ()

IF (ONNX_RUNTIME_LIBRARIES)
	get_filename_component(ONNX_LIB_DIR ${ONNX_RUNTIME_LIBRARIES} DIRECTORY)
	LIST (APPEND BUNDLE_DIRS "${ONNX_LIB_DIR}")
ENDIF()


#-------------------------
# Common Settings
#-------------------------

option (openiA_USE_IDE_FOLDERS "Whether to group projects in subfolders in the IDE (mainly Visual Studio). Default: enabled." ON)
IF (openiA_USE_IDE_FOLDERS)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_CMake")
ENDIF()

# open_iA Version number
include(GetGitRevisionDescription)
git_describe(openiA_VERSION openiA_HASH --tags)
MESSAGE(STATUS "Build version: ${openiA_VERSION}")
set (BUILD_INFO "${BUILD_INFO}    \"git revision: ${openiA_HASH}\\n\"\n")

add_compile_definitions(UNICODE _UNICODE)    # Enable Unicode

IF (UNIX)
    SET(CMAKE_INSTALL_RPATH "\$ORIGIN")      # Set RunPath in all created libraries / executables to $ORIGIN
    #    SET (CMAKE_BUILD_RPATH_USE_ORIGIN ON)
ENDIF()


# Helper functions for adding libraries

# "old style" libraries (e.g. ITK or VTK < 9, with no imported targets)
# -> not working like this, since we have to use (I/V)TK_USE_FILE anyway for module autoinitialization
#FUNCTION (ADD_LEGACY_LIBRARIES libname libprefix pubpriv liblist)
#	FOREACH(lib ${liblist})
#		set (fulllib "${libprefix}${lib}")
#		IF (openiA_DEPENDENCY_INFO)
#			MESSAGE(STATUS "    ${fulllib} - libs: ${${fulllib}_LIBRARIES}, include: ${${fulllib}_INCLUDE_DIRS}")
#		ENDIF()
#		TARGET_INCLUDE_DIRECTORIES(${libname} ${pubpriv} ${${fulllib}_INCLUDE_DIRS})
#		TARGET_LINK_LIBRARIES(${libname} ${pubpriv} ${${fulllib}_LIBRARIES})
#	ENDFOREACH()
#ENDFUNCTION()

# "new style" libraries that bring in all dependencies automatically, and that only need to be linked to
FUNCTION (ADD_IMPORTEDTARGET_LIBRARIES libname libprefix pubpriv liblist)
	FOREACH(lib ${liblist})
		set (fulllib "${libprefix}${lib}")
		IF (openiA_DEPENDENCY_INFO)
			MESSAGE(STATUS "    ${fulllib}")
		ENDIF()
		TARGET_LINK_LIBRARIES(${libname} ${pubpriv} ${fulllib})
	ENDFOREACH()
ENDFUNCTION()

FUNCTION (ADD_VTK_LIBRARIES libname pubpriv liblist)
	IF (VTK_VERSION VERSION_LESS "9.0.0")
		#LIST (APPEND liblist ${VTK_BASE_LIB_LIST})
		#ADD_LEGACY_LIBRARIES(${libname} ${VTK_LIB_PREFIX} ${pubpriv} "${liblist}")
	ELSE()
		ADD_IMPORTEDTARGET_LIBRARIES(${libname} ${VTK_LIB_PREFIX} ${pubpriv} "${liblist}")
	ENDIF()
ENDFUNCTION()
