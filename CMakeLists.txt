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
	PyTschirp-sequential-rev2.cpp
)

add_library(pytschirp ${SOURCES})
target_link_libraries(pytschirp ${SOURCES} juce-utils midikraft-base midikraft-sequential-rev2 ${JUCE_LIBRARIES} pybind11::module)
set_target_properties(pytschirp PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}"
                                         SUFFIX "${PYTHON_MODULE_EXTENSION}")


#pybind11_add_module(pytschirp_rev2 PyTschirp-sequential-rev2.cpp)
#target_link_libraries(pytschirp_rev2 juce-utils midikraft-base midikraft-sequential-rev2 ${JUCE_LIBRARIES})

