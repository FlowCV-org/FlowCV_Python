message(STATUS "Configuring pybind11")

include(FetchContent)

# Fetch pybind11
FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11
        GIT_TAG v2.10.0
)
FetchContent_GetProperties(pybind11)
if (NOT pybind11_POPULATED)
    message(STATUS "Fetching pybind11 repo")
    FetchContent_Populate(pybind11)
    set(PYBIND11_PYTHON_VERSION 3.6.5 CACHE STRING "")
endif()

# Fetch nlohmann JSON
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.7.3
)
FetchContent_GetProperties(nlohmann_json)
if (NOT nlohmann_json_POPULATED)
    message(STATUS "Fetching nlohmann JSON repo")
    FetchContent_Populate(nlohmann_json)
endif()

# Fetch pybind11 JSON
FetchContent_Declare(
        pybind11_json
        GIT_REPOSITORY https://github.com/pybind/pybind11_json
        GIT_TAG 0.2.13
)
FetchContent_GetProperties(pybind11_json)
if (NOT pybind11_json_POPULATED)
    message(STATUS "Fetching pybind11_json repo")
    FetchContent_Populate(pybind11_json)
endif()
