cmake_minimum_required(VERSION 3.21)
project(FlowCV_Python)

set(CMAKE_CXX_STANDARD 17)
if (WIN32)
    set( CMAKE_CXX_FLAGS "/EHsc " )
endif()

find_package( FlowCV REQUIRED )

add_subdirectory(Python_Node)
