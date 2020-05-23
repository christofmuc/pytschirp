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
target_link_libraries(pytschirplib PRIVATE pybind11 juce-utils midikraft-base midikraft-librarian)
target_include_directories(pytschirplib PRIVATE ${JUCE_INCLUDES})

add_library(pytschirp_embedded embedded_module.cpp embedded_module.h)
target_link_libraries(pytschirp_embedded PRIVATE pybind11 pytschirplib juce-utils midikraft-base midikraft-librarian ${SYNTHMODULES})
target_include_directories(pytschirp_embedded PUBLIC ${CMAKE_CURRENT_LIST_DIR} PRIVATE ${JUCE_INCLUDES})

pybind11_add_module(pytschirp pytschirp.cpp)
target_link_libraries(pytschirp PRIVATE pytschirplib juce-utils midikraft-base midikraft-librarian ${SYNTHMODULES} ${JUCE_LIBRARIES})
set_target_properties(pytschirp PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}"
                                         SUFFIX "${PYTHON_MODULE_EXTENSION}")

add_executable(testExe test.cpp)

IF(WIN32)
	target_link_libraries(testExe PRIVATE juce-utils midikraft-base ${SYNTHMODULES} ${JUCE_LIBRARIES})
ENDIF()

