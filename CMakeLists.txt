# Required CMake Version
###############
 cmake_minimum_required(VERSION 2.8)
#cmake_minimum_required(VERSION 3.1.0)

# Project setup
###############
PROJECT(ViennaLS)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# Default build type
##############
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Debug")
ENDIF()

# Default compile options
#################
SET(CMAKE_CXX_FLAGS "-Wall -pedantic")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 ")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

# Check flags for C++11 (C++0x)
##############
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


# Specify general include directory
################
INCLUDE_DIRECTORIES(".")
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/viennats/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/viennats/LSlib/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/viennats/Visualization/)

# Specify that the output executable is placed in the root build directory
################
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})


# Find prerequisites
####################

# Look for additiona CMake modules in the cmake subdirectory ..
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# OpenMP ..
FIND_PACKAGE(OpenMP REQUIRED)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

# Boost ..
SET(BOOST_MIN_VERSION 1.46.0)
FIND_PACKAGE(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS filesystem system)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

#Qt5
FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
FIND_PACKAGE(Qt5DataVisualization REQUIRED)
set(QT5_LIBRARIES Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DataVisualization)


ADD_DEFINITIONS(-DGUI_ENABLED)

# SPRNG ..
#FIND_PACKAGE(SPRNG REQUIRED)
#INCLUDE_DIRECTORIES(${SPRNG_INCLUDE_DIR})
#SET(LIBRARIES ${LIBRARIES} ${SPRNG_LIBRARY})

# HDF5
#FIND_PACKAGE(HDF5 COMPONENTS CXX)
#IF(HDF5_FOUND)
#  IF(HDF5_hdf5_cpp_LIBRARY)
#    add_definitions(-DUSE_HDF5)
#    add_definitions(${HDF5_DEFINITIONS})
#   INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
#    SET(LIBRARIES ${LIBRARIES} ${HDF5_LIBRARIES})
#  ELSE(HDF5_hdf5_cpp_LIBRARY)
#    message(FATAL_ERROR "Missing HDF5 C++ library")
#  ENDIF(HDF5_hdf5_cpp_LIBRARY)
#ELSE(HDF5_FOUND)
#  message(STATUS "Missing HDF5 library: ViennaTS will be built without TDR file support")
#ENDIF(HDF5_FOUND)

# Subdirectories
################
add_subdirectory(src)

# Add visibility of headers
# Necessary for Qt-Creator usage.
################
file( GLOB_RECURSE MAKE_HEADERS_VISIBLE_SRC *.hpp *.h)
add_custom_target( MAKE_HEADERS_VISIBLE SOURCES ${MAKE_HEADERS_VISIBLE_SRC} )
