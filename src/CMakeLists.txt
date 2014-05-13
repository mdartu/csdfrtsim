cmake_minimum_required(VERSION 2.8)
project(rtsim)

set(CMAKE_CXX_FLAGS "-std=c++0x")

set(SRCS sc_schedulable_module.cc 
         sc_scheduler.cc 
         scheduler.cc 
         system/systemloader.cc 
         system/systemvalidator.cc
         systembuilder.cc 
         edfscheduler.cc
         globaledfscheduler.cc
         monitor.cc 
         graspmonitor.cc 
         statsmonitor.cc
)

find_package(LibXml2 REQUIRED)

if (NOT ${LIBXML2_FOUND})
  message(FATAL_ERROR Couldn't find libxml2)
endif()

find_path(SYSTEMC_INCLUDE_DIR systemc PATHS ${SYSTEMC_INCLUDE_DIR})
find_library(SYSTEMC_LIB systemc PATHS ${SYSTEMC_LIB_DIR} )

if (NOT SYSTEMC_LIB OR NOT SYSTEMC_INCLUDE_DIR)
  message(FATAL_ERROR "Couldn't find systemc. Please set SYSTEMC_INCLUDE_DIR and SYSTEMC_LIB.")
endif()

include_directories(${LIBXML2_INCLUDE_DIR} ${SYSTEMC_INCLUDE_DIR})
add_executable(test.bin test.cc ${SRCS})
target_link_libraries(test.bin ${LIBXML2_LIBRARIES} ${SYSTEMC_LIB})
