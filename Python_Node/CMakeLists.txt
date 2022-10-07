project(Python_Node)

# Setup pybind11
include(../CMake/pybind11Config.cmake)
add_subdirectory(${CMAKE_BINARY_DIR}/_deps/pybind11-src ${CMAKE_BINARY_DIR}/_deps/pybind11-build)
include_directories(${CMAKE_BINARY_DIR}/_deps/pybind11_json-src/include)
include_directories(${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)

# Add IMGUI File Dialog
set(IMGUI_FILE_DIALOG_DIR ${FLOWCV_PROJ_DIR}/third-party/FileBrowser)
include_directories(${IMGUI_FILE_DIALOG_DIR})
if (WIN32)
    include_directories(${IMGUI_FILE_DIALOG_DIR}/Dirent)
endif()

add_library(
        ${PROJECT_NAME} SHARED
        python_node.cpp
        python_node_data.cpp
        ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileBrowser.cpp
        ${IMGUI_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
        ${FlowCV_SRC}
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        ${IMGUI_LIBS}
        ${OpenCV_LIBS}
        pybind11::embed
        pybind11::module
        pybind11::pybind11
)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
        SUFFIX ".fp"
        )
