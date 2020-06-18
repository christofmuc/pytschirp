#
#  Copyright (c) 2019 Christof Ruch. All rights reserved.
#
#  Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
#

cmake_minimum_required(VERSION 3.14)

project(PyTschirp)

IF(NOT DEFINED JUCE_LIBRARIES)
	SET(JUCE_DIR_HINT "${CMAKE_CURRENT_LIST_DIR}/../third_party")
	find_package(JUCE REQUIRED 
		COMPONENTS 
			juce_core		
			juce_events
			juce_audio_basics
			juce_audio_devices
			juce_data_structures
			juce_gui_basics # Colour
			juce_cryptography # MD5
	)
ENDIF()
IF (NOT DEFINED JUCE_LINUX_LINK_LIBRARIES)
	# They normally should be set by the top level module
	set(LINUX_JUCE_LINK_LIBRARIES
		PkgConfig::WEBKIT 
		PkgConfig::GTK 
		PkgConfig::GLEW 
		Xext 
		X11 
		pthread 
		${CMAKE_DL_LIBS} 
		freetype 
		curl 
		asound)
ENDIF()

set(SOURCES
	PyTschirpPatch.cpp PyTschirpPatch.h
	PyTschirpAttribute.cpp PyTschirpAttribute.h
	PyTschirpSynth.cpp PyTschirpSynth.h
)

set(SYNTHMODULES
	midikraft-kawai-k3
	midikraft-sequential-rev2
)

add_library(pytschirplib ${SOURCES})
IF(UNIX)
	# Required for the Linux version of the library to avoid a linker error when building the python module
	set_property(TARGET pytschirplib PROPERTY POSITION_INDEPENDENT_CODE ON) 
ENDIF()
target_link_libraries(pytschirplib PRIVATE pybind11 juce-utils midikraft-base midikraft-librarian ${APPLE_BOOST})
target_include_directories(pytschirplib PRIVATE ${JUCE_INCLUDES})

add_library(pytschirp_embedded embedded_module.cpp embedded_module.h)
target_link_libraries(pytschirp_embedded PRIVATE pybind11 pytschirplib juce-utils midikraft-base midikraft-librarian ${SYNTHMODULES} ${APPLE_BOOST})
target_include_directories(pytschirp_embedded PUBLIC ${CMAKE_CURRENT_LIST_DIR} PRIVATE ${JUCE_INCLUDES})

pybind11_add_module(pytschirp pytschirp.cpp)
set_target_properties(pytschirp PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}"
                                         SUFFIX "${PYTHON_MODULE_EXTENSION}")
IF(WIN32)
	target_link_libraries(pytschirp PRIVATE pytschirplib juce-utils midikraft-base midikraft-librarian ${SYNTHMODULES} ${JUCE_LIBRARIES})
ELSEIF(APPLE)
	target_link_libraries(pytschirp PRIVATE pytschirplib juce-utils midikraft-base midikraft-librarian ${SYNTHMODULES} ${JUCE_LIBRARIES} ${APPLE_BOOST})
ELSE()
	target_link_libraries(pytschirp PRIVATE 
		pytschirplib juce-utils midikraft-base midikraft-librarian 
		${SYNTHMODULES} 
		${JUCE_LIBRARIES}
		${LINUX_JUCE_LINK_LIBRARIES}
		)
ENDIF()

add_executable(testExe test.cpp)
IF(WIN32)	
	target_link_libraries(testExe PRIVATE juce-utils midikraft-base ${SYNTHMODULES} ${JUCE_LIBRARIES})
ELSEIF(APPLE)
	target_link_libraries(testExe PRIVATE juce-utils midikraft-base ${SYNTHMODULES} ${JUCE_LIBRARIES})
ELSE()
	target_link_libraries(testExe PRIVATE juce-utils midikraft-base ${SYNTHMODULES} ${JUCE_LIBRARIES} ${LINUX_JUCE_LINK_LIBRARIES})
ENDIF()

