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
	)
ENDIF()

set(SOURCES
	PyTschirp.h
	PyTschirpAttribute.h
	PyTschirpSynth.h
	pytschirp.cpp
)

pybind11_add_module(pytschirp ${SOURCES})
target_link_libraries(pytschirp PRIVATE juce-utils midikraft-base midikraft-sequential-rev2 ${JUCE_LIBRARIES})
set_target_properties(pytschirp PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}"
                                         SUFFIX "${PYTHON_MODULE_EXTENSION}")

add_executable(testExe test.cpp)
target_link_libraries(testExe PRIVATE juce-utils midikraft-base midikraft-sequential-rev2 ${JUCE_LIBRARIES})
