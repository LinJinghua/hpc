cmake_minimum_required(VERSION 2.8)

project(hpc C)

# set(CMAKE_VERBOSE_MAKEFILE ON)

# Specify the minimum version you require.
# find_package(libmongoc-1.0 1.7 REQUIRED)

# message("--   mongoc found version \"${MONGOC_VERSION}\"")
# message("--   mongoc include path \"${MONGOC_INCLUDE_DIRS}\"")
# message("--   mongoc libraries \"${MONGOC_LIBRARIES}\"")

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS_DIR)

set(MONGOC_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/hiredis
    ${PROJECT_SOURCE_DIR}/include/libmongoc-1.0
    ${PROJECT_SOURCE_DIR}/include/libbson-1.0
)
set(MONGOC_LIBRARIES
    ${PROJECT_SOURCE_DIR}/lib/libbson-1.0.so
    ${PROJECT_SOURCE_DIR}/lib/libmongoc-1.0.so
    ${PROJECT_SOURCE_DIR}/lib/libhiredis.so
    ${PROJECT_SOURCE_DIR}/lib/libz.a
)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--disable-new-dtags,-rpath=$ORIGIN,-rpath=${PROJECT_SOURCE_DIR}/lib")

# add_compile_options(-O3 -Wextra -Wextra -Werror)

message("--   mongoc src  ${SRCS_DIR}")
message("--   mongoc project  ${PROJECT_SOURCE_DIR}")


add_executable(producer ${SRCS_DIR})
target_include_directories(producer PRIVATE "${MONGOC_INCLUDE_DIRS}")
target_link_libraries(producer PRIVATE "${MONGOC_LIBRARIES}")

# target_compile_definitions(producer PRIVATE DEBUG_CHECK)
target_compile_definitions(producer PRIVATE TEST_ZLIB)
# target_compile_definitions(producer PRIVATE TEST_REDIS_RW)

add_custom_command(TARGET producer
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/producer ${CMAKE_BINARY_DIR}/consumer
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/producer ${CMAKE_BINARY_DIR}/pmongo
)
