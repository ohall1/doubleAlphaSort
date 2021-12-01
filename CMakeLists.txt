cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

    SET(CMAKE_BUILD_TYPE MinSizeRel)
    include(CheckCCompilerFlag)
    include(CheckCXXCompilerFlag)

project(doubleAlpha)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree Net Graf3d Graf Gpad Thread)

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS} -lTreePlayer )
#ROOT_GENERATE_DICTIONARY(G__data2Tree data2Tree.hpp BRIKENTreeData.h LINKDEF data2TreeLinkDef.hpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(doubleAlpha main.cpp)
target_link_libraries(doubleAlpha ${ROOT_LIBRARIES})