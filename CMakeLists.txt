cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

    SET(CMAKE_BUILD_TYPE MinSizeRel)
    include(CheckCCompilerFlag)
    include(CheckCXXCompilerFlag)

project(doublealpha)

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
ROOT_GENERATE_DICTIONARY(G__eventDict dataItems.h LINKDEF LinkDef.h)
add_library(eventDict SHARED dataItems.cpp G__eventDict.cxx)
target_link_libraries(eventDict ${ROOT_LIBRARIES} -lTreePlayer)

add_executable(doubleAlpha main.cpp)
target_link_libraries(doubleAlpha ${ROOT_LIBRARIES} eventDict)

# Library threads
find_package (Threads)
# C++11
set_property(TARGET doubleAlpha PROPERTY CXX_STANDARD 14)
set_property(TARGET doubleAlpha PROPERTY CXX_STANDARD_REQUIRED ON)
# PIC
set_property(TARGET doubleAlpha PROPERTY POSITION_INDEPENDENT_CODE ON)
# 64-bit
check_c_compiler_flag(-m64 M64TestFlagC)
check_cxx_compiler_flag(-m64 M64TestFlagCXX)