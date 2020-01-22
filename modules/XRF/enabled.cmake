SET (SPECTRA_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/refSpectra.7z")
FOREACH(cfg ${CMAKE_CONFIGURATION_TYPES})
	STRING (TOUPPER "${cfg}" CFG)
	SET (SPECTRA_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG}}/refSpectra")
	IF (NOT EXISTS "${SPECTRA_DIR}")
		FILE(MAKE_DIRECTORY "${SPECTRA_DIR}")
		INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/extractSpectra.cmake")
	ENDIF()
ENDFOREACH()

INSTALL(DIRECTORY DESTINATION refSpectra)
INSTALL(CODE "
	SET(SPECTRA_ARCHIVE \"${SPECTRA_ARCHIVE}\")
	SET(SPECTRA_DIR \"\${CMAKE_INSTALL_PREFIX}/refSpectra\")
	INCLUDE(\"${CMAKE_CURRENT_SOURCE_DIR}/extractSpectra.cmake\")
")