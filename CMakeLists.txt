cmake_minimum_required(VERSION 3.0.0)
project(lab VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

AUX_SOURCE_DIRECTORY("${PROJECT_SOURCE_DIR}/src" main)

add_executable(lab 
    ${main}
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
