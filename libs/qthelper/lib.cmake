TARGET_LINK_LIBRARIES(${libname} PUBLIC	iAbase)
if ("${VTK_RENDERING_BACKEND}" STREQUAL "OpenGL")
	TARGET_INCLUDE_DIRECTORIES(${libname} PUBLIC ${Qt${QT_VERSION_MAJOR}OpenGL_INCLUDE_DIRS})
endif()
if (openiA_OPENGL_DEBUG)
	TARGET_COMPILE_DEFINITIONS(${libname} PRIVATE OPENGL_DEBUG)
endif()
