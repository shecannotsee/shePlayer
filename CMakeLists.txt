########################################################################################################################

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-g")

########################################################################################################################

set(PROJECT_NAME "shePlayer" VERSION 0.0.1)
set(CMAKE_BUILD_TYPE "Debug") # [Release]or[Debug]or[MinSizeRel]
project(${PROJECT_NAME})

########################################################################################################################

set(test "ON")
set(library "ON")

# test set
if (test STREQUAL "ON")
    set(test_name "${PROJECT_NAME}_test")
else()
    message(STATUS "CMakeLists.txt error")
endif ()

# lib set
if (library STREQUAL "ON")
    set(library_static_name "${PROJECT_NAME}_static")
    set(library_dynamic_name "${PROJECT_NAME}_dynamic")
    message(STATUS "This is a library")
elseif(library STREQUAL "OFF")
    message(STATUS "This is an executable program")
else()
    message(STATUS "CMakeLists.txt error")
endif ()
# message(STATUS "${test_name}")
# message(STATUS "${library_static_name}")
# message(STATUS "${library_dynamic_name}")

########################################################################################################################

get_filename_component(cmake_upper_path ${CMAKE_SOURCE_DIR} DIRECTORY)
# message(STATUS "cmake_upper_path's path:${cmake_upper_path}")
set(libraries ${CMAKE_SOURCE_DIR}/third_party)
#set(ffmpeg ${libraries}/ffmpeg-5.1.3)
set(ffmpeg ${libraries}/ffmpeg-4.3.6)
set(SDL2 ${libraries}/SDL2)

########################################################################################################################

# include
include_directories(${CMAKE_SOURCE_DIR}/src
        ${ffmpeg}/include
        ${SDL2}/include)
# lib
link_directories(
        ${ffmpeg}/lib
        ${SDL2}/lib)

########################################################################################################################

file(GLOB_RECURSE SRC "${CMAKE_SOURCE_DIR}/src/*")
file(GLOB_RECURSE TEST_SRC "${CMAKE_SOURCE_DIR}/test/*")
# list(REMOVE_ITEM SRC "${CMAKE_SOURCE_DIR}/src/TaskControl.cpp") # Exclude specific files

# test target
add_executable(${test_name} ${TEST_SRC} ${SRC})
target_link_libraries(${test_name} "-pthread"
        -Wl,--start-group
        avcodec avfilter avformat avutil avdevice swresample swscale
        -Wl,--end-group
        z SDL2
        )

# lib target
if (library STREQUAL "ON")
    add_library(${library_static_name} STATIC ${SRC})
    # Rename the generated static library
    set_target_properties(${library_static_name} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

    add_library(${library_dynamic_name} SHARED ${SRC})
    # Rename the generated dynamic library
    set_target_properties(${library_dynamic_name} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    # exe target
elseif(library STREQUAL "OFF")
    add_executable(${PROJECT_NAME} ${TEST_SRC} ${SRC})
    target_link_libraries(${PROJECT_NAME} "-pthread")
    message(STATUS "This is an executable program")
    # error
else()
    message(STATUS "CMakeLists.txt error:target error")
endif ()


########################################################################################################################

# local debug
set(CMAKE_INSTALL_PREFIX "./")
# Release Code
#set(CMAKE_INSTALL_PREFIX "/")

message(STATUS "The default installation path is ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Please use \"make install DESTDIR=./PATH\" to set install path")

# test install
install(TARGETS
        ${test_name} DESTINATION ${PROJECT_NAME}/bin
        )

if (library STREQUAL "ON")
    # include install
    install(FILES
            ${CMAKE_SOURCE_DIR}/src/she_base64.h # source head
            DESTINATION # to
            ${PROJECT_NAME}/include # target dir
            )
    # lib install
    install(TARGETS
            ${library_static_name} ${library_dynamic_name}
            DESTINATION ${PROJECT_NAME}/include
            )
endif ()
########################################################################################################################
